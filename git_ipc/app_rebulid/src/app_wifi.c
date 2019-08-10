#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <setjmp.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include "app_debug.h"
#include "netsdk.h"
#include "sdk/sdk_api.h"
#include "sensor.h"
#include "wifi/ja_smart_link.h"
#include "wifi/ja_wifi_seek.h"
#include "sound.h"
#include "log.h"
#include "bsp/keytime.h"
#include "wifi/ja_wifi_seek.h"
#include <base/ja_process.h>
#include "ipcam_network.h"
#include "global_runtime.h"
#include "app_wifi.h"
#include "ifconf.h"
#include "wifi/wifi.h"
#include "base/ja_process.h"

#define PING_TIMES_MAX       3
#define PING_TIME_OUT     100
#define PACKET_SIZE     1500
#define RSSI_LEVEL1      88
#define RSSI_LEVEL2      70
#define BPS_LEVEL_MAX  7
struct wifi_link_status
{
    int timeout_count;
    int timein_count;
    int wifi_conneted;
    int signal_strength;
    int last_level;
    int constantBitRate; 
    int resolution;
    int last_signal_strength;
    int signal_equal_count;
    char wifi_chip[12];
    char wlan_gateway[64];  
    int (*WifiGetResolution)(struct wifi_link_status* const thiz, ST_NSDK_VENC_CH* pvenc_ch);

};

#if 0
typedef struct near_ap{
	int num;
	stAP_info ap_info[50];
}ST_WIFI_NEAR_AP, *LP_WIFI_NEAR_AP;

static ST_WIFI_NEAR_AP gs_wifi_near_ap = {0};
#endif

static struct wifi_link_status  wifi_stat = {0,0,1,0,1,0,0,0,0,"",""};
int model_type = 0;

static unsigned short icmp_cal_chksum(unsigned short *addr, int len)
{
	int nleft=len;
	int sum=0;
	unsigned short *w=addr;
	unsigned short answer=0;
	while(nleft>1)
	{
		sum+=*w++;
		nleft-=2;
	}
	if( nleft==1)
	{
		*(unsigned char *)(&answer)=*(unsigned char *)w;
		sum+=answer;
	}
	sum=(sum>>16)+(sum&0xffff);
	sum+=(sum>>16);
	answer=~sum;
	return answer;
}

static int icmp_pack(void* _buf, int echo_seq)
{
    int packsize;
    int datalen = 56;
    struct icmp *icmp;
    struct timeval *tval;
    icmp=(struct icmp*)_buf;
    icmp->icmp_type=ICMP_ECHO;
    icmp->icmp_code=0;
    icmp->icmp_cksum=0;
    icmp->icmp_seq=echo_seq;
    icmp->icmp_id=getpid();
    packsize = 8 + datalen;
    tval= (struct timeval *)icmp->icmp_data;
    gettimeofday(tval,NULL);
    icmp->icmp_cksum=icmp_cal_chksum( (unsigned short *)icmp,packsize);

    return packsize;
}

static int icmp_unpack(char *buf, int len, int echo_seq)
{
    int pklen;
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    ip=(struct ip *)buf;
    iphdrlen=ip->ip_hl<<2;
    icmp=(struct icmp *)(buf+iphdrlen);
    pklen = len -iphdrlen;
    if(pklen<8)
    {
        printf("ICMP packets\'s length is less than 8\n");
        return -1;
    }
 
    if( (icmp->icmp_type==ICMP_ECHOREPLY) && (icmp->icmp_id==getpid()) && 
    (icmp->icmp_seq==echo_seq))
    {
        return 0;
    }
    else
    {
        //printf("Not my response\n");
        return -1;
    }
}

static int icmp_get_time(const char* ip_str, int ret_time[PING_TIMES_MAX])
{
    int count = PING_TIMES_MAX-1;
    int ping_seq = 0;
    //int is_using = 0;
    int sockfd = -1;
    //char ip_str[32];
    //sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    //sprintf(ip_str, "192.168.1.36");
    //printf("check_ip: %s\r\n", ip_str);



    /*struct protoent* protocol = getprotobyname("icmp");
    if(protocol == NULL)
    {
        printf("getprotobyname error\n");
        break;
    }*/

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP/*protocol->p_proto*/);
    if(sockfd < 0)
    {
        printf("sockfd error, errno=%d\n", errno);
        return -1;
    }

    unsigned long inaddr = (unsigned long)inet_addr(ip_str);
    if(inaddr == INADDR_NONE)
    {
        printf("inet_addr error\n");
        close(sockfd);
        return -1;
    }

    do{
        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(struct sockaddr_in));
        dest_addr.sin_family=AF_INET;
        memcpy(&dest_addr.sin_addr, &inaddr, sizeof(inaddr));

        char sendpacket[PACKET_SIZE];
        int packetsize = icmp_pack(sendpacket, ping_seq);
        int ret = sendto(sockfd, sendpacket, packetsize, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        if(ret != packetsize)
        {
            printf("sendto error, errno=%d\n", errno);
            break;
        }

        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        char recvpacket[PACKET_SIZE];
        int recvlen;

        struct timeval begin_tv;
        gettimeofday(&begin_tv, NULL);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;
        fd_set rfd_set;
        FD_ZERO(&rfd_set);
        FD_SET(sockfd, &rfd_set);
        ret = select(sockfd + 1, &rfd_set, NULL, NULL, &timeout);
        if(ret < 0)
        {
            printf("select error, errno=%d\n", errno);
            break;
        }

        if(ret == 0)
        {
            //printf("select timeout,ping_seq=%d,ret=%d\n",ping_seq,ret);
            ret_time[ping_seq] = 1000;
            //break;
        }

        if(ret > 0 && FD_ISSET(sockfd, &rfd_set))
        {
            recvlen = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from, &fromlen);
            if(recvlen > 0)
            {
                ret = icmp_unpack(recvpacket, recvlen, ping_seq);
                if(ret != -1)
                {
                    struct timeval end_tv;
                    gettimeofday(&end_tv, NULL);

                    struct timeval result_tv;
                    timersub(&end_tv, &begin_tv, &result_tv);

                    int time = result_tv.tv_sec * 1000 + result_tv.tv_usec / 1000;
                    ret_time[ping_seq] = time;
                    //printf("%d bytes from %s: icmp_seq=%d  time=%d ms\n",recvlen, ip_str, ping_seq, time);
                }
                else
                {
                    ret_time[ping_seq] = 1000;
                    //printf("not my pack\n");
                }
            }
        }
        ping_seq ++;
    }while(count--);

    if(sockfd != -1)
    {
        close(sockfd);
    }

    return 0;
}

static int icmp_check_timeout()
{
    int i;
    int ret = 0;
    int icmp_time[PING_TIMES_MAX];
    const char* devip="172.20.14.1";
    
    icmp_get_time(devip, icmp_time);
    //printf("checkip:%s--time:%dms,%dms,%dms.\n",devip,icmp_time[0],icmp_time[1],icmp_time[2]);
    for(i=0; i<PING_TIMES_MAX; i++){
        if( icmp_time[i] > PING_TIME_OUT){
            ret += 1;
        }
     }

    return ret;
}

static int wifi_get_rx_signal()
{
	int ret = 0;
	char file_dir[256];
	char buf[1024] = {0};
	FILE *fp = NULL;
	sprintf(file_dir, "/proc/net/%s/wlan0/rx_signal", wifi_stat.wifi_chip);
	fp = fopen(file_dir, "rb");
	if(NULL != fp){
		fread(buf, sizeof(buf), 1, fp);
		//printf("rx_signal\n%s\n", buf);
		sscanf(buf+9, "signal_strength:%d", &ret);
		//printf("%d\n", ret);
		fclose(fp);
	}
	//printf("\n\n file_dir:%s|(%d)\n\n",file_dir,ret);	
	return ret;
}

