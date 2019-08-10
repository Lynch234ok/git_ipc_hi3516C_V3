
#include "netsdk.h"
#include <stdarg.h>
#include "netsdk_private.h"
#include "socket_tcp.h"
#include "generic.h"
#include "app_debug.h"
#include "usrm.h"
#include "netsdk.h"

static int gs_multi_conf_mode = -1;

const ST_NSDK_MAP_STR_DEC convenient_mode_map[] = {
	{sNSDK_MULTI_CONF_MODE_INSIDE, kNSDK_MULTI_CONF_MODE_INSIDE},
	{sNSDK_MULTI_CONF_MODE_OUTSIDE, kNSDK_MULTI_CONF_MODE_OUTSIDE},
	{sNSDK_MULTI_CONF_MODE_CUSTOM, kNSDK_MULTI_CONF_MODE_CUSTOM},
};

static LP_JSON_OBJECT netsdk_find_list(LP_JSON_OBJECT lists, int id)
{
	int i = 0;
	int const n_lists = json_object_array_length(lists);
	// one channel
	for(i = 0; i < n_lists; ++i){
		LP_JSON_OBJECT list = json_object_array_get_idx(lists, i);
		if(json_object_get_int(json_object_object_get(list, "id")) == id){
			return list;
		}
	}
	return NULL;
}


