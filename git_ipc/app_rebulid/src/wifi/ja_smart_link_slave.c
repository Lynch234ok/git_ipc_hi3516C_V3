

#include "ja_smart_link.h"
#include <stdarg.h>
#include "base/ja_process.h"
#include <sys/prctl.h>  

extern int JN_Wifi_AP_Get_Connected_MAC(char *ifcae, int num, unsigned char mac[][6], char *wifi_type);
typedef struct smartlinkAttr{
	stWifiAttr attr;
	fWIFI_MODE_SWITCH do_trap;
	bool immediately_to_scan;
	smartl_link_print LOG;
	fWIFI_GET_WIFI_MODE get_wifi_mode;
	fWIFI_SET_WIFI_MODE set_wifi_mode;
}stSmartlinkAttr, *lpSmartlinkAttr;

typedef enum{
	RTL8188EUS,
	RTL8188FU
}emWIFI_DRIVER_TYPE;
emWIFI_DRIVER_TYPE current_wifi_driver_type;
/* 此值是用于rtl8188驱动的无线网卡进入monitor后切换信道的值 */
int rtl8188_monitor_lunxun_channel[14] ;
int rtl8188_monitor_channel[28] ;


static emWIFI_MODE wireless_mode_before_sl ;
static int m_receive_socket_rtl8188_fd;
static int buffer_tou;
static char m_receive_rtl8188_buffer[2000];

static char packet_pan_dap[6];
static char packet_cun_dap[6];

static char packet_cun_dsap[18];
static char packet_cun_dsap1[18];
static char packet_cun_dsap2[18];

static char packet_cun_dack[18];
static char packet_cun_dack_1[19];
static char packet_cun_dack_2[19];
static char beeacon_packet_pan_d[10];
static char beeacon_packet_cun_d[10];
static char smart_link_interface[30];
static char last_connect_essid[64];
static bool m_smart_link_thread = SMART_LINK_TREAD_FALSE;
static pthread_t m_smart_link_thread_id = ( pthread_t )NULL;
static st_wireless_envirement swe ;
st_all_ap_essid smart_link_all_ap_essid;
st_all_ap_essid smart_link_all_ap_essid_tmp;
char smart_link_message_from_host[SMART_LINK_MESSAGE_LENGTH];
static int mwpa_flag = 1;
#define SMART_LINK_WIFI_STRENGTH_LIMIT (72)
static int NVR_num = 0;
static int m_limit_sinal = 0;
#define SMART_LINK_TIME 1 //min
static bool m_is_running = 0;
static bool m_need_more_scan_time = 0;
static bool m_has_scan_product_ap = 0;
extern int need_to_run_assoc(char * ifname);
#define NK_PRODUCT_ESSID_HEAD "###"
#define MAX_AP_NUM 50
struct NVR_match_essid _possible_essid[MAX_AP_NUM];

#define P3_SMART_LINK //for P3 MTY

void print_log(const char *fmt, ...)
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

static stSmartlinkAttr _smartlinkAttr = {.LOG = print_log};

static void bubble_sort( pst_wireless_envirement pswe )
{
    int i, j, temp;
	
    for (j = 0; j < ( pswe->existing_channel_number - 1 ); j ++ ) {
        for (i = 0; i < ( pswe->existing_channel_number - 1 - j ); i ++ ) {
            if( pswe->channel_max_rssi[i] < pswe->channel_max_rssi[i + 1] ) {
                temp = pswe->channel_max_rssi[i] ;
                pswe->channel_max_rssi[i] = pswe->channel_max_rssi[i + 1] ;
                pswe->channel_max_rssi[i + 1] = temp ;
				temp = pswe->existing_channel[i] ;
				pswe->existing_channel[i] = pswe->existing_channel[i + 1] ;
				pswe->existing_channel[i + 1] = temp ;
            }
        }
    }
}

int __scan_wireless_signal( char *__interface, int timeout)
{
#ifndef SIOCSIWSCAN
#define SIOCSIWSCAN	0x8B18
#endif
#ifndef SIOCGIWSCAN
#define SIOCGIWSCAN	0x8B19
#endif
	iwreq wrq;
	int fd = -1, ret = -1;
	char buffer[126 * 2];
	bzero ( buffer, sizeof ( buffer ) );
	snprintf ( wrq.ifr_name, IFNAMSIZ, __interface );
	wrq.u.data.pointer = ( caddr_t ) buffer;
	wrq.u.data.length = sizeof ( buffer );
	wrq.u.data.flags = 0;

	if( ( fd = socket ( PF_INET, SOCK_DGRAM, IPPROTO_IP ) ) < 0 ) {
		printf( "create iwconfig socket error !!!|n" );
		return -1;
	}

	if ( ( ioctl ( fd, SIOCSIWSCAN, &wrq ) ) != 0 ) {
		printf( "ioctl  SIOCSIWSCAN error " );
		close ( fd ) ;
		return -1;
	}
	
	timeout = (timeout <= 0 || timeout > 300) ? 5 : timeout;
	timeout *= 5;
	//每次休眠200ms
	while(1){
		//用于接收扫描到的数据包,但是此处只是判断有无结果
		//最后结果通过/proc/net/rtl8188eu/wlan0/survey_info文件读取
		if((ioctl(fd, SIOCGIWSCAN, &wrq)) != 0){
			if(errno == E2BIG){
				//提示数据包过大,证明已经搜索完毕
				ret = 0;
				break;
			}
			else if(errno == EAGAIN){
				if(timeout <= 0){
					perror("scan wireless recv timeout!\n");
					break;
				}
			}
			else{
				perror("ioctl SIOCGIWSCAN");
				break;
			}
		}
		else{
			ret = 0;
			break;
		}

		usleep(200000);
		timeout--;
		if(timeout <= 0){
			break;
		}
	}

	if(fd > 0){
		close(fd);
	}
	return ret;
}

static int check_wifi_driver_type()
{
	FILE *fp;
	
	if( ( fp=fopen("/proc/net/rtl8188eu/wlan0/survey_info", "r"))!=NULL ){
		_smartlinkAttr.LOG("wifi driver is rtl8188eu \n");
		current_wifi_driver_type = RTL8188EUS;
		fclose(fp);
		return 1;
	}
	if( ( fp=fopen("/proc/net/rtl8188fu/wlan0/survey_info", "r") ) != NULL ){
		_smartlinkAttr.LOG("wifi driver is rtl8188fu \n");
		current_wifi_driver_type = RTL8188FU;
		fclose(fp);
		return 1;
	}
	return 0;
}
static int sort_the_channel()
{
	FILE *fp = NULL;
	char buffer[130] ;
	int index, ch, rssi, i ;
	char buffer_jie[130] ;
	bool cun_qu ;
	int bssid_count = 0 ;
	unsigned int mac_zhuan[6];
	int sdbm,noise,age;
	char flag[60],essid[60];
	int num = 0;
	NVR_num = 0;

	if(current_wifi_driver_type == RTL8188EUS){
		if ( ( fp = fopen ( "/proc/net/rtl8188eu/wlan0/survey_info", "r" ) ) == NULL ) {
			_smartlinkAttr.LOG( "can not open rtl8188eus survey_info file \n" ) ;
			_smartlinkAttr.LOG("try rtl8188fu surey_info file \n");
			return -1;
		}
	}else if(current_wifi_driver_type == RTL8188FU){
		if ( ( fp = fopen ( "/proc/net/rtl8188fu/wlan0/survey_info", "r" ) ) == NULL ){
			_smartlinkAttr.LOG("open rtl8188fu survey_info file faileed too\n");
			return -1 ;
		}
	}
	fgets ( buffer, 130, fp ) ;
	memset ( &swe, 0, sizeof ( st_wireless_envirement ) );
	memset(_possible_essid,0,sizeof(_possible_essid));
	
	while( fgets ( buffer, 130, fp ) && (bssid_count < 100) ) {
		sscanf ( buffer,"%d  %129s  %d  %d  %d  %d    %d  %59s  %59s\n", &index, buffer_jie, 
				&ch, &rssi, &sdbm, &noise, &age, flag, essid) ;
		sscanf(buffer_jie,"%02x:%02x:%02x:%02x:%02x:%02x", &mac_zhuan[0], &mac_zhuan[1],
							 &mac_zhuan[2], &mac_zhuan[3], &mac_zhuan[4], &mac_zhuan[5]);
		for ( i = 0; i < 6; i ++ ) {
			swe.bssid[bssid_count][i] = mac_zhuan[i];
		}
		
		if(num < MAX_AP_NUM ){
			if( !memcmp(essid, NK_PRODUCT_ESSID_HEAD, strlen(NK_PRODUCT_ESSID_HEAD)) ){
				for ( i = 0; i < 6; i ++ ) {
					_possible_essid[num].mac[i] = mac_zhuan[i];
				}
				strcpy(_possible_essid[num].essid, essid);
				_possible_essid[num].rssi = rssi;
				num ++;
			}
		}
		
		swe.bssid_channel[bssid_count] = ch;
		bssid_count ++ ;
		cun_qu = 0 ;
		for ( i = 0; i < swe.existing_channel_number ; i ++ ) {
			if ( ch != swe.existing_channel[i] ) {
				continue ;
			}
			swe.channel_max_rssi[i] = ( swe.channel_max_rssi[i] > rssi ) ? swe.channel_max_rssi[i] : rssi;
			cun_qu = 1 ;
		}

		if ( cun_qu == 0 ) {
			swe.existing_channel[i] = ch ;
			swe.channel_max_rssi[i] = rssi ;
			swe.existing_channel_number ++ ;
			cun_qu = 1;
		}
	}
	bubble_sort ( &swe ) ;

	memset ( &rtl8188_monitor_lunxun_channel, 0, sizeof ( rtl8188_monitor_lunxun_channel ) ) ;
	memset ( &rtl8188_monitor_channel, 0 , sizeof ( rtl8188_monitor_channel ) ) ;
	for ( i = 0 ; i < swe.existing_channel_number; i ++ ) {
		rtl8188_monitor_lunxun_channel[i] = swe.existing_channel[i];
		if ( ( swe.existing_channel[i] >= 1 ) && ( swe.existing_channel[i] <= 4 ) ) {
			rtl8188_monitor_channel[i * 2] = 1 ;
			rtl8188_monitor_channel[i * 2 + 1 ] = 1 ;
		}
		else if ( swe.existing_channel[i] != 14 ) {
			rtl8188_monitor_channel[i * 2] = 2 ;
			rtl8188_monitor_channel[i * 2 + 1 ] = 1 ;
		}
		else {
			rtl8188_monitor_channel[i * 2] = 0 ;
			rtl8188_monitor_channel[i * 2 + 1 ] = 0 ;
		}
		
	}

    fclose(fp);
	return 1;	
}

