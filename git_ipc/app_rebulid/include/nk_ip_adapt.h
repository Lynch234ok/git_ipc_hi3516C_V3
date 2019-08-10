#ifndef NK_IP_ADAPT_H
#define NK_IP_ADAPT_H

#include <stdio.h>
#include <time.h> 
#include <arpa/inet.h>

typedef int (*fNK_NET_ADAPT_ENABLE)();

extern int nk_net_adapt_ip_set_conflict_flag(int conflict);
extern int nk_net_adapt_ip_set_pause_flag(time_t flag, int peroid);
extern int nk_net_adapt_ip_is_enable();
extern int nk_net_adapt_ip_should_adapt(char *shelf_ip, char* client_ip);
extern int nk_net_adapt_ip_set_enable_hook(fNK_NET_ADAPT_ENABLE enable_hook);

#endif
