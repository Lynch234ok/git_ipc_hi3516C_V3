#include "p2p_json_parse.h"
#include "netsdk.h"
#include "netsdk_util.h"
#include "sdk/sdk_api.h"

#include "app_debug.h"
#include "netsdk_private.h"
#include "app_msg_push.h"
#include "usrm.h"
#include "_base64.h"
#include "urlXxcode.h"
#include <sys/statvfs.h>
#include <dirent.h>
#include "generic.h"
#include <stdarg.h>
#include <base/ja_process.h>
#include <bsp/rtc.h>
#include "schedule_parse.h"
#include "tfcard.h"
#include "fisheye.h"
#include "custom.h"
#include "led_pwm.h"
#include "../netsdk.h"
#include "../netsdk_private.h"
#include "global_runtime.h"
#include "http_auth/_md5.h"
#include <secure_chip.h>
#include <NkEmbedded/mem_allocator.h>
#include "include/aes.h"
#include "app_wifi.h"
#include "sound.h"
#include "wpa_supplicant/include/wpa_status.h"
#include "p2pdevice.h"
#include "sensor.h"

#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
#endif
#include "app_cloud_rec.h"
#include "ntp.h"

extern lpSDK_VIN_API sdk_vin;

typedef enum {
    TOTAL_SIZE,///�ļ�ϵͳ�Ĵ�С
    FREE_SIZE, ///���ɿռ�
    USED_SIZE, ///���ÿռ�
    AVAIL_SIZE ///�û�ʵ�ʿ���ʹ�õĿռ�
}VFsize;

bool p2p_setup_is_get(void *request)
{
	
	bool ret = true;
	char text[32];
	LP_JSON_OBJECT obj; 
	obj = NETSDK_json_parse(request);

	memset(text, 0, sizeof(text));
	if(NULL != obj){
		NETSDK_json_get_string(obj, "Method", text, sizeof(text));
		if(!strcmp(text, "get")){
			//set
			ret = true;
		}else{
			//get
			ret = false;
		}

		json_object_put(obj);
	}
	return ret;
}

static NK_SSize aesDecode(char* crypto, int len, NK_PByte aesKey, int aesKeyLen, char* origin)
{
	NK_Byte mem[1024] = "\0";

	NK_SSize deSize = 0;
	NK_AES *AES = NK_Nil;

	if((crypto == NK_Nil) || (aesKey == NK_Nil) || (origin == NK_Nil)) {
		APP_TRACE("aesDecode fail");
		return -1;
	}

	NK_Allocator *Alloctr = NK_Alloc_Create(mem, sizeof(mem));

	if(NK_Nil != Alloctr) {
		AES = NK_AES_Create(Alloctr, aesKey, aesKeyLen);
		if(NK_Nil != AES) {
			deSize = AES->decrypt(AES, (NK_PByte)crypto, len, (NK_PByte)origin);

			NK_AES_Free(&AES);
		}
		NK_Alloc_Free(&Alloctr, NK_Nil);
	}

	return deSize;

}

static int p2p_parse_userpwd_varify(char *verify_str, char *user_name, char *passwd)
{
	if(NULL == verify_str || (0 == strlen(verify_str))){
		return -1;
	}
	//base64 decode
	size_t verify_str_len = strlen(verify_str);
	char *base64_decbuf = (char*)calloc(2*verify_str_len, 1);	
	BASE64_decode(verify_str, strlen(verify_str), base64_decbuf, 2*verify_str_len);

	if(base64_decbuf)
	{
		char *tmp = NULL;
		tmp = strstr(base64_decbuf, "&&");
		if(tmp){
			sprintf(passwd, tmp+2);
			memset(tmp, 0, 1);
		}
		if(user_name){
			tmp = strstr(base64_decbuf, "&");
			if(tmp)
			{
				sprintf(user_name, tmp+1);
			}
		}
	}
	if(base64_decbuf)
	{
		free(base64_decbuf);
		base64_decbuf = NULL;
	}
	return 0;
}

static int p2p_parse_userpwd_varify2(char *verify_str, char *user_name, char *passwd)
{
	NK_Byte video_share_key[] = {0xFF, 0x9A, 0x12, 0x34, 0xC2, 0xAA, 0x55, 0x3D,0xB4, 0x5C,
	0x83, 0xD2, 0xA9, 0xFF, 0x07, 0x4F};

	char aes_outbuf[256] = "\0";
	NK_SSize size = 0;

	if(NULL == verify_str || (0 == strlen(verify_str))){
		return -1;
	}

	//base64 decode
	size_t verify_str_len = strlen(verify_str);
	char *base64_decbuf = (char*)calloc(2*verify_str_len, 1);
	int decode_len = BASE64_decode(verify_str, strlen(verify_str), base64_decbuf, 2*verify_str_len);
	size = aesDecode(base64_decbuf, decode_len, video_share_key, sizeof(video_share_key), aes_outbuf);
	printf("Decode Res::%s\n", aes_outbuf);

	if(size > 0)
	{
		char *save_str = NULL;
		char *tmp = NULL;

		tmp = strtok_r(aes_outbuf, "&", &save_str);
		tmp = strtok_r(NULL, "&", &save_str);
		sprintf(user_name, "%s", (tmp != NULL) ? tmp : "");
		tmp = strtok_r(NULL, "&", &save_str);
		sprintf(passwd, "%s", (tmp != NULL) ? tmp : "");

		APP_TRACE("user_name:%s passwd:%s\n", user_name , passwd);
	}
	else {
		APP_TRACE("Verify decode failed(%s)", verify_str);
	}

	if(base64_decbuf)
	{
		free(base64_decbuf);
		base64_decbuf = NULL;
	}
	
	return 0;
}

bool p2p_Auth_Verify_pass(void *request)
{
	char varify_text[255]={0};
	char username[25]={0};
	char password[25]={0};
	LP_JSON_OBJECT obj, Auth_user=NULL; 
	obj = NETSDK_json_parse(request);

	if(NULL != obj){		
		Auth_user = NETSDK_json_get_child(obj, "Authorization");
		if(Auth_user){
			NETSDK_json_get_string(Auth_user, "Verify", varify_text, sizeof(varify_text));

			if(strlen(varify_text) == 0){
				//verify
				NETSDK_json_get_string(Auth_user, "username", username, sizeof(username));
				NETSDK_json_get_string(Auth_user, "password", password, sizeof(password));			
			}else{
				//verify
				p2p_parse_userpwd_varify2(varify_text, username, password);
			}
		}
	}

	json_object_put(obj);
	if(username || (0 != strlen(username))){		
		if(USRM_GREAT == USRM_check_user(username, password)){
			return true;
		}
	}
	return false;
}

bool p2p_Auth_userpwd_set(char *username, char *oldpwd, char *newpasswd)
{
	USRM_I_KNOW_U_t* i_m = NULL;
	bool set_success = false;
	
	if(NULL == username || (0 == strlen(username))){
		return false;
	}
	//set
	// user check in
	i_m = USRM_login(username, oldpwd);
	if(i_m){
		USRM_HOW_ABOUT_t how_about = USRM_GREAT;		
		how_about = i_m->set_password(i_m, oldpwd, newpasswd);
		if(USRM_GREAT == how_about){
			APP_TRACE("Set user \"%s\" password success!", strdupa(username));
			USRM_store();
			set_success = true;
		}
		// check out
		USRM_logout(i_m);
		i_m = NULL;
		return set_success;
	}
	return false;
}


int mtion_setsensitivityLevel(ST_NSDK_MD_CH *md_ch, char *sensitivityLevel)
{
#ifdef P6
    if(0 == strcmp(sensitivityLevel, "lowest"))
    {
        md_ch->detectionGrid.sensitivityLevel = 60;
    }
    else if(0 == strcmp(sensitivityLevel, "low"))
    {
        md_ch->detectionGrid.sensitivityLevel = 70;
    }
    else if(0 == strcmp(sensitivityLevel, "normal"))
    {
        md_ch->detectionGrid.sensitivityLevel = 80;
    }
    else if(0 == strcmp(sensitivityLevel, "high"))
    {
        md_ch->detectionGrid.sensitivityLevel = 90;
    }
    else if(0 == strcmp(sensitivityLevel, "highest"))
    {
        md_ch->detectionGrid.sensitivityLevel = 95;
    }
#else
	if(0==strcmp(sensitivityLevel, "lowest"))
	{
		md_ch->detectionGrid.sensitivityLevel = 60;
	}
		else if(0==strcmp(sensitivityLevel, "low"))
	{
		md_ch->detectionGrid.sensitivityLevel = 80;
	}
	else if(0==strcmp(sensitivityLevel, "normal"))
	{
		md_ch->detectionGrid.sensitivityLevel = 90;
	}
	else if(0==strcmp(sensitivityLevel, "high"))
	{
		md_ch->detectionGrid.sensitivityLevel = 95;
	}
	else if(0==strcmp(sensitivityLevel, "highest"))
	{
		md_ch->detectionGrid.sensitivityLevel = 98;
	}
#endif
	 NETSDK_conf_md_ch_set(1, md_ch);
}

int dateinfo_filtrate(time_t begin_time, time_t end_time, char *filepath)
{
	struct tm *begintime, *endtime;
	int dir_count = 0;
	int begin_year = 0, begin_month = 0, begin_day = 0, end_year = 0, end_month = 0, end_day = 0;

	char ts_date_info[256] = {0};
	char datetmp[32] = {0}, month_char[5] = {0};
	char *ptr = NULL, *month_ptr = NULL;
	char dateinfo_final[256] = {0};
	unsigned long int year_num = 0, month_num = 0, day_num = 0;

	dir_count = NK_TFCARD_GetTsRecPath_Of_Hex(ts_date_info);

	//printf("\n### all file = %s\n", ts_date_info);

	if(-1 == dir_count) //scan dir error
	{
		snprintf(dateinfo_final, sizeof(dateinfo_final), "%s", "[]");
		strcpy(filepath, dateinfo_final);
		printf("file[%d] = %s\n", dir_count, filepath);

		return -1;
	}

	if(0 == dir_count) //nothing in the file of path
	{
        printf("nothing !\n");
		snprintf(dateinfo_final, sizeof(dateinfo_final), "%s", "[]");
		strcpy(filepath, dateinfo_final);
		printf("file[%d] = %s\n", dir_count, filepath);

        return -1;
	}

	begintime	= localtime(&begin_time);
	begin_year	= begintime->tm_year + 1900;
	begin_month = begintime->tm_mon + 1;
	begin_day	= begintime->tm_mday;

	endtime   = localtime(&end_time);
	end_year  = endtime->tm_year + 1900,
	end_month = endtime->tm_mon + 1;
	end_day   = endtime->tm_mday;

	printf("want to find: %d-%d-%d to %d-%d-%d\n\n", begin_year, begin_month , begin_day, end_year, end_month ,end_day);

	snprintf(dateinfo_final, sizeof(dateinfo_final), "%s", "[ ");

	for(ptr = strtok(ts_date_info, "|"); ptr != NULL; ptr = strtok(NULL, "|"))
	{
		// function
		// printf("ptr = %s\n", ptr);
		strcpy(datetmp, ptr);

		//printf("datetmp = %s\n\n", datetmp);
		snprintf(month_char, 3, "%s", datetmp + 5);
		month_ptr = month_char;
		while(*(month_ptr++) == '0');

		year_num  = strtol(datetmp,       NULL, 0);
		month_num = strtol(month_ptr - 1, NULL, 0);
		day_num   = strtol(datetmp + 8,   NULL, 16);

		printf("date = %ld-%ld, d = %lX\n", year_num, month_num, day_num);
		if(begin_year == end_year)
		{
			if((year_num == begin_year) && (month_num <= end_month) && (month_num >= begin_month))
			{
				snprintf(dateinfo_final + strlen(dateinfo_final), sizeof(dateinfo_final), "\"%s\", ", datetmp);
			}
		}
		if((begin_year + 1) == end_year)
		{
			if(year_num == begin_year)
			{
				if((begin_month <= month_num) && (month_num <= 12))
				{
					snprintf(dateinfo_final + strlen(dateinfo_final), sizeof(dateinfo_final), "\"%s\", ", datetmp);
				}
			}
			if(year_num == end_year)
			{
				if((1 <= month_num) && (month_num <= end_month))
				{
					snprintf(dateinfo_final + strlen(dateinfo_final), sizeof(dateinfo_final), "\"%s\", ", datetmp);
				}
			}
		}

	}

	if(2 < strlen(dateinfo_final))
	{
		snprintf(dateinfo_final + strlen(dateinfo_final) - 2, sizeof(dateinfo_final), "%s", " ]");
	}
	else
	{
		snprintf(dateinfo_final, sizeof(dateinfo_final), "%s", "[]");
	}

	strcpy(filepath, dateinfo_final);

	printf("file = %s\n", filepath);

	return 0;
}


int set_motinfo(LP_JSON_OBJECT motinfo, int sensitivityLevel)
{
	 int ret = -1;
#ifdef P6
    if(sensitivityLevel>=0 && sensitivityLevel <= 60)
    {
        ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "lowest");
    }
    else if(sensitivityLevel > 60 && sensitivityLevel <= 70)
    {
        ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "low");
    }
    else if(sensitivityLevel > 70 && sensitivityLevel <= 80)
    {
        ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "normal");
    }
    else if(sensitivityLevel > 80 && sensitivityLevel <= 90)
    {
        ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "high");
    }
    else if(sensitivityLevel > 90 && sensitivityLevel <= 100)
    {
        ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "highest");
    }
#else
	 if(sensitivityLevel>=0 && sensitivityLevel <= 60)
	 {
	 	ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "lowest");
	 }
	 else if(sensitivityLevel > 60 && sensitivityLevel <= 80)
	 {
		 ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "low");
	 }
	 else if(sensitivityLevel > 80 && sensitivityLevel <= 90)
	 {
		 ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "normal");					 	
	 }
	 else if(sensitivityLevel > 90 && sensitivityLevel <= 95)
	 {
		 ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "high");
	 }
	 else if(sensitivityLevel > 95 && sensitivityLevel <= 100)
	 {
		 ret = NETSDK_json_set_string2(motinfo, "SensitivityLevel", "highest");
	 }
#endif
	 return ret;
}

static LP_JSON_OBJECT p2p_video_find_channel(LP_JSON_OBJECT channels, int id)
{
	int i = 0;
	int const n_channels = json_object_array_length(channels);
	// one channel
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		if(json_object_get_int(json_object_object_get(channel, "id")) == id){
			return channel;
		}
	}
	return NULL;
}

static NK_Int int_to_string(NK_PChar buf, NK_Int num, NK_Size bufLen)
{
    snprintf(buf,bufLen,"%d",num);
}

#ifdef DANA_P2P
static void *p2p_json_reboot(void *arg)
{
    usleep(2000000);
    APP_TRACE("reboot");
    GLOBAL_reboot_system();

}
#endif

static void ipcam_timer_record()
{
	NK_TFRCORD_SetParam();
	TICKER_del_task(ipcam_timer_record);
}