static int nk_mac_2_real_signal(char mac)
{
	switch(mac){
		case 0:
			return 0;

		case 1:
			return 30;

		case 2:
			return 35;

		case 3:
			return 40;

		case 4:
			return 45;

		case 5:
			return 65;

		case 6:
			return 70;

		case 7:
			return 75;

		case 8:
			return 80;	

		case 9:
			return 85;

		default:
			return 255;
	}

}

static int smartlink_check_rssi( char *essid, int signalstrength )
{
	int i = 0;
	
	while(i < MAX_AP_NUM){
		if( !strcmp(_possible_essid[i].essid, essid) ){
			printf("the sender essid is %s \n", _possible_essid[i].essid);
			printf("the sender ap strength is %d\n", (_possible_essid[i].rssi + 100));
			if( (_possible_essid[i].rssi + 100) >= nk_mac_2_real_signal(signalstrength) ){
				printf("sinal check pass\n");
				return 1;
			}
		}
		i ++;
	}

	printf("signal check failed\n");
	memset(smart_link_message_from_host, 0, sizeof(smart_link_message_from_host));
	return 0;
}

static int reset_rtl8188eu_monitor_channel( int channel )
{
	char cmdstr[100];				

	switch ( channel ) {
		case 1 :
			sprintf(cmdstr,"echo 1 1 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");
			break;
			
		case 2 :
			sprintf(cmdstr,"echo 2 1 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");
			break;
			
		case 3 :
			sprintf(cmdstr,"echo 3 1 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");			
			break;
				
		case 4 :
			sprintf(cmdstr,"echo 4 1 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");			
			break;

		case 5 :
			sprintf(cmdstr,"echo 5 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");
			break;
			
		case 6 :
			sprintf(cmdstr,"echo 6 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");
			break;
		case 7 :
			sprintf(cmdstr,"echo 7 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");			
			break;
				
		case 8 :
			sprintf(cmdstr,"echo 8 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");			
			break;

		case 9 :
			sprintf(cmdstr,"echo 9 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");
			break;
			
		case 10 :
			sprintf(cmdstr,"echo 10 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");
			break;
			
		case 11 :
			sprintf(cmdstr,"echo 11 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");			
			break;
				
		case 12 :
			sprintf(cmdstr,"echo 12 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");			
			break;

		case 13 :
			sprintf(cmdstr,"echo 13 2 1 > /proc/net/rtl8188eu/wlan0/monitor  \n");			
			break;
				
		case 14 :
			sprintf(cmdstr,"echo 14 0 0 > /proc/net/rtl8188eu/wlan0/monitor  \n");			
			break;
			
		default :
			printf ("set wrong channel, please check the parameter\n");
			break;
	}
	_smartlinkAttr.LOG("%s", cmdstr);
	NK_SYSTEM(cmdstr);
	return 1;
}

static int reset_rtl8188fu_monitor_channel( int channel )
{
	char cmdstr[100];				

	switch ( channel ) {
		case 1 :
			sprintf(cmdstr,"echo 1 1 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");
			break;
			
		case 2 :
			sprintf(cmdstr,"echo 2 1 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");
			break;
			
		case 3 :
			sprintf(cmdstr,"echo 3 1 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");			
			break;
				
		case 4 :
			sprintf(cmdstr,"echo 4 1 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");			
			break;

		case 5 :
			sprintf(cmdstr,"echo 5 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");
			break;
			
		case 6 :
			sprintf(cmdstr,"echo 6 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");
			break;
		case 7 :
			sprintf(cmdstr,"echo 7 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");			
			break;
				
		case 8 :
			sprintf(cmdstr,"echo 8 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");			
			break;

		case 9 :
			sprintf(cmdstr,"echo 9 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");
			break;
			
		case 10 :
			sprintf(cmdstr,"echo 10 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");
			break;
			
		case 11 :
			sprintf(cmdstr,"echo 11 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");			
			break;
				
		case 12 :
			sprintf(cmdstr,"echo 12 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");			
			break;

		case 13 :
			sprintf(cmdstr,"echo 13 2 1 > /proc/net/rtl8188fu/wlan0/monitor  \n");			
			break;
				
		case 14 :
			sprintf(cmdstr,"echo 14 0 0 > /proc/net/rtl8188fu/wlan0/monitor  \n");			
			break;
			
		default :
			printf ("set wrong channel, please check the parameter\n");
			break;
	}
	_smartlinkAttr.LOG("%s", cmdstr);
	NK_SYSTEM(cmdstr);
	return 1;
}

static void wirte_ap_conf()
{
	FILE *fin;
    char *str;
	
    fin=fopen("/tmp/smart_ap.conf","w");
	if( fin == NULL ){
		printf("Open the file failure...\n");
		return ;
	}

	str = "interface=wlan1\n";
	fputs(str,fin);
	str = "ctrl_interface=/tmp/hostapd\n";
	fputs(str,fin);
	str = "wmm_enabled=1\nmacaddr_acl=0\n";
	fputs(str,fin);	   
	str = "auth_algs=1\n";
	fputs(str,fin);
	str = "ignore_broadcast_ssid=0\n";
	fputs(str,fin);
	str = "ssid=123456\n";
	fputs(str,fin);
	str = "channel=1\n";
	fputs(str,fin);
	str = "hw_mode=g\n";
	fputs(str,fin);
	str = "ieee80211n=1";
	fputs(str,fin);

	fclose(fin);
		
}

static int JN_Wifi_Exit()
{
	char cmdstr[50];
	sprintf(cmdstr,"ifconfig %s down", smart_link_interface );	
	NK_SYSTEM( cmdstr );
	NK_SYSTEM("kill -9 `pidof wpa_supplicant`");
	mwpa_flag = 0;
	NK_SYSTEM("kill -9 `pidof udhcpc`");
	NK_SYSTEM("kill -9 `pidof hostapd`");
	NK_SYSTEM("kill -9 `pidof udhcpd`");
	sprintf(cmdstr,"ifconfig %s up", smart_link_interface );	
	NK_SYSTEM( cmdstr );
	return 0;
}

//return -1 need restart wlan0;return 0 igore, return 1 normal.
int smartlink_check_sta_status(char *nic_name)
{
    struct ifreq ifr;
    int skfd;
	
    //open net device.
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if( skfd < 0 ){
        return 0;
    }
    //check driver status.
    strcpy(ifr.ifr_name, nic_name);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("ioctl");
		_smartlinkAttr.LOG( "ioctl12: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
        close(skfd);
        return 0;
    }
    if(!(ifr.ifr_flags & IFF_UP)){      //when rssi unaqual zero.sta mode
        _smartlinkAttr.LOG("Device(%s)_down.\n",nic_name);
        close(skfd);
        return -1;    
    } 
    if(!(ifr.ifr_flags & IFF_RUNNING)){      //when rssi unaqual zero.sta mode
		close(skfd);
		return -1;
    }
	#ifndef  SMARTLINK_SLAVE
	if(need_to_run_assoc("wlan0")){
		close(skfd);
		return -1;
	}
	#endif
    
    //check ipaddr.
    #if 0
    strncpy(ifr.ifr_name, nic_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0; 
    ret = ioctl(skfd, SIOCGIFADDR, &ifr) ;
   // if ((ret < 0) && (ifr.ifr_flags & IFF_RUNNING)) 
   if (ret < 0){
        perror("ioctl");
		_smartlinkAttr.LOG( "ioctl: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
        close(skfd);
        return -1;
   }
    #endif
    close(skfd);
    
 
	return 1; 
}

//get all mac address which are connect success(hostapd_cli all_sta)(return all the mac of sta and the sum of connections)
int smartlink_check_ap_status(char *nic_name)
{
	FILE * fp; 
	int res;
	char *p, *ptr;
	char buf[150];
	char str[1024];
	char wifi_mac[32];
	char u8MACAddr[6];
	int index;

	index = -1;
	
	memset(buf, '\0', sizeof(buf));
	memset(str, '\0', sizeof(str));
	
	sprintf(str, "hostapd_cli -i%s all_sta", nic_name);
	
	if ((fp = popen(str, "r") ) == NULL) 
	{ 
		_smartlinkAttr.LOG("popen error: %s/n", strerror(errno)); 
		return -1; 
	} 
	else
	{
		while(fgets(buf, sizeof(buf), fp)) 
		{ 			
			ptr = strstr(buf, "dot11RSNAStatsSTAAddress"); 
			if(ptr)
			{
				p = strtok(ptr,"=");
				p = strtok(NULL, "\n");
				index++;
				strcpy(wifi_mac, p);
				if(strcmp(p,"") != 0 && strcmp(p, "00:00:00:00:00:00") !=0)
				{
					res = sscanf(p, "%x:%x:%x:%x:%x:%x", (unsigned int *)u8MACAddr, (unsigned int *)u8MACAddr+1, (unsigned int *)u8MACAddr+2, (unsigned int *)u8MACAddr+3, (unsigned int *)u8MACAddr+4, (unsigned int *)u8MACAddr+5);
					{
						//fix it
					}
					res = 0;
				}
			}			
		} 
		if ( (res = pclose(fp)) == -1) 
		{ 
			perror("close popen file pointer fp error!\n");
			_smartlinkAttr.LOG( "ioctl: %s [%s:%d]\n", strerror(errno),  __FILE__,  __LINE__);
			return -1;
		} 	
	}
	
	return index;
}

static int smartlink_net_interface_monitor(char *nic_name)
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
	
	close(skfd);

	return -1; 
}

bool need_to_run_monitor()
{
	bool retu = 0;

	if(need_to_run_assoc(smart_link_interface)){
		return 1;
	}
	
	return retu ;
}

#define NK_ESSID_MAC_LEN (12)
int NK_ESSID_to_MAC(char *essid, unsigned char mac[6])
{
    char buf[128], *ptr = NULL;
    unsigned int tmp[6];
    if(!essid || strlen(essid) < NK_ESSID_MAC_LEN){
        return -1;
    }
    ptr = essid + strlen(essid) - NK_ESSID_MAC_LEN;
    snprintf(buf, sizeof(buf), "%s", ptr);

    if(sscanf(buf, "%02x%02x%02x%02x%02x%02x", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]) != 6){
        return -1;
    }

    mac[0] = tmp[0];
    mac[1] = tmp[1];
    mac[2] = tmp[2];
    mac[3] = tmp[3];
    mac[4] = tmp[4];
    mac[5] = tmp[5];
    return 0;
}

