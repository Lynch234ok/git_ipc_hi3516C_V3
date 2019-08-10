
#include "netsdk.h"
#include <stdarg.h>
#include <hi_type.h>
#include "netsdk_private.h"
#include "sdk/sdk_api.h"
#include "sensor.h"
#include "generic.h"
#include "ticker.h"
#include "securedat.h"
#include "app_debug.h"
#include "ntp.h"
#include "version.h"
#include "sensor.h"
#include "socket_tcp.h"
#include "remote_upgrade.h"
#include <pan_tilt.h>
#include <AutoFocus.h>
#include "base/ja_process.h"
#include "cgi_bin.h"
#include "model_conf.h"
#include "custom.h"
#include "global_runtime.h"
#include "bsp/bsp.h"
#include "fisheye.h"
#include "wpa_supplicant/include/wpa_status.h"
#include "app_gsensor.h"
#include "app_bluetooth.h"
#include "production_test.h"
#include "network_interface.h"
#include "app_wifi.h"
#include "ipcam_timer.h"
#include "ipcam_network.h"
#include "media_buf.h"
#include "app_motion_detect.h"
#include "uart_protocol.h"



extern void NETSDK_conf_network_save();
pthread_mutex_t osd_mutex;
static const char *netsdk_sn_to_mac(const char *sn, char *result, int result_max)
{
	unsigned int serialNumber = atoi(sn + strlen(sn) - 9);
	snprintf(result, result_max, "00:9a:%02x:%02x:%02x:%02x",
		(serialNumber >> 24) & 0xff, (serialNumber >> 16) & 0xff, (serialNumber >> 8) & 0xff, (serialNumber >> 0) & 0xff);
	return result;
}

static int netsdk_system_device_info(LP_NSDK_SYSTEM_DEVICE_INFO dev_info)
{
	char serialNumber[32] = "H1250100000000";
	char macAddress[32] = {""}, text[32] = {""};
	struct tm buildDateTime;
	
	if(0 == UC_SNumberGet(serialNumber)) {
	}else{
		struct timespec timetic;
		clock_gettime(CLOCK_MONOTONIC, &timetic);
		srand((unsigned) timetic.tv_nsec);
		int i = 0;
		for(i = 14 - 7; i < 14; ++i){
			serialNumber[i] = rand() % 10 + '0';
		}
		//APP_TRACE("serialNumber:%s", serialNumber);
	}

    ST_MODEL_CONF model_conf;
    ST_NSDK_SYSTEM_SETTING info;
    NETSDK_conf_system_get_setting_info(&info);
	strcpy(dev_info->deviceDescription, "HD IP Camera");
    if(0 < strlen(info.capabilitySet.model))
    {
        strcpy(dev_info->model, info.capabilitySet.model);  // capability set model
    }
    else if(NULL != MODEL_CONF_get(&model_conf)) {
        strcpy(dev_info->model, model_conf.modelName);  // ´Ómodel_confÖĞ¶ÁÈ¡Éè±¸ĞÍºÅ
    }
	strcpy(dev_info->soc, getenv("SOC"));
	strcpy(dev_info->serialNumber, serialNumber);
	netsdk_sn_to_mac(serialNumber, macAddress, sizeof(macAddress));
	strcpy(dev_info->macAddress, macAddress);
	snprintf(dev_info->firmwareVersion, sizeof(dev_info->firmwareVersion), "%d.%d.%d.%s", SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT);
	//strcpy(dev_info->firmwareVersion, "V1.0.0");
	BUILD_DATE_TIME(&buildDateTime);
	strcpy(dev_info->firmwareReleaseDate, ISO8601(&buildDateTime, false, text, sizeof(text)));
	snprintf(dev_info->hardwareVersion, sizeof(dev_info->hardwareVersion), "%d.%d.%d.%s", HWVER_MAJ, HWVER_MIN, HWVER_REV, HWVER_EXT);
	//strcpy(dev_info->hardwareVersion, "V1.0.0");
    memset(dev_info->productCode, 0, sizeof(dev_info->productCode));
#if defined(DANA_P2P)

    /* å¤§æ‹¿product   codeç›®å‰æ˜¯ä»¥è¿™ç§æ–¹å¼å†™æ­» */
    int fixMode = FISHEYE_get_fix_mode();
    if(eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL == fixMode)
    {
        snprintf(dev_info->productCode, sizeof(dev_info->productCode), "10009Y");
    }
    else if(eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL == fixMode)
    {
        snprintf(dev_info->productCode, sizeof(dev_info->productCode), "1000B1");
    }
#else
    snprintf(dev_info->productCode, sizeof(dev_info->productCode), "");
#endif

	return 0;
}

static void netsdk_venc_channel_id_convert_sdk(int id, int *vin_id, int *stream_id)
{
	if(id < 100){
		*vin_id = 0;
	}else{
		*vin_id = id / 100 - 1;
	}
	*stream_id = id % 100 - 1;
}

static int netsdk_aenc_ch_set(int id, LP_NSDK_AENC_CH aenc_ch)
{
	int ain = 0;
	//fix me for the aenc param
	sdk_enc->release_stream_g711a(ain);

    if(sdk_audio->audio_set_ain_ch_attr_ptnum) {
        APP_TRACE("set the ptnum :%d", sdk_audio->audio_set_ain_ch_attr_ptnum(aenc_ch->codecType));
    }

	if(aenc_ch->enabled){
		sdk_enc->create_audio_stream(ain, ain, aenc_ch->codecType);
	}
	return 0;
}


//FIX ME:it was static before
/*static*/int netsdk_venc_ch_changed(int id, LP_NSDK_VENC_CH venc_ch)
{ 
	int vin = 0;
	int stream = 0;
	
	ST_SDK_ENC_STREAM_ATTR stream_attr;
	LP_SDK_ENC_STREAM_H264_ATTR h264_attr = &stream_attr.H264_attr;
	LP_SDK_ENC_STREAM_H265_ATTR h265_attr = &stream_attr.H265_attr;
	
	netsdk_venc_channel_id_convert_sdk(id, &vin, &stream);

	
#if defined(HI3518E_V2)	| defined(HI3516C_V2) 
	if(0 == stream){
		char file_path[40] = "/media/conf/main_resolution";
		FILE *fid = fopen(file_path, "rb");
		char buf[16] = "";
		char tmp[5] = "";
		int width ,height;
		if(NULL != fid){
			fread(buf,sizeof(buf),1,fid);
			fclose(fid);
			fid = NULL;
			sscanf(buf,"%d %s %d",&width,tmp,&height);
			
			if(width != venc_ch->resolutionWidth || height != venc_ch->resolutionHeight){				
				if(sdk_enc){	
					if(0 == sdk_enc->enc_resolution(venc_ch->resolutionWidth, venc_ch->resolutionHeight)){
						if(width * height >= 1920*1080 || venc_ch->resolutionWidth*venc_ch->resolutionHeight >= 1920*1080){
#ifdef F5

#else
							return 0;
#endif
						}else{
						}
					}else{					
						APP_TRACE("Resolution size exceeds limit or reset failed!!");
					}
				}else{
					APP_TRACE("sdk_enc don't exist");
				}
			}else{
				APP_TRACE("Resolution No change!");
			}
				
		}else{
			APP_TRACE("Open %s failed!",file_path);

		}
	}
#endif

	pthread_mutex_lock(&osd_mutex);

	switch(venc_ch->codecType){
		default:
		case kNSDK_CODEC_TYPE_H264:	
			if(0 == SDK_ENC_get_stream(vin, stream, &stream_attr)){
			// release all the overlays firstly
			APP_OVERLAY_release_title(vin);
			APP_OVERLAY_release_clock(vin);
			APP_OVERLAY_release_id(vin);
			//usleep(10000);
			
			if(kNSDK_BR_CONTROL_CBR == venc_ch->bitRateControlType){
				h264_attr->rc_mode = kSDK_ENC_H264_RC_MODE_CBR;
			}else if(kNSDK_BR_CONTROL_VBR == venc_ch->bitRateControlType){
				h264_attr->rc_mode = kSDK_ENC_H264_RC_MODE_VBR;
			}
			h264_attr->bps = venc_ch->constantBitRate;
			h264_attr->fps = venc_ch->frameRate;
			h264_attr->gop = venc_ch->keyFrameInterval;
			if(venc_ch->freeResolution){
				h264_attr->width = venc_ch->resolutionWidth;
				h264_attr->height = venc_ch->resolutionHeight;
			}else{
				h264_attr->width = (venc_ch->resolution>>16) & 0xffff;
				h264_attr->height = (venc_ch->resolution>>0) & 0xffff;
			}
			h264_attr->profile= venc_ch->h264Profile;
			stream_attr.enType = kSDK_ENC_BUF_DATA_H264;
			switch(venc_ch->definitionType){
				case kNSDK_DEFINITION_FLUENCY:
					h264_attr->quality = kSDK_ENC_QUALITY_FLUENCY;
					break;
				case kNSDK_DEFINITION_BD:
					h264_attr->quality = kSDK_ENC_QUALITY_BD;
					break;
				case kNSDK_DEFINITION_HD:
					h264_attr->quality = kSDK_ENC_QUALITY_HD;
				case kNSDK_DEFINITION_AUTO:
				default:
					h264_attr->quality = kSDK_ENC_QUALITY_AUTO;
					break;									
			}
            /* ä¸»è¦ç”¨äºå…¼å®¹å¯¹æ¥NVRæ—¶ï¼Œä¸å¼€å¯é«˜çº§è·³å¸§å‚è€ƒ */
            if(venc_ch->ImageTransmissionModel == eNSDK_LOW_BPS_MODEL) {
                h264_attr->refEnable = 1;
            }
            else if(venc_ch->ImageTransmissionModel == eNSDK_COMPATIBILITY_MODE) {
                h264_attr->refEnable = 0;
            }
            else {
                h264_attr->refEnable = 1;
            }
			APP_TRACE("%d %d %d %d", h264_attr->width, h264_attr->height, h264_attr->fps, h264_attr->bps);
			if(0 == SDK_ENC_set_stream(vin, stream, &stream_attr)){

				// setup success
				APP_OVERLAY_create_title(vin);
				APP_OVERLAY_create_clock(vin);
				APP_OVERLAY_create_id(vin);
				
				pthread_mutex_unlock(&osd_mutex);
				return 0;
			}
		}
	    break;
		
		case kNSDK_CODEC_TYPE_H265:					
			if(0 == SDK_ENC_get_stream(vin, stream, &stream_attr)){
			// release all the overlays firstly
			APP_OVERLAY_release_title(vin);
			APP_OVERLAY_release_clock(vin);
			APP_OVERLAY_release_id(vin);
#if defined(BSD_OSD)
			APP_OVERLAY_release_table(0);
#endif
			//usleep(10000);
			
			if(kNSDK_BR_CONTROL_CBR == venc_ch->bitRateControlType){
				h265_attr->rc_mode = kSDK_ENC_H265_RC_MODE_CBR;
			}else if(kNSDK_BR_CONTROL_VBR == venc_ch->bitRateControlType){
				h265_attr->rc_mode = kSDK_ENC_H265_RC_MODE_VBR;
			}
			h265_attr->bps = venc_ch->constantBitRate;
			h265_attr->fps = venc_ch->frameRate;
			h265_attr->gop = venc_ch->keyFrameInterval;
			if(venc_ch->freeResolution){
				h265_attr->width = venc_ch->resolutionWidth;
				h265_attr->height = venc_ch->resolutionHeight;
			}else{
				h265_attr->width = (venc_ch->resolution>>16) & 0xffff;
				h265_attr->height = (venc_ch->resolution>>0) & 0xffff;
			}
			h265_attr->profile= venc_ch->h264Profile;
			stream_attr.enType = kSDK_ENC_BUF_DATA_H265;
			switch(venc_ch->definitionType){
				case kNSDK_DEFINITION_FLUENCY:
					h265_attr->quality = kSDK_ENC_QUALITY_FLUENCY;
					break;
				case kNSDK_DEFINITION_BD:
					h265_attr->quality = kSDK_ENC_QUALITY_BD;
					break;
				case kNSDK_DEFINITION_HD:
					h265_attr->quality = kSDK_ENC_QUALITY_HD;
				case kNSDK_DEFINITION_AUTO:
				default:
					h265_attr->quality = kSDK_ENC_QUALITY_AUTO;
					break;									
			}
			APP_TRACE("%d %d %d %d", h265_attr->width, h265_attr->height, h265_attr->fps, h265_attr->bps);
			if(0 == SDK_ENC_set_stream(vin, stream, &stream_attr)){
				// setup success
				APP_OVERLAY_create_title(vin);
				APP_OVERLAY_create_clock(vin);
				APP_OVERLAY_create_id(vin);
#if defined(BSD_OSD)
		        APP_OVERLAY_create_table(0);
#endif
				pthread_mutex_unlock(&osd_mutex);
				
				return 0;
			}
		}

		break;
	}
	pthread_mutex_unlock(&osd_mutex);
	return -1;
}

