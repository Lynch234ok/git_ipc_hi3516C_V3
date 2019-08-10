/*
* Wifi 
* scanning wifi message
*/
#include "sta.h"
#include "ap.h"
#include "wifi.h"
#include "base/ja_process.h"

int JN_Wifi_GetBssid(char bssid[48])
{
	FILE * fp; 
	int res;
	char *p, *ptr, *ptr2;
	char buf[1024];

	memset(buf, '\0', sizeof(buf));
	
	if ((fp = popen("ifconfig", "r") ) == NULL) 
	{ 
		perror("popen");
		printf("popen error: %s/n", strerror(errno)); 
		return -1; 
	} 
	else
	{
		while(fgets(buf, sizeof(buf), fp)) 
		{ 			
			//get bssid
			ptr = strstr(buf, "wlan0"); 
			if(ptr)
			{ 
				ptr2 = strstr(ptr, "HWaddr");
				if(ptr2)
				{
					p = strtok(ptr2," ");
					p = strtok(NULL, "\n");
					strcpy(bssid, p);
				}
			}			
		} 
		if ( (res = pclose(fp)) == -1) 
		{ 
			printf("close popen file pointer fp error!\n"); 
			return res;
		} 
		else if (res == 0) 
		{
			return res;
		} 
		else 
		{ 
			printf("popen res is :%d\n", res); 
			return res; 
		} 
	}
	return 0;
}

int JN_Wifi_STA_Search(struct wifi_struct tswifi[100])
{
	int i;
	
	ScanApp("iwlist wlan0 scanning | grep -E 'Cell|ESSID|PSK|Version|Quality'", tswifi);

	i = 1;
	while(strcmp(tswifi[i].cellNo,"")!=0)
	{
		printf("%s,%s,%s,%s,%s,%s,%s,%s\n",tswifi[i].cellNo,tswifi[i].essid,tswifi[i].key,tswifi[i].quality,tswifi[i].signalLevel,tswifi[i].wpa1,tswifi[i].wpa2,tswifi[i].bssid);
		i++;
	}
	
	return 0;
}

int JN_Wifi_STA_Getstatus()
{
	sleep(4);
	
	CheckSTA("wlan0");   //check STA
	
	return 0;
}

int JN_Wifi_Start(char *pMode, struct ap_struct *tsap, struct sta_struct *tssta)
{
	int ret = 0;
	if(!strcmp(pMode, "AP") && tsap)
	{
		AP_setHostapd(tsap,"/tmp/ap.conf"); //set the AP configuration

		AP_openAP("/tmp/ap.conf");   //open AP (hostapd)
		//sleep(2);
		usleep(200000);
	
		AP_setUdhcpd(tsap, "wlan0", "/tmp/udhcpd.conf");  //set udhcpd configure
	
		AP_openUdhcpd("/tmp/udhcpd.conf");   //open udhcpd

	}
	if(!strcmp(pMode, "STA") && tssta)
	{
		STA_setupSTA(tssta->essid, tssta->key, "/tmp/wpa_supplicant.conf");   //wpa_passphrase STA
	
		//sleep(1);
		usleep(100000);
	
		MegCheck("wext", "wlan0", "/tmp/wpa_supplicant", "/tmp/wpa_supplicant.conf");   //message check (wpa_supplicant)

		//sleep(6);
		usleep(200000);

		char str_cmd[128];
		sprintf(str_cmd, "ifconfig wlan0 %s netmask %s", tssta->staticIp, tssta->staticNetmask);
		NK_SYSTEM(str_cmd);
		if(tssta->dhcpEnabled){
			ret = STA_openUdhcpc("wlan0", tssta->staticIp, NK_USER_CONFIG_UDHCPC_SCRIPT);	//open udhcpc
			printf("JN_Wifi_Start : STA_openUdhcpc\n");
		}
	}
	
	return ret;
}

int JN_Wifi_Stop(char *pMode)
{
	if(!strcmp(pMode, "AP"))
	{
		NK_SYSTEM("kill -9 `pidof hostapd`");
		NK_SYSTEM("kill -9 `pidof udhcpd`");
	}
	if(!strcmp(pMode, "STA"))
	{
		NK_SYSTEM("kill -9 `pidof wpa_supplicant`");
	}
	
	return 0;
}