static int wifi_get_10mp_resolution(struct wifi_link_status* Pwifi_stat, ST_NSDK_VENC_CH* Pvenc_ch)
{
	int constantBitRate, resolution;
	
	switch(Pwifi_stat->last_level){
	    case 0:
	    	constantBitRate = 1536;
	    	resolution = kNSDK_RES_1280X720;
	    	break;
	    case 1:
	    	constantBitRate = 1280;
	    	resolution = kNSDK_RES_1280X720;
	    	break;        	
	    case 2:
	    	constantBitRate = 1024;
	    	resolution = kNSDK_RES_1280X720;
	    	break;
	    case 3:
	    	constantBitRate = 768;
	    	resolution = kNSDK_RES_1280X720;
	    	break;
	    case 4:
	    	constantBitRate = 512;
	    	resolution = kNSDK_RES_960X576;
	    	break;
	    case 5:
	    	constantBitRate = 384;
	    	resolution = kNSDK_RES_960X576;
	    	break;
	    case 6:
	    	constantBitRate = 256;
	    	resolution = kNSDK_RES_640X360;
	    	break;
	    case 7:
	    	constantBitRate = 128;
	    	resolution = kNSDK_RES_640X360;
	    	break;        	
	    default:
	    	constantBitRate = 1024;
	    	resolution = kNSDK_RES_1280X720;
	    	break;
	}

	if(0 != constantBitRate && constantBitRate != Pwifi_stat->constantBitRate){
		Pwifi_stat->constantBitRate = constantBitRate;
		Pwifi_stat->resolution = resolution;
		Pvenc_ch->constantBitRate = constantBitRate;
		Pvenc_ch->resolution = resolution;
		return 1;
	}

	return 0;
}


static int wifi_get_13mp_resolution(struct wifi_link_status* Pwifi_stat, ST_NSDK_VENC_CH* Pvenc_ch)
{
	int constantBitRate, resolution;
	
	switch(Pwifi_stat->last_level){
	    case 0:
	    	constantBitRate = 2048;
	    	resolution = kNSDK_RES_1280X960;
	    	break;
	    case 1:
	    	constantBitRate = 1536;
	    	resolution = kNSDK_RES_1280X960;
	    	break;        	
	    case 2:
	    	constantBitRate = 1280;
	    	resolution = kNSDK_RES_1280X960;
	    	break;
	    case 3:
	    	constantBitRate = 1024;
	    	resolution = kNSDK_RES_1280X960;
	    	break;
	    case 4:
	    	constantBitRate = 768;
	    	resolution = kNSDK_RES_1280X960;
	    	break;
	    case 5:
	    	constantBitRate = 512;
	    	resolution = kNSDK_RES_720X576;
	    	break;
	    case 6:
	    	constantBitRate = 384;
	    	resolution = kNSDK_RES_640X480;
	    	break;
	    case 7:
	    	constantBitRate = 128;
	    	resolution = kNSDK_RES_640X480;
	    	break;        	
	    default:
	    	constantBitRate = 1280;
	    	resolution = kNSDK_RES_1280X720;
	    	break;
	}

	if(0 != constantBitRate && constantBitRate != Pwifi_stat->constantBitRate){
		Pwifi_stat->constantBitRate = constantBitRate;
		Pwifi_stat->resolution = resolution;
		Pvenc_ch->constantBitRate = constantBitRate;
		Pvenc_ch->resolution = resolution;
		return 1;
	}

	return 0;
}


static int wifi_get_20mp_resolution(struct wifi_link_status* Pwifi_stat, ST_NSDK_VENC_CH* Pvenc_ch)
{
	int constantBitRate, resolution;
	
	switch(Pwifi_stat->last_level){
	    case 0:
	    	constantBitRate = 3072;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;
	    case 1:
	    	constantBitRate = 2560;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;        	
	    case 2:
	    	constantBitRate = 2048;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;
	    case 3:
	    	constantBitRate = 1536;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;
	    case 4:
	    	constantBitRate = 1024;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;
	    case 5:
	    	constantBitRate = 768;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;
	    case 6:
	    	constantBitRate = 512;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;
	    case 7:
	    	constantBitRate = 128;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;        	
	    default:
	    	constantBitRate = 2048;
	    	resolution = kNSDK_RES_1920X1080;
	    	break;
	}

	if(0 != constantBitRate && constantBitRate != Pwifi_stat->constantBitRate){
		Pwifi_stat->constantBitRate = constantBitRate;
		Pwifi_stat->resolution = resolution;
		Pvenc_ch->constantBitRate = constantBitRate;
		Pvenc_ch->resolution = resolution;
		return 1;
	}

	return 0;
}

//for fisheye
static int wifi_get_23mp_resolution(struct wifi_link_status* Pwifi_stat, ST_NSDK_VENC_CH* Pvenc_ch)
{
	//do nothing
	return 0;
}

static int wifi_get_40mp_resolution(struct wifi_link_status* Pwifi_stat, ST_NSDK_VENC_CH* Pvenc_ch)
{
	int constantBitRate, resolution;
	
	switch(Pwifi_stat->last_level){
	    case 0:
	    	constantBitRate = 4096;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;
	    case 1:
	    	constantBitRate = 3072;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;
	    case 2:
	    	constantBitRate = 2560;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;        	
	    case 3:
	    	constantBitRate = 2048;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;
	    case 4:
	    	constantBitRate = 1536;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;
	    case 5:
	    	constantBitRate = 1024;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;
	    case 6:
	    	constantBitRate = 512;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;
	    case 7:
	    	constantBitRate = 128;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;       	
	    default:
	    	constantBitRate = 2560;
	    	resolution = kNSDK_RES_2592X1520;
	    	break;
	}

	if(0 != constantBitRate && constantBitRate != Pwifi_stat->constantBitRate){
		Pwifi_stat->constantBitRate = constantBitRate;
		Pwifi_stat->resolution = resolution;
		Pvenc_ch->constantBitRate = constantBitRate;
		Pvenc_ch->resolution = resolution;
		return 1;
	}

	return 0;
}


//Intelligent adjustment resolution.
int APP_WIFI_calculate_bps(ST_NSDK_VENC_CH* pvenc_ch)
{
	int ret = 0;
	int ret_to = 0;

	//get signal strength.
	wifi_stat.signal_strength = wifi_get_rx_signal();
	if((!wifi_stat.wifi_conneted) || (0 == wifi_stat.signal_strength)){
		//Get a corresponding resolution
		wifi_stat.last_level = 1;
		if(wifi_stat.WifiGetResolution){
			ret = wifi_stat.WifiGetResolution(&wifi_stat, pvenc_ch);
		}
		APP_TRACE("BPS adepter1:rssi(%d)oc(%d)ic(%d)to(%d)bps(%d).\n", 
		wifi_stat.signal_strength, wifi_stat.timeout_count, wifi_stat.timein_count, ret_to, pvenc_ch->constantBitRate);
		return ret;
	}
	//time counter.
	ret_to = icmp_check_timeout();
	if(0 == ret_to){
		wifi_stat.timeout_count = 0;  //out counter reset.
		wifi_stat.timein_count++;
	}else{
		wifi_stat.timein_count = 0;     //in counter reset.
		wifi_stat.timeout_count++;       
	}
	//Up and Down adapter 
	if(wifi_stat.signal_strength >= RSSI_LEVEL1){  //1.signal strenger down faster.
		if(wifi_stat.timeout_count >= 2){
			wifi_stat.timeout_count = 0; //out counter reset.
			wifi_stat.last_level += 1;
			if(wifi_stat.last_level > 3) wifi_stat.last_level = 3; //if too near,Not allowed below  768 
		}
		if(wifi_stat.timein_count >= 20){
			wifi_stat.timein_count = 0;  //in counter reset.
			wifi_stat.last_level -= 1;
		}        
	}else if(wifi_stat.signal_strength >= RSSI_LEVEL2) {  //2.signal weak down slower.
		if(wifi_stat.timeout_count >= 3){
			wifi_stat.timeout_count = 0; //out counter reset.
			wifi_stat.last_level += 1;
			if(wifi_stat.last_level > 6) wifi_stat.last_level = 6; //if mid,Not allowed below  256
		}
		if(wifi_stat.timein_count >= 16){
			wifi_stat.timein_count = 0;  //in counter reset.
			wifi_stat.last_level -= 1;
		}        
	}else{
		if(wifi_stat.timeout_count >= 5){
			wifi_stat.timeout_count = 0; //out counter reset.
			wifi_stat.last_level += 1;
		}
		if(wifi_stat.timein_count >= 16){
			wifi_stat.timein_count = 0;  //in counter reset.
			wifi_stat.last_level -= 1;
			//if too far,Not allowed above 1280
			if(wifi_stat.last_level < 1)wifi_stat.last_level =1;
		}        
	}
#if 0//do auto adapt resolution
	static int cnt = 0, step = 1;
	cnt += step;
	if(7 < cnt){
		step = -1;
	}else if(0 > cnt){
		step = 1;
	}
	printf("cnt = %d  ;  step = %d\n", cnt, step);
	wifi_stat.last_level = cnt;
#endif

	
	//Prevention the boundary
	if(wifi_stat.last_level < 0)  wifi_stat.last_level = 0;
	if(wifi_stat.last_level > BPS_LEVEL_MAX)  wifi_stat.last_level = BPS_LEVEL_MAX;

	//Get a corresponding resolution
	if(wifi_stat.WifiGetResolution){
		ret = wifi_stat.WifiGetResolution(&wifi_stat, pvenc_ch);
	}
	if(ret){
		APP_TRACE("BPS adepter:rssi(%d)oc(%d)ic(%d)to(%d)bps(%d).\n", 
		wifi_stat.signal_strength, wifi_stat.timeout_count, wifi_stat.timein_count, ret_to, pvenc_ch->constantBitRate);
	}
	return ret;
}

