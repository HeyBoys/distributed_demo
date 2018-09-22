/*
 * client module for test amp
 */ 
#include "header.h"
static amp_comp_context_t *clt_ctxt = NULL;
#define MAX_THREAD_NUM (32)
#define PAGE_SIZE (4096)
#define TOTAL_PAGE (4)
#define MAX_PAGE_NUM  (64)

static int total_req_num = 10000;
static int total_thread_num = 24;
static int total_pages = 1;
static int thr_seqno[MAX_THREAD_NUM];

int log_fd = -1;
#define LOG_FILE "./clt_log_file"
int total_num = 10;
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
	char tmpbuf[64];
	amp_kiov_t * kiov = NULL;	
	char * bufpp[TOTAL_PAGE];
	char * bufp = NULL;	
	err = __amp_alloc_request(&req);
	if(0 != err)
	{
		printf("alloc request error, err:%d\n", err);
		exit(1);
	}
				
	size= AMP_MESSAGE_HEADER_LEN + sizeof(test_msg_t);
	reqmsgp = (amp_message_t *)malloc(size);
	if(!reqmsgp)
	{
		printf("alloc for msg error, err:%d\n", err);
		exit(1);	
	}
	kiov = (amp_kiov_t *)malloc(sizeof(amp_kiov_t) * TOTAL_PAGE);
	if(!kiov)
	{
		printf("[main] kiov alloc memory failed\n");
		return 0;	
	}
	int j;
	memset(kiov, 0, sizeof(amp_kiov_t)*TOTAL_PAGE);
	for(i = 0; i < TOTAL_PAGE; i++)
	{
		bufpp[i] = (char *)malloc(PAGE_SIZE);
		if(!bufpp[i])
		{
			for(j = 0; j < i; j++)
			{
				free(bufpp[j]);	
			}	
			return 0;
		}
	}
	for(i=0; i<total_num;i++)
	{
		memset(reqmsgp,0,size);
		msgp = (test_msg_t *)((char*)reqmsgp + AMP_MESSAGE_HEADER_LEN);
		msgp->type = 0;
		msgp->len = strlen(msgp->msg) + 1;
		msgp->seqno = i;
		msgp->page_num = TOTAL_PAGE;
		msgp->client_id = CLIENT_ID1;
		for(j = 0; j < TOTAL_PAGE; j++)
		{
			char tmpch;
			tmpch = 'z' - j % 26;
			memset(bufpp[j], tmpch, PAGE_SIZE);
			bufpp[j][PAGE_SIZE-1] = '\0'; 
			kiov[j].ak_addr = bufpp[j];
			kiov[j].ak_len = PAGE_SIZE;
			kiov[j].ak_offset = 0;
			kiov[j].ak_flag = 0;	
		}


		sprintf(msgp->msg, "Hi, this is No.%d request, I send you some data, wait for your reply!", i);
		req->req_msg = reqmsgp;
		req->req_msglen = size;
		req->req_iov = kiov;
		req->req_niov = TOTAL_PAGE;
		req->req_need_ack = 1;
		req->req_resent = 1;
		req->req_type = AMP_REQUEST|AMP_DATA;
																			
		err = amp_send_sync(clt_ctxt, req, SERVER, SERVER_ID1, 1);
		if(0 != err)
		{
			printf("send error, err:%d\n", err);
			exit(1);
		}
		msgp = (test_msg_t *)((char*)req->req_reply + AMP_MESSAGE_HEADER_LEN);
		printf("Received from server:%s\n",msgp->buf);
		
		for(int i = 0;i<msgp->page_num;i++)
		{
			printf("%d:%s",i,req->req_iov[i].ak_addr);
		}
	/*	__client_freepages(req->req_niov, &req->req_iov);
		free(req->req_iov);
		req->req_iov = NULL;
		req->req_niov = 0;*/
		amp_free(req->req_reply, req->req_replylen);
	}
	if(kiov)
		free(kiov);
	for(i = 0; i < TOTAL_PAGE; i++)
		free(bufpp[i]);
	__amp_free_request(req);
	printf("Finished!\n");
	

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
/*	
	for (i=0; i<MAX_THREAD_NUM; i++) {
		amp_sem_init_locked(&clt_startsem[i]);
		amp_sem_init_locked(&clt_stopsem[i]);
		amp_sem_init_locked(&clt_beginworksem[i]);
		clt_shutdown[i] = 0;
	}
*/
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
    //for (i = 0; i < 16 ; i ++){
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

   // }

	/*for(i=0; i<total_thread_num; i++) {
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
	
	amp_sys_finalize(clt_ctxt);

EXIT:
	printf("client: leave\n");
	return err;
}

/*end of file*/
