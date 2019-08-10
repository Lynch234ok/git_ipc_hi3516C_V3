
#include "ticker.h"
#include "overlay.h"
#include "esee_client.h"
#include "sdk/sdk_api.h"
#include "generic.h"
#include "gpio.h"
#include "sensor.h"
#include "sysconf.h"
#include "app_overlay.h"
#include "media_buf.h"
#include "netsdk.h"
#include "hichip.h"
#include "firmware.h"
#include "wifi/wifi.h"
#include "ifconf.h"
#include "checkip.h"
#include "net_dhcp.h"
#include "sound.h"
#include "base/ja_process.h"
#include "bsp/keytime.h"
#include "ipcam_timer.h"
#include "ntp.h"
#include "global_runtime.h"
#include "app_wifi.h"
#include "wifi/ja_wifi_seek.h"
#include <sys/prctl.h>
#include "gsensor.h"
#include "fisheye.h"
#include "production_test.h"
#include "bsp/bsp.h"
#include "app_msg_push.h"

static void ipcam_close_ip_auto_adapted(void *arg){
#if !defined(DHCP)
	static int cnt = 2;
	ST_NSDK_NETWORK_INTERFACE interface1, interface2;
	if(0 >= cnt--){
		NETSDK_conf_interface_get(1, &interface1);
		if(interface1.lan.addressingType != kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC){
			interface1.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC;
			NETSDK_conf_interface_set(1, &interface1, eNSDK_CONF_SAVE_JUST_SAVE);
			printf("close onvif IP adapted\r\n");
			cnt = 2;
		}

		NETSDK_conf_interface_get(4, &interface2);
		if(interface2.lan.addressingType != kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC){
			interface2.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC;
			NETSDK_conf_interface_set(4, &interface2, false);
			printf("close wlan0 onvif IP adapted\r\n");
			cnt = 2;
		}
	}
#endif// !defined(DHCP)
}

static void ipcam_wifi_iface_daemon(void *arg)
{
	int ret = 0;
	static int del_couter = 0;
	bool model_exist;
	ST_NSDK_NETWORK_INTERFACE n_interface;
#if defined(WIFI) && !defined(SMART_LINK)
	model_exist = APP_WIFI_model_exist();
	NETSDK_conf_interface_get(4, &n_interface);
	if(model_exist && 
		n_interface.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE){

		const char *eth = getenv("DEF_ETHER");
		if(strcmp(eth, "eth0")){//if use eth0,recover the bps and resolution
			ret = APP_WIFI_check_ifstatus("wlan0");
	            	if((!FIRMWARE_is_upgrading()) && ( ret != 0 && ret != 1)){
	            		if(ret < 0 ){
		                printf("\n\nwifi device disconnect,app restart!\n\n");
		                exit(0);
	                }
	                if(ret == 0xE1 || ret == 0xE2){
				printf("\n\nwifi drv abnormal(err:0x%x),reboot!\n\n", ret);
		                NK_SYSTEM("reboot");
	                }
	            }
	            del_couter = 0;
		}
	}else{
		if(del_couter>3){
		       printf("\n\nDEVICE_del_wifiiface_daemon.\n\n");
			TICKER_del_task(ipcam_wifi_iface_daemon);
		}
		del_couter++;
	}
#endif
}

static void ipcam_wifi_modify_bps(void *arg)
{
	static int del_couter = 0;
	bool model_exist;
	ST_NSDK_NETWORK_INTERFACE n_interface;
	model_exist = APP_WIFI_model_exist();
	NETSDK_conf_interface_get(4, &n_interface);
	if(model_exist && 
		n_interface.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE &&
		n_interface.wireless.wirelessStaMode.wirelessFixedBpsModeEnabled){
		//int bps = APP_WIFI_calculate_bps();
		ST_NSDK_VENC_CH venc_ch;
		NETSDK_conf_venc_ch_get(101, &venc_ch);

		const char *eth = getenv("DEF_ETHER");
		if(!strcmp(eth, "eth0")){//if use eth0,recover the bps and resolution
			printf("use eth0\n");
			//venc_ch.freeResolution = false;
			//venc_ch.resolution = kNSDK_RES_1280X720;
			//venc_ch.constantBitRate = 2048;		
			//NETSDK_conf_venc_ch_set(101, &venc_ch);
			netsdk_venc_ch_changed(101, &venc_ch);
			TICKER_del_task(ipcam_wifi_modify_bps);
		}else if(APP_WIFI_calculate_bps(&venc_ch)){
			venc_ch.freeResolution = false;
			//venc_ch.constantBitRate = bps;
			//NETSDK_conf_venc_ch_set(101, &venc_ch);
			netsdk_venc_ch_changed(101, &venc_ch);
		}
		del_couter = 0;
	}else{
		if(del_couter>3){
			printf("\n del_task:ipcam_wifi_modify_bps!\n");
			TICKER_del_task(ipcam_wifi_modify_bps);
		}
		del_couter++;
	}
}

