/*
 * server module for test amp
 */ 
#include "amp.h"
#include "header.h"

#define MAX_SERVER_THREAD  16

static amp_comp_context_t *ctxt = NULL;
amp_sem_t  request_sem;
amp_sem_t  startsem[MAX_SERVER_THREAD];
amp_sem_t  stopsem[MAX_SERVER_THREAD];
LIST_HEAD(request_queue);
amp_lock_t request_queue_lock;
static int server_shutdown[MAX_SERVER_THREAD];
static int server_thread_num = 10;
static int server_seqno[MAX_SERVER_THREAD];
pthread_t  server_thread[MAX_SERVER_THREAD];

#define LOG_FILE "./log-file"
int  log_fd;

int 
__queue_req(amp_request_t *req)
{
	//printf("__queue_req: enter\n");
	
	amp_lock(&request_queue_lock);
	list_add_tail(&req->req_list, &request_queue);
	amp_sem_up(&request_sem);
	amp_unlock(&request_queue_lock);
	return 1;
}

int
__server_allocpages (void *msgp, amp_u32_t *niov, amp_kiov_t **iov)
{
	int err = 0;
	amp_kiov_t  *kiov = NULL;
	test_msg_t *testmsgp = NULL;
	char *bufp = NULL;
	int page_num;
	int i, j;

	if (!msgp) {
		printf("__server_allocpages: no msgp!\n");
		err = -EINVAL;
		goto EXIT;
	}

	testmsgp = (test_msg_t *)msgp;

	page_num = testmsgp->page_num;

	kiov = (amp_kiov_t *)malloc(sizeof(amp_kiov_t) * page_num);
	if (!kiov) {
		printf("__server_allocpages: alloc for kiov error\n");
		err = -ENOMEM;
		goto EXIT;
	}
	memset(kiov, 0, sizeof(amp_kiov_t) * page_num);
	for (i=0; i<page_num; i++) {
		bufp = (char *)malloc(4096);
		if (!bufp) {
			printf("__server_allocpages: alloc page error\n");
			for (j=0; j<i; j++)
				free(kiov[j].ak_addr);
			free(kiov);
			err = -ENOMEM;
			goto EXIT;
		}
		memset(bufp, 0, 4096);
		kiov[i].ak_addr = bufp;
		kiov[i].ak_len = 4096;
		kiov[i].ak_offset = 0;
		kiov[i].ak_flag = 0;
	}

	*iov = kiov;
	*niov = page_num;

EXIT:
	//printf("__server_allocpages: page_num:%d\n", page_num);
	return err;
}

void
__server_freepages(amp_u32_t niov, amp_kiov_t **iov)
{
	int i;
	amp_kiov_t  *kiovp = NULL;

	kiovp = *iov;
	for(i=0; i<niov; i++) {
		if (kiovp->ak_addr)
			free(kiovp->ak_addr);
		kiovp ++;
	}
}