#define SENSOR_10MP_RESOLUTION (1280*720)		
#define SENSOR_13MP_RESOLUTION (1280*960)	
#define SENSOR_20MP_RESOLUTION (1920*1080)
#define SENSOR_23MP_RESOLUTION (1536*1536)
#define SENSOR_40MP_RESOLUTION (2592*1520)

static int wifi_set_default_resolution()
{
	//int addr[4];
	uint32_t width, height;
	
	SENSOR_get_resolution(&width,&height);
	//APP_TRACE("get sensor width:%d, height:%d .",width, height);
	switch(width*height){
		default:
		case SENSOR_10MP_RESOLUTION:
			wifi_stat.WifiGetResolution = wifi_get_10mp_resolution;
			break;
		case SENSOR_13MP_RESOLUTION:
			wifi_stat.WifiGetResolution = wifi_get_13mp_resolution;
			break;
		case SENSOR_20MP_RESOLUTION:
			wifi_stat.WifiGetResolution = wifi_get_20mp_resolution;
			break;
		case SENSOR_23MP_RESOLUTION:
			wifi_stat.WifiGetResolution = wifi_get_23mp_resolution;
			break;			
		case SENSOR_40MP_RESOLUTION:
			wifi_stat.WifiGetResolution = wifi_get_40mp_resolution;
			break;			
	}
	/*NETSDK_conf_interface_get(4, &wlan0);	
	if(4 == sscanf(wlan0.lan.staticIP, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3])){
		sprintf(wifi_stat.wlan_gateway,"%d.%d.%d.%d", addr[0], addr[1], addr[2], 1);
	}
	else{
		strcpy(wifi_stat.wlan_gateway,"172.20.14.1");
	}*/

	return 1;	
}

void *wifi_self_reboot(void)
{
	NK_SYSTEM("ifconfig wlan0 down;ifconfig wlan1 down");
	NK_SYSTEM("rmmod rtl8188eus;rmmod rtl8188fu"); //for hisi
	NK_SYSTEM("rmmod 8188eu;rmmod 8188fu");        //for ak
	sleep(1);
	BSP_Wifi_power_enable(false);
	printf("##### wifi power off #####\n");
	sleep(2);
	BSP_Wifi_power_enable(true);//wifi power up
	printf("##### wifi power on #####\n");
	sleep(2);
    GLOBAL_reboot_system();
	//exit(0);
    //IPCAM_network_restart();
}

typedef enum
{
	NK_WIFI_2_4_G = 0,
	NK_WIFI_5_8_G,
	NK_WIFI_WIRED,
	NK_WIFI_TYPE_AUTO,
	NK_WIFI_TYPE_NR
}tNK_WIFI_TYPE;
static int usb1_wifi_idProduct2Type(char *idProduct)
{
	if(idProduct){
		if(!strcmp(idProduct, "8176") || !strcmp(idProduct, "8179") || !strcmp(idProduct, "f179")){
			return NK_WIFI_2_4_G;
		}
		else if(!strcmp(idProduct, "0811")){
			return NK_WIFI_5_8_G;
		}
		else{
			return -1;
		}
	}
}

static int usb1_wifi_mod_get_idProduct(char *_idProduct, int _size)
{
	FILE *fp = NULL; 
	int res = -1;
	char *p = NULL;
	char buf[128] = {0};

	fp = popen("lsusb", "r");
	if(fp == NULL){ 
		perror("[popen] lsusb");
	}
	else{
		while(fgets(buf, sizeof(buf), fp)){
			if(strstr(buf, "ID")){
				p = strstr(strstr(buf, "ID"), ":");
				if(p && strlen(p) >= 4){
					strtok(p, "\n");
					res = usb1_wifi_idProduct2Type(p+1);
					if(res >= 0 && res < NK_WIFI_TYPE_NR){
						if(_idProduct){
							snprintf(_idProduct, _size, "%s", p+1);
						}
						break;
					}
				}
			}
		} 
		if(pclose(fp) == -1)
		{ 
			printf("close popen file pointer fp error!\n"); 
			return -1;
		} 
	}
	return res;
}

typedef enum
{
	NK_WIFI_UNKONWN_TYPE = 0,
	NK_WIFI_FTV,
	NK_WIFI_RTL8188EUS,
	NK_WIFI_HIGH_POWER,
	NK_WIFI_AU,
	NK_WIFI_CHIP_TYPE_CNT,
}tNK_WIFI_CHIP_TYPE;

#define NK_WIFI_RTL8188EUS_MAP_VAl (0xFF)
#define NK_WIFI_RTL8188EUS_HIGH_POWER_MAP_VAl (0x04)

static int APP_WIFI_get_efuse_map(char *map, int _size)
{
	FILE *fp = NULL; 
	int res = -1;
	char *p = NULL;
	char buf[128] = {0};
	if(APP_WIFI_check_ifstatus("wlan0") < 0){
		NK_SYSTEM("ifconfig wlan0 up");
	}
	NK_SYSTEM("/usr/share/ipcam/wifi_tools/rtwpriv wlan0 efuse_mask off");
	
	fp = popen("/usr/share/ipcam/wifi_tools/rtwpriv wlan0  efuse_get rmap,ca,1", "r");
	if(fp == NULL){ 
		perror("[popen] get rmap");
	}
	else{
		while(fgets(buf, sizeof(buf), fp)){
			if(strstr(buf, "efuse_get")){
				p = strstr(strstr(buf, "efuse_get"), ":");
				if(p && strlen(p) >= 2){
					strtok(p, "\n");
					if(map){
						snprintf(map, _size, "%s", p+1);
					}
					res = 1;
					break;	
				}
			}
		} 
		if(pclose(fp) == -1)
		{ 
			printf("close popen file pointer fp error!\n"); 
			return -1;
		} 
	}
	return res;
}

int APP_WIFI_model_remove()
{
	char cmd[256];
	if(strcmp(wifi_stat.wifi_chip, "rtl8188eu") == 0){
		NK_SYSTEM("rmmod rtl8188eus");
		memset(wifi_stat.wifi_chip, 0, sizeof(wifi_stat.wifi_chip));
	}
	else if(strcmp(wifi_stat.wifi_chip, "rtl8821au") == 0){
		NK_SYSTEM("rmmod rtl8821au");
		memset(wifi_stat.wifi_chip, 0, sizeof(wifi_stat.wifi_chip));
	}
	else if(strcmp(wifi_stat.wifi_chip, "rtl8188fu") == 0){
		NK_SYSTEM("rmmod rtl8188fu");
		memset(wifi_stat.wifi_chip, 0, sizeof(wifi_stat.wifi_chip));
	}
	else{
		APP_TRACE("unknow wifi module (%s)", wifi_stat.wifi_chip);
		return -1;
	}

	return 0;
}