static void ipcam_wifi_wps_proc(void *arg)
{
	//wifi module init thread
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, "ipcam_wifi_wps_proc");
	if(APP_WIFI_model_exist()){
#if defined(WIFI)
		JN_Wifi_Exit();
		ST_NSDK_NETWORK_INTERFACE n_interface;
		NETSDK_conf_interface_get(4, &n_interface);
		//change to sta mode
		n_interface.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE;
		NETSDK_conf_interface_set(4, &n_interface, eNSDK_CONF_SAVE_RESTART);
#endif
	}

	//JN_Wifi_Exit();
	/*JN_Wifi_STA_Init();
	sleep(1);
	JN_Wifi_Start_Wps(NULL);*/
	
	pthread_exit(NULL);
}

static void ipcam_wps_sta_init()
{
	pthread_t pid = NULL;
	pthread_create(&pid, NULL, ipcam_wifi_wps_proc, NULL);
}


static void ipcam_wps_detect()
{
	static unsigned char old_status = 1;//1:up  0:down
	unsigned char status;
	APP_GPIO_get_pin("factory reset", &status);
	if(1 == old_status && 0 == status){//falling edge
		//do wps
		printf("wps start!\r\n");
		ipcam_wps_sta_init();
	}
	old_status = status;
}


static void ipcam_wifi_gateway_detect()
{
	int i;
	ifconf_interface_t ifconf_irf;
	memset(&ifconf_irf, 0, sizeof(ifconf_interface_t));
	ifconf_get_interface("wlan0", &ifconf_irf);
	char gateway[4] = {0};
	ST_NSDK_NETWORK_INTERFACE n_interface;
	NETSDK_conf_interface_get(4, &n_interface);

	if(NSDK_NETWORK_WIRELESS_MODE_STATIONMODE != n_interface.wireless.wirelessMode){
		return;
	}
	
	if(0 != ifconf_irf.ipaddr.s_addr){
		//get gateway
		for(i = 0; i<4; i++){
			gateway[i] = ifconf_irf.ipaddr.s_b[i]&ifconf_irf.netmask.s_b[i];
		}
		gateway[3] += 1;
		if(0 == is_ip_using(gateway)){
			//there is no AP
			printf("there is no AP:%d.%d.%d.%d\n", gateway[0], gateway[1], gateway[2], gateway[3]);
			NK_SYSTEM("kill -9 `pidof udhcpc`");
			usleep(100*1000);
			NK_SYSTEM("udhcpc -i wlan0 &");
		}
	}else{
		printf("can't get udhcp IP\n");
	}
}

