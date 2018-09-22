#include "header.h"

amp_comp_context_t  *this_ctxt;
amp_request_t  *req = NULL;
test_msg_t   *msgp = NULL;
amp_message_t *reqmsgp = NULL;

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
int  network_init ()
{
	int err = 0;
	int addr;
	int totalnum = 10;
	struct in_addr  naddr;
	int size;
	int i;
	
	err = inet_aton("127.0.0.1", &naddr);
	if (!err) {
		printf("[main] wrong ip address\n");
		exit(1);
	}
	addr = htonl(naddr.s_addr);

	this_ctxt = amp_sys_init(CLIENT, CLIENT_ID1);
	if (!this_ctxt) {
		printf("[main] sys init error\n");
		exit(1);
	}

	err = amp_create_connection(this_ctxt, 
                                    SERVER, 
                                    SERVER_ID1, 
				    addr,
				    SERVER_PORT,
				    AMP_CONN_TYPE_TCP,
				    AMP_CONN_DIRECTION_CONNECT,
				    NULL,
				    __client_allocpages,
				    __client_freepages);
	if (err < 0) {
		printf("[main] connect to server error, err:%d\n", err);
		amp_sys_finalize(this_ctxt);
		exit(1);
	}
	return size;
}
/*	for(i=0; i<totalnum; i++) {
		memset(reqmsgp, 0, size);
		msgp = (test_msg_t *)((char *)reqmsgp + AMP_MESSAGE_HEADER_LEN);
		sprintf(msgp->msg, "Hi, this is No.%d client!!!!!!!!!!!!!!!!!!!!!!!!!, We need your reply", i);
		msgp->len = strlen(msgp->msg) + 1;
		msgp->type= 8;
		msgp->client_id = CLIENT_ID1;
		msgp->seqno = i;
		req->req_msg = reqmsgp;
		req->req_msglen = size;
		req->req_need_ack = 1;
		req->req_resent = 1;
		req->req_type = AMP_REQUEST|AMP_MSG;
		err = amp_send_sync(this_ctxt, req, SERVER, SERVER_ID1, 1);
		if (err < 0) {
			printf("send error, err:%d\n", err);
			exit(1);
		}
		msgp = (test_msg_t *)((char *)req->req_reply +  AMP_MESSAGE_HEADER_LEN);
		printf("Received from server:%s\n", msgp->msg);
		amp_free(req->req_reply, req->req_replylen);
	}
	gettimeofday(&endtm, NULL);

	timeused = (endtm.tv_sec - begintm.tv_sec) * 1000000 + endtm.tv_usec - begintm.tv_usec;

	printf("Total request num:%d, time used:%d usec\n", 
                totalnum, timeused);
	printf("Ratio: %d usec/ops\n", timeused/totalnum);
	amp_sys_finalize(this_ctxt);
	
	printf("Finished!\n");
*/
	//return 0;