static void netsdk_video_conf_delay_set(int id, void* venc_ch)
{
	APP_TRACE("NetSDK Video Conf Set!!");
	APP_TRACE("%d %d %d %d", ((LP_NSDK_VENC_CH)venc_ch)->resolutionWidth, ((LP_NSDK_VENC_CH)venc_ch)->resolutionHeight, ((LP_NSDK_VENC_CH)venc_ch)->frameRate, ((LP_NSDK_VENC_CH)venc_ch)->constantBitRate);
	netsdk_venc_ch_changed(id, (LP_NSDK_VENC_CH)venc_ch);
	TICKER_del_task2(netsdk_video_conf_delay_set, id); // only run once!
}

void NETSDK_venc_ch_delay_set(int id, void* venc_ch)
{
	TICKER_del_task2(netsdk_video_conf_delay_set, id);
	TICKER_add_task2(netsdk_video_conf_delay_set, 2, false, id, id, venc_ch, sizeof(ST_NSDK_VENC_CH));
}

static int netsdk_venc_request_keyframe(int id)
{
	int vin = 0;
	int stream = 0;
	netsdk_venc_channel_id_convert_sdk(id, &vin, &stream);
	SDK_ENC_request_stream_keyframe(vin, stream);
	return 0;
}

static int netsdk_ain_ch_set(int ain, LP_NSDK_AIN_CH ain_ch)
{
#if defined(HI3516E_V1)
	int audioInputGain = 0;
	if(ain_ch->microphoneType == kNSDK_ACTIVE_PICKUP){	
		audioInputGain =  50;//30db
		BSP_Audio_set_volume_val(audioInputGain, 0, -1, -1);
	}else if(ain_ch->microphoneType == kNSDK_PASSIVE_MIC){
		audioInputGain =  100;//60db
		BSP_Audio_set_volume_val(audioInputGain, 0, -1, -1);
	}
#else
	APP_TRACE("input:%d   output:%d", ain_ch->inputVolume, ain_ch->outputVolume);
	//BSP_Audio_set_volume_val(-1, ain_ch->inputVolume, -1, ain_ch->outputVolume);
#endif
	return 0;
}

//static int netsdk_vin_ch_set(int vin, LP_NSDK_VIN_CH vin_ch)
int netsdk_vin_ch_set(int vin, LP_NSDK_VIN_CH vin_ch)
{
	int i;
	ST_SDK_VIN_COVER_ATTR cover_attr;
    ST_CUSTOM_SETTING custom;
		
	for( i = 0; i < 4; i++){
		cover_attr.enable = vin_ch->privacyMask[i].enabled;
		cover_attr.x = vin_ch->privacyMask[i].regionX/100;
		cover_attr.y = vin_ch->privacyMask[i].regionY/100;
		cover_attr.width = vin_ch->privacyMask[i].regionWidth/100;
		cover_attr.height = vin_ch->privacyMask[i].regionHeight/100;
		cover_attr.color = vin_ch->privacyMask[i].regionColor | 0xFF000000;
		sdk_vin->set_cover(0,i,&cover_attr);
	}
	SENSOR_shutter_set(vin_ch->powerLineFrequencyMode);
	SENSOR_brightness_set(vin_ch->brightnessLevel);
	SENSOR_contrast_set(vin_ch->contrastLevel);
	SENSOR_hue_set(vin_ch->hueLevel);
	SENSOR_saturation_set(vin_ch->saturationLevel);
	if(vin_ch->flip){
		SENSOR_mirror_flip(MODE_FLIP);
	}else{
		SENSOR_mirror_flip(MODE_UNFLIP);
	}
	if(vin_ch->mirror){
		SENSOR_mirror_flip(MODE_MIRROR);
	}else{
		SENSOR_mirror_flip(MODE_UNMIRROR);
	}

	//SENSOR_sharpen_set(vin_ch->sharpnessLevel);
	return 0;
}

int netsdk_md_ch_set(int vin, LP_NSDK_MD_CH md_ch)
{
	int i = 0, ii = 0;
	bool enabled = false;

	APP_TRACE("md_ch->detectionType = %d", md_ch->detectionType);
	
	if(kNSDK_MD_TYPE_GRID != md_ch->detectionType &&
		kNSDK_MD_TYPE_REGION != md_ch->detectionType){
		return -1;
	}
	if(md_ch->detectionType == kNSDK_MD_TYPE_GRID){
		float threshold = (float)(100 - md_ch->detectionGrid.sensitivityLevel) / 100.0;

		if(threshold > 0.8){
			threshold = 0.8;
		}
		if(threshold < 0.02){
			threshold = 0.02;
		}
		
		sdk_vin->stop_md(0);
		sdk_vin->clear_md_bitmap_mask(0);
		sdk_vin->set_md_bitmap_mode(0,true);
		sdk_vin->set_md_bitmap_threshold(0, threshold);
		
		for(i = 0; i < md_ch->detectionGrid.rowGranularity; ++i){
			for(ii = 0; ii <md_ch->detectionGrid.columnGranularity; ++ii){
				LP_NSDK_MD_GRID const detectionGrid = &md_ch->detectionGrid;
				bool const flag = detectionGrid->getGranularity(detectionGrid, i, ii);
				sdk_vin->set_md_bitmap_one_mask(0, ii, i, flag);
			}
		}
		enabled = true;
	}else if(md_ch->detectionType == kNSDK_MD_TYPE_REGION){
		int i;
		char md_name[16];
		
		sdk_vin->stop_md(0);
		sdk_vin->clear_md_rect(0);
		sdk_vin->set_md_bitmap_mode(0,false);
		for(i = 0;i < (sizeof(md_ch->detectionRegion.ch)/sizeof(md_ch->detectionRegion.ch[0])); ++i){				
			if(md_ch->detectionRegion.ch[i].enabled == true){
				float threshold = (float)(100 - md_ch->detectionRegion.ch[i].sensitivityLevel) / 100.0;

				if(threshold > 0.8){
					threshold = 0.8;
				}
				if(threshold < 0.2){
					threshold = 0.2;
				}
				
				sprintf(md_name,"%d",i+1);
				sdk_vin->add_md_rect(0,
					md_name,
					md_ch->detectionRegion.ch[i].regionX/100,
					md_ch->detectionRegion.ch[i].regionY/100,
					md_ch->detectionRegion.ch[i].regionWidth/100,
					md_ch->detectionRegion.ch[i].regionHeight/100,
					threshold);
				enabled = true;
			}
		}
	}
	
	if(md_ch->enabled == true && enabled==true){
		sdk_vin->start_md(0);
	}
	return 0;
}

static time_t _motion_detection_timestamp[1];
void APP_NETSDK_mark_motion_detection(int videoInputID)
{
	int const n_channels = sizeof(_motion_detection_timestamp) / sizeof(_motion_detection_timestamp[0]);
	if(videoInputID - 1 < n_channels){
		// update motion detection timestamp
		//APP_TRACE("Motion @ %d", _motion_detection_timestamp[videoInputID - 1]);
		time(&_motion_detection_timestamp[videoInputID - 1]);
	}
}

static bool netsdk_md_status(int vin, bool clearFlag)
{
	vin = 0; // FIXME:
	time_t const t = time(NULL);
	
	if(clearFlag){
		// clear the timestamp of motion detection
		_motion_detection_timestamp[vin] = 0;
	}else{
		int diff = t - _motion_detection_timestamp[vin];
		APP_TRACE("Last Motion Detection(%d) is %d", vin, diff);
		if(abs(diff) <= 5){
			return true;
		}else{
			return false;
		}
	}
	return false;
}

static int netsdk_venc_snapshot(int id, int type, int width, int height, char *file_path)
{
	int ret = -1;
	FILE *fid = NULL;
	struct tm tm_now;
	time_t t = time(NULL);
	ST_NSDK_NETWORK_INTERFACE lan0;
	NETSDK_conf_interface_get(1, &lan0);

	localtime_r(&t, &tm_now);
	sprintf(file_path, "/tmp/%s %04d-%02d-%02d %02d-%02d-%02d.jpg", 
		lan0.lan.staticIP, 
		tm_now.tm_year + 1900,
		tm_now.tm_mon + 1,
		tm_now.tm_mday,
		tm_now.tm_hour,
		tm_now.tm_min,
		tm_now.tm_sec);
	//sprintf(file_path, "/tmp/netsdk_%dx%d_%08x%08x.jpg", width, height, rand(), rand());
	APP_TRACE("JPEG:%s", file_path);
	fid = fopen(file_path, "w+b");
	if(NULL != fid){
		int vin = 0;
		int stream = 0;
		netsdk_venc_channel_id_convert_sdk(id, &vin, &stream);
		ret = sdk_enc->snapshot(vin, kSDK_ENC_SNAPSHOT_QUALITY_MEDIUM, width, height, fid);
		fclose(fid);
		fid = NULL;
		return ret;
	}
	return -1;
}

static void netsdk_video_conf_delay_save()
{
	TICKER_del_task(netsdk_video_conf_delay_save); // only run once!
	APP_TRACE("NetSDK Video Conf Save!!");
	NETSDK_conf_video_save();
}

static int netsdk_video_conf_save(EM_NSDK_CONF_SAVE_OPERATION opteration, int delay)
{
	TICKER_del_task(netsdk_video_conf_delay_save);
	TICKER_add_task(netsdk_video_conf_delay_save, delay, false);
}

static void netsdk_audio_conf_delay_save()
{
	TICKER_del_task(netsdk_audio_conf_delay_save); // only run once!
	APP_TRACE("NetSDK Audio Conf Save!!");
	NETSDK_conf_audio_save();
}

static int netsdk_audio_conf_save(EM_NSDK_CONF_SAVE_OPERATION opteration, int delay)
{
	TICKER_del_task(netsdk_audio_conf_delay_save);
	TICKER_add_task(netsdk_audio_conf_delay_save, delay, false);
	return 0;
}

static void netsdk_system_conf_delay_save()
{
	TICKER_del_task(netsdk_system_conf_delay_save); // only run once!
	APP_TRACE("NetSDK System Conf Save!!");
	NETSDK_conf_system_save();
}

static int netsdk_system_conf_save(EM_NSDK_CONF_SAVE_OPERATION opteration, int delay)
{
	TICKER_del_task(netsdk_system_conf_delay_save);
	TICKER_add_task(netsdk_system_conf_delay_save, delay, false);
	return 0;
}

static int netsdk_alarm_in_port_status(int id, LP_NSDK_ALARM_IN_CH alarmInCh, int *status)
{
	char ioName[32] = {""};
	bool ioPinLevel = false;
	snprintf(ioName, sizeof(ioName), "alarm in %d", id - 1);
	APP_TRACE("Port Status \"%s\"", ioName);
	if(0 == APP_GPIO_get_pin2(ioName, &ioPinLevel)){
		if(status){
			if(ioPinLevel){
				*status = kNSDK_IO_STATE_HIGH;
			}else{
				*status = kNSDK_IO_STATE_LOW;
			}
			return 0;
		}
	}
	return -1;
}

static int netsdk_alarm_out_port_status(int id, LP_NSDK_ALARM_OUT_CH alarmInCh, int *status)
{
	char ioName[32] = {""};
	bool ioPinLevel = false;
	snprintf(ioName, sizeof(ioName), "alarm out %d", id - 1);
	APP_TRACE("Port Status \"%s\"", ioName);
	if(0 == APP_GPIO_get_pin(ioName, &ioPinLevel)){
		if(status){
			if(ioPinLevel){
				*status = kNSDK_IO_STATE_HIGH;
			}else{
				*status = kNSDK_IO_STATE_LOW;
			}
			return 0;
		}
	}
	return -1;
}

