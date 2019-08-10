#include "nk_ip_adapt.h"

#define MAX_NET_ADAPT_PEROID (60)

typedef struct
{
    time_t flag;
    int peroid;
    int is_ip_conflict;
    fNK_NET_ADAPT_ENABLE enable_hook;
}stNK_NET_ADAPT_CTX, *lpNK_NET_ADAPT_CTX;

stNK_NET_ADAPT_CTX m_net_adapt_args = {0, MAX_NET_ADAPT_PEROID, 0, NULL};

int nk_net_adapt_ip_set_conflict_flag(int conflict)
{
    lpNK_NET_ADAPT_CTX thiz = &m_net_adapt_args;
    thiz->is_ip_conflict = conflict;
    return 0;
}

int nk_net_adapt_ip_set_pause_flag(time_t flag, int peroid)
{
    lpNK_NET_ADAPT_CTX thiz = &m_net_adapt_args;
    thiz->flag = flag;
    thiz->peroid = peroid;
    return 0;
}

int nk_net_adapt_ip_is_enable()
{
    lpNK_NET_ADAPT_CTX thiz = &m_net_adapt_args;
    if(time(NULL) - thiz->flag <= thiz->peroid){
        return 0;
    }
    if(!thiz->enable_hook){
        return 0;
    }
    else{
        return thiz->enable_hook();
    }
    return 0;
}

int nk_net_adapt_ip_should_adapt(char *shelf_ip, char* client_ip)
{
    lpNK_NET_ADAPT_CTX thiz = &m_net_adapt_args;
    unsigned int shelf, client;
	if((NULL == shelf_ip) || (NULL == client_ip)){
		return -1;
	}	
	//do IP adapted
	shelf = inet_addr(shelf_ip);
	client= inet_addr(client_ip);

	if(thiz->is_ip_conflict || (shelf & 0xffffff) != (client & 0xffffff)){
		printf("shelf_ip : %s, client_ip : %s conflict %d \r\n", shelf_ip, client_ip, thiz->is_ip_conflict);
		return 1;
	}

    return 0;
}

int nk_net_adapt_ip_set_enable_hook(fNK_NET_ADAPT_ENABLE enable_hook)
{
    lpNK_NET_ADAPT_CTX thiz = &m_net_adapt_args;
    thiz->enable_hook = enable_hook;
    return 0;
}