bool APP_WIFI_model_exist()
{	
	bool ret_val = false;
#if defined(WIFI)
	int ret;
	int i, ii;
	FILE *fp = NULL;
	char file_dir[256], file_buf[5] = {0};
    char cmd_buf[256];
    char map[64] = {0};
	if(strcmp(wifi_stat.wifi_chip, "rtl8188eu") && strcmp(wifi_stat.wifi_chip, "rtl8821au") && strcmp(wifi_stat.wifi_chip, "rtl8188fu")){
			char idProduct[32] = {0};
			char wifi_chip_type[6] = {0};
			memset(cmd_buf, sizeof(cmd_buf), 0);
			usb1_wifi_mod_get_idProduct(idProduct, sizeof(idProduct));
			if(!strcmp(idProduct, "8176") || !strcmp(idProduct, "8179")){
        		ret_val = true;
				model_type = 0;
        		strcpy(wifi_stat.wifi_chip, "rtl8188eu");
				snprintf(cmd_buf, sizeof(cmd_buf), "insmod %s/mpp/juan_ko/rtl8188eus.ko", IPCAM_ENV_HOME_DIR);
        	}else if(!strcmp(idProduct, "0811") ){
        		ret_val = true;
        		strcpy(wifi_stat.wifi_chip, "rtl8821au");
        		snprintf(cmd_buf, sizeof(cmd_buf), "insmod %s/mpp/juan_ko/rtl8811AU.ko", IPCAM_ENV_HOME_DIR);
        	}else if(!strcmp(idProduct, "f179") ){
        		ret_val = true;
				model_type = 1;
        		strcpy(wifi_stat.wifi_chip, "rtl8188fu");
				snprintf(cmd_buf, sizeof(cmd_buf), "insmod %s/mpp/juan_ko/rtl8188fu.ko", IPCAM_ENV_HOME_DIR);
        	}else{
				ret_val = false;
			}

			
			if(ret_val){
				APP_TRACE("find wifi device:%s.", wifi_stat.wifi_chip);

				NK_WIFI_adapter_monitor_thread_start(wifi_self_reboot, smart_link_status);

				NK_SYSTEM(cmd_buf);
#if defined(WIFI_MODEL)
				if(strcmp(wifi_stat.wifi_chip, "rtl8188eu") == 0){
					if(APP_WIFI_get_efuse_map(map, sizeof(map))){
						int map_val = 0;
						sscanf(map, "0x%02x", &map_val);
						if(map_val ==  NK_WIFI_RTL8188EUS_MAP_VAl){
							sprintf(wifi_chip_type, "%02d", NK_WIFI_RTL8188EUS);
							SYSCONF_set_software_version_ext(wifi_chip_type);
						}else if(map_val ==  NK_WIFI_RTL8188EUS_HIGH_POWER_MAP_VAl){
							sprintf(wifi_chip_type, "%02d", NK_WIFI_HIGH_POWER);
							SYSCONF_set_software_version_ext(wifi_chip_type);
						}
					}
				}
				else if(strcmp(wifi_stat.wifi_chip, "rtl8188fu") == 0){
					sprintf(wifi_chip_type, "%02d", NK_WIFI_FTV);
					SYSCONF_set_software_version_ext(wifi_chip_type);
				}
				else if(strcmp(wifi_stat.wifi_chip, "rtl8188au") == 0){
					sprintf(wifi_chip_type, "%02d", NK_WIFI_AU);
					SYSCONF_set_software_version_ext(wifi_chip_type);
				}
				SYSCONF_set_software_version_ext(wifi_chip_type);
#endif
				wifi_set_default_resolution();
						//	NK_WIFI_adapter_monitor_thread_start(wifi_self_reboot);
			}
			if(ret_val){
				memset(cmd_buf, sizeof(cmd_buf), 0);
				snprintf(cmd_buf, sizeof(cmd_buf), "echo 1 > /proc/sys/net/ipv4/conf/eth0/arp_ignore;"
											"echo 1 > /proc/sys/net/ipv4/conf/wlan0/arp_ignore;"
											"echo 1 > /proc/sys/net/ipv4/conf/wlan1/arp_ignore");
				NK_SYSTEM(cmd_buf);
			}
	}else{
            ret_val = true;
	}
#endif
	return ret_val;
}


int APP_WIFI_get_rssi()
{
	int ret = 0;
	char file_dir[256];
	char buf[1024] = {0};
	FILE *fp = NULL;
	sprintf(file_dir, "/proc/net/%s/wlan0/rx_signal", wifi_stat.wifi_chip);
	fp = fopen(file_dir, "rb");
	if(NULL != fp){
		fread(buf, sizeof(buf), 1, fp);
		//printf("rx_signal\n%s\n", buf);
		sscanf(buf, "rssi:-%d", &ret);
		//printf("%d\n", ret);
		fclose(fp);
	}
	return ret;
}

int APP_WIFI_get_Connected_MAC(char *ether, int num, unsigned char mac[][6])
{
	int ret = 0;
#if defined(WIFI)
	ret = JN_Wifi_AP_Get_Connected_MAC(ether, num, mac, wifi_stat.wifi_chip);
#endif
	return ret;
}

int APP_WIFI_get_Rate(char *ether, unsigned char mac_addr[6])
{
	int ret = 0;
#if defined(WIFI)
	ret = JN_Wifi_USB1_GetSignal(ether, mac_addr);
#endif
	return ret;
}

#if 0
int APP_WIFI_calculate_signal_level(int rssi)//0~5
{
	int ret = 0;
	static int last_rssi = 45;
	if(0 == rssi){
		return ret;
	}
}

int APP_WIFI_calculate_bps(int rssi)
{
	int ret = 2048;
	int threshold0 = 45;
	int threshold1 = 48;
	int threshold2 = 53;
	int threshold3 = 58;
	int threshold4 = 63;
	int threshold5 = 68;
	int threshold6 = 73;
	int level = 0;
	static int last_level = 0, last_rssi, signal_raise_cnt = 3;
	if(0 == rssi){
		return ret;
	}
	if(rssi > threshold6){
		if(5 >= last_level){
			if(rssi > threshold6 + 1){
				level = 6;
			}else{
				level = 5;
			}
		}else{
			level = 6;
		}
	}else if(rssi > threshold5){
		if(6 <= last_level){
			if(rssi < threshold6 -1){
				level = 5;
			}else{
            	level = 6;
			}
		}else if(4 >= last_level){
			if(rssi > threshold5 + 1){
				level = 5;
			}else{
				level = 4;
			}
		}else{//4<level<6
			level = 5;
		}
	}else if(rssi > threshold4){
		if(5 <= last_level){
			if(rssi < threshold5 -1){
				level = 4;
			}else{
            	level = 5;
			}
		}else if(3 >= last_level){
			if(rssi > threshold4 + 1){
				level = 4;
			}else{
				level = 3;
			}
		}else{//3<level<5
			level = 4;
		}
	}else if(rssi > threshold3){
		if(4 <= last_level){
			if(rssi < threshold4 -1){
				level = 3;
			}else{
				level = 4;
			}
		}else if(2 >= last_level){
			if(rssi > threshold3 + 1){
				level = 3;
			}else{
				level = 2;
			}
		}else{// 2<level<4
			level = 3;
		}
	}else if(rssi > threshold2){
		if(3 <= last_level){
			if(rssi < threshold3 -1){
				level = 2;
			}else{
				level = 3;
			}
		}else if(1 >= last_level){
			if(rssi > threshold2 + 1){
				level = 2;
			}else{
				level = 1;
			}
		}else{// 1<level<3
			level = 2;
		}
	}else if(rssi > threshold1){
		if(2 <= last_level){
			if(rssi < threshold2 -1){
				level = 1;
			}else{
				level = 2;
			}
		}else if(0 >= last_level){
			if(rssi > threshold1 + 1){
				level = 1;
			}else{
				level = 0;
			}
		}else{// 0<level<2
			level = 1;
		}
	}else if(rssi > threshold0){
		if(1 <= last_level){
			if(rssi < threshold1 -1){
				level = 0;
			}else{
				level = 1;
			}
		}else{// last < 1
			level = 0;
		}
	}else{
		level = 0;
	}

	if(last_level > level){
		if(--signal_raise_cnt < 0){
			last_level = level;
			signal_raise_cnt = 3;
		}
	}else{
		last_level = level;
		signal_raise_cnt = 3;
	}
	
	switch(last_level){
		case 0:
			ret = 1536;
			break;
		case 1:
			ret = 1024;
			break;
		case 2:
			ret = 768;
			break;
		case 3:
			ret = 512;
			break;
		case 4:
			ret = 256;
			break;
		case 5:
			ret = 128;
			break;
		case 6:
			ret = 64;
			break;
		default:
			ret = 2048;
			break;
	}
	
	//printf("rssi/lastlevel/level/bps:%d-%d-%d-%d---%d\n", rssi, last_level, level, ret, signal_raise_cnt);
	return ret;
}
#endif
int wifi_get_adapter_state()
{
	int ret = 0;
	char file_dir[256];
	char buf[256] = {0};
	FILE *fp = NULL;
	sprintf(file_dir, "/proc/net/%s/wlan0/adapter_state", wifi_stat.wifi_chip);
	fp = fopen(file_dir, "rb");
	if(NULL != fp){
		fread(buf, sizeof(buf), 1, fp);
		//printf("rx_signal\n%s\n", buf);
		sscanf(buf, "bSurpriseRemoved=%d", &ret);
		//printf("\n%d\n", ret);
		fclose(fp);
	}
	return ret?0xe1:1;
}
//return -1 need restart wlan0;return 0 igore, return 1 normal.
int APP_WIFI_check_ifstatus(char *nic_name)
{
    //char *nic_name = "wlan0";
    int ret = 0;
    //struct sockaddr_in sin;
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
        close(skfd);
        return 0;
    }
    if(!(ifr.ifr_flags & IFF_UP)){      //when rssi unaqual zero.sta mode
        printf("Device(%s)_down.\n",nic_name);
        close(skfd);
        return -1;    
    } 
    if(ifr.ifr_flags & IFF_RUNNING){      //when rssi unaqual zero.sta mode
    	wifi_stat.wifi_conneted = TRUE;
    } else{
		wifi_stat.wifi_conneted = FALSE;
    }
