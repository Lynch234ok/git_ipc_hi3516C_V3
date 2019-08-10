
#ifndef __AP_H__
#define __AP_H__
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


struct ap_struct
{
	char essid[64];
	char wpa_passphrase[100];
	char hw_mode[8];
	char wpa_key_mgmt[64];
	char channel[8];
    char ipStart[20];
    char ipNumber[12];
    char dns[20];
    char subnet[20];
    char router[20];
};

void AP_openUdhcpd(char *fileName);   //open udhcpd
void AP_setUdhcpd(struct ap_struct *tsap, char *interface, char *fileName);  //set udhcpd configure
void AP_openAP(char *fileName);   //open AP (hostapd)   
void AP_setHostapd(struct ap_struct *tsap, char *fileName);  //set the AP configuration

#ifdef __cplusplus
};
#endif
#endif //__AP_H__