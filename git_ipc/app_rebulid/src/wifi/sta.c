/*
* STA
*/

#include "sta.h"
#include "base/ja_process.h"
#include "global_runtime.h"

//open udhcpc
int STA_openUdhcpc(char *interfaceName, char *staticIp, char *script)
{
	char str[256];
	if(!interfaceName || !staticIp || !staticIp){
		return -1;
	}

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "udhcpc -q -i %s -A 0 -r %s -s %s&", interfaceName, staticIp, script);
	NK_SYSTEM(str);

	return 0;
}

//get frequency and signalLevel from "wpa_cli -iwlan0 -p/tmp/wpa_supplicant scan ; wpa_cli -iwlan0 -p/tmp/wpa_supplicant scan_results"
int GetSignal(char bssid[48], char interfaceName[100], char frequency[100], char signalLevel[100])
{
	char cmd[150];
	FILE * fp; 
	int res;
	char *p, *ptr;
	int index;
	char buf[1024];
	
	memset(cmd, '\0', sizeof(cmd));
	memset(buf, '\0', sizeof(buf));
	
	sprintf(cmd, "wpa_cli -i%s -p/tmp/wpa_supplicant scan ; wpa_cli -i%s -p/tmp/wpa_supplicant scan_results", interfaceName, interfaceName);

	if (!strcmp(cmd, "")) 
	{ 
		printf("cmd is NULL!\n");
		return -1;
	} 
	if ((fp = popen(cmd, "r") ) == NULL) 
	{ 
		perror("popen");
		printf("popen error: %s/n", strerror(errno)); 
		return -1; 
	} 
	else
	{
		while(fgets(buf, sizeof(buf), fp)) 
		{ 
			//printf("///%s", buf);
			
			//get flag
			ptr = strstr(buf, bssid);
			if(ptr)
			{ 
				p = strtok(buf,"\t");
				p = strtok(NULL, "\t");
				strcpy(frequency, p);
			    p = strtok(NULL, "\t");
				strcpy(signalLevel, p);
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
}


//check STA
int CheckSTA(char interfaceName[100])
{
	char cmd[150];
	FILE * fp; 
	int res;
	char *p, *ptr;
	int index;
	int flag;
	int ret;
	char buf[1024];
	char bssid[150];
	char frequency[100];
	char signalLevel[100];

	memset(cmd, '\0', sizeof(cmd));
	memset(buf, '\0', sizeof(buf));
	flag = 0;
	
	sprintf(cmd, "wpa_cli -i%s -p/tmp/wpa_supplicant status", interfaceName);

	if (!strcmp(cmd, "")) 
	{ 
		printf("cmd is NULL!\n");
		return -1;
	} 
	if ((fp = popen(cmd, "r") ) == NULL) 
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
			ptr = strstr(buf, "bssid"); 
			if(ptr)
			{ 
				p = strtok(buf,"=");
				p = strtok(NULL, "\n");
				strcpy(bssid, p);
			}
			
			//check state
			ptr = strstr(buf, "wpa_state");
			if(ptr)
			{ 
				p = strtok(buf,"=");
				p = strtok(NULL, "\n");
				
			    if(!strcmp(p, "DISCONNECTED")){
					printf("////////////[wifi] disconnect!\n");
			    } 
			    if(!strcmp(p, "INTERFACE_DISABLED")){
					printf("////////////[wifi] Interface disabled!\n");
			    } 
			    if(!strcmp(p, "INACTIVE")){
					printf("////////////[wifi] Inactive state (wpa_supplicant disabled)!\n");
			    }   
			    if(!strcmp(p, "SCANNING")){
					printf("////////////[wifi] Now is scanning for a network!\n");
			    } 
			    if(!strcmp(p, "AUTHENTICATING")){
					printf("////////////[wifi] Trying to authenticate with a BSS/SSID!\n");
			    } 
			    if(!strcmp(p, "ASSOCIATING")){
					printf("////////////[wifi] Trying to associate with a BSS/SSID!\n");
			    } 
			    if(!strcmp(p, "ASSOCIATED")){
					printf("////////////[wifi] Association completed!\n");
			    } 
			    if(!strcmp(p, "4WAY_HANDSHAKE")){
					printf("////////////[wifi] WPA 4-Way Key Handshake in progress!\n");
			    } 
			    if(!strcmp(p, "GROUP_HANDSHAKE")){
					printf("////////////[wifi] WPA Group Key Handshake in progress!\n");
			    } 
			    if(!strcmp(p, "COMPLETED")){
					printf("////////////[wifi] All authentication completed!  Wifi connect success!\n");
				    flag = 1;
			    } 
			}
					
			if (flag == 1)
			{					
				GetSignal(bssid, interfaceName, frequency, signalLevel);   //get frequency and signalLevel from "wpa_cli -iwlan0 scan ; wpa_cli -iwlan0 scan_results"
			
				printf("////////////[wifi] frequency=%s,signal level=%s.\n",frequency,signalLevel);	
				
				break;		
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
}

//message check (wpa_supplicant)   
//wpa_supplicant -D = driver name (can be multiple drivers: nl80211,wext) -i = interface name -c = Configuration file  -B = run daemon in the background
void MegCheck(char driverName[200], char interfaceName[100],char CtrlInterface[200],char  ConfigueFile[200])
{
	char str[750];
	FILE *fb;
	
	memset(str, '\0', sizeof(str));
	
	sprintf(str, "%s/wifi_tools/wpa_supplicant -D%s -i%s -C%s -c%s -B &", IPCAM_ENV_HOME_DIR, driverName, interfaceName, CtrlInterface, ConfigueFile);
	NK_SYSTEM(str);
	/*fb= popen(str, "r");
    if(fb==NULL)
	{ 
		printf("popen error: %s/n", strerror(errno)); 
		
	}else{
		//pclose(fb);
	} */
	/*ret = system(str);
	if(ret == -1)
		perror("Meg Check error!");*/
}

//wpa_passphrase STA
void STA_passphraseSTA(char essid[100], char passwd[100], char ConfigueFile[200])
{
	char str[200];
	FILE *fp;
	
	memset(str, 0, sizeof(str));
	
	sprintf(str, "%s/wifi_tools/wpa_passphrase '%s' '%s' > %s", IPCAM_ENV_HOME_DIR, essid, passwd, ConfigueFile);
	NK_SYSTEM(str);
    /*fp= popen(str, "r");
    if(fp==NULL)
	{ 
		printf("popen error: %s/n", strerror(errno)); 
		
	}else{
		pclose(fp);
	}*/
}

#include "utf8_gbk_mem.h"

//wpa setup STA configure file
void STA_setupSTA(char essid[100], char passwd[100], char ConfigueFile[200])
{
	FILE *fp;
	char str[2048];
	char tmpgbk[100] = {0};

	if(strlen(essid) > 0) {
		memset(tmpgbk, 0, sizeof(tmpgbk));
		utf82gbk_(essid, tmpgbk);
	}

	fp = fopen(ConfigueFile, "w+");
	if(fp==NULL)
	{ 
		printf("fopen %s error: %s/n", ConfigueFile, strerror(errno)); 	
		return;
	} 
	
	//output data
	if(strlen(passwd) == 0){
		snprintf(str, sizeof(str), "ctrl_interface=/tmp/wpa_supplicant\n"
						"update_config=1\n"
						"network={\n"
						"	ssid=\"%s\"\n"
						"	key_mgmt=NONE\n"
						"}\n", 
						essid);
		if((strlen(tmpgbk) > 0) && (0 != strcmp(tmpgbk, essid))) { //ignore when string not changed
			snprintf(str, sizeof(str)-1, "%s network={\n"
						"	ssid=\"%s\"\n"
						"	key_mgmt=NONE\n"
						"}\n", str, tmpgbk);
		}
	}else{
		snprintf(str, sizeof(str)-1, "ctrl_interface=/tmp/wpa_supplicant\n"
					"update_config=1\n"
					"network={\n"
					"  ssid=\"%s\"\n"
					"  psk=\"%s\"\n"
					"  scan_ssid=1\n"
					"}\n", essid, passwd);

		if((strlen(tmpgbk) > 0) && (0 != strcmp(tmpgbk, essid))) { //ignore when string not changed
			snprintf(str, sizeof(str)-1, "%s network={\n"
					"  ssid=\"%s\"\n"
					"  psk=\"%s\"\n"
					"  scan_ssid=1\n"
					"}\n", str, tmpgbk, passwd);
		}
	}
	fputs(str, fp); 

	fclose(fp);
}


//regular expression match pattern
int PatternMatch(char *bematch, char *pattern, char match[100])
{
	char errbuf[1024];                                                                    
	regex_t reg;                                       
	int err,nm = 10;                                   
	regmatch_t pmatch[nm];                             
		                                               
	if(regcomp(&reg,pattern,REG_EXTENDED) < 0)
	{        
		regerror(err,&reg,errbuf,sizeof(errbuf));          
		printf("err:%s\n",errbuf); 
		return -1;                        
	}                                                  
		                                               
	err = regexec(&reg,bematch,nm,pmatch,0);           
		                                               
	if(err == REG_NOMATCH){                            
		printf("no match\n");                              
		return -1;                                            
	}else if(err){                                     
		regerror(err,&reg,errbuf,sizeof(errbuf));          
		printf("err:%s\n",errbuf);                         
		return -1;                                          
	}                                                  
	
	int i;                                               
	for(i=0;i<nm && pmatch[i].rm_so!=-1;i++)
	{      
		int len = pmatch[i].rm_eo-pmatch[i].rm_so;         
		if(len)
		{                                           
			memset(match,'\0',sizeof(match));                  
			memcpy(match,bematch+pmatch[i].rm_so,len);                                   
		}                                                  
	} 
	return 1;        
}

//segregate the message
void SegregateMsg(char * msg, struct wifi_struct tswifi[100], int index)
{
	char *p, *p2, *ptr;
	char *pattern;
	char match[100];
	
	memset(match, '\0', sizeof(match));
	
	//SSID
	ptr = strstr(msg, "ESSID");
	if(ptr)
	{
		p = strtok(msg, "\"");
		p = strtok(NULL, "\"");
		if(p){		
			strcpy(tswifi[index].essid, p);
			//printf("22***%s,%s\n",p,tswifi[index].essid);
		}
	}
	
	//wap1
	ptr = strstr(msg, "WPA Version");
	if(ptr)
	{ 
		p = strtok(msg," ");
		p = strtok(NULL, "\n");
		if(p){				
			strcpy(tswifi[index].wpa1, p);
			//printf("22***%s,%s\n",p,tswifi[index].wpa1);
		}  
	}
 
	//wpa2
	ptr = strstr(msg, "WPA2 Version");
	if(ptr)
	{ 
		p = strtok(msg," ");
		p = strtok(NULL, "\n");
		if(p){				
			strcpy(tswifi[index].wpa2, p);
			//printf("22***%s,%s\n",p,tswifi[index].wpa2);
		}  
	}
	
	//key:PSK
	ptr = strstr(msg, "Authentication Suites");
	if(ptr)
	{   /*
		p = strtok(msg, ":");
		//p = strtok(NULL, " ");
		p = strtok(NULL, "\n");
		if(p){				
			strcpy(tswifi[index].key, p);
			printf("22***%s,%s\n",p,tswifi[index].key);
		}*/
		pattern = "^Authentication.{14}(.*)\n$";		
		PatternMatch(ptr, pattern, match); 
		strcpy(tswifi[index].key, match);
		//printf("22***%s,%s\n",match,tswifi[index].key);  
	}
	
	//quality  ||   signal level
	ptr = strstr(msg, "Quality");
	if(ptr)
	{
		p = strtok(msg, " ");
		p2 = strtok(NULL, " ");
		p2 = strtok(NULL, " ");
		
		//quality
		p = strtok(p, "=");
		p = strtok(NULL, "=");
		if(p){		
			strcpy(tswifi[index].quality, p);
			//printf("22***%s,%s\n",p,tswifi[index].quality);
		}

		//signal level
		p = strtok(p2, "=");
		p = strtok(NULL, "=");
		if(p){		
			strcpy(tswifi[index].signalLevel, p);
			//printf("22***%s,%s\n",p,tswifi[index].signalLevel);
		}
	}
}

//scanning
int ScanApp(char * cmd, struct wifi_struct tswifi[100]) 
{ 
	FILE * fp; 
	int res;
	char *p, *ptr, *a;
	int index;
	char buf[1024]; 
	
	if (cmd == NULL) 
	{ 
		printf("cmd is NULL!\n");
		return -1;
	} 
	if ((fp = popen(cmd, "r") ) == NULL) 
	{ 
		perror("popen");
		printf("popen error: %s/n", strerror(errno)); 
		return -1; 
	} 
	else
	{
		while(fgets(buf, sizeof(buf), fp)) 
		{ 
			//printf("%s", buf);
			
			//cell no
			ptr = strstr(buf, "Cell");
			if(ptr)
			{
				p = strtok(buf, " ");
				p = strtok(NULL, " ");
				if(p){
					index = atoi(p);
					strcpy(tswifi[index].cellNo, p);			
					//printf("22***%s,%s\n",p,tswifi[index].cellNo);
				}
				a= strtok(NULL, "Address:");
				//printf("################## %s\n",a);
				a= strtok(NULL, " ");
				//printf("################## %s\n",a);           
				a = strtok(NULL, "\n");
				if(a){		
					 strcpy(tswifi[index].bssid,a);
					//printf("22***%s,%s\n",a,tswifi[index].bssid);
				}
			}

			SegregateMsg(buf, tswifi, index);  //segregate the message
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
} 

/*
int main(void)
{
	int i;
	struct wifi_struct 	swifi[100];
	for(i=0; i<100; i++)
	{
		memset(&swifi[i], 0, sizeof(struct wifi_struct));
	}
	
	//install driver,wlan0,DNS
	system("/mnt/wifi/code/wifi/data.sh");   //absolute path
	
	//scanning
	ScanApp("iwlist wlan0 scanning | grep -E 'Cell|ESSID|PSK|Version|Quality'", swifi);
	
	printf("111*******************************\n");
	printf("*******************************\n");
	printf("*******************************\n");

	i = 1;
	while(strcmp(swifi[i].cellNo,"")!=0)
	{
		printf("%s,%s,%s,%s,%s,%s,%s,%s\n",swifi[i].cellNo,swifi[i].essid,swifi[i].key,swifi[i].quality,swifi[i].signalLevel,swifi[i].wpa1,swifi[i].wpa2,swifi[i].bssid);
		i++;
	}
	
	printf("222*******************************\n");
	printf("*******************************\n");
	printf("*******************************\n");

	sleep(1);
	
	SetupSTA("JUANR&D__", "www.dvr163.com", "/nfsroot/wpa_supplicant.conf");   //wpa_passphrase STA
	
	printf("333*******************************\n");
	printf("*******************************\n");
	printf("*******************************\n");
	
	sleep(1);
	
	MegCheck("wext", "wlan0", "/var/run/wpa_supplicant", "/nfsroot/wpa_supplicant.conf");   //message check (wpa_supplicant)
	
	printf("444*******************************\n");
	printf("*******************************\n");
	printf("*******************************\n");
	
	sleep(4);
	
	CheckSTA("wlan0");   //check STA
	
	printf("end*******************************\n");
	printf("*******************************\n");
	printf("*******************************\n");
	
	return 0;
}

*/
