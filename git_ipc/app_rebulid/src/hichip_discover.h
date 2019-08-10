

#include "hichip.h"
#include "http_util.h"

#ifndef HICHIP_DISCOVER_H_
#define HICHIP_DISCOVER_H_
typedef enum
{
	HICHIP_DISCOVER_TYPE_MULTICAST = 0,
	HICHIP_DISCOVER_TYPE_BROADCAST,
	HICHIP_DISCOVER_TYPE_CNT,
}enHICHIP_DISCOVER_TYPE;

typedef struct
{
	enHICHIP_DISCOVER_TYPE type;
	int sock;
	char peer_ip[16];
	int peer_port;
}stHICHIP_DISCOVER_RESOPNSE, *lpHICHIP_DISCOVER_RESOPNSE;

extern struct sockaddr_in HICHIP_DISCOVER_multicast_addr();
extern int HICHIP_DISCOVER_sock_broadcast_create(char *bindip, int port, int bReuseAddr,int rwTimeoutMS);
extern int HICHIP_DISCOVER_sock_broadcast_send(int sock,char *ip,int port,void *buf,unsigned int size, int flags);
extern int HICHIP_DISCOVER_sock_create(const char *local_ip);
extern void HICHIP_DISCOVER_sock_release(int sock);

extern int HICHIP_DISCOVER_process_search(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content);
extern int HICHIP_DISCOVER_process_cmd(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content);
extern int HICHIP_DISCOVER_process_gb28181(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content);

extern int HICHIP_DISCOVER_Ping(lpHICHIP_CONF_FUNC conf_func);
extern int HICHIP_DISCOVER_Pong(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content);
#endif //HICHIP_DISCOVER_H_

