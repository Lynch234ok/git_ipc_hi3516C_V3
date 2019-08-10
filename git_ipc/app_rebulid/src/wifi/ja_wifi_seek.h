#ifndef	__JA_WIFI_SEEK_H__
#define	__JA_WIFI_SEEK_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "ja_smart_link.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h> 
//#include <linux/if_packet.h>
#include <sys/types.h>
#include <sys/socket.h>	
#include <sys/ioctl.h>	
#include <sys/reboot.h>	
#include <stdbool.h>
#include <linux/types.h>		/* for "caddr_t" et al		*/
#include <linux/socket.h>		/* for "struct sockaddr" et al	*/
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pthread.h>

#define STRENGTHTOLINK 83
#define RTL_IOCTL_CHECK_APDATER (SIOCIWFIRSTPRIV + 31)
#define RTL_IOCTL_SEARCH_ALL_AP_INFO (SIOCIWFIRSTPRIV + 18)
#define RTW_IOCTL_GET_LINKED_AP_INFO (SIOCSIWMODE+4)

#define  WIFI_DEVIF   "wlan0"   

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;

#define NK_WIFI_MAX_ARP_IP_LEN (16)

	
typedef struct adapter_status{
	//net status
	u8 mode;      //0-STA 1-AP 2-ADHOC 3-ADHOC_MASTER 4-MONITOR .... 
	u8 ifname;    // 0-wlan0 1-wlan1 .....
	u8 removedstatus;
	u8 driver_stop;
	u8 cur_channel;
	u8 sinal_strength;
	u8 signal_qual;
	unsigned int aes_error_count;
	s32	fw_state;	
} adapter_status, *lpadapter_status;


typedef struct stAP_info
{
	int index;
	u8 mac[6];
	int ch;
	int rssi;
	int sinal;
	int noise;
	int age;
	u8 ssid[32];
	u8 save ;

}stAP_info,*lpstAP_info;

typedef struct ieee_param_ex_searchAP_info {
	
	stAP_info data[50];
}ieee_param_ex_searchAP_info;

typedef struct linked_ap_info{
	u8 sinal_strength;
	u8 linkstatus;//0-notlink 1-linked
}linked_ap_info,*lplinked_ap_info;


typedef struct ieee_param_ex_linked_apinfo {
	linked_ap_info data[2];
}ieee_param_ex_linked_apinfo;

typedef struct ieee_param_ex_adpdater {
	adapter_status data[2];
}ieee_param_ex_adpdater;

typedef void *(*fWIFI_MODE_do)(u8* essid,u8* password,u8 beforemode);
typedef void *(*fWIFI_SYSTEM_REBOOT_do)(void);
typedef int (*fWIFI_SYSTEM_CHECK_SMARTLINK_do)(void);

int check_adpater_status(adapter_status *stadpdater_status,char *ifname);
int get_linked_AP_info(struct linked_ap_info * link_info,char *ifname);
int search_ap_info(u8* macaddr);
int search_ap_info_2(u8* macaddr, u8* essid,char *ifname);
int nk_wifi_assoc_under_APmode(u8* essid , u8* password,char *ifname1,char *ifname2);
int nk_wifi_assoc_under_STAmode(u8* essid, u8* password,char *ifname);
int nk_wifi_assocAP(fWIFI_MODE_do wifiswitch,fWIFI_MODE_SWITCH do_trap,char *ifname1,char *ifname2);
int nk_wifi_check_status_and_assoc_AP(fWIFI_MODE_SWITCH do_trap,char *ifname1,char *ifname2,char *lastessid);
void nk_wifi_stop_wireless_macth_code();
void reset_the_time_of_smartlink();
	
/*This function is used to start the assoc thread ,lastessid is the essid that the IPC connect last time.
  It is used in the smartlink. _wifi_switch is an function pointer that used to connect the NVR at AP mode.
  And the _do_trap is used to connect NVR at STA mode.	
*/
int NK_WIFI_start_assoc_thread(char *lastessid,fWIFI_MODE_SWITCH _do_trap,char *ifname1,char *ifname2);
int NK_WIFI_stop_assoc_thread(char *ifname);

int NK_WIFI_adapter_monitor_thread_start(fWIFI_SYSTEM_REBOOT_do _do_reboot, fWIFI_SYSTEM_CHECK_SMARTLINK_do _do_check_smartlink_status);
int NK_WIFI_adapter_monitor_thread_stop(void);

int NK_WIFI_search_ap(lpstAP_info apinfo_list,char *ifname, u32 limitstrength);
void NK_WIFI_set_wifi_disappear_flag();
int NK_WIFI_get_linked_AP_info(struct linked_ap_info * link_info,char *ifname);

int NK_WIFI_arp_pick_ip(char *iface, char ip_ret[][NK_WIFI_MAX_ARP_IP_LEN], int max_num, unsigned int *ipformat);
int NK_WiFI_set_adapter_minitor_pause_flag(bool flag);

#ifdef __cplusplus
}
#endif
#endif