static void netsdk_alarm_out_trigger_pulse(uint32_t lPara, uint32_t pPara)
{
	int const alarmOutChannelID = *((int *)lPara);
	bool *const pulseState = (bool *)(pPara);
	char alarmOutName[32] = {""};
	
	snprintf(alarmOutName, sizeof(alarmOutName), "alarm out %d", alarmOutChannelID - 1);
	if(kNSDK_IO_STATE_HIGH == *pulseState){
		APP_GPIO_set_pin(alarmOutName, false);
		APP_TRACE("Pulse \"%s\" positive", alarmOutName);
	}else{
		APP_GPIO_set_pin(alarmOutName, true);
		APP_TRACE("Pulse \"%s\" negative", alarmOutName);
	}
	
	// remove this task at last
	TICKER_del_task(netsdk_alarm_out_trigger_pulse);
}

static int netsdk_alarm_out_trigger(int id, LP_NSDK_ALARM_OUT_CH_TRIGGER_METHOD triggerMethod)
{
	char alarmOutName[32] = {""};
	snprintf(alarmOutName, sizeof(alarmOutName), "alarm out %d", id - 1);

	APP_TRACE("Trigger \"%s\"", alarmOutName);

	switch(triggerMethod->outputState){
	case kNSDK_IO_STATE_LOW:
		{
			APP_GPIO_set_pin(alarmOutName, true);
			break;
		}
	case kNSDK_IO_STATE_HIGH:
		{
			APP_GPIO_set_pin(alarmOutName, false);
			break;
		}
	case kNSDK_IO_STATE_PULSE:
		{
			//int pulseState = calloc(sizeof(triggerMethod->pulseState), 1);
			//*pulseState = triggerMethod->pulseState;
			int pulseState = triggerMethod->pulseState;
			if(kNSDK_IO_STATE_HIGH == pulseState){
				APP_GPIO_set_pin(alarmOutName, false);
				APP_GPIO_set_pin(alarmOutName, true);
			}else if(kNSDK_IO_STATE_LOW == pulseState){
				APP_GPIO_set_pin(alarmOutName, true);
				APP_GPIO_set_pin(alarmOutName, false);
			}
			TICKER_add_task2(netsdk_alarm_out_trigger_pulse, triggerMethod->pulseDuration, false, id, (long)id, (void*)&pulseState, sizeof(int));
			break;
		}
	default:
		return -1;
	}
	return 0;
}

static int netsdk_ptz_config_changed(int nBaudRate, int nDateBit, char strParity, int nStopBit)
{
#if defined(UART_PROTOCOL)
	return UART_protocol_reconfig(nBaudRate,  nDateBit, strParity, nStopBit);
#endif
    return 0;

}

static void netsdk_network_conf_delay_save()
{
	TICKER_del_task(netsdk_network_conf_delay_save); // only run once!
	APP_TRACE("NetSDK Network Conf Save!!");
	exit(0);
}

static void netsdk_network_conf_delay_restart()
{
	TICKER_del_task(netsdk_network_conf_delay_restart); // only run once!
	APP_TRACE("NetSDK Network Conf Save!!");
	IPCAM_timer_network_destroy();
	IPCAM_network_restart();
	IPCAM_timer_network_init();
}

static void netsdk_network_conf_delay_restart_wireless()
{
    TICKER_del_task(netsdk_network_conf_delay_restart_wireless); // only run once!
    APP_TRACE("Wireless Restart!!");
    IPCAM_network_wireless_restart();
}

static int netsdk_network_conf_save(EM_NSDK_CONF_SAVE_OPERATION opteration, int delay)
{
	NETSDK_conf_network_save();
	if (opteration == eNSDK_CONF_SAVE_RESTART) {
		TICKER_del_task(netsdk_network_conf_delay_restart);
		TICKER_add_task(netsdk_network_conf_delay_restart, delay, false);
	} else if (opteration == eNSDK_CONF_SAVE_REBOOT) {
		TICKER_del_task(netsdk_network_conf_delay_save);
		TICKER_add_task(netsdk_network_conf_delay_save, delay, false);
	}  else if (opteration == eNSDK_CONF_SAVE_RESTART_WIRELESS) {
		TICKER_del_task(netsdk_network_conf_delay_restart_wireless);
		TICKER_add_task(netsdk_network_conf_delay_restart_wireless, delay, false);
	} else {
		APP_TRACE("NetSDK Network Conf Just Save!!");
	} 
}

static int netsdk_system_changed(LP_NSDK_SYSTEM_TIME sys_time)
{
	APP_TRACE("GMT: %d", sys_time->greenwichMeanTime);
	APP_TRACE("ntpEnable: %d", sys_time->ntpEnabled);
	APP_TRACE("ntpServerDomain: %s", sys_time->ntpServerDomain);
	APP_TRACE("ntpServerBackupOne: %s", sys_time->ntpServerBackupOne);
	APP_TRACE("ntpServerBackupTwo: %s", sys_time->ntpServerBackupTwo);
	GMT_SET(sys_time->greenwichMeanTime);

	if(sys_time->ntpEnabled){
		NTP_start(sys_time->ntpServerDomain, sys_time->ntpServerBackupOne, sys_time->ntpServerBackupTwo, NULL, 5, 0); // ÒòÎª×îĞÂµÄĞŞ¸Ä£¬ntpÍ¬²½Ê±²»ÉèÖÃÊ±Çø£¬ËùÒÔ²ÎÊı4²»ÉúĞ§
	}
	return 0;
}

static int netsdk_system_DST_changed(LP_NSDK_SYSTEM_DST dst_time)
{
	DST_SET(dst_time->enable ? dst_time->offset : 0, 
		dst_time->week[0].month, dst_time->week[0].week, dst_time->week[0].weekday, dst_time->week[0].hour, dst_time->week[0].minute, 
		dst_time->week[1].month, dst_time->week[1].week, dst_time->week[1].weekday, dst_time->week[1].hour, dst_time->week[1].minute);
	
	return 0;
}

static int netsdk_fix_model_conf_param()
{
	ST_NSDK_VENC_CH venc_ch;
	ST_MODEL_CONF model_conf;
	int ret = 0;//is reseting factory

	if(NULL != MODEL_CONF_get(&model_conf)){
		//use param setting from model config
		//video main stream
		NETSDK_conf_venc_ch_get(101, &venc_ch);
		if(0 != strcmp(model_conf.video[0].resolutionProperty.opt, venc_ch.resolutionProperty.opt)){
			venc_ch.resolution = model_conf.video[0].resolution;
			venc_ch.frameRate = model_conf.video[0].fps;
			venc_ch.constantBitRate = model_conf.video[0].bps;
			memset(venc_ch.resolutionProperty.opt, 0, sizeof(ST_NSDK_PARA_PROPERTY));
			strncpy(venc_ch.resolutionProperty.opt, model_conf.video[0].resolutionProperty.opt, sizeof(ST_NSDK_PARA_PROPERTY));
			if(NULL != MODEL_CONF_get(&model_conf)){
				if(MODEL_CONF_check_int_valid(model_conf.osd.timeX)){
					venc_ch.datetimeOverlay.o.regionX = model_conf.osd.timeX;
				}
				if(MODEL_CONF_check_int_valid(model_conf.osd.timeY)){
					venc_ch.datetimeOverlay.o.regionY = model_conf.osd.timeY;
				}
			}
			NETSDK_conf_venc_ch_set2(101, &venc_ch, true);
			NETSDK_venc_ch_delay_set(101, &venc_ch);
			APP_TRACE("MODEL CONF matching main stream!");

			ret = 1;
		}

		//video sub stream
		NETSDK_conf_venc_ch_get(102, &venc_ch);
		if(0 != strcmp(model_conf.video[1].resolutionProperty.opt, venc_ch.resolutionProperty.opt)){
			venc_ch.resolution = model_conf.video[1].resolution;
			venc_ch.frameRate = model_conf.video[1].fps;
			venc_ch.constantBitRate = model_conf.video[1].bps;
			memset(venc_ch.resolutionProperty.opt, 0, sizeof(ST_NSDK_PARA_PROPERTY));
			strncpy(venc_ch.resolutionProperty.opt, model_conf.video[1].resolutionProperty.opt, sizeof(ST_NSDK_PARA_PROPERTY));
			NETSDK_conf_venc_ch_set2(102, &venc_ch, true);
			NETSDK_venc_ch_delay_set(102, &venc_ch);
			APP_TRACE("MODEL CONF matching sub stream!");
			ret = 1;
		}
		ST_NSDK_IMAGE image;
		NETSDK_conf_image_get(&image);
		image.videoMode.fixType = FISHEYE_get_fix_mode();
        if(1 == ret){
            image.irCutFilter.irCutControlMode = model_conf.ircutFilter.irCutControlMode;
        }
		NETSDK_conf_image_set(&image);
	}else{
		//use param setting from netsdk json
		APP_TRACE("model config file is not exist!");
	}
	return ret;
}


static void netsdk_video_set_default_resolution(int sensor_type)
{
	ST_NSDK_VENC_CH venc_ch;
	NETSDK_conf_venc_ch_get(101, &venc_ch);
	
	if(sensor_type == SENSOR_MODEL_APTINA_AR0130 ||
		sensor_type == SENSOR_MODEL_SC1035 ||
		sensor_type == SENSOR_MODEL_SC1135 ||		
		sensor_type == SENSOR_MODEL_IMX225 ){
		venc_ch.resolution = kNSDK_RES_1280X960;
	}	
    else if(sensor_type == SENSOR_MODEL_SONY_IMX178){
		venc_ch.resolution = kNSDK_RES_2592X1944;
	}	
	else if(sensor_type == SENSOR_MODEL_APTINA_AR0330 && (!strcmp(SOC_MODEL,"HI3516D"))){
		venc_ch.resolution = kNSDK_RES_2048X1520;
	}
	else if(sensor_type == SENSOR_MODEL_OV_OV4689){
		venc_ch.resolution = kNSDK_RES_2592X1520;
	}
	else if(sensor_type == SENSOR_MODEL_SONY_IMX185 || sensor_type == SENSOR_MODEL_MN34220){
		venc_ch.resolution = kNSDK_RES_1920X1080;
		venc_ch.constantBitRate = 4096;
	}else if(sensor_type == SENSOR_MODEL_OV5658){
		venc_ch.resolution = kNSDK_RES_2592X1944;
	}else if(sensor_type == SENSOR_MODEL_SC1045 || sensor_type == SENSOR_MODEL_SC1145){
		venc_ch.resolution = kNSDK_RES_1280X720;
	}

////FIXME
	if((!strcmp(PRODUCT_MODEL,"578010") || !strcmp(PRODUCT_MODEL,"578011"))){	//!strcmp(SOC_MODEL,"HI3518E_V2") 
		if(venc_ch.resolution == kNSDK_RES_1280X720){
			venc_ch.constantBitRate = 2048;
		}else if(venc_ch.resolution == kNSDK_RES_1280X960){
			venc_ch.constantBitRate = 2560;		
		}else if(venc_ch.resolution == kNSDK_RES_1920X1080){		
			venc_ch.constantBitRate = 3072;			
		}else{
		}
	}
	
	NETSDK_conf_venc_ch_set(101, &venc_ch);
	NETSDK_venc_ch_delay_set(101, &venc_ch);
	
}


static int netsdk_network_wireless_status(EM_NSDK_NETWORK_WIFI_STATUS_TYPE type, LP_NSDK_NETWORK_WIFI_STATUS_ATTR status_attr)
{
	if(status_attr){
		status_attr->statusType = type;
		switch(type){
			case eNSDK_NETWORK_WIFI_STATUS_TYPE_MODEL_EXIST:
				status_attr->modelExist = APP_WIFI_model_exist();
				break;
			case eNSDK_NETWORK_WIFI_STATUS_TYPE_STATION_SIGNAL:
				status_attr->stationSignal = APP_WIFI_get_rssi();
				break;
			default:
				status_attr->statusType = eNSDK_NETWORK_WIFI_STATUS_TYPE_NONE;
				break;
		}
	}
	return 0;
}


static void netsdk_image_conf_delay_save()
{
	TICKER_del_task(netsdk_image_conf_delay_save); // only run once!
	APP_TRACE("NetSDK Image Conf Save!!");
	NETSDK_conf_image_save();
}

static int netsdk_image_conf_save(EM_NSDK_CONF_SAVE_OPERATION opteration, int delay)
{
	TICKER_del_task(netsdk_image_conf_delay_save);
	TICKER_add_task(netsdk_image_conf_delay_save, delay, false);
	return 0;
}

static void netsdk_sensor_cfg_init(int init)
{
	ST_NSDK_IMAGE image;
	char srcFilePath[128] = "";
	char sensor_name[16];
	NETSDK_conf_image_get(&image);

	emSENSOR_MODEL sensor_type = 0;

	memset(sensor_name,0,sizeof(sensor_name));
	sensor_type = SENSOR_get_sensor_model(sensor_name);
	
	 /* ¸ù¾İIDºÅÊÇ·ñÊÇÒÔF ¿ªÍ·È·¶¨ispcfgÎÄ¼ş */

	if(NK_False == GLOBAL_sn_front()){
		sprintf(srcFilePath, "%s/Cx/isp_cfg_%s_%d.ini", getenv("ISPCFG"), sensor_name, image.imageStyle);
	}else{
		sprintf(srcFilePath, "%s/Px/isp_cfg_%s_%d.ini", getenv("ISPCFG"), sensor_name, image.imageStyle);
	}

	if(!IS_FILE_EXIST(ISP_CFG_TMP_INI) || !init){
		APP_TRACE("copy isp cfg file:%s", srcFilePath);
		COPY_FILE(srcFilePath,ISP_CFG_TMP_INI);
	}
}


static void netsdk_sensor_wdr_avenc_init()
{
	int i = 0, ii = 0, iii = 0;
	int stream_cnt = NETSDK_venc_get_channels();

	for(i = 0; i < 1; ++i){
		// video input
		for(ii = 0; ii < stream_cnt; ++ii){
			// video h264 encode
			char preferred_name[32] = {""};
			char alternate_name[32] = {""};
			sprintf(preferred_name, "ch%d_%d.264", i, ii);	
			//fix me
			if(ii ==0){
				//main stream
				sprintf(alternate_name, "720p.264");
			}else if(ii == 1){
				//sub stream 1
				sprintf(alternate_name, "360p.264");
			}else{
				//sub stream 2
				sprintf(alternate_name, "qvga.264");
			}

			int const entry_available = 400;
			int const entry_key_available = 1;

		
			// success to new a mediabuf
		//	ST_SDK_ENC_STREAM_H264_ATTR stream_h264_attr;	

			int stream_rc_mode;

			ST_SDK_ENC_STREAM_ATTR stream_attr;	
			LP_SDK_ENC_STREAM_H264_ATTR stream_h264_attr = &stream_attr.H264_attr;				
			LP_SDK_ENC_STREAM_H264_ATTR stream_h265_attr = &stream_attr.H265_attr;
			

			int venc_id = 0;
			int const buf_id = MEDIABUF_lookup_byname(preferred_name);
		//	strcpy(stream_h264_attr.name, preferred_name);

			// start video stream
			// FIXME:
			ST_NSDK_VIN_CH vin_ch;
			if(NETSDK_conf_vin_ch_get(1, &vin_ch)&& ii==0){
				netsdk_vin_ch_set(1, &vin_ch);
			}	
			ST_NSDK_VENC_CH venc_ch;
			if(NETSDK_conf_venc_ch_get((i+1)*100+ii+1, &venc_ch)){
				switch(venc_ch.codecType){
					default:
					case kNSDK_CODEC_TYPE_H264:							
						stream_attr.enType = kSDK_ENC_BUF_DATA_H264;
						stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR;
						strcpy(stream_h264_attr->name, preferred_name);
						
						if(venc_ch.freeResolution){
							stream_h264_attr->width = venc_ch.resolutionWidth;
							stream_h264_attr->height = venc_ch.resolutionHeight;
						}else{
							stream_h264_attr->width = (venc_ch.resolution >> 16) & 0xffff;
							stream_h264_attr->height = (venc_ch.resolution >> 0) & 0xffff;
						}
						//printf("\n\n\nresolution: %dx%d\n\n\n", stream_h264_attr->width, stream_h264_attr->height);
						stream_h264_attr->fps = venc_ch.frameRate;
						stream_h264_attr->gop = venc_ch.keyFrameInterval;
						// FIXME:
						if(stream_h264_attr->gop > 100){
							stream_h264_attr->gop = 100;
						}
	//						stream_h264_attr->profile = kSDK_ENC_H264_PROFILE_MAIN;
						stream_h264_attr->profile = venc_ch.h264Profile;
						switch(venc_ch.bitRateControlType){
							case kNSDK_BR_CONTROL_VBR:
								stream_rc_mode = kSDK_ENC_H264_RC_MODE_VBR; break;
							case kNSDK_BR_CONTROL_CBR:
								stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR; break;
							default:
								stream_rc_mode = kSDK_ENC_H264_RC_MODE_VBR; break;
						}
						stream_h264_attr->rc_mode = stream_rc_mode;
						stream_h264_attr->bps = venc_ch.constantBitRate;
						stream_h264_attr->quality = 0;
						stream_h264_attr->buf_id = buf_id;

                        /* ä¸»è¦ç”¨äºå…¼å®¹å¯¹æ¥NVRæ—¶ï¼Œä¸å¼€å¯é«˜çº§è·³å¸§å‚è€ƒ */
						if(venc_ch.ImageTransmissionModel == eNSDK_LOW_BPS_MODEL) {
                            stream_h264_attr->refEnable = 1;
                        }
                        else if(venc_ch.ImageTransmissionModel == eNSDK_COMPATIBILITY_MODE) {
                            stream_h264_attr->refEnable = 0;
                        }
                        else {
                            stream_h264_attr->refEnable = 1;
                        }
                        stream_h264_attr->_enable_smartP = false;
					//	sdk_enc->create_stream_h264(i, ii, stream_h264_attr);
					//	sdk_enc->enable_stream_h264(i, ii, true);

						
						SDK_ENC_create_stream(i, ii, &stream_attr);
						SDK_ENC_enable_stream(i, ii, true);

						break;
					case kNSDK_CODEC_TYPE_H265:
						
						stream_attr.enType = kSDK_ENC_BUF_DATA_H265;
						stream_rc_mode = kSDK_ENC_H265_RC_MODE_CBR;
						strcpy(stream_h265_attr->name, preferred_name);
						if(venc_ch.freeResolution){
							stream_h265_attr->width = venc_ch.resolutionWidth;
							stream_h265_attr->height = venc_ch.resolutionHeight;
						}else{
							stream_h265_attr->width = (venc_ch.resolution >> 16) & 0xffff;
							stream_h265_attr->height = (venc_ch.resolution >> 0) & 0xffff;
						}
						//printf("\n\n\nresolution: %dx%d\n\n\n", stream_h265_attr->width, stream_h265_attr->height);
						stream_h265_attr->fps = venc_ch.frameRate;
						stream_h265_attr->gop = venc_ch.keyFrameInterval;
						// FIXME:
						if(stream_h265_attr->gop > 100){
							stream_h265_attr->gop = 100;
						}
	//						stream_h265_attr->profile = kSDK_ENC_H264_PROFILE_MAIN;
						stream_h265_attr->profile = venc_ch.h264Profile;
						switch(venc_ch.bitRateControlType){
							case kNSDK_BR_CONTROL_VBR:
								stream_rc_mode = kSDK_ENC_H265_RC_MODE_VBR; break;
							case kNSDK_BR_CONTROL_CBR:
								stream_rc_mode = kSDK_ENC_H265_RC_MODE_CBR; break;
							default:
								stream_rc_mode = kSDK_ENC_H265_RC_MODE_VBR; break;
						}
						stream_h265_attr->rc_mode = stream_rc_mode;
						stream_h265_attr->bps = venc_ch.constantBitRate;
						stream_h265_attr->quality = 0;
						stream_h265_attr->buf_id = buf_id;
                        stream_h265_attr->_enable_smartP = false;
						
						SDK_ENC_create_stream(i, ii, &stream_attr);
						SDK_ENC_enable_stream(i, ii, true);
						break;	
				}					
			}



			
			
		}

		ST_NSDK_IMAGE image;
		if(NETSDK_conf_image_get(&image)){
			netsdk_image_changed(&image);
		}
	}
}



static int netsdk_sensor_wdr_reset(uint8_t bEnable)    // 0 1
{
	uint8_t	old_wdr_enable = 0;
	char sensor_name[16];
	emSENSOR_MODEL sensor_type = 0;

	memset(sensor_name,0,sizeof(sensor_name));
	sensor_type = SENSOR_get_sensor_model(sensor_name);

	SENSOR_WDR_mode_get(&old_wdr_enable);


	if(old_wdr_enable != bEnable && sensor_type != SENSOR_MODEL_OV_OV4689 && sensor_type != SENSOR_MODEL_MN34220){		
		SENSOR_WDR_mode_enable(bEnable); //usr auto DRC mode		
	}else if(old_wdr_enable != bEnable && (sensor_type == SENSOR_MODEL_OV_OV4689 ||  sensor_type == SENSOR_MODEL_MN34220) ){
		SDK_ENC_wdr_destroy();
		APP_MD_destroy();	  
		SENSOR_WDR_mode_enable(bEnable);   
		netsdk_sensor_wdr_avenc_init();
		APP_MD_init(1);
	}
	return 0;
}

static emSDK_ENC_FIX_MODE fixmode_netsdk2sdk(EM_NSDK_IMAGE_FISHEYE_FIX_MODE fixMode)
{
	emSDK_ENC_FIX_MODE sdkMode;
	switch(fixMode){
		default:
		case eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL:
			sdkMode = eSDK_ENC_FIX_MODE_WALL;
			break;
		case eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_180;
			break;
		case eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_SPLIT;
			break;
	}
	return sdkMode;
}

static emSDK_ENC_SHOW_MODE showmode_netsdk2sdk(EM_NSDK_IMAGE_FISHEYE_SHOW_MODE showMode)
{
	emSDK_ENC_SHOW_MODE sdkMode;
	switch(showMode){
		default:
		case eNSDK_IMAGE_FISHEYE_MODE_WALL_ORIGIN:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_ORIGIN;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_WALL_180:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_180;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_WALL_SPLIT:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_SPLIT;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_WALL_WALL_SPLIT:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_WALL_SPLIT;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_WALL_4R:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_4R;
			break; 
		case eNSDK_IMAGE_FISHEYE_MODE_WALL_KITR:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_KITR;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_WALL_KITO:
			sdkMode = eSDK_ENC_SHOW_MODE_WALL_KITO;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_CELL_ORIGIN:
			sdkMode = eSDK_ENC_SHOW_MODE_CELL_ORIGIN;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_CELL_360:
			sdkMode = eSDK_ENC_SHOW_MODE_CELL_360;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_CELL_SPLIT:
			sdkMode = eSDK_ENC_SHOW_MODE_CELL_SPLIT;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_CELL_4R:
			sdkMode = eSDK_ENC_SHOW_MODE_CELL_4R;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_CELL_180:
			sdkMode = eSDK_ENC_SHOW_MODE_CELL_180;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_CELL_WALL_SPLIT:
			sdkMode = eSDK_ENC_SHOW_MODE_CELL_WALL_SPLIT;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_CELL_KITR:
			sdkMode = eSDK_ENC_SHOW_MODE_CELL_KITR;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_CELL_KITO:
			sdkMode = eSDK_ENC_SHOW_MODE_CELL_KITO;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_TABLE_ORIGIN:
			sdkMode = eSDK_ENC_SHOW_MODE_TABLE_ORIGIN;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_TABLE_360:
			sdkMode = eSDK_ENC_SHOW_MODE_TABLE_360;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_TABLE_SPLIT:
			sdkMode = eSDK_ENC_SHOW_MODE_TABLE_SPLIT;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_TABLE_4R:
			sdkMode = eSDK_ENC_SHOW_MODE_TABLE_4R;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_TABLE_VR:
			sdkMode = eSDK_ENC_SHOW_MODE_TABLE_VR;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_TABLE_KITR:
			sdkMode = eSDK_ENC_SHOW_MODE_TABLE_KITR;
			break;
		case eNSDK_IMAGE_FISHEYE_MODE_TABLE_KITO:
			sdkMode = eSDK_ENC_SHOW_MODE_TABLE_KITO;
			break;
		
	}
	
	return sdkMode;
}


int netsdk_image_changed(LP_NSDK_IMAGE image)
{
	netsdk_sensor_cfg_init(false);
	SENSOR_cfg_load(ISP_CFG_TMP_INI);
	SENSOR_denoise_3d_strength(image->denoise3d.denoise3dStrength);
	SENSOR_scene_mode_set(image->sceneMode);
	SENSOR_WB_mode_set(image->awbMode);
	SENSOR_sharpen_set(image->manualSharpness.sharpnessLevel, image->manualSharpness.enabled);
	SENSOR_ircut_control_mode_set(image->irCutFilter.irCutControlMode);
	SENSOR_ircut_mode_set(image->irCutFilter.irCutMode);
	SENSOR_lowlight_enable(image->lowlightMode);
	netsdk_sensor_wdr_reset(image->wdr.enabled);
	SENSOR_WDR_strength_set(image->wdr.WDRStrength);
	//enc mode:fisheye
	sdk_enc->enc_mode(0, 0, fixmode_netsdk2sdk(image->videoMode.fixType), showmode_netsdk2sdk(image->videoMode.showMode));
	return 0;
}