#if 0
	//if wifi driver dead, reboot and fixed it.
	if((wifi_stat.signal_strength > 5) && (ifr.ifr_flags & IFF_RUNNING)){
	//printf("signal_strength:%d,last_signal_strength:%d,signal_equal_count:%d.\n", 
	//    wifi_stat.signal_strength, wifi_stat.last_signal_strength,wifi_stat.signal_equal_count);
	
		if(wifi_stat.signal_strength != wifi_stat.last_signal_strength){
			wifi_stat.last_signal_strength = wifi_stat.signal_strength;
			wifi_stat.signal_equal_count = 0;
		}else{
			wifi_stat.signal_equal_count += 1;
			if(wifi_stat.signal_equal_count > 60){
				wifi_stat.signal_equal_count = 0;
				return 0xE2;
			}
		}
	}
#endif    
    //check ipaddr.
    strncpy(ifr.ifr_name, nic_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0; 
    ret = ioctl(skfd, SIOCGIFADDR, &ifr) ;
   // if ((ret < 0) && (ifr.ifr_flags & IFF_RUNNING)) 
   if (ret < 0){
        perror("ioctl");
        close(skfd);
        return -1;
   }
    close(skfd);
    
    /*if(strlen(wifi_stat.wlan_gateway)<6){
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	//printf("if ip :%s.\n", inet_ntoa(sin.sin_addr));
    }*/
    
    ret = wifi_get_adapter_state();
 
	return ret; 
}

#if  0
int main()
{
    struct timeval tpstart,tpend; 
    float timeuse; 
    //unsigned char ip[4]={0x8,0x8,0x8,0x8};
    unsigned char ip[4]={0xac,0x14,0xe,0x1};
    while(1){
        gettimeofday(&tpstart,NULL); 
        APP_WIFI_calculate_bps();
        gettimeofday(&tpend,NULL); 
        timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
        tpend.tv_usec-tpstart.tv_usec; 
        timeuse/=1000000; 
        printf("Used Time:%f\n",timeuse);        
        sleep(2);
    }
    //if(!icmp_time_out(ip)){
    //    printf("have  obstruction.\n");
    //}    
}

#endif

static int  get_a_rand_num()
{
	int i = 0;
	struct timeval tv;
	struct timezone tz;
	
	gettimeofday ( &tv, &tz );
	i = tv.tv_usec%255;
	APP_TRACE("i is %d",i);
	if(i < 6){
		i = i + 5;
	}
	APP_TRACE("rand ip last number is %d",i);
	return i;
}

static void wifi_update(ST_NSDK_NETWORK_INTERFACE n_interface){
#if defined(WIFI)
	if(n_interface.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT){  //AP
				if(strcmp(n_interface.wireless.dhcpServer.dhcpIpGateway,n_interface.lan.staticGateway)){
					//staticGateway
					strncpy(n_interface.lan.staticGateway,n_interface.wireless.dhcpServer.dhcpIpGateway,sizeof(n_interface.lan.staticGateway));
					//staticPreferredDns
					strncpy(n_interface.dns.staticPreferredDns,n_interface.wireless.dhcpServer.dhcpIpDns,sizeof(n_interface.dns.staticPreferredDns));
					//staticIP
					strncpy(n_interface.lan.staticIP,n_interface.wireless.dhcpServer.dhcpIpGateway,sizeof(n_interface.lan.staticIP));
				}

				JN_Wifi_AP_Init(n_interface.lan.staticIP, n_interface.lan.staticNetmask, n_interface.lan.staticGateway, n_interface.dns.staticPreferredDns);
				JN_Wifi_AP_Setup(n_interface.wireless.wirelessApMode.wirelessEssId, 
								 n_interface.wireless.wirelessApMode.wirelessPsk, 
								 n_interface.wireless.wirelessApMode.wireLessApMode, 
								 n_interface.wireless.wirelessApMode.wirelessWpaMode, 
								 n_interface.wireless.wirelessApMode.wirelessApMode80211nChannel, 
								 n_interface.wireless.dhcpServer.dhcpIpNumber, 
								 n_interface.wireless.dhcpServer.dhcpIpRange, 
								 n_interface.wireless.dhcpServer.dhcpIpDns, 
								 "255.255.255.0", 
								 n_interface.wireless.dhcpServer.dhcpIpGateway);
			}else if(n_interface.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE){	 //STA
				JN_Wifi_STA_Init();
				JN_Wifi_STA_Setup(n_interface.wireless.wirelessStaMode.wirelessApEssId,
								  n_interface.wireless.wirelessStaMode.wirelessApPsk,
								  n_interface.lan.staticIP, n_interface.lan.staticNetmask,
								  n_interface.wireless.dhcpServer.dhcpAutoSettingEnabled);
			}else if(n_interface.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_NONE){  //none

			}
#endif
}