static void ipcam_app_hichip()
{
	char *eth_name = "eth0", *def_eth = "eth0";
	static char def_ifcae[64] = "eth0";
	static int last_status = 1;
	static int del_couter = 0;
	char cmd_str[256];
	int ret = check_nic(eth_name);
	//printf("detect hichip :%s\n", ret?"wlan0":"eth0");
	ST_NSDK_NETWORK_INTERFACE lan, n_interface;
    ifconf_interface_t ifconf_irf;
	NETSDK_conf_interface_get(1, &lan);
    NETSDK_conf_interface_get(4, &n_interface);
	if(APP_WIFI_model_exist()){
		if(ret != last_status){
			last_status = ret;

			if(0 == last_status){
                def_eth = getenv("DEF_ETHER");
				if(def_eth){
					if(strcmp(def_eth, "eth0") == 0){
						return 0;
					}
					else{
						snprintf(def_ifcae, sizeof(def_ifcae), def_eth);
					}
				}
                APP_HICHIP_destroy();
				printf("hichip switch to eth0.\n");
				//0:eth0 is linked

				//use eth0 as default route
				ifconf_interface_t intrface;
				char ip[64]={""}, netmask[64] = {""};
				memset(&intrface, 0, sizeof(intrface));
				ifconf_get_interface("eth0", &intrface);
				if(NULL != intrface.ipaddr.s_addr){
					intrface.ipaddr.s_addr &= intrface.netmask.s_addr;
					snprintf(ip, sizeof(ip), "%s", ifconf_ipv4_ntoa(intrface.ipaddr));
					snprintf(netmask, sizeof(netmask), "%s", ifconf_ipv4_ntoa(intrface.netmask));
					snprintf(cmd_str, sizeof(cmd_str), "route add -net %s netmask %s dev %s", ip, netmask, "eth0");
					NK_SYSTEM(cmd_str);
					intrface.ipaddr.s_addr = inet_addr(lan.lan.staticIP);
					intrface.netmask.s_addr = inet_addr(lan.lan.staticNetmask);
					intrface.gateway.s_addr = inet_addr(lan.lan.staticGateway);
					network_ifconf_set_interface("eth0", &intrface);
					sleep(1);
				}

                // del wlan0 route
				memset(&intrface, 0, sizeof(intrface));
				ifconf_get_interface("wlan0", &intrface);
				if(NULL != intrface.ipaddr.s_addr){
					intrface.ipaddr.s_addr &= intrface.netmask.s_addr;
					snprintf(ip, sizeof(ip), "%s", ifconf_ipv4_ntoa(intrface.ipaddr));
					snprintf(netmask, sizeof(netmask), "%s", ifconf_ipv4_ntoa(intrface.netmask));
					snprintf(cmd_str, sizeof(cmd_str), "route del -net %s netmask %s dev %s", ip, netmask, "wlan0");
					NK_SYSTEM(cmd_str);
				}

				snprintf(cmd_str, sizeof(cmd_str), "route del default gw 0.0.0.0 dev eth0");
				NK_SYSTEM(cmd_str);

                snprintf(cmd_str, sizeof(cmd_str), "route add default gw %s dev eth0", lan.lan.staticGateway);
                NK_SYSTEM(cmd_str);

				snprintf(def_ifcae, sizeof(def_ifcae), "eth0");
				setenv("DEF_ETHER", def_ifcae, true);

				ifconf_get_interface(def_ifcae, &ifconf_irf);
				snprintf(cmd_str, sizeof(cmd_str), "%02x:%02x:%02x:%02x:%02x:%02x", ifconf_irf.hwaddr.s_b[0], ifconf_irf.hwaddr.s_b[1],
				 ifconf_irf.hwaddr.s_b[2], ifconf_irf.hwaddr.s_b[3], ifconf_irf.hwaddr.s_b[4],ifconf_irf.hwaddr.s_b[5]);
				setenv("DEVICE_MAC", cmd_str, true);

				APP_HICHIP_init();
			}else{
				//use wireless interface
                snprintf(def_ifcae, sizeof(def_ifcae), "wlan0");
				def_eth = getenv("DEF_ETHER");
				if(strcmp(def_eth, def_ifcae) == 0){
					return 0;
				}
				APP_HICHIP_destroy();
				printf("hichip switch to %s.\n", def_ifcae);
				setenv("DEF_ETHER", def_ifcae, true);

				ifconf_interface_t intrface;
				char ip[64]={""}, netmask[64] = {""}, new_ip[64], new_gw[64];
#if defined(DHCP)
				memset(&intrface, 0, sizeof(intrface));
				ifconf_get_interface("eth0", &intrface);
				if(NULL != intrface.ipaddr.s_addr){
					snprintf(new_ip, sizeof(new_ip), "%s", ifconf_ipv4_ntoa(intrface.ipaddr));
					snprintf(new_gw, sizeof(new_gw), "%s", ifconf_ipv4_ntoa(intrface.gateway));
					intrface.ipaddr.s_addr &= intrface.netmask.s_addr;
					snprintf(ip, sizeof(ip), "%s", ifconf_ipv4_ntoa(intrface.ipaddr));
					snprintf(netmask, sizeof(netmask), "%s", ifconf_ipv4_ntoa(intrface.netmask));
					snprintf(cmd_str, sizeof(cmd_str), "route del -net %s netmask %s dev %s", ip, netmask, "eth0");
					NK_SYSTEM(cmd_str);
					if((strcmp(new_gw, lan.lan.staticGateway) != 0 || strcmp(new_ip, lan.lan.staticIP) != 0)
						&& MATCH_GATEWAY(new_ip, netmask, new_gw)){
						snprintf(lan.lan.staticGateway, sizeof(lan.lan.staticGateway), "%s", new_gw);
						snprintf(lan.lan.staticIP, sizeof(lan.lan.staticIP), "%s", new_ip);
					    printf("IP-GW:%s-%s\n", lan.lan.staticIP, lan.lan.staticGateway);
					    NETSDK_conf_interface_set(1, &lan, eNSDK_CONF_SAVE_JUST_SAVE);
				    }
                }
#endif
				snprintf(cmd_str, sizeof(cmd_str), "route del default gw 0.0.0.0 dev eth0");
				NK_SYSTEM(cmd_str);

                // add wlan0 route
				memset(&intrface, 0, sizeof(intrface));
				ifconf_get_interface("wlan0", &intrface);
				if(NULL != intrface.ipaddr.s_addr){
					intrface.ipaddr.s_addr &= intrface.netmask.s_addr;
					snprintf(ip, sizeof(ip), "%s", ifconf_ipv4_ntoa(intrface.ipaddr));
					snprintf(netmask, sizeof(netmask), "%s", ifconf_ipv4_ntoa(intrface.netmask));
					snprintf(cmd_str, sizeof(cmd_str), "route del -net %s netmask %s dev %s", ip, netmask, "wlan0");
					NK_SYSTEM(cmd_str);
					snprintf(cmd_str, sizeof(cmd_str), "route add -net %s netmask %s dev %s", ip, netmask, "wlan0");
					NK_SYSTEM(cmd_str);
				}

				TICKER_add_task(ipcam_wifi_modify_bps, 5, true);
				APP_HICHIP_init();

				ifconf_get_interface(def_ifcae, &ifconf_irf);
				snprintf(cmd_str, sizeof(cmd_str), "%02x:%02x:%02x:%02x:%02x:%02x", ifconf_irf.hwaddr.s_b[0], ifconf_irf.hwaddr.s_b[1],
				 ifconf_irf.hwaddr.s_b[2], ifconf_irf.hwaddr.s_b[3], ifconf_irf.hwaddr.s_b[4],ifconf_irf.hwaddr.s_b[5]);
				setenv("DEVICE_MAC", cmd_str, true);
			}
		}
		del_couter = 0;
	}else{
		if(del_couter>3){
			TICKER_del_task(ipcam_app_hichip);
		}
		del_couter++;
	}
}