int netsdk_image_changed2(LP_NSDK_IMAGE image)
{
    netsdk_sensor_cfg_init(false);
    SENSOR_cfg_load(ISP_CFG_TMP_INI);
    SENSOR_denoise_3d_strength(image->denoise3d.denoise3dStrength);
    SENSOR_WB_mode_set(image->awbMode);
    SENSOR_sharpen_set(image->manualSharpness.sharpnessLevel, image->manualSharpness.enabled);
    SENSOR_ircut_control_mode_set(image->irCutFilter.irCutControlMode);
    SENSOR_ircut_mode_set(image->irCutFilter.irCutMode);
    SENSOR_lowlight_enable(image->lowlightMode);
    netsdk_sensor_wdr_reset(image->wdr.enabled);
    SENSOR_WDR_strength_set(image->wdr.WDRStrength);
    //enc mode:fisheye
    sdk_enc->enc_mode(0, 0, fixmode_netsdk2sdk(image->videoMode.fixType), showmode_netsdk2sdk(image->videoMode.showMode));
    return 0;
}

int netsdk_image_scene_mode_set(LP_NSDK_IMAGE image)
{
    SENSOR_scene_mode_set(image->sceneMode);
    return 0;

}


static void netsdk_system_operation_delay_set(int hash, void *operation)
{
	int opt = *((int *)operation);
	switch (opt){
		
		case eNSDK_SYSTEM_OPERATION_REBOOT:
		{
			APP_TRACE("NETSDK_SYSTEM REBOOT!");
			GLOBAL_reboot_system();
		}
		break;
		case eNSDK_SYSTEM_OPERATION_DEFAULT:
		{
			APP_TRACE("NETSDK_SYSTEM DEFAULT FACTORY!");
			GLOBAL_remove_conf_file();
            WPA_resetWifiConnectedFlag();
			exit(0);
		}
		break;
		case eNSDK_SYSTEM_OPERATION_REMOTE_UPGRADE:
		{
			APP_TRACE("NETSDK_SYSTEM REMOTE UPGRADE!");
			TICKER_del_task2(netsdk_system_operation_delay_set, 1);//FIX ME
			REMOTE_UPGRADE_start();
		}
		break;
		default:
			break;
	}
	TICKER_del_task2(netsdk_system_operation_delay_set, 1); // only run once!
}


static int netsdk_system_operation(EM_NSDK_SYSTEM_OPERATION operation)
{
	TICKER_del_task2(netsdk_system_operation_delay_set, 1);
	TICKER_add_task2(netsdk_system_operation_delay_set, 2, false, 1, 1, (void *)(&operation), sizeof(EM_NSDK_SYSTEM_OPERATION));
	return 0;
}

static void netsdk_network_check_wifi_Apssid()
{
	ST_NSDK_NETWORK_INTERFACE n_interface;
	char sn[32], essid[64];
	bool model_exist = false;
	model_exist = APP_WIFI_model_exist();
	if(model_exist){
		NETSDK_conf_interface_get(4, &n_interface);
		if(!strcmp(n_interface.wireless.wirelessApMode.wirelessEssId, "hotspot")){
			if(0 == UC_SNumberGet(sn)) {
				snprintf(essid, sizeof(essid), "IPC%s", sn);
				strcpy(n_interface.wireless.wirelessApMode.wirelessEssId, essid);
				NETSDK_conf_interface_set(4, &n_interface, eNSDK_CONF_SAVE_JUST_SAVE);
			}
		}
	}
}

//AF param structure
typedef struct _sensor_net_af_param
{
	int sock;
	HI_S32 fd;
	HI_S32 Cmd;
	HI_U32 u32Fv;
	void *pStepMotor;
	ISP_AF_INFO_S *pstAfInfo;
	int failed_cnt;
}stSensorNetAfParam, *LPSensorNetAfParam;

static int DelayCount = 0;
static bool AF_SEND_FLAG = true;

//af callback for netsdk getting
int netsdk_af_callback(int focusMetries1,int focusMetries2, int focusMetries3, int* param)
{
#if defined(STEPER_AF)
	HI_U32 u32Fv;
	enAUTO_FOCUS_CMD ReCmd;
	TJA_PanTilt *pStepMotor = ((LPSensorNetAfParam)param)->pStepMotor;
	ISP_AF_INFO_S *pstAfInfo = ((LPSensorNetAfParam)param)->pstAfInfo;

	if (pStepMotor->goTargetPulse == JA_True
		&& pStepMotor->goTarget == JA_False) 
	{
		//go target
		pStepMotor->goTarget = JA_True;
		pStepMotor->InitTarget(JA_True, JA_True, ZOOM_SPEED);
		pStepMotor->goTargetPulse = JA_False;
	}

	HI_S32 s32Ret = g_stJaAutoFocus.auto_focus(pStepMotor, &ReCmd, &u32Fv, pstAfInfo);
	if (HI_SUCCESS != s32Ret)
	{
		printf("auto_focus error!(s32Ret = 0x%x)\n", s32Ret);
		return -1;
	}
	else if(ReCmd < FOCUS_CMD_CNT)
	{	
		if(pStepMotor->isInit == JA_True
			|| pStepMotor->goTarget == JA_True)
		{
			return 0;
		}
		
		switch(ReCmd)
		{
			case FOCUS_CMD_IN:
				pStepMotor->tilt(JA_True, JA_True, JA_False, g_stJaAutoFocus.MotorSpeed);
				break;
				
			case FOCUS_CMD_OUT:
				pStepMotor->tilt(JA_True, JA_False, JA_False, g_stJaAutoFocus.MotorSpeed);
				break;
				
			case FOCUS_CMD_STOP:
				pStepMotor->stopTilt();
				break;

			case FOCUS_CMD_INIT:
				pStepMotor->Init(JA_True, JA_False, ZOOM_SPEED);
				break;

			case FOCUS_CMD_GOTO:
				pStepMotor->tiltTo(JA_True, pStepMotor->TiltStepBackup, JA_True, ZOOM_SPEED);
				pStepMotor->FocusGoTo = JA_False;
				break;

			default:
				break;
		}
	}
#endif	

#if defined(UART_PROTOCOL)
#include "uart_protocol.h"



	ST_NSDK_PTZ_CFG stPtz;
	memset(&stPtz, 0, sizeof(ST_NSDK_PTZ_CFG));
	NETSDK_conf_ptz_ch_get(&stPtz);

	if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"BEISIDE")){//è´æ–¯å¾—å‘é€AFå˜ç„¦æ•°æ®æ¥å£
		//printf("**%s %d   BEISIDE***\n",__FUNCTION__,__LINE__);
		if(DelayCount >= 100){
			DATA data;
			data.sendData.afData[0] = (focusMetries1 / 8) & 0xff;
			data.sendData.afData[1] = ((focusMetries1 / 8) >> 8)& 0xff;
			data.sendData.afData[2] = (focusMetries2 / 4) & 0xff;
			data.sendData.afData[3] = ((focusMetries2 / 4) >> 8)& 0xff;
			data.sendData.agcData = 128;
			data.sendData.color = SENSOR_DAYNIGHT_mode_get();
			data.sendData.baud_or_multiple = 0;
			UART_auto_send(UART_CMD_SEND_DATA_BSD, &data);
		}else{
			DelayCount++;
		}
	}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"HUIXUN")){//æ±‡è®¯è§†é€šæ¥å£
		_af_32Fv1 = focusMetries3;

	}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"RONGTIANSHI")){	
		_af_32Fv1 = focusMetries3;
		if(true == AF_SEND_FLAG){
			DATA data;
			data.sendData.afData[0] = (_af_32Fv1 >> 8) & 0xff ;
			data.sendData.afData[1] = _af_32Fv1 & 0xff;
			data.sendData.color = SENSOR_DAYNIGHT_mode_get();
			UART_auto_send(UART_CMD_SEND_AF_DATA_RONGTIANSHI, &data);
			AF_SEND_FLAG = false;
		}else{
			AF_SEND_FLAG = true;
		}

	}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"BAITEJIA")){
		//FIXME
		_af_32Fv1 = focusMetries1;

	}else{	
		//Default AF data,the same as HUIXUN
		_af_32Fv1 = focusMetries1;

	}

#endif

	return 0;
}

static int netsdk_image_autofocus_status(int sock, bool set_flag)
{
	stSensorAfAttr afAttr;

	if(set_flag){//clear af callback function
		afAttr.param = NULL;
		afAttr.af_callback = NULL;
		SENSOR_set_af_attr(&afAttr);
	}else{//af callback function init
		afAttr.param = calloc(1, sizeof(stSensorNetAfParam));
		((LPSensorNetAfParam)afAttr.param)->sock = sock;
		((LPSensorNetAfParam)afAttr.param)->failed_cnt = 0;
#if defined(STEPER_AF)
		_PstepMotor = ((LPSensorNetAfParam)afAttr.param)->pStepMotor = PanTilt();
#endif
/*
#if defined(UART_PROTOCOL)
extern int APP_UART_protocol_init(void);
		APP_UART_protocol_init();
#endif
*/
		afAttr.af_callback = netsdk_af_callback;
		SENSOR_set_af_attr(&afAttr);
	}
}

static char *remote_upgrade_error_str[] = {
	"none",
	"gethostbyname error",
	"no server response",
	"xml prase error",
	"hardware serial number error",
	"soc error",
	"file md5 error",
	"version limit",
	"oem resource limit",
	"skip_rootfs",
};

static char *remote_upgrade_status_str[] = {
	"idle",
	"prepare",
	"checking network",
	"check update detail",
	"download rootfs file",
	"download resource file",
	"sucess",
	"falied",
};

static int netsdk_system_remote_upgrade_status(char *ret_status, int *rate)
{
	int status = REMOTE_UPGRADE_get_status();
	strcpy(ret_status, remote_upgrade_status_str[status]);
	*rate = REMOTE_UPGRADE_get_rate();
	return 0;
}

static int netsdk_system_remote_upgrade_error(char *ret_error)
{
	int error = REMOTE_UPGRADE_get_errno();
	strcpy(ret_error, remote_upgrade_error_str[error]);
	APP_TRACE("%s/%s", ret_error, remote_upgrade_error_str[error]);
	return 0;
}