int JN_Wifi_AP_Setparam(struct ap_struct *tsap, char *essid, char *wpa_passphrase, int hw_mode, int wpa_key_mgmt, int channel, char *ipStart, char *ipNumber, char *dns, char *subnet, char *router)
{
	strcpy(tsap->essid,essid);
	strcpy(tsap->wpa_passphrase,wpa_passphrase);
	strcpy(tsap->ipStart,ipStart);
	strcpy(tsap->ipNumber,ipNumber);
	strcpy(tsap->dns,dns);
	strcpy(tsap->subnet,subnet);
	strcpy(tsap->router,router);   
	switch(hw_mode)
	{
		case 1<<0:
			strcpy(tsap->hw_mode,"b");
			break;
		case 1<<1:
			strcpy(tsap->hw_mode,"g");
			break;
		case 1<<2:
			strcpy(tsap->hw_mode,"n");
			break;
		case 1<<3:
			strcpy(tsap->hw_mode,"bg");
			break;
		case 1<<4:
			strcpy(tsap->hw_mode,"bgn");
			break;
	}
	switch(wpa_key_mgmt)
	{
		case 1<<0:
			strcpy(tsap->wpa_key_mgmt,"WPA-PSK");
			break;
		case 1<<1:
			strcpy(tsap->wpa_key_mgmt,"WPA2-PSK");
			break;
	}
	
	switch(channel)
	{
		case 0:
			strcpy(tsap->channel,"1");
			break;
		default:
			sprintf(tsap->channel, "%d", channel);
			break;
	}
	
	return 0;
}

int JN_Wifi_STA_Setparam(struct sta_struct *tssta, char *essid, char *key)
{
	strcpy(tssta->essid,essid);
	strcpy(tssta->key, key);
	
	return 0;
}
/*
int JN_Wifi_SetMode(char *pMode)
{
	strcpy(mode, pMode);
	return 0;
}

int JN_Wifi_GetMode(char *pMode)
{
	strcpy(pMode, mode);
	return 0;
}
*/

//init AP
int JN_Wifi_AP_Init(char *wlanIp, char *wlanNetmask, char *wlanGateway, char *wlanDns)
{
	char str[200];
	memset(str, 0, sizeof(str));

	strcpy(str, "ifconfig wlan0 up;");
	//ip
	sprintf(str, "%s ifconfig wlan0 %s netmask %s;", str, wlanIp, wlanNetmask);
	//dns
	sprintf(str, "%s echo nameserver %s > /tmp/resolv.conf;", str, wlanDns);
	//gateway
	//sprintf(str, "%s route add default gw %s;", str, wlanGateway);
	
	NK_SYSTEM(str);

	return 0;
}


//init STA
int JN_Wifi_STA_Init()
{
	char str[200];
	memset(str, 0, sizeof(str));

	strcpy(str, "ifconfig wlan0 up;");

	NK_SYSTEM(str);

	return 0;
}


int JN_Wifi_Exit(void)
{
	NK_SYSTEM("ifconfig wlan0 down");
	NK_SYSTEM("kill -9 `pidof wpa_supplicant`");
	NK_SYSTEM("kill -9 `pidof udhcpc`");
	NK_SYSTEM("kill -9 `pidof hostapd`");
	NK_SYSTEM("kill -9 `pidof udhcpd`");
	return 0;
}

int JN_Wifi_AP_Setup(char *essid, char *wpa_passphrase, int hw_mode, int wpa_key_mgmt, int channel, char *ipStart, char *ipNumber, char *dns, char *subnet, char *router)
{
	struct wifi_struct swifi[100];
	struct sta_struct ssta;
	struct ap_struct sap;
	int i;

	for(i=0; i<100; i++)
	{
		memset(&swifi[i], 0, sizeof(struct wifi_struct));
	}
	
	JN_Wifi_Stop("AP");

	JN_Wifi_AP_Setparam(&sap, essid, wpa_passphrase, hw_mode, wpa_key_mgmt, channel, ipStart, ipNumber, dns, subnet, router);
	
	JN_Wifi_Start("AP", &sap, &ssta);

	return 0;
}

