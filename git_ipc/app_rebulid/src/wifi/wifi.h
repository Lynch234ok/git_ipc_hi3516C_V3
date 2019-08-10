
#ifndef __WIFI_H__
#define __WIFI_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <regex.h>
	

#include "sta.h"
#include "ap.h"
#include "ja_smart_link.h"

struct sta_struct
{	
	char essid[100];
	char key[100];
	char staticIp[32];
	char staticNetmask[32];
	bool dhcpEnabled;
};

//char mode[10];

typedef struct ieee_param_ex {
	unsigned int cmd;
	unsigned char sta_addr[6];
	unsigned char data[0];
}ieee_param_ex;

struct rtw_ieee80211_ht_cap {
	unsigned short 	cap_info;
	unsigned char 	ampdu_params_info;
	unsigned char 	supp_mcs_set[16];
	unsigned short 	extended_ht_cap_info;
	unsigned int		tx_BF_cap_info;
	unsigned char	       antenna_selection_info;
};

struct ja_ipcam_sta{
	int UndecoratedSmoothedPWDB;
	int UndecoratedSmoothedCCK;
	int UndecoratedSmoothedOFDM;
	unsigned long long PacketMap;
	unsigned char ValidBit;
};

typedef struct ieee_param {
	unsigned int cmd;
	unsigned char sta_addr[6];
	union {
		struct {
			unsigned char name;
			unsigned int value;
		} wpa_param;
		struct {
			unsigned int len;
			unsigned char reserved[32];
			unsigned char data[0];
		} wpa_ie;
	        struct{
			int command;
    			int reason_code;
		} mlme;
		struct {
			unsigned char alg[16];
			unsigned char set_tx;
			unsigned int err;
			unsigned char idx;
			unsigned char seq[8]; /* sequence counter (set: RX, get: TX) */
			unsigned short key_len;
			unsigned char key[0];
		} crypt;
		struct {
			unsigned short aid;
			unsigned short capability;
			int flags;
			unsigned char tx_supp_rates[16];		
			struct rtw_ieee80211_ht_cap ht_cap;
		} add_sta;
		struct {
			unsigned char	reserved[2];//for set max_num_sta
			unsigned char	buf[0];
		} bcn_ie;
	} u;	   
}ieee_param;

int JN_Wifi_GetBssid(char bssid[48]);
int JN_Wifi_STA_Search(struct wifi_struct tswifi[100]);
int JN_Wifi_STA_Getstatus();
int JN_Wifi_Start(char *pMode, struct ap_struct *tsap, struct sta_struct *tssta);
int JN_Wifi_Stop(char *pMode);
int JN_Wifi_AP_Setparam(struct ap_struct *tsap, char *essid, char *wpa_passphrase, int hw_mode, int wpa_key_mgmt, int channel, char *ipStart, char *ipNumber, char *dns, char *subnet, char *router);
int JN_Wifi_STA_Setparam(struct sta_struct *tssta, char *essid, char *key);
int JN_Wifi_AP_Init(char *wlanIp, char *wlanNetmask, char *wlanGateway, char *wlanDns);   //init AP
int JN_Wifi_STA_Init();   //init STA
int JN_Wifi_Exit(void);
int JN_Wifi_AP_Setup(char *essid, char *wpa_passphrase, int hw_mode, int wpa_key_mgmt, int channel, char *ipStart, char *ipNumber, char *dns, char *subnet, char *router);
int JN_Wifi_STA_Setup(char *essid, char *key, char *ip, char *netmask, bool dhcpEnabled);
int JN_Wifi_AP_Get_Connected_MAC(char *ifcae, int num, unsigned char mac[][6], char *wifi_type);
int JN_Wifi_USB1_GetSignal(char *ifcae, unsigned char mac_addr[6]);

int JN_Wifi_Concurrent_Set_Bridge_Off(char *ether1, char *ether2);
int JN_Wifi_Concurrent_Set_Bridge(char *ether1, char *ether2, char *br_ip, char *netmask);
#ifdef __cplusplus
};
#endif
#endif //__WIFI_H__