int custom_conf_match()
{
    ST_CUSTOM_SETTING custom;
	if(0 == CUSTOM_get(&custom)){
		//system
		ST_NSDK_SYSTEM_SETTING system_setting;
        int systemSetFlag = 0;
		NETSDK_conf_system_get_setting_info(&system_setting);
		if(CUSTOM_check_int_valid(custom.function.promptSoundType)){
			system_setting.promptSound.soundType = custom.function.promptSoundType;
            systemSetFlag = 1;
		}

        // capability set
        if(true == CUSTOM_check_string_valid(custom.function.model))
        {
            snprintf(system_setting.capabilitySet.model, sizeof(system_setting.capabilitySet.model), "%s", custom.function.model);
            systemSetFlag = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.audioInput))
        {
            system_setting.capabilitySet.audioInput = custom.function.audioInput;
            systemSetFlag = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.audioOutput))
        {
            system_setting.capabilitySet.audioOutput = custom.function.audioOutput;
            systemSetFlag = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.lightControl))
        {
            system_setting.capabilitySet.lightControl = custom.function.lightControl;
            systemSetFlag = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.bulbControl))
        {
            system_setting.capabilitySet.bulbControl = custom.function.bulbControl;
            systemSetFlag = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.ptz))
        {
            system_setting.capabilitySet.ptz = custom.function.ptz;
            systemSetFlag = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.sdCard))
        {
            system_setting.capabilitySet.sdCard = custom.function.sdCard;
            systemSetFlag = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.fisheye))
        {
            system_setting.capabilitySet.fisheye = custom.function.fisheye;
            systemSetFlag = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.pirEnabled))
        {
            system_setting.capabilitySet.pir = (custom.function.pirEnabled > 0) ? true : false;
            system_setting.pirManager.pirTrigger = custom.function.pirTrigger;
            systemSetFlag = 1;
        }

        if(true == CUSTOM_check_int_valid(custom.function.customAlarmSound))
        {
            if(0 == custom.function.customAlarmSound)
            {
                snprintf(system_setting.mdAlarm.WarningToneType, sizeof(system_setting.mdAlarm.WarningToneType), "default");
            }
            else
            {
                snprintf(system_setting.mdAlarm.WarningToneType, sizeof(system_setting.mdAlarm.WarningToneType), "custom");
            }

            systemSetFlag = 1;
        }

        if(1 == systemSetFlag)
        {
            NETSDK_conf_system_set_setting_info(&system_setting);
        }

        ST_NSDK_SYSTEM_REC_MANAGER recManager;
        NETSDK_conf_system_get_record_info(&recManager);
        if(true == CUSTOM_check_int_valid(custom.function.pirEnabled))
        {
            recManager.useIOAlarm = (custom.function.pirEnabled > 0) ? true : false;
            NETSDK_conf_system_set_record_info(&recManager);
        }
		
		//image
		ST_NSDK_IMAGE image;
		int image_set_flag = 0;
		NETSDK_conf_image_get(&image);
		if(CUSTOM_check_int_valid(custom.function.irCutControlMode)){
			image.irCutFilter.irCutControlMode = custom.function.irCutControlMode;
			image_set_flag = 1;
		}

		int tmpFixMode = 0;
		if((tmpFixMode = FISHEYE_get_fix_mode()) >= 0) {
			image.videoMode.fixType = tmpFixMode;
			image_set_flag = 1;
		}
		if(CUSTOM_check_int_valid(custom.function.imageStyle)){
			image.imageStyle = custom.function.imageStyle;
			image_set_flag = 1;
		}
        if(true == CUSTOM_check_int_valid(custom.function.pirEnabled))
        {
            if(0 < custom.function.pirEnabled)
            {
                image.irCutFilter.irCutMode = kNSDK_IMAGE_IRCUT_MODE_AUTO;
                image_set_flag = 1;
            }
        }
		if(image_set_flag){
			NETSDK_conf_image_set(&image);
		}

        //ptz inline  LP_NSDK_PTZ_CFG NETSDK_conf_ptz_ch_set(LP_NSDK_PTZ_CFG pst_PtzConfig)
        
        ST_NSDK_PTZ_CFG stPtz = {0};
        int ptz_set_config = 0;
        
        NETSDK_conf_ptz_ch_get(&stPtz);
        if(CUSTOM_check_int_valid(custom.st_ptzAttr.nAddress)){
			stPtz.stPtzExternalConfig.nAddress= custom.st_ptzAttr.nAddress;
			ptz_set_config = 1;
		}
        if(CUSTOM_check_int_valid(custom.st_ptzAttr.nBaudRate)){
			stPtz.stPtzExternalConfig.nBaudRate= custom.st_ptzAttr.nBaudRate;
			ptz_set_config = 1;
		}
        if(CUSTOM_check_int_valid(custom.st_ptzAttr.nDateBit)){
			stPtz.stPtzExternalConfig.nDateBit= custom.st_ptzAttr.nDateBit;
			ptz_set_config = 1;
		}
        if(CUSTOM_check_int_valid(custom.st_ptzAttr.nStopBit)){
			stPtz.stPtzExternalConfig.nStopBit = custom.st_ptzAttr.nStopBit;
			ptz_set_config = 1;
		}
	
		if(CUSTOM_check_string_valid(custom.st_ptzAttr.strPtzCustomTpye)){	
			strncpy(stPtz.stPtzExternalConfig.strptzCustomTpye, custom.st_ptzAttr.strPtzCustomTpye,sizeof(custom.st_ptzAttr.strPtzCustomTpye));
			ptz_set_config = 1;
		}

        if(ptz_set_config){
			NETSDK_conf_ptz_ch_set(&stPtz);
#if defined(UART_PROTOCOL)
			UART_protocol_reconfig(stPtz.stPtzExternalConfig.nBaudRate,stPtz.stPtzExternalConfig.nDateBit,stPtz.stPtzExternalConfig.strParity,stPtz.stPtzExternalConfig.nStopBit);
#endif
		}
        
        
        
		
		//network
		ST_NSDK_NETWORK_INTERFACE lan;
		NETSDK_conf_interface_get(1, &lan);
		if(CUSTOM_check_int_valid(custom.function.ipAdapted)){
			NETSDK_conf_interface_set(1, &lan, eNSDK_CONF_SAVE_JUST_SAVE);
		}

		//audio
		ST_NSDK_AIN_CH ain;
		NETSDK_conf_ain_ch_get(1, &ain);
		if(CUSTOM_check_int_valid(custom.function.audioInputVolume)){
			NETSDK_conf_ain_ch_set(1, &ain);
		}

        // video
        ST_NSDK_VIN_CH vin_ch;
        int vin_set_config = 0;
        NETSDK_conf_vin_ch_get(1, &vin_ch);
        if(CUSTOM_check_int_valid(custom.function.powerLineFrequencyMode)) {
            vin_ch.powerLineFrequencyMode = custom.function.powerLineFrequencyMode;
            vin_set_config = 1;
        }
//#if defined(BSD_CUSTOM)
        if(true == CUSTOM_check_int_valid(custom.function.flipEnabled))
        {
            vin_ch.flip = custom.function.flipEnabled;
            vin_set_config = 1;
        }
        if(true == CUSTOM_check_int_valid(custom.function.mirrorEnabled))
        {
            vin_ch.mirror = custom.function.mirrorEnabled;
            vin_set_config = 1;
        }
//#endif
        if(1 == vin_set_config)
        {
            NETSDK_conf_vin_ch_set(1, &vin_ch);
        }
	}
	return 0;
}

/**
 * æ¯”å¯¹ä¼ å…¥çš„ä¸¤ä¸ªå€¼ï¼ŒåŸå€¼ä¸ç›®æ ‡å€¼ä¸ä¸€æ ·åˆ™æŠŠåŸå€¼æ”¹å˜ä¸ºç›®æ ‡å€¼ï¼Œå¹¶è¿”å›ä¿®æ”¹æ ‡å¿—
 * @param src åŸå€¼ï¼Œä¸€æ ·åˆ™ä¸ä¿®æ”¹åŸå€¼ï¼Œä¸ä¸€æ ·è·Ÿç›®æ ‡å€¼åŒæ­¥
 * @param dst ç›®æ ‡å€¼
 * @param setFlag 1è¡¨ç¤ºåŸå€¼è¢«ä¿®æ”¹
 */
static void netsdk_diff_int_match(int *src, int dst, int *setFlag)
{
    if((NULL == src) || (NULL == setFlag))
    {
        return;
    }

    if(*src != dst)
    {
        *src = dst;
        *setFlag = 1;
    }

}

/**
 *  é€šè¿‡æŠŠåŸæœ‰çš„é…ç½®å€¼ç»“æ„ä½“ä¼ å…¥æ­¤æ¥å£æ¥æ ¹æ®tfå¡è®¾ç½®å€¼ä»è€Œæ›´æ–°åŸæœ‰èƒ½åŠ›é›†é…ç½®
 *  å‡å¦‚tfå¡ä¸­æ²¡æœ‰æ›´æ–°ï¼Œåˆ™å¯¹åº”çš„èƒ½åŠ›é›†å­—æ®µæŒ‰ç…§åŸæ ·è¿”å›
 *  @return 0è¡¨ç¤ºä¸éœ€è¦ä¿å­˜è®¾ç½®|1è¡¨ç¤ºéœ€è¦ä¿å­˜è®¾ç½®|-1è¡¨ç¤ºå¤±è´¥
 */
static int netsdk_capability_tfcard_get(LP_NSDK_SYSTEM_SETTING systemInfo, LP_CUSTOM_SETTING custom)
{
    ST_PRODUCT_TEST_INFO productInfo;
    int setFlag = 0;

    if((NULL == custom) || (NULL == systemInfo)
        || (0 != PRODUCT_TEST_getCapability(&productInfo)))
    {
        return -1;
    }

    if(0 < strlen(productInfo.model))
    {
        snprintf(systemInfo->capabilitySet.model, sizeof(systemInfo->capabilitySet.model), "%s", productInfo.model);
        snprintf(custom->function.model, sizeof(custom->model), "%s", productInfo.model);
        setFlag = 1;
    }

    if(-1 != productInfo.audioInput)
    {
        systemInfo->capabilitySet.audioInput = (productInfo.audioInput > 0) ? true : false;
        custom->function.audioInput = productInfo.audioInput;
        setFlag = 1;
    }

    if(-1 != productInfo.audioOutput)
    {
        systemInfo->capabilitySet.audioOutput = (productInfo.audioOutput > 0) ? true : false;
        custom->function.audioOutput = productInfo.audioOutput;
        setFlag = 1;
    }

    switch(productInfo.lightControl)
    {
        case EN_NSDK_LIGHT_CTL_TYPE_NONE:
            systemInfo->capabilitySet.lightControl = EN_NSDK_LIGHT_CTL_TYPE_NONE;
            custom->function.lightControl = EN_NSDK_LIGHT_CTL_TYPE_NONE;
            setFlag = 1;
        break;
        case EN_NSDK_LIGHT_CTL_TYPE_DOUBLE:
            systemInfo->capabilitySet.lightControl = EN_NSDK_LIGHT_CTL_TYPE_DOUBLE;
            custom->function.lightControl = EN_NSDK_LIGHT_CTL_TYPE_DOUBLE;
            setFlag = 1;
        break;
        case EN_NSDK_LIGHT_CTL_TYPE_WARMTH:
            systemInfo->capabilitySet.lightControl = EN_NSDK_LIGHT_CTL_TYPE_WARMTH;
            custom->function.lightControl = EN_NSDK_LIGHT_CTL_TYPE_WARMTH;
            setFlag = 1;
        break;
        default:
        break;
    }

    switch(productInfo.bulbControl)
    {
        case EN_NSDK_BULB_CTL_NONE:
            systemInfo->capabilitySet.bulbControl = EN_NSDK_BULB_CTL_NONE;
            custom->function.bulbControl = EN_NSDK_BULB_CTL_NONE;
            setFlag = 1;
        break;
        case EN_NSDK_BULB_CTL_SGL:
            systemInfo->capabilitySet.bulbControl = EN_NSDK_BULB_CTL_SGL;
            custom->function.bulbControl = EN_NSDK_BULB_CTL_SGL;
            setFlag = 1;
        break;
        case EN_NSDK_BULB_CTL_SGL_PWM:
            systemInfo->capabilitySet.bulbControl = EN_NSDK_BULB_CTL_SGL_PWM;
            custom->function.bulbControl = EN_NSDK_BULB_CTL_SGL_PWM;
            setFlag = 1;
        break;
        case EN_NSDK_BULB_CTL_DBL:
            systemInfo->capabilitySet.bulbControl = EN_NSDK_BULB_CTL_DBL;
            custom->function.bulbControl = EN_NSDK_BULB_CTL_DBL;
            setFlag = 1;
        break;
        case EN_NSDK_BULB_CTL_DBL_PWM:
            systemInfo->capabilitySet.bulbControl = EN_NSDK_BULB_CTL_DBL_PWM;
            custom->function.bulbControl = EN_NSDK_BULB_CTL_DBL_PWM;
            setFlag = 1;
        break;
        default:
        break;
    }

    if(-1 != productInfo.ptz)
    {
        systemInfo->capabilitySet.ptz = (productInfo.ptz > 0) ? true : false;
        custom->function.ptz = productInfo.ptz;
        setFlag = 1;
    }
    if(-1 != productInfo.sdCard)
    {
        systemInfo->capabilitySet.sdCard = (productInfo.sdCard > 0) ? true : false;
        custom->function.sdCard = productInfo.sdCard;
        setFlag = 1;
    }
    switch(productInfo.fisheye)
    {
        case EN_NSDK_FISHEYE_LENS_TYPE_NONE:
            systemInfo->capabilitySet.fisheye = EN_NSDK_FISHEYE_LENS_TYPE_NONE;
            custom->function.fisheye = EN_NSDK_FISHEYE_LENS_TYPE_NONE;
            setFlag = 1;
        break;
        case EN_NSDK_FISHEYE_LENS_TYPE_180:
            systemInfo->capabilitySet.fisheye = EN_NSDK_FISHEYE_LENS_TYPE_180;
            custom->function.fisheye = EN_NSDK_FISHEYE_LENS_TYPE_180;
            setFlag = 1;
        break;
        case EN_NSDK_FISHEYE_LENS_TYPE_360:
            systemInfo->capabilitySet.fisheye = EN_NSDK_FISHEYE_LENS_TYPE_360;
            custom->function.fisheye = EN_NSDK_FISHEYE_LENS_TYPE_360;
            setFlag = 1;
        break;
        case EN_NSDK_FISHEYE_LENS_TYPE_720:
            systemInfo->capabilitySet.fisheye = EN_NSDK_FISHEYE_LENS_TYPE_720;
            custom->function.fisheye = EN_NSDK_FISHEYE_LENS_TYPE_720;
            setFlag = 1;
        break;
        default:
        break;
    }

    return setFlag;

}