static void ipcam_watchdog_autofeed()
{
	WATCHDOG_feed();
}

static void ipcam_get_alarm_in()
{
	bool pin_status;
	APP_GPIO_get_pin("alarm in 0", &pin_status);
	//printf("alarm in:%d\r\n", pin_status);
	if(BSP_Alarm(ALARM_IN, 1, 0)){
		HICHIP_set_io_alarm_status(true);
	}

	printf("alarm in:%d\r\n", BSP_Alarm(ALARM_IN, 1, 0));

	static  int low_level_cnt = 0;
	int ret = -1;
	ret = ReadPIRResult();
	if(1 ==ret){
		printf("alarm in:%d\r\n",ret);
		low_level_cnt = 0;
		HICHIP_set_io_alarm_status(true);
	}else if (0 == ret){
		printf("alarm in:%d\r\n",ret);
		low_level_cnt ++;
		if(low_level_cnt >=3 ){
			low_level_cnt = 0;
			HICHIP_set_io_alarm_status(false);
		}
	}else if(-1 == ret){
		low_level_cnt = 0;
		printf("alarm return :-1  READ ERRO\r\n");

	}
}

static void ipcam_eseeclient_report()
{
	ESEE_CLIENT_INFO_t info;
	if(0 == ESEE_CLIENT_get_info(&info)){
		/*
		int i = 0;
		printf("id = %s\r\n", info.id);
		printf("ip4 = %s\r\n", info.ip4);
		printf("heartbeat = %u\r\n", info.heartbeat_port);
		printf("port total map: %d\r\n", info.port_cnt);
		for(i = 0; i < info.port_cnt; ++i){
			printf("port(%d) = %u\r\n", i, info.port[i]);
		}*/
	}
}

static void ipcam_ircut_auto_switch()
{
	//FIX ME
	SENSOR_ircut_auto_switch(1, 1);
}

static void ipcam_get_sensor_gains()
{
	SENSOR_get_gain();
}

static void get_ip_conflict_proc(void *args)
{
	ifconf_interface_t ifconf_irf;
	char def_eth[128];
	unsigned char ip[32];

	if(NULL == getenv("DEF_ETHER")){
		snprintf(def_eth, sizeof(def_eth), "eth0");
	}else{
		snprintf(def_eth, sizeof(def_eth), "%s", getenv("DEF_ETHER"));
	}

	ifconf_get_interface(def_eth, &ifconf_irf);
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, "get_ip_conflict_proc");
	snprintf(ip, sizeof(ip), "%s", ifconf_ipv4_ntoa(ifconf_irf.ipaddr));
	nk_net_adapt_ip_set_conflict_flag(NET_check_ip_conflict(ip));
	pthread_exit(0);
}

static void ipcam_get_ip_conflict()
{
	ST_NSDK_NETWORK_INTERFACE n_interface;
	NETSDK_conf_interface_get(1, &n_interface);
	if(n_interface.lan.addressingType != kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC){
		int pid;
		pthread_create(&pid, NULL, get_ip_conflict_proc, NULL);
	}
}

static void ipcam_send_hello()
{
	char eth_name[] = "eth0";
	static int last_status = 0;
	int ret = check_nic(eth_name);
	if(ret != last_status){
		last_status = ret;
 
		if(0 == last_status){	
			//0:eth0 is linked
/* FIX ME: onvif private function*/
#if defined(ONVIFNVT)
extern int ONVIF_send_hello();
			ONVIF_send_hello(); 
#endif
		}else{
			//0:eth0 isn't  linked 
		}
	}
 
}

