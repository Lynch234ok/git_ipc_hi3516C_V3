#include <stdlib.h>
#include <stdarg.h>
#include "app_debug.h"
#include <generic.h>
#include <assert.h>
#include "wpa_ctrl.h"
#include "wpa_status.h"
#include "wifi/ja_wifi_seek.h"
#include "app_wifi.h"
#include "sound.h"
#include "ipcam_timer.h"
#include "ipcam_network.h"
#include "base/ja_process.h"
#include "bsp/keytime.h"
#include <sys/prctl.h>

#define WIRED_IF_NAME			"eth0" ///< 有线网卡名称。
#define WIFI_IF_NAME			"wlan0" ///< 无线网卡名称。
#define WIFI_CTRL_INTF			"/tmp/wpa_supplicant"
//#define WIFI_CONF_DIR			"/etc/jffs2/" ///< 配置文件目录。
//#define WPA_CONF_FILE			"/etc/jffs2/wpa_supplicant.conf" ///< WPA 认证配置文件。

static ST_WPA_CONNECT_STATUS wpa_attr;
#define MATCH_EVENT(__evt, __type) (0 == strncmp((__evt), (__type), strlen(__evt)) ? NK_True : NK_False)
#define FIXED_WIFI_IP()	do{ system("ifconfig wlan0 172.14.10.1 netmask 255.255.255.0 up"); } while(0)

/* wifi连接成功标志文件 */
#define WIFI_CONNECT_FLAG    "/media/conf/wifi_connect_flag"

/* 
	创建WIFI_CONNECT_FLAG文件，标记设备初次配置路由已连接成功
	后面除升级、恢复出厂设置，不会有提示音。恢复出厂设置会删除标记文件
*/
static NK_Void setWifiConnectedFlag()
{
	char cmd[64];

	snprintf(cmd, sizeof(cmd), "echo success > %s", WIFI_CONNECT_FLAG);
	NK_SYSTEM(cmd);

}

static bool getWifiConnectedFlag()
{
	if(IS_FILE_EXIST(WIFI_CONNECT_FLAG)) {
		return true;
	}
	else {
		return false;
	}

}

static NK_Void resetWifiConnectedFlag()
{
	REMOVE_FILE(WIFI_CONNECT_FLAG);

}

static NK_Void reset_network()
{
	wpa_attr.timeout = 0;
	IPCAM_network_switch_to_ap();
	wpa_attr.terminated = true;

}

/*
 * 处理无线连接事件。
 */
static bool handle_wifi_event(NK_PChar text)
{
	
	NK_PChar save_ptr = NK_Nil;
	NK_PChar evt = NK_Nil;
	NK_PChar evt_prefix = "CTRL-EVENT-";
	NK_Char cmd[128];
	bool wifi_status = false;
#if defined(WIFI)
	if ('<' == text[0]) {
		/// 跳过控制台输出等级显示。
		text += 3;
	}

	/// 复制字符串避免误操作。
	evt = strdupa(text);
	evt = strtok_r(evt, " ", &save_ptr);
	//APP_TRACE("RTL8188: Event %s",evt);
	if (MATCH_EVENT(evt_prefix, evt)) {
		/// 控制事件分类。
		if (MATCH_EVENT(evt, WPA_EVENT_SCAN_RESULTS)) {
			/// 扫描结果。
		//	APP_TRACE("RTL8188: Wi-Fi Event %s.", evt);
		} else if (APP_WIFI_check_sta_status("wlan0") != 1) {
			if(MATCH_EVENT(evt, WPA_EVENT_BSS_ADDED)
				|| MATCH_EVENT(evt, WPA_EVENT_BSS_REMOVED)){
				/// 加入移除 BSS。
				//int id = atoi(strtok_r(NK_Nil, " ", &save_ptr));
				//NK_PChar bss = strtok_r(NK_Nil, " ", &save_ptr);	
				//	APP_TRACE("RTL8188: Wi-Fi Event %s %d %s.", evt, id, bss);
			}
		} else if (MATCH_EVENT(evt, WPA_EVENT_CONNECTED) || (APP_WIFI_check_sta_status("wlan0") == 1)) {
			/// 连接成功。
			struct linked_ap_info link_info;
			NK_WIFI_get_linked_AP_info(&link_info, "wlan0");
			if((link_info.linkstatus == 1) && (link_info.sinal_strength >= 10)) {
				//APP_TRACE("RTL8188: Wi-Fi Event %s.", text);
				if(getWifiConnectedFlag() == false) {
	            	SearchFileAndPlay(SOUND_WiFi_connection_completed, NK_True);
					setWifiConnectedFlag();
	            }
#ifdef LED_CTRL
				initLedContrl(DEF_LED_ID,true,LED_LIGHT_MODE);
#endif
	        	wifi_status = true;
			}
		} else if (MATCH_EVENT(evt, WPA_EVENT_DISCONNECTED)) {
			/// 加入移除 BSS。
			APP_TRACE("RTL8188: Wi-Fi Event %s.", text);
		} else if (MATCH_EVENT(evt, WPA_EVENT_TEMP_DISABLED)) {
			/// 加入移除 BSS。
			APP_TRACE("RTL8188: Wi-Fi Event %s.", text);
			/// FIXME 粗略判断一下是否密码错误。
			if (strstr(text, "reason=WRONG_KEY")){
				if(getWifiConnectedFlag() == false) {
					SearchFileAndPlay(SOUND_Password_error, NK_True);	
					reset_network();
				}
#ifdef LED_CTRL
				initLedContrl(DEF_LED_ID,true,LED_MIN_MODE);
#endif
				APP_TRACE("RTL8188: Wireless password error %s.", text);
				wifi_status = true;
			}
		} else {
			APP_TRACE(text);
		}
	}else{
		APP_TRACE("RTL8188: Ignore Notice \"%s\".", text);
		/// FIXME 粗略判断一下是否密码错误。
		if (strstr(text, "incorrect")){
			if(getWifiConnectedFlag() == false) {
				SearchFileAndPlay(SOUND_Password_error, NK_True);	
				reset_network();
			}
#ifdef LED_CTRL
			initLedContrl(DEF_LED_ID,true,LED_MIN_MODE);
#endif
			APP_TRACE("RTL8188: Wireless password error %s.", text);
			wifi_status = true;
		}
	}
#endif
	return wifi_status;
}

