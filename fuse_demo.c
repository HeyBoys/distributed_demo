/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */


#define FUSE_USE_VERSION 30
#define PAGE_SIZE 4096
#include <config.h>

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "client.h"
extern amp_comp_context_t  *this_ctxt;
int size;
/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *demo_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int demo_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	} else
		res = -ENOENT;
	
/*	int err;

	memset(reqmsgp,0,size);
	msgp =  (test_msg_t *)((char *)reqmsgp + AMP_MESSAGE_HEADER_LEN);
	strcpy(msgp->msg.buf,path); 
	msgp->len = strlen(msgp->msg.buf) + 1;
	msgp->client_id = CLIENT_ID1;
	msgp->type = GETATTR;
	req->req_msg = reqmsgp;
	req->req_msglen = size;
	req->req_need_ack = 1;
	req->req_resent = 1;
	req->req_type = AMP_REQUEST|AMP_MSG;
	err = amp_send_sync(this_ctxt, req, SERVER, SERVER_ID1, 1);
	if(err<0)
	{
		printf("send error:%d\n",err);
		exit(1);
	}
	msgp = (test_msg_t *)((char *)req->req_reply +  AMP_MESSAGE_HEADER_LEN);
	printf("Received sucess\n");
//	*stbuf = *msgp->msg.st;*/
	return res;
}

static int demo_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	filler(buf, options.filename, NULL, 0, 0);

	
	return 0;
}

static int demo_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path+1, options.filename) != 0)
		return -ENOENT;