static void ipcam_get_interface_conflict()
{
	unsigned char netclass = 0;
	ST_NSDK_NETWORK_INTERFACE lan, vlan;
	unsigned int eth0_ip = 0, eth0_1_ip = 0;
	struct in_addr addr;
	NETSDK_conf_interface_get(1, &lan);
	NETSDK_conf_interface_get(2, &vlan);

	eth0_ip = inet_addr(lan.lan.staticIP);
	eth0_1_ip = inet_addr(vlan.lan.staticIP);
	if((eth0_ip & 0xFFFFFF) == (eth0_1_ip & 0xFFFFFF))
	{
		netclass = eth0_ip & 0x0FF;
		if(0 <= netclass && 127 > netclass) {
			netclass = 172;
		}else if(127 <= netclass && 192 > netclass) {
			netclass = 192;
		}else if (192 <= netclass) {
			netclass = 10;
		}else{
			netclass = 193;
		}

		eth0_1_ip = (eth0_1_ip & 0xFFFFFF00) | (netclass & 0xFF);
		memcpy(&addr, &eth0_1_ip, sizeof(unsigned int));		
		memset(vlan.lan.staticIP, 0, sizeof(vlan.lan.staticIP));
		sprintf(vlan.lan.staticIP, "%s", inet_ntoa(addr));
		printf("Change vlan IP to %s\n", vlan.lan.staticIP);
		NETSDK_conf_interface_set(2, &vlan, eNSDK_CONF_SAVE_RESTART);
	}
}

static void ipcam_get_interface_conflict2()
{
	unsigned char netclass = 0;
	ifconf_interface_t intrface;
	ST_NSDK_NETWORK_INTERFACE wlan, vlan;
	unsigned int eth0_ip = 0, eth0_1_ip = 0;
	struct in_addr addr;
#if defined(WIFI)
	NETSDK_conf_interface_get(1, &vlan);
	NETSDK_conf_interface_get(4, &wlan);
	if(APP_WIFI_model_exist() && wlan.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE
		&& vlan.lan.addressingType != kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC){
		ifconf_get_interface("wlan0", &intrface);
		
		eth0_ip = intrface.ipaddr.s_addr;
		eth0_1_ip = inet_addr(vlan.lan.staticIP);
		if((eth0_ip & 0xFFFF) == (eth0_1_ip & 0xFFFF))
		{
			netclass = eth0_ip & 0x0FF;
			if(0 <= netclass && 127 > netclass) {
				netclass = 172;
			}else if(127 <= netclass && 192 > netclass) {
				netclass = 192;
			}else if (192 <= netclass) {
				netclass = 10;
			}else{
				netclass = 193;
			}

			eth0_1_ip = (eth0_1_ip & 0xFFFFFF00) | (netclass & 0xFF);
			memcpy(&addr, &eth0_1_ip, sizeof(unsigned int));		
			memset(vlan.lan.staticIP, 0, sizeof(vlan.lan.staticIP));
			sprintf(vlan.lan.staticIP, "%s", inet_ntoa(addr));
			printf("WIFI-Change lan IP to %s\n", vlan.lan.staticIP);
			NETSDK_conf_interface_set(1, &vlan, eNSDK_CONF_SAVE_RESTART);
			TICKER_del_task(ipcam_get_interface_conflict2);
			
		}
	}
#endif
}

