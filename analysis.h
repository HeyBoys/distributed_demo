#include "header.h"
/*void do_open(test_msg_t *msgp);
void do_readdir(test_msg_t * msgp);
void do_getattr(test_msg_t *msgp);
void  work(test_msg_t * msgp);*/
void do_open(amp_request_t  * req);
void do_readdir(amp_request_t  *req);
void do_getattr(amp_request_t  *req);
void do_write(amp_request_t  *req);
void do_read(amp_request_t  *req);
void work(amp_request_t  *req);

