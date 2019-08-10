#ifndef WPA_TRACE_H
#define WPA_TRACE_H

#include <NkUtils/types.h>

typedef struct wpa_connect_status
{

	pthread_t thread_tid;
	bool terminated;
	int timeout;
}ST_WPA_CONNECT_STATUS,*LP_WPA_CONNECT_STATUS;

/* ��ȡ�豸��������·���ѳɹ����ӱ�ǣ���Ҫ���жϱ���ļ�WIFI_CONNECT_FLAG */
/* ����ļ����ڣ��򲻲�����ʾ�� */
extern bool WPA_getWifiConnectedFlag();

/* 
ɾ��WIFI_CONNECT_FLAG����ļ���
��Ҫ�û��豸�����ǵ��� ������ʱ��Ҫ������ʾ��ʹ��
*/
extern NK_Void WPA_resetWifiConnectedFlag();

extern NK_Int32 wifi_wpa_init();

extern NK_Void WPA_stop_connect();

#endif