static void ipcam_check_sta_status()
{
// wifi¡¨Ω”◊¥Ã¨ºÏ≤‚¬ﬂº≠“—∏¸ªªŒ™wpa∑Ω Ω£¨
// ºŸ»Á“™ª÷∏¥’‚¿Ôµƒ≈–∂œ¬ﬂº≠–Ë“™–ﬁ∏ƒµƒµÿ∑Ωª·±»Ωœ∂‡£¨“ÚŒ™’‚¿Ôµƒ¬ﬂº≠ «±»ΩœªÏ¬“
#if 0
#if defined(WIFI)

/*
¥À∫Í∂®“Â ˝÷µ∏˘æ›¥À»ŒŒÒµƒ∂® ±÷µ∂¯∂®£¨
ƒø±Í◊ˆµΩ≈‰÷√¬∑”…µº÷¬…Ë±∏¥Û‘º20√Î∂ººÏ≤ÈµΩ
wifi¡¨Ω” ß∞‹æÕ«–ªªµΩapƒ£ Ω÷ÿ–¬≈‰÷√
(∂® ±÷µ) * PLAY_FALSE_NUM_MAX   = ‘º20√Î
*/
#define PLAY_FALSE_NUM_MAX  2       // ≤•∑≈¡¨Ω” ß∞‹◊Ó¥Û¥Œ ˝

    char cmd[128];
	static NK_Int play_false_num = 0;
	int ret;
    char *check_sta_iface = NSDK_NETWORK_WIRELESS_MODE_STA_ETH;
    ST_NSDK_NETWORK_INTERFACE wlan;
    static int check_connect_times = 0;

    NETSDK_conf_interface_get(4, &wlan);
	if(APP_WIFI_model_exist() && wlan.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE){
		ret = APP_WIFI_check_sta_status(check_sta_iface);
        struct linked_ap_info link_info;
        NK_WIFI_get_linked_AP_info(&link_info, check_sta_iface);
        if(1 == ret && (link_info.linkstatus == 1 && link_info.sinal_strength >= 10)){
			if(!IS_FILE_EXIST(WIFI_CONNECT_FLAG)) {
                check_connect_times ++;
                if(check_connect_times > 1){
    				SearchFileAndPlay(SOUND_WiFi_connection_completed, NK_False);

                    /* ¥¥Ω®WIFI_CONNECT_FLAG±Í÷æŒƒº˛£¨±Ì æwifi¡¨Ω”≥…π¶£¨
                        Õ£÷π≈–∂œ20√Î¡¨Ω”wifi ß∞‹¬ﬂº≠ */
                    snprintf(cmd, sizeof(cmd), "echo success > %s", WIFI_CONNECT_FLAG);
    		        NK_SYSTEM(cmd);

                    /* ¥¥Ω®AUDIO_OUTPUT_ENABLEŒƒº˛°£Õ£÷πÃ· æ“Ù≤•∑≈ */
                    snprintf(cmd, sizeof(cmd), "echo true > %s", AUDIO_OUTPUT_ENABLE);
    		        NK_SYSTEM(cmd);
                    check_connect_times = 0;
				}
			}
#ifdef LED_CTRL
			initLedContrl(DEF_LED_ID,true,LED_LIGHT_MODE);
#endif
			play_false_num = 0;
		}else{
            /*
                Â¶ÇÊûúWIFI_CONNECT_FLAGÊñá‰ª∂Â≠òÂú®ÂàôË°®ÊòéËÆæÂ§áÂ∑≤ÈÖçÁΩÆÂπ∂ÊàêÂäüËøûÊé•ËøáË∑ØÁî± 
                ÊâÄ‰ª•‰∏çÊí≠ÊîæÊèêÁ§∫Èü≥Ôºå‰πü‰∏çÂÅö20ÁßíÂà§Êñ≠ËøûÊé•Â§±Ë¥•ÂêéÂàáÊç¢Âà∞apÊ®°Âºè
                **ÂêéÈù¢ÊúâÂèØËÉΩ‰ºöÂ¢ûÂä†ÂÖ∂ÂÆÉËøûÊé•Â§±Ë¥•ÁöÑÈÄªËæë
            */
            if(IS_FILE_EXIST(WIFI_CONNECT_FLAG)) {
                play_false_num = 0;
                return;
            }

            if(play_false_num < PLAY_FALSE_NUM_MAX){
                SearchFileAndPlay(SOUND_WiFi_connection_failed, NK_False);
				SearchFileAndPlay(SOUND_Please_setup_again, NK_False);
                play_false_num++;
            }

            /*
                ≤•∑≈PLAY_FALSE_NUM_MAX¥Œ¡¨Ω” ß∞‹∫Û£¨
                º¥20√Î◊Û”“∫Û£¨◊‘∂Ø«–ªªµΩapƒ£ Ω
            */
            if(play_false_num == PLAY_FALSE_NUM_MAX) {
                play_false_num = 0;
				IPCAM_network_switch_to_ap();
            }
		}
	}else{
		play_false_num = 0;
	}
#endif
#endif
}

