
#ifndef __BUBBLE_HEAD_FILE__
#define __BUBBLE_HEAD_FILE__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "spook.h"
#include "httpd.h"


typedef enum{
	CMD_PTZ_UP = 0,
	CMD_PTZ_DOWN,
	CMD_PTZ_LEFT,
	CMD_PTZ_RIGHT,
	CMD_PTZ_AUTO,
	CMD_PTZ_FOCUSFAR,
	CMD_PTZ_FOCUSNEAR,
	CMD_PTZ_ZOOMIN,
	CMD_PTZ_ZOOMOUT,
	CMD_PTZ_IRISOPEN,
	CMD_PTZ_IRISCLOSE,
	CMD_PTZ_AUX1,
	CMD_PTZ_AUX2,

	CMD_PTZ_CNT,
}REMOTE_PTZ_CMD;

typedef enum{
	BUBBLE_STREAM_TYPE_H264 = 264,
	BUBBLE_STREAM_TYPE_H265,
}BUBBLE_STREAM_TYPE;


typedef struct BUBBLE_ATTR
{
	BUBBLE_STREAM_TYPE (*GET_VENC_TYPE)(int stream);//stream:1/2/3
	int (*GET_VENC_RESOLUTION)(int stream, int *ret_width, int *ret_height);
}ST_BUBBLE_attr, *LP_BUBBLE_attr;


extern SPOOK_SESSION_PROBE_t BUBBLE_probe(const void *msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t BUBBLE_loop(bool *trigger, int sock, time_t *read_pts);

extern int BUBBLE_over_http_cgi(HTTPD_SESSION_t* http_session);

#include "web_server.h"
extern int BUBBLE_over_http_cgi2(LP_HTTP_CONTEXT context);

extern int BUBBLE_init(LP_BUBBLE_attr arg);
extern void BUBBLE_destroy();

#endif