//	if ((fi->flags & 3) != O_RDONLY)
//		return -EACCES;

	return 0;
}
static int demo_write(const char* path, const char* buf, size_t ssize, off_t offset, struct fuse_file_info* fi)
{
	char xxx[100]= {'0'};
	int err;
	amp_request_t  *req ;
	test_msg_t   *msgp ;
	amp_message_t *reqmsgp ;

/*	err = __amp_alloc_request(&req);
	if (err < 0) {
		printf("alloc request error, err:%d\n", err);
		exit(1);
	}
	size = AMP_MESSAGE_HEADER_LEN + sizeof(test_msg_t);
	reqmsgp = (amp_message_t *)malloc(size);
	if (!reqmsgp) {
		printf("alloc for req msg error, err:%d\n", err);
		exit(1);
	}
	memset(reqmsgp,0,size)		;
	msgp = (test_msg_t *)((char*)reqmsgp + AMP_MESSAGE_HEADER_LEN);
	int page_number = ssize / PAGE_SIZE;
	unsigned int	last_page_size = ssize % 4096;
	msgp->page_num = page_number + (last_page_size != 0);
	__client_allocpages(msgp,&req->req_niov,&req->req_iov);
	for(int i = 0;i<page_number;i++)
	{
		memcpy(req->req_iov[i].ak_addr,buf+PAGE_SIZE * i,PAGE_SIZE);
	}
	
	if(last_page_size != 0)
		{
			memcpy(req->req_iov[page_number].ak_addr,buf+PAGE_SIZE * page_number,last_page_size);
			req->req_iov[page_number].ak_len = last_page_size;
		}
	memset(req->req_iov[0].ak_addr,'a',4095);
	req->req_iov[page_number].ak_addr[req->req_iov[page_number].ak_len] = '\0';
	printf("data:%s",req->req_iov[page_number].ak_addr);
	printf("page_number:%d\n",page_number);
	printf("msgp->page_num:%d\n",msgp->page_num);
	printf("len:%d\n",req->req_iov[page_number].ak_len);
	printf("niov:%d\n",req->req_niov);
	printf("iov:%p\n",&req->req_iov[0]);
//	memset(req->req_iov[0].ak_addr,'a',4095);
//	msgp->type = READ_DATA;
	msgp->command = WRITE;
//	msgp->len = ssize;
//	sprintf(msgp->msg,"This is clinent write data\n");
//	memcpy(msgp->buf,path,strlen(path));
//	req->req_iov[0].ak_addr[4095] = '\0';
	req->req_need_ack = 1;
	req->req_resent = 1;
	req->req_msg = reqmsgp;
	req->req_msglen = size;
	req->req_type = AMP_REQUEST | AMP_DATA;
//	sprintf(xxx,"%s",req->req_iov[0].ak_addr);
	//write(fd,xxx,strlen(xxx));
	err = amp_send_sync(this_ctxt, req, SERVER, SERVER_ID1, 1);
	amp_message_t *replymsgp = NULL;
	replymsgp = req->req_reply;
//	msgp = (test_msg_t *)((char *)replymsgp + AMP_MESSAGE_HEADER_LEN);
//	printf("%s\n",msgp->msg);
	

	//printf("recv from server%s",req->req_iov[0].ak_addr);

	if(reqmsgp)
		free(reqmsgp);
	if(req)
		__amp_free_request(req);*/
//	 amp_sys_finalize(this_ctxt);
	err = __amp_alloc_request(&req);
	if (err < 0) {
		printf("alloc request error, err:%d\n", err);
		exit(1);
	}
	size = AMP_MESSAGE_HEADER_LEN + sizeof(test_msg_t);
	reqmsgp = (amp_message_t *)malloc(size);
	if (!reqmsgp) {
		printf("alloc for req msg error, err:%d\n", err);
		exit(1);
	}
	memset(reqmsgp,0,size)		;
	msgp = (test_msg_t *)((char*)reqmsgp + AMP_MESSAGE_HEADER_LEN);
//	msgp->page_num = 1;
//	__client_allocpages(msgp,&req->req_niov,&req->req_iov);

	int page_number = ssize / PAGE_SIZE;
	unsigned int	last_page_size = ssize % 4096;
	msgp->page_num = page_number + (last_page_size != 0);
	__client_allocpages(msgp,&req->req_niov,&req->req_iov);
	for(int i = 0;i<page_number;i++)
	{
		memcpy(req->req_iov[i].ak_addr,buf+PAGE_SIZE * i,PAGE_SIZE);
	}
	
	if(last_page_size != 0)
		{
			memcpy(req->req_iov[page_number].ak_addr,buf+PAGE_SIZE * page_number,last_page_size);
//			req->req_iov[page_number].ak_len = last_page_size;
		}
//	memset(req->req_iov[0].ak_addr,'a',4095);
//	req->req_iov[0].ak_addr[4095] = '\0';

	
//		printf("%s\n",req->req_iov[0].ak_addr);
	msgp->ssize = ssize;
	msgp->command = WRITE;
	req->req_need_ack = 1;
	req->req_resent = 1;
	req->req_msg = reqmsgp;
	req->req_msglen = size;
//	req->req_iov = NULL;
//	req->req_niov = 0;
	req->req_type = AMP_REQUEST | AMP_DATA;
	err = amp_send_sync(this_ctxt, req, SERVER, SERVER_ID1, 1);
	amp_message_t *replymsgp = NULL;
	replymsgp = req->req_reply;
	msgp = (test_msg_t *)((char *)replymsgp + AMP_MESSAGE_HEADER_LEN);

	if(!req->req_iov) {
		printf("No.%d request no data read\n",  0 + 1);
	} else {
			int j;
			for(j = 0;j<req->req_niov;j++)
				printf("%s\n",req->req_iov[j].ak_addr);
				__client_freepages(req->req_niov, &req->req_iov);
				free(req->req_iov);
				req->req_iov = NULL;
				req->req_niov = 0;
			}
			amp_free(replymsgp, req->req_replylen);
	if(reqmsgp)
		free(reqmsgp);
	if(req)
		__amp_free_request(req);
//	 amp_sys_finalize(this_ctxt);*/
	 return ssize;
}
static int demo_read(const char *path, char *buf, size_t ssize, off_t offset,
		      struct fuse_file_info *fi)
{
/*	size_t len;
	(void) fi;
	if(strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	len = strlen(options.contents);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, options.contents + offset, size);
	} else
		size = 0;
*/
	printf("ssize:%lu\n" ,ssize);
	amp_request_t  *req ;
	test_msg_t   *msgp ;
	amp_message_t *reqmsgp ;
	int err;
	err = __amp_alloc_request(&req);
	if (err < 0) {
		printf("alloc request error, err:%d\n", err);
		exit(1);
	}
	size = AMP_MESSAGE_HEADER_LEN + sizeof(test_msg_t);
	reqmsgp = (amp_message_t *)malloc(size);
	if (!reqmsgp) {
		printf("alloc for req msg error, err:%d\n", err);
		exit(1);
	}
	memset(reqmsgp,0,size)		;
	msgp = (test_msg_t *)((char*)reqmsgp + AMP_MESSAGE_HEADER_LEN);
	msgp->type = READ_DATA;
	msgp->ssize = ssize;
	int page_number = ssize / PAGE_SIZE;
	int last_page_size= ssize % PAGE_SIZE;
	msgp->command = READ;
	msgp->page_num = page_number + (last_page_size != 0 );
	msgp->ssize = ssize;
	req->req_need_ack = 1;
	req->req_resent = 1;
	req->req_msg = reqmsgp;
	req->req_msglen = size;
	req->req_type = AMP_REQUEST | AMP_MSG;
	err = amp_send_sync(this_ctxt, req, SERVER, SERVER_ID1, 1);
	amp_message_t *replymsgp = NULL;
	replymsgp = req->req_reply;
	msgp = (test_msg_t *)((char *)replymsgp + AMP_MESSAGE_HEADER_LEN);
	if(!req->req_iov) {
		printf("No.%d request no data read\n",  0 + 1);
	} 
	else 
	{
		int j;
		int last_page_size = ssize % PAGE_SIZE;
		for(j = 0;j<req->req_niov-1;j++)
			memcpy(buf+j*PAGE_SIZE,req->req_iov[j].ak_addr,PAGE_SIZE);
		if(last_page_size == 0)
			memcpy(buf+(req->req_niov-1)*PAGE_SIZE,req->req_iov[req->req_niov-1].ak_addr,PAGE_SIZE);
		else
			memcpy(buf+(req->req_niov-1)*PAGE_SIZE,req->req_iov[req->req_niov-1].ak_addr,last_page_size);
			free(req->req_iov);
			req->req_iov = NULL;
			req->req_niov = 0;
	}
	amp_free(replymsgp, req->req_replylen);
	if(reqmsgp)
		free(reqmsgp);
	if(req)
		__amp_free_request(req);
	
