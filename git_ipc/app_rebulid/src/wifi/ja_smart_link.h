#ifndef	__JA_SMART_LINK_H__
#define	__JA_SMART_LINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>   
#include <string.h>   
#include <errno.h>   
#include <stdlib.h>   
#include <unistd.h>   
#include <sys/types.h>   
#include <sys/socket.h>   
#include <netinet/in.h>   
#include <arpa/inet.h>   
#include <fcntl.h>
#include <syslog.h>  
#include <stdbool.h>
#include <sys/times.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/time.h> 
#include <math.h> 
#include <linux/types.h>		/* for "caddr_t" et al		*/
#include <linux/socket.h>		/* for "struct sockaddr" et al	*/
#include <pthread.h>


#define SIOCSIWFREQ	0x8B04	
#define SIOCSIWMODE	0x8B06	
#define SIOCGIWRANGE 0x8B0B		
#define SIOCSIWSCAN	0x8B18		
#define MAX_INTERFACE   (16) 
#define GET_DATA_PACKET_TIME 100
#define SMART_LINK_TREAD_TRUE 1
#define SMART_LINK_TREAD_FALSE 0
#define SMART_LINK_MESSAGE_LENGTH 50
#define SMART_LINK__INTERRUPT_TIME 100

//#define DEBGU
#ifdef DEBGU
#define DBG printf
#else 
#define DBG(x,...) do{}while(0)
#endif


/* 下面四个结构体为设置通道和切换无线网卡模式所使用 */
typedef struct	
{
  __s32		value;		/* The value of the parameter itself */
  __u8		fixed;		/* Hardware should not use auto select */
  __u8		disabled;	/* Disable the feature */
  __u16		flags;		/* Various specifc flags (if any) */
}iw_param;

typedef struct	
{
  caddr_t	pointer;	/* Pointer to the data  (in user space) */
  __u16		length;		/* number of fields or size in bytes */
  __u16		flags;		/* Optional params */
}iw_point;

typedef struct	
{
	__u32		m;		/* Mantissa */
	__u16		e;		/* Exponent */
	__u8		i;		/* List index (when in range struct) */
	__u8		flags;		/* Flags (fixed/auto) */
}iw_freq;

typedef struct	 
{
	union
	{
		char	ifrn_name[IFNAMSIZ];
	} ifr_ifrn;
	union
	{
		char		name[IFNAMSIZ];

		 iw_point	essid;		/* Extended network name */
		 iw_param	nwid;		/* network id (or domain - the cell) */
		 iw_freq	freq;		/* frequency or channel : * 0-1000 = channel * > 1000 = frequency in Hz */
		 iw_param	sens;		/* signal level threshold */
		 iw_param	bitrate;	/* default bit rate */
		 iw_param	txpower;	/* default transmit power */
		 iw_param	rts;		/* RTS threshold threshold */
		 iw_param	frag;		/* Fragmentation threshold */
		__u32		mode;		/* Operation mode */
		 iw_point	encoding;	/* Encoding stuff : tokens */
		 iw_param	power;		/* PM duration/timeout */
		struct sockaddr	ap_addr;	/* Access point address */
		 iw_point	data;		/* Other large parameters */
	}	u;
}iwreq;

/* 存放根据抓取beacon包得到的ap信息 */
typedef struct
{
	char essid[100][30];
	char flags[100];
}st_all_ap_essid,*pst_all_ap_essid;

typedef enum{
	SMARTLINK_WIFI_MODE_NONE,
	SMARTLINK_WIFI_MODE_AP, 
	SMARTLINK_WIFI_MODE_STATION, 
	SMARTLINK_WIFI_MODE_MONITOR,
	SMARTLINK_WIFI_MODE_REPEATER,
	SMARTLINK_WIFI_MODE_COUNT,
}emWIFI_MODE;
	
typedef enum{
	SMARTLINK_STATUS_SEARCH_CHANNEL,
	SMARTLINK_STATUS_START_TO_RECEIVE_PACKEET,
	SMARTLINK_STATUS_RECEIVE_NO_PACKET,
	SMARTLINK_STATUS_RECEIVE_PACKET_TIMEOUT,
	SMARTLINK_STATUS_SWITCH_TO_MONITOR,
	SMARTLINK_STATUS_SUCCESS,
	SMARTLINK_STATUS_SWITCH_TO_PD_TEST,
	SMARTLINK_STATUS_COUNT,
}emWIFI_SMARTLINK_STATUS;

typedef struct wifiAttr
{
	char essid[64];
	char password[128];
	char token[32];
	emWIFI_MODE mode;
	bool dhcp;
	emWIFI_SMARTLINK_STATUS status;
}stWifiAttr, *lpWifiAttr;

typedef struct NVR_match_essid{
	char essid[35];
	int rssi;
	char mac[6];
}stNK_Match_Essid, *lpNK_Match_Essid;

typedef struct{
	char bssid[100][6];
	int bssid_channel[100];
	int existing_channel[16];
	int channel_max_rssi[16];
	int existing_channel_number;
}st_wireless_envirement, *pst_wireless_envirement;


typedef void *(*fWIFI_MODE_SWITCH)(lpWifiAttr wifi);
typedef void (*smartl_link_print)(const char *fmt,...);
typedef int (*fWIFI_GET_WIFI_MODE)(void);
typedef void (*fWIFI_SET_WIFI_MODE)(int wifi_mode);

/* interface is the name of  wireless device that you want to receive packet */
int smart_link_rtl8188_init( char *wlan_interface,  fWIFI_MODE_SWITCH do_trap, char *ESSID, emWIFI_MODE mode, bool immediately, smartl_link_print sl_print,
							fWIFI_GET_WIFI_MODE get_wifi_mode_callback, fWIFI_SET_WIFI_MODE set_wifi_mode_callback);


/* close the socket that open in smart_link_rtl8188_init function */
int smart_link_rtl8188_quit( char *wlan_interface );

/* the function of receive packet 
 * ap_message is the area that you want to store the message from host
 * smart_config_time is time to receive the head of packet that from host, in second
 * paae use to store the essid of all the ap that wireless device search  
 * attention : this function only stay on one channel to receive the packet
 */
//int smart_link_rtl8188 ( char *ap_message, int smart_config_time, pst_all_ap_essid paae);
int smart_link_rtl8188 ( int smart_config_time );


/* 
 * as same as the smart_link_rtl8188_init() function, but this function can search all the  2.4G wireless signal
 */
//int smart_link_rtl8188_all_channel( char *ap_message,int smart_config_time,pst_all_ap_essid paae);
int smart_link_rtl8188_all_channel( int smart_config_time );

int set_channel ( char *ifname, double channel );
int set_wlan_mode ( char *ifname, unsigned int mode );
int set_wlan_mode_monitor ( char *ifname );
int set_wlan_mode_managed ( char *ifname );
int set_wlan_mode_ap(char *ifname);
int smart_link_status();
int nk_smart_link_if_has_product_AP();
#ifdef __cplusplus
}
#endif
#endif	//__JA_SMART_LINK_H__


