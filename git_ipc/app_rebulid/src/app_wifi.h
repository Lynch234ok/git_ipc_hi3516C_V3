
#ifndef APP_WIFI_H_
#define APP_WIFI_H_

#include <NkUtils/n1_def.h>
#include "netsdk.h"
#include "wifi/ja_wifi_seek.h"

typedef struct NK_WIFI_HOTSPOT
{
	/**
	 * 无线热点的 BSSID。
	 */
	char bssid[32];
	/**
	 * 无线热点通信信道，0 表示自动。
	 */
	int channel;
	/**
	 * 无线热点信号强度。
	 */
	int dBm, sdBm;
	/**
	 * 无线热点生存期。
	 */
	int age;

	/**
	 * 无线热点的 ESSID。
	 */
	char essid[128];

	/**
	 * 无线热点的 PSK。
	 */
	char psk[32];

    /*
    * 无线热点的加密方式。
    */
    char encrypt[64];
}stNK_WIFI_HotSpot, *lpNK_WIFI_HotSpot;

//Intelligent adjustment resolution.
extern int APP_WIFI_calculate_bps(ST_NSDK_VENC_CH* pvenc_ch);

extern int APP_WIFI_model_remove();

extern bool APP_WIFI_model_exist();

extern int APP_WIFI_get_rssi();

extern int wifi_get_adapter_state();

extern int APP_WIFI_check_ifstatus(char *nic_name);

//extern int APP_WIFI_is_sta_connected();

extern int SMART_link_init(char *ether);

extern int SMART_link_deinit(char *ether);

extern int APP_WIFI_check_sta_status(char *nic_name);

extern bool APP_WIFI_If_Exist(char *ether);

//extern int APP_WIFI_get_rssi(char *ether);
extern int APP_WIFI_get_rssi();

extern int APP_WIFI_Wifi_Scan(char *ether, stNK_WIFI_HotSpot *essid_list, int max_num);

extern int APP_WIFI_get_Connected_MAC(char *ether, int num, unsigned char mac[][6]);

extern int APP_WIFI_get_Rate(char *ether, unsigned char mac_addr[6]);

extern int APP_Wifi_Concurrent_RestartAp(char *sta_essid, char *sta_psk, char *ap_essid, char *ap_psk, char *ap_ether);

extern int  APP_WIFI_Wifi_Exit();

extern int APP_Wifi_Concurrent_Set_Bridge(char *ether1, char *ether2, char *br_ip, char *netmask);

extern int APP_Wifi_Concurrent_Set_Bridge_Off(char *ether1, char *ether2);

extern int APP_WIFI_exit_wifi();

extern fWIFI_SYSTEM_REBOOT_do APP_WIFI_self_reboot();

extern fWIFI_SYSTEM_CHECK_SMARTLINK_do APP_WIFI_check_smartlink_status();

extern void APP_WIFI_search_ap();

extern int APP_WIFI_smartlink_is_running();

extern void APP_WIFI_get_near_ap(lpNK_WIFI_HotSpot lpAPs, unsigned int *nAPs);

#endif //APP_WIFI_H_