static void ipcam_compare_ip()
{
	ST_NSDK_NETWORK_INTERFACE net_n;
	ifconf_interface_t intrface_ifconf;
    int ether_iface_index = 1;
	char *ether_iface = getenv("DEF_ETHER");
    ST_PRODUCT_TEST_INFO product_info;

//#if defined(WIFI)
    if(strcmp(ether_iface, "eth0") != 0) {
        ether_iface_index = 4;
    }

    if(0 != NETSDK_tmp_interface_get(1, &net_n)) // Virtual IP‰∏çËøõË°å‰øùÂ≠ò
    {
        return;
    }

	memset(&intrface_ifconf, 0, sizeof(intrface_ifconf));

	NETSDK_conf_interface_get(ether_iface_index, &net_n);//get net config

	if(net_n.lan.addressingType != kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC){
		return;
	}

	ifconf_get_interface(ether_iface, &intrface_ifconf);
	if(intrface_ifconf.ipaddr.s_addr == 0 || intrface_ifconf.ipaddr.s_addr == -1
		|| intrface_ifconf.netmask.s_addr == 0 || intrface_ifconf.netmask.s_addr == -1
		|| intrface_ifconf.gateway.s_addr == 0 || intrface_ifconf.gateway.s_addr == -1){
		return;
	}

    if((0 == PRODUCT_TEST_getWifi(&product_info)) && (0 < strlen(product_info.staEssid)))
    {
        NETSDK_tmp_interface_get(4, &net_n);
        if(inet_addr(net_n.lan.staticIP) != intrface_ifconf.ipaddr.s_addr)
        {
            snprintf(net_n.lan.staticIP, sizeof(net_n.lan.staticIP), "%d.%d.%d.%d",
                intrface_ifconf.ipaddr.s_b[0], intrface_ifconf.ipaddr.s_b[1], intrface_ifconf.ipaddr.s_b[2], intrface_ifconf.ipaddr.s_b[3]);
            NETSDK_tmp_interface_set(4, &net_n, eNSDK_CONF_SAVE_JUST_SAVE);
            APP_HICHIP_destroy();
            APP_HICHIP_init();
        }
    }
    else
    {
        if(inet_addr(net_n.lan.staticIP) !=  intrface_ifconf.ipaddr.s_addr){
            if((intrface_ifconf.ipaddr.s_addr & intrface_ifconf.netmask.s_addr) == (intrface_ifconf.gateway.s_addr & intrface_ifconf.netmask.s_addr)){
                snprintf(net_n.lan.staticIP, sizeof(net_n.lan.staticIP), "%d.%d.%d.%d",
                    intrface_ifconf.ipaddr.s_b[0], intrface_ifconf.ipaddr.s_b[1], intrface_ifconf.ipaddr.s_b[2], intrface_ifconf.ipaddr.s_b[3]);
                snprintf(net_n.lan.staticGateway, sizeof(net_n.lan.staticGateway), "%d.%d.%d.%d",
                    intrface_ifconf.gateway.s_b[0], intrface_ifconf.gateway.s_b[1], intrface_ifconf.gateway.s_b[2], intrface_ifconf.gateway.s_b[3]);
                snprintf(net_n.lan.staticNetmask, sizeof(net_n.lan.staticNetmask), "%d.%d.%d.%d",
                    intrface_ifconf.netmask.s_b[0], intrface_ifconf.netmask.s_b[1], intrface_ifconf.netmask.s_b[2], intrface_ifconf.netmask.s_b[3]);
                NETSDK_conf_interface_set(ether_iface_index, &net_n, eNSDK_CONF_SAVE_JUST_SAVE);
            }
            APP_HICHIP_destroy();
            APP_HICHIP_init();
        }
    }
//#endif
}

static void ipcam_timer_stop_sound_wave()
{
	TICKER_del_task(ipcam_timer_stop_sound_wave);
#if defined(SOUND_WAVE)
//	SW_destroy();
#endif
}

/*
  ∂® ±ntpÕ¨≤ΩœµÕ≥ ±º‰
*/
static void ipcam_timer_ntp_sync()
{
    ST_NSDK_SYSTEM_TIME sys_time;
    if(NETSDK_conf_system_get_time(&sys_time)) {
        if(sys_time.ntpEnabled) {
            NTP_start(sys_time.ntpServerDomain, sys_time.ntpServerBackupOne, sys_time.ntpServerBackupTwo, NULL, 0, 0); // Âõ†‰∏∫ÊúÄÊñ∞ÁöÑ‰øÆÊîπÔºåntpÂêåÊ≠•Êó∂‰∏çËÆæÁΩÆÊó∂Âå∫ÔºåÊâÄ‰ª•ÂèÇÊï∞4‰∏çÁîüÊïà
        }
    }

}

/*
    ∂® ±±£¥ÊœµÕ≥ ±º‰µΩŒƒº˛÷–
*/
static void ipcam_timer_save_recover_time()
{
    GLOBAL_saveFileFromTime();

}

static void ipcam_timer_gsensor_check()
{
    double p_angle_x, p_angle_y, p_angle_z;
    int angle_z = 0;
    int cur_fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL;
    int new_fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL;
    int main_resolution;
    ST_NSDK_VENC_CH venc;
    ST_NSDK_IMAGE image;
#if defined(GSENSOR)
    if(GSENSOR_is_init()) {
        GSENSOR_get_angles(&p_angle_x, &p_angle_y, &p_angle_z);

        cur_fixMode = FISHEYE_get_fix_mode();
        angle_z = abs(p_angle_z) / 10;
        if(angle_z >= 40) {
            new_fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL;
            main_resolution = kNSDK_RES_1536X784;
        }
        else {
            new_fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL;
            main_resolution = kNSDK_RES_1536X1536;
        }
        if(new_fixMode != cur_fixMode) {
            NETSDK_conf_image_get(&image);
            image.videoMode.fixType = new_fixMode;
            NETSDK_conf_image_set2(&image, true);

            NETSDK_conf_venc_ch_get(101, &venc);
            venc.resolution = main_resolution;
            NETSDK_conf_venc_ch_set2(101, &venc, true);
            netsdk_venc_ch_changed(101, &venc);
        }
    }
#endif

}