static int p2pParseCapabilitysetGet(LP_JSON_OBJECT obj)
{
    ST_NSDK_SYSTEM_SETTING systemInfo;

    NETSDK_conf_system_get_setting_info(&systemInfo);

    if(NULL != obj)
    {
        NETSDK_json_set_int2(obj, "version", systemInfo.capabilitySet.version);
        NETSDK_json_set_int2(obj, "maxChannel", systemInfo.capabilitySet.maxChannel);
        NETSDK_json_set_string2(obj, "model", systemInfo.capabilitySet.model);
        NETSDK_json_set_boolean2(obj, "powerBattery", systemInfo.capabilitySet.powerBattery);
        NETSDK_json_set_boolean2(obj, "audioInput", systemInfo.capabilitySet.audioInput);
        NETSDK_json_set_boolean2(obj, "audioOutput", systemInfo.capabilitySet.audioOutput);
        NETSDK_json_set_boolean2(obj, "bluetooth", systemInfo.capabilitySet.bluetooth);
        NETSDK_json_set_int2(obj, "lightControl", systemInfo.capabilitySet.lightControl);
        NETSDK_json_set_int2(obj, "bulbControl", systemInfo.capabilitySet.bulbControl);
        NETSDK_json_set_boolean2(obj, "ptz", systemInfo.capabilitySet.ptz);
        NETSDK_json_set_boolean2(obj, "sdCard", systemInfo.capabilitySet.sdCard);
        NETSDK_json_set_boolean2(obj, "lte", systemInfo.capabilitySet.lte);
        NETSDK_json_set_boolean2(obj, "wifi", systemInfo.capabilitySet.wifi);
        NETSDK_json_set_boolean2(obj, "rj45", systemInfo.capabilitySet.rj45);
        NETSDK_json_set_boolean2(obj, "rtc", systemInfo.capabilitySet.rtc);
        NETSDK_json_set_int2(obj, "fisheye", systemInfo.capabilitySet.fisheye);
        NETSDK_json_set_boolean2(obj, "wifiStationCanSet", systemInfo.capabilitySet.wifiStationCanSet);

        return 0;

    }

    return -1;

}

static int p2p_parse_wireleseStationGet(LP_JSON_OBJECT getObj)
{
    LP_JSON_OBJECT APsArrayObj = NULL;
    LP_JSON_OBJECT APsObj = NULL;
    ST_NSDK_NETWORK_INTERFACE stInterface;
    ST_NSDK_SYSTEM_SETTING systemInfo;
    stNK_WIFI_HotSpot nearAp[256];
    unsigned int nAPs = sizeof(nearAp);
    int i = 0;
    char en_base64[256] = {0};

    if(NULL == getObj)
    {
        return -1;
    }

    /*
     * 判断能力集是否支持无线设置 true == wifiStationCanSet
     */
    NETSDK_conf_system_get_setting_info(&systemInfo);
    if(false == systemInfo.capabilitySet.wifiStationCanSet)
    {
        return -1;
    }

    if(NULL != NETSDK_conf_interface_get(4, &stInterface))
    {
        if(NSDK_NETWORK_WIRELESS_MODE_STATIONMODE == stInterface.wireless.wirelessMode)
        {
            base64_encode(stInterface.wireless.wirelessStaMode.wirelessApEssId, en_base64, strlen(stInterface.wireless.wirelessStaMode.wirelessApEssId));
            NETSDK_json_set_string2(getObj, "ssid", en_base64);
        }
        else if(NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT == stInterface.wireless.wirelessMode)
        {
            NETSDK_json_set_string2(getObj, "ssid", "");
        }

        NETSDK_json_set_boolean2(getObj, "dhcp", stInterface.wireless.dhcpServer.dhcpAutoSettingEnabled);
        if(false == stInterface.wireless.dhcpServer.dhcpAutoSettingEnabled)
        {
            NETSDK_json_set_string2(getObj, "ipAddr", stInterface.lan.staticIP);
            NETSDK_json_set_string2(getObj, "netmask", stInterface.lan.staticNetmask);
            NETSDK_json_set_string2(getObj, "gateway", stInterface.lan.staticGateway);
            NETSDK_json_set_string2(getObj, "dns", stInterface.dns.staticPreferredDns);
        }
    }

    APsArrayObj = NETSDK_json_get_child(getObj, "APs");
    if(NULL != APsArrayObj)
    {
        memset(nearAp, 0, sizeof(nearAp));
        APP_WIFI_get_near_ap(nearAp, &nAPs);

        for(i = 0; i < nAPs; i++)
        {
            APsObj = json_object_new_object();
            if(NULL != APsObj)
            {
                base64_encode(nearAp[i].essid, en_base64, strlen(nearAp[i].essid));
                NETSDK_json_set_string2(APsObj, "ssid", en_base64);
                NETSDK_json_set_int2(APsObj, "rssi", nearAp[i].dBm);
                if(!strncmp(nearAp[i].encrypt, "[ESS]", 5))
                {
                    NETSDK_json_set_boolean2(APsObj, "encrypt", false);
                }
                else
                {
                    NETSDK_json_set_boolean2(APsObj, "encrypt", true);
                }
                json_object_array_put_idx(APsArrayObj, i, APsObj);
            }
        }
    }

    return 0;

}

static int p2p_parse_wireleseStationSet(LP_JSON_OBJECT setObj)
{
    ST_NSDK_NETWORK_INTERFACE wlan;
    ST_NSDK_SYSTEM_SETTING systemInfo;
    char ssid[256] = {0};
    char psk[256] = {0};
    char decode_ssid[256] = {0};
    char decode_psk[256] = {0};
    char ip[64] = {0};
    char netmask[64] = {0};
    char gateway[64] = {0};
    char dns[64] = {0};
    bool dhcpTmp = false;
    bool setFlag = false;

    if(NULL == setObj)
    {
        return -1;
    }

    /*
     * 判断能力集是否支持无线设置 true == wifiStationCanSet
     */
    NETSDK_conf_system_get_setting_info(&systemInfo);
    if(false == systemInfo.capabilitySet.wifiStationCanSet)
    {
        return -1;
    }

    NETSDK_conf_interface_get(4, &wlan);

    if((NULL != NETSDK_json_get_string(setObj, "ssid", ssid, sizeof(ssid)))
        && (NULL != NETSDK_json_get_string(setObj, "psk", psk, sizeof(psk))))
    {
        base64_decode(ssid, decode_ssid, strlen(ssid));
        base64_decode(psk, decode_psk, strlen(psk));
        snprintf(wlan.wireless.wirelessStaMode.wirelessApEssId,
            sizeof(wlan.wireless.wirelessStaMode.wirelessApEssId), "%s",
            decode_ssid);
        snprintf(wlan.wireless.wirelessStaMode.wirelessApPsk,
            sizeof(wlan.wireless.wirelessStaMode.wirelessApPsk), "%s",
            decode_psk);
        wlan.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_STATIONMODE;
        wlan.wireless.dhcpServer.dhcpAutoSettingEnabled = true;
        wlan.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC;
        if(NSDK_NETWORK_WIRELESS_MODE_STATIONMODE == wlan.wireless.wirelessMode)
        {
            WPA_stop_connect();
            WPA_resetWifiConnectedFlag();
        }
        SearchFileAndPlay(SOUND_WiFi_setting, NK_False);
        SearchFileAndPlay(SOUND_Please_wait, NK_False);
        setFlag = true;
    }

    if(true == NETSDK_json_check_child(setObj, "dhcp"))
    {
        dhcpTmp = NETSDK_json_get_boolean(setObj, "dhcp");
        if(dhcpTmp != wlan.wireless.dhcpServer.dhcpAutoSettingEnabled)
        {
            wlan.wireless.dhcpServer.dhcpAutoSettingEnabled = dhcpTmp;
            setFlag = true;
        }
    }

    if(false == wlan.wireless.dhcpServer.dhcpAutoSettingEnabled)
    {
        if(NULL != NETSDK_json_get_string(setObj, "ipAddr", ip, sizeof(ip)))
        {
            snprintf(wlan.lan.staticIP, sizeof(wlan.lan.staticIP), "%s", ip);
            setFlag = true;
        }

        if(NULL != NETSDK_json_get_string(setObj, "netmask", netmask, sizeof(netmask)))
        {
            snprintf(wlan.lan.staticNetmask, sizeof(wlan.lan.staticNetmask), "%s", netmask);
            setFlag = true;
        }

        if(NULL != NETSDK_json_get_string(setObj, "gateway", gateway, sizeof(gateway)))
        {
            snprintf(wlan.lan.staticGateway, sizeof(wlan.lan.staticGateway), "%s", gateway);
            setFlag = true;
        }

        if(NULL != NETSDK_json_get_string(setObj, "dns", dns, sizeof(dns)))
        {
            snprintf(wlan.dns.staticPreferredDns, sizeof(wlan.dns.staticPreferredDns), "%s", dns);
            setFlag = true;
        }
    }

    if(true == setFlag)
    {
        NETSDK_conf_interface_set_by_delay(4, &wlan, eNSDK_CONF_SAVE_RESTART_WIRELESS, 2);
    }

    return 0;

}

static int p2p_parse_videoManagerGet(LP_JSON_OBJECT getObj)
{
    LP_JSON_OBJECT arrObj = NULL;
    ST_NSDK_VENC_CH vencConf;
	ST_NSDK_VIN_CH vin_ch;
    int nStreamId = 0;
    int nStreamCount = 2;


    if(NULL == getObj)
    {
        return -1;
    }

    nStreamCount = NETSDK_venc_get_channels();
    NETSDK_json_set_int2(getObj, "streamCount", nStreamCount);

    // 缺省返回主码流
    if(true == NETSDK_json_check_child(getObj, "streamId"))
    {
        nStreamId = NETSDK_json_get_int(getObj, "streamId");
        if(1 == nStreamId)
        {
            nStreamId = 101;
        }
        else if(2 == nStreamId)
        {
            nStreamId = 102;
        }
        else if((3 == nStreamId) && (3 == nStreamCount))
        {
            nStreamId = 103;
        }
        else
        {
            APP_TRACE("stream(%d) id not support!!!", nStreamId);
            return 0;
        }
    }
    else
    {
        NETSDK_json_set_int2(getObj, "streamId", 1);
        nStreamId = 101;
    }

    NETSDK_conf_venc_ch_get(nStreamId, &vencConf);

    // 当前只支持到H264和H265，后面有增加编码类型要同步更改
    if(kNSDK_CODEC_TYPE_H264 == vencConf.codecType)
    {
        NETSDK_json_set_string2(getObj, "encType", "H.264");
    }
    else if(kNSDK_CODEC_TYPE_H265 == vencConf.codecType)
    {
        NETSDK_json_set_string2(getObj, "encType", "H.265");
    }
    else
    {
        NETSDK_json_set_string2(getObj, "encType", "");
    }

    arrObj = NETSDK_json_parse(vencConf.codecTypeOpt);
    if(NULL != arrObj)
    {
        json_object_object_add(getObj, "encTypeProperty", arrObj);
    }

    if(SENSOR_MODEL_IMX307 != g_sensor_type)
    {
        NETSDK_conf_vin_ch_get(1, &vin_ch);
        NETSDK_json_set_boolean(getObj, "flipEnabled", vin_ch.flip);
        NETSDK_json_set_boolean(getObj, "mirrorEnabled", vin_ch.mirror);
    }

    return 0;

}

static int p2p_parse_videoManagerSet(LP_JSON_OBJECT setObj)
{
    ST_NSDK_VENC_CH vencConf;
	ST_NSDK_VIN_CH vin_ch;
	ST_CUSTOM_SETTING custom;
    int nStreamId = 0;
    int nStreamCount = 2;
    int codecTypeTmp = kNSDK_CODEC_TYPE_H264;
    char text[16] = {0};
	bool flipEnanbled = false;
	bool mirrorEnanbled = false;

    if(NULL == setObj)
    {
        return -1;
    }

    nStreamCount = NETSDK_venc_get_channels();
    if(NETSDK_json_check_child(setObj, "streamId"))
    {
        nStreamId = NETSDK_json_get_int(setObj, "streamId");
        if(1 == nStreamId)
        {
            nStreamId = 101;
        }
        else if(2 == nStreamId)
        {
            nStreamId = 102;
        }
        else if((3 == nStreamId) && (3 == nStreamCount))
        {
            nStreamId = 103;
        }
        else
        {
            APP_TRACE("stream(%d) id not support!!!", nStreamId);
            return 0;
        }

        NETSDK_conf_venc_ch_get(nStreamId, &vencConf);
        if(NULL != NETSDK_json_get_string(setObj, "encType", text, sizeof(text)))
        {
            // 当前只支持到H264和H265，后面有增加编码类型要同步更改
            codecTypeTmp = vencConf.codecType;
            if(!strncmp(text, "H.264", 5))
            {
                codecTypeTmp = kNSDK_CODEC_TYPE_H264;
            }
            else if(!strncmp(text, "H.265", 5))
            {
                codecTypeTmp = kNSDK_CODEC_TYPE_H265;
            }

            if(codecTypeTmp != vencConf.codecType)
            {
                vencConf.codecType = codecTypeTmp;
                NETSDK_conf_venc_ch_set2(nStreamId, &vencConf, true);
                if(NULL != netsdk->videoEncodeChannelChanged)
                {
                    netsdk->videoEncodeChannelChanged(nStreamId, &vencConf);
                }
            }
        }

        if(SENSOR_MODEL_IMX307 != g_sensor_type)
        {
            NETSDK_conf_vin_ch_get(1, &vin_ch);
            if(NULL != NETSDK_json_check_child(setObj, "flipEnabled"))
            {
                flipEnanbled = NETSDK_json_get_boolean(setObj, "flipEnabled");
                vin_ch.flip = flipEnanbled;
                if(true == flipEnanbled)
                {
                    SENSOR_mirror_flip(MODE_FLIP);
                }
                else
                {
                    SENSOR_mirror_flip(MODE_UNFLIP);
                }
            }
            if(NULL != NETSDK_json_check_child(setObj, "mirrorEnabled"))
            {
                mirrorEnanbled = NETSDK_json_get_boolean(setObj, "mirrorEnabled");
                vin_ch.mirror = mirrorEnanbled;
                if(true == mirrorEnanbled)
                {
                    SENSOR_mirror_flip(MODE_MIRROR);
                }
                else
                {
                    SENSOR_mirror_flip(MODE_UNMIRROR);
                }
            }
            NETSDK_conf_vin_ch_set(1, &vin_ch);

        }
    }

    return 0;

}

static int p2p_parse_ptzGet(LP_JSON_OBJECT getObj)
{
    ST_NSDK_PTZ_CFG ptzCfg;

    if(NULL == getObj)
    {
        return -1;
    }

    NETSDK_conf_ptz_ch_get(&ptzCfg);

    NETSDK_json_set_int2(getObj, "ptzCtrlSpeed", ptzCfg.stPtzExternalConfig.nSpeed);

    return 0;

}