/**
 * å…ˆæ ¹æ®è®¾å¤‡æ£€æµ‹å†³å®šè®¾å¤‡çš„èƒ½åŠ›é›†ï¼Œå†æ ¹æ®tfå¡å®šåˆ¶è®¾ç½®èƒ½åŠ›é›†
 */
static bool netsdk_capability_device_self(LP_NSDK_SYSTEM_SETTING systemInfo)
{
    ST_MODEL_CONF modelConf;
    ST_CUSTOM_SETTING custom;
    int customGet = -1;
    EM_NSDK_IMAGE_FISHEYE_FIX_MODE fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE;
    bool tmpFlag;
    int setFlag = 0;

    if(NULL == systemInfo)
    {
        return false;
    }

    customGet = CUSTOM_get(&custom);
    if((0 == customGet && CUSTOM_check_string_valid(custom.function.model)))
    {
        if(strncmp(systemInfo->capabilitySet.model, custom.function.model, strlen(custom.function.model)))
        {
            snprintf(systemInfo->capabilitySet.model, sizeof(systemInfo->capabilitySet.model), "%s", custom.function.model);
            setFlag = 1;
        }
    }
    else if((NULL != MODEL_CONF_get(&modelConf))
        && strncmp(systemInfo->capabilitySet.model, modelConf.modelName, strlen(modelConf.modelName)))
    {
        snprintf(systemInfo->capabilitySet.model, sizeof(systemInfo->capabilitySet.model), "%s", modelConf.modelName);
        setFlag = 1;
    }

#if defined(BLUETOOTH)
    tmpFlag = APP_BLUETOOTH_is_support();
#else
    tmpFlag = false;
#endif
    if(tmpFlag != systemInfo->capabilitySet.bluetooth)
    {
        systemInfo->capabilitySet.bluetooth = tmpFlag;
        setFlag = 1;
    }

#if defined(WIFI)
    tmpFlag = APP_WIFI_model_exist();
#else
    tmpFlag = false;
#endif
    if((tmpFlag != systemInfo->capabilitySet.wifi)
        || (tmpFlag != systemInfo->capabilitySet.wifiStationCanSet))
    {
        systemInfo->capabilitySet.wifi = tmpFlag;
        systemInfo->capabilitySet.wifiStationCanSet = tmpFlag;
        setFlag = 1;
    }

    tmpFlag = network_check_interface();
    if(tmpFlag != systemInfo->capabilitySet.rj45)
    {
        systemInfo->capabilitySet.rj45 = tmpFlag;
        setFlag = 1;
    }

    if(0 == customGet && CUSTOM_check_int_valid(custom.function.fisheye))
    {
        netsdk_diff_int_match(&systemInfo->capabilitySet.fisheye, custom.function.fisheye, &setFlag);
    }
    else
    {
        fixMode = FISHEYE_get_fix_mode();
        if(eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL == fixMode)
        {
            netsdk_diff_int_match(&systemInfo->capabilitySet.fisheye, EN_NSDK_FISHEYE_LENS_TYPE_180, &setFlag);
        }
        else if(eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL == fixMode)
        {
            netsdk_diff_int_match(&systemInfo->capabilitySet.fisheye, EN_NSDK_FISHEYE_LENS_TYPE_360, &setFlag);
        }
        else if(eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE == fixMode)
        {
            netsdk_diff_int_match(&systemInfo->capabilitySet.fisheye, EN_NSDK_FISHEYE_LENS_TYPE_180, &setFlag);
        }
        else
        {
            netsdk_diff_int_match(&systemInfo->capabilitySet.fisheye, EN_NSDK_FISHEYE_LENS_TYPE_NONE, &setFlag);
        }
    }

    // 16ev100+imx307é»˜è®¤è®¾ç½®ä¸ºé»‘å…‰
#if defined(HI3516E_V1)
    char sensor_name[16] = {0};

    if(0 == customGet && CUSTOM_check_int_valid(custom.function.lightControl))
    {
        netsdk_diff_int_match(&systemInfo->capabilitySet.lightControl, custom.function.lightControl, &setFlag);
    }
    else if(SENSOR_MODEL_IMX307 == SENSOR_get_sensor_model(sensor_name))
    {

        // æš–å…‰é»‘å¤œå…¨å½©æªæœºç¯æ§
        netsdk_diff_int_match(&systemInfo->capabilitySet.lightControl, EN_NSDK_LIGHT_CTL_TYPE_WARMTH, &setFlag);
    }
#endif

    // 4g(lte)å’Œrtcæœªåšæ£€æµ‹åˆ¤æ–­


    return (setFlag > 0) ? true : false;

}

static int netsdk_capability_init()
{
    ST_NSDK_SYSTEM_SETTING systemInfo;
    ST_NSDK_SYSTEM_CAPABILITY_SET capabilitySet;
    ST_CUSTOM_SETTING custom;

    if(NULL == NETSDK_conf_system_get_setting_info(&systemInfo))
    {
        return -1;
    }

    /**
     * å…ˆæ ¹æ®è®¾å¤‡æ£€æµ‹å†³å®šè®¾å¤‡çš„èƒ½åŠ›é›†ï¼Œå†æ ¹æ®äº§æµ‹æˆ–è€…tfå¡å®šåˆ¶è®¾ç½®èƒ½åŠ›é›†
     */
    if(true == netsdk_capability_device_self(&systemInfo))
    {
        NETSDK_conf_system_set_setting_info(&systemInfo);
    }

    /**
     * æ ¹æ®tfå¡å®šåˆ¶èƒ½åŠ›é›†ï¼ŒåŒæ—¶ä¿å­˜åˆ°äº§æµ‹æ–‡ä»¶custom.confæ–‡ä»¶ä¸­
     */
    CUSTOM_get(&custom);
    if(0 < netsdk_capability_tfcard_get(&systemInfo, &custom))
    {
        NETSDK_conf_system_set_setting_info(&systemInfo);
        CUSTOM_set(&custom);
    }

    return 0;

}

int APP_NETSDK_init(void)
{
	int reset_default = 0;
	ST_NSDK_INIT netsdk_init;
	uint32_t chip_id;
	if(!sdk_sys){
		APP_TRACE("sdk_sys not init!");
		return -1;
	}
	sdk_sys->get_chip_id(&chip_id);
	char str[128];
	sprintf(str, "%x", chip_id);
	setenv("SOC", str , true);
	// configuration folder directory

#if defined (PX) | defined (CX)
    if(NK_False == GLOBAL_sn_front()){
		sprintf(str, "%s_cx", getenv("DEFNETSDK"));
	}
    else {
        sprintf(str, "%s_px", getenv("DEFNETSDK"));
    }
#else
	sprintf(str, "%s_%x", getenv("DEFNETSDK"), chip_id);
#endif

	strcpy(netsdk_init.confDirectory, getenv("NETSDK"));	// FIXME:
	strcpy(netsdk_init.defConfDirectory, str); // FIXME:
	// check folder existed
	if(!CHECK_DIR_EXIST(netsdk_init.confDirectory)){
		// FIXME:
		char shell_cmd[128] = {""};
		sprintf(shell_cmd, "cp -Rf %s %s", netsdk_init.defConfDirectory, netsdk_init.confDirectory);
		NK_SYSTEM(shell_cmd);
	}
	// callback function
	netsdk_init.systemDeviceInfo = netsdk_system_device_info;
	netsdk_init.systemOperation = netsdk_system_operation;
	netsdk_init.videoInputChannelChanged = netsdk_vin_ch_set;
	netsdk_init.audioInputChannelChanged = netsdk_ain_ch_set;
	netsdk_init.motionDetectionChannelChanged = netsdk_md_ch_set;
	netsdk_init.motionDetectionChannelStatus = netsdk_md_status;
	netsdk_init.videoEncodeChannelChanged = netsdk_venc_ch_changed;
	netsdk_init.audioEncodeChannelChanged = netsdk_aenc_ch_set;
	netsdk_init.videoEncodeRequestKeyFrame = netsdk_venc_request_keyframe;
	netsdk_init.videoEncodeSnapShot = netsdk_venc_snapshot;
	netsdk_init.systemChanged = netsdk_system_changed;
	netsdk_init.systemDSTChanged = netsdk_system_DST_changed;
	netsdk_init.networkWirelessStatus = netsdk_network_wireless_status;
	netsdk_init.imageChanged = netsdk_image_changed;
	netsdk_init.AutoFocusStatus = netsdk_image_autofocus_status;
	netsdk_init.remoteUpgradeStatus = netsdk_system_remote_upgrade_status;
	netsdk_init.remoteUpgradeError = netsdk_system_remote_upgrade_error;

	netsdk_init.alarmInputChannelPortStatus = netsdk_alarm_in_port_status;
	netsdk_init.alarmOutputChannelPortStatus = netsdk_alarm_out_port_status;
	netsdk_init.alarmOutputChannelTrigger = netsdk_alarm_out_trigger;
	netsdk_init.ptzUartConfigChanged = netsdk_ptz_config_changed;
	// callback save
	netsdk_init.video_conf_save = netsdk_video_conf_save;
	netsdk_init.audio_conf_save = netsdk_audio_conf_save;
	netsdk_init.system_conf_save = netsdk_system_conf_save;
	netsdk_init.network_conf_save = netsdk_network_conf_save;
	netsdk_init.io_conf_save = NULL;
	netsdk_init.image_conf_save = netsdk_image_conf_save;
	// initialize NETSDK module
	NETSDK_init(&netsdk_init);

	if(netsdk_fix_model_conf_param()){
		//custom OEM conf setting
		custom_conf_match();
	}

	/* ´Ë´¦Ôö¼ÓÒ»¸öÂß¼­£¬Ö»ÓÃÓÚÍ³Ò»ÁËIRCUTÇĞ»»·½Ê½ÎªÄ¬ÈÏÈíÇĞ°æ±¾Ö®Ç°µÄ°æ±¾¡£
	ÒòÎªP2/P4³ö»õÃ»ÓĞ¶¨ÖÆIRCUTÇĞ»»·½Ê½£¬Ê¹ÓÃ¹Ì¼şÄ¬ÈÏÓ²ÇĞÇĞ»»·½Ê½£¬¼ÙÈçºóÃæ¹Ì¼ş¸ÄÎªÄ¬ÈÏÈíÇĞ£¬
	P2/P4Éè±¸»Ö¸´³ö³§ÉèÖÃºó£¬»áµ¼ÖÂIRCUTÇĞ»»ÓÉÓ²ÇĞ±äÎªÈíÇĞ(Ö»Éı¼¶²»»Ö¸´³ö³§ÉèÖÃ²»»á¸Ä±äÔ­ÉèÖÃµÄIRCUTÇĞ»»·½Ê½) */
	/* ¸ÄÎªÄ¬ÈÏÈíÇĞµÄ°æ±¾Î´·¢²¼£¬ËùÒÔºóÃæÔÙÔö¼Ó°æ±¾ÅĞ¶Ï P6µÄÅĞ¶ÏÏÈÆÁ±Î */
	//if((SWVER_MAJ == 1) && (SWVER_MIN <= 9) && (SWVER_REV <= 13)) {
		ST_MODEL_CONF model_conf;
		if(NULL != MODEL_CONF_get(&model_conf)) {
			if((strncmp(model_conf.modelName, "P2", 2) == 0) 
				|| (strncmp(model_conf.modelName, "P4", 2) == 0)) {
				//|| (strncmp(model_conf.modelName, "P6", 2) == 0)) {
                ST_NSDK_IMAGE image;
                ST_CUSTOM_SETTING custom;
                if(0 == CUSTOM_get(&custom)) {
                    if(CUSTOM_check_int_valid(custom.function.irCutControlMode) == false) {
                        NETSDK_conf_image_get(&image);
                        image.irCutFilter.irCutControlMode = kNSDK_IMAGE_IRCUT_CONTROL_MODE_HARDWARE;
                        custom.function.irCutControlMode = image.irCutFilter.irCutControlMode;
                        CUSTOM_set(&custom);
                        NETSDK_conf_image_set(&image);
                    }
                }
			}
		}
	//}
	//do some different default setting between different sensor Here

	netsdk_network_check_wifi_Apssid();

	//isp cfg ini file init
	netsdk_sensor_cfg_init(true);

	netsdk_capability_init();

	pthread_mutex_init(&osd_mutex, NULL);
	return 0;
}

