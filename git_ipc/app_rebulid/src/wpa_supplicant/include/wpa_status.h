#ifndef WPA_TRACE_H
#define WPA_TRACE_H

#include <NkUtils/types.h>

typedef struct wpa_connect_status
{

	pthread_t thread_tid;
	bool terminated;
	int timeout;
}ST_WPA_CONNECT_STATUS,*LP_WPA_CONNECT_STATUS;

/* 获取设备初次配置路由已成功连接标记，主要是判断标记文件WIFI_CONNECT_FLAG */
/* 标记文件存在，则不播放提示音 */
extern bool WPA_getWifiConnectedFlag();

/* 
删除WIFI_CONNECT_FLAG标记文件，
主要用户设备重置是调用 和升级时需要播放提示音使用
*/
extern NK_Void WPA_resetWifiConnectedFlag();

extern NK_Int32 wifi_wpa_init();

extern NK_Void WPA_stop_connect();

#endif