static void* wifi_mode_config(lpWifiAttr attr){
	ST_NSDK_NETWORK_INTERFACE wlan;
	
	NETSDK_conf_interface_get(4, &wlan);//wifi
	if(attr){
		if(SMARTLINK_STATUS_SWITCH_TO_MONITOR == attr->status){
#ifdef LED_CTRL
			initLedContrl(DEF_LED_ID,true,LED_MIN_MODE);

#endif
			//SearchFileAndPlay(SOUND_WiFi_connection_failed, NK_True);
			//SearchFileAndPlay(SOUND_Please_setup_again, NK_False);
		}else if(SMARTLINK_STATUS_SUCCESS == attr->status ||  SMARTLINK_STATUS_SWITCH_TO_PD_TEST == attr->status){
			if(strlen(attr->essid) > 0){
				switch(attr->mode){
					case SMARTLINK_WIFI_MODE_AP:
						wlan.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT;
						snprintf(wlan.wireless.wirelessApMode.wirelessEssId, sizeof(wlan.wireless.wirelessApMode.wirelessEssId),"%s",
							attr->essid);
						snprintf(wlan.wireless.wirelessApMode.wirelessPsk, sizeof(wlan.wireless.wirelessApMode.wirelessPsk), "%s", 
							attr->password);
						break;
					case SMARTLINK_WIFI_MODE_STATION:
						wlan.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_STATIONMODE;
						snprintf(wlan.wireless.wirelessStaMode.wirelessApEssId, sizeof(wlan.wireless.wirelessStaMode.wirelessApEssId), "%s",
							attr->essid);
						snprintf(wlan.wireless.wirelessStaMode.wirelessApPsk, sizeof(wlan.wireless.wirelessStaMode.wirelessApPsk), "%s",
							attr->password);
						wlan.wireless.dhcpServer.dhcpAutoSettingEnabled = attr->dhcp;
						wlan.lan.addressingType = attr->dhcp ? kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC : kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC;
						//ip
						snprintf(wlan.lan.staticIP,sizeof(wlan.lan.staticIP),"172.20.14.%d",get_a_rand_num());
                        wlan.wireless.wirelessStaMode.wirelessFixedBpsModeEnabled = true;
						break;
					default:
						wlan.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_NONE;
						break;
				}
				printf("%s-%d   essid:%s  psw:%s  mode:%d\n",
					__FUNCTION__, __LINE__, attr->essid, attr->password, attr->mode);

				if(SMARTLINK_STATUS_SWITCH_TO_PD_TEST == attr->status){
					NETSDK_tmp_interface_set(4, &wlan, eNSDK_CONF_SAVE_RESTART);
					printf("product test : %s\n", attr->essid);
				}
				else{
					//set the venc to the compatibility mode
					ST_NSDK_VENC_CH venc_ch;
					NETSDK_conf_venc_ch_get(101, &venc_ch);
					int venc_changed1 = 0, venc_changed2 = 0;
					if(venc_ch.ImageTransmissionModel != eNSDK_COMPATIBILITY_MODE){
						venc_ch.ImageTransmissionModel = eNSDK_COMPATIBILITY_MODE;
						venc_changed1 = 1;
					}
					if(venc_ch.definitionType != kNSDK_DEFINITION_AUTO){
						venc_ch.definitionType = kNSDK_DEFINITION_AUTO;
						venc_changed1 = 1;
					}
					if(venc_changed1 == 1){
						NETSDK_conf_venc_ch_set(101, &venc_ch);
					}

					memset(&venc_ch, 0, sizeof(ST_NSDK_VENC_CH));
					NETSDK_conf_venc_ch_get(102, &venc_ch);
					if(venc_ch.ImageTransmissionModel != eNSDK_COMPATIBILITY_MODE){
						venc_ch.ImageTransmissionModel = eNSDK_COMPATIBILITY_MODE;
						venc_changed2 = 1;
					}
					if(venc_ch.definitionType != kNSDK_DEFINITION_AUTO){
						venc_ch.definitionType = kNSDK_DEFINITION_AUTO;
						venc_changed2 = 1;
					}
					if(venc_changed2 == 1){
						NETSDK_conf_venc_ch_set(102, &venc_ch);
					}
#ifdef VIDEO_CTRL
					VIDEO_CTRL_destroy();
#endif

					ST_NSDK_AENC_CH aenc_ch;
					NETSDK_conf_aenc_ch_get(101, &aenc_ch);
					if(aenc_ch.codecType != kNSDK_AENC_CODEC_TYPE_G711A){
						aenc_ch.codecType = kNSDK_AENC_CODEC_TYPE_G711A;
						NETSDK_conf_aenc_ch_set(101, &aenc_ch);
					}

                    NETSDK_conf_interface_set_by_delay(4, &wlan, eNSDK_CONF_SAVE_RESTART, 2);
                    SearchFileAndPlay(SOUND_WiFi_setting, NK_False);
    				SearchFileAndPlay(SOUND_Please_wait, NK_False);
    				printf("smartlink token:%s\n", attr->token);
    				NK_Corsee_SetToken(attr->token);
                }
			}		
			//wifi_update(wlan);
			IPCAM_timer_open_wifi_modify_bps();
			//TFCARD_rename_and_stop_rec(); //stop tfcard record
		}
		else if(SMARTLINK_STATUS_RECEIVE_PACKET_TIMEOUT == attr->status){
			//SearchFileAndPlay(SOUND_Network_configure_failed, NK_True);
		/*#ifdef LED_CTRL
			initLedContrl(DEF_LED_ID,true,LED_MIN_MODE);
		#endif*/
			//wifi_update(wlan); //cause AP not kill at the wireless match code thread,so don't start AP again when thread time out
		}else if(SMARTLINK_STATUS_START_TO_RECEIVE_PACKEET == attr->status){
			//SearchFileAndPlay(SOUND_Network_configuring, NK_False);
		}else if(SMARTLINK_STATUS_SEARCH_CHANNEL == attr->status){
			//SearchFileAndPlay(SOUND_Please_configure_network, NK_False);
		}else{

		}
	}
}

#if defined(WIFI) && defined(SMART_LINK)
void smartlink_log(const char *fmt, ...)
{
	va_list   arg;
	int   done;
	char logString[512]={0};
	char logstring[512]={0};
	int strLen = 0;
	va_start(arg,fmt);
	vsprintf(logString, fmt, arg);
	va_end(arg);
	sprintf(logstring, "%s%s","[smartlink_log]:", logString); 
	strLen = strlen(logstring);
	printf("[smartlink_log]:%s", logString);
	if(0 != strLen){
		//onFlush(logstring, strLen);
	}
}

int SMART_link_get_wifi_mode_callback()
{
	ST_NSDK_NETWORK_INTERFACE wlan;
	NETSDK_conf_interface_get(4, &wlan);
	emWIFI_MODE mode;

	switch(wlan.wireless.wirelessMode){
		case NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT:
			mode = SMARTLINK_WIFI_MODE_AP;
			break;
		case NSDK_NETWORK_WIRELESS_MODE_STATIONMODE:
			mode = SMARTLINK_WIFI_MODE_STATION;
			break;
		case NSDK_NETWORK_WIRELESS_MODE_NONE:
		default:
			mode = SMARTLINK_WIFI_MODE_NONE;
			break;
	}

	return mode;
}

void SMART_link_switch_mode_callback(int mode)
{
	ST_NSDK_NETWORK_INTERFACE wlan;
	NETSDK_conf_interface_get(4, &wlan);
	
	if(mode == SMARTLINK_WIFI_MODE_AP){
		wlan.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT;
		NETSDK_conf_interface_set(4, &wlan, eNSDK_CONF_SAVE_JUST_SAVE);
	}else if(mode == SMARTLINK_WIFI_MODE_STATION){
		wlan.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_STATIONMODE;
		NETSDK_conf_interface_set(4, &wlan, eNSDK_CONF_SAVE_JUST_SAVE);
	}
}

int SMART_link_init(char *ether)
{
	ST_NSDK_NETWORK_INTERFACE wlan;
	emWIFI_MODE mode;
	bool immediately = false;
	NETSDK_conf_interface_get(4, &wlan);
	switch(wlan.wireless.wirelessMode){
		case NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT:
			mode = SMARTLINK_WIFI_MODE_AP;
			break;
		case NSDK_NETWORK_WIRELESS_MODE_STATIONMODE:
			mode = SMARTLINK_WIFI_MODE_STATION;
			break;
		case NSDK_NETWORK_WIRELESS_MODE_NONE:
		default:
			mode = SMARTLINK_WIFI_MODE_NONE;
			break;
	}

	immediately = true;
	smart_link_rtl8188_init(ether, wifi_mode_config, wlan.wireless.wirelessStaMode.wirelessApEssId, mode, immediately,
		smartlink_log, SMART_link_get_wifi_mode_callback, SMART_link_switch_mode_callback);
    return 0;
}

int SMART_link_deinit(char *ether)
{
	return smart_link_rtl8188_quit(ether);
}

#endif


//return -1 need restart wlan0;return 0 igore, return 1 normal.
int APP_WIFI_check_sta_status(char *nic_name)
{
    //char *nic_name = "wlan0";
    int ret = 0;
    //struct sockaddr_in sin;
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
        close(skfd);
        return 0;
    }
    if(!(ifr.ifr_flags & IFF_UP)){      //when rssi unaqual zero.sta mode
        printf("Device(%s)_down.\n",nic_name);
        close(skfd);
        return -1;    
    } 
    if(!(ifr.ifr_flags & IFF_RUNNING)){      //when rssi unaqual zero.sta mode
		close(skfd);
		return -1;
    }
    
    //check ipaddr.
    strncpy(ifr.ifr_name, nic_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0; 
    ret = ioctl(skfd, SIOCGIFADDR, &ifr) ;
   // if ((ret < 0) && (ifr.ifr_flags & IFF_RUNNING)) 
   if (ret < 0){
        perror("ioctl");
        close(skfd);
        return -1;
   }
	close(skfd);
    
    /*if(strlen(wifi_stat.wlan_gateway)<6){
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	//printf("if ip :%s.\n", inet_ntoa(sin.sin_addr));
    }*/
    
    //ret = wifi_get_adapter_state();
 
	return 1; 
}