static int p2p_parse_ptzSet(LP_JSON_OBJECT setObj)
{
    ST_NSDK_PTZ_CFG ptzCfg;
    int nSpeed = 0;

    if(NULL == setObj)
    {
        return -1;
    }

    nSpeed = NETSDK_json_get_int(setObj, "ptzCtrlSpeed");
    if(nSpeed > 8)
    {
        nSpeed = 8;
    }else if(nSpeed < 1)
    {
        nSpeed = 1;
    }

    NETSDK_conf_ptz_ch_get(&ptzCfg);
    if(nSpeed != ptzCfg.stPtzExternalConfig.nSpeed)
    {
        ptzCfg.stPtzExternalConfig.nSpeed = nSpeed;
        NETSDK_conf_ptz_ch_set(&ptzCfg);
    }

    return 0;
}

static int p2p_parse_devCoverGet(LP_JSON_OBJECT getObj)
{
	char region_Color[16] = {0};
	int i = 0, j = 0;
	int arrlen = 0;
	ST_NSDK_VIN_CH vin_ch;
	ST_SDK_VIN_COVER_ATTR cover;

	if(NULL == getObj)
	{
		return -1;
	}

	NETSDK_conf_vin_ch_get(1, &vin_ch);

	LP_JSON_OBJECT dev_child_json = json_object_array_get_idx(getObj, 0);//进入第一层中括号
	if(NULL == dev_child_json)
	{
		dev_child_json = json_object_new_object();
		NETSDK_json_set_int(dev_child_json, "channelNum", vin_ch.id);
		NETSDK_json_set_boolean(dev_child_json, "enabled", true);
		NETSDK_json_set_int(dev_child_json, "maxRegion", 4);

		j = 0;
		LP_JSON_OBJECT region_child_json = json_object_new_array();// 生成regions数组，只生成true的区域
		if(NULL != region_child_json)
		{
			for(i = 0; i < 4; i++)
			{
				if(true == vin_ch.privacyMask[i].enabled)
				{
					LP_JSON_OBJECT region_info = json_object_new_object();
					if(NULL != region_info)
					{
						NETSDK_json_set_float(region_info, "regionX", vin_ch.privacyMask[i].regionX);
						NETSDK_json_set_float(region_info, "regionY", vin_ch.privacyMask[i].regionY);
						NETSDK_json_set_float(region_info, "regionW", vin_ch.privacyMask[i].regionWidth);
						NETSDK_json_set_float(region_info, "regionH", vin_ch.privacyMask[i].regionHeight);

						snprintf(region_Color, sizeof(region_Color), "%x", vin_ch.privacyMask[i].regionColor);
						NETSDK_json_set_string(region_info, "regionColor", region_Color);

						json_object_array_put_idx(region_child_json, j, region_info);// 坐标信息添加到regions
						j++;
					 }
				 }
			 }
			 json_object_object_add(dev_child_json, "Regions", region_child_json);// regions添加到devCoverSetting下的数组，和上面的通道号添加同级
			 j = 0;
		}
		json_object_array_put_idx(getObj, 0, dev_child_json);
	}

	j = 0;
	for(i = 0; i < 4; i++)
	{
		cover.enable = vin_ch.privacyMask[i].enabled;
		cover.x = vin_ch.privacyMask[i].regionX/100;
		cover.y = vin_ch.privacyMask[i].regionY/100;
		cover.width = vin_ch.privacyMask[i].regionWidth/100;
		cover.height = vin_ch.privacyMask[i].regionHeight/100;
		cover.color = vin_ch.privacyMask[i].regionColor | 0xff000000;


		if((0 == cover.x) && (0 == cover.y) && (0 == cover.width) && (0 == cover.height))
		{
			printf("### region data error !!\n");
		}
		else
		{
            if(NULL != sdk_vin && NULL != sdk_vin->set_cover)
            {
                sdk_vin->set_cover(0, j, &cover);  // 设置通道0，id从0开始
            }
		}

		j++;
	}
	j = 0;
}


static int p2p_parse_devCoverSet(LP_JSON_OBJECT setObj)
{
	LP_JSON_OBJECT Regions = NULL;
	ST_NSDK_VIN_CH vin_ch;
	ST_SDK_VIN_COVER_ATTR cover;

	int i = 0, j = 0; // for array
	int chnNum = 0;
	int maxRegion = 0;
	int arrlen_out = 0, arrlen_in = 0;
	int region_setflag = 0;
	bool devCS_enable = false;
	float region_X = 0, region_Y = 0, region_W = 0, region_H = 0;
	char region_Color[16] = {0};
	uint32_t regionColor_num = 0;


	if(NULL == setObj)
	{
		return -1;
	}

	arrlen_out = json_object_array_length(setObj); // 获取最外层数组的长度，"devCoverSetting"下的数组长度

	for(i = 0; i < arrlen_out; i++)
	{
		LP_JSON_OBJECT devCoverParamData = json_object_array_get_idx(setObj, i);// 遍历最外层的数组，下面判断通道号
		if(NULL != devCoverParamData)
		{
			chnNum = NETSDK_json_get_int(devCoverParamData, "channelNum");
			devCS_enable = NETSDK_json_get_boolean(devCoverParamData, "enabled");
			maxRegion = NETSDK_json_get_int(devCoverParamData, "maxRegion");

			Regions = NETSDK_json_get_child(devCoverParamData, "Regions");// 获取内层数组的长度，"Regions"下的数组
			if(NULL != Regions)
			{
				arrlen_in = json_object_array_length(Regions); // Regions下的数组长度

				NETSDK_conf_vin_ch_get(1, &vin_ch);// vin_ch 获取到的是所有通道的数据

				if(0 == arrlen_in)
				{
					devCS_enable = false,
					region_X = 0,
					region_Y = 50,
					region_W = 50,
					region_H = 50;
					regionColor_num = 0xffffff;

					for(j = 0; j < 4; j++)
					{
						vin_ch.privacyMask[j].id = j + 1; // 保存到配置video_1.json文件的通道1，相当于设备的通道0
						vin_ch.privacyMask[j].enabled = devCS_enable;
						vin_ch.privacyMask[j].regionX = region_X;
						vin_ch.privacyMask[j].regionY = region_Y;
						vin_ch.privacyMask[j].regionWidth = region_W;
						vin_ch.privacyMask[j].regionHeight = region_H;
						vin_ch.privacyMask[j].regionColor = regionColor_num;

						// 设置区域
						cover.enable = devCS_enable;
						cover.x = region_X/100;
						cover.y = region_Y/100;
						cover.width = region_W/100;
						cover.height = region_H/100;
						cover.color = regionColor_num | 0xff000000;
                        if(NULL != sdk_vin && NULL != sdk_vin->set_cover)
                        {
                            sdk_vin->set_cover(0, j, &cover);
                        }
					}
				}
				else
				{

					//先把数据设成默认值，下面在更新到配置文件
					for(j = 0; j < 4; j++)
					{
						vin_ch.privacyMask[j].id = j + 1; // 保存到配置video_1.json文件的通道1，相当于设备的通道0
						vin_ch.privacyMask[j].enabled = false;
						vin_ch.privacyMask[j].regionX = 0;
						vin_ch.privacyMask[j].regionY = 50;
						vin_ch.privacyMask[j].regionWidth = 50;
						vin_ch.privacyMask[j].regionHeight = 50;
						vin_ch.privacyMask[j].regionColor = 0xffffff;

						cover.enable = false;
						cover.x = 0;
						cover.y = 50/100;
						cover.width = 50/100;
						cover.height = 50/100;
						cover.color = 0xffffffff;
                        if(NULL != sdk_vin && NULL != sdk_vin->set_cover)
                        {
                            sdk_vin->set_cover(0, j, &cover);  // 设置通道0，id从0开始
                        }
					}

					for(j = 0; j < arrlen_in; j++) // 遍历传过来的数据的所有区域，更新数据
					{
						LP_JSON_OBJECT regionsParamData = json_object_array_get_idx(Regions, j);
						if(NULL != regionsParamData)
						{
							region_X = NETSDK_json_get_float(regionsParamData, "regionX");
							region_Y = NETSDK_json_get_float(regionsParamData, "regionY");
							region_W = NETSDK_json_get_float(regionsParamData, "regionW");
							region_H = NETSDK_json_get_float(regionsParamData, "regionH");
							NETSDK_json_get_string(regionsParamData, "regionColor", region_Color, sizeof(region_Color));

							// 保存到配置文件，为了开机启动
							vin_ch.privacyMask[j].id = j + 1; // 保存到配置video_1.json文件的通道1，相当于设备的通道0
							vin_ch.privacyMask[j].enabled = devCS_enable;
							vin_ch.privacyMask[j].regionX = region_X;
							vin_ch.privacyMask[j].regionY = region_Y;
							vin_ch.privacyMask[j].regionHeight = region_H;
							vin_ch.privacyMask[j].regionWidth = region_W;

							regionColor_num = strtol(region_Color, NULL, 16);
							vin_ch.privacyMask[j].regionColor = regionColor_num;

							// 设置区域
							cover.enable = devCS_enable;
							cover.x = region_X/100;
							cover.y = region_Y/100;
							cover.width = region_W/100;
							cover.height = region_H/100;
							cover.color = regionColor_num | 0xff000000;
						}
                        if(NULL != sdk_vin && NULL != sdk_vin->set_cover)
                        {
                            sdk_vin->set_cover(0, j, &cover);  // 设置通道0，id从0开始
                        }
					}

				}
			}
		}
	}

	NETSDK_conf_vin_ch_set(1, &vin_ch);// 保存到配置文件，用于开机启动，id从1开始

}

ssize_t p2p_parse(void *request, void *response, int max_response_len)
{
	const ST_NSDK_MAP_STR_DEC promptSoundType_map[] = {
		{"chinese", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_CHINESE},
		{"english", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_ENGLISH},
		{"german", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_GERMAN},
		{"korean", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_KOREAN},
		{"portuguese", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_PORTUGUESE},
		{"russian", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_RUSSIAN},
		{"spanish", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_SPANISH}
	};

#if defined(TS_RECORD)
    const ST_NSDK_MAP_STR_DEC tfcardStatus_map[] = {
        {"ok", EN_NK_TFCARD_STATUS_OK},
        {"already_format", EN_NK_TFCARD_STATUS_FORMATED},
        {"no_format", EN_NK_TFCARD_STATUS_NO_FORMAT},
        {"exception", EN_NK_TFCARD_STATUS_EXCEPTION},
        {"no_tfcard", EN_NK_TFCARD_STATUS_NO_TFCARD},
        {"formatting", EN_NK_TFCARD_STATUS_FORMATTING},
        {"abnormal", EN_NK_TFCORD_STATUS_NO_ABNORMAL}
    };
#endif

	LP_JSON_OBJECT obj; 
	int ret = 0;
			
	obj = NETSDK_json_parse(request);
	if(NULL != obj){
		//userУ��
		if(!p2p_Auth_Verify_pass(request))
		{
			json_object_put(obj);
			return -1;
		}
		
		if(p2p_setup_is_get(request)){
			//get
			LP_JSON_OBJECT ipcam=NULL, devInfo=NULL, modeSetinfo=NULL, fisheyeSetinfo = NULL, Psound = NULL, Tfcard = NULL, alarmSetinfo=NULL, motinfo=NULL, push_msg=NULL, SystemOper=NULL, timeS=NULL, upginfo=NULL;			
            LP_JSON_OBJECT ledPwm = NULL, wirelessManager = NULL;
            LP_JSON_OBJECT wirelessStation = NULL;
            LP_JSON_OBJECT videoManager = NULL;
            LP_JSON_OBJECT ptz = NULL;
            LP_JSON_OBJECT devCoverSetting = NULL;
            LP_JSON_OBJECT optTmp = NULL;
            LP_JSON_OBJECT tmpJson = NULL;
            LP_JSON_OBJECT capabilitySet = NULL;
            LP_JSON_OBJECT record=NULL;
            ST_NSDK_VIN_CH vin_ch;

            /* 能力集获取 */
            capabilitySet = NETSDK_json_get_child(obj, "CapabilitySet");
            if(NULL != capabilitySet)
            {
                p2pParseCapabilitysetGet(capabilitySet);
            }

			ipcam = NETSDK_json_get_child(obj, "IPCam");
			if(NULL!=ipcam){
				//DeviceInfo
				devInfo = NETSDK_json_get_child(ipcam, "DeviceInfo");
				
				if(NULL!=devInfo){
					ST_NSDK_SYSTEM_DEVICE_INFO sysInfo;
					NETSDK_conf_system_get_device_info(&sysInfo);
					int m = 0, count=0;
					int n = strlen(sysInfo.firmwareVersion);
					//OEM number ----56112100
					char *oemstr = strrchr(sysInfo.firmwareVersion, '.');
					ST_CUSTOM_SETTING custom;
					if(0 == CUSTOM_get(&custom) && CUSTOM_check_string_valid(custom.model.oemNumber)){
						NETSDK_json_set_string2(devInfo, "OEMNumber", custom.model.oemNumber);
						APP_TRACE("SET OEM number:%s", custom.model.oemNumber);
					}else if(oemstr){
						char OEMNum[7];
						memset(OEMNum, 0, sizeof(OEMNum));
						memcpy(OEMNum, oemstr+1, 6);
						NETSDK_json_set_string2(devInfo, "OEMNumber", OEMNum);
					}

					while(m<n)
					{
						if(sysInfo.firmwareVersion[m] == '.')
						{
							count++;
							if(count == 3)
								break;
						}
						m++;
					}
					memset(sysInfo.firmwareVersion+m, 0, n-m);
					//1.4.2-----.56112100
					//memset(sysInfo.firmwareVersion+5, 0, sizeof(sysInfo.firmwareVersion)-5); 
					char version_tmp[64];
					snprintf(version_tmp, sizeof(version_tmp), "%s.0", sysInfo.firmwareVersion);
					NETSDK_json_set_string2(devInfo, "FWVersion", version_tmp);

					NETSDK_json_set_string2(devInfo, "Model", sysInfo.model);
					NETSDK_json_set_string2(devInfo, "ID", sysInfo.serialNumber);
					NETSDK_json_set_string2(devInfo, "FWMagic", "SlVBTiBJUENBTSBGSVJNV0FSRSBERVNJR05FRCBCWSBMQVc=");
				}

				//ModeSetting
				modeSetinfo = NETSDK_json_get_child(ipcam, "ModeSetting");
				if(modeSetinfo != NULL) {
                    ST_NSDK_AENC_CH aenc_ch;
                    ST_NSDK_IMAGE image;
                    char text[20]={0};
                    ST_NSDK_SYSTEM_SETTING sysInfo;
                    ST_CUSTOM_SETTING custom;
                    memset(&custom, 0, sizeof(ST_CUSTOM_SETTING));
                    NETSDK_conf_system_get_setting_info(&sysInfo);

                    NETSDK_conf_aenc_ch_get(101, &aenc_ch);
                    NETSDK_json_set_boolean2(modeSetinfo,"AudioEnabled", aenc_ch.enabled);

                    if(NETSDK_conf_image_get(&image)){
                        if(0 ==  image.sceneMode)
                        {
                            NETSDK_json_set_string2(modeSetinfo, "SceneMode", "auto");
                        }
                        if(1 ==  image.sceneMode)
                        {
                            NETSDK_json_set_string2(modeSetinfo, "SceneMode", "indoor");
                        }
                        if(2 ==  image.sceneMode)
                        {
                            NETSDK_json_set_string2(modeSetinfo, "SceneMode", "outdoor");
                        }
                        
                        // 暖光黑夜全彩枪机灯控
                        if(EN_NSDK_LIGHT_CTL_TYPE_WARMTH == sysInfo.capabilitySet.lightControl)
                        {
                            NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "auto");
                            optTmp = json_object_new_object();
                            tmpJson = NETSDK_json_parse("[ \"auto\" ]");
                            json_object_object_add(optTmp, "opt", tmpJson);
                            json_object_object_add(modeSetinfo, "IRCutFilterModeProperty", optTmp);
                        }
                        else
                        {
                            //irCutMode
                            // 区分Cx单品 自动|白天|夜晚
                            // 普通 红外模式|照明模式|智能模式
                            if((true == GLOBAL_sn_front()) || (0 == CUSTOM_get(&custom) && 0 < custom.function.pirEnabled)) {
                                if(kNSDK_IMAGE_IRCUT_MODE_AUTO == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "auto");
                                }
                                if(kNSDK_IMAGE_IRCUT_MODE_DAYLIGHT == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "daylight");
                                }
                                if(kNSDK_IMAGE_IRCUT_MODE_NIGHT == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "night");
                                }
                                optTmp = json_object_new_object();
                                tmpJson = NETSDK_json_parse("[ \"auto\", \"daylight\", \"night\"]");
                                json_object_object_add(optTmp, "opt", tmpJson);
                                json_object_object_add(modeSetinfo, "IRCutFilterModeProperty", optTmp);
                            }
                            else {
#if defined(BSD_CUSTOM)
                                if(kNSDK_IMAGE_IRCUT_MODE_AUTO == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "auto");
                                }
                                if(kNSDK_IMAGE_IRCUT_MODE_DAYLIGHT == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "daylight");
                                }
                                if(kNSDK_IMAGE_IRCUT_MODE_NIGHT == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "night");
                                }
                                optTmp = json_object_new_object();
                                tmpJson = NETSDK_json_parse("[ \"auto\", \"daylight\", \"night\"]");
#else
                                if(kNSDK_IMAGE_IRCUT_MODE_AUTO == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "ir");
                                }
                                if(kNSDK_IMAGE_IRCUT_MODE_LIGHTMODE == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "light");
                                }
                                if(kNSDK_IMAGE_IRCUT_MODE_SMARTMODE == image.irCutFilter.irCutMode)
                                {
                                    NETSDK_json_set_string2(modeSetinfo, "IRCutFilterMode", "smart");
                                }
                                optTmp = json_object_new_object();
                                tmpJson = NETSDK_json_parse("[ \"ir\", \"light\", \"smart\"]");
