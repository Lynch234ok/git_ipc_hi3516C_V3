#include "ja_wifi_seek.h"
#include <stdarg.h>
#include <sys/prctl.h>
#include "ifconf.h"
#include "base/ja_process.h"
//#define DEBUG_LOG_SAVE

#ifndef SIOCIWFIRSTPRIV
#define SIOCIWFIRSTPRIV  0x8BE0
#endif
struct rtw_ieee80211_ht_cap {
	unsigned short 	cap_info;
	unsigned char 	ampdu_params_info;
	unsigned char 	supp_mcs_set[16];
	unsigned short 	extended_ht_cap_info;
	unsigned int		tx_BF_cap_info;
	unsigned char	       antenna_selection_info;
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
int is_running_smartlink = 0;
static bool m_assoc_thread = 0;
static pthread_t m_assoc_thread_id = ( pthread_t )NULL;
static char last_connect_essid[64];
static fWIFI_MODE_do wifiswitch = NULL;
static fWIFI_MODE_SWITCH do_trap = NULL;
fWIFI_SYSTEM_REBOOT_do do_reboot = NULL;
fWIFI_SYSTEM_CHECK_SMARTLINK_do do_check_smartlink_status = NULL;
static char mifname1[6];
static char mifname2[6];
#define MAX_AP_NUM 50
#define LIMIT_STRENGTH 40
#define SMART_LINK_RUN_TIME 10 //min
static stAP_info AP_info[MAX_AP_NUM];
struct wifi_seek_attr{
	int start_time;
	int end_time;
	int last_end_time;
	int seek_count;
};
static struct wifi_seek_attr _wifi_seek_attr = {0,0,0,0};
int g_IPC_is_repeater = 0;
int g_init_mode ;

typedef struct Adapter_moinitor_Arg{
	bool adapter_thread;
	pthread_t adapter_thread_id;
	bool pause_flg;
}stAdapter_monitor_Arg, *lpAdapter_monitor_Arg;

static stAdapter_monitor_Arg m_ApapterMonitorArg = {0, ( pthread_t )NULL, 0};

extern int __scan_wireless_signal( char *__interface, int timeout);
static int m_wifi_is_disappear;

void print_log2(const char *fmt,...)
{
	va_list   arg;
	char logString[512]={0};
	char logstring[512]={0};
	
	va_start(arg,fmt);
	vsprintf(logString, fmt, arg);
	va_end(arg);
	sprintf(logstring, "%s%s","[smartlink_log]:", logString); 
	printf("[smartlink_log]:%s", logString);
}

int check_adpater_status(adapter_status *stadpdater_status,char *ifname)
{
	int err = 0;	
	int skfd;
	iwreq iwr;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if( skfd < 0 ) {
		print_log2("open socket error socket: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
		return -1;
	}
	
	memset(&iwr, 0, sizeof(iwreq));
	strncpy(iwr.ifr_ifrn.ifrn_name, ifname, strlen(ifname));
	iwr.u.data.length = (unsigned short)sizeof(adapter_status);
	iwr.u.data.pointer = ( caddr_t )stadpdater_status;	
	
	err = ioctl(skfd, RTL_IOCTL_CHECK_APDATER, &iwr);
	if(err<0){	
		print_log2("Ioctl error: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
		close(skfd);
		return 0xE4;
	}
	close(skfd);
	
	return 1;
}

int get_linked_AP_info(struct linked_ap_info * link_info,char *ifname)
{
	int err = -1;
	iwreq iwr;
	int skfd;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd<0){
		print_log2("open socket error socket: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
		return -1;
	}

	memset(&iwr, 0, sizeof(iwreq));
	strncpy(iwr.ifr_ifrn.ifrn_name, ifname, strlen(ifname));
	iwr.u.data.length = (unsigned short)sizeof(struct linked_ap_info);
	iwr.u.data.pointer = ( caddr_t )link_info;	

	err = ioctl(skfd,RTW_IOCTL_GET_LINKED_AP_INFO, &iwr);
	if(err<0){	
		print_log2("Ioctl error: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
		close(skfd);
		return -1;
	}
	close(skfd);
	
	return err;
}

int NK_WIFI_get_linked_AP_info(struct linked_ap_info * link_info,char *ifname)
{
	return get_linked_AP_info(link_info,ifname);
}

int NK_WIFI_search_ap(lpstAP_info apinfo_list,char *ifname, u32 limitstrength)
{
	int ap_num = 0;
	int i ;
	iwreq iwr;

	int skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd<0){
		print_log2("open socket error socket: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
		return -1;
	}
	__scan_wireless_signal(ifname ,2);
	memset(&iwr, 0, sizeof(iwreq));
	strncpy(iwr.ifr_ifrn.ifrn_name, ifname, strlen(ifname));
	iwr.u.data.length = sizeof(stAP_info)*MAX_AP_NUM;
	iwr.u.data.flags = limitstrength;
	iwr.u.data.pointer = ( caddr_t )apinfo_list;	

	if(ioctl(skfd,RTL_IOCTL_SEARCH_ALL_AP_INFO, &iwr) < 0){	
		print_log2("Ioctl error: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
		close(skfd);
		return -1;
	}
	close(skfd);

	for(i=0; i < MAX_AP_NUM; i++){
		if(apinfo_list[i].save){
			ap_num ++;
		}
	}
	
	return ap_num;
}

int get_the_enougth_strlength_AP(u8* essid , u8* macaddr)
{
	int i = 0;
	int j = 0;
	int ret = 0;
	int count = 0;
	
	printf("%5s  %-17s  %3s  %-3s  %-4s  %-4s  %5s   %32s\n", "index", "bssid", "ch", "RSSI", "SdBm", "Noise", "age",  "ssid");
	while(AP_info[i].save == 1 && i < MAX_AP_NUM){
		printf("%5d   %02x:%02x:%02x:%02x:%02x:%02x %3d  %3d  %4d  %4d    %5d    %32s\n",AP_info[i].index,
		AP_info[i].mac[0],AP_info[i].mac[1], AP_info[i].mac[2],
		AP_info[i].mac[3],AP_info[i].mac[4], AP_info[i].mac[5], AP_info[i].ch,AP_info[i].rssi, AP_info[i].sinal, AP_info[i].noise, AP_info[i].age, AP_info[i].ssid);
		if((100 + AP_info[i].rssi) >= STRENGTHTOLINK && strstr((const char *)AP_info[i].ssid,"NVR")){	
			count = i;
			
			for(j = 0; j < 6; j ++){
				macaddr[j] = AP_info[count].mac[j];
			}
			
			j=0;
			while(AP_info[count].ssid[j] != 0){
				essid[j] = AP_info[count].ssid[j];
				j ++;
				
			}
			essid[j] = '\0';
			ret = 2;
			return ret;
		}
		i ++;
	}
	return ret;
}

int nk_wifi_assoc_under_APmode(u8* essid , u8* password,char *ifname1,char *ifname2)
{
	int ret = 0;
	u8 mac[6];
	u8 essidget[40];
	int i = 0;
	
	memset(AP_info,0,sizeof(AP_info));
	NK_WIFI_search_ap(AP_info,ifname2,LIMIT_STRENGTH);
	ret = get_the_enougth_strlength_AP(essidget,mac);
	
	if(ret == 2){
		printf("get the NVR macaddr and start to explain the essid and password\n");
		printf("essid get is %s\n",essidget);
		for(i = 0; essidget[i] != 0; i ++){
			essid[i] = essidget[i];
		}
		essid[i] = '\0';
		snprintf((char *)password, 9, "%d%d%d%d%d%d%d%d", mac[0]%10==0?7:mac[0]%10, ((mac[0]&mac[3])+20)%10, ((mac[1]&mac[3])+13)%10, mac[3]%10, mac[2]%10, ((mac[4]&mac[2])+10)%10, ((mac[5]&mac[2])+18)%10, mac[5]%10);
		printf("now get the essid is %s\n", essid);
		printf("and the password is %s\n", password);
		return 1;
	}
	else{
		printf("search none NVR\n");
		return 0;
	}
}


int nk_wifi_assoc_under_STAmode(u8* essid, u8* password,char *ifname)
{
	struct linked_ap_info link_info;
	int ret = 0;
	u8 mac[6];
	u8 essidget[40];
	int i = 0;
	
	get_linked_AP_info(&link_info,ifname);
	if(link_info.linkstatus==0){		
		memset(AP_info,0,sizeof(AP_info));
		NK_WIFI_search_ap(AP_info,ifname,LIMIT_STRENGTH);
		ret = get_the_enougth_strlength_AP(essidget,mac);
		if(ret == 2){
			printf("get the NVR macaddr and start to explain the essid and password\n");
			for(i=0;essidget[i]!= 0;i++){
				essid[i] = essidget[i];
			}
			essid[i]= '\0';
			snprintf((char *)password,9,"%d%d%d%d%d%d%d%d", mac[0]%10==0?7:mac[0]%10, ((mac[0]&mac[3])+20)%10, ((mac[1]&mac[3])+13)%10, mac[3]%10, mac[2]%10, ((mac[4]&mac[2])+10)%10, ((mac[5]&mac[2])+18)%10, mac[5]%10);
			printf("now get the essid is %s\n",essid); 
			printf("and the password is %s\n",password);
			return 1;
		}
		else{
			printf("search none NVR\n");
			return 0;
		}
	}
	else if(link_info.linkstatus==1){
		printf("the IPCAM has linked to an AP\n");
		return 2;
		
	}		
	return ret;

}



int need_to_run_assoc(char *ifname)
{
	struct linked_ap_info _linked_ap_info;
	get_linked_AP_info(&_linked_ap_info, ifname);
	if(_linked_ap_info.linkstatus == 1 && _linked_ap_info.sinal_strength > 10){
		return 0;
	}
	else{
		return 1;
	}
}

static int get_the_time()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday ( &tv, &tz );
	return tv.tv_sec;
}

int nk_wifi_assocAP(fWIFI_MODE_do wifiswitch,fWIFI_MODE_SWITCH do_trap,char *ifname1,char *ifname2)
{
	int ret = 0;
	//u8 essid[40];
	//u8 password[20];
	adapter_status stadpater[2];
	//stWifiAttr attr;
	
	check_adpater_status(stadpater,ifname1);
	
	while(m_assoc_thread){
		if(_wifi_seek_attr.seek_count == 0){
		_wifi_seek_attr.start_time = get_the_time();
		//printf("start time is %d\n",_wifi_seek_attr.start_time);
		}else{
			_wifi_seek_attr.end_time = get_the_time();
			if((_wifi_seek_attr.end_time-_wifi_seek_attr.last_end_time) > 1000){
				_wifi_seek_attr.start_time = get_the_time();
			}
			_wifi_seek_attr.last_end_time = _wifi_seek_attr.end_time;
		/*printf("end time is %d,wireless match code has run %d seconds\n",_wifi_seek_attr.end_time,
			(_wifi_seek_attr.end_time-_wifi_seek_attr.start_time));*/
		}
		if((_wifi_seek_attr.end_time - _wifi_seek_attr.start_time)/60 >= SMART_LINK_RUN_TIME ){
			printf("wireless match code has run more than 10 minutes,now stop it\n");
			NK_WIFI_stop_assoc_thread("wlan0");
			break;
		}
		_wifi_seek_attr.seek_count ++;

		if(need_to_run_assoc(ifname1)){
			if(is_running_smartlink == 0){
					smart_link_rtl8188_init(ifname1, do_trap, last_connect_essid, SMARTLINK_WIFI_MODE_STATION, 0, print_log2, NULL, NULL);
					printf("start smartlink\n");
					is_running_smartlink = 1;
			}
		}

		#if 0
		switch(stadpater[0].mode){
			case 0:
			case 4:
				if(need_to_run_assoc(ifname1)){
					if(is_running_smartlink == 0){
						smart_link_rtl8188_init(ifname1, do_trap, last_connect_essid, SMARTLINK_WIFI_MODE_STATION, 0, print_log2);
						printf("start smartlink\n");
						is_running_smartlink = 1;
					}
				}
				break;
			case 1:
				sleep(2);
				ret = nk_wifi_assoc_under_APmode(essid, password,ifname1,ifname2);
				if(ret==1){
					printf("essid: %s\n",essid);
					printf("pwd: %s\n",password);
					strcpy(attr.essid,(char *)essid);
					strcpy(attr.password,(char *)password);
					attr.status = SMARTLINK_STATUS_SUCCESS;
					attr.mode = SMARTLINK_WIFI_MODE_STATION;
					attr.dhcp = 0;//dhcp
					system("ifconfig wlan0 down");
					system("kill -9 `pidof udhcpc`");
					system("kill -9 `pidof hostapd`");
					system("kill -9 `pidof udhcpd`");
					system("ifconfig wlan0 up");
					do_trap(&attr);
					exit(0);
				}
				break;
			
			default :
				//printf("unkown mode\n");
				//return 0;
				break;

		}
		#endif
		ret = 0;
		check_adpater_status(stadpater,ifname1);
		sleep(3);
	}

	pthread_exit ( NULL ) ;	
	return ret;

}



void reset_the_time_of_smartlink()
{
	if(_wifi_seek_attr.seek_count != 0){
		_wifi_seek_attr.seek_count = 0;
		printf("reset the wireless match code time\n");

	}

}

void nk_wifi_stop_wireless_macth_code()
{
	if(is_running_smartlink == 1){
		//smart_link_rtl8188_quit("wlan0");
		is_running_smartlink = 0;
	}

}

int nk_wifi_check_status_and_assoc_AP(fWIFI_MODE_SWITCH do_trap,char *ifname1,char *ifname2,char *lastessid)
{
	int ret = 0;
	adapter_status stadpater[2];
	
	if(_wifi_seek_attr.seek_count == 0){
		_wifi_seek_attr.start_time = get_the_time();
		check_adpater_status(stadpater, ifname1);
		if( stadpater[0].mode == 1 ){
			printf("the init mode is AP\n");
			g_init_mode = 1;
		}
		//printf("start time is %d\n",_wifi_seek_attr.start_time);
	}else{
		_wifi_seek_attr.end_time = get_the_time();
		if((_wifi_seek_attr.end_time - _wifi_seek_attr.last_end_time) > 1000){
			_wifi_seek_attr.start_time = get_the_time();
		}
		_wifi_seek_attr.last_end_time = _wifi_seek_attr.end_time;
		/*printf("end time is %d,wireless match code has run %d seconds\n",_wifi_seek_attr.end_time,
			(_wifi_seek_attr.end_time-_wifi_seek_attr.start_time));*/
	}
	if((_wifi_seek_attr.end_time - _wifi_seek_attr.start_time)/60 >= SMART_LINK_RUN_TIME ){
		printf("wireless match code has run more than 10 minutes,now stop it\n");
		nk_wifi_stop_wireless_macth_code();
		return -1;

	}
	_wifi_seek_attr.seek_count ++;
	if(!need_to_run_assoc(ifname1)){
		//printf("don't need to run assoc\n");
		return 1;
	}
	
	if(need_to_run_assoc(ifname1)){
		if(is_running_smartlink == 0){
			smart_link_rtl8188_init(ifname1, do_trap, lastessid, SMARTLINK_WIFI_MODE_AP, 0, print_log2, NULL, NULL);
			printf("start smartlink\n");
			is_running_smartlink = 1;
		}
	}
	#if 0
	switch(stadpater[0].mode){
		case 0:
		case 4:
			//under STA mode
			usleep(200000);
			
			break;
		case 1:
			//under AP mode 
			ret = nk_wifi_assoc_under_APmode(essid, password,ifname1,ifname2);
			if(ret==1){
				printf("essid: %s\n",essid);
				printf("pwd: %s\n",password);
				strcpy(attr.essid,(char *)essid);
				strcpy(attr.password,(char *)password);
				attr.status = SMARTLINK_STATUS_SUCCESS;
				attr.mode = SMARTLINK_WIFI_MODE_STATION;
				attr.dhcp = 0;//dhcp
				system("ifconfig wlan0 down");
				system("kill -9 `pidof udhcpc`");
				system("kill -9 `pidof hostapd`");
				system("kill -9 `pidof udhcpd`");
				system("ifconfig wlan0 up");
				do_trap(&attr);
				exit(0);
			}
			break;	
		default :
			//printf("unkown mode\n");
			//return 0;
			break;

	}
	#endif
	return ret;
}

static void* nk_wifi_assoc(void *arg)
{
	m_assoc_thread = 1;
	nk_wifi_assocAP(wifiswitch, do_trap,mifname1, mifname2);

	return NULL;
}

/*start assoc wifi thread
*argument :
*	lastessid:the essid IPC connect last time ,it is used unde STA mode in the smartlink thread
*	_wifi_switch: the function used to connect the NVR's AP under AP mode
*	_do_trap: the function used to connect the NVR's AP under STA mode 
*	ifname1:the wlan interface 1
*	ifname2:the wlan interface 2 ,it is mainly used under the AP mode to search the AP
*start successfully: return 1
*fail to start: return 0
*/
int NK_WIFI_start_assoc_thread(char *lastessid,fWIFI_MODE_SWITCH _do_trap,char *ifname1,char *ifname2)
{
	printf("now start the assoc thread\n");
	strcpy(last_connect_essid, lastessid);
	do_trap = _do_trap;
	strncpy(mifname1, ifname1, strlen(ifname1));
	strncpy(mifname2, ifname1, strlen(ifname2));
	if ( !m_assoc_thread_id ){
		m_assoc_thread = 0;
		if ( pthread_create( &m_assoc_thread_id, 0, nk_wifi_assoc, NULL) != 0 ){
			m_assoc_thread = 0;
			m_assoc_thread_id = (pthread_t)NULL;
			return 0;
		}
		return 1;
	}
	
	return 0;
}

/*stop assoc wifi thread
 *argument :
 *	ifname:the wlan interface that has been used to start the smartlink,you should stop it at the stop thread
 *stop successfully: return 1
 *fail to stop:return 0
*/
int NK_WIFI_stop_assoc_thread(char *ifname)
{
	printf("enter the stop function\n");
	if ( m_assoc_thread_id){
		m_assoc_thread = 0;
		pthread_join ( m_assoc_thread_id, NULL ) ;
		m_assoc_thread_id = (pthread_t)NULL;
		if(is_running_smartlink == 1){
			smart_link_rtl8188_quit(ifname);
			is_running_smartlink = 0;
		}
		printf("stop the assoc thread successfully\n");
		return 1;
	}	
	return 0;
}

static int nk_wifi_net_interface_monitor(char *nic_name)
{
	//int addr[4];
	struct ifreq ifr;
	int skfd;
	
	//open net device.
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if( skfd < 0 ){
		return -1;
	}
	//check driver status.
	strncpy(ifr.ifr_name, nic_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0; 
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		perror("ioctl");
		close(skfd);
		return -1;
	}

	if(ifr.ifr_flags & IFF_RUNNING){      //when rssi unaqual zero.sta mode
		//printf("Device(%s)_no RUNNING.\n",nic_name);
		close(skfd);
		return 1;
	}

	if(ifr.ifr_flags & IFF_UP){      //when rssi unaqual zero.sta mode
		//printf("Device(%s)_UP.\n",nic_name);
		close(skfd);
		return 0; 
	}
	/*   
	static char wlan_gateway[64]; 
	//get ipaddr 
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0){
		perror("ioctl");
		close(skfd);
		return -1;
	}

	printf("ip:%s,wlan_gateway:%s\n", inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr),wlan_gateway);
	if(4 == sscanf(((unsigned int*)&(ifr.ifr_addr))->sin_addr, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3])){
		sprintf(wlan_gateway,"%d.%d.%d.%d", addr[0], addr[1], addr[2], 1);
	}
	else{
		strcpy(wlan_gateway,"172.20.14.1");
	}
	*/
	close(skfd);

	return -1; 
}

static unsigned long long *nk_wifi_ipconfig(char *ath0)  
{   
	int fd = open("/proc/net/dev", O_RDONLY | O_EXCL);  
	if (-1 == fd){  
	    printf("/proc/net/dev not exists!\n");  
	    return NULL;  
	}  
	  
	char buf[1024];  
	lseek(fd, 0, SEEK_SET);  
	int nBytes = read(fd, buf, sizeof(buf)-1);  
	if (-1 == nBytes){  
		perror("wifi ifconfig read error.\n");  
		close(fd);  
		return NULL;  
	}  
	close(fd);
	buf[nBytes] = '\0';  

	//返回第一次指向ath0位置的指针  
	char* pDev = strstr(buf, ath0);  
	if (NULL == pDev){  
		printf("wifi don't find dev %s\n", ath0);  
		
		return NULL;  
	}  
	char *p;  
	char ifconfig_value[20];  
	int i = 0;  
	static unsigned long long  rx2_tx10[2];  
	/*去除空格，制表符，换行符等不需要的字段*/  
	for (p = strtok(pDev, " \t\r\n"); p; p = strtok(NULL, " \t\r\n")){  
	    i++;  
	    strcpy(ifconfig_value, p);  
	    /*得到的字符串中的第二个字段是接收流量*/  
	    if(i == 2)  {  
	        rx2_tx10[0] = atoll(ifconfig_value);  
	    }  
	    /*得到的字符串中的第十个字段是发送流量*/  
	    if(i == 10){  
	        rx2_tx10[1] = atoll(ifconfig_value);  
	        break;  
	    }  
	}  
	return rx2_tx10;  
}  
#ifdef DEBUG_LOG_SAVE
static char printbuf[128];
int log_print_save(int index, const char *fmt)
{
	int fd;
	static off_t thispos;

	fd=open("/tmp/temp",O_WRONLY|O_CREAT);
	if(fd < 0){
		printf("wifi print error 0.\n");
		return -1;
	}

	switch(index){
		case 0:
			thispos = strlen(fmt);
			if(write(fd,fmt,strlen(fmt)) < 0){
				printf("wifi print error 1.\n");
				close(fd);
				return -1;
			}
			break;			
		case 1:
			if(lseek(fd, thispos, SEEK_SET) < 0){
				printf("wifi print error 2.\n");
				close(fd);
				return -1;
			}
			if(write(fd,fmt,strlen(fmt)) < 0){
				printf("wifi print error 3.\n");
				close(fd);
				return -1;
			}
			break;
		default:
			break;
	}
	//printf("print index:%d, currpos: %d.\n",index,thispos);
	close(fd);

	return 0;
}
#endif

int NK_WIFI_arp_pick_ip(char *iface, char ip_ret[][NK_WIFI_MAX_ARP_IP_LEN], int max_num, unsigned int *ipformat)
{
	FILE *fp = NULL;
	int ret = 0, num = 0, type, flags;
	char ip[128];
	char hwa[128];
	char mask[128];
	char line[128];
	char dev[128];
	unsigned int u8ip[4];

	if(!iface || !ip_ret || max_num <= 0){
		return num;
	}

	fp = fopen("/proc/net/arp", "r");
	if(!fp){
		return num;
	}

	/* Bypass header -- read one line */
	fgets(line, sizeof(line), fp);

	/* Read the ARP cache entries. */
	while (fgets(line, sizeof(line), fp)) {

		mask[0] = '-'; mask[1] = '\0';
		dev[0] = '-'; dev[1] = '\0';
		/* All these strings can't overflow
		 * because fgets above reads limited amount of data */
		ret = sscanf(line, "%127s 0x%x 0x%x %127s %127s %127s\n",
					 ip, (unsigned int *)&type, (unsigned int *)&flags, hwa, mask, dev);
		if (ret < 4)
			break;

		if(strcmp(dev, iface) == 0){
			/* #define ATF_COM         0x02        completed entry (ha valid)   */
			if(flags != 0x02){
				continue;
			}
			snprintf(ip_ret[num], NK_WIFI_MAX_ARP_IP_LEN, "%s", ip);
			if(ipformat){
				sscanf(ip_ret[num], "%d.%d.%d.%d", (int *)&u8ip[0], (int *)&u8ip[1], (int *)&u8ip[2], (int *)&u8ip[3]);
				if(memcmp(u8ip, ipformat, sizeof(unsigned int) * 3) == 0){
					num ++;
				}
			}
			else{
				num ++;
			}
		}

		if(num >= max_num){
			break;
		}
	}

	fclose(fp);
	fp = NULL;

	return num;
}

static int nk_wifi_monitor_data()  
{  
	unsigned long long rx_bytes, tx_bytes;
	static unsigned long long last_rx_bytes = 0, last_tx_bytes = 0; 
	static int data_equal_count = 0;
	unsigned long long  *ifconfig_result;   
	ifconf_interface_t ifconf_irf;
	unsigned int u8ip[4];
	char gw_ip[NK_WIFI_MAX_ARP_IP_LEN];
	char arp_get_ip[1][NK_WIFI_MAX_ARP_IP_LEN];
	char cmd[64];
	 
	ifconfig_result = nk_wifi_ipconfig(WIFI_DEVIF);  

	//if wifi driver dead, signal stop change, reboot and fixed it.
	if(ifconfig_result){
		rx_bytes = ifconfig_result[0];
		tx_bytes = ifconfig_result[1];
#ifdef DEBUG_LOG_SAVE
		memset(printbuf,0,sizeof(printbuf));
		sprintf(printbuf,"#1# rx_bytes:%llu,last_rx:%llu,tx_bytes:%llu,last_tx:%llu,data_count:%d.\n", 
		  rx_bytes, last_rx_bytes, tx_bytes, last_tx_bytes,data_equal_count);
		log_print_save(1, printbuf); 
		printf("%s", printbuf);
#endif		
		if((rx_bytes == last_rx_bytes) || (tx_bytes == last_tx_bytes)){
			//system("ping 172.20.14.1 -w 1");
			//system("ping 192.168.1.198 -w 1");
			memset(&ifconf_irf, 0, sizeof(ifconf_interface_t));
			ifconf_get_interface(WIFI_DEVIF, &ifconf_irf);
			snprintf(gw_ip, sizeof(gw_ip), "%d.%d.%d.%d", ifconf_irf.gateway.s_b[0], ifconf_irf.gateway.s_b[1], ifconf_irf.gateway.s_b[2], ifconf_irf.gateway.s_b[3]);
			snprintf(cmd, sizeof(cmd), "ping %s -w 1", gw_ip);
			//printf("[%s:%d] cmd(%s)\n", __func__, __LINE__, cmd);
			NK_SYSTEM(cmd);

			memset(arp_get_ip, 0, sizeof(arp_get_ip));
			u8ip[0] = ifconf_irf.ipaddr.s_b[0];
			u8ip[1] = ifconf_irf.ipaddr.s_b[1];
			u8ip[2] = ifconf_irf.ipaddr.s_b[2];
			u8ip[3] = ifconf_irf.ipaddr.s_b[3];
			if(NK_WIFI_arp_pick_ip(WIFI_DEVIF, arp_get_ip, 1, u8ip) > 0){
				if(strcmp(arp_get_ip, gw_ip) != 0){
					snprintf(cmd, sizeof(cmd), "ping %s -w 1", arp_get_ip[0]);
					//printf("[%s:%d] cmd(%s)\n", __func__, __LINE__, cmd);
					NK_SYSTEM(cmd);
				}
			}

			last_rx_bytes = rx_bytes;
			last_tx_bytes = tx_bytes;
			data_equal_count += 1;
			if(data_equal_count > 15){
				data_equal_count = 0;
				return 1;
			}
		}else{
			last_rx_bytes = rx_bytes;
			last_tx_bytes = tx_bytes;
			data_equal_count = 0;
		}
	}

	return 0;
}  

int nk_wifi_check_adpater_status()
{
	int ret;
	adapter_status stadpater[2];

	memset(stadpater, 0, sizeof(adapter_status));
	ret = check_adpater_status(stadpater,"wlan0");
	if(ret >= 0){
		if(ret == 0xE4){
			printf("#####wifi device disappear, reboot and fixed it.#####\n");
			return 0xE4;
		}
		//wlan0 driver stop
		if((1 == stadpater[0].removedstatus) || (1 == stadpater[0].driver_stop)){
			printf("#####wifi driver stop, reboot and fixed it.#####\n");
			return 0xE5;
		}
		//aes error
		if(stadpater[0].aes_error_count > 100){
			printf("#####wifi aes_error, reboot and fixed it.#####\n");
			return 0xE3;
		}
			
		if(!(stadpater[0].fw_state & 0x00000010 /* no AP mode */) && (stadpater[0].fw_state & 0x00000001)){
			if(nk_wifi_monitor_data()){
				printf("#####wifi signal stop, reboot and fixed it.#####\n");
				return 0xE2;
			}
		}
	}
#if defined(REPEATER)	
	//if IP addr is null
	if(nk_wifi_net_interface_monitor("wlan1") < 0){
		printf("#####wlan0 down or no ip, reboot and fixed it.#####\n");
		return 0xE6;
	}
#else
	//if IP addr is null
	if(nk_wifi_net_interface_monitor("wlan0") < 0){
//		printf("#####wlan0 down or no ip, reboot and fixed it.#####\n");
		return 0xE6;
	}	
#endif 	
#ifdef DEBUG_LOG_SAVE	
	memset(printbuf,0,sizeof(printbuf));
	sprintf(printbuf,"#0# io ret:%d,removed:%d, stop:%d, aes:%u, fw_state:0x%x, qual:%d, streng:%d.\n",ret,stadpater[0].removedstatus, stadpater[0].driver_stop,stadpater[0].aes_error_count, stadpater[0].fw_state,stadpater[0].signal_qual, stadpater[0].sinal_strength);
	log_print_save(0, printbuf);
	printf("%s", printbuf);
#endif	
	return 0;	
}

static void nk_wifi_adapter_reset()
{
	NK_SYSTEM("ifconfig wlan0 down;ifconfig wlan1 down");
	NK_SYSTEM("rmmod rtl8188eus;rmmod rtl8188fu"); //for hisi
	NK_SYSTEM("rmmod 8188eu;rmmod 8188fu");        //for ak
	sleep(1);
    GLOBAL_reboot_system();
}

static void* nk_wifi_adapter_monitor(void *arg)
{
	int err_ret;
	static int err_ret_count = 0;

	prctl(PR_SET_NAME, "wifi monitor");

	for(err_ret = 0; err_ret < 15 && m_ApapterMonitorArg.adapter_thread; err_ret++){
		sleep(1);
	}
	printf("nk_wifi_adapter_monitor thread id (%lu)%sstart.\n",m_ApapterMonitorArg.adapter_thread_id, m_ApapterMonitorArg.adapter_thread ? " " : " not ");
	while(m_ApapterMonitorArg.adapter_thread){
		sleep(20);
		if(m_ApapterMonitorArg.pause_flg == true){
			//printf("pause the wifi adapter check\n");
			continue;
		}
		err_ret = nk_wifi_check_adpater_status() ;
		if(err_ret == 0xE4 || (!do_check_smartlink_status())){
			if((0xE3 == err_ret) || (0xE2 == err_ret)){
				if(do_reboot){
					do_reboot();
				}else{
					nk_wifi_adapter_reset();
				}
			}else if(err_ret > 0){
				err_ret_count ++;
				printf("### nk_wifi_err_ret_count(%d). ###\n", err_ret_count);
				if(err_ret_count > 3){
					err_ret_count = 0;
					if(do_reboot){
						do_reboot();
					}else{
						nk_wifi_adapter_reset();
					}
				}
			}else{
				err_ret_count = 0;
				//printf("### wifi wlan0 health. ###\n");
			}
		}
	}	
	return NULL;
}

int NK_WIFI_adapter_monitor_thread_start(fWIFI_SYSTEM_REBOOT_do _do_reboot, fWIFI_SYSTEM_CHECK_SMARTLINK_do _do_check_smartlink_status)
{
    pthread_attr_t pthread_attr;
    int nRet;

	if(_do_reboot)
		do_reboot = _do_reboot;

	if(_do_check_smartlink_status){
		do_check_smartlink_status = _do_check_smartlink_status;
	}

	if ( !m_ApapterMonitorArg.adapter_thread ){
		m_ApapterMonitorArg.adapter_thread = 1;
        nRet = pthread_attr_init(&pthread_attr);
        if(0 == nRet)
        {
            pthread_attr_setstacksize(&pthread_attr, 131072);
            pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
            if ( pthread_create( &m_ApapterMonitorArg.adapter_thread_id, &pthread_attr, nk_wifi_adapter_monitor, NULL) != 0 ){
                m_ApapterMonitorArg.adapter_thread = 0;
                m_ApapterMonitorArg.adapter_thread_id = (pthread_t)NULL;
                pthread_attr_destroy(&pthread_attr);
                return 0;
            }
            pthread_attr_destroy(&pthread_attr);
            return 1;
        }
	}
	
	return 0;
}

/*stop assoc wifi thread
 *argument :
 *	ifname:the wlan interface that has been used to start the smartlink,you should stop it at the stop thread
 *stop successfully: return 1
 *fail to stop:return 0
*/
int NK_WIFI_adapter_monitor_thread_stop(void)
{
	printf("enter the stop function\n");
	if ( m_ApapterMonitorArg.adapter_thread_id){
		m_ApapterMonitorArg.adapter_thread = 0;
		pthread_join ( m_ApapterMonitorArg.adapter_thread_id, NULL ) ;
		m_ApapterMonitorArg.adapter_thread_id = (pthread_t)NULL;
		printf("stop the assoc thread successfully\n");
		return 1;
	}	
	return 0;
}	

void NK_WIFI_set_wifi_disappear_flag()
{
	is_running_smartlink = 0;
}

int NK_WiFI_set_adapter_minitor_pause_flag(bool flag)
{
	m_ApapterMonitorArg.pause_flg = flag;
	return m_ApapterMonitorArg.pause_flg;
}

#ifdef WIFI_SEEK
static int write_wpaconfinfo(char *essid, char *password)
{
	 	FILE *fin,*ftp;
        char *str;
        fin=fopen("/tmp/wpa_supplicant.conf","a+");
        ftp=fopen("/tmp/tmp.txt","a+");
        if(fin==NULL || ftp==NULL){
                printf("Open the file failure...\n");
                exit(0);
        }
        str="ctrl_interface=/tmp/wpa_supplicant\n";
        fputs(str,ftp);
        str="update_config=1\n";
        fputs(str,ftp);
        str="network={\n";
        fputs(str,ftp);
        fprintf(ftp,"     ssid=\"%s\"\n     psk=\"%s\"\n     scan_ssid=1\n}\n",essid,password);
        fclose(fin);
        fclose(ftp);
        remove("/tmp/wpa_supplicant.conf");
        rename("/tmp/tmp.txt","/tmp/wpa_supplicant.conf");
		return 0;


}
/*This function is used under the AP mode*/


/*This function used in the smartlink */
static void* wifi_mode_switch(lpWifiAttr attr)
{

	void *ret = 0;
    char cmd[256] = {0};
	
	if(attr){
		printf("%s:%d, essid:%s  psw:%s  mode:%d  dhcp:%d token=%s\n",
			__FUNCTION__, __LINE__, attr->essid, attr->password, attr->mode, attr->dhcp,attr->token);

	}
	else{
		printf("%s:%d need to parse the protocal:smart_link_message_from_host[]!\n",
			__FUNCTION__, __LINE__);
	}

	
	if(attr->status == SMARTLINK_STATUS_SUCCESS){
		if(attr->essid[0]==0){
			//get the beacon packet of last connect essid	
			printf("get the beacon packet of last connect essid\n");
			smart_link_rtl8188_quit("wlan0");
            snprintf(cmd, sizeof(cmd), "%s/wifi_tools/wpa_supplicant -Dwext -iwlan0  -c /tmp/wpa_supplicant.conf -B -dd", IPCAM_ENV_HOME_DIR);
			NK_SYSTEM(cmd);
			sleep(10);
			is_running_smartlink = 0;
		}

		else {
			//use the new essid to connect
			printf("get new essid and the password!\n");
			write_wpaconfinfo(attr->essid, attr->password);
			smart_link_rtl8188_quit("wlan0");
			snprintf(cmd, sizeof(cmd), "%s/wifi_tools/wpa_supplicant -Dwext -iwlan0  -c /tmp/wpa_supplicant.conf -B -dd", IPCAM_ENV_HOME_DIR);
			NK_SYSTEM(cmd);
			sleep(10);
			is_running_smartlink = 0;
		}

	}
	return ret;
}

int main()
{
	int i;
	struct linked_ap_info _linked_ap_info;
	/*is_running_smartlink = 0;
	int connected;
	int linkstrength;
	int i = 0;
	adapter_status stadpater[2];*/
	while(1){
		nk_wifi_check_status_and_assoc_AP(wifi_mode_switch, "wlan0", "wlan1", "NVR9a2015823658");
		sleep(3);
	}
	//nk_wifi_check_status_and_assoc_AP(wifi_mode_switch, "wlan0", "wlan1", "NVR9a2");

	printf("start ...\n");
 	//nk_wifi_adapter_monitor_thread_start(NULL);
	//getchar();

	get_linked_AP_info(&_linked_ap_info, "wlan0");
	printf("linkstatus=%d:sinal_strength=%d\n",_linked_ap_info.linkstatus, _linked_ap_info.sinal_strength);
 	
	memset(AP_info,0,sizeof(AP_info));
	NK_WIFI_search_ap(AP_info,"wlan0",LIMIT_STRENGTH);
	for(i=0; i<MAX_AP_NUM; i++){
		if(AP_info[i].save)
			printf("%3d, %3d, %3d,ssid%d:%s\n",AP_info[i].ch,AP_info[i].rssi,AP_info[i].sinal,AP_info[i].index, AP_info[i].ssid);
	}

	return 0;	
}
#endif