/**
 * 监听无线连接事件无线通知。
 */
static NK_PVoid
wifi_event_listener(void *arg)
{
	struct wpa_ctrl *Monitor = NK_Nil;
	NK_Char ctrl_intf[128];
	/// 控制管道路径。
	bool wifi_event = false;
	int ret = 0;
	ST_NSDK_NETWORK_INTERFACE wlan;
    prctl(PR_SET_NAME, "wpa_listener");

#if defined(WIFI)
	snprintf(ctrl_intf, sizeof(ctrl_intf), "%s/%s", WIFI_CTRL_INTF, WIFI_IF_NAME);
 	while (!wpa_attr.terminated) {
		/// wpa_supplicant

		NETSDK_conf_interface_get(4, &wlan);
		if(APP_WIFI_model_exist() && wlan.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE) {

		/// 不断地循环等待连接直到成功打开为止。
			do
			{
				/// 打开无线控制句柄。				
				Monitor = wpa_ctrl_open(ctrl_intf);

				if (!Monitor) {
					sleep(1);
				}

			} while (!wpa_attr.terminated && !Monitor);


			/// 连接到无线控制接口。
			if (0 == wpa_ctrl_attach(Monitor)) {	
			while(!wpa_attr.terminated){	
				if (1== wpa_ctrl_pending(Monitor)){
					NK_Char recv_buff[1024];
					NK_Size n_recv = sizeof(recv_buff) - 1;

					if (0 == wpa_ctrl_recv(Monitor, recv_buff, &n_recv) && n_recv > 0)
					{
						/// 防止字符串泄漏。
						recv_buff[n_recv] = '\0';
						/// 处理事件。
						wifi_event = handle_wifi_event(recv_buff);
						if(false == wifi_event){
							//if(false == wifi_event ){
							//超时大约是30s
							if(wpa_attr.timeout++ > 60) {
								if(getWifiConnectedFlag() == false) {
									SearchFileAndPlay(SOUND_WiFi_connection_failed, NK_True);
									reset_network();
								}
							}
							//APP_TRACE("------------timeout:%d---------",wpa_attr.timeout);
						}

						
					} else {
						break;
					}
				}	
				usleep(500000);
			}

			/// 断开控制器。
			wpa_ctrl_detach(Monitor);
			} 			
			wpa_ctrl_close(Monitor);
			//用于模式自动切换后重连
			//if (wpa_attr.thread_tid) {
			//	wpa_attr.terminated = false;
			//}
			/// 断开连接。
			/// kill -all
		}
		sleep(1);
	}
#endif
	pthread_exit(NULL);
}


static NK_Void stop_connect()
{
	if (wpa_attr.thread_tid) {
		wpa_attr.terminated = true;
		pthread_join(wpa_attr.thread_tid, NULL);
		wpa_attr.thread_tid = (pthread_t)NULL;
	}

}



static NK_Void start_connect()
{	
    int nRet;

    pthread_attr_t pthread_attr;
    nRet = pthread_attr_init(&pthread_attr);

    if(0 == nRet)
    {
        pthread_attr_setstacksize(&pthread_attr, 131072);
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);

        while(0 != pthread_create(&wpa_attr.thread_tid, &pthread_attr, wifi_event_listener,NULL))
        {
            APP_TRACE("[%s:%d]creat wifi_wpa thread failed",__func__,__LINE__);
            sleep(1);
        }
        pthread_attr_destroy(&pthread_attr);
    }
    else
    {
        while(0 != pthread_create(&wpa_attr.thread_tid, NULL, wifi_event_listener,NULL))
        {
            APP_TRACE("[%s:%d]creat wifi_wpa thread failed",__func__,__LINE__);
            sleep(1);
        }
    }

}

bool WPA_getWifiConnectedFlag()
{
	return getWifiConnectedFlag();

}

NK_Void WPA_resetWifiConnectedFlag()
{
	resetWifiConnectedFlag();

}

NK_Int32 wifi_wpa_init()
{
	wpa_attr.thread_tid = (pthread_t)NULL;
	wpa_attr.terminated = false;
	wpa_attr.timeout = 0;
	start_connect();
	return 0;
}

NK_Void WPA_stop_connect()
{
	stop_connect();
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);

}