#endif
                                json_object_object_add(optTmp, "opt", tmpJson);
                                json_object_object_add(modeSetinfo, "IRCutFilterModeProperty", optTmp);
                            }
                        }
                    
                        char multi_conf_mode[16];
                        NETSDK_get_multi_conf_mode(multi_conf_mode);
                        //���������ڼ� ���ģʽ
                        NETSDK_json_set_string2(modeSetinfo, "ConvenientSetting", multi_conf_mode);
                    
                        //image style
                        if(1 == image.imageStyle){
                            NETSDK_json_set_string2(modeSetinfo, "imageStyle", "standard");
                        }else if(2 == image.imageStyle){
                            NETSDK_json_set_string2(modeSetinfo, "imageStyle", "bright");
                        }else if(3 == image.imageStyle){
                            NETSDK_json_set_string2(modeSetinfo, "imageStyle", "gorgeous");
                        }
                        
                    }
                    
                    //audio volume setting
                    LP_JSON_OBJECT audioVolumeSetting = NETSDK_json_get_child(modeSetinfo, "AudioVolume");
                    if(audioVolumeSetting){
                        ST_NSDK_AIN_CH ain;
                        NETSDK_conf_ain_ch_get(1, &ain);
                        NETSDK_json_set_int(audioVolumeSetting, "AudioInputVolume", ain.inputVolume);
                        NETSDK_json_set_int(audioVolumeSetting, "AudioOutputVolume", ain.outputVolume);
                    }
                    
                    //NETSDK_json_set_string2(modeSetinfo, "Definition", "auto");
                    //definition type
                    LP_JSON_OBJECT videoJSON = json_object_get(netsdk->video_conf);
                    LP_JSON_OBJECT channelListJSON = NETSDK_json_get_child(videoJSON, "videoEncode.videoEncodeChannel");
                    LP_JSON_OBJECT channel = p2p_video_find_channel(channelListJSON, 101);
                    char definitionType[16]={0};
                    
                    NETSDK_json_get_string(channel, "definitionType", definitionType, sizeof(definitionType));
                    //venc_ch->definitionType= NETSDK_MAP_STR2DEC(definition_type, definitionType, kNSDK_DEFINITION_AUTO);
                    NETSDK_json_set_string2(modeSetinfo, "Definition", definitionType);
                    if(NULL != videoJSON) {
                        json_object_put(videoJSON);
                        videoJSON = NULL;
                    }

                    NETSDK_conf_system_get_setting_info(&sysInfo);
                    NETSDK_json_set_string2(modeSetinfo, "area", sysInfo.area);
                    // powerLineFrequencyMode
                    NETSDK_conf_vin_ch_get(1, &vin_ch);
                    NETSDK_json_set_int2(modeSetinfo, "powerLineFrequencyMode", vin_ch.powerLineFrequencyMode);
                    optTmp = json_object_new_array();
                    if(NULL != optTmp) {
                        tmpJson = json_object_new_int(50);
                        json_object_array_add(optTmp, tmpJson);
                        tmpJson = json_object_new_int(60);
                        json_object_array_add(optTmp, tmpJson);
                        tmpJson = json_object_new_object();
                        if(NULL != tmpJson) {
                            json_object_object_add(tmpJson, "opt", optTmp);
                            json_object_object_add(modeSetinfo, "powerLineFrequencyModeProperty", tmpJson);
                        }
                    }
                }
				
				//AlarmSetting
				alarmSetinfo = NETSDK_json_get_child(ipcam, "AlarmSetting");
                if(alarmSetinfo != NULL) {
                    //MotionDetection
                    motinfo = NETSDK_json_get_child(alarmSetinfo, "MotionDetection");
                    if(motinfo != NULL) {
                        ST_NSDK_MD_CH md_ch;
                        ST_NSDK_SYSTEM_SETTING sysInfo;
                        if(NETSDK_conf_md_ch_get(1, &md_ch)){
                            NETSDK_json_set_boolean2(motinfo, "Enabled", md_ch.enabled);
                            set_motinfo(motinfo, md_ch.detectionGrid.sensitivityLevel);
                        }
                        if(NETSDK_conf_system_get_setting_info(&sysInfo)) {
                            NETSDK_json_set_boolean2(motinfo, "MotionWarningTone", sysInfo.mdAlarm.MotionWarningTone);
                        }
						if(NETSDK_conf_system_get_setting_info(&sysInfo)) {
                            NETSDK_json_set_boolean2(motinfo, "MotionRecord", sysInfo.MotionRecordEnabled);
                        }
                        char wtt_buf[32];
                        if(NULL != NETSDK_conf_system_get_setting_info(&sysInfo))
                        {
                            //if(true == IS_FILE_EXIST(DST_FILE_PATH_CST_SOUND))
                            if((true == IS_FILE_EXIST(DST_FILE_PATH_CST_SOUND)) && (0 == strcmp(sysInfo.mdAlarm.WarningToneType, "custom")))//文件存在 && system_1.json是custom
                            {
                                NETSDK_json_set_string2(motinfo, "WarningToneType", "custom");
                            }
                            else
                            {
                                NETSDK_json_set_string2(motinfo, "WarningToneType", "default");
                            }
                            optTmp = json_object_new_object();
                            tmpJson = NETSDK_json_parse("[ \"default\", \"custom\"]");
                            json_object_object_add(optTmp, "opt", tmpJson);
                            json_object_object_add(motinfo, "WarningToneTypeProperty", optTmp);
                        }

                    }
                    
                    //MessagePushEnabled
                    ST_NSDK_SYSTEM_SETTING sinfo;
                    NETSDK_conf_system_get_setting_info(&sinfo);
                    NETSDK_json_set_boolean2(alarmSetinfo, "MessagePushEnabled", sinfo.messagePushEnabled);
                    //MessagePushSchedule
                    LP_JSON_OBJECT  Schedule= NETSDK_json_get_child(alarmSetinfo, "MessagePushSchedule");
                    
                    if(Schedule){
                        int i;
                        for (i = 0
                                ; i < sizeof(sinfo.AlarmNotification.Schedule)
                                    / sizeof(sinfo.AlarmNotification.Schedule[0]);
                                ++i)
                        {
                            LP_JSON_OBJECT Scheduletime = json_object_array_get_idx(Schedule, i);
                            if(sinfo.AlarmNotification.Schedule[i].enabled)
                            {
                                char beginTime[64]={0};
                                char endTime[64]={0};
                                char weekday[64]={0};
                                
                                schedule_time_to_string(sinfo.AlarmNotification.Schedule[i].BeginTime.hour
                                        , sinfo.AlarmNotification.Schedule[i].BeginTime.min
                                        , sinfo.AlarmNotification.Schedule[i].BeginTime.sec
                                        , beginTime, sizeof(beginTime));
                        
                                schedule_time_to_string(sinfo.AlarmNotification.Schedule[i].EndTime.hour
                                        , sinfo.AlarmNotification.Schedule[i].EndTime.min
                                        , sinfo.AlarmNotification.Schedule[i].EndTime.sec
                                        , endTime, sizeof(endTime));
                        
                                schedule_weekday_to_string(sinfo.AlarmNotification.Schedule[i].weekday
                                        , weekday
                                        , sizeof(weekday));
                                if(!Scheduletime){
                                    Scheduletime = json_object_new_object();
                                    NETSDK_json_set_string2(Scheduletime, "Weekday", weekday);
                                    NETSDK_json_set_string2(Scheduletime, "BeginTime", beginTime);
                                    NETSDK_json_set_string2(Scheduletime, "EndTime", endTime);
                                    //NETSDK_json_set_int(Scheduletime, "id", i);
                                    NETSDK_json_set_int2(Scheduletime, "id", i);
                                    json_object_array_put_idx(Schedule, i, Scheduletime);
                                }
                            }
                        }
                    }
                    NETSDK_json_set_boolean2(alarmSetinfo, "ScheduleSupport", true);
                }
				

				//SystemOperation
				SystemOper = NETSDK_json_get_child(ipcam, "SystemOperation");
                if(SystemOper != NULL) {
                    //TimeSync
                    time_t utc_time;
                    char utc_time_str[15] = {0};
					LP_JSON_OBJECT UTCTime = NULL, TimeZone = NULL;
					ST_NSDK_SYSTEM_TIME sys_time;

                    timeS = NETSDK_json_get_child(SystemOper, "TimeSync");
					if (NULL != timeS) {
						utc_time = time(NULL);
						snprintf(utc_time_str, sizeof(utc_time_str), "%ld", utc_time);
						NETSDK_json_set_string(timeS, "UTCTime", utc_time_str);

						NETSDK_conf_system_get_time(&sys_time);
						NETSDK_json_set_int(timeS, "TimeZone", sys_time.greenwichMeanTime);
					}
                    
                    //if(timeS)
                    //  NETSDK_json_set_string2(timeS, "LocalTime", mtime);
                    
                    //Upgrade
                    upginfo = NETSDK_json_get_child(SystemOper, "Upgrade");

					LP_JSON_OBJECT dstJSON = NETSDK_json_get_child(SystemOper, "DaylightSavingTime");
					if(dstJSON != NULL){
						int i, n;
						LP_JSON_OBJECT weekJSON = NULL, tmpWeekJSON = NULL;;
						ST_NSDK_SYSTEM_DST dstInfo = {0};

						NETSDK_conf_system_get_DST_info(&dstInfo);
						NETSDK_json_set_boolean2(dstJSON, "Enabled", dstInfo.enable);
                        NETSDK_json_set_string2(dstJSON, "Country", dstInfo.country);
						NETSDK_json_set_int2(dstJSON, "Offset", dstInfo.offset);
						if((weekJSON = NETSDK_json_get_child(dstJSON, "Week")) != NULL){
							n = json_object_array_length(weekJSON);
							for(i = 0; i < n && i < sizeof(dstInfo.week) / sizeof(dstInfo.week[0]); i++){
								if((tmpWeekJSON = json_object_array_get_idx(weekJSON, i)) != NULL){
									NETSDK_json_set_string2(tmpWeekJSON, "Type", dstInfo.week[i].type);
									NETSDK_json_set_int2(tmpWeekJSON, "Month", dstInfo.week[i].month);
									NETSDK_json_set_int2(tmpWeekJSON, "Week", dstInfo.week[i].week);
									NETSDK_json_set_int2(tmpWeekJSON, "Weekday", dstInfo.week[i].weekday);
									NETSDK_json_set_int2(tmpWeekJSON, "Hour", dstInfo.week[i].hour);
									NETSDK_json_set_int2(tmpWeekJSON, "Minute", dstInfo.week[i].minute);
								}
							}
						}
					}
                }

				//PromptSounds
				ST_NSDK_SYSTEM_SETTING pinfo;				
				Psound = NETSDK_json_get_child(ipcam, "PromptSounds");
				if(Psound){
					NETSDK_conf_system_get_setting_info(&pinfo);
					NETSDK_json_set_boolean2(Psound, "Enabled", pinfo.promptSound.enabled);
					/*if(pinfo.promptSound.soundType == 0){
						NETSDK_json_set_string2(Psound, "Type", "chinese");
					}
					else if(pinfo.promptSound.soundType == 1){
						NETSDK_json_set_string2(Psound, "Type", "english");
					}*/
					NETSDK_json_set_string2(Psound, "Type", pinfo.promptSound.soundTypeStr);
					LP_JSON_OBJECT optionFromJSON = NETSDK_json_parse(pinfo.promptSound.soundTypeOpt);
					json_object_object_add(Psound, "TypeOption", optionFromJSON);
				}

				Tfcard = NETSDK_json_get_child(ipcam, "TfcardManager");
				if(Tfcard){
					unsigned int spacesize=0;
					unsigned int totalsize=0;
					int flag = 0;
					unsigned char spacestr[10]={0};
					unsigned char totalspace[10]={0};

#if defined(TS_RECORD)
                    char *tfcard_status = NULL;
                    int status;
                    status = NK_TFCARD_GetStatus();
                    if(EN_NK_TFCARD_STATUS_OK == status)
                    {
                        totalsize = NK_TFCARD_GetCapacity();
                        snprintf(totalspace, sizeof(totalspace), "%d", totalsize);
                        if(0 != strlen(totalspace))
                        {
                            NETSDK_json_set_string2(Tfcard, "TotalSpacesize", totalspace);
                        }

                        spacesize = NK_TFCARD_GetFreeSpace();
                        snprintf(spacestr, sizeof(spacestr), "%d", spacesize);
                        if(0 != strlen(spacestr))
                        {
                            NETSDK_json_set_string2(Tfcard, "LeaveSpacesize", spacestr);
                        }
                    }
                    tfcard_status = NETSDK_MAP_DEC2STR(tfcardStatus_map, status, "no_tfcard");
                    NETSDK_json_set_string2(Tfcard, "Status", tfcard_status);
#else
#if defined(TFCARD)
					char tfcard_status[32];
					int status;
					status = NK_TFCARD_get_status(tfcard_status);
					if(emTFCARD_STATUS_OK == status){
						totalsize = NK_TFCARD_get_capacity();
						snprintf(totalspace, sizeof(totalspace), "%d", totalsize);
						if(0 != strlen(totalspace)){
							NETSDK_json_set_string2(Tfcard, "TotalSpacesize", totalspace);
						}
						
						spacesize = NK_TFCARD_get_freespace();
						snprintf(spacestr, sizeof(spacestr), "%d", spacesize);
						if(0 != strlen(spacestr)){
							NETSDK_json_set_string2(Tfcard, "LeaveSpacesize", spacestr);
						}
					}
					NETSDK_json_set_string2(Tfcard, "Status", tfcard_status);
#endif
#endif

					ST_NSDK_SYSTEM_SETTING minfo = {0};
					NETSDK_conf_system_get_setting_info(&minfo);
                    NETSDK_json_set_boolean2(Tfcard, "TimeRecordEnabled", minfo.timeRecordEnabled);
					LP_JSON_OBJECT recordSchedule = NETSDK_json_get_child(Tfcard,"TFcard_recordSchedule");
					if(recordSchedule){
						int i;
						for(i = 0
								; i < sizeof(minfo.TFcard_Record.Schedule)
									/ sizeof(minfo.TFcard_Record.Schedule[0]);
								i++)
						{
							LP_JSON_OBJECT recordScheduleTime = json_object_array_get_idx(recordSchedule,i);
							if(minfo.TFcard_Record.Schedule[i].enabled)
							{
								char beginTime[64] = {0};
								char endTime[64] = {0};
								char weekday[64] = {0};

								schedule_time_to_string(minfo.TFcard_Record.Schedule[i].BeginTime.hour
									, minfo.TFcard_Record.Schedule[i].BeginTime.min
									, minfo.TFcard_Record.Schedule[i].BeginTime.sec
									, beginTime, sizeof(beginTime));
								schedule_time_to_string(minfo.TFcard_Record.Schedule[i].EndTime.hour
									, minfo.TFcard_Record.Schedule[i].EndTime.min
									, minfo.TFcard_Record.Schedule[i].EndTime.sec
									, endTime, sizeof(endTime));
								schedule_weekday_to_string(minfo.TFcard_Record.Schedule[i].weekday
									, weekday
									, sizeof(weekday));
								if(!recordScheduleTime){
									recordScheduleTime = json_object_new_object();
									NETSDK_json_set_string2(recordScheduleTime, "Weekday", weekday);
									NETSDK_json_set_string2(recordScheduleTime, "BeginTime", beginTime);
									NETSDK_json_set_string2(recordScheduleTime, "EndTime", endTime);
									NETSDK_json_set_int2(recordScheduleTime, "id", i);
									json_object_array_put_idx(recordSchedule, i, recordScheduleTime);
								}
							}
						}
					}
					NETSDK_json_set_boolean2(Tfcard, "ScheduleSupport", true);
				}