#define NK_PD_TEST_ESSID_PREFIX "PDTEST_M8+rO_WI"
#define NK_PD_TEST_PASSWD		"O88+IIC4_KOWO"
static int nk_check_prodect_test(lpWifiAttr attr, lpNK_Match_Essid match_essid, int size)
{
	int i = 0, signal = 0, test = 0, opt = 0, tmp = 0;
	char *ptr = NULL;
	if(!match_essid){
		return 0;
	}

	_smartlinkAttr.LOG("enter nk_check_prodect_test\n");

	for(i = 0; i < size; i++){
		if(strncmp(NK_PD_TEST_ESSID_PREFIX, match_essid[i].essid, strlen(NK_PD_TEST_ESSID_PREFIX)) == 0){
			_smartlinkAttr.LOG("essid %s match\n", match_essid[i].essid);
			ptr = (match_essid[i].essid + strlen(NK_PD_TEST_ESSID_PREFIX));
			if(sscanf(ptr, "#%d#%d#%d", &test, &opt, &signal) != 3){
				_smartlinkAttr.LOG("essid opt not match\n");
				continue;
			}
			if(signal <= (match_essid[i].rssi + 100)){
				attr->dhcp = opt & 0x1;
				tmp = (opt >> 1) & 0x1;
				attr->status = tmp ? SMARTLINK_STATUS_SUCCESS : SMARTLINK_STATUS_SWITCH_TO_PD_TEST;
				snprintf(attr->essid, sizeof(attr->essid), "%s", match_essid[i].essid);
				snprintf(attr->password, sizeof(attr->password), "%s", NK_PD_TEST_PASSWD);
				attr->mode = SMARTLINK_WIFI_MODE_STATION;
				_smartlinkAttr.LOG("essid %s match, signal %d match\n", match_essid[i].essid, match_essid[i].rssi + 100);
				return 1;
			}
			else{
				_smartlinkAttr.LOG("essid %s match, but signal %d is lower %d\n", match_essid[i].essid, match_essid[i].rssi + 100, signal);
			}
		}
	}

	_smartlinkAttr.LOG("enter nk_check_prodect_test mismatch\n");

	return 0;
}

static int nk_check_prodect_test_by_mac(lpWifiAttr attr, lpNK_Match_Essid match_essid, int size)
{
	int i = 0, signal = 0, test = 0, opt = 0, tmp = 0;
	char *ptr = NULL;
	unsigned char real_mac[6] = {0};
	char NVR_pwd[16] = {0};
	int current_AP_rssi = 0, is_success = 0;

	if(!match_essid){
		return 0;
	}

	//_smartlinkAttr.LOG("enter nk_check_prodect_test_by_mac\n");

	for(i = 0; i < size; i++){
		if( ((match_essid[i].mac[0] & 0xF0) == 0xA0 ) && 
			((match_essid[i].mac[1]>>4) & 0x0F) == (0x0A & (match_essid[i].mac[0] & 0x0F)) ){
			_smartlinkAttr.LOG("mac %02x:%02x:%02x:%02x:%02x:%02x check pass\n", match_essid[i].mac[0], match_essid[i].mac[1],
				match_essid[i].mac[2], match_essid[i].mac[3], match_essid[i].mac[4], match_essid[i].mac[5]);
			if( memcmp(match_essid[i].essid, NK_PRODUCT_ESSID_HEAD, strlen(NK_PRODUCT_ESSID_HEAD)) ){
				continue;
			}
			signal = match_essid[i].mac[2];
			if(signal <= (match_essid[i].rssi + 100) && (match_essid[i].rssi + 100) > current_AP_rssi){	
				memset(attr, 0, sizeof(stWifiAttr));
				NK_ESSID_to_MAC((match_essid[i].essid + strlen(NK_PRODUCT_ESSID_HEAD)), real_mac);
				if(0 == memcmp(&match_essid[i].mac[3], &real_mac[3], 3)){
					attr->dhcp = match_essid[i].mac[1] & 0x1;
					tmp = (match_essid[i].mac[1] >> 1) & 0x1;
					attr->status = tmp ? SMARTLINK_STATUS_SUCCESS : SMARTLINK_STATUS_SWITCH_TO_PD_TEST;
					snprintf(attr->essid, sizeof(attr->essid), "%s", (match_essid[i].essid + strlen(NK_PRODUCT_ESSID_HEAD)));
					snprintf( NVR_pwd, 9, "%d%d%d%d%d%d%d%d", real_mac[0]%10==0?7:real_mac[0]%10, ((real_mac[0]&real_mac[3])+20)%10, 
						((real_mac[1]&real_mac[3])+13)%10, real_mac[3]%10, real_mac[2]%10, ((real_mac[4]&real_mac[2])+10)%10, ((real_mac[5]&real_mac[2])+18)%10, real_mac[5]%10);
					snprintf(attr->password, sizeof(attr->password), "%s", NVR_pwd);
					attr->mode = SMARTLINK_WIFI_MODE_STATION;
					current_AP_rssi = match_essid[i].rssi + 100;
					is_success = 1;
					_smartlinkAttr.LOG("essid %s match, signal %d match\n", match_essid[i].essid, match_essid[i].rssi + 100);
					m_has_scan_product_ap = 1;
				}
			} else {
				_smartlinkAttr.LOG("essid %s match, but signal %d is lower than the mac_check_sinal %d or the last mactch AP sinal %d, add the scan time\n", 
									match_essid[i].essid, match_essid[i].rssi + 100, signal, current_AP_rssi);
				m_need_more_scan_time = 1;
				m_has_scan_product_ap = 1;
				
			}
		}
	}
	
	if(is_success){
		return 1;
	}
	//_smartlinkAttr.LOG("enter nk_check_prodect_test_by_mac mismatch\n");

	return 0;
}

#define SCAN_MORE_TIME 4
static int ja_set_smart_link_envirement()
{
	int scan_time = 0;
	int i = 0;
	
#ifndef P3_SMART_LINK
	JN_Wifi_Exit();
	set_wlan_mode_managed( smart_link_interface );
#else
	mwpa_flag = 0;
#endif

	for(scan_time = 0; scan_time < 2; scan_time ++){
		__scan_wireless_signal(smart_link_interface , 3);
		sort_the_channel();
		if(nk_check_prodect_test_by_mac(&_smartlinkAttr.attr, _possible_essid, 
			sizeof(_possible_essid)/sizeof(_possible_essid[0]))){
			m_need_more_scan_time = 0;
			return 1;
		}
	}
	
#ifndef P3_SMART_LINK
	if(m_need_more_scan_time){
		for(i = 0; i < SCAN_MORE_TIME; i ++){
			for(scan_time = 0; scan_time < 5; scan_time ++){
				__scan_wireless_signal(smart_link_interface, 3);
				sort_the_channel();
				if(nk_check_prodect_test_by_mac(&_smartlinkAttr.attr, _possible_essid, 
					sizeof(_possible_essid)/sizeof(_possible_essid[0]))){
					m_need_more_scan_time = 0;
					return 1;
				}
			}	
		}
	}
	
	m_need_more_scan_time = 0;


	set_wlan_mode_monitor( smart_link_interface );
	usleep ( 300000 );	
	_smartlinkAttr.attr.status = SMARTLINK_STATUS_SWITCH_TO_MONITOR;
#endif
	return 1;
}

static int parse_wifi_attr(lpWifiAttr attr)
{
	char eap[150] = {0};	
	int mod_dhcp;
	int i;
	int essid_end;
	int psk_end;
	char sinal;
	
	if(attr){
		memset(attr, 0, sizeof(stWifiAttr));
	}else{
		return 0;
	}

	if ( !strcmp( smart_link_message_from_host, "" ) )
	{
		return 0;
	}

	memset(eap, 0, sizeof(eap));
	if( 2 != sscanf( smart_link_message_from_host,  "%d#%149s", &mod_dhcp, eap ) ){
		_smartlinkAttr.LOG("The message from host is not the right format.\n");
		return -1;
	}
	
	attr->mode = mod_dhcp / 10 ;
	attr->dhcp = ( bool ) ( mod_dhcp % 10 );
	
	for ( i = 0; i < 47; i ++ ) {   										//essid
		if ( smart_link_message_from_host[i + 3] != '#' ) {
			attr->essid[i] = smart_link_message_from_host[i + 3] ;
		}
		else {
			if ( smart_link_message_from_host[i + 4] != '&' ) {
				attr->essid[i] = smart_link_message_from_host[i + 3] ;
			}
			else {
				essid_end = i + 5 ;
				break;
			}
		}
	}

	for ( i = 0; i < 64; i ++ ) {										//psk		
		if ( smart_link_message_from_host[essid_end  + i] != '#' ) {
			attr->password[i] = smart_link_message_from_host[essid_end  + i] ;
		}
		else {
			if ( smart_link_message_from_host[essid_end  + i + 1] != '&' ) {
				attr->password[i] = smart_link_message_from_host[essid_end  + i] ;
			}
			else {
				psk_end = essid_end + i + 2 ;
				break;
			}
		}
	}

	if ( smart_link_message_from_host[psk_end] != 0 ) {		      // token
		for ( i = 0; i < 4; i ++ ) {
			attr->token[i] = smart_link_message_from_host[psk_end+ i];
		}

		if( smart_link_message_from_host[psk_end+ i] != 0 ) {
			sinal  = smart_link_message_from_host[psk_end+ i];
			m_limit_sinal = atoi(&sinal);
			printf("####the sinal is %d\n#####", m_limit_sinal);
		}else{
			m_limit_sinal = 0;
		}
	}
	
	return 0;
}