int JN_Wifi_STA_Setup(char *essid, char *key, char *ip, char *netmask, bool dhcpEnabled)
{
	struct sta_struct ssta;
	struct ap_struct sap;
	int i;
	
	JN_Wifi_Stop("STA");
	//JN_Wifi_STA_Setparam(&ssta, essid, key);
	snprintf(ssta.essid, sizeof(ssta.essid), "%s", essid);
	snprintf(ssta.key, sizeof(ssta.key), "%s", key);
	snprintf(ssta.staticIp, sizeof(ssta.staticIp), "%s", ip);
	snprintf(ssta.staticNetmask, sizeof(ssta.staticNetmask), "%s", netmask);
	ssta.dhcpEnabled = dhcpEnabled;
	
	JN_Wifi_Start("STA", &sap, &ssta);

	return 0;
}

int JN_Wifi_Concurrent_Set_Bridge_Off(char *ether1, char *ether2)
{
	char str[200];
	if(!ether1 || !ether2){
		return -1;
	}

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), 
		"brctl  delif  br0 %s;brctl  delif  br0 %s;ifconfig br0 down;brctl  delbr  br0", ether1, ether2);
	NK_SYSTEM(str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "ifconfig %s 0.0.0.0;", ether1);
	NK_SYSTEM(str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "ifconfig %s 0.0.0.0;", ether2);
	NK_SYSTEM(str);

	return 0;
}

int JN_Wifi_Concurrent_Set_Bridge(char *ether1, char *ether2, char *br_ip, char *netmask)
{
	char str[200];
	if(!ether1 || !ether2 || !br_ip){
		return -1;
	}

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), 
		"brctl  addbr  br0;ifconfig br0 up;brctl  addif  br0 %s;brctl  addif  br0 %s;ifconfig br0 %s netmask %s", ether1, ether2, br_ip, netmask);
	NK_SYSTEM(str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "ifconfig %s 0.0.0.0;", ether1);
	NK_SYSTEM(str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "ifconfig %s 0.0.0.0;", ether2);
	NK_SYSTEM(str);

	return 0;
}

int JN_Wifi_AP_Get_Connected_MAC(char *ifcae, int num, unsigned char mac[][6], char *wifi_type)
{
	FILE * fp; 
	int res;
	char *p, *ptr;
	char buf[256] = {0};
	int index = -1;
	snprintf(buf, sizeof(buf), "%s/wifi_tools/hostapd_cli -p /tmp/hostapd/ -i%s all_sta", IPCAM_ENV_HOME_DIR, ifcae ? ifcae : "wlan0");
	
	if((fp = popen(buf, "r")) == NULL){ 
		perror("[popen] hostapd_cli -iwlan0 all_sta");
		return -1; 
	}
	else{
		index = 0;
		while(fgets(buf, sizeof(buf), fp)) {
			
			if((strcmp(wifi_type, "rtl8188fu") == 0) || (strcmp(wifi_type, "rtl8188eu") == 0)){
				//for rtl8188fu(4.3.23) rtl8188eus(4.3.24)  driver type
				res = sscanf(buf, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[index][0], &mac[index][1], &mac[index][2], &mac[index][3], &mac[index][4], &mac[index][5]);
				if(res == 6){
					//printf("connext mac %02x:%02x:%02x:%02x:%02x:%02x\n", mac[index][0], mac[index][1], mac[index][2], mac[index][3], mac[index][4], mac[index][5]);
					index++;
				}
			}else{
				//default ,may be externed by other type
				ptr = strstr(buf, "dot11RSNAStatsSTAAddress");  
				if(ptr){
					p = strtok(ptr,"=");
					p = strtok(NULL, "\n");
					if(p != NULL && strlen(p) > 1 && strcmp(p, "00:00:00:00:00:00") != 0){
						res = sscanf(p, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[index][0], &mac[index][1], &mac[index][2], &mac[index][3], &mac[index][4], &mac[index][5]);
						if(res == 6){
							index++;
						}
					}
				}
			}
			
			if(index >= num){
				break;
			}
		} 
		if (pclose(fp) == -1){ 
			perror("close popen file pointer fp error!\n"); 
			return -1;
		} 	
	}	
	return index;
}