	return ssize;
}

static struct fuse_operations demo_oper = {
	.init           = demo_init,
	.getattr	= demo_getattr,
	.readdir	= demo_readdir,
	.open		= demo_open,
	.read		= demo_read,
	.write		= demo_write,
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
//	close(1);
	//printf("fd:%d\n",fd);
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("hello");
	options.contents = strdup("Hello World!\n");

	/* Parse options */

	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*) "";
	}
	network_init();

	printf("network_init sucess\n");
/*	memset(reqmsgp,0,size)		;
	msgp = (test_msg_t *)((char*)reqmsgp + AMP_MESSAGE_HEADER_LEN);
	msgp->page_num = 1;
	__client_allocpages(msgp,&req->req_niov,&req->req_iov);
	memset(req->req_iov[0].ak_addr,'a',4095);
	msgp->type = READ_DATA;
	req->req_iov[0].ak_addr[4095] = '\0';
	
		printf("%s\n",req->req_iov[0].ak_addr);
	req->req_need_ack = 1;
	req->req_resent = 1;
	req->req_msg = reqmsgp;
	req->req_msglen = size;
//	req->req_iov = NULL;
//	req->req_niov = 0;
	req->req_type = AMP_REQUEST | AMP_DATA;
	int err;
	err = amp_send_sync(this_ctxt, req, SERVER, SERVER_ID1, 1);
	amp_message_t *replymsgp = NULL;
	replymsgp = req->req_reply;
	msgp = (test_msg_t *)((char *)replymsgp + AMP_MESSAGE_HEADER_LEN);

	if(!req->req_iov) {
		printf("No.%d request no data read\n",  0 + 1);
	} else {
			int j;
			for(j = 0;j<req->req_niov;j++)
				printf("%s\n",req->req_iov[j].ak_addr);
				__client_freepages(req->req_niov, &req->req_iov);
				free(req->req_iov);
				req->req_iov = NULL;
				req->req_niov = 0;
			}
		//	amp_free(replymsgp, req->req_replylen);
	

//	printf("recv from server%s",req->req_iov[0].ak_addr);*/
/*	if(reqmsgp)
		free(reqmsgp);
	if(req)
		__amp_free_request(req);
	 amp_sys_finalize(this_ctxt);*/
	return fuse_main(args.argc, args.argv, &demo_oper, NULL);
}