static void *smart_link_search_thread(void* arg)
{
	int ret;
	int channel = 0;
	char cmdstr[256];
	int start_time, end_time, last_end_time;
	struct timeval tv;
	struct timezone tz;
	
	gettimeofday ( &tv, &tz );
	start_time = tv.tv_sec;
	prctl(PR_SET_NAME, "smartlink_thread");
	while ( m_smart_link_thread  )
	{	
#ifndef WIFI_WITHOUT_ETH
		if( 1 == smartlink_net_interface_monitor("eth0") ){
			sleep(5);
			continue;
		}
#endif
		if(_smartlinkAttr.get_wifi_mode){
			if(_smartlinkAttr.get_wifi_mode() == SMARTLINK_WIFI_MODE_STATION){
				_smartlinkAttr.LOG("Wireless  has been match code, quit the thread\n");
				mwpa_flag = 1;
				smart_link_rtl8188_quit(smart_link_interface);
				break;
			}
		}
		
	#ifdef P3_SMART_LINK
		unsigned char connect_mac[2][6];
		char wifi_type[10];
		if(current_wifi_driver_type == RTL8188FU){
			strcpy(wifi_type, "rtl8188fu");
		}else if(current_wifi_driver_type == RTL8188EUS){
			strcpy(wifi_type, "rtl8188eu");
		}
		if(JN_Wifi_AP_Get_Connected_MAC(smart_link_interface, 2, connect_mac, wifi_type) > 0){
			goto checktime;
		}
	#endif
		
		
		if( need_to_run_monitor() ){	
			if(ja_set_smart_link_envirement()==0){
				goto checktime;
			}
			
			if(_smartlinkAttr.attr.status == SMARTLINK_STATUS_SUCCESS ||
				_smartlinkAttr.attr.status == SMARTLINK_STATUS_SWITCH_TO_PD_TEST){
				if(_smartlinkAttr.do_trap){
					_smartlinkAttr.do_trap(&_smartlinkAttr.attr);
				}
				m_is_running = false;
				mwpa_flag = 1;
				smart_link_rtl8188_quit(smart_link_interface);
				break;
			}
		#ifdef P3_SMART_LINK 				
			goto checktime;
			//do not run minotor mode to recv packets
		#endif
			_smartlinkAttr.attr.mode = SMARTLINK_WIFI_MODE_MONITOR;
			if(_smartlinkAttr.immediately_to_scan){
				_smartlinkAttr.attr.status = SMARTLINK_STATUS_SEARCH_CHANNEL;
			}
			
			if(_smartlinkAttr.do_trap){
				_smartlinkAttr.do_trap(&_smartlinkAttr.attr);
			}
			
			for ( channel = 0; channel < swe.existing_channel_number; channel ++ )
			{
				if(current_wifi_driver_type == RTL8188EUS){
					sprintf(cmdstr,"echo %d %d %d > /proc/net/rtl8188eu/wlan0/monitor  \n",rtl8188_monitor_lunxun_channel[channel],
															rtl8188_monitor_channel[channel*2],rtl8188_monitor_channel[channel*2+1]);
				}else if(current_wifi_driver_type == RTL8188FU){
					sprintf(cmdstr,"echo %d %d %d > /proc/net/rtl8188fu/wlan0/monitor  \n",rtl8188_monitor_lunxun_channel[channel],
															rtl8188_monitor_channel[channel*2],rtl8188_monitor_channel[channel*2+1]);
				}
				_smartlinkAttr.LOG("Smart link in channel : %d\n", rtl8188_monitor_lunxun_channel[channel] );
				NK_SYSTEM(cmdstr); 

				if(rtl8188_monitor_lunxun_channel[channel] == 11 ){
					ret = smart_link_rtl8188( 5 );
				}else{
					ret = smart_link_rtl8188( 3 );
				}
				if( ret == 0 ){		
					if ( 0 != parse_wifi_attr(&_smartlinkAttr.attr) ){
						_smartlinkAttr.LOG("Parse message error!\n");
						continue;
					}
					
					if( m_limit_sinal > 0 ) {
						if( !smartlink_check_rssi(_smartlinkAttr.attr.essid, m_limit_sinal) ){
							continue;
						}
					}
					
					channel = 15;
					set_wlan_mode_managed( smart_link_interface );
					_smartlinkAttr.immediately_to_scan = false;
					_smartlinkAttr.attr.status = SMARTLINK_STATUS_SUCCESS;
					if(_smartlinkAttr.do_trap){
						snprintf ( last_connect_essid, 64, "%s", _smartlinkAttr.attr.essid );
						_smartlinkAttr.LOG("last_connetc_essid=%s\n", last_connect_essid);
						_smartlinkAttr.do_trap(&_smartlinkAttr.attr);
						mwpa_flag = 1;
						sleep(30);
						if(!need_to_run_assoc(smart_link_interface)){
							break;
						}
					}		
				}else if(-2 == ret){
					_smartlinkAttr.attr.status = SMARTLINK_STATUS_RECEIVE_PACKET_TIMEOUT;
					printf("receive packet time out on the channel, goto next channel\n");
				}
			}			
		}
checktime: gettimeofday( &tv, &tz );
		end_time = tv.tv_sec;
		if( (end_time - start_time)/60 > 100 ){
			start_time = tv.tv_sec;
		}
	#ifdef  P3_SMART_LINK
		if( (end_time - start_time)/60 >= SMART_LINK_TIME ){
			_smartlinkAttr.LOG("time out stop smartlink !\n");
			smart_link_rtl8188_quit(smart_link_interface);
		}
	#else
		/*
		if( (end_time - start_time)/60 >= 1 ){
			_smartlinkAttr.LOG("just run scan pro time out stop smartlink !\n");
			smart_link_rtl8188_quit(smart_link_interface);
		}*/
	#endif
		//usleep(500000);
		sleep(5);
	}		
	
	pthread_exit ( NULL ) ;
}	


static void smart_link_start_thread()
{
	if ( !m_smart_link_thread_id )
	{
		m_smart_link_thread = SMART_LINK_TREAD_TRUE;
		if ( pthread_create( &m_smart_link_thread_id, 0, smart_link_search_thread, NULL ) != 0 )
		{
			m_smart_link_thread = SMART_LINK_TREAD_FALSE;
			m_smart_link_thread_id = (pthread_t)NULL;
		}
		m_is_running = true;
	}
}

static void smart_link_stop_thread()
{
	if ( m_smart_link_thread_id)
	{
		m_smart_link_thread = SMART_LINK_TREAD_FALSE;
		pthread_join ( m_smart_link_thread_id, NULL ) ;
		m_smart_link_thread_id = (pthread_t)NULL;
	}
}	
	

/* float类型转换为iw_req */
void iw_float2freq ( double in, iw_freq *out )
{
  out->e = (short) (floor(log10(in)));
  if(out->e > 8)
    {
      out->m = ((long) (floor(in / pow(10,out->e - 6)))) * 100;
      out->e -= 8;
    }
  else
    {
      out->m = (long) in;
      out->e = 0;
    }
}


/* 设置信道函数，ifname为无线网卡设备名称，channel为欲切换信道 */
int set_channel ( char *ifname, double channel )
{
	iwreq wrq;
	double	freq;
	int fd;
    freq = channel;
	
	iw_float2freq ( freq, &(wrq.u.freq) );
	wrq.u.freq.flags = 0x01;	
	
	if( ( fd = socket ( PF_INET, SOCK_DGRAM, IPPROTO_IP ) ) < 0 )
		{
			_smartlinkAttr.LOG( "create iwconfig socket error !!!|n" );
			return -1;
		}
	strncpy ( wrq.ifr_name, ifname, IFNAMSIZ );
	
	if( ( ioctl ( fd, SIOCSIWFREQ, &wrq ) ) == 0 )
		{
			_smartlinkAttr.LOG( "set %s to channel %d success!\n", ifname, ( int )freq );
			close(fd);
			return -1;
		}
	else
		{
			_smartlinkAttr.LOG( "set %s to channel %d failed!\n", ifname, ( int ) freq );
			close ( fd ) ;
			return 0;
		}
}

 /* 设置无线网卡模式，ifname为无线网卡名称，mode值为0-7 对应
	mode{ "Auto","Ad-Hoc","Managed","Master","Repeater","Secondary","Monitor","Unknown/bug" }*/
int set_wlan_mode ( char *ifname, unsigned int mode )
{
	iwreq wrq;	
	int fd;
	
	if( ( fd = socket ( PF_INET, SOCK_DGRAM, IPPROTO_IP ) ) < 0 )
	{
		_smartlinkAttr.LOG( "create iwconfig socket error !!!|n" );
		return -1;
	}
	
	wrq.u.mode = mode;
	strncpy ( wrq.ifr_name, ifname, IFNAMSIZ );
	if ( ( ioctl ( fd, SIOCSIWMODE, &wrq ) ) != 0 )
	{
		_smartlinkAttr.LOG( "ioctl  SIOCSIWMODE %d error ", wrq.u.mode );
		close ( fd ) ;
		return -1;
	}	
	close ( fd ) ;
	return 0;
}


/* 设置无线网卡为monitor模式，ifname为无线网卡设备名称 */
int set_wlan_mode_monitor ( char *ifname )
{
	if ( set_wlan_mode ( ifname, 6 ) == 0 )
	{
		_smartlinkAttr.LOG( "set %s mode monitor success!\n", ifname );
		return 0;
	}		
	else 
	{
		_smartlinkAttr.LOG ( "set %s mode monitor fail!\n", ifname );
		return -1;
	}
} 




/* 设置无线网卡为managed模式，ifname为无线网卡设备名称 */
int set_wlan_mode_managed ( char *ifname )
{
	if ( set_wlan_mode ( ifname, 2 ) == 0 )
	{
		_smartlinkAttr.LOG ( "set %s mode managed success!\n", ifname );
		return 0;
	}		
	else 
	{
		_smartlinkAttr.LOG ( "set %s mode managed fail!\n", ifname );
		return -1;
	}
}

int set_wlan_mode_ap(char *ifname)
{
	if ( set_wlan_mode ( ifname, 3 ) == 0 )
	{
		_smartlinkAttr.LOG ( "set %s mode AP success!\n", ifname );
		return 0;
	}		
	else 
	{
		_smartlinkAttr.LOG ( "set %s mode AP fail!\n", ifname );
		return -1;
	}
}

