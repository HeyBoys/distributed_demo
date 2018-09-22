/*
 * client module for test amp
 */ 
#include "header.h"
static amp_comp_context_t *clt_ctxt = NULL;
#define MAX_THREAD_NUM (32)
#define MAX_PAGE_NUM  (64)

//amp_sem_t  clt_startsem[MAX_THREAD_NUM];
//amp_sem_t  clt_stopsem[MAX_THREAD_NUM];
//amp_sem_t  clt_beginworksem[MAX_THREAD_NUM];
static int clt_shutdown[MAX_THREAD_NUM];
static int total_req_num = 1;
static int total_thread_num = 24;
static int total_pages = 1;
static int thr_seqno[MAX_THREAD_NUM];
//pthread_t  clt_thread[MAX_THREAD_NUM];

int log_fd = -1;
#define LOG_FILE "./clt_log_file"

int
__client_allocpages (void *msgp, amp_u32_t *niov, amp_kiov_t **iov)
{
	int err = 0;
	amp_kiov_t  *kiov = NULL;
	test_msg_t *testmsgp = NULL;
	char *bufp = NULL;
	int page_num;
	int i, j;

	if (!msgp) {
		printf("__client_allocpages: no msgp!\n");
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
__client_freepages(amp_u32_t niov, amp_kiov_t **iov)
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


void*
__clt_service_thread(void *argv)
{
	int err = 0;
	int i;
	int seqno = 0;
	char thrname[16];
	amp_request_t *req = NULL;
	test_msg_t  *msgp = NULL;
	amp_message_t *reqmsgp = NULL;
	amp_message_t *replymsgp = NULL;
	amp_u32_t   size;
	struct timeval begintv, endtv;
	char tmpbuf[64];


//	seqno = *(int *)(argv);
//	printf("seqno:%d\n", seqno);

	if (seqno < 0 || seqno > MAX_THREAD_NUM) {
		printf("wrong seqno:%d!\n", seqno);
		return NULL;
	} 
	sprintf(thrname, "cltthr_%d", seqno);
//	amp_sem_up(&clt_startsem[seqno]);
	
	err = __amp_alloc_request(&req);
	if (err < 0) {
		printf("clthr_%d: alloc for request error,err:%d\n", seqno, err);
		err = 0;
		goto EXIT;
	}

	size = AMP_MESSAGE_HEADER_LEN + sizeof(test_msg_t);
	reqmsgp = (amp_message_t *)malloc(size);
	if (!reqmsgp) {
		printf("clthr_%d: alloc for memory error\n", seqno);
		err = -ENOMEM;
		goto EXIT;
	}

//	amp_sem_down(&clt_beginworksem[seqno]);
//	gettimeofday(&begintv, NULL);
	for (i=0; i<total_req_num; i++) {
		//if (i % 100 == 0)
		//printf("clthr_%d: to serve No.%d request\n", seqno, i);
		memset(reqmsgp, 0, size);
		msgp = (test_msg_t *)((char *)reqmsgp + AMP_MESSAGE_HEADER_LEN);
		sprintf(msgp->buf, 
                      " clthr_%d, No.%d msg", 
                        seqno, 
                        i + 1);
		msgp->len = strlen(msgp->buf);
		
		msgp->page_num = total_pages;
		msgp->type = READ_DATA;

		req->req_msg = reqmsgp;
		req->req_msglen = size;
		req->req_need_ack = 1;
		req->req_resent = 1;
		req->req_type = AMP_REQUEST | AMP_MSG;
		req->req_iov = NULL;
		req->req_niov = 0;
		
		sprintf(tmpbuf, "clt_%d, No.%d msg\n", seqno, i + 1);
		tmpbuf[strlen(tmpbuf) + 1] = '\0';
		if (write(log_fd, tmpbuf, strlen(tmpbuf)) != strlen(tmpbuf)) {
			printf("clthr_%d: write %s to log file error, errno:%d\n", 
                                seqno, tmpbuf, errno);
			exit(-1);
		}
		

		//printf("clthr_%d: before send msg, req:%p\n", seqno, req);
SEND_AGAIN:
		err = amp_send_sync(clt_ctxt, req, SERVER, 1, 0);
		if (err < 0) {
			printf("clthr_%d: send No.%d request error, err:%d\n", seqno, i + 1, err);
			/*	

			sleep(3);
			goto SEND_AGAIN;
			*/
			goto EXIT;
		}
		/*
		erret amp_send_async(clt_ctxt, req, SERVER, 1, 0);
		if (err < 0) {
			printf("clthr_%d: send request error, err:%d\n", seqno, err);
			goto EXIT;
		}
		amp_sem_down(&req->req_waitsem);
		if (req->req_error < 0) {
			err = req->req_error;
			printf("clthr_%d: send request error, err:%d\n", 
                               seqno, err);
			goto EXIT;
		}
		*/

		//printf("clthr_%d: send the message, and to receive ack from server\n", seqno);

		replymsgp = req->req_reply;
		if (!replymsgp) 
			printf("clthr_%d: No.%d request no reply gotten\n", seqno, i+1);
		else {
			msgp = (test_msg_t *)((char *)replymsgp + AMP_MESSAGE_HEADER_LEN);
			//printf("clthr_%d: msg replied from server:%s\n", seqno, msgp->buf);
			if(!req->req_iov) {
				printf("clthr_%d: No.%d request no data read\n", seqno, i + 1);
			} else {
				int j;
				for(j = 0;j<req->req_niov;j++)
					printf("%s",req->req_iov[j].ak_addr);
				__client_freepages(req->req_niov, &req->req_iov);
				free(req->req_iov);
				req->req_iov = NULL;
				req->req_niov = 0;
			}
			amp_free(replymsgp, req->req_replylen);
		}
	}

//	gettimeofday(&endtv, NULL);

	fsync(log_fd);
	
EXIT:
	if (reqmsgp)
		free(reqmsgp);

	if (req) {
		printf("clthr_%d: free request: %p\n", seqno, req);
		__amp_free_request(req);
	}

/*	printf("clthr_%d: total_req_num: %d, total_thread_num:%d, total_pages:%d, timeused: %ld sec\n",
		seqno, total_req_num, total_thread_num, total_pages, endtv.tv_sec - begintv.tv_sec);
	printf("clthr_%d: to be down, bye bye\n", seqno);
	amp_sem_up(&clt_stopsem[seqno]);*/
	return NULL;
}

int 
main ()
{
	amp_s32_t  err = 0;
	amp_u32_t  addr;
	amp_s32_t  i;
	struct in_addr inaddr;
	printf("client: enter\n");

	log_fd = open(LOG_FILE, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (log_fd < 0) {
		printf("open log file error, err:%d\n", errno);
		exit(-1);
	}
	
/*	for (i=0; i<MAX_THREAD_NUM; i++) {
		amp_sem_init_locked(&clt_startsem[i]);
		amp_sem_init_locked(&clt_stopsem[i]);
		amp_sem_init_locked(&clt_beginworksem[i]);
		clt_shutdown[i] = 0;
	}*/

	clt_ctxt = amp_sys_init(CLIENT, CLIENT1_ID);
	if (!clt_ctxt) {
		printf("client: sys init error\n");
		err = -1;
		goto EXIT;
	}

	if (!inet_aton("127.0.0.1", &inaddr)) {
		printf("client: inet error\n");
		err = -1;
		goto EXIT;
	}
	addr = inaddr.s_addr;
	addr = ntohl(addr);
	err = amp_create_connection(clt_ctxt, 
		                    SERVER, 
				    1, 
				    addr,
				    SERVER_PORT, 
				    AMP_CONN_TYPE_TCP,
				    AMP_CONN_DIRECTION_CONNECT,
				    NULL,
				    __client_allocpages,
				    __client_freepages);
	
	if (err < 0) {
		printf("client: connect to server error, err:%d\n", err);
		amp_sys_finalize(clt_ctxt);
		err = -1;
		goto EXIT;
	}
__clt_service_thread(0);
/*	for(i=0; i<total_thread_num; i++) {
		thr_seqno[i] = i;
		if (pthread_create(&clt_thread[i], 
                                   NULL, 
                                   __clt_service_thread, 
                                   &thr_seqno[i])) {
			printf("client: create No.%d thread error, errno:%d\n", i+1, errno);
			err = -errno;
			goto EXIT;
		}
	}
*/	
//	printf("client: tell all thread to work\n");

//	for(i=0; i<total_thread_num; i++)
	//	amp_sem_up(&clt_beginworksem[i]);
//	printf("!!!!!\n");
//	for(i=0; i<total_thread_num; i++)
//		amp_sem_down(&clt_stopsem[i]);

	amp_sys_finalize(clt_ctxt);

EXIT:
	printf("client: leave\n");
	return err;
}

/*end of file*/