static int iw_sockets_open(void)
{
	static const int families[] = {
		AF_INET,
		AF_IPX,
#ifdef PLATFORM_LINUX
		AF_AX25,
#endif
		AF_APPLETALK
		};
	unsigned int	i;
	int		sock;

	for(i = 0; i < sizeof(families)/sizeof(int); ++i)
	{
		sock = socket(families[i], SOCK_DGRAM, 0);
		if(sock >= 0)
		{
			return sock;
		}
	}
	return -1;
}

#define SIOCIWFIRSTPRIV 0x8BE0
#define RTW_IOCTL_MP_SIGNAL  (SIOCIWFIRSTPRIV + 28)
static int wlan_ioctl_mp_signal(int skfd, char *ifname, unsigned char mac_addr[6])             
{
	int err = -1;
	iwreq iwr;
	int i=0;
	struct ieee_param ieee,*param;
	memset(&ieee, 0, sizeof(ieee));
	ieee.cmd=24;
	for(i = 0;i < 6; i++)
	{
		ieee.sta_addr[i] = mac_addr[i];  
	}
	
	memset(&iwr, 0, sizeof(iwreq));
	strncpy(iwr.ifr_ifrn.ifrn_name, ifname, strlen(ifname));
	iwr.u.data.length = (unsigned short)sizeof(ieee);
	iwr.u.data.pointer=&ieee;	
	param=&ieee;
	struct ieee_param_ex *param_ex = (struct ieee_param_ex *)param;
	struct ja_ipcam_sta *psta_data = (struct ja_ipcam_sta *)((void *)param + sizeof(param_ex->cmd) + sizeof(param_ex->sta_addr));
	
	err = ioctl(skfd, RTW_IOCTL_MP_SIGNAL, &iwr);
	
	if(err == 0)
	{
		memcpy(&err, &psta_data->UndecoratedSmoothedPWDB, sizeof(int));
	}		
	return err;
}

int JN_Wifi_USB1_GetSignal(char* ifname, unsigned char mac_addr[6])
{
	int skfd;
	int err;
	
	skfd = iw_sockets_open();
	if(skfd < 0)
	{
		err = -1;
	}else
	{
		err = wlan_ioctl_mp_signal(skfd, ifname ? ifname : "wlan0", mac_addr);
		close(skfd);
	}
	return err;
}

/*
int main(void)
{
	struct wifi_struct swifi[100];
	struct sta_struct ssta;
	struct ap_struct sap;
	
	JN_Wifi_Init(swifi);
	
	JN_Wifi_SetMode("STA");
	
	char tmode[20];
	JN_Wifi_GetMode(tmode);
	printf("/////////////mode=%s\n",tmode);
	
	printf("111*******************************\n");
	printf("*******************************\n");
	printf("*******************************\n");
	
	if(!strcmp(tmode, "AP"))
	{
		JN_Wifi_AP_Setparam(&sap, "goodlucky","goodlucky","g","WPA-PSK","11", "192.168.2.200", "20", "8.8.8.8", "255.255.255.0", "192.168.2.1");
	}
	if(!strcmp(tmode, "STA"))
	{
		JN_Wifi_STA_Setparam(&ssta, "JUANR&D__", "www.dvr163.com");
	}
	
	printf("222*******************************\n");
	printf("*******************************\n");
	printf("*******************************\n");
	
	JN_Wifi_Start(tmode, &sap, &ssta);
	
	printf("333*******************************\n");
	printf("*******************************\n");
	printf("*******************************\n");
	
	JN_Wifi_STA_Search(swifi);
	JN_Wifi_STA_Getstatus();

	JN_Wifi_Exit();
	
	return 0;
}
*/
