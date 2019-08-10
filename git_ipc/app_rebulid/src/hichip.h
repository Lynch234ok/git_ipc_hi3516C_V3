
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "httpd.h"

#ifndef HICHIP_H_
#define HICHIP_H_

#define HICHIP_BROADCAST_PORT (18002)
#define HICHIP_MULTICAST_PORT (8002)
#define HICHIP_MULTICAST_NET_SEGMENT "224.0.0.0"
#define HICHIP_MULTICAST_IPADDR "239.255.255.250"

typedef const char * (*fHICHIP_NONCE)(void);
typedef const char * (*fHICHIP_DEVICE_ID)(void);
typedef const char * (*fHICHIP_DEVICE_MODEL)(void);
typedef const char * (*fHICHIP_DEVICE_NAME)(void);

typedef const char * (*fHICHIP_ETHER_LAN)(void); // for nvr
typedef const char * (*fHICHIP_ETHER_VLAN)(void); // for dnvr
typedef int (*fHICHIP_GB28181_CONF)(const void*);
typedef void (*fHICHIP_IP_ADAPT_PAUSE)(int);
typedef int (*fHICHIP_GET_VIDEO_COUNT)(void);
typedef int (*fHICHIP_GET_STREAM_NAME_BY_INDEX)(int, char *);

typedef const char * (*fHICHIP_TRUN_ON_ONVIF)(void);
typedef const char * (*fHICHIP_TRUN_OFF_ONVIF)(void);

typedef struct HICHIP_CONF_FUNC {
	// device info
	fHICHIP_NONCE nonce;
	fHICHIP_DEVICE_ID device_id;
	fHICHIP_DEVICE_MODEL device_model;
	fHICHIP_DEVICE_NAME device_name;
	// network info
	fHICHIP_ETHER_LAN ether_lan;
	fHICHIP_ETHER_VLAN ether_vlan;
	fHICHIP_GB28181_CONF gb28181_conf;
	// pause ip adapt
	fHICHIP_IP_ADAPT_PAUSE ip_adapt_pause;
	fHICHIP_GET_VIDEO_COUNT get_video_count;
	fHICHIP_GET_STREAM_NAME_BY_INDEX get_stream_name_by_index;

	fHICHIP_TRUN_ON_ONVIF turn_on_onvif;
	fHICHIP_TRUN_OFF_ONVIF turn_off_onvif;
}stHICHIP_CONF_FUNC, *lpHICHIP_CONF_FUNC;

typedef struct HICHIP_MAP_STR_DEC {
	const char *str;
	int dec;
}ST_HICHIP_MAP_STR_DEC, *LP_HICHIP_MAP_STR_DEC;


extern int HICHIP_init(stHICHIP_CONF_FUNC conf_func);
extern void HICHIP_destroy();
extern int HICHIP_Lock_init();
extern int HICHIP_Lock_destroy();

extern int HICHIP_http_cgi(HTTPD_SESSION_t* session);
extern void HICHIP_update_interface(const char *eth);
extern int hichip_map_str2dec(const ST_HICHIP_MAP_STR_DEC map[], int map_items, const char *str_key, int def_val);
extern const char *hichip_map_dec2str(const ST_HICHIP_MAP_STR_DEC map[], int map_items, int dec_key, const char *def_val);


#endif //HICHIP_H_

