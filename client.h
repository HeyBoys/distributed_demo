#include"header.h"
int network_init ();
int  __client_allocpages (void *msgp, amp_u32_t *niov, amp_kiov_t **iov);
void __client_freepages(amp_u32_t niov, amp_kiov_t **iov);
