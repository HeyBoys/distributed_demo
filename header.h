#ifndef __HEADER_H_
#define __HEADER_H_
#include "amp.h"

#define  SERVER_PORT 5446

#define SERVER  1
#define CLIENT  2


#define SERVER_ID1   1
#define SERVER_ID2   2
#define SERVER_ID3   3
#define SERVER_ID4   4


#define CLIENT_ID1   1
#define CLIENT_ID2   2
#define CLIENT_ID3   3
#define CLIENT_ID4   4

#define CLIENT1_ID   1
#define CLIENT2_ID   2
#define CLIENT3_ID   3
#define CLIENT4_ID   4
#define CLIENT5_ID   5
#define CLIENT6_ID   6
#define CLIENT7_ID   7
#define CLIENT8_ID   8

#define SERVER1_ID   1
#define SERVER2_ID   2
#define SERVER3_ID   3
#define SERVER4_ID   4
#define SERVER5_ID   5
#define SERVER6_ID   6
#define SERVER7_ID   7
#define SERVER8_ID   8

#define GETATTR 0
#define READLINK 1
#define MKNOD 2
#define MKDIR 3
#define UNLINK 4
#define RMDIR 5
#define SYMLINK 6
#define RENAME 7
#define LINK 8
#define CHMOd 9
#define CREATE 10
#define OPEN 11
#define READ 12
#define WRITE 13
#define OPENDIR 14
#define READDIR 15

#define READ_DATA   (118)
//extern int server_id = SERVER_ID1;

struct __test_msg {
	int type;
	int len;
	int seqno;
	int client_id;
	int page_num;
	int pad;	
	int res;
	unsigned int command;
	int size;
	int ssize;
	int return_val;
/*	union 
	{	
		char buf[1024];
		struct stat st;
	}msg;*/
	char buf[1024];
	char msg[512];
};
typedef struct __test_msg  test_msg_t;


#endif
  