int smart_link_rtl8188_init( char *wlan_interface,  fWIFI_MODE_SWITCH do_trap, char *ESSID, emWIFI_MODE mode, bool immediately, smartl_link_print sl_print,
							fWIFI_GET_WIFI_MODE get_wifi_mode_callback, fWIFI_SET_WIFI_MODE set_wifi_mode_callback)
{ 
	struct ifreq buf1;
	struct ifreq ifr;
	int it = 1; 
	int ret = 0;

	if( m_smart_link_thread_id ){	
		return 0;
	}
	
	m_has_scan_product_ap = 0;
	memset(&_smartlinkAttr, 0, sizeof(stSmartlinkAttr));
	_smartlinkAttr.LOG = sl_print; 
	snprintf( smart_link_interface, 30, "%s", wlan_interface );
	_smartlinkAttr.LOG("smart_link_interface=%s\n",smart_link_interface);
	_smartlinkAttr.do_trap = do_trap;
	_smartlinkAttr.get_wifi_mode = get_wifi_mode_callback;
	_smartlinkAttr.set_wifi_mode = set_wifi_mode_callback;
	wireless_mode_before_sl = mode;
	_smartlinkAttr.immediately_to_scan = immediately;
	snprintf ( last_connect_essid, 64, "%s", ESSID );
	_smartlinkAttr.LOG("last_connect_essid=%s\n", last_connect_essid );
	NVR_num = 0;

	packet_pan_dap[0] = 0xff;
	packet_pan_dap[1] = 0xff;
	packet_pan_dap[2] = 0xff;
	packet_pan_dap[3] = 0xff;
	packet_pan_dap[4] = 0xff;
	packet_pan_dap[5] = 0xff;

	beeacon_packet_pan_d[0] = 0x80;
	beeacon_packet_pan_d[1] = 0x00;
	beeacon_packet_pan_d[2] = 0x00;
	beeacon_packet_pan_d[3] = 0x00;
	beeacon_packet_pan_d[4] = 0xff;
	beeacon_packet_pan_d[5] = 0xff;
	beeacon_packet_pan_d[6] = 0xff;
	beeacon_packet_pan_d[7] = 0xff;
	beeacon_packet_pan_d[8] = 0xff;
	beeacon_packet_pan_d[9] = 0xff;

	memset ( smart_link_message_from_host, 0 , SMART_LINK_MESSAGE_LENGTH ) ;
	
    if ( ( m_receive_socket_rtl8188_fd = socket( PF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) ) ) < 0 )
	    {
			_smartlinkAttr.LOG("socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL): %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);  
	        return -1;
	    }  
	
    memset ( &buf1, 0, sizeof(buf1));
	strncpy ( buf1.ifr_name, wlan_interface, sizeof ( buf1.ifr_name ) - 1 );  
	_smartlinkAttr.LOG ( "net device: %s\n", buf1.ifr_name ) ;    
	
    ret = ioctl ( m_receive_socket_rtl8188_fd, SIOCGIFFLAGS, ( char* )&buf1 ) ; 
    if ( ret )   
		{
			_smartlinkAttr.LOG("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);  
    	}
	
	buf1.ifr_flags |= IFF_PROMISC; 		
	ret = ioctl ( m_receive_socket_rtl8188_fd, SIOCSIFFLAGS, ( char* )&buf1 );  
    if ( ret!=0 )   
		{
			_smartlinkAttr.LOG("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
			return -2;
		}		
	
	ret = setsockopt( m_receive_socket_rtl8188_fd, SOL_SOCKET, SO_REUSEADDR, &it, sizeof(it));	
	if ( ret != 0 )
		{
			_smartlinkAttr.LOG( "setsockopt: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__ ) ;
			return -3;
		}			
	
	memset ( &ifr, 0, sizeof ( ifr ) ) ;
	strncpy(ifr.ifr_name, wlan_interface, sizeof(ifr.ifr_name));
	if (ioctl( m_receive_socket_rtl8188_fd, SIOCGIFINDEX, &ifr) == -1)
	{
		_smartlinkAttr.LOG("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
		return -4;
	}  
		
	struct ifreq if_wlan0;
	memset ( &if_wlan0, 0, sizeof ( if_wlan0 ) );
    strncpy ( if_wlan0.ifr_name, wlan_interface, IFNAMSIZ );
    
	if ( setsockopt( m_receive_socket_rtl8188_fd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&if_wlan0, sizeof(if_wlan0)) )
		{
			_smartlinkAttr.LOG("setsockopt: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
			return -5;	
		}

	//set_wlan_mode_monitor( wlan_interface );
	_smartlinkAttr.LOG("enter smart_link_start_thread \n");
	check_wifi_driver_type();
	smart_link_start_thread();
	_smartlinkAttr.LOG("init function end \n");
	return 0;
}

int smart_link_rtl8188_quit( char *wlan_interface )
{
	int ret = -1;
	
	_smartlinkAttr.LOG("enter smart_link_stop_thread \n");

	if( !m_smart_link_thread_id ){
		return 0;	
	}
	
	
	smart_link_stop_thread();
	if(m_receive_socket_rtl8188_fd){
		ret = close( m_receive_socket_rtl8188_fd ) ;
	}
	_smartlinkAttr.LOG("quit function end \n");
	if ( ret != 0 )
		{
			_smartlinkAttr.LOG ( "smart_link_rtl8188_quit faild !!\n" ) ;
			ret = set_wlan_mode_managed ( wlan_interface );
			if ( ret == 0 )
				{
					return -2;
				}
			else
				{
					return -3;
				}			
		}
	
	if(mwpa_flag == 0){
	#ifdef P3_SMART_LINK
		ret = 0;
	#else
		ret = set_wlan_mode_managed ( wlan_interface );
	#endif
		_smartlinkAttr.attr.status = SMARTLINK_STATUS_RECEIVE_PACKET_TIMEOUT;
		if(_smartlinkAttr.do_trap){
			_smartlinkAttr.do_trap(&_smartlinkAttr.attr);
		}
		mwpa_flag = 1;
	}
	m_is_running = false;
	if ( ret == 0 )
		{
			return 0;
		}
	else
		{
			return -1;
		}	
}

// 返回0 表示抓包成功
int smart_link_rtl8188 ( int smart_config_time )
{   
	int start_time,end_time;	
	struct timeval tv;
	struct timezone tz;
	int ap_flag=-1;
	int ap_length=-1; 
	int ii,b,i;
	char ap[100];
	int packet_tou=0;
	int packet_tou1=0;
	int packet_tou2=0;	
	int packet_tou3=0;
	int packet_tou4=0;
	int splen=0;
	int splen1 = -1;
	int splen2 = -2;
	int splen3 = -3;
	int splen4 = -4;
	int ret=0;
	int rets = 0;
	int baotou = 0;
	bool from_ack_packet;

	int packet_data[100];
	char packet_flags[100];
	char packet_flags_pan[100];
	memset(packet_flags_pan,121,100);
	memset(packet_flags,0,100);
	int packet_get_num=0;
	int data_shi;
	int data_ge;
	int pck=0;
	int ap_message_location=0;
	int packet_data_location=0;
	int buffer_differ = 0;
	char mac_of_src[6];
	
	while ( baotou != 4  )
	{
start:
		ret = recvfrom ( m_receive_socket_rtl8188_fd ,
			m_receive_rtl8188_buffer ,sizeof( m_receive_rtl8188_buffer ), 0, NULL, NULL ); 
		if ( m_receive_rtl8188_buffer[0] == 0x00 && m_receive_rtl8188_buffer[1] == 0x00 && m_receive_rtl8188_buffer[2] != 0x00 ) {
				 buffer_tou = (int)m_receive_rtl8188_buffer[2];
				 buffer_differ =  buffer_tou - 18 ;
				 ret -= buffer_differ ;
		}		
		rets = ret - 2 ;

		if ( baotou == 0 )	//get the baotou	
		{	
			gettimeofday ( &tv, &tz );
			start_time = tv.tv_sec;
meibao:
			ret = recvfrom ( m_receive_socket_rtl8188_fd , m_receive_rtl8188_buffer, sizeof( m_receive_rtl8188_buffer ), 0, NULL, NULL );	
			if ( m_receive_rtl8188_buffer[0] == 0x00 && m_receive_rtl8188_buffer[1] == 0x00 && m_receive_rtl8188_buffer[2] != 0x00 ) {
					 buffer_tou = (int)m_receive_rtl8188_buffer[2];
					 buffer_differ =  buffer_tou - 18 ;
					 ret -= buffer_differ ;
			}			
			rets = ret - 2 ;
			
			if ( ( ret == 374 ) || ( ret == 439 ) )
			{	
				for ( b = 0 ;b < 6; b ++ ) //all ap packet
				{
					packet_cun_dap[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
				}
				if ( strcmp ( packet_pan_dap, packet_cun_dap) == 0 )
				{		
					if ( ret == 374 && ( packet_tou1 != 1 )  )
					{
						for( b = 0; b < 18; b ++ ) 
						{							
							packet_cun_dsap1[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
						}
						for( b = 0; b < 6; b ++ ) 
						{							
							packet_cun_dack_1[b] = m_receive_rtl8188_buffer[ buffer_tou + 4 + 6 + b];
							packet_cun_dack_1[6 + b] = m_receive_rtl8188_buffer[ buffer_tou + 4 + 6 + 6 + b];
							packet_cun_dack_1[12 + b] = m_receive_rtl8188_buffer[ buffer_tou + 4 + b];
						}
						packet_tou1 = 1;
						packet_tou ++ ;
					}	
					
					if ( ret == 439 && ( packet_tou2 != 1 )  )
					{
						for( b = 0; b < 18; b ++ ) 
						{							
							packet_cun_dsap2[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
						}
						for( b = 0; b < 6; b ++ ) 
						{							
							packet_cun_dack_2[b] = m_receive_rtl8188_buffer[ buffer_tou + 4 + 6 + b];
							packet_cun_dack_2[6 + b] = m_receive_rtl8188_buffer[ buffer_tou + 4 + 6 + 6 + b];
							packet_cun_dack_2[12 + b] = m_receive_rtl8188_buffer[ buffer_tou + 4 + b];
						}
						packet_tou2 = 1;
						packet_tou ++ ;
					}					
				}			
			}	

			if ( ( rets == 374 ) || ( rets == 439 ) )
			{	
				if ( rets == 374 && ( packet_tou3 != 1 )  )
				{
					for( b = 0; b < 18; b ++ ) 
					{							
						packet_cun_dack_1[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
					}	
					for( b = 0; b < 6; b ++ ) 
					{							
						packet_cun_dsap1[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + 6 + 6 + b];
						packet_cun_dsap1[6 + b] = m_receive_rtl8188_buffer[buffer_tou + 4  + b];
						packet_cun_dsap1[12 + b] = m_receive_rtl8188_buffer[buffer_tou + 4 + 6 + b];
					}
					packet_tou3 = 1;
					packet_tou ++ ;
				}	
				
				if ( rets == 439 && ( packet_tou4 != 1 )  )
				{
					for( b = 0; b < 18; b ++ ) 
					{							
						packet_cun_dack_2[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
					}	
					for( b = 0; b < 6; b ++ ) 
					{							
						packet_cun_dsap2[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + 6 + 6 + b];
						packet_cun_dsap2[6 + b] = m_receive_rtl8188_buffer[buffer_tou + 4  + b];
						packet_cun_dsap2[12 + b] = m_receive_rtl8188_buffer[buffer_tou + 4 + 6 + b];
					}
					packet_tou4 = 1;
					packet_tou ++ ;
				}		
			}

			if ( packet_tou == 2 )
			{
				if ( strcmp ( packet_cun_dack_1, packet_cun_dack_2 ) == 0 )
				{
				  	int bssid_loc;
					for( b = 0; b < 18; b ++ ) 
					{
						packet_cun_dack[b] = packet_cun_dack_1[b];
					}
					for( b = 0; b < 18; b ++ ) 
					{
						packet_cun_dsap[b] = packet_cun_dsap1[b];
					}
					baotou = 1;
					_smartlinkAttr.attr.status = SMARTLINK_STATUS_START_TO_RECEIVE_PACKEET;
					if(_smartlinkAttr.do_trap){
						_smartlinkAttr.do_trap(&_smartlinkAttr.attr);
					}
					
					for ( b = 0 ; b < 100 ; b ++ ) {
						int mac_count = 0;
						for ( bssid_loc = 0; bssid_loc < 6; bssid_loc ++ ) {
							if ( swe.bssid[b][bssid_loc] != packet_cun_dack[bssid_loc] ) {
								break;
							}
							mac_count ++;
						}
						
						if ( mac_count == 6 ) {
							if(current_wifi_driver_type == RTL8188EUS){
								reset_rtl8188eu_monitor_channel( swe.bssid_channel[b]);
							}else if(current_wifi_driver_type == RTL8188FU){
								reset_rtl8188fu_monitor_channel( swe.bssid_channel[b]);
							}
							goto printf_envirement ;
						}
 					}
printf_envirement:
					printf("get the baotou ,the bssid of AP is	%02x:%02x:%02x:%02x:%02x:%02x\n",packet_cun_dack[0], packet_cun_dack[1],
												packet_cun_dack[2],packet_cun_dack[3],packet_cun_dack[4],packet_cun_dack[5]); 
					printf("the bssid of sender is	%02x:%02x:%02x:%02x:%02x:%02x\n",packet_cun_dack[6],packet_cun_dack[7],
												packet_cun_dack[8],packet_cun_dack[9],packet_cun_dack[10],packet_cun_dack[11]); 
					//mac of src
					for( b = 0; b < 6; b ++){
					
					//printf("%02x:%02x:%02x:%02x:%02x:%02x \n", packet_cun_dack[6],packet_cun_dack[7],
						//						packet_cun_dack[8],packet_cun_dack[9],packet_cun_dack[10],packet_cun_dack[11]);
						mac_of_src[b] = packet_cun_dack[0 + b];
					}
					
					printf("the destination is	%02x:%02x:%02x:%02x:%02x:%02x\n",packet_cun_dack[12],packet_cun_dack[13],
												packet_cun_dack[14],packet_cun_dack[15],packet_cun_dack[16],packet_cun_dack[17]); 
					_smartlinkAttr.LOG("stay in this channel %d second ,waiting the packet....\n" , SMART_LINK__INTERRUPT_TIME);
					goto start;
				}		
				packet_tou = 0;
				packet_tou1 = 0;
				packet_tou2 = 0;					
			}
			
			gettimeofday ( &tv, &tz );
			end_time = tv.tv_sec;
			if ( ret > 55 ) // scan ap from beacon packet
			{
				for ( b = 0; b < 10; b ++ )
				{
					beeacon_packet_cun_d[b] = m_receive_rtl8188_buffer[18 + b];
				}
				if ( strcmp ( beeacon_packet_pan_d, beeacon_packet_cun_d ) == 0 )
				{				
					ap_flag = (int)*( m_receive_rtl8188_buffer + 18 + 36 );
					if ( ap_flag == 0x00)
					{
						ap_length = (int)*( m_receive_rtl8188_buffer + 18 + 37 );
						if ( ap_length != 0 )
						{
							memset ( ap ,0, 100 );
							for ( ii = 0; ii < ap_length && ii < 98; ii ++ )
							{
								ap[ii] = m_receive_rtl8188_buffer[18 + 38 + ii];
							}							
							ap[ii + 1] = 0;
							//printf("ap is %s\n packet length is %d\n", ap, ret);
							/*
							if ( !strcmp( ap, last_connect_essid) )
							{
								_smartlinkAttr.LOG(" get the beacon packet of %s\n",ap );											
								memset ( &smart_link_all_ap_essid, 0 , sizeof ( st_all_ap_essid ) );
								memset ( smart_link_message_from_host, 0 , SMART_LINK_MESSAGE_LENGTH );
								return 0;
							}*/
							for ( b = 0; b < 100; b ++ )
							{
								if ( smart_link_all_ap_essid.flags[b] == 1 )
								{
									if ( strcmp ( ap, smart_link_all_ap_essid.essid[b] ) == 0 ) 
										{
											break;
										}
								}
							}
							if ( b == 100 )
							{
								for ( b = 0; b < 100; b ++ )
								{
									if ( smart_link_all_ap_essid.flags[b] == 0 )
									{
										
										for ( i = 0; i < strlen ( ap ); i ++ )
										{
											smart_link_all_ap_essid.essid[b][i] = ap[i];
										}
										smart_link_all_ap_essid.essid[b][i+1] = 0;									
										smart_link_all_ap_essid.flags[b] = 1;
										break;										
									}
								}
							}							
						}
					}						
				}
			}
			if ( ( end_time - start_time ) > smart_config_time )
			{
				baotou = 4;
				return -1;
				goto start;
			}
			goto meibao;
		}
			
				 
		if ( baotou == 2 )			 // get the total number of packet should receive
		{	
			if ( ( ret > 440 ) && ( ret < 500 ) )
			{
				for ( b = 0; b < 18; b ++ )
				{
					packet_cun_dsap1[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
				}
				if ( strcmp ( packet_cun_dsap1, packet_cun_dsap ) == 0 )
				{				
					splen2 = ( ret - 439 ) * 2; 
				}
			}
	
			if ( ( ret > 1400 ) && ( ret < 1480 ) )
			{
				for ( b = 0; b < 18; b ++ )
				{
					packet_cun_dsap2[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
				}
				if ( strcmp ( packet_cun_dsap2, packet_cun_dsap ) == 0 )
				{				
					splen1 = ( ret - 1399 ) * 2;
				}
			}

			if ( ( rets > 440 ) && ( rets < 500 ) )
			{
				for ( b = 0; b < 18; b ++ )
				{
					packet_cun_dack_1[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
				}
				if ( strcmp ( packet_cun_dack_1, packet_cun_dack) == 0 )
				{				
					splen3 = ( rets - 439 ) * 2;
				}
			}
	
			if ( ( rets > 1400 ) && ( rets < 1480 ) )
			{
				for ( b = 0; b < 18; b ++ )
				{
					packet_cun_dack_2[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
				}
				if ( strcmp ( packet_cun_dack_2, packet_cun_dack ) == 0 )
				{					
					splen4 = ( rets - 1399 ) * 2;			
				}
			}
	
			if ( splen2 == splen1 )
			{
				splen = splen1;
				from_ack_packet = 0;
				//no ssid and password
				if( splen == 22 ){
					printf("NVR is default essid and password");
					
					char NVR_essid[16];
					char NVR_pwd[16];
					
					sprintf( NVR_essid, "NVR%02x%02x%02x%02x%02x%02x", mac_of_src[0], mac_of_src[1],
						mac_of_src[2], mac_of_src[3], mac_of_src[4], mac_of_src[5] );
					printf( "NVR ssid is %s\n", NVR_essid );
					
					snprintf( NVR_pwd, 9, "%d%d%d%d%d%d%d%d", mac_of_src[0]%10==0?7:mac_of_src[0]%10, ((mac_of_src[0]&mac_of_src[3])+20)%10, 
						((mac_of_src[1]&mac_of_src[3])+13)%10, mac_of_src[3]%10, mac_of_src[2]%10, ((mac_of_src[4]&mac_of_src[2])+10)%10, ((mac_of_src[5]&mac_of_src[2])+18)%10, mac_of_src[5]%10);
					memset ( smart_link_message_from_host, 0 , SMART_LINK_MESSAGE_LENGTH );
					sprintf( smart_link_message_from_host, "20#%s#&%s#&WNVR", NVR_essid, NVR_pwd );
					
					return 0;
					
				}
				
				_smartlinkAttr.LOG("all packet _num =%d\n", splen );
				baotou = 3;
			}

			if ( splen3 == splen4 )
			{
				splen = splen3;
				from_ack_packet = 1;
				_smartlinkAttr.LOG("all packet _num =%d\n", splen );
				baotou = 3;
			}

			gettimeofday ( &tv, &tz );
			end_time = tv.tv_sec;
			if ( ( end_time - start_time ) > SMART_LINK__INTERRUPT_TIME )
			{
				baotou = 4 ;
				return -2;
			}
		}

		if ( baotou == 3 )	// get the message	packet from host 
		{
			if ( from_ack_packet == 0 )
			{
				if ( ( ret > 499 ) && ( ret < 1420 ) )
				{
					for ( b = 0; b < 18; b ++ )
					{
						packet_cun_dsap2[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
					} 
					if(strcmp(packet_cun_dsap2,packet_cun_dsap)==0)
					{
						ret -= 500;
						data_shi = ret / 10;
						data_ge = ret % 10;
						if( packet_flags[data_shi] != 121 )
						{
							pck ++;
							if( packet_flags[data_shi]==1)
							{
								if(packet_data[data_shi]==data_ge)
								{
									packet_flags[data_shi]=121;
									packet_data[data_shi]=data_ge;
									packet_get_num++;								
									_smartlinkAttr.LOG("get  %d  packet \n",packet_get_num);
									
									if ( packet_get_num == splen )
									{
										baotou = 4;
										for ( packet_data_location = 0; packet_data_location < splen; packet_data_location += 2 )
										{
											smart_link_message_from_host[ap_message_location] = packet_data[packet_data_location] * 10 + packet_data[packet_data_location + 1] + 32;
											ap_message_location ++;
										}
										smart_link_message_from_host[splen / 2] = 0;									
										int b;
										for(b=0;b<100;b++)
										{
											if(smart_link_all_ap_essid.flags[b]==1)
											{
												_smartlinkAttr.LOG("smart_link_all_ap_essid[%d]=%s\n",b,smart_link_all_ap_essid.essid[b]);			
											}
										}
										_smartlinkAttr.LOG("smart_link_message_from_host=%s\n",smart_link_message_from_host);
										return 0;
									}
								}
								else 
								{
									packet_data[data_shi]=0;
									packet_flags[data_shi]=0;
								}
							}
							if( packet_flags[data_shi]==0)
							{
								packet_data[data_shi]=data_ge;
								packet_flags[data_shi]=1;
							}					
						}	
					}
				}
			}

			if ( from_ack_packet == 1)
			{
				if ( ( rets > 499 ) && ( rets < 1420 ) )
				{
					for ( b = 0; b < 18; b ++ )
					{
						packet_cun_dack_2[b] = m_receive_rtl8188_buffer[buffer_tou + 4 + b];
					} 
					if( strcmp( packet_cun_dack_2,packet_cun_dack ) == 0 )
					{
						rets -= 500;
						data_shi = rets / 10;
						data_ge = rets % 10;
						if( packet_flags[data_shi] != 121 )
						{
							pck++;
							if( packet_flags[data_shi] == 1)
							{
								if(packet_data[data_shi]==data_ge)
								{
									packet_flags[data_shi]=121;
									packet_data[data_shi]=data_ge;
									packet_get_num++;
									_smartlinkAttr.LOG("get  %d  packet( plus ac)  \n",packet_get_num); 
									if ( packet_get_num == splen )
									{
										baotou = 4;
										for ( packet_data_location = 0; packet_data_location < splen; packet_data_location += 2 )
										{
											smart_link_message_from_host[ap_message_location] = packet_data[packet_data_location] * 10 + packet_data[packet_data_location + 1] + 32;
											ap_message_location ++;
										}
										smart_link_message_from_host[splen / 2] = 0;									
										int b;
										for(b=0;b<100;b++)
										{
											if(smart_link_all_ap_essid.flags[b]==1)
											{
												_smartlinkAttr.LOG("smart_link_all_ap_essid[%d]=%s\n",b,smart_link_all_ap_essid.essid[b]);			
											}
										}
										_smartlinkAttr.LOG("smart_link_message_from_host=%s\n",smart_link_message_from_host);
										return 0;
									}
								}
								else 
								{
									packet_data[data_shi]=0;
									packet_flags[data_shi]=0;
								}
							}
							if( packet_flags[data_shi]==0)
							{
								packet_data[data_shi]=data_ge;
								packet_flags[data_shi]=1;
							}					
						}	
					}
				}
			}
			gettimeofday ( &tv, &tz );
			end_time = tv.tv_sec;
			if ( ( end_time - start_time ) > SMART_LINK__INTERRUPT_TIME )
			{
				baotou = 4 ;
				return -2;
			}
		}		

		if ( ret > 55 ) // scan ap from beacon packet
		{
			for ( b = 0; b < 10; b ++ )
			{
				beeacon_packet_cun_d[b] = m_receive_rtl8188_buffer[18 + b];
			}
			if ( strcmp ( beeacon_packet_pan_d, beeacon_packet_cun_d ) == 0 )
			{				
				ap_flag = (int)*( m_receive_rtl8188_buffer + 18 + 36 );
				if ( ap_flag == 0x00)
				{
					ap_length = (int)*( m_receive_rtl8188_buffer + 18 + 37 );
					if ( ap_length != 0 )
					{
						memset ( ap ,0, 100 );
						for ( ii = 0; ii < ap_length && ii < 98; ii ++ )
						{
							ap[ii] = m_receive_rtl8188_buffer[18 + 38 + ii];
						}
						
						ap[ii + 1] = 0;
						for ( b = 0; b < 100; b ++ )
						{
							if ( smart_link_all_ap_essid.flags[b] == 1 )
							{
								if ( strcmp ( ap, smart_link_all_ap_essid.essid[b] ) == 0 ) 
									{
										break;
									}
							}
						}
						if ( b == 100 )
						{
							for ( b = 0; b < 100; b ++ )
							{
								if ( smart_link_all_ap_essid.flags[b] == 0 )
								{
									
									for ( i = 0; i < strlen ( ap ); i ++ )
									{
										smart_link_all_ap_essid.essid[b][i] = ap[i];
									}
									smart_link_all_ap_essid.essid[b][i+1] = 0;									
									smart_link_all_ap_essid.flags[b] = 1;
									break;										
								}
							}
						}							
					}
				}						
			}
		}				
					
		if ( baotou == 1 )	 // handle the time
		{
			gettimeofday ( &tv, &tz );
			start_time = tv.tv_sec;
			baotou = 2;
		}
	}  
	return -1;
}

int smart_link_status()
{
	return m_is_running;
}

int nk_smart_link_if_has_product_AP()
{
	return m_has_scan_product_ap;
}

/* N1协议的smartconfig，当无线网卡采用7601驱动时，利用此函数获取NVR无线对码发出的包 
   ifname为无线网卡设备名称，AP_ESSID存储nvr所发送的essid信息，AP_PSK存储nvr所发送的密码信息
   smart_config_time为ipc端进行无线对码的时间（以秒为单位），paae 存储在未对码成功时扫描到的
   ap信息，对码成功后自动退出此函数返回0，函数执行失败或对码失败后返回值!0*/ 
   
 int SmartConfig_MTK7601 (char *ifname,char *ap_message,int smart_config_time,pst_all_ap_essid paae)
{   
	int start_time,end_time;	
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	start_time=(int)tv.tv_sec;
	int fd;    
    struct ifreq buf1;
    int ret = 0;  
    if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        perror("socket(PF_PACKET, SOCK_RAW, 768)");
        close(fd);
        return -1;
    }  
    memset(&buf1,0,sizeof(buf1));
	strncpy(buf1.ifr_name,ifname,sizeof(buf1.ifr_name)-1);  
	printf("net device: %s\n", buf1.ifr_name);     
    ret = ioctl(fd, SIOCGIFFLAGS, (char*)&buf1); 
    if(ret)          printf("ioctl SIOCGIFFLAGS error: %d\n", ret); 
	buf1.ifr_flags|=IFF_PROMISC; 		
	ret = ioctl(fd, SIOCSIFFLAGS, (char*)&buf1);  
    if(ret)          printf("ioctl SIOCSIFFLAGS error : %d\n", ret); 
	int it=1;
	ret=setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &it, sizeof(it));		 
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
	{
		printf("ioctl  SIOCGIFINDEX error !!!\n");
        close(fd);
		return -2;
	}  
	
	struct ifreq if_wlan0;
	memset(&if_wlan0,0,sizeof(if_wlan0));
    strncpy(if_wlan0.ifr_name, ifname, IFNAMSIZ);
    
	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
            (char *)&if_wlan0, sizeof(if_wlan0)) )
		{
			printf("SO_BINDTODEVICE error \n");
            close(fd);
			return -3;	
		}
	
	char recv_buf[10240];
	char *packet_pan_d;
	char *packet_cun_d;
	char *packet_cun_d1;
	char *packet_cun_ds;
	char *packet_cun_ds1;
	char *packet_cun_ds2;
	char *packet_pan_ds;
	packet_pan_d=calloc(10,1);
	packet_cun_d=calloc(10,1);
	packet_cun_d1=calloc(10,1);
	packet_cun_ds=calloc(16,1);
	packet_cun_ds1=calloc(16,1);
	packet_cun_ds2=calloc(16,1);
	packet_pan_ds=calloc(16,1);
	*(packet_pan_d)=0x08;
	*(packet_pan_d+1)=0x42;
	*(packet_pan_d+2)=0x00;
	*(packet_pan_d+3)=0x00;
	*(packet_pan_d+4)=0xff;
	*(packet_pan_d+5)=0xff;
	*(packet_pan_d+6)=0xff;
	*(packet_pan_d+7)=0xff;
	*(packet_pan_d+8)=0xff;
	*(packet_pan_d+9)=0xff;
	char *beeacon_packet_pan_d;
	char *beeacon_packet_cun_d;
	beeacon_packet_pan_d=calloc(10,1);
	beeacon_packet_cun_d=calloc(10,1);
	*(beeacon_packet_pan_d)=0x80;
	*(beeacon_packet_pan_d+1)=0x00;
	*(beeacon_packet_pan_d+2)=0x00;
	*(beeacon_packet_pan_d+3)=0x00;
	*(beeacon_packet_pan_d+4)=0xff;
	*(beeacon_packet_pan_d+5)=0xff;
	*(beeacon_packet_pan_d+6)=0xff;
	*(beeacon_packet_pan_d+7)=0xff;
	*(beeacon_packet_pan_d+8)=0xff;
	*(beeacon_packet_pan_d+9)=0xff;
	int ap_flag=-1;
	int ap_length=-1; 
	int ii,b,i;
	char ap[100];
	int packet_tou=0;
	int packet_tou1=0;
	int packet_tou2=0;
	
	while(packet_tou!=2)
	{
		ret=recvfrom(fd,recv_buf,sizeof(recv_buf),0,NULL,NULL);	
		
		if(ret==496&&(packet_tou1!=1))
		{	
			for(b=0;b<10;b++)//packet_cun_d=recv_buf+18;
			{
				*(packet_cun_d+b)=*(recv_buf+144+b);
			}
			if(strcmp(packet_pan_d,packet_cun_d)==0)
			{				
				for(b=0;b<16;b++) //packet_cun_ds1=recv_buf+18;
				{
					*(packet_cun_ds1+b)=*(recv_buf+144+b);
				}
				packet_tou1=1;
				packet_tou++;
				if(packet_tou==2)
				{
					if(strcmp(packet_cun_ds1,packet_cun_ds2)==0)
					{
						for(b=0;b<16;b++) //packet_cun_ds=packet_cun_ds1;
						{
							*(packet_cun_ds+b)=*(packet_cun_ds1+b);
						} 
					}
					else
					{
						packet_tou=0;
						packet_tou1=0;
						packet_tou2=0;
					}	
				}
			}
		}
		
		if(ret==(439+122)&&(packet_tou2!=1))
		{	
			for(b=0;b<10;b++)  //packet_cun_d1=recv_buf+18;
			{
				*(packet_cun_d1+b)=*(recv_buf+18+126+b);
			}
			if(strcmp(packet_pan_d,packet_cun_d1)==0)
			{	
				for(b=0;b<16;b++) //packet_cun_ds2=recv_buf+18;
				{
					*(packet_cun_ds2+b)=*(recv_buf+18+126+b);
				}
				packet_tou2=1;
				packet_tou++;
				if(packet_tou==2)
				{
					if(strcmp(packet_cun_ds1,packet_cun_ds2)==0)
					{						
						for(b=0;b<16;b++)//packet_cun_ds=packet_cun_ds1;
						{
							*(packet_cun_ds+b)=*(packet_cun_ds1+b);
						}
					}
					else
					{
						packet_tou=0;
						packet_tou1=0;
						packet_tou2=0;						
					}				
				}
			}
		}

		if(ret>55+126)
		{
			for(b=0;b<10;b++)
			{
				*(beeacon_packet_cun_d+b)=*(recv_buf+144+b);
			}
			if(strcmp(beeacon_packet_pan_d,beeacon_packet_cun_d)==0)
			{				
				
					ap_flag=(int)*(recv_buf+144+36);
					if(ap_flag==0x00)
					{
						ap_length=(int)*(recv_buf+144+37);
						if(ap_length!=0)
						{
							memset(ap,0,100);
							for(ii=0;ii<ap_length;ii++)
							{
								ap[ii]=*(recv_buf+144+38+ii);
							}
							ap[ii+1]=0;
							for(b=0;b<100;b++)
							{
								if(paae->flags[b]==1)
								{
									if(strcmp(ap,paae->essid[b])==0) break;									
								}
							}
							if(b==100)
							{
								for(b=0;b<100;b++)
								{
									if(paae->flags[b]==0)
									{
										
										for(i=0;i<strlen(ap);i++)
										{
											paae->essid[b][i]=ap[i];
										}
										paae->essid[b][i+1]=0;									
										paae->flags[b]=1;
										break;										
									}
								}
							}		
						}
					}		
			}
		}	
		gettimeofday(&tv,&tz);
		end_time=(int)tv.tv_sec;
		if((end_time-start_time)>=smart_config_time) {
            close(fd);
			free(packet_pan_d);
			free(packet_pan_ds);		
			free(packet_cun_ds);
			free(packet_cun_d);
			free(packet_cun_d1);
			free(packet_cun_ds1);
			free(packet_cun_ds2);
			free(beeacon_packet_cun_d);
			free(beeacon_packet_pan_d);
            return -4;
        }
	}	
	printf("get the baotou ,the bssid of AP is  %02x:%02x:%02x:%02x:%02x:%02x\n",*(packet_cun_ds+10),
						*(packet_cun_ds+11),*(packet_cun_ds+12),*(packet_cun_ds+13),*(packet_cun_ds+14),*(packet_cun_ds+15));	
	int splen=0;
	int splen1=-1;
	packet_tou=3;
	while(packet_tou!=5)
	{		
		ret=recvfrom(fd,recv_buf,sizeof(recv_buf),0,NULL,NULL);				
		if(ret>562&&ret<622)
		{			
			for(b=0;b<16;b++)//packet_cun_ds1=recv_buf+18;
			{
				*(packet_cun_ds1+b)=*(recv_buf+18+126+b);
			}
			if(strcmp(packet_cun_ds1,packet_cun_ds)==0)
			{				
				splen=(ret-439-122)*2;				
			}
		}

		if(ret>(1440+122)&&ret<(1500+122))
		{				
			for(b=0;b<16;b++)//packet_cun_ds2=recv_buf+18;
			{
				*(packet_cun_ds2+b)=*(recv_buf+18+126+b);
			}
			if(strcmp(packet_cun_ds2,packet_cun_ds)==0)
			{				
				splen1=(ret-1439-122)*2;				
			}
		}	
		
		if(splen==splen1)
		{
			packet_tou=5;
		}
		
		if(ret>55+126)
		{
			for(b=0;b<10;b++)
			{
				*(beeacon_packet_cun_d+b)=*(recv_buf+144+b);
			}
			if(strcmp(beeacon_packet_pan_d,beeacon_packet_cun_d)==0)
			{				
				
					ap_flag=(int)*(recv_buf+144+36);
					if(ap_flag==0x00)
					{
						ap_length=(int)*(recv_buf+144+37);
						if(ap_length!=0)
						{
							memset(ap,0,100);
							for(ii=0;ii<ap_length;ii++)
							{
								ap[ii]=*(recv_buf+144+38+ii);
							}
							ap[ii+1]=0;
							for(b=0;b<100;b++)
							{
								if(paae->flags[b]==1)
								{
									if(strcmp(ap,paae->essid[b])==0) break;									
								}
							}
							if(b==100)
							{
								for(b=0;b<100;b++)
								{
									if(paae->flags[b]==0)
									{
										
										for(i=0;i<strlen(ap);i++)
										{
											paae->essid[b][i]=ap[i];
										}
										paae->essid[b][i+1]=0;									
										paae->flags[b]=1;
										break;										
									}
								}
							}		
						}
					}		
			}
		}	
		gettimeofday(&tv,&tz);
		end_time=(int)tv.tv_sec;
		if((end_time-start_time)>smart_config_time) {
            close(fd);
			free(packet_pan_d);
			free(packet_pan_ds);		
			free(packet_cun_ds);
			free(packet_cun_d);
			free(packet_cun_d1);
			free(packet_cun_ds1);
			free(packet_cun_ds2);
			free(beeacon_packet_cun_d);
			free(beeacon_packet_pan_d);
            return -5;
        }
	}
	
	printf("The all packet number is %d\n",splen);
	int packet_data[splen];
	char *packet_flags;
	char *packet_flags_pan;
	packet_flags=calloc(splen,1);	
	packet_flags_pan=calloc(splen,1);
	memset(packet_flags_pan,121,splen);
	int packet_get_num=0;
	int data_shi;
	int data_ge;
	while(packet_get_num!=splen)
	{
		ret=recvfrom(fd,recv_buf,sizeof(recv_buf),0,NULL,NULL);
		if(ret>(499+122)&&ret<(1440+122))
		{
			DBG("ret_chu_shi=%d\n",ret);
			 for(b=0;b<16;b++)
			{
				*(packet_cun_ds2+b)=*(recv_buf+18+126+b);
			} 
			if(strcmp(packet_cun_ds2,packet_cun_ds)==0)
			{
				ret=ret-122-500;
				data_shi=ret/10;
				data_ge=ret%10;
				if(*(packet_flags+data_shi)!=121)
				{
					if(*(packet_flags+data_shi)==1)
					{
						if(packet_data[data_shi]==data_ge)
						{
							*(packet_flags+data_shi)=121;
							packet_data[data_shi]=data_ge;
							packet_get_num++;
							printf("get  %d  packet  \n",packet_get_num);
						}
						else 
						{
							packet_data[data_shi] = 0;
							*(packet_flags + data_shi) = 0x00;
						}
					}
					if(*(packet_flags+data_shi)==0)
					{
						packet_data[data_shi]=data_ge;
						*(packet_flags+data_shi)=1;
						DBG("get half packet !!\n");
					}					
				}	
			}
		}
		gettimeofday(&tv,&tz);
		end_time=(int)tv.tv_sec;
		if((end_time-start_time)>smart_config_time) {
            close(fd);
			free(packet_flags);	
			free(packet_pan_d);
			free(packet_pan_ds);		
			free(packet_cun_ds);
			free(packet_cun_d);
			free(packet_cun_d1);
			free(packet_flags_pan);
			free(packet_cun_ds1);
			free(packet_cun_ds2);
			free(beeacon_packet_cun_d);
			free(beeacon_packet_pan_d);
            return -7;
        }
	}
	gettimeofday(&tv,&tz);
	end_time=(int)tv.tv_sec;
	printf("smart_config_time=%d\n",end_time-start_time);
#ifdef DEBGU
	int DBG_k;
	for(DBG_k=0;DBG_k<splen;DBG_k++)
	{
		DBG(" %d ",packet_data[DBG_k]);
	}
	DBG("\n");
#endif	
	free(packet_flags);	
	free(packet_pan_d);
	free(packet_pan_ds);		
	free(packet_cun_ds);
	free(packet_cun_d);
	free(packet_cun_d1);
	free(packet_flags_pan);
	free(packet_cun_ds1);
	free(packet_cun_ds2);
	free(beeacon_packet_cun_d);
	free(beeacon_packet_pan_d);
	int essid_location=0;
	int packet_data_location=0;
	
	for(packet_data_location=0;packet_data_location<splen;packet_data_location+=2)
	{
		*(ap_message+essid_location)=packet_data[packet_data_location]*10+packet_data[packet_data_location+1]+32;
		essid_location++;
		DBG("essid_location=%d\n",essid_location);
	}
	*(ap_message+splen/2)=0;

    close(fd);
	return 0;      
}  


#ifdef  SMARTLINK_SLAVE 
static void* wifi_mode_switch(lpWifiAttr attr){
	void *ret = 0;;
	if(attr){
		printf("%s:%d, essid:%s  psw:%s  mode:%d  dhcp:%d token=%s\n",
			__FUNCTION__, __LINE__, attr->essid, attr->password, attr->mode, attr->dhcp,attr->token);
	}
	else{
		printf("%s:%d need to parse the protocal:smart_link_message_from_host[]!\n",
			__FUNCTION__, __LINE__);
	}
	return ret;
}


 int  main() 
{
	char *ifname="wlan0";
	smartl_link_print sl_print;
	sl_print = print_log ;
	emWIFI_MODE device_wireless_mode =  SMARTLINK_WIFI_MODE_STATION;
	g_init_mode = 0;
	smart_link_rtl8188_init( ifname, wifi_mode_switch, "NVR123bb", device_wireless_mode, 0, sl_print );
	getchar();
	smart_link_rtl8188_quit( ifname );
	return 0;
} 
#endif



