#include "analysis.h"
#define PAGE_SIZE 4096
void do_open(amp_request_t  *req)
{
     return;
}
void do_getattr(amp_request_t  *req)
{
/*	struct stat st;
	int res = stat(msgp->msg.buf,&st);
	if(res >0)
	{
		*(msgp->msg.st) = st;
	}
*/	

}
void do_read(amp_request_t  *req)
{
	printf("Yes,wo are in server read\n");
	int fd = open("read.txt",O_RDONLY);
	test_msg_t *msgp;
	msgp = (test_msg_t *)((char *)req->req_msg + AMP_MESSAGE_HEADER_LEN);
	printf("ssize:%d\n",msgp->ssize);
	for(int i = 0;i<msgp->page_num-1;i++)
		read(fd,req->req_iov[i].ak_addr,PAGE_SIZE);
	int last_page_size = msgp->ssize % PAGE_SIZE; 
	if(last_page_size == 0)
		read(fd,req->req_iov[msgp->page_num-1].ak_addr,PAGE_SIZE);
	else
		read(fd,req->req_iov[msgp->page_num-1].ak_addr,last_page_size);
}
void do_readdir(amp_request_t  *req)
{
     return;
}
void do_write(amp_request_t  *req)
{
	printf("Yes,wo are in server write\n");
	test_msg_t *msgp;
	msgp = (test_msg_t *)((char *)req->req_msg + AMP_MESSAGE_HEADER_LEN);
//	int fd = open(msgp->buf,O_WRONLY);
	int fd = open("write.txt",O_WRONLY|O_CREAT,0644);
	//write(fd,,)i
	int last_page_size = msgp->ssize % 4096;
	for(int i = 0;i<msgp->page_num-1;i++)
	{
		write(fd,req->req_iov[i].ak_addr,PAGE_SIZE);
	}
	if(last_page_size == 0)
		write(fd,req->req_iov[msgp->page_num-1].ak_addr,PAGE_SIZE);
	else
		write(fd,req->req_iov[msgp->page_num-1].ak_addr,last_page_size);

}
void  work(amp_request_t  *req)
{   
    test_msg_t *msgp;
	msgp = (test_msg_t *)((char *)req->req_msg + AMP_MESSAGE_HEADER_LEN);
	switch(msgp->command)
    {   
        case OPEN:do_open(req);break;
        case READDIR:do_readdir(req);break;
		case GETATTR:do_getattr(req);break;
		case WRITE:do_write(req);break;
		case READ:do_read(req);break;
    }
	
    return;
}