int IPCAM_timer_init()
{
	TICKER_add_task(ipcam_watchdog_autofeed, 5, true);
	TICKER_add_task(ipcam_ircut_auto_switch, 1, true);
	TICKER_add_task(ipcam_get_sensor_gains, 1, true);
//	TICKER_add_task(ipcam_get_alarm_in, 1, true);
	TICKER_add_task(ipcam_app_hichip, 1, true);
	TICKER_add_task(ipcam_wifi_modify_bps, 5, true);
	//TICKER_add_task(ipcam_close_ip_auto_adapted, 6*3600, true);//close onvif ip adapt after 12 hours
	TICKER_add_task(ipcam_get_ip_conflict, 30, true);
	//TICKER_add_task(ipcam_wps_detect, 1, true);
	//TICKER_add_task(ipcam_wifi_gateway_detect, 30, true);
	//TICKER_add_task(ipcam_wifi_iface_daemon, 10, false);
	//TICKER_add_task(ipcam_check_sta_status, 5, false);
#if !defined(HI3516E_V1)
	TICKER_add_task(ipcam_send_hello, 1, false);
#endif
	//TICKER_add_task(ipcam_get_interface_conflict, 10, true);
	//TICKER_add_task(ipcam_get_interface_conflict2, 10, true);
	TICKER_add_task(ipcam_compare_ip, 5, true);
	//TICKER_add_task(ipcam_timer_stop_sound_wave, 600, false);
    TICKER_add_task(ipcam_timer_ntp_sync, 43200, false);
    TICKER_add_task(ipcam_timer_save_recover_time, 300, false);
    //TICKER_add_task(ipcam_timer_gsensor_check, 1, false);
	return 0;
}

void IPCAM_timer_destroy()
{
    TICKER_del_task(ipcam_check_sta_status);
	//TICKER_del_task(ipcam_wifi_iface_daemon);
    TICKER_del_task(ipcam_wifi_modify_bps);
	TICKER_del_task(ipcam_app_hichip);
	TICKER_del_task(ipcam_get_alarm_in);
	TICKER_del_task(ipcam_get_sensor_gains);
	TICKER_del_task(ipcam_ircut_auto_switch);
	//TICKER_del_task(ipcam_watchdog_autofeed);
	TICKER_del_task(ipcam_timer_stop_sound_wave);
    TICKER_del_task(ipcam_timer_ntp_sync);
    TICKER_del_task(ipcam_timer_save_recover_time);
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
}

void IPCAM_timer_sdk_destroy()
{
     TICKER_del_task(ipcam_get_sensor_gains); 
	 TICKER_del_task(ipcam_ircut_auto_switch);
}

void IPCAM_timer_network_destroy()
{
	TICKER_del_task(ipcam_app_hichip);
	//TICKER_del_task(ipcam_wifi_iface_daemon);
}

void IPCAM_timer_network_init()
{
	TICKER_add_task(ipcam_app_hichip, 1, true);
	//TICKER_add_task(ipcam_wifi_iface_daemon, 2, true);
}

void IPCAM_timer_open_wifi_modify_bps()
{
	TICKER_del_task(ipcam_wifi_modify_bps);
	TICKER_add_task(ipcam_wifi_modify_bps, 5, true);
}

void IPCAM_timer_destory_wifi_modify_bps()
{
	TICKER_del_task(ipcam_wifi_modify_bps);
}

/*
	÷ª”–‘⁄“‘œ¬«Èøˆœ¬ø™∆Ùwifi sta ¡¨Ω”◊¥Ã¨ºÏ≤‚
	1.ipc≥ı ºªØ ±£¨wifi¡¨Ω”∑Ω Ω «staƒ£ Ωø™∆Ù,apƒ£ Ω≤ªø™∆Ù
	2.APªÚ…˘≤®≈‰÷√ÕÍ±œ£¨∏ƒŒ™staƒ£ Ω∫Û
*/
void IPCAM_timer_check_sta_status_start()
{
	TICKER_del_task(ipcam_check_sta_status);
	TICKER_add_task(ipcam_check_sta_status, 10, false);

}

/*
	‘⁄“‘œ¬«Èøˆœ¬πÿ±’wifi sta ¡¨Ω”◊¥Ã¨ºÏ≤‚£¨±‹√‚“Ï≤Ωµº÷¬µƒÃ· æ“Ù¥Ì±®Œ Ã‚
	»Á£¨Ã· æ“Ù±®ª÷∏¥≥ˆ≥ß…Ë÷√£¨»ª∫Û”÷±®Œﬁœﬂ¡¨Ω” ß∞‹
	1.«–ªªµΩAPƒ£ Ω
	2.ª÷∏¥≥ˆ≥ß…Ë÷√
*/
void IPCAM_timer_check_sta_status_stop()
{
	TICKER_del_task(ipcam_check_sta_status);

}

