
#ifndef __STA_H__
#define __STA_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <regex.h>
#include "global_runtime.h"

struct wifi_struct
{
	char cellNo[12];
    char bssid[48];
	char essid[64];
	char key[100];
	char quality[100];
	char signalLevel[100];
	char wpa1[200];
    char wpa2[300];
};

#define NK_USER_CONFIG_UDHCPC_SCRIPT IPCAM_ENV_HOME_DIR"/shell/landhcp.script"
int STA_openUdhcpc(char *interfaceName, char *staticIp, char *script);	//open udhcpc
int GetSignal(char bssid[48], char interfaceName[100], char frequency[100], char signalLevel[100]);
int CheckSTA(char interfaceName[100]);    //check STA
void MegCheck(char driverName[200], char interfaceName[100],char CtrlInterface[200],char  ConfigueFile[200]);   //message check (wpa_supplicant) 
int PatternMatch(char *bematch, char *pattern, char match[100]);  //regular expression match pattern
void SegregateMsg(char * msg, struct wifi_struct tswifi[100], int index);  //segregate the message
int ScanApp(char * cmd, struct wifi_struct tswifi[100]);   //scanning
void STA_passphraseSTA(char essid[100], char passwd[100], char ConfigueFile[200]);	//wpa_passphrase STA
void STA_setupSTA(char essid[100], char passwd[100], char ConfigueFile[200]);	//wpa setup STA configure file


#ifdef __cplusplus
};
#endif
#endif //__STA_H__