#if 0
				record =  NETSDK_json_get_child(ipcam, "RecordManager");
				if(record != NULL){
					ST_NSDK_SYSTEM_REC_MANAGER recManager = {0};

					NETSDK_conf_system_get_record_info(&recManager);
					NETSDK_json_set_string2(record, "Mode", recManager.recMode);
					NETSDK_json_set_boolean2(record, "UseIOAlarm", recManager.useIOAlarm);
				}
#endif
				//FisheyeSetting
				fisheyeSetinfo = NETSDK_json_get_child(ipcam, "FisheyeSetting");
				if(fisheyeSetinfo){
					stFISHEYE_config conf = {0};
					int tmpFixMode;
					FISHEYE_config_get(&conf);

                    NETSDK_json_set_string2(fisheyeSetinfo, "LensName", conf.lensName);

					tmpFixMode = FISHEYE_get_fix_mode();
					if(eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL == tmpFixMode){
						NETSDK_json_set_string2(fisheyeSetinfo,"FixMode","wall");
					}else if(eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL == tmpFixMode){
						NETSDK_json_set_string2(fisheyeSetinfo,"FixMode","cell");
					}else if(eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE == tmpFixMode){
						NETSDK_json_set_string2(fisheyeSetinfo,"FixMode","table");
					}else{
						NETSDK_json_set_string2(fisheyeSetinfo,"FixMode","none");
					}

					LP_JSON_OBJECT fixParam = NETSDK_json_get_child(fisheyeSetinfo,"FixParam");
					if(fixParam){
						int i = 0;
						for(i = 0 ; i < (sizeof(conf.param) / sizeof(conf.param[0])) ; i++)
						{
							LP_JSON_OBJECT fixParamData = json_object_array_get_idx(fixParam,i);

							if(!fixParamData){
								fixParamData = json_object_new_object();
                                if(conf.type) {
                                    NETSDK_json_set_int2(fixParamData, "CenterCoordinateX", conf.param[i].CenterCoordinateX);
                                    NETSDK_json_set_int2(fixParamData, "CenterCoordinateY", conf.param[i].CenterCoordinateY);
                                    NETSDK_json_set_int2(fixParamData, "Radius", conf.param[i].Radius);
                                    NETSDK_json_set_int2(fixParamData, "AngleX", conf.param[i].AngleX);
                                    NETSDK_json_set_int2(fixParamData, "AngleY", conf.param[i].AngleY);
                                    NETSDK_json_set_int2(fixParamData, "AngleZ", conf.param[i].AngleZ);
                                }
                                else {
                                    NETSDK_json_set_float2(fixParamData, "CenterCoordinateX", conf.param2[i].CenterCoordinateX);
                                    NETSDK_json_set_float2(fixParamData, "CenterCoordinateY", conf.param2[i].CenterCoordinateY);
                                    NETSDK_json_set_float2(fixParamData, "Radius", conf.param2[i].Radius);
                                    NETSDK_json_set_float2(fixParamData, "AngleX", conf.param2[i].AngleX);
                                    NETSDK_json_set_float2(fixParamData, "AngleY", conf.param2[i].AngleY);
                                    NETSDK_json_set_float2(fixParamData, "AngleZ", conf.param2[i].AngleZ);
                                }
								NETSDK_json_set_int2(fixParamData, "id", i);
								json_object_array_put_idx(fixParam, i, fixParamData);
							}
						}
					}
				}

#if defined (LED_PWM)
                ledPwm = NETSDK_json_get_child(ipcam, "ledPwm");
                if(ledPwm == NULL) {
                    if(LED_PWM_is_pwm() == 0) {
                        int i = 0;
                        stLED_PWM_config ledPwmConfig;
                        LP_JSON_OBJECT child_json = NULL;

                        LED_PWM_get(&ledPwmConfig);
                        ledPwm = json_object_new_object();
                        if(ledPwm) {
                            NETSDK_json_set_int2(ledPwm, "channelCount", ledPwmConfig.channelCount);
                            NETSDK_json_set_int2(ledPwm, "switch", ledPwmConfig.ledSwitch);
                            LP_JSON_OBJECT child_json = json_object_new_array();
                            if(child_json) {
                                for(i = 0; i < ledPwmConfig.channelCount; i++) {
                                    LP_JSON_OBJECT info = json_object_array_get_idx(child_json, i);
                                    if(info == NULL) {
                                        info = json_object_new_object();
                                        NETSDK_json_set_int2(info, "type", ledPwmConfig.array[i].type);
                                        NETSDK_json_set_int2(info, "num", ledPwmConfig.array[i].num);
                                        NETSDK_json_set_int2(info, "channel", ledPwmConfig.array[i].channel);
                                        json_object_array_put_idx(child_json, i, info);
                                    }
                                }
                                json_object_object_add(ledPwm, "channelInfo", child_json);
                                json_object_object_add(ipcam, "ledPwm", ledPwm);
                            }
                        }
                    }
                }
                else {
                    if(LED_PWM_is_pwm() == 0)
                    {
                        int i = 0;
                        stLED_PWM_config ledPwmConfig;
                        LP_JSON_OBJECT child_json = NULL;

                        LED_PWM_get(&ledPwmConfig);
                        NETSDK_json_set_int2(ledPwm, "channelCount", ledPwmConfig.channelCount);
                        NETSDK_json_set_int2(ledPwm, "switch", ledPwmConfig.ledSwitch);
                        child_json = NETSDK_json_get_child(ledPwm, "channelInfo");
                        if(child_json) {
                            for(i = 0; i < ledPwmConfig.channelCount; i++) {
                                LP_JSON_OBJECT info = json_object_array_get_idx(child_json, i);
                                if(info == NULL) {
                                    info = json_object_new_object();
                                    NETSDK_json_set_int2(info, "type", ledPwmConfig.array[i].type);
                                    NETSDK_json_set_int2(info, "num", ledPwmConfig.array[i].num);
                                    NETSDK_json_set_int2(info, "channel", ledPwmConfig.array[i].channel);
                                    json_object_array_put_idx(child_json, i, info);
                                }
                            }
                        }
                    }
                }
#endif
                wirelessManager = NETSDK_json_get_child(ipcam, "WirelessManager");
                if(wirelessManager != NULL) {
#if defined (CX)
                    if(false == GLOBAL_sn_front()) {
                        ST_NSDK_NETWORK_INTERFACE inter;
                        char base64ApPsk[128];
                        NETSDK_conf_interface_get(4, &inter);
                        if(inter.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT) {
                            memset(base64ApPsk, 0, sizeof(base64ApPsk));
                            BASE64_encode(inter.wireless.wirelessApMode.wirelessPsk, strlen(inter.wireless.wirelessApMode.wirelessPsk),
                                    base64ApPsk, sizeof(base64ApPsk));
                            NETSDK_json_set_string2(wirelessManager, "ApPsk", base64ApPsk);
                        }
                    }
#endif
                }

                /*
                 * wirelessStation
                 */
                 wirelessStation = NETSDK_json_get_child(ipcam, "WirelessStation");
                 if(NULL != wirelessStation)
                 {
                     p2p_parse_wireleseStationGet(wirelessStation);
                 }

                 /*
                  * videoManager
                  */
                 videoManager = NETSDK_json_get_child(ipcam, "videoManager");
                 if(NULL != videoManager)
                 {
                    p2p_parse_videoManagerGet(videoManager);
                 }

                 /*
                  * ptz
                  */
                 ptz = NETSDK_json_get_child(ipcam, "ptzManager");
                 if(NULL != ptz)
                 {
                    p2p_parse_ptzGet(ptz);
                 }

#if defined(CLOUD_REC)
				do {
					LP_JSON_OBJECT osscloudSetting;
					LP_JSON_OBJECT isBoundJson;
					LP_JSON_OBJECT uploadArrJson;
					LP_JSON_OBJECT arrEleJson;
					LP_JSON_OBJECT chIdJson;
					LP_JSON_OBJECT enabledJson;
					ST_NSDK_NETWORK_OSSCLD osscld;
					int i;
					int arrayLen;
					int chNum;
					bool errInFor;

					osscloudSetting = NETSDK_json_get_child(ipcam, "OsscloudSetting");
					if (NULL == osscloudSetting) {
						break;
					}

					if (NULL == NETSDK_conf_osscloud_get(&osscld)) {
						APP_TRACE("%s: Failed to get OSS Cloud Config", __FUNCTION__);
						break;
					}

					NETSDK_json_set_boolean(osscloudSetting, "IsBound", osscld.isBound);
					NETSDK_json_set_int(osscloudSetting, "ChNum", osscld.chNum);

					uploadArrJson = json_object_new_array();
					if(NULL == uploadArrJson) {
						APP_TRACE("%s: Failed to get new json array", __FUNCTION__);
						break;
					}

					// fixed as main stream
					chNum = osscld.chNum;
					errInFor = false;
					for (i = 0; i < chNum; i++) {
						arrEleJson = json_object_new_object();
						if(NULL == arrEleJson) {
							APP_TRACE("%s: Failed to get new json object", __FUNCTION__);
							errInFor = true;
							break;
						}
						json_object_array_add(uploadArrJson, arrEleJson);
						NETSDK_json_set_int(arrEleJson, "ID", i+1);
						NETSDK_json_set_boolean(arrEleJson, "Enabled", osscld.channel[i].stream[0].enable);
						NETSDK_json_set_int(arrEleJson, "Type", osscld.channel[i].stream[0].type);
					}

					if (errInFor) {
						json_object_put(uploadArrJson);
						break;
					}

					json_object_object_del(osscloudSetting, "Upload");
					json_object_object_add(osscloudSetting, "Upload", uploadArrJson);


				} while(0);