int APP_WIFI_exit_wifi()
{
	TICKER_destroy();
	NK_SYSTEM("ifconfig wlan0 down");
	NK_SYSTEM("ifconfig wlan1 down");
	char cmd[128], driver_name[64];
	if(!strcmp(wifi_stat.wifi_chip, "rtl8188eu")){
		sprintf(driver_name, "rtl8188eus.ko");
	}else if(!strcmp(wifi_stat.wifi_chip, "rtl8821au")){
		sprintf(driver_name, "rtl8811AU.ko");
	}else if(!strcmp(wifi_stat.wifi_chip, "rtl8188fu")){
		sprintf(driver_name, "rtl8188fu.ko");
	}else{
		sprintf(driver_name, "rtl8188eus.ko");
	}
	snprintf(cmd, sizeof(cmd), "rmmod %s", driver_name);
	NK_SYSTEM(cmd);
	return 0;
}

int APP_WIFI_Wifi_Exit()
{
#if defined(WIFI)
	//APP_Wifi_Concurrent_Set_Bridge_Off(NSDK_NETWORK_WIRELESS_MODE_REPEATER_STA_ETH, NSDK_NETWORK_WIRELESS_MODE_REPEATER_AP_ETH);
	JN_Wifi_Exit();
	//将concurrent模式下设置的AP设置为managed模式,防止AP关不掉的情况
	/*
	if(APP_WIFI_If_Exist(NSDK_NETWORK_WIRELESS_MODE_REPEATER_AP_ETH)){
		set_wlan_mode_managed(NSDK_NETWORK_WIRELESS_MODE_REPEATER_AP_ETH);
	}*/
#endif
	return 0;
}

int APP_WIFI_is_sta_connected()
{
	return wifi_stat.wifi_conneted;
}

extern int __scan_wireless_signal( char *__interface, int timeout);
int APP_WIFI_Wifi_Scan(char *ether, stNK_WIFI_HotSpot *essid_list, int max_num)
{
	int ret = -1, i = 0;

#if defined(WIFI)

	FILE *fp = NULL;
	char str[256];
	char list[10][64];
	int found = 0, bssid_id = -1, dBm_id = -1, SdBm_id = -1, ssid_id = -10, channel_id = -1, age_id = -1, encrypt_id = -1;
	char *p = NULL;

	if(!essid_list){
		return ret;
	}

	__scan_wireless_signal(ether ? ether : "wlan0", 5);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "/proc/net/%s/%s/survey_info", wifi_stat.wifi_chip, ether ? ether : "wlan0");

	if((fp = fopen (str, "r" )) == NULL){
		printf("can not open survey_info file \n");
		return ret;
	}

	if(fgets(str, sizeof(str), fp)){
		memset(list, 0, sizeof(list));
		ret = sscanf(str, "%s %s %s %s %s %s %s %s %s %s", list[0], list[1], list[2], list[3], list[4], list[5], list[6], list[7], list[8], list[9]);
		//printf("%s %s %s %s %s %s %s %s %s %s\n", list[0], list[1], list[2], list[3], list[4], list[5], list[6], list[7], list[8], list[9]);
		for(i = 0; i < sizeof(list) / sizeof(list[0]); i++){
			if(strcmp(list[i], "bssid") == 0){
				bssid_id = i;
			}
			else if(strcmp(list[i], "dBm") == 0 || strcmp(list[i], "RSSI") == 0 ){
				dBm_id = i;
			}
			else if(strcmp(list[i], "SdBm") == 0){
				SdBm_id = i;
			}
			else if(strcmp(list[i], "ssid") == 0){
				ssid_id = i;
			}
			else if(strcmp(list[i], "ch") == 0){
				channel_id = i;
			}
			else if(strcmp(list[i], "age") == 0){
				age_id = i;
			}
            else if(strcmp(list[i], "flag") == 0){
				encrypt_id = i;
			}
		}
		if(ret - 1 == ssid_id){
			found = 1;
		}
	}
	if(!found){
		printf("[%s:%d] Not found! ret=%d ssid_id=%d\n", __func__, __LINE__, ret, ssid_id);
		fclose(fp);
		return -1;
	}
	ret = 0;
	while(fgets ( str, sizeof(str), fp )) {
		memset(list, 0, sizeof(list));
		i = sscanf(str, "%s %s %s %s %s %s %s %s %s %s", list[0], list[1], list[2], list[3], list[4], list[5], list[6], list[7], list[8], list[9]);
		//printf("%s %s %s %s %s %s %s %s %s %s\n", list[0], list[1], list[2], list[3], list[4], list[5], list[6], list[7], list[8], list[9]);

		if(i <= ssid_id){
			continue;
		}
		p = strstr(str, list[ssid_id]);
		strtok(p, "\r\n");
		snprintf(list[ssid_id], sizeof(list[ssid_id]), "%s", p);
		
		if(bssid_id != -1){
			if(snprintf(essid_list[ret].bssid, sizeof(essid_list[ret].bssid), "%s", list[bssid_id]) < 1){
				printf("[%d] bssid is NULL\n", ret);
				continue;
			}
		}	
		if(channel_id != -1){
			essid_list[ret].channel = atoi(list[channel_id]);
		}
		if(dBm_id != -1){
			essid_list[ret].dBm = atoi(list[dBm_id]);
		}
		if(SdBm_id != -1){
			essid_list[ret].sdBm = atoi(list[SdBm_id]);
		}
		if(age_id != -1){
			essid_list[ret].age = atoi(list[age_id]);
		}
		if(ssid_id != -10){
			if(snprintf(essid_list[ret].essid, sizeof(essid_list[ret].essid) > 32 ? 33 : sizeof(essid_list[ret].essid), "%s", list[ssid_id]) < 1){
				printf("[%d] essid is NULL\n", ret);
				continue;
			}
		}
        if(encrypt_id != -1){
            snprintf(essid_list[ret].encrypt, sizeof(essid_list[ret].encrypt), "%s", list[encrypt_id]);
        }
		//printf("%s %d %d %d %d %s %s\n", essid_list[ret].bssid, essid_list[ret].channel, essid_list[ret].dBm, 
		//		essid_list[ret].sdBm, essid_list[ret].age, essid_list[ret].essid, essid_list[ret].encrypt);
		ret++;
		if(ret >= max_num){
			break;
		}
	}

	fclose(fp);
	fp = NULL;

	return ret;
	
#endif 
}