static void netsdk_recover_json(LP_JSON_OBJECT dst_jso, LP_JSON_OBJECT src_jso)
{
    //video motionDetection
	LP_JSON_OBJECT srcMotionDetectionJSON = NETSDK_json_get_child(src_jso, "motionDetection");
	LP_JSON_OBJECT dstMotionDetectionJSON = NETSDK_json_get_child(dst_jso, "motionDetection");
    if(NULL != srcMotionDetectionJSON && NULL != dstMotionDetectionJSON) {
        LP_JSON_OBJECT srcMdChannelsJSON = NETSDK_json_get_child(srcMotionDetectionJSON, "motionDetectionChannel");
        LP_JSON_OBJECT dstMdChannelsJSON = NETSDK_json_get_child(dstMotionDetectionJSON, "motionDetectionChannel");

        if(NULL != srcMdChannelsJSON && NULL != dstMdChannelsJSON) {
            LP_JSON_OBJECT srcMdchannelJSON = netsdk_find_list(srcMdChannelsJSON, 1);
            LP_JSON_OBJECT dstMdchannelJSON = netsdk_find_list(dstMdChannelsJSON, 1);
            if(NULL != srcMdchannelJSON && NULL != dstMdchannelJSON) {
                NETSDK_json_copy_child(srcMdchannelJSON, dstMdchannelJSON, "enabled");

                LP_JSON_OBJECT srcDetectionGridJSON = NETSDK_json_get_child(srcMdchannelJSON, "detectionGrid");
                LP_JSON_OBJECT dstDetectionGridJSON = NETSDK_json_get_child(dstMdchannelJSON, "detectionGrid");
                if(NULL != srcDetectionGridJSON && NULL != dstDetectionGridJSON) {
                    NETSDK_json_copy_child(srcDetectionGridJSON, dstDetectionGridJSON, "sensitivityLevel");
                }
            }
        }
    }

	//video reselution
	LP_JSON_OBJECT srcEncodeJSON = NETSDK_json_get_child(src_jso, "videoEncode");
	LP_JSON_OBJECT dstEncodeJSON = NETSDK_json_get_child(dst_jso, "videoEncode");

	if(NULL != srcEncodeJSON && NULL != dstEncodeJSON){
		LP_JSON_OBJECT srcChannelsJSON = NETSDK_json_get_child(srcEncodeJSON, "videoEncodeChannel");
		LP_JSON_OBJECT dstChannelsJSON = NETSDK_json_get_child(dstEncodeJSON, "videoEncodeChannel");

		if(NULL != srcChannelsJSON && NULL != dstChannelsJSON){
			//only for main stream
			LP_JSON_OBJECT srcchannelJSON = netsdk_find_list(srcChannelsJSON, 101);
			LP_JSON_OBJECT dstchannelJSON = netsdk_find_list(dstChannelsJSON, 101);

			if(NULL != srcchannelJSON && NULL != dstchannelJSON){
				NETSDK_json_copy_child(srcchannelJSON, dstchannelJSON, "resolution");
				NETSDK_json_copy_child(srcchannelJSON, dstchannelJSON, "freeResolution");
				NETSDK_json_copy_child(srcchannelJSON, dstchannelJSON, "resolutionWidth");
				NETSDK_json_copy_child(srcchannelJSON, dstchannelJSON, "resolutionHeight");
				NETSDK_json_copy_child(srcchannelJSON, dstchannelJSON, "ImageTransmissionModel");
				NETSDK_json_copy_child(srcchannelJSON, dstchannelJSON, "keyFrameInterval");
			}

			//for sub stream
			LP_JSON_OBJECT srcchannelJSON_sub = netsdk_find_list(srcChannelsJSON, 102);
			LP_JSON_OBJECT dstchannelJSON_sub = netsdk_find_list(dstChannelsJSON, 102);

			if(NULL != srcchannelJSON_sub && NULL != dstchannelJSON_sub){
				NETSDK_json_copy_child(srcchannelJSON_sub, dstchannelJSON_sub, "resolution");
				NETSDK_json_copy_child(srcchannelJSON_sub, dstchannelJSON_sub, "freeResolution");
				NETSDK_json_copy_child(srcchannelJSON_sub, dstchannelJSON_sub, "resolutionWidth");
				NETSDK_json_copy_child(srcchannelJSON_sub, dstchannelJSON_sub, "resolutionHeight");
				NETSDK_json_copy_child(srcchannelJSON_sub, dstchannelJSON_sub, "ImageTransmissionModel");
				NETSDK_json_copy_child(srcchannelJSON_sub, dstchannelJSON_sub, "keyFrameInterval");
			}
		}
	}
	
	//network interface
	LP_JSON_OBJECT srcInterfacesListJSON = NETSDK_json_get_child(src_jso, "networkInterface");
	LP_JSON_OBJECT dstInterfacesListJSON = NETSDK_json_get_child(dst_jso, "networkInterface");

	if(NULL != srcInterfacesListJSON && NULL != dstInterfacesListJSON){
		LP_JSON_OBJECT srcInterfacesJSON = NETSDK_json_get_child(srcInterfacesListJSON, "networkInterfaceList");
		LP_JSON_OBJECT dstInterfacesJSON = NETSDK_json_get_child(dstInterfacesListJSON, "networkInterfaceList");

		if(NULL != srcInterfacesJSON && NULL != dstInterfacesJSON){
			//only for lan
			LP_JSON_OBJECT srcInterfaceJSON = netsdk_find_list(srcInterfacesJSON, 1);
			LP_JSON_OBJECT dstInterfaceJSON = netsdk_find_list(dstInterfacesJSON, 1);

			if(NULL != srcInterfaceJSON && NULL != dstInterfaceJSON){
				LP_JSON_OBJECT srcLanJSON = NETSDK_json_get_child(srcInterfaceJSON, "lan");
				LP_JSON_OBJECT dstLanJSON = NETSDK_json_get_child(dstInterfaceJSON, "lan");

				if(NULL != srcLanJSON && NULL != dstLanJSON){
					NETSDK_json_copy_child(srcLanJSON, dstLanJSON, "addressingType");
					NETSDK_json_copy_child(srcLanJSON, dstLanJSON, "staticIP");
					NETSDK_json_copy_child(srcLanJSON, dstLanJSON, "staticNetmask");
					NETSDK_json_copy_child(srcLanJSON, dstLanJSON, "staticGateway");
				}
			}
			//only for wifi interface
			LP_JSON_OBJECT srcWifiInterfaceJSON = netsdk_find_list(srcInterfacesJSON, 4);
			LP_JSON_OBJECT dstWifiInterfaceJSON = netsdk_find_list(dstInterfacesJSON, 4);

			if(NULL != srcWifiInterfaceJSON && NULL != dstWifiInterfaceJSON){
				LP_JSON_OBJECT srcLanJSON = NETSDK_json_get_child(srcWifiInterfaceJSON, "lan");
				LP_JSON_OBJECT dstLanJSON = NETSDK_json_get_child(dstWifiInterfaceJSON, "lan");

				if(NULL != srcLanJSON && NULL != dstLanJSON){
					//NETSDK_json_copy_child(srcLanJSON, dstLanJSON, "addressingType");
					NETSDK_json_copy_child(srcLanJSON, dstLanJSON, "staticIP");
					NETSDK_json_copy_child(srcLanJSON, dstLanJSON, "staticNetmask");
					NETSDK_json_copy_child(srcLanJSON, dstLanJSON, "staticGateway");
				}
				
				LP_JSON_OBJECT srcWirelessJSON = NETSDK_json_get_child(srcWifiInterfaceJSON, "wireless");
				LP_JSON_OBJECT dstWirelessJSON = NETSDK_json_get_child(dstWifiInterfaceJSON, "wireless");

				if(NULL != srcWirelessJSON && NULL != dstWirelessJSON){
					NETSDK_json_copy_child(srcWirelessJSON, dstWirelessJSON, "wirelessMode");
					{
					LP_JSON_OBJECT srcStationModeJSON = NETSDK_json_get_child(srcWirelessJSON, "stationMode");
					LP_JSON_OBJECT dstStationModeJSON = NETSDK_json_get_child(dstWirelessJSON, "stationMode");

					if(NULL != srcStationModeJSON && NULL != dstStationModeJSON) {
						if(NULL == NETSDK_json_get_child(srcStationModeJSON, "SettingB64En")) {
							char STA_Base64_Buffer[256] = {0};
							char STA_Encode_Buffer[256] = {0};

							NETSDK_json_get_string(srcStationModeJSON, "wirelessApEssId",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(srcStationModeJSON, "wirelessApEssId", STA_Encode_Buffer);
							printf("\nSettingB64En STA wirelessApEssId: %s\n", STA_Encode_Buffer);

							NETSDK_json_get_string(srcStationModeJSON, "wirelessApPsk",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(srcStationModeJSON, "wirelessApPsk", STA_Encode_Buffer);
							printf("\nSettingB64En STA wirelessApPsk: %s\n", STA_Encode_Buffer);
						}

						NETSDK_json_copy_child(srcStationModeJSON, dstStationModeJSON, "wirelessApEssId");
						NETSDK_json_copy_child(srcStationModeJSON, dstStationModeJSON, "wirelessApPsk");
					}
					}

					{
					LP_JSON_OBJECT srcStationModeBackupJSON = NETSDK_json_get_child(srcWirelessJSON, "stationModeBackup");
					LP_JSON_OBJECT dstStationModeBackupJSON = NETSDK_json_get_child(dstWirelessJSON, "stationModeBackup");

					if(NULL != srcStationModeBackupJSON && NULL != dstStationModeBackupJSON) {
						if(NULL == NETSDK_json_get_child(srcStationModeBackupJSON, "SettingB64En")) {
							char STA_Base64_Buffer[256] = {0};
							char STA_Encode_Buffer[256] = {0};

							NETSDK_json_get_string(srcStationModeBackupJSON, "wirelessApEssId",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(srcStationModeBackupJSON, "wirelessApEssId", STA_Encode_Buffer);
							printf("\nSettingB64En STA Backup wirelessApEssId: %s\n", STA_Encode_Buffer);

							NETSDK_json_get_string(srcStationModeBackupJSON, "wirelessApPsk",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(srcStationModeBackupJSON, "wirelessApPsk", STA_Encode_Buffer);
							printf("\nSettingB64En STA Backup wirelessApPsk: %s\n", STA_Encode_Buffer);
						}

						NETSDK_json_copy_child(srcStationModeBackupJSON, dstStationModeBackupJSON, "wirelessApEssId");
						NETSDK_json_copy_child(srcStationModeBackupJSON, dstStationModeBackupJSON, "wirelessApPsk");
					}
					}

					{
					LP_JSON_OBJECT srcAPModeJSON = NETSDK_json_get_child(srcWirelessJSON, "accessPointMode");
					LP_JSON_OBJECT dstAPModeJSON = NETSDK_json_get_child(dstWirelessJSON, "accessPointMode");

					if(NULL != srcAPModeJSON && NULL != dstAPModeJSON) {
						if(NULL == NETSDK_json_get_child(srcAPModeJSON, "SettingB64En")) {
							char AP_Base64_Buffer[256] = {0};
							char AP_Encode_Buffer[256] = {0};

							NETSDK_json_get_string(srcAPModeJSON, "wirelessEssId",
								AP_Base64_Buffer, sizeof(AP_Base64_Buffer));
							base64_encode(AP_Base64_Buffer, AP_Encode_Buffer, strlen(AP_Base64_Buffer));
							NETSDK_json_set_string2(srcAPModeJSON, "wirelessEssId", AP_Encode_Buffer);
							printf("\nSettingB64En AP wirelessEssId: %s\n", AP_Encode_Buffer);

							NETSDK_json_get_string(srcAPModeJSON, "wirelessPsk",
								AP_Base64_Buffer, sizeof(AP_Base64_Buffer));
							base64_encode(AP_Base64_Buffer, AP_Encode_Buffer, strlen(AP_Base64_Buffer));
							NETSDK_json_set_string2(srcAPModeJSON, "wirelessPsk", AP_Encode_Buffer);
							printf("\nSettingB64En AP wirelessPsk: %s\n", AP_Encode_Buffer);
						}

						NETSDK_json_copy_child(srcAPModeJSON, dstAPModeJSON, "wirelessEssId");
						NETSDK_json_copy_child(srcAPModeJSON, dstAPModeJSON, "wirelessPsk");
					}
					}

					{
						//wireless dhcp client
						LP_JSON_OBJECT srcdhcpJSON = NETSDK_json_get_child(srcWirelessJSON, "dhcpServer");
						LP_JSON_OBJECT dstdhcpJSON = NETSDK_json_get_child(dstWirelessJSON, "dhcpServer");

						if(NULL != srcdhcpJSON && NULL != dstdhcpJSON){
							NETSDK_json_copy_child(srcdhcpJSON, dstdhcpJSON, "dhcpAutoSettingEnabled");
						}
					}
				}
			}
		}
	}

    // irCutControlMode
    LP_JSON_OBJECT srcImageJSON = NETSDK_json_get_child(src_jso, "image");
    LP_JSON_OBJECT dstImageJSON = NETSDK_json_get_child(dst_jso, "image");
    if(NULL != srcImageJSON && NULL != dstImageJSON) {
        LP_JSON_OBJECT srcIrCutFilterJSON = NETSDK_json_get_child(srcImageJSON, "irCutFilter");
        LP_JSON_OBJECT dstIrCutFilterJSON = NETSDK_json_get_child(dstImageJSON, "irCutFilter");
        if(NULL != srcIrCutFilterJSON && NULL != dstIrCutFilterJSON) {
            NETSDK_json_copy_child(srcIrCutFilterJSON, dstIrCutFilterJSON, "irCutControlMode");
        }

        LP_JSON_OBJECT srcVideoModeJSON = NETSDK_json_get_child(srcImageJSON, "videoMode");
        LP_JSON_OBJECT dstVideoModeJSON = NETSDK_json_get_child(dstImageJSON, "videoMode");
        if(NULL != srcVideoModeJSON && NULL != dstVideoModeJSON) {
            NETSDK_json_copy_child(srcVideoModeJSON, dstVideoModeJSON, "fixMode");
            NETSDK_json_copy_child(srcVideoModeJSON, dstVideoModeJSON, "wallMode");
            NETSDK_json_copy_child(srcVideoModeJSON, dstVideoModeJSON, "cellMode");
            NETSDK_json_copy_child(srcVideoModeJSON, dstVideoModeJSON, "tableMode");
        }
    }

    // system time
    LP_JSON_OBJECT srcTimeJSON = NETSDK_json_get_child(src_jso, "time");
    LP_JSON_OBJECT dstTimeJSON = NETSDK_json_get_child(dst_jso, "time");
    if(NULL != srcTimeJSON && NULL != dstTimeJSON) {
        NETSDK_json_copy_child(srcTimeJSON, dstTimeJSON, "timeZone");
		
	    LP_JSON_OBJECT srcDST = NETSDK_json_get_child(srcTimeJSON, "DaylightSavingTime");
		if(NULL != srcDST){
			json_object_object_add(dstTimeJSON, "DaylightSavingTime", srcDST);
		}
    }

    // system PromptSounds
    LP_JSON_OBJECT srcPromptSoundsJSON = NETSDK_json_get_child(src_jso, "PromptSounds");
    LP_JSON_OBJECT dstPromptSoundsJSON = NETSDK_json_get_child(dst_jso, "PromptSounds");
    if(NULL != srcPromptSoundsJSON && NULL != dstPromptSoundsJSON) {
        NETSDK_json_copy_child(srcPromptSoundsJSON, dstPromptSoundsJSON, "Enabled");
        NETSDK_json_copy_child(srcPromptSoundsJSON, dstPromptSoundsJSON, "Type");
    }

    // system MessagePush
    if((NETSDK_json_check_child(src_jso, "MessagePushEnabled") == true) 
        && (NETSDK_json_check_child(dst_jso, "MessagePushEnabled") == true)) {
        NETSDK_json_copy_child(src_jso, dst_jso, "MessagePushEnabled");
    }
	
    // system Record Manager
    LP_JSON_OBJECT srcRecJSON = NETSDK_json_get_child(src_jso, "RecordManager");
    LP_JSON_OBJECT dstRecJSON = NETSDK_json_get_child(dst_jso, "RecordManager");
    if(NULL != srcRecJSON && NULL != dstRecJSON) {
        NETSDK_json_copy_child(srcRecJSON, dstRecJSON, "Mode");
        //NETSDK_json_copy_child(srcRecJSON, dstRecJSON, "UseIOAlarm");
    }

    char text[32] = {0};
    int i = 0;
    int len = 0;
    LP_JSON_OBJECT srcMessagePushJSON = NETSDK_json_get_child(src_jso, "MessagePushSchedule");
    LP_JSON_OBJECT dstMessagePushJSON = NETSDK_json_get_child(dst_jso, "MessagePushSchedule");
    if((srcMessagePushJSON != NULL) && (dstMessagePushJSON != NULL)) {
        len = json_object_array_length(srcMessagePushJSON); // 获取原设置的数组长度
        for(i = 0; i < len; i++) {
            LP_JSON_OBJECT srcIndex = json_object_array_get_idx(srcMessagePushJSON, i);
            LP_JSON_OBJECT dstIndex = json_object_array_get_idx(dstMessagePushJSON, i);
            if(srcIndex) {
                if(dstIndex) {
                    NETSDK_json_copy_child(srcIndex, dstIndex, "Weekday");
                    NETSDK_json_copy_child(srcIndex, dstIndex, "BeginTime");
                    NETSDK_json_copy_child(srcIndex, dstIndex, "EndTime");
                }
                else {
                    dstIndex = json_object_new_object();
                    if(NETSDK_json_get_string(srcIndex, "Weekday", text, sizeof(text))) {
                        NETSDK_json_set_string2(dstIndex, "Weekday", text);
                    }
                    if(NETSDK_json_get_string(srcIndex, "BeginTime", text, sizeof(text))) {
                        NETSDK_json_set_string2(dstIndex, "BeginTime", text);
                    }
                    if(NETSDK_json_get_string(srcIndex, "EndTime", text, sizeof(text))) {
                        NETSDK_json_set_string2(dstIndex, "EndTime", text);
                    }
                    json_object_array_put_idx(dstMessagePushJSON, i, dstIndex);
                }
            }
        }
    }

    // system TFcard_recordSchedule
    //time record
    if((NETSDK_json_check_child(src_jso, "TimeRecordEnabled") == true) 
        && (NETSDK_json_check_child(dst_jso, "TimeRecordEnabled") == true)) {
        NETSDK_json_copy_child(src_jso, dst_jso, "TimeRecordEnabled");
    }

    LP_JSON_OBJECT srcTFcardRecordJSON = NETSDK_json_get_child(src_jso, "TFcard_recordSchedule");
    LP_JSON_OBJECT dstTFcardRecordJSON = NETSDK_json_get_child(dst_jso, "TFcard_recordSchedule");
    if((NULL != srcTFcardRecordJSON) && (NULL != dstTFcardRecordJSON)) {
        len = json_object_array_length(srcTFcardRecordJSON); // 获取原设置的数组长度
        for(i = 0; i < len; i++) {
            LP_JSON_OBJECT srcIndex = json_object_array_get_idx(srcTFcardRecordJSON, i);
            LP_JSON_OBJECT dstIndex = json_object_array_get_idx(dstTFcardRecordJSON, i);
            if(srcIndex) {
                if(dstIndex) {
                    NETSDK_json_copy_child(srcIndex, dstIndex, "Weekday");
                    NETSDK_json_copy_child(srcIndex, dstIndex, "BeginTime");
                    NETSDK_json_copy_child(srcIndex, dstIndex, "EndTime");
                }
                else {
                    dstIndex = json_object_new_object();
                    if(dstIndex) {
                        if(NETSDK_json_get_string(srcIndex, "Weekday", text, sizeof(text))) {
                            NETSDK_json_set_string2(dstIndex, "Weekday", text);
                        }
                        if(NETSDK_json_get_string(srcIndex, "BeginTime", text, sizeof(text))) {
                            NETSDK_json_set_string2(dstIndex, "BeginTime", text);
                        }
                        if(NETSDK_json_get_string(srcIndex, "EndTime", text, sizeof(text))) {
                            NETSDK_json_set_string2(dstIndex, "EndTime", text);
                        }
                        json_object_array_put_idx(dstTFcardRecordJSON, i, dstIndex);
                    }
                }
            }
        }
    }

    // system area
    LP_JSON_OBJECT srcAreaJSON = NETSDK_json_get_child(src_jso, "area");
    LP_JSON_OBJECT dstAreaJSON = NETSDK_json_get_child(dst_jso, "area");
    if((NETSDK_json_check_child(src_jso, "area") == true)
        && (NETSDK_json_check_child(dst_jso, "area") == true)) {
        NETSDK_json_copy_child(src_jso, dst_jso, "area");
    }
    // system mdAlarm
    LP_JSON_OBJECT srcMdAlarmJSON = NETSDK_json_get_child(src_jso, "MdAlarm");
    LP_JSON_OBJECT dstMdAlarmJSON = NETSDK_json_get_child(dst_jso, "MdAlarm");
    if((NULL != srcMdAlarmJSON) && (NULL != dstMdAlarmJSON)) {
        NETSDK_json_copy_child(srcMdAlarmJSON, dstMdAlarmJSON, "MotionWarningTone");
    }

	
    //Oss Cloud Setting
    LP_JSON_OBJECT srcOssCloudJSON = NETSDK_json_get_child(src_jso, "ossCloud");
    LP_JSON_OBJECT dstOssCloudJSON = NETSDK_json_get_child(dst_jso, "ossCloud");
    if(NULL != srcOssCloudJSON && NULL != dstOssCloudJSON){
		LP_JSON_OBJECT srcUploadJSON = NETSDK_json_get_child(srcOssCloudJSON, "upload");
		LP_JSON_OBJECT dstUploadJSON = NETSDK_json_get_child(dstOssCloudJSON, "upload");
		if(NULL != srcUploadJSON && NULL != dstUploadJSON){
			int i;
			int srcChannels = json_object_array_length(srcUploadJSON);
			int dstChannels = json_object_array_length(dstUploadJSON);
			LP_JSON_OBJECT srcChannel, dstChannel;
			APP_TRACE("srcChannels = %d dstChannels = %d", srcChannels, dstChannels);
			
			for(i = 0; i < srcChannels && i < dstChannels; ++i){
				srcChannel = json_object_array_get_idx(srcUploadJSON, i);
				dstChannel = json_object_array_get_idx(dstUploadJSON, i);
				if(NULL != srcChannel && NULL != dstChannel){
					NETSDK_json_copy_child(srcChannel, dstChannel, "enabled");
					NETSDK_json_copy_child(srcChannel, dstChannel, "type");
				}
			}
		}
		NETSDK_json_copy_child(srcOssCloudJSON, dstOssCloudJSON, "isBound");
    }
}

static LP_JSON_OBJECT netsdk_load_json(const char *fileName)
{
	char confPath[128] = {""}, defConfPath[128] = {""};
	char version[64] = {""}, defVersion[64] = {""};
	LP_JSON_OBJECT confJSON = NULL;
	LP_JSON_OBJECT defConfJSON = NULL;
	FILE *fd = NULL;
	char mode_str[32];


	snprintf(defConfPath, sizeof(defConfPath), "%s/%s.json", netsdk->defConfDirectory, fileName);

	if(IS_FILE_EXIST(defConfPath)){
		//only one conf
		snprintf(confPath, sizeof(confPath), "%s/%s.json", netsdk->confDirectory, fileName);
	}else{
		//use multi conf
		fd = fopen(sNSDK_MULTI_CONF_MODE_FILE_PATH, "r+");
		if(fd){
			if(-1 == gs_multi_conf_mode){
				memset(mode_str, 0, sizeof(mode_str));
				fread(mode_str, 1, sizeof(mode_str), fd);
				//printf("gs_multi_conf_mode-%s = %s\n", fileName, mode_str);
				gs_multi_conf_mode = NETSDK_MAP_STR2DEC(convenient_mode_map, mode_str, kNSDK_MULTI_CONF_MODE_DEFAULT);
			}
			snprintf(confPath, sizeof(confPath), "%s/%s_%d.json", netsdk->confDirectory, fileName, gs_multi_conf_mode);
			snprintf(defConfPath, sizeof(defConfPath), "%s/%s_%d.json", netsdk->defConfDirectory, fileName, gs_multi_conf_mode);			
			fclose(fd);	
		}else{
			//file do not exist,use inside mode (video_0.json)as default
			snprintf(confPath, sizeof(confPath), "%s/%s_%d.json", netsdk->confDirectory, fileName, kNSDK_MULTI_CONF_MODE_DEFAULT);
			snprintf(defConfPath, sizeof(defConfPath), "%s/%s_%d.json", netsdk->defConfDirectory, fileName, kNSDK_MULTI_CONF_MODE_DEFAULT);
			fd = fopen(sNSDK_MULTI_CONF_MODE_FILE_PATH, "wb+");
			if(fd){
				fwrite(sNSDK_MULTI_CONF_MODE_DEFAULT, sizeof(sNSDK_MULTI_CONF_MODE_DEFAULT), 1 , fd);
				fclose(fd);
			}
		}
	}	

	confJSON = NETSDK_json_load(confPath);
	if(NULL != confJSON){
		defConfJSON  = NETSDK_json_load(defConfPath);
		if(NULL != defConfJSON){
			// try to match the version between configuration and default configuration
			bool reload = true;
			
			if(NETSDK_json_get_string(confJSON, "version", version, sizeof(version))
				&& NETSDK_json_get_string(defConfJSON, "version", defVersion, sizeof(defVersion))){
				if(strlen(version) == strlen(defVersion)
					&& 0 == strcmp(version, defVersion)){
					// match version, needless to reload
					reload = false;
				}else{
					APP_TRACE("JSON Version \"%s/%s\" Mismatch", version, defVersion);
				}
			}

			// check reload
			if(reload){
				netsdk_recover_json(defConfJSON, confJSON);
				
				// save default configuration file to configuration file
				NETSDK_json_save(defConfJSON, confPath);

				// put configuration firstly
				json_object_put(confJSON);
				confJSON = NULL;
				// put the default JSON reference
				json_object_put(defConfJSON);
				defConfJSON = NULL;
				
				// reload this json again
				return netsdk_load_json(fileName);
			}

			// put the default JSON reference
			json_object_put(defConfJSON);
			defConfJSON = NULL;
		}else{
			APP_TRACE("JSON \"%s\" Not Found!", defConfPath);
		}
		return confJSON;
	}else{
		APP_TRACE("JSON \"%s\" Create!", confPath);
		COPY_FILE(defConfPath, confPath);
		return netsdk_load_json(fileName);
	}
	return NULL;
}

int NETSDK_conf_save(LP_JSON_OBJECT json, const char *fileName)
{
	char confPath[128] = {""}, defConfPath[128] = {""};
	snprintf(defConfPath, sizeof(defConfPath), "%s/%s.json", netsdk->defConfDirectory, fileName);

	if(IS_FILE_EXIST(defConfPath)){
		//only one conf
		printf("save only one conf:%s\n", fileName);
		snprintf(confPath, sizeof(confPath), "%s/%s.json", netsdk->confDirectory, fileName);
	}else{
		if(-1 == gs_multi_conf_mode){
			snprintf(confPath, sizeof(confPath), "%s/%s_0.json", netsdk->confDirectory, fileName);
		}else{
			snprintf(confPath, sizeof(confPath), "%s/%s_%d.json", netsdk->confDirectory, fileName, gs_multi_conf_mode);
		}
		printf("multi conf mode :%d", gs_multi_conf_mode);
	}
	return NETSDK_json_save(json, confPath);
}

int NETSDK_init(LP_NSDK_INIT init)
{
	if(!netsdk){
		
		// get the reference of netsdk handle
		NETSDK_private_get();

		// init the configuration folder path
		snprintf(netsdk->confDirectory, sizeof(netsdk->confDirectory), "%s", init->confDirectory);
		snprintf(netsdk->defConfDirectory, sizeof(netsdk->defConfDirectory), "%s", init->defConfDirectory);

		// load configuration context
		NETSDK_conf_load(false);
		
		// init run time funtion
		netsdk->systemDeviceInfo = init->systemDeviceInfo;
		netsdk->systemOperation = init->systemOperation;
		netsdk->videoInputChannelChanged = init->videoInputChannelChanged;
		netsdk->audioInputChannelChanged = init->audioInputChannelChanged;
		netsdk->motionDetectionChannelChanged = init->motionDetectionChannelChanged;
		netsdk->motionDetectionChannelStatus = init->motionDetectionChannelStatus;
		netsdk->videoEncodeChannelChanged = init->videoEncodeChannelChanged;
		netsdk->audioEncodeChannelChanged = init->audioEncodeChannelChanged;
		netsdk->videoEncodeRequestKeyFrame = init->videoEncodeRequestKeyFrame;
		netsdk->videoEncodeSnapShot = init->videoEncodeSnapShot;
		netsdk->systemChanged = init->systemChanged;
		netsdk->systemDSTChanged = init->systemDSTChanged;
		netsdk->networkWirelessStatus = init->networkWirelessStatus;
		netsdk->imageChanged = init->imageChanged;
		netsdk->AutoFocusStatus = init->AutoFocusStatus;
		netsdk->remoteUpgradeStatus = init->remoteUpgradeStatus;
		netsdk->remoteUpgradeError = init->remoteUpgradeError;

		netsdk->alarmInputChannelPortStatus = init->alarmInputChannelPortStatus;
		netsdk->alarmOutputChannelPortStatus = init->alarmOutputChannelPortStatus;
		netsdk->alarmOutputChannelTrigger = init->alarmOutputChannelTrigger;
		netsdk->ptzUartConfigChanged = init->ptzUartConfigChanged;

		// save configuration callback
		netsdk->audio_conf_save = init->audio_conf_save;
		netsdk->video_conf_save = init->video_conf_save;
		netsdk->system_conf_save = init->system_conf_save;
		netsdk->network_conf_save = init->network_conf_save;
		netsdk->io_conf_save = init->io_conf_save;
		netsdk->image_conf_save = init->image_conf_save;
		
		return 0;
	}
	return -1;
}

void NETSDK_destroy()
{
	if(netsdk){
		// free json context
		json_object_put(netsdk->stream_conf);
		netsdk->stream_conf = NULL;
		json_object_put(netsdk->ptz_conf);
		netsdk->ptz_conf = NULL;
		json_object_put(netsdk->io_conf);
		netsdk->io_conf = NULL;
		json_object_put(netsdk->video_conf);
		netsdk->video_conf = NULL;
		json_object_put(netsdk->audio_conf);
		netsdk->audio_conf = NULL;
		json_object_put(netsdk->jsonSystem);
		netsdk->jsonSystem = NULL;
		// release the reference of netsdk handle
		NETSDK_private_put();
	}
}

// end with NULL
static const char *netsdk_uri(char *uriBuf, ...)
{
	va_list vargList;
	const char *subURI = NULL;

	// add the netsdk prefix
	strcpy(uriBuf, "/NetSDK");
	va_start(vargList, uriBuf);
	while(NULL != (subURI = va_arg(vargList, const char *))){
		if('/' != subURI[0]){
			strcat(uriBuf, "/");
		}
		strcat(uriBuf, subURI);
	}
	va_end(vargList);

	APP_TRACE("NetSDK URI: %s", uriBuf);
	return uriBuf;
}

void NETSDK_http_destroy()
{
}

int NETSDK_http_service(LP_HTTP_CONTEXT context)
{
	int ret = 0;
	char content[16 * 1024] = {""};
	int const contentMax = sizeof(content);
	const char *prefix = "/NetSDK";
	const char *subURI = strdupa(context->request_header->uri);
	const char *authTag = NULL;
	bool authorized = false;
	subURI += strlen(prefix); // offset to remove the NetSDK prefix
	STR_TO_UPPER(subURI); // convert all into captions letters

	// check authorization
	//authorized = false;
	authTag = context->request_header->read_tag(context->request_header, "Authorization");
	if(!authTag){
		APP_TRACE("No Authorization!!");
		// do something
	}else{
		prefix = "Basic ";
		if(0 == strncasecmp(prefix, authTag, strlen(prefix))){
			// available base64 prefix detected
			HTTP_CSTR_t authBase64 = strdupa(authTag + strlen(prefix));
			int const authBase64Len = strlen(authBase64);
			char *authPlaint = alloca(authBase64Len);
			char *username = NULL, *password = NULL, *token = NULL;

			memset(authPlaint, 0, authBase64Len); // very important
			base64_decode(authBase64, authPlaint, strlen(authBase64));

			username = strtok_r(authPlaint, ":", &token);
			if(NULL == username){
				username = "";
			}
			password = strtok_r(NULL, ":", &token);
			if(NULL == password){
				password = "";
			}
			// FIXME: freezing!!
			USRM_I_KNOW_U_t * usrm = NULL;
			usrm = USRM_login(username, password);
			if(usrm != NULL){
				if(HTTP_IS_GET(context)){
					authorized = true;
				}else{//put
					if((usrm->permit_flag & USRM_PERMIT_SETTING) != 0){
						authorized = true;
					}
				}
				USRM_logout(usrm);
			}
		}
	}
			
	if(!authorized){
		LP_HTTP_HEAD_FIELD response_header = NULL;
		char response_buf[1024] = {""};
		size_t response_len = 0;
		stSOCKET_TCP sock_tcp;
		lpSOCKET_TCP tcp = socket_tcp2_r(context->sock, &sock_tcp);
		
		response_header = HTTP_UTIL_new_response_header(context->request_header->protocol,
			context->request_header->version, 401, NULL);
		response_header->add_tag_server(response_header, "IE/10.0");		
		response_header->add_tag_date(response_header, 0);
		response_header->add_tag_int(response_header, "Content-Length", 0, true);
		response_header->add_tag_text(response_header, "Connection", "close", true);
		//response_header->add_tag_text(response_header, "WWW-Authenticate", "Basic realm=\"NetSDK API\"", true);
		response_len = response_header->to_text(response_header, response_buf, sizeof(response_buf));
		response_header->dump(response_header);
		response_header->free(response_header);
		response_header = NULL;

		// clear keep alive
		//context->keep_alive = 0;
		
		//if(tcp->send2(tcp, response_buf, response_len) == response_len){ FIXME:
		ret = tcp->send(tcp, response_buf, response_len, 0);
		if(ret < 0){
			// do something!
		}

		usleep(5000);
		return 0;
	}
	
	if(prefix = "/SYSTEM", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		ret = NETSDK_system_instance(context, subURI, content, contentMax);
	}else if(prefix = "/AUDIO", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		ret = NETSDK_audio_instance(context, subURI, content, contentMax);
	}else if(prefix = "/VIDEO", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		ret = NETSDK_video_instance(context, subURI, content, contentMax);
	}else if(prefix = "/IO", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		ret = NETSDK_io_instance(context, subURI, content, contentMax);
	}else if(prefix = "/PTZ", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		ret = NETSDK_ptz_instance(context, subURI, content, contentMax);
	}else if(prefix = "/STREAM", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		ret = NETSDK_stream_instance(context, subURI, content, contentMax);
	}else if(prefix = "/NETWORK", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		ret = NETSDK_network_instance(context, subURI, content, contentMax);
	}else if(prefix = "/IMAGE", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		ret = NETSDK_image_instance(context, subURI, content, contentMax);
	}else{
	
	}

	if(kNSDK_INS_RET_CONTENT_SENT == ret){
		// do nothing
	}else if(kNSDK_INS_RET_CONTENT_READY == ret){
		NETSDK_response(context->sock, content);
	}else if(ret <= 0){
		int const code = -1 * ret;
		const char *mesg = "OK";

		switch(ret){
			case kNSDK_INS_RET_OK:
				mesg = "OK"; break;
			case kNSDK_INS_RET_DEVICE_BUSY:
				mesg = "Device Busy"; break;
			case kNSDK_INS_RET_DEVICE_ERROR:
				mesg = "Device Error"; break;
			case kNSDK_INS_RET_DEVICE_NOT_SUPPORT:
				mesg = "Device Not Support"; break;
			case kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT:
				mesg = "Device Not Implemented"; break;
			case kNSDK_INS_RET_INVALID_OPERATION:
				mesg = "Invalid Operation"; break;
			case kNSDK_INS_RET_INVALID_DOCUMENT:
				mesg = "Invalid Document"; break;
			case kNSDK_INS_RET_UNKNOWN_ERROR:
			default:
				mesg = "Unknown Error"; break;
		}

		snprintf(content, contentMax,
			"{"
			"\"requestMethod\" : \"%s\","
			"\"requestURL\" : \"%s\","
			"\"requestQuery\" : \"%s\","
			"\"statusCode\" : \"%d\","
			"\"statusMessage\" : \"%s\""
			"}",
			context->request_header->method,
			context->request_header->uri,
			context->request_header->query,
			code,
			mesg);
		
		NETSDK_response(context->sock, content);
	}
	return 0;
}

int NETSDK_conf_load(bool reload)
{
	if(reload){
		gs_multi_conf_mode = -1;//init multi config mode value
        netsdk->lock_sync_enabled = 0;
        pthread_rwlock_wrlock(&netsdk->audio_sync);
        pthread_rwlock_wrlock(&netsdk->video_sync);
        pthread_rwlock_wrlock(&netsdk->io_sync);
        pthread_rwlock_wrlock(&netsdk->ptz_sync);
        pthread_rwlock_wrlock(&netsdk->network_sync);
        pthread_rwlock_wrlock(&netsdk->image_sync);
        pthread_rwlock_wrlock(&netsdk->system_sync);
		// free json context
		if(netsdk->stream_conf){
			json_object_put(netsdk->stream_conf);
			netsdk->stream_conf = NULL;
		}
		if(netsdk->ptz_conf){
			json_object_put(netsdk->ptz_conf);
			netsdk->ptz_conf = NULL;
		}
		if(netsdk->io_conf){
			json_object_put(netsdk->io_conf);
			netsdk->io_conf = NULL;
		}
		if(netsdk->video_conf){
			json_object_put(netsdk->video_conf);
			netsdk->video_conf = NULL;
		}
		if(netsdk->audio_conf){
			json_object_put(netsdk->audio_conf);
			netsdk->audio_conf = NULL;
		}
		if(netsdk->jsonSystem){
			json_object_put(netsdk->jsonSystem);
			netsdk->jsonSystem = NULL;
		}
        if(NULL != netsdk->network_conf)
        {
            json_object_put(netsdk->network_conf);
            netsdk->network_conf = NULL;
        }
        if(NULL != netsdk->image_conf)
        {
            json_object_put(netsdk->image_conf);
            netsdk->image_conf = NULL;
        }
	}
		// load configuration context
	netsdk->jsonSystem = netsdk_load_json("system");
	netsdk->audio_conf = netsdk_load_json("audio");
	netsdk->video_conf = netsdk_load_json("video");
	netsdk->io_conf = netsdk_load_json("io");
	netsdk->ptz_conf = netsdk_load_json("ptz");
	netsdk->stream_conf = netsdk_load_json("stream");
	netsdk->network_conf = netsdk_load_json("network");
	netsdk->image_conf = netsdk_load_json("image");

    if(reload)
    {
        pthread_rwlock_unlock(&netsdk->audio_sync);
        pthread_rwlock_unlock(&netsdk->video_sync);
        pthread_rwlock_unlock(&netsdk->io_sync);
        pthread_rwlock_unlock(&netsdk->ptz_sync);
        pthread_rwlock_unlock(&netsdk->network_sync);
        pthread_rwlock_unlock(&netsdk->image_sync);
        pthread_rwlock_unlock(&netsdk->system_sync);
        netsdk->lock_sync_enabled = 1;
    }

	printf("%s\n", __FUNCTION__);
	return 0;
}

int NETSDK_get_multi_conf_mode(char *ret_mode)
{
	if(NULL != ret_mode){
		FILE *fd = NULL;
		char read_value[32] = {""};

		fd = fopen(sNSDK_MULTI_CONF_MODE_FILE_PATH, "r");
		if(fd){
			fread(read_value, 1, sizeof(read_value), fd);
			snprintf(ret_mode, 16, "%s", read_value);
			fclose(fd);
		}else{
			sprintf(ret_mode, sNSDK_MULTI_CONF_MODE_DEFAULT);
		}
		printf("%s--%s\r\n", __FUNCTION__, ret_mode);
		return NETSDK_MAP_STR2DEC(convenient_mode_map, read_value, kNSDK_MULTI_CONF_MODE_DEFAULT);
	}
	return 0;
}

int NETSDK_set_multi_conf_mode(char *mode)
{
	if(NULL != mode){
		FILE *fd = NULL;
		char read_value[32] = {""};

		fd = fopen(sNSDK_MULTI_CONF_MODE_FILE_PATH, "r+");
		if(fd){
			fread(read_value, 1, sizeof(read_value), fd);
			fclose(fd);
			if(!strncmp(read_value, mode, strlen(mode))){
				
				return -1;
			}else{
				fd = fopen(sNSDK_MULTI_CONF_MODE_FILE_PATH, "wb+");
				printf("%s--%s\r\n", __FUNCTION__, mode);
				if(fd){
					fwrite(mode, strlen(mode), 1, fd);
					fclose(fd);
				}
			}
		}else{
			return -1;
		}	
		
		return 0;
	}
	return -1;
}