#endif
				 /*
				  * recordInfo
				  */
				 LP_JSON_OBJECT recordInfo;
				 LP_JSON_OBJECT recordScheduleDateInfo;
				 LP_JSON_OBJECT arrEleJson;

				 int arrLen = 0;
				 int i = 0, chnNum = 0;
				 time_t	beginTimes = 0, endTimes = 0;
				 char ts_dateinfo[256] = {0};

				 //LP_JSON_OBJECT fixParam = NETSDK_json_get_child(fisheyeSetinfo, "FixParam");
				 recordInfo = NETSDK_json_get_child(ipcam, "recordInfo");
				 if (NULL != recordInfo) {
					 recordScheduleDateInfo = NETSDK_json_get_child(recordInfo, "recordScheduleDateInfo");
					 if(NULL != recordScheduleDateInfo){
						 arrLen = json_object_array_length(recordScheduleDateInfo);
						 for(i = 0; i < arrLen; i++){
							 LP_JSON_OBJECT reoordParamData = json_object_array_get_idx(recordScheduleDateInfo,i);
							 if(NULL != reoordParamData){
								 chnNum = NETSDK_json_get_int(reoordParamData, "chnNum");
								 if(0 == chnNum)
								 {
								    beginTimes = NETSDK_json_get_int(reoordParamData, "beginTimeS");
								    endTimes   = NETSDK_json_get_int(reoordParamData, "endTimeS");

									//printf("\n\n### beginTimes = %d, endTimes =%d\n\n", beginTimes, endTimes);
								 }
							 }
						 }
					 }
				 }
				 //functions
				 if((true == TFCARD_OnExistTF()) && (true == TFCARD_OnDetectTF())) // 判断TF卡的挂载情况
				 {
                    dateinfo_filtrate(beginTimes, endTimes, ts_dateinfo);
					printf("### dateinfo : have tfcard !!\n");
				 }
				 else
				 {
					snprintf(ts_dateinfo, sizeof(ts_dateinfo), "%s", "[]");
					printf("### dateinfo : no tfcard !!\n");
				 }

				 recordInfo = NETSDK_json_get_child(ipcam, "recordInfo");
				 if (NULL != recordInfo) {

					 recordScheduleDateInfo = json_object_new_array();
					 if(NULL != recordScheduleDateInfo) {

						 for (i = 0; i < (chnNum + 1); i++) {
							 arrEleJson = json_object_new_object();

							 if(NULL != arrEleJson) {

								 json_object_array_add(recordScheduleDateInfo, arrEleJson);

								 NETSDK_json_set_int(arrEleJson, "chnNum", chnNum);
								 NETSDK_json_set_int(arrEleJson, "beginTimeS", beginTimes);
								 NETSDK_json_set_int(arrEleJson, "endTimeS", endTimes);
								// NETSDK_json_set_string(arrEleJson, "recordDay", ts_dateinfo);

								 tmpJson = NETSDK_json_parse(ts_dateinfo);
								 json_object_object_add(arrEleJson, "recordDay", tmpJson);
							 }
						 }

						 json_object_object_del(recordInfo, "recordScheduleDateInfo");
						 json_object_object_add(recordInfo, "recordScheduleDateInfo", recordScheduleDateInfo);
					 }
				}

				 /*
				  * devCoverSetting
				  */
				 devCoverSetting = NETSDK_json_get_child(ipcam, "devCoverSetting");
				 if(NULL != devCoverSetting)
				 {
					p2p_parse_devCoverGet(devCoverSetting);
				 }

			}

            ret = snprintf(response, max_response_len, "%s", json_object_to_json_string(obj));
            if(NULL != obj)
            {
                json_object_put(obj);
                obj = NULL;
            }

            if(ret <= 0 || ret > strlen(response)){
                return -2;
            }

            return strlen(response);
		}else{
				//set	
				LP_JSON_OBJECT ipcam=NULL, devInfo=NULL, modeSetinfo=NULL, fisheyeSetinfo = NULL, alarmSetinfo=NULL, Psound =NULL, Tfcard = NULL, motinfo=NULL, SystemOper=NULL, timeS=NULL, upginfo=NULL;
                LP_JSON_OBJECT ledPwm = NULL, wirelessManager = NULL;
                LP_JSON_OBJECT wirelessStation = NULL;
                LP_JSON_OBJECT videoManager = NULL;
                LP_JSON_OBJECT ptz = NULL;
                LP_JSON_OBJECT record = NULL;
                LP_JSON_OBJECT devCoverSetting = NULL;
                int ain = 0;
				ST_NSDK_AENC_CH aenc_ch;
				ST_NSDK_IMAGE image;
                ST_NSDK_VIN_CH vin_ch;
                int plFrequencyMode;

				ipcam = NETSDK_json_get_child(obj, "IPCam");
				if(NULL!=ipcam){
					//ModeSetting
					//AudioEnabled
					modeSetinfo = NETSDK_json_get_child(ipcam, "ModeSetting");

					if(NULL != modeSetinfo){
						char Convenientset[16];
                        ST_NSDK_SYSTEM_TIME sys_time;
                        bool got_time = true;
						if(NULL != NETSDK_json_get_string(modeSetinfo, "ConvenientSetting", Convenientset, sizeof(Convenientset))){
							if(0 == NETSDK_set_multi_conf_mode(Convenientset)){
                                if (NULL == NETSDK_conf_system_get_time(&sys_time)) {
                                    APP_TRACE("NETSDK_conf_system_get_time Failed!");
                                    got_time = false;
                                }

                                //setting mode sucess
								NETSDK_conf_load(true);
                                custom_conf_match();
                                 // sync time setting
                                if (got_time) {
                                    if (NULL == NETSDK_conf_system_set_time(&sys_time)) {
                                        APP_TRACE("NETSDK_conf_system_set_time Failed!");
                                    }
#if defined(TS_RECORD)
                                    TICKER_add_task(ipcam_timer_record, 5, false);
#endif
                                }
							}
							//json_object_put(obj);
							//sprintf((char *)response, "{\r\n\"option\" :\"success\"\r\n}\r\n");
							//return strlen((char *)response);
						}
					}

					if(NULL != modeSetinfo)
					{
						NETSDK_conf_aenc_ch_get(101, &aenc_ch);
                        if(NETSDK_json_check_child(modeSetinfo, "AudioEnabled")) {
    						if(NETSDK_json_get_boolean(modeSetinfo, "AudioEnabled"))
    						{
                                if(false == aenc_ch.enabled) {
                                    aenc_ch.enabled = true;

                                    NETSDK_conf_aenc_ch_set(101, &aenc_ch);

                                    if(sdk_enc)
                                    {
                                        sdk_enc->create_audio_stream(0, 0, aenc_ch.codecType);
                                    }
                                }
    						}
    						else
    						{
                                if(true == aenc_ch.enabled) {
                                    aenc_ch.enabled = false;
                                    NETSDK_conf_aenc_ch_set(101, &aenc_ch);
                                    if(sdk_enc){
                                        sdk_enc->release_stream_g711a(0);
                                    }
                                }
    						}
                        }
					}

					//ModeSetting
					ST_NSDK_IMAGE image;
					char *str = NULL;
					char s_mode[10]={0};
					bool image_setting_flag = false;
					char text[32] = {0};
					int sceneModeTmp = 0;
					if(NETSDK_conf_image_get(&image))
					{
						if(NULL != modeSetinfo)
						{
							if(NULL != NETSDK_json_get_string(modeSetinfo, "SceneMode", s_mode, sizeof(s_mode))){

								if(0 == strcmp(s_mode, "auto"))
								{
                                    sceneModeTmp = 0;
								}
								if(0 == strcmp(s_mode, "indoor"))
								{
                                    sceneModeTmp = 1;
								}
								if(0 == strcmp(s_mode, "outdoor"))
								{
                                    sceneModeTmp = 2;
								}
                                if(sceneModeTmp != image.sceneMode)
                                {
                                    NETSDK_conf_image_set(&image);
                                    netsdk_image_scene_mode_set(&image);
                                }
							}

							//irCutMode	
							if(NULL != NETSDK_json_get_string(modeSetinfo, "IRCutFilterMode", s_mode, sizeof(s_mode))){
                                ST_NSDK_SYSTEM_SETTING sinfo;
                                ST_CUSTOM_SETTING custom;
                                memset(&custom, 0, sizeof(ST_CUSTOM_SETTING));
                                NETSDK_conf_system_get_setting_info(&sinfo);
                                if(EN_NSDK_LIGHT_CTL_TYPE_WARMTH != sinfo.capabilitySet.lightControl)
                                {

                                    // 区分Cx单品 自动|白天|夜晚
                                    // 普通 红外模式|照明模式|智能模式
                                    if((true == GLOBAL_sn_front()) || (0 ==  CUSTOM_get(&custom) && 0 < custom.function.pirEnabled)) {
                                        if(0 == strcmp(s_mode, "auto"))
                                        {
                                            image.irCutFilter.irCutMode= kNSDK_IMAGE_IRCUT_MODE_AUTO;
                                        }
                                        else if(0 == strcmp(s_mode, "daylight"))
                                        {
                                            image.irCutFilter.irCutMode = kNSDK_IMAGE_IRCUT_MODE_DAYLIGHT;
                                        }
                                        else if(0 == strcmp(s_mode, "night"))
                                        {
                                            image.irCutFilter.irCutMode = kNSDK_IMAGE_IRCUT_MODE_NIGHT;
                                        }
                                    }
                                    else {
#if defined(BSD_CUSTOM)
                                        if(0 == strcmp(s_mode, "auto"))
                                        {
                                            image.irCutFilter.irCutMode= kNSDK_IMAGE_IRCUT_MODE_AUTO;
                                        }
                                        else if(0 == strcmp(s_mode, "daylight"))
                                        {
                                            image.irCutFilter.irCutMode = kNSDK_IMAGE_IRCUT_MODE_DAYLIGHT;
                                        }
                                        else if(0 == strcmp(s_mode, "night"))
                                        {
                                            image.irCutFilter.irCutMode = kNSDK_IMAGE_IRCUT_MODE_NIGHT;
                                        }
#else
                                        if(0 == strcmp(s_mode, "ir"))
                                        {
                                            image.irCutFilter.irCutMode= kNSDK_IMAGE_IRCUT_MODE_AUTO;
                                        }
                                        else if(0 == strcmp(s_mode, "light"))
                                        {
                                            image.irCutFilter.irCutMode = kNSDK_IMAGE_IRCUT_MODE_LIGHTMODE;
                                        }
                                        else if(0 == strcmp(s_mode, "smart"))
                                        {
                                            image.irCutFilter.irCutMode = kNSDK_IMAGE_IRCUT_MODE_SMARTMODE;
                                        }
#endif
                                    }
                                    image_setting_flag = true;
                                }
							}

							//imageStyle
							if(NULL != NETSDK_json_get_string(modeSetinfo, "imageStyle", text, sizeof(text))){
								if(!strcmp(text, "standard")){
									image.imageStyle = 1;
								}else if(!strcmp(text, "bright")){
									image.imageStyle = 2;
								}else if(!strcmp(text, "gorgeous")){
									image.imageStyle = 3;
								}
								image_setting_flag = true;
							}

							if(image_setting_flag){
								//need to be setting
								NETSDK_conf_image_set(&image);
								netsdk_image_changed2(&image);
							}
						}
					}

					//audio volume setting
					bool audio_setting_flag = false;
					LP_JSON_OBJECT audioVolumeSetting = NETSDK_json_get_child(modeSetinfo, "AudioVolume");
					if(audioVolumeSetting){
						ST_NSDK_AIN_CH ain;
						NETSDK_conf_ain_ch_get(1, &ain);
						if(NETSDK_json_check_child(audioVolumeSetting, "AudioInputVolume")){
							ain.inputVolume = NETSDK_json_get_int(audioVolumeSetting, "AudioInputVolume");
							audio_setting_flag = true;
						}
						if(NETSDK_json_check_child(audioVolumeSetting, "AudioOutputVolume")){
							ain.outputVolume = NETSDK_json_get_int(audioVolumeSetting, "AudioOutputVolume");
							audio_setting_flag = true;
						}
						if(audio_setting_flag){
							NETSDK_conf_ain_ch_set(1, &ain);
							//BSP_Audio_set_volume_val(-1, ain.inputVolume, -1, ain.outputVolume);
						}
					}

					//Definition����
					//definition type
					if(modeSetinfo){
						const ST_NSDK_MAP_STR_DEC definition_type[] = {
							{"auto", kNSDK_DEFINITION_AUTO},
							{"fluency", kNSDK_DEFINITION_FLUENCY},
							{"BD", kNSDK_DEFINITION_BD},
							{"HD", kNSDK_DEFINITION_HD},
						};
						LP_JSON_OBJECT videoJSON = json_object_get(netsdk->video_conf);
						LP_JSON_OBJECT channelListJSON = NETSDK_json_get_child(videoJSON, "videoEncode.videoEncodeChannel");
						LP_JSON_OBJECT channel = p2p_video_find_channel(channelListJSON, 101);
						char definitionType[16];
						ST_NSDK_VENC_CH venc_ch;
						int tmpDefinitionType = 0;
						NETSDK_conf_venc_ch_get(101, &venc_ch);

						if(NULL != NETSDK_json_get_string(modeSetinfo, "Definition", definitionType, sizeof(definitionType))){
                            tmpDefinitionType = NETSDK_MAP_STR2DEC(definition_type, definitionType, 0);
                            if(tmpDefinitionType != venc_ch.definitionType) {
                                venc_ch.definitionType = tmpDefinitionType;
                                if(NULL != NETSDK_conf_venc_ch_set(101, &venc_ch)){
                                    netsdk_venc_ch_changed(101, &venc_ch);
                                }
                            }
						}
                        if(NULL != videoJSON) {
                            json_object_put(videoJSON);
                            videoJSON = NULL;
                        }

                        // powerLineFrequencyMode
                        if(NETSDK_json_check_child(modeSetinfo, "powerLineFrequencyMode")) {
                            ST_CUSTOM_SETTING custom;
                            NETSDK_conf_vin_ch_get(1, &vin_ch);
                            plFrequencyMode = NETSDK_json_get_int(modeSetinfo, "powerLineFrequencyMode");
                            vin_ch.powerLineFrequencyMode = plFrequencyMode;
                            if(0 == CUSTOM_get(&custom)) {
                                if(custom.function.powerLineFrequencyMode != plFrequencyMode)
                                {
                                    custom.function.powerLineFrequencyMode = plFrequencyMode;
                                    CUSTOM_set(&custom);
                                }
                            }
                            if(netsdk->videoInputChannelChanged) {
                                netsdk->videoInputChannelChanged(1, &vin_ch);
                            }
                            NETSDK_conf_vin_ch_set(1, &vin_ch);
                        }
					}		
                    if(NULL != modeSetinfo) {
                        if(NULL != NETSDK_json_get_string(modeSetinfo, "area", text, sizeof(text))) {
                            ST_NSDK_SYSTEM_SETTING sysInfo;
                            NETSDK_conf_system_get_setting_info(&sysInfo);
                            snprintf(sysInfo.area, sizeof(sysInfo.area), "%s", text);
                            NETSDK_conf_system_set_setting_info(&sysInfo);
                        }
                    }

					//AlarmSetting
					alarmSetinfo = NETSDK_json_get_child(ipcam, "AlarmSetting");
                    char level[25];
					char customflag[32];

					if(NULL != alarmSetinfo){						
						//MotionDetection
						motinfo = NETSDK_json_get_child(alarmSetinfo, "MotionDetection");						
						//MessagePushEnabled
						ST_NSDK_SYSTEM_SETTING sinfo;
						NETSDK_conf_system_get_setting_info(&sinfo);

                        if(NETSDK_json_check_child(alarmSetinfo, "MessagePushEnabled")) {
    						if(NETSDK_json_get_boolean(alarmSetinfo, "MessagePushEnabled"))
    						{				
    							sinfo.messagePushEnabled = true;
    							//ESEE_msg_push();
    						}
    						else
    						{
    							sinfo.messagePushEnabled = false;
    						}
                        }
						
						
						//MessagePushSchedule
						int i = 0;
						int arrN = 0;
						LP_JSON_OBJECT arr = NETSDK_json_get_child(alarmSetinfo, "MessagePushSchedule");
						if(arr){
							// ��ռ�¼
							for(i = 0
									; i < sizeof(sinfo.AlarmNotification.Schedule)
										/ sizeof(sinfo.AlarmNotification.Schedule[0])
									; ++i)
							{
								sinfo.AlarmNotification.Schedule[i].enabled = false;
							}

							arrN = json_object_array_length(arr);
							for(i = 0; i < arrN
										&& i < sizeof(sinfo.AlarmNotification.Schedule)
											/ sizeof(sinfo.AlarmNotification.Schedule[0])
									; i++)
							{					
								LP_JSON_OBJECT Scheduletime = json_object_array_get_idx(arr, i);
								if(Scheduletime)
								{
									char beginTime[12]={0};
									char endTime[12]={0};
									char weekday[18]={0};
									int beginSec, endSec;
									NETSDK_json_get_string(Scheduletime, "Weekday", weekday, sizeof(weekday));
									NETSDK_json_get_string(Scheduletime, "BeginTime", beginTime, sizeof(beginTime));
									NETSDK_json_get_string(Scheduletime, "EndTime", endTime, sizeof(endTime));
						
									if(NULL != beginTime && NULL != endTime && NULL != weekday)
									{
										if(0 != schedule_parse_time(beginTime
												, &sinfo.AlarmNotification.Schedule[i].BeginTime.hour
												, &sinfo.AlarmNotification.Schedule[i].BeginTime.min
												, &sinfo.AlarmNotification.Schedule[i].BeginTime.sec))
										{
											APP_TRACE("Parse Begin Time Failed");
											continue;
										}
						
										if(0 != schedule_parse_time(endTime
												, &sinfo.AlarmNotification.Schedule[i].EndTime.hour
												, &sinfo.AlarmNotification.Schedule[i].EndTime.min
												, &sinfo.AlarmNotification.Schedule[i].EndTime.sec))
										{
											APP_TRACE("Parse End Time Failed");
											continue;
										}
						
										if(0 != schedule_parse_weekday(weekday
												, &sinfo.AlarmNotification.Schedule[i].weekday))
										{
											APP_TRACE("Parse Weekday Failed");
											continue;
										}
						
										sinfo.AlarmNotification.Schedule[i].weekday &= 0x7f;
										beginSec = sinfo.AlarmNotification.Schedule[i].BeginTime.hour * 3600
												+ sinfo.AlarmNotification.Schedule[i].BeginTime.min * 60
												+ sinfo.AlarmNotification.Schedule[i].BeginTime.sec;
										endSec = sinfo.AlarmNotification.Schedule[i].EndTime.hour * 3600
												+ sinfo.AlarmNotification.Schedule[i].EndTime.min * 60
												+ sinfo.AlarmNotification.Schedule[i].EndTime.sec;
						
										// �ж������Լ�ʱ��ĺϷ���
										if(0 != sinfo.AlarmNotification.Schedule[i].weekday
												&& beginSec < endSec)
										{
											sinfo.AlarmNotification.Schedule[i].enabled = true;
										}
									}
									else
									{
										sinfo.AlarmNotification.Schedule[i].enabled = false;
									}
								}
								else{
									sinfo.AlarmNotification.Schedule[i].enabled = false;
								}
							}
						}
						//sinfo.AlarmNotification.Schedule.enabled = true;
						NETSDK_conf_system_set_setting_info(&sinfo);

                        ST_NSDK_MD_CH md_ch;

                        NETSDK_conf_md_ch_get(1, &md_ch);
                        if(NULL != motinfo)
                        {
                            md_ch.enabled = NETSDK_json_get_boolean(motinfo, "Enabled");
                            NETSDK_json_get_string(motinfo, "SensitivityLevel", level, sizeof(level));
                            mtion_setsensitivityLevel(&md_ch, level);
                            netsdk_md_ch_set(0, &md_ch);
                            if(NETSDK_json_check_child(motinfo, "MotionWarningTone")) {
                                NETSDK_conf_system_get_setting_info(&sinfo);
                                sinfo.mdAlarm.MotionWarningTone = NETSDK_json_get_boolean(motinfo, "MotionWarningTone");
                                NETSDK_conf_system_set_setting_info(&sinfo);
                            }

                            if(NETSDK_json_check_child(motinfo, "WarningToneType"))
                            {
                               NETSDK_conf_system_get_setting_info(&sinfo);
                               NETSDK_json_get_string(motinfo, "WarningToneType", customflag, sizeof(customflag));
                               snprintf(sinfo.mdAlarm.WarningToneType, sizeof(sinfo.mdAlarm.WarningToneType), "%s", customflag);
                               NETSDK_conf_system_set_setting_info(&sinfo);
                            }
							if(NETSDK_json_check_child(motinfo, "MotionRecord")) {
							   NETSDK_conf_system_get_setting_info(&sinfo);
							   sinfo.MotionRecordEnabled = NETSDK_json_get_boolean(motinfo, "MotionRecord");
							   NETSDK_conf_system_set_setting_info(&sinfo);
                            }
                        }
                    }

					//SystemOperation
					SystemOper = NETSDK_json_get_child(ipcam, "SystemOperation");

					if(SystemOper){
						char str_t[15]={0};
						struct timeval timval, cur_tv;
						time_t utc_time = 0;
                        int tz_num = 0;
						LP_JSON_OBJECT TimeZoneJson;
						ST_NSDK_SYSTEM_TIME sys_time;
						//TimeSync
						timeS = NETSDK_json_get_child(SystemOper, "TimeSync");
                        if(NULL != timeS) {

                            // �Ȼ�ȡϵͳ���õ�ʱ����Ϣ; ���app����������Ϣ����ʱ���ֶΣ�������ʱ��
                            if (NULL != NETSDK_conf_system_get_time(&sys_time)) {
                                tz_num = sys_time.greenwichMeanTime;

                                TimeZoneJson = NETSDK_json_get_child(timeS, "TimeZone");
                                if (NULL != TimeZoneJson) {
                                    tz_num = json_object_get_int(TimeZoneJson);

                                    sys_time.greenwichMeanTime = tz_num;

                                    if (NULL != NETSDK_conf_system_set_time(&sys_time)) {
                                        if(netsdk->systemChanged){
                                            APP_TRACE("netsdk->systemChanged %x",netsdk->systemChanged);
                                            sys_time.ntpEnabled = false;  // 远程设置时间同步不启动ntp
                                            netsdk->systemChanged(&sys_time);
                                        }
                                    }
                                }
							}

                            if (NULL != NETSDK_json_get_string(timeS, "UTCTime", str_t, sizeof(str_t))) {
								utc_time = atol(str_t);
                            } else {
                            	str_t[0] = '\0';
                                // ���� LocalTime ��Ϊ�˺���ǰ���ּ���
                               if (NULL != NETSDK_json_get_string(timeS, "LocalTime", str_t, sizeof(str_t))) {
									utc_time = atol(str_t);
                                }
                            }
                            if(utc_time > 0) {
                                gettimeofday(&cur_tv, NULL);
                                if(abs(utc_time - cur_tv.tv_sec) > 10) {
                                    char *defEth = getenv("DEF_ETHER");
                                    ST_NSDK_NETWORK_INTERFACE netInter;
                                    NETSDK_conf_interface_get(4, &netInter);
                                    if((NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT == netInter.wireless.wirelessMode) &&
                                        (0 == strncmp(defEth, "wlan0", strlen("wlan0"))))
                                    {
                                        timval.tv_sec = utc_time;
                                        timval.tv_usec = 0;
                                        settimeofday(&timval,NULL);
                                        RTC_settime(utc_time);
                                    }
                                    else
                                    {
                                        if(NULL != NETSDK_conf_system_get_time(&sys_time))
                                        {
                                            if(sys_time.ntpEnabled)
                                            {
                                                NTP_start(sys_time.ntpServerDomain, sys_time.ntpServerBackupOne, sys_time.ntpServerBackupTwo, NULL, 5, 0);
                                            }
                                        }
                                    }
                                }
                            }
                        }

						//Upgrade
						upginfo = NETSDK_json_get_child(SystemOper, "Upgrade");
#ifdef DANA_P2P
                        /* reboot */
                        /* 需要等待OnRemoteSetup回调成功后才能重启 所以建立一个线程延时操作 */
                        if(NETSDK_json_check_child(SystemOper, "Reboot")) {
                            if(NETSDK_json_get_boolean(SystemOper, "Reboot") == true) {
                                pthread_t reboot_tid = (pthread_t)NULL;
                                pthread_create(&reboot_tid, NULL, p2p_json_reboot, NULL);
                            }
                        }
#endif

						LP_JSON_OBJECT dstJSON = NETSDK_json_get_child(SystemOper, "DaylightSavingTime");
						if(dstJSON != NULL){
							int i, j, n;
							LP_JSON_OBJECT weekJSON = NULL, tmpWeekJSON = NULL;;
							ST_NSDK_SYSTEM_DST dstInfo = {0};
							char tmpType[8];

							NETSDK_conf_system_get_DST_info(&dstInfo);
							dstInfo.enable = NETSDK_json_get_boolean(dstJSON, "Enabled");
                            NETSDK_json_get_string(dstJSON, "Country", dstInfo.country, sizeof(dstInfo.country));
							dstInfo.offset = NETSDK_json_get_int(dstJSON, "Offset");
							if((weekJSON = NETSDK_json_get_child(dstJSON, "Week")) != NULL){
								n = json_object_array_length(weekJSON);
								for(j = 0; j < n; j++){
									if((tmpWeekJSON = json_object_array_get_idx(weekJSON, j)) != NULL){
										memset(tmpType, 0, sizeof(tmpType));
										NETSDK_json_get_string(tmpWeekJSON, "Type", tmpType, sizeof(tmpType));
										for(i = 0; i < sizeof(dstInfo.week) / sizeof(dstInfo.week[0]); i++){
											if(strcmp(tmpType, dstInfo.week[i].type) == 0){
												dstInfo.week[i].month = NETSDK_json_get_int(tmpWeekJSON, "Month");
												dstInfo.week[i].week = NETSDK_json_get_int(tmpWeekJSON, "Week");
												dstInfo.week[i].weekday = NETSDK_json_get_int(tmpWeekJSON, "Weekday");
												dstInfo.week[i].hour = NETSDK_json_get_int(tmpWeekJSON, "Hour");
												dstInfo.week[i].minute = NETSDK_json_get_int(tmpWeekJSON, "Minute");
												break;
											}
										}
									}
								}
							}
							NETSDK_conf_system_set_DST_info(&dstInfo);
							if(netsdk->systemDSTChanged){
								netsdk->systemDSTChanged(&dstInfo);
							}
						}
					}
					if(upginfo)
					{
						NETSDK_json_get_string(upginfo, "Enabled", level, sizeof(level));
						if(NETSDK_json_get_boolean(upginfo, "Enabled"))
						{
							REMOTE_UPGRADE_start();
						}
					}
					//PromptSounds
					ST_NSDK_SYSTEM_SETTING pinfo;
					char ptype[20] = {0};
					Psound = NETSDK_json_get_child(ipcam, "PromptSounds");
					if(Psound)
					{
						NETSDK_conf_system_get_setting_info(&pinfo);
						NETSDK_json_get_string(Psound, "Type", ptype, sizeof(ptype));
                        if(NETSDK_json_check_child(Psound, "Enabled")) {
    						if(NETSDK_json_get_boolean(Psound, "Enabled"))
    						{
    							pinfo.promptSound.enabled = true;
    						}
    						else
    						{
    							pinfo.promptSound.enabled = false;
    						}
                        }
						pinfo.promptSound.soundType = NETSDK_MAP_STR2DEC(promptSoundType_map, ptype, pinfo.promptSound.soundType);
                        ST_CUSTOM_SETTING custom;
                        if(0 == CUSTOM_get(&custom)) {
                            if(custom.function.promptSoundType != pinfo.promptSound.soundType)
                            {
                                custom.function.promptSoundType = pinfo.promptSound.soundType;
                                CUSTOM_set(&custom);
                            }
                        }
						NETSDK_conf_system_set_setting_info(&pinfo);
					}
				#if defined(TFCARD)
					Tfcard = NETSDK_json_get_child(ipcam, "TfcardManager");
					if(Tfcard){
						char formate[10]={0};
						NETSDK_json_get_string(Tfcard, "Operation", formate, sizeof(formate));
						if(0 == strcmp(formate, "format"))
						{
#if defined(TS_RECORD)
                            NK_TFCARD_Format();
                        }
                        else if(0 == strcmp(formate, "repair"))
                        {
                            TFSDK_CARD_Repair();
                        }
#else
						    TFCARD_stop_record();
							NK_TFCARD_format();
						}
#endif
						//TFcard_recordSchedule
						ST_NSDK_SYSTEM_SETTING minfo = {0};
						NETSDK_conf_system_get_setting_info(&minfo);

                        if(NETSDK_json_check_child(Tfcard, "TimeRecordEnabled")) {
                            if(NETSDK_json_get_boolean(Tfcard, "TimeRecordEnabled"))
                            {
                                minfo.timeRecordEnabled = true;
                            }
                            else
                            {
                                minfo.timeRecordEnabled = false;
                            }
                        }

						int i = 0;
						int arrN = 0;
						LP_JSON_OBJECT arr = NETSDK_json_get_child(Tfcard, "TFcard_recordSchedule");

						if(arr){
							//clear setting before
							for(i = 0
									; i < sizeof(minfo.TFcard_Record.Schedule)
										/ sizeof(minfo.TFcard_Record.Schedule[0])
									; i++)
							{
								minfo.TFcard_Record.Schedule[i].enabled = false;
							}

							arrN = json_object_array_length(arr);
							//printf("\n******arrN = %d******\n",arrN);
							for(i = 0; i < arrN
										&& i < sizeof(minfo.TFcard_Record.Schedule)
											/ sizeof(minfo.TFcard_Record.Schedule[0])
									; i++)
							{
								LP_JSON_OBJECT recordScheduleTime = json_object_array_get_idx(arr, i);
								if(recordScheduleTime){
									char beginTime[12] = {0};
									char endTime[12] = {0};
									char weekday[18] = {0};
									int beginSec,endSec;

									NETSDK_json_get_string(recordScheduleTime, "Weekday", weekday, sizeof(weekday));
									NETSDK_json_get_string(recordScheduleTime, "BeginTime", beginTime, sizeof(beginTime));
									NETSDK_json_get_string(recordScheduleTime, "EndTime", endTime, sizeof(endTime));

									if(NULL != beginTime && NULL != endTime && NULL != weekday)
									{
										if(0 != schedule_parse_time(beginTime
												, &minfo.TFcard_Record.Schedule[i].BeginTime.hour
												, &minfo.TFcard_Record.Schedule[i].BeginTime.min
												, &minfo.TFcard_Record.Schedule[i].BeginTime.sec))
										{
											APP_TRACE("Parse Begin Time Failed");
											continue;
										}
										if(0 != schedule_parse_time(endTime
												, &minfo.TFcard_Record.Schedule[i].EndTime.hour
												, &minfo.TFcard_Record.Schedule[i].EndTime.min
												, &minfo.TFcard_Record.Schedule[i].EndTime.sec))
										{
											APP_TRACE("Parse End Time Failed");
											continue;
										}
										if(0 != schedule_parse_weekday(weekday
												, &minfo.TFcard_Record.Schedule[i].weekday))
										{
											APP_TRACE("Parse Weekday Failed");
											continue;
										}

										minfo.TFcard_Record.Schedule[i].weekday &= 0x7f;
										beginSec = minfo.TFcard_Record.Schedule[i].BeginTime.hour * 3600
												+ minfo.TFcard_Record.Schedule[i].BeginTime.min * 60
												+ minfo.TFcard_Record.Schedule[i].BeginTime.sec;
										endSec = minfo.TFcard_Record.Schedule[i].EndTime.hour * 3600
												+ minfo.TFcard_Record.Schedule[i].EndTime.min * 60
												+ minfo.TFcard_Record.Schedule[i].EndTime.sec;

										// verify time
										if(0 != minfo.TFcard_Record.Schedule[i].weekday
												&& beginSec < endSec)
										{
											minfo.TFcard_Record.Schedule[i].enabled = true;
										}
									}
									else{
										minfo.TFcard_Record.Schedule[i].enabled = false;
									}
								}
								else{
									minfo.TFcard_Record.Schedule[i].enabled = false;
								}
							}
						}else{
							printf("\n***not \"TFcard_recordSchedule\" field in setup_jsonp***\n");
						}
						NETSDK_conf_system_set_setting_info(&minfo);
#if defined(TS_RECORD)
						TICKER_add_task(ipcam_timer_record, 5, false);
#endif
					}else{
						printf("\n***not Tfcard_Manager field in setup_jsonp***\n");
					}
				#endif
#if 0
					record =  NETSDK_json_get_child(ipcam, "RecordManager");
					if(record != NULL){
						ST_NSDK_SYSTEM_REC_MANAGER recManager = {0};

						NETSDK_conf_system_get_record_info(&recManager);
						NETSDK_json_get_string(record, "Mode", recManager.recMode, sizeof(recManager.recMode));
						if(NETSDK_json_check_child(record, "UseIOAlarm")){
							recManager.useIOAlarm = NETSDK_json_get_boolean(record, "UseIOAlarm");
						}
						NETSDK_conf_system_set_record_info(&recManager);
					}
#endif
					fisheyeSetinfo = NETSDK_json_get_child(ipcam, "FisheyeSetting");
					if(fisheyeSetinfo){
						ST_NSDK_IMAGE image2 = {0};
						NK_Char fixMode[32] = {0};
						int fixType;
						if(NETSDK_json_get_string(fisheyeSetinfo,"FixMode",fixMode,sizeof(fixMode)) != NULL) {
							if(0 == strcmp(fixMode, "wall")){
								fixType = eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL;
							}else if(0 == strcmp(fixMode, "cell")){
								fixType = eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL;
							}else if(0 == strcmp(fixMode, "table")){
								fixType = eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE;
							}else{
								fixType = eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE;
							}
						}
						if(NETSDK_conf_image_get(&image2)){
							image2.videoMode.fixType = fixType;
							NETSDK_conf_image_set(&image2);
						}

						stFISHEYE_config conf = {0};
						FISHEYE_config_get(&conf);

						int i = 0;
						int arrN = 0;
						LP_JSON_OBJECT fixParam = NETSDK_json_get_child(fisheyeSetinfo, "FixParam");
                        bool type = false;
                        if(NETSDK_json_check_child(fisheyeSetinfo, "LensName")) {
                            NETSDK_json_get_string(fisheyeSetinfo, "LensName", conf.lensName, sizeof(conf.lensName));
                        }
                        else {
                            sprintf(conf.lensName, "m109");
                        }
						conf.fixMode = fixType;
						if(fixParam){
							arrN = json_object_array_length(fixParam);
							for(i = 0; ((i < arrN) && (i < (sizeof(conf.param) / sizeof(conf.param[0]))));i++){
								LP_JSON_OBJECT fixParamData = json_object_array_get_idx(fixParam,i);
								if(fixParamData){
                                    /* ֻ�жϵ�һ�����ݣ������������int�洢������Ǹ�����float�洢 */
                                    if(json_type_int == json_object_get_type(json_object_object_get(fixParamData, "CenterCoordinateX"))) {
                                        conf.param[i].CenterCoordinateX = NETSDK_json_get_int(fixParamData, "CenterCoordinateX");
                                        conf.param[i].CenterCoordinateY = NETSDK_json_get_int(fixParamData, "CenterCoordinateY");
                                        conf.param[i].Radius = NETSDK_json_get_int(fixParamData, "Radius");
                                        conf.param[i].AngleX = NETSDK_json_get_int(fixParamData, "AngleX");
                                        conf.param[i].AngleY = NETSDK_json_get_int(fixParamData, "AngleY");
                                        conf.param[i].AngleZ = NETSDK_json_get_int(fixParamData, "AngleZ");
                                        type = true;
                                    }
                                    else {
                                        conf.param2[i].CenterCoordinateX = NETSDK_json_get_float(fixParamData, "CenterCoordinateX");
                                        conf.param2[i].CenterCoordinateY = NETSDK_json_get_float(fixParamData, "CenterCoordinateY");
                                        conf.param2[i].Radius = NETSDK_json_get_float(fixParamData, "Radius");
                                        conf.param2[i].AngleX = NETSDK_json_get_float(fixParamData, "AngleX");
                                        conf.param2[i].AngleY = NETSDK_json_get_float(fixParamData, "AngleY");
                                        conf.param2[i].AngleZ = NETSDK_json_get_float(fixParamData, "AngleZ");
                                        type = false;
                                    }
								}
							}
							FISHEYE_config_set(&conf, type);
						}
					}

#if defined (LED_PWM)
                    ledPwm = NETSDK_json_get_child(ipcam, "ledPwm");
                    if(ledPwm) {
                        int i = 0, j = 0;
                        stLED_PWM_config setLedPwmConfig;
                        LED_PWM_get(&setLedPwmConfig);  // Ϊ�˱���channelCount��������ʹ���豸�����ֵ
                        setLedPwmConfig.ledSwitch = NETSDK_json_get_int(ledPwm, "switch");
                        LP_JSON_OBJECT child_json = NETSDK_json_get_child(ledPwm, "channelInfo");
                        if(child_json) {
                            for(i = 0; i < setLedPwmConfig.channelCount; i++) {
                                LP_JSON_OBJECT info = json_object_array_get_idx(child_json, i);
                                if(info) {
                                    for(j = 0; j < setLedPwmConfig.channelCount; j++) {
                                        if(setLedPwmConfig.array[j].channel == NETSDK_json_get_int(info, "channel")) {
                                            setLedPwmConfig.array[j].num = NETSDK_json_get_int(info, "num");
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        LED_PWM_set(setLedPwmConfig);
                    }
#endif

#if defined(CLOUD_REC)
                    do {
                        LP_JSON_OBJECT osscloudSetting;
                        LP_JSON_OBJECT isBoundJson;
                        LP_JSON_OBJECT chNumJson;
                        LP_JSON_OBJECT uploadArrJson;
                        LP_JSON_OBJECT arrEleJson;
                        LP_JSON_OBJECT chIdJson;
                        LP_JSON_OBJECT enabledJson;
                        LP_JSON_OBJECT recTypeJson;
                        ST_NSDK_NETWORK_OSSCLD osscld;
                        int i;
                        int arrayLen;
                        int chNum;
                        int channel;
                        int recType;
                        bool errInFor;

                        osscloudSetting = NETSDK_json_get_child(ipcam, "OsscloudSetting");
                        if (NULL == osscloudSetting) {
                            break;
                        }

                        // OsscloudSetting.IsBound
                        isBoundJson = NETSDK_json_get_child(osscloudSetting, "IsBound");
                        if(NULL == isBoundJson){
                            APP_TRACE("%s: Failed to get \"IsBound\" from ossCloud json", __FUNCTION__);
                            break;
                        }

                        // OsscloudSetting.Upload
                        uploadArrJson = NETSDK_json_get_child(osscloudSetting, "Upload");
                        if(NULL == uploadArrJson) {
                            APP_TRACE("%s: Failed to get \"Upload\" from ossCloud json", __FUNCTION__);
                            break;
                        }

                        if (!json_object_is_type(uploadArrJson, json_type_array)) {
                            APP_TRACE("%s: \"Upload\" object is not array", __FUNCTION__);
                            break;
                        }

                        arrayLen = json_object_array_length(uploadArrJson);
                        if (arrayLen < 0) {
                            APP_TRACE("%s: get arrayLen is out of range", __FUNCTION__);
                            break;
                        }

                        if (NULL == NETSDK_conf_osscloud_get(&osscld)) {
                            APP_TRACE("%s: Failed to get OSS Cloud Config", __FUNCTION__);
                            break;
                        }

                        osscld.isBound = json_object_get_boolean(isBoundJson)?true:false;

                        // fixed as main stream
                        errInFor = false;
                        chNum = osscld.chNum;
                        for (i = 0; i < arrayLen; i++) {
                            arrEleJson = json_object_array_get_idx(uploadArrJson, i);
                            if(NULL == arrEleJson) {
                                APP_TRACE("%s: Failed to get object from array", __FUNCTION__);
                                errInFor = true;
                                break;
                            }

                            // OsscloudSetting.Upload[].ChId
                            chIdJson = NETSDK_json_get_child(arrEleJson, "ID");
                            if(NULL == chIdJson) {
                                APP_TRACE("%s: Failed to get \"ID\" from arrEleJson", __FUNCTION__);
                                errInFor = true;
                                break;
                            }

                            // OsscloudSetting.Upload[].Enabled
                            enabledJson = NETSDK_json_get_child(arrEleJson, "Enabled");
                            if(NULL == enabledJson) {
                                APP_TRACE("%s: Failed to get \"Enabled\" from arrEleJson", __FUNCTION__);
                                errInFor = true;
                                break;
                            }

                            // ID Start from 1
                            channel = json_object_get_int(chIdJson);
                            channel -= 1;
                            if (channel < 0 || channel > (chNum - 1)) {
                                APP_TRACE("%s: channel number is out of range, ch: %d, total ch: %d",
                                          __FUNCTION__, channel, chNum);
                                errInFor = true;
                                break;
                            }

                            // OsscloudSetting.Upload[].Type
                            recTypeJson = NETSDK_json_get_child(arrEleJson, "Type");
                            if (NULL != recTypeJson) {
                                recType = json_object_get_int(recTypeJson);
                                if (recType < 0 || recType > 1) {
                                    APP_TRACE("%s: record type number is out of range", __FUNCTION__);
                                    errInFor = true;
                                    break;
                                }

                                osscld.channel[channel].stream[0].type = recType;
                            } else {
                                APP_TRACE("%s: no \"Type\" field in arrEleJson", __FUNCTION__);
                            }

                            osscld.channel[channel].stream[0].enable = json_object_get_boolean(enabledJson)?true:false;
                        }

                        if (errInFor) {
                            break;
                        }

                        if (NULL == NETSDK_conf_osscloud_set(&osscld)) {
                            APP_TRACE("%s: Failed to set OSS Cloud Config", __FUNCTION__);
                            break;
                        }

                        if(true == osscld.channel[0].stream[0].enable)
                        {
                            char sn_str[32] = {0};
                            if(0 == GLOBAL_GetID(sn_str)) {
                                NK_CLOUD_REC_Init(sn_str, strlen(sn_str), 1);
                            }
                        }
                        NK_CLOUD_REC_Update(&osscld);
                    } while(0);
#endif


					LP_JSON_OBJECT Userinfo=NULL, Authinfo=NULL;
					char method[10]={0};
					char p2p_Verify[255]={0};
					char username[25]={0};
					char oldpwd[25]={0};
					char newpasswd[25]={0};
					char recvarify[255]={0};
					Userinfo = NETSDK_json_get_child(obj, "UserManager");
					Authinfo = NETSDK_json_get_child(obj, "Authorization"); 

					if(Userinfo && Authinfo){

						NETSDK_json_get_string(Authinfo, "Verify", recvarify, sizeof(recvarify));
						NETSDK_json_get_string(Userinfo, "Method", method, sizeof(method));
						NETSDK_json_get_string(Userinfo, "Verify", p2p_Verify, sizeof(p2p_Verify));
						NETSDK_json_get_string(Userinfo, "username", username, sizeof(username));

						if(NULL == recvarify || (0 == strlen(recvarify)) )
						{
							NETSDK_json_get_string(Authinfo, "password", oldpwd, sizeof(oldpwd));
						}
						else
						{
							//parse varify
							p2p_parse_userpwd_varify2(recvarify, username, oldpwd);
						}
						if(!p2p_Verify || (0 == strlen(p2p_Verify)))
						{							
							NETSDK_json_get_string(Userinfo, "password", newpasswd, sizeof(newpasswd));
						}
						else
						{							
							p2p_parse_userpwd_varify(p2p_Verify, username, newpasswd);
						}
					}

					if(method && (0 == strcmp(method, "modify")))
					{
						if(!p2p_Auth_userpwd_set(username, oldpwd, newpasswd)){
							json_object_put(obj);
							return 0;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      						}
					}

                    wirelessManager = NETSDK_json_get_child(ipcam, "WirelessManager");
                    if(wirelessManager != NULL) {
#if defined (CX)
                        if(false == GLOBAL_sn_front()) {
                            ST_NSDK_NETWORK_INTERFACE inter;
                            char base64ApPsk[128];
                            NETSDK_conf_interface_get(4, &inter);
                            if(inter.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT) {
                                memset(base64ApPsk, 0, sizeof(base64ApPsk));
                                if(NULL != NETSDK_json_get_string(wirelessManager, "ApPsk", base64ApPsk, sizeof(base64ApPsk))) {
                                    BASE64_decode(base64ApPsk, strlen(base64ApPsk),
                                    inter.wireless.wirelessApMode.wirelessPsk, sizeof(inter.wireless.wirelessApMode.wirelessPsk));
                                    NETSDK_conf_interface_set_by_delay(4, &inter, eNSDK_CONF_SAVE_RESTART, 5);
                                }
                            }
                        }
#endif
                    }

                    wirelessStation = NETSDK_json_get_child(ipcam, "WirelessStation");
                    if(NULL != wirelessStation)
                    {
                        p2p_parse_wireleseStationSet(wirelessStation);
                    }

                    /*
                    * videoManager
                    */
                    videoManager = NETSDK_json_get_child(ipcam, "videoManager");
                    if(NULL != videoManager)
                    {
                        p2p_parse_videoManagerSet(videoManager);
                    }

                    /*
                     * ptz
                     */
                    ptz = NETSDK_json_get_child(ipcam, "ptzManager");
                    if(NULL != ptz)
                    {
                        p2p_parse_ptzSet(ptz);
                    }

					/*
					 * devCoverSetting
					 */
					devCoverSetting = NETSDK_json_get_child(ipcam, "devCoverSetting");
					if(NULL != devCoverSetting)
					{
						p2p_parse_devCoverSet(devCoverSetting);
					}

					json_object_put(obj);
                    obj = NULL;
					sprintf((char *)response, "{\r\n\"option\" :\"success\"\r\n}\r\n");
					return strlen((char *)response);
			}
			
		}
	}
	return 0;
}