void APP_NETSDK_destroy()
{
	pthread_mutex_destroy(&osd_mutex);
	NETSDK_destroy();
}

// end with NULL
static const char *netsdk_uri(char *uri_buf, ...)
{
	va_list arg_list;
	const char *sub_uri = NULL;

	// add the netsdk prefix
	strcpy(uri_buf, "/NetSDK");
	va_start(arg_list, uri_buf);
	while(NULL != (sub_uri = va_arg(arg_list, const char *))){
		if('/' != sub_uri[0]){
			strcat(uri_buf, "/");
		}
		strcat(uri_buf, sub_uri);
	}
	va_end(arg_list);

	//APP_TRACE("NetSDK URI: %s", uri_buf);
	return uri_buf;
}

int APP_NETSDK_http_init()
{
	int i = 0, ii = 0, iii = 0;
	char uri[1024] = {""}, uri_fmt[1024] = {""};
	int const n_vin_ch = 1, n_ain_ch = 1;
	int const n_privacy_msk_rgn = 4;
	int const n_venc_ch = 3, n_aenc_ch = 1;
	int const n_venc_text_overlay = 2;
	int const n_alarm_input = 1;
	int const n_alarm_output = 1;
	int const n_ptz_ch = 3;
	int const n_stream_ch;
	int const n_network_interface = 4;
	int const n_network_port = 4;

	//network
	WEBS_add_cgi(netsdk_uri(uri,"Network","Interfaces",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Network","Interfaces","properties",NULL),NETSDK_http_service, kH_METH_GET);
	for(i = 0; i < n_network_interface; i++){
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt,NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","lan",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","lan","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","upnp",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","pppoe",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","pppoe","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","ddns",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","ddns","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","Wireless",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","Wireless_B64En",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Interface","%d","Wireless","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Wireless","status",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Wireless","stationSignal",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		/*snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","wireless","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);*/
	}
	WEBS_add_cgi(netsdk_uri(uri,"Network","Ports",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Network","Ports","properties",NULL),NETSDK_http_service, kH_METH_GET);
	for(i = 0; i < n_network_port; i++){
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Port","%d",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Network","Port","%d","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_GET);
	}
	WEBS_add_cgi(netsdk_uri(uri,"Network","Dns",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Network","Dns","properties",NULL),NETSDK_http_service, kH_METH_GET);

	WEBS_add_cgi(netsdk_uri(uri,"Network","Esee",NULL), NETSDK_http_service, kH_METH_GET);
	//WEBS_add_cgi(netsdk_uri(uri,"Network","Esee","properties",NULL),NETSDK_http_service, kH_METH_GET);

	
	//system
	WEBS_add_cgi(netsdk_uri(uri,"System","time","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","localTime",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","localTime","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","timeZone",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","timeZone","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","daylightSavingTime",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","daylightSavingTime","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","calendarStyle",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","calendarStyle","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","ntp",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","ntp","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","properties", NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","deviceName",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","deviceAddress",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","model",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","serialNumber",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","macAddress",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","firmwareVersion",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","firmwareReleaseDate",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","hardwareVersion",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","deviceInfo","productCode",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","operation","reboot",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","operation","default",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","operation","remoteUpgrade",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","operation","remoteUpgradeInfo","status",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","operation","remoteUpgradeInfo","error",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","MdAlarm","MotionWarningTone",NULL), NETSDK_http_service, kH_METH_GET);

	WEBS_add_cgi(netsdk_uri(uri,"System","time","rtc",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"System","time","rtc","properties",NULL), NETSDK_http_service, kH_METH_GET);

    WEBS_add_cgi(netsdk_uri(uri,"System","module","gsensor",NULL), NETSDK_http_service, kH_METH_ALL);
    WEBS_add_cgi(netsdk_uri(uri,"System","module","gsensor","properties",NULL), NETSDK_http_service, kH_METH_GET);
    WEBS_add_cgi(netsdk_uri(uri,"System","module","gsensor","calibration",NULL), NETSDK_http_service, kH_METH_PUT);

    WEBS_add_cgi(netsdk_uri(uri,"System","module","bluetooth",NULL), NETSDK_http_service, kH_METH_ALL);
    WEBS_add_cgi(netsdk_uri(uri,"System","module","bluetooth","properties",NULL), NETSDK_http_service, kH_METH_GET);
    WEBS_add_cgi(netsdk_uri(uri,"System","module","bluetooth","status",NULL), NETSDK_http_service, kH_METH_GET);

	// video
	WEBS_add_cgi(netsdk_uri(uri,"video","input","channels",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"video","input","channels","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"video","MotionDetection","channels",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"video","MotionDetection","channels","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"video","encode","channels",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"video","encode","channels","properties",NULL), NETSDK_http_service, kH_METH_GET);
	
	for(i = 0; i < n_vin_ch; ++i){
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);

		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","brightnessLevel",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","sharpnessLevel",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","saturationLevel",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","hueLevel",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","contrastLevel",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","flip",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","mirror",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
	
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","privacyMasks",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","input","channel","%d","privacyMasks","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		
		for(ii = 0; ii < n_privacy_msk_rgn; ++ii){
			snprintf(uri_fmt, sizeof(uri_fmt),
				netsdk_uri(uri,"video","input","channel","%d","privacyMask","%d",NULL), i + 1, ii + 1);
			WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
			snprintf(uri_fmt, sizeof(uri_fmt),
				netsdk_uri(uri,"video","input","channel","%d","privacyMask","%d","properties",NULL), i + 1, ii + 1);
			WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		}

		// motion detection
		
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","MotionDetection","channel","%d","status",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","MotionDetection","channel","%d",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","MotionDetection","channel","%d","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		//snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","MotionDetection","channel","%d","detectionGrid",NULL), i + 1);
		//WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		//snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","MotionDetection","channel","%d","detectionRegions",NULL), i + 1);
		//WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		//snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","MotionDetection","channel","%d","detectionRegions","properties",NULL), i + 1);
		//WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		/*
		for(ii = 0; ii < n_privacy_msk_rgn; ++ii){
			snprintf(uri_fmt, sizeof(uri_fmt),
				netsdk_uri(uri,"video","MotionDetection","channel","%d","detectionRegion","%d",NULL), i + 1, ii + 1);
			WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
			snprintf(uri_fmt, sizeof(uri_fmt),
				netsdk_uri(uri,"video","MotionDetection","channel","%d","detectionRegion","%d","properties",NULL), i + 1, ii + 1);
			WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		}*/

		for(ii = 0; ii < n_venc_ch; ++ii){
			int encode_channel_id = (i + 1) * 100 + ii + 1;

			do{
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","properties",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","snapShot",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_PUT | kH_METH_POST);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","requestKeyFrame",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_PUT | kH_METH_POST);

				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","channelNameOverlay",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","channelNameOverlay","properties",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","datetimeOverlay",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","datetimeOverlay","properties",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","deviceIDOverlay",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","deviceIDOverlay","properties",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);

				for(iii = 0; iii < n_venc_text_overlay; ++iii){
					snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","textOverlays",NULL), encode_channel_id);
					WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
					snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","textOverlays","properties",NULL), encode_channel_id);
					WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
					snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","textOverlay","%d",NULL), encode_channel_id, iii + 1);
					WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
					snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"video","encode","channel","%d","textOverlay","%d","properties",NULL), encode_channel_id, iii + 1);
					WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
				}

				// if only one video input channel
				// also support 1,2,3,.. channel id format
			}while(encode_channel_id > 100 && (encode_channel_id %= 100, 1 == n_vin_ch));
		}
			
	}

	//audio
	WEBS_add_cgi(netsdk_uri(uri,"audio","input","channels",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"audio","input","channels","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"audio","encode","channels",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"audio","encode","channels","properties",NULL), NETSDK_http_service, kH_METH_GET);

	for(i = 0; i< n_ain_ch;i++){
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"audio","input","channel","%d",NULL), i+1 );
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"audio","input","channel","%d","properties",NULL), i+1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		
		for(ii = 0; ii < n_aenc_ch; ii++){
			int encode_channel_id = (i + 1) * 100 + ii + 1;
			do{
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"audio","encode","channel","%d",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_PUT | kH_METH_POST);
				snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"audio","encode","channel","%d","properties",NULL), encode_channel_id);
				WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_PUT | kH_METH_POST);
			}while(0);
		}
	}
	// io
	WEBS_add_cgi(netsdk_uri(uri,"IO","alarmInput","channels",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"IO","alarmInput","channels","properties",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"IO","alarmOutput","channels",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"IO","alarmOutput","channels","properties",NULL), NETSDK_http_service, kH_METH_GET);
	for(i = 0; i < n_alarm_input; ++i){
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"IO","alarmInput","channel","%d",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"IO","alarmInput","channel","%d","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"IO","alarmInput","channel","%d","portStatus",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
	}
	for(i = 0; i < n_alarm_input; ++i){
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"IO","alarmOutput","channel","%d",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"IO","alarmOutput","channel","%d","properties",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"IO","alarmOutput","channel","%d","portStatus",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"IO","alarmOutput","channel","%d","trigger",NULL), i + 1);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
	}

	// ptz
	WEBS_add_cgi(netsdk_uri(uri,"PTZ","channels",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"PTZ","channels","properties",NULL), NETSDK_http_service, kH_METH_GET);
	for(i = 0; i < n_ptz_ch; i++){
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"PTZ","channel","%d",NULL), i);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"PTZ","channel","%d","properties",NULL), i);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"PTZ","channel","%d","control",NULL), i);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"PTZ","channel","%d","externalconfig",NULL), i);
		WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
	}

	// network transport stream
	WEBS_add_cgi(netsdk_uri(uri,"Stream","channels",NULL), NETSDK_http_service, kH_METH_GET);
	for(i = 0; i < n_vin_ch; ++i){
		for(ii = 0; ii < n_venc_ch; ++ii){
			int streamID = (i + 1) * 100 + ii + 1;
			snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Stream","channel","%d",NULL), streamID);
			WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
			snprintf(uri_fmt, sizeof(uri_fmt), netsdk_uri(uri,"Stream","channel","%d","transportRTSP",NULL), streamID);
			WEBS_add_cgi(uri_fmt, NETSDK_http_service, kH_METH_ALL);
		}
	}

	// sdcard transport stream
#if defined(SDCARD)|defined(TFCARD)
	WEBS_add_cgi(netsdk_uri(uri,"SDCard","media","search",NULL), NETSDK_sdcard_media_search, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"SDCard","media","playbackFLV",NULL), NETSDK_sdcard_media_playback_flv, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"SDCard","format",NULL), NETSDK_sdcard_format, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"SDCard","status",NULL), NETSDK_sdcard_status, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"SDCard","checkRW",NULL), NETSDK_sdcard_checkRW, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"SDCard","file",NULL), NETSDK_sdcard_file, kH_METH_GET);
#endif
	//image
	WEBS_add_cgi(netsdk_uri(uri,"Image",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image","properties",NULL),NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "irCutFilter",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "irCutFilter", "properties",NULL),NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "manualSharpness",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "manualSharpness", "properties",NULL),NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "denoise3d",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "denoise3d", "properties",NULL),NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "wdr",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "wdr", "properties",NULL),NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "AF",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "videoMode",NULL), NETSDK_http_service, kH_METH_GET);
	WEBS_add_cgi(netsdk_uri(uri,"Image", "videoMode", "properties",NULL),NETSDK_http_service, kH_METH_GET);

	//for factory test
	WEBS_add_cgi(netsdk_uri(uri,"factory",NULL),WEB_CGI_factory_test, kH_METH_ALL);
	WEBS_add_cgi("/custom/OEM",WEB_CGI_factory_setting, kH_METH_ALL);

#ifdef GSENSOR
    WEBS_add_cgi("/gsensor",APP_GSENSOR_web_cgi_get_angles, kH_METH_ALL);
#endif

	return 0;
}

void APP_NETSDK_http_destroy()
{
}