void *
__service_thread(void *argv)
{
	int err = 0;
	amp_request_t  *req = NULL;
	test_msg_t  *msgp = NULL;
	amp_kiov_t  *kiovp = NULL;
	int i;
	int seqno;
	char thread_name[32];
	int  type;
	
	seqno = *((int *)argv);

	sprintf(thread_name, "mythread_%d", seqno);

	amp_sem_up(&startsem[seqno]);
	printf("No.%d thread started!\n", seqno);

AGAIN:
	amp_sem_down(&request_sem);
	
	if (server_shutdown[seqno])
		goto EXIT;

	amp_lock(&request_queue_lock);
	if (list_empty(&request_queue)) {
		amp_unlock(&request_queue_lock);
		goto AGAIN;
	}
	req = list_entry(request_queue.next, amp_request_t, req_list);
	list_del_init(&req->req_list);
	amp_unlock(&request_queue_lock);
	
	msgp = (test_msg_t *)((char *)req->req_msg + AMP_MESSAGE_HEADER_LEN);
	type = msgp->type;
	msgp->buf[strlen(msgp->buf)] = '\n';
	msgp->buf[strlen(msgp->buf) + 1] = '\0';
	
	if (write(log_fd, msgp->buf, strlen(msgp->buf)) != 
	    strlen(msgp->buf)) {
		printf("write to log file error, err:%d\n", errno);
		exit(-1);
	}
	

	//printf("%s\n", msgp->buf);
	

	if (req->req_type & AMP_DATA) {
		//printf("__service_thread: request has data, page number:%d\n", req->req_niov);
		kiovp = req->req_iov;
		for(i = 0; i < req->req_niov; i++)
		{
//				printf("%dst:\n%s\n", i+1, kiovp[i].ak_addr);
			memset(kiovp[i].ak_addr,'d'+ i,4095);

		}
	/*	if (!kiovp) 
			printf("__service_thread: no req_iov\n");
		else {
			for (i=0; i<req->req_niov; i++) {
			
				bufp = kiovp[i].ak_addr;
				printf("received data: %s\n", bufp);
				
				free(kiovp[i].ak_addr);
			}
			free(kiovp);
			req->req_iov = NULL;
		}*/
	}
	
	if (type == READ_DATA) {
		err = __server_allocpages(msgp, &req->req_niov, &req->req_iov);
		if (err < 0)
			printf("__service_thread: alloc for page error\n");
		
	}
	
	msgp->type = 0;
	sprintf(msgp->buf, "This is server: %s!", thread_name);
	msgp->len = strlen(msgp->buf);

	req->req_reply = req->req_msg; 
	req->req_msg = NULL;
	req->req_replylen = req->req_msglen;
	req->req_need_ack = 0;
	req->req_resent = 0;
	if (type == READ_DATA && err >= 0)
		req->req_type = AMP_REPLY|AMP_DATA;
	else
		req->req_type = AMP_REPLY|AMP_MSG;

	/*
	printf("__service_thread: before send req: %p, req_reply:%p, type:%d, id:%d\n", 
                req, 
                req->req_reply,
		req->req_remote_type,
		req->req_remote_id);
	*/
	err = amp_send_sync(ctxt, req, req->req_remote_type, req->req_remote_id, 0);
	/*printf("__service_thread: after send req\n");*/
	
	amp_free(req->req_reply, req->req_replylen);
	if (req->req_iov) {
		__server_freepages(req->req_niov, &req->req_iov);
		free(req->req_iov);
		req->req_iov = NULL;
		req->req_niov = 0;
	}
	__amp_free_request(req);
	goto AGAIN;

EXIT:
	printf("%s: to be down, bye bye\n", thread_name);
	amp_sem_up(&stopsem[seqno]);
	return NULL;
}

int 
main (void)
{
	amp_s32_t  err = 0;
	amp_s32_t  i;

	printf("server: enter\n");

	log_fd = open(LOG_FILE, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (log_fd < 0) {
		printf("create log file:%s error, errno:%d\n", \
                        LOG_FILE, errno);
		exit(1);
	}

	amp_sem_init_locked(&request_sem);

	for (i=0; i<MAX_SERVER_THREAD; i++) {
		amp_sem_init_locked(&startsem[i]);
		amp_sem_init_locked(&stopsem[i]);
		server_shutdown[i] = 0;
		server_seqno[i] = i;
	}

	amp_lock_init(&request_queue_lock);

	ctxt = amp_sys_init(SERVER, 1);
	if (!ctxt) {
		printf("server: sys init error\n");
		err = -1;
		goto EXIT;
	}

	err = amp_create_connection(ctxt, 
		                    CLIENT, 
				    1, 
				    INADDR_ANY, 
				    SERVER_PORT, 
				    AMP_CONN_TYPE_TCP,
				    AMP_CONN_DIRECTION_LISTEN,
				    __queue_req,
				    __server_allocpages,
				    __server_freepages);
	if (err < 0) {
		printf("server: create listen connection error, err:%d\n", err);
		amp_sys_finalize(ctxt);
		err = -1;
		goto EXIT;
	}


	printf("server: start service thread\n");
	
	for (i=0; i<server_thread_num; i++) {
		server_seqno[i] = i;
		if (pthread_create(&server_thread[i], NULL, __service_thread, &server_seqno[i])) {
			printf("server: create No.%d service thread error, errno:%d\n", i+1, errno);
			amp_sys_finalize(ctxt);
			return -1;
		}
		amp_sem_down(&startsem[i]);
	}
	
	while (1) {
		sleep(3);
		if (fsync(log_fd)) {
			printf("sync log file error, errno:%d\n", errno);
			exit(-1);
		}
		if (!list_empty(&request_queue))
			printf("request queue is not empty\n");
	}

	amp_sys_finalize(ctxt);

EXIT:
	printf("server: leave\n");
	return err;
}

/*end of file*/