void APP_WIFI_get_near_ap(lpNK_WIFI_HotSpot lpAPs, unsigned int *nAPs)
{
#if defined(WIFI)
    int i = 0, count = 0;

    if(lpAPs && nAPs) {
        lpNK_WIFI_HotSpot ret_ap = lpAPs;
        static stNK_WIFI_HotSpot essid_list[64];
        static time_t tms;
        static int essid_num;
        static unsigned char dBm[64];
        int j, *dBmList = NULL;

        if(time(NULL) - tms >= 8 || essid_num <= 0) {
            essid_num = APP_WIFI_Wifi_Scan("wlan0", essid_list, sizeof(essid_list) / sizeof(essid_list[0]));
            tms = time(NULL);
            if(essid_num > 0) {
                memset(dBm, 0, sizeof(dBm));
                if((dBmList = alloca(sizeof(int) * essid_num)) == NULL) {
                    essid_num = 0;
                }
                for(i = 0; i < essid_num; i++) {
                    if(essid_list[i].essid[0] == '\0' || strlen(essid_list[i].essid) > sizeof(ret_ap->essid) - 1 || essid_list[i].essid[0] == ' ' 
                        ){
                        essid_list[i].essid[0] = '\0';
                        dBmList[i] = -100;
                    }
                    else{
                        dBmList[i] = essid_list[i].dBm;
                    }
                    dBm[i] = i;
                }
                for(j = 1; j < essid_num; j++) {
                    for(i = 0; i < essid_num - j; i++) {
                        if(dBmList[i] < dBmList[i + 1]) {
                            count = dBmList[i + 1];
                            dBmList[i + 1] = dBmList[i];
                            dBmList[i] = count;

                            count = dBm[i + 1];
                            dBm[i + 1] = dBm[i];
                            dBm[i] = count;
                        }
                    }
                }
                for(count = essid_num, i = essid_num; i > 0; i--) {
                    if(dBmList[i - 1] <= -99) {
                        count--;
                    }
                    else {
                        break;
                    }
                }
                essid_num = count;
                for(i = 0; i < essid_num - 1; i++) {
                    for(j = i + 1; j < essid_num; j++) {
                        if(essid_list[dBm[j]].essid[0] == '\0'){
                            continue;
                        }
                        if(strcmp(essid_list[dBm[i]].essid, essid_list[dBm[j]].essid) == 0) {
                            APP_TRACE("Repeat SSID[%d] (%s)", dBm[j], essid_list[dBm[j]].essid);
                            essid_list[dBm[j]].essid[0] = '\0';
                        }
                    }
                }
            }
        }
        if(essid_num > 0) {
            for(i = 0, count = 0; i < essid_num && i < *nAPs; i++) {
                if(essid_list[dBm[i]].essid[0] == '\0') {
                    continue;
                }
                snprintf(ret_ap->essid, sizeof(ret_ap->essid), "%s", essid_list[dBm[i]].essid);
                snprintf(ret_ap->bssid, sizeof(ret_ap->bssid), "%s", essid_list[dBm[i]].bssid);
                snprintf(ret_ap->encrypt, sizeof(ret_ap->encrypt), "%s", essid_list[dBm[i]].encrypt);
                ret_ap->dBm = essid_list[dBm[i]].dBm;
                //APP_TRACE("SSID:%s(%d) BSSID:%s(%d) ENCRY:%s(%d) RSSI:%d", ret_ap->essid, strlen(ret_ap->essid), 
                //    ret_ap->bssid, strlen(ret_ap->bssid), ret_ap->encrypt, strlen(ret_ap->encrypt), ret_ap->dBm);
                ret_ap++;
                count++;
            }
            APP_TRACE("Got %d , return %d", essid_num, count);
            *nAPs = count;
        }
    }
#endif

}

#if 0
static void search_ap_dump(lpstAP_info ap_info, int index)
{
	//printf("ap_info[%d].index:%d\n", index, ap_info->index);
	//printf("ap_info[%d].mac:%02x:%02x:%02x:%02x:%02x:%02x\n", index, ap_info->mac[0], ap_info->mac[1], ap_info->mac[2], ap_info->mac[3], ap_info->mac[4], ap_info->mac[5]);
	//printf("ap_info[%d].ch:%d\n", index, ap_info->ch);
	//printf("ap_info[%d].rssi:%d\n", index, ap_info->rssi);
	//printf("ap_info[%d].sinal:%d\n", index, ap_info->sinal);
	//printf("ap_info[%d].index:%d\n", index, ap_info->noise);
	//printf("ap_info[%d].age:%d\n", index, ap_info->age);
	printf("ap_info[%d].ssid:%s;%d\n", index, (char *)ap_info->ssid, strlen((char *)ap_info->ssid));
	//printf("ap_info[%d].save:%d\n", index, ap_info->save);
}

void APP_WIFI_search_ap()
{
	int i, ap_num = 0;
	
	memset(&gs_wifi_near_ap, 0, sizeof(ST_WIFI_NEAR_AP));
	ap_num = NK_WIFI_search_ap(gs_wifi_near_ap.ap_info, "wlan0", 20);

	if(ap_num > 0){
		gs_wifi_near_ap.num = ap_num;
		APP_TRACE("ap_num = %d", ap_num);
		for(i = 0; i< ap_num; i++){
			//search_ap_dump(&gs_wifi_near_ap.ap_info[i], i);
		}
	}
}

int APP_WIFI_get_near_ap(stAP_info *ap_info, int ap_size)
{
	int ret_num = 0;
	
	if(ap_info != NULL && gs_wifi_near_ap.num > 0){
		ret_num = gs_wifi_near_ap.num < ap_size ? gs_wifi_near_ap.num : ap_size;
		memcpy(ap_info, gs_wifi_near_ap.ap_info, sizeof(stAP_info)*ret_num);
	}
	
	return ret_num;
}
#endif

bool APP_WIFI_If_Exist(char *ether)
{
	char ptr[128];
	if(!ether){
		return 0;
	}

	snprintf(ptr, sizeof(ptr), "/proc/net/%s/%s", wifi_stat.wifi_chip,  ether);
	return access(ptr, F_OK) == 0 ? true : false;
}

extern int ifconf_get_interface(const char if_name[IFNAMSIZ], ifconf_interface_t* ifr);
int APP_Wifi_Concurrent_RestartAp(char *sta_essid, char *sta_psk, char *ap_essid, char *ap_psk, char *ap_ether)
{
#if defined(WIFI)
	ifconf_interface_t ifconf_irf;
	char essid[64], host_ip[16], dhcp_ip_start[16];
	int channel = 14, ip_start_mask = 10, ret = -1;
	memset(&ifconf_irf, 0, sizeof(ifconf_interface_t));
	memset(essid, 0, sizeof(essid));
	if(!ap_ether || !sta_essid || !sta_psk || !ap_essid){
		return -1;
	}

	snprintf(essid, sizeof(essid), "%s", ap_essid);
	ifconf_get_interface(NSDK_NETWORK_WIRELESS_MODE_REPEATER_ETH, &ifconf_irf);

	snprintf(host_ip, sizeof(host_ip), "%d.%d.%d.%d", ifconf_irf.ipaddr.s_b[0], ifconf_irf.ipaddr.s_b[1], ifconf_irf.ipaddr.s_b[2], ifconf_irf.ipaddr.s_b[3]);
	if(ifconf_irf.ipaddr.s_b[3] > ip_start_mask + 20){
		snprintf(dhcp_ip_start, sizeof(dhcp_ip_start), "%d.%d.%d.%d", ifconf_irf.ipaddr.s_b[0], ifconf_irf.ipaddr.s_b[1], ifconf_irf.ipaddr.s_b[2], ip_start_mask);
	}
	else{
		snprintf(dhcp_ip_start, sizeof(dhcp_ip_start), "%d.%d.%d.%d", ifconf_irf.ipaddr.s_b[0], ifconf_irf.ipaddr.s_b[1], ifconf_irf.ipaddr.s_b[2],  ifconf_irf.ipaddr.s_b[3] + 1);
	}

    // FIXME
	//return JN_Wifi_AP_Setup(ap_ether, essid, ap_psk, NSDK_NETWORK_WIRELESS_APMODE_80211BGN, NSDK_NETWORK_WIRELESS_WPAMODE_WPA2_PSK, 
	//				channel, dhcp_ip_start, "20", "8.8.8.8", "255.255.255.0", host_ip, NSDK_NETWORK_WIRELESS_MODE_REPEATER_ETH, 1, 0);
    return 0;
#endif
	return -1;
}

int APP_Wifi_Concurrent_Set_Bridge(char *ether1, char *ether2, char *br_ip, char *netmask)
{
	ifconf_interface_t ifconf_irf;
	char str[200];
	int ret = -1;
#if defined(WIFI)
	if(!ether1 || !ether2 || !br_ip){
		return -1;
	}

	ifconf_get_interface(ether1, &ifconf_irf);

	ret = JN_Wifi_Concurrent_Set_Bridge(ether1, ether2, br_ip, netmask);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "ifconfig br0 hw ether %02x:%02x:%02x:%02x:%02x:%02x", ifconf_irf.hwaddr.s_b[0], ifconf_irf.hwaddr.s_b[1], ifconf_irf.hwaddr.s_b[2],
		ifconf_irf.hwaddr.s_b[3], ifconf_irf.hwaddr.s_b[4], ifconf_irf.hwaddr.s_b[5]);
	NK_SYSTEM(str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "echo 1 > /proc/sys/net/ipv4/conf/br0/arp_ignore");
	NK_SYSTEM(str);
#endif
	return ret;
}

int APP_Wifi_Concurrent_Set_Bridge_Off(char *ether1, char *ether2)
{
	int ret = -1;
#if defined(WIFI)
	ret = JN_Wifi_Concurrent_Set_Bridge_Off(ether1, ether2);
#endif
	return ret;
}

int APP_WIFI_smartlink_is_running()
{
#if defined(WIFI) && defined(SMART_LINK)
    return smart_link_status();
#endif

}

