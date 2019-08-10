#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sysconf.h"
#include "sensor.h"
#include "netsdk.h"
#include "ptz.h"

#include "env_common.h"
#include "generic.h"
#include "app_debug.h"
#include "ifconf.h"
#include "ticker.h"
#include <NkUtils/macro.h>
#define ONVIF_MD_MAX_ROW (64)
#define ONVIF_MD_MAX_COLUM (64)
#define ZW_PROTOCOL_FLAG "zw_protocol_flag"

static int _get_system_information(lpNVP_DEV_INFO info)
{
	ST_NSDK_SYSTEM_DEVICE_INFO info_n;
	
	memset(&info_n, 0, sizeof(info_n));
	NETSDK_conf_system_get_device_info(&info_n);
	
	strcpy(info->manufacturer, "GUANGZHOU");
	strcpy(info->devname, info_n.deviceName);
	strcpy(info->model, info_n.model);
	strcpy(info->sn, info_n.serialNumber);
	strcpy(info->firmware, info_n.firmwareVersion);
	strcpy(info->sw_version, info_n.firmwareVersion);
	strcpy(info->sw_builddate, info_n.firmwareReleaseDate);
	strcpy(info->hw_version, info_n.hardwareVersion);
	strcpy(info->hwid, "HW000");

	return 0;
}

static int _get_date_time(lpNVP_SYS_TIME systime)
{
	time_t t_now;
	struct tm *ptm;
	struct tm tm_local, tm_gm;

	ST_NSDK_SYSTEM_TIME time_n;
	
	memset(&time_n, 0, sizeof(time_n));
	NETSDK_conf_system_get_time(&time_n);

	systime->ntp_enable = time_n.ntpEnabled;
	strncpy(systime->ntp_server, time_n.ntpServerDomain, sizeof(systime->ntp_server) - 1);
	systime->tzone = time_n.greenwichMeanTime;

	time(&t_now);
	localtime_r(&t_now, &tm_local);
	gmtime_r(&t_now, &tm_gm);

	ptm = &tm_local;
	systime->local_time.date.year = ptm->tm_year;
	systime->local_time.date.month = ptm->tm_mon;
	systime->local_time.date.day = ptm->tm_mday;
	systime->local_time.time.hour = ptm->tm_hour;
	systime->local_time.time.minute = ptm->tm_min;
	systime->local_time.time.second = ptm->tm_sec;
		
	ptm = &tm_gm;
	systime->gm_time.date.year = ptm->tm_year;
	systime->gm_time.date.month = ptm->tm_mon;
	systime->gm_time.date.day = ptm->tm_mday;
	systime->gm_time.time.hour = ptm->tm_hour;
	systime->gm_time.time.minute = ptm->tm_min;
	systime->gm_time.time.second = ptm->tm_sec;
	
	return 0;
}

static int _set_date_time(lpNVP_SYS_TIME systime)
{
	time_t t_set;
	struct timeval tv_set;
	struct tm tm_set;
	ST_NSDK_SYSTEM_TIME time_n;
	
	memset(&time_n, 0, sizeof(time_n));
	NETSDK_conf_system_get_time(&time_n);

	time_n.ntpEnabled = systime->ntp_enable;
	strncpy(time_n.ntpServerDomain, systime->ntp_server, sizeof(time_n.ntpServerDomain) - 1);
	time_n.greenwichMeanTime = systime->tzone;

	NETSDK_conf_system_set_time(&time_n);
	
	tm_set.tm_year = systime->gm_time.date.year;
	tm_set.tm_mon = systime->gm_time.date.month;
	tm_set.tm_mday = systime->gm_time.date.day;
	tm_set.tm_hour = systime->gm_time.time.hour;
	tm_set.tm_min = systime->gm_time.time.minute;
	tm_set.tm_sec = systime->gm_time.time.second;
	
	GMT_SET(0);
	t_set = mktime(&tm_set);
	
	tv_set.tv_sec = t_set;
	tv_set.tv_usec = 0;
	settimeofday(&tv_set, NULL);
	GMT_SET(time_n.greenwichMeanTime);
	
	APP_TRACE("set datetime zone:%d", time_n.greenwichMeanTime);
	
	return 0;
}


static int _get_interface(lpNVP_ETHER_CONFIG ether)
{
	ST_NSDK_NETWORK_INTERFACE net_n;
	ST_NSDK_NETWORK_PORT port_n;
	ST_NSDK_SYSTEM_DEVICE_INFO sysinfo;
	ifconf_interface_t irf;
	char *ether_iface = getenv("DEF_ETHER");
	int ether_iface_index = 1;
	
	memset(&net_n, 0, sizeof(net_n));
	memset(&port_n, 0, sizeof(port_n));
	memset(&sysinfo, 0, sizeof(sysinfo));
	if(strcmp(ether_iface, "eth0")){
		ether_iface_index = 4;
	}

	NETSDK_conf_interface_get(ether_iface_index, &net_n);
	NETSDK_conf_port_get(1, &port_n);
	NETSDK_conf_system_get_device_info(&sysinfo);

	ifconf_get_interface(ether_iface, &irf);
	if (net_n.lan.addressingType == kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC) {
		ether->dhcp = false;
		NVP_IP_INIT_FROM_STRING(ether->ip, net_n.lan.staticIP);
		NVP_IP_INIT_FROM_STRING(ether->netmask, net_n.lan.staticNetmask);
		NVP_IP_INIT_FROM_STRING(ether->gateway, net_n.lan.staticGateway);
	} else {
		ether->dhcp = true;
		memcpy(ether->ip, irf.ipaddr.s_b, sizeof(ether->ip));
		memcpy(ether->netmask, irf.netmask.s_b, sizeof(ether->netmask));
		memcpy(ether->gateway, irf.gateway.s_b, sizeof(ether->gateway));
	}
	memcpy(ether->mac, irf.hwaddr.s_b, sizeof(ether->mac));

	NVP_IP_INIT_FROM_STRING(ether->dns1, net_n.dns.staticPreferredDns);
	NVP_IP_INIT_FROM_STRING(ether->dns2, net_n.dns.staticAlternateDns);	

	ether->http_port = port_n.value;
	ether->rtsp_port = 554;

	return 0;
}

static int _set_interface(lpNVP_ETHER_CONFIG ether)
{
	ST_NSDK_NETWORK_INTERFACE net_n;
	ST_NSDK_NETWORK_PORT port_n;
	char *ether_iface = getenv("DEF_ETHER");
	int ether_iface_index = 1;
	bool port_flag = false;

	memset(&net_n, 0, sizeof(net_n));
	memset(&port_n, 0, sizeof(port_n));
	if(strcmp(ether_iface, "eth0")){
		ether_iface_index = 4;
	}

	NETSDK_conf_interface_get(ether_iface_index, &net_n);
	NETSDK_conf_port_get(1, &port_n);

	if (ether->dhcp == true) {
		net_n.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC;
	} else {
		net_n.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC;
		_ip_2string(ether->ip, net_n.lan.staticIP);
		_ip_2string(ether->netmask, net_n.lan.staticNetmask);
		_ip_2string(ether->gateway, net_n.lan.staticGateway);
	}
	
	if(MATCH_GATEWAY(net_n.lan.staticIP, net_n.lan.staticNetmask, net_n.lan.staticGateway) == 0){
		snprintf(net_n.lan.staticGateway, sizeof(net_n.lan.staticGateway), "%d.%d.%d.%d", ether->ip[0] & ether->netmask[0], ether->ip[1] & ether->netmask[1],
			ether->ip[2] & ether->netmask[2], (ether->ip[3] & ether->netmask[3]) | 0x1);
	}
	APP_TRACE("_set_interface %s %s %s %s", ether_iface, net_n.lan.staticIP, net_n.lan.staticNetmask, net_n.lan.staticGateway);

	_ip_2string(ether->dns1, net_n.dns.staticPreferredDns); 	
	_ip_2string(ether->dns2, net_n.dns.staticAlternateDns);
	if (port_n.value != ether->http_port) {
		APP_TRACE("spook port %d -> %d", port_n.value, ether->http_port);
		port_n.value = ether->http_port;
		port_flag = true;
	}

	if (NETSDK_conf_interface_set(ether_iface_index, &net_n, false)){
		if(port_flag){
			//FIXME 
			//port�˿ڲ����ƣ�Ŀǰ�����޸ĵ�port�˿ںŵĲ�����app
			if(NETSDK_conf_port_set(1, &port_n, true)){		
				return 0;
			}
		}else{			
			return 0;
		}
	}
	return -1;
}

static int _get_color(lpNVP_COLOR_CONFIG color, int chn)
{
	ST_NSDK_VIN_CH vin_n;
	ST_NSDK_IMAGE img_n;

	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);

	memset(&img_n, 0, sizeof(img_n));
	NETSDK_conf_image_get(&img_n);


	color->brightness = (float)vin_n.brightnessLevel;
	color->contrast = (float)vin_n.contrastLevel;
	color->hue = (float)vin_n.hueLevel;
	color->saturation = (float)vin_n.saturationLevel;
	color->sharpness = (float)img_n.manualSharpness.sharpnessLevel;
	
	APP_TRACE("color (%d, %d, %d, %d, %d)", vin_n.brightnessLevel, vin_n.contrastLevel, vin_n.hueLevel, vin_n.saturationLevel, vin_n.sharpnessLevel);
	APP_TRACE("color (%f, %f, %f, %f, %f)", color->brightness, color->contrast, color->hue, color->saturation, color->sharpness);
	
	return 0;
}


static int _set_color(lpNVP_COLOR_CONFIG color, int chn)
{
	ST_NSDK_VIN_CH vin_n;
	
	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);

	vin_n.brightnessLevel = color->brightness;
	vin_n.contrastLevel = color->contrast;
	vin_n.hueLevel = color->hue;
	vin_n.saturationLevel = color->saturation;
	vin_n.sharpnessLevel = color->sharpness;

	if (NETSDK_conf_vin_ch_set(chn + 1, &vin_n) ){		
		netsdk_vin_ch_set(chn + 1, &vin_n);
		return 0;
	}else
		return -1;
}

static int _get_image_option(lpNVP_IMAGE_OPTIONS image, int chn)
{
	// FIX me
	image->brightness.min = 0;
	image->brightness.max = 100;
	image->saturation.min = 0;
	image->saturation.max = 100;
	image->contrast.min = 0;
	image->contrast.max = 100;
	image->hue.min = 0;
	image->hue.max = 100;
	image->sharpness.min = 0;
	image->sharpness.max = 255;

	image->ircut_mode.nr = 3;
	image->ircut_mode.list[0] = NVP_IRCUT_MODE_AUTO;
	image->ircut_mode.list[1] = NVP_IRCUT_MODE_DAYLIGHT;
	image->ircut_mode.list[2] = NVP_IRCUT_MODE_NIGHT;

	return 0;
}

static int _get_image(lpNVP_IMAGE_CONFIG image, int chn)
{
	ST_NSDK_IMAGE img_n;

	memset(&img_n, 0, sizeof(img_n));
	NETSDK_conf_image_get(&img_n);

	image->ircut.control_mode = img_n.irCutFilter.irCutControlMode;
	image->ircut.ircut_mode = img_n.irCutFilter.irCutMode;

	image->wdr.enabled = img_n.wdr.enabled;
	image->wdr.WDRStrength = img_n.wdr.WDRStrength;

	image->manual_sharp.enabled = img_n.manualSharpness.enabled;
	image->manual_sharp.sharpnessLevel = img_n.manualSharpness.sharpnessLevel;

	image->d3d.enabled = img_n.denoise3d.enabled;
	image->d3d.denoise3dStrength = img_n.denoise3d.denoise3dStrength;

	_get_color(&image->color, chn);

	_get_image_option(&image->option, chn);
	
	return 0;
}

static int _set_image(lpNVP_IMAGE_CONFIG image, int chn)
{
	ST_NSDK_IMAGE img_n;
	ST_NSDK_VIN_CH vin_n;

	memset(&img_n, 0, sizeof(img_n));
	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);
	NETSDK_conf_image_get(&img_n);

	vin_n.brightnessLevel = image->color.brightness;
	vin_n.contrastLevel = image->color.contrast;
	vin_n.hueLevel = image->color.hue;
	vin_n.saturationLevel = image->color.saturation;
	vin_n.sharpnessLevel = image->color.sharpness;

	img_n.irCutFilter.irCutControlMode = image->ircut.control_mode;
	img_n.irCutFilter.irCutMode = image->ircut.ircut_mode;

	img_n.manualSharpness.enabled = image->manual_sharp.enabled;
	img_n.manualSharpness.sharpnessLevel = image->color.sharpness;

	img_n.wdr.enabled = image->wdr.enabled;
	img_n.wdr.WDRStrength = image->wdr.WDRStrength;

	img_n.denoise3d.enabled = image->d3d.enabled;
	img_n.denoise3d.denoise3dStrength = image->d3d.denoise3dStrength;

	if (NETSDK_conf_vin_ch_set(chn + 1, &vin_n) ){
		netsdk_vin_ch_set(chn + 1, &vin_n);
		if (NETSDK_conf_image_set(&img_n)) {
			netsdk_image_changed(&img_n);
			return 0;
		}
	}
	return -1;
}

static int _get_video_source(lpNVP_V_SOURCE src, int chn)
{
	ST_NSDK_VIN_CH vin_n;

	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);

	src->resolution.width = vin_n.captureWidth;
	src->resolution.height = vin_n.captureHeight;	

	src->fps = vin_n.captureFrameRate;

	_get_image(&src->image, chn);
	
	return 0;
}

static int _set_video_source(lpNVP_V_SOURCE src, int chn)
{	
	return _set_image(&src->image, chn);
}


static int _get_video_input_conf(lpNVP_VIN_CONFIG vin, int chn)
{
	ST_NSDK_VIN_CH vin_n;

	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);

	vin->rect.nX = 0;
	vin->rect.nY = 0;
	vin->rect.width = vin_n.captureWidth;
	vin->rect.height = vin_n.captureHeight;	

	if (vin_n.flip || vin_n.mirror)
		vin->rotate.enabled = true;
	else
		vin->rotate.enabled = false;
	
	vin->rotate.degree = 0;
	if (vin_n.mirror)
		vin->rotate.degree += 90;
	if (vin_n.flip)
		vin->rotate.degree += 180;
	
	return 0;
}

static int _set_video_input_conf(lpNVP_VIN_CONFIG vin, int chn)
{	
	ST_NSDK_VIN_CH vin_n;

	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);

	vin_n.captureWidth = vin->rect.width;
	vin_n.captureHeight = vin->rect.height;	

	if (vin->rotate.enabled) {
		if (vin->rotate.degree == NVP_ROTATE_MIRROR) {
			vin_n.mirror = true;
		} else if (vin->rotate.degree == NVP_ROTATE_FLIP) {
			vin_n.flip = true;
		} else if (vin->rotate.degree == NVP_ROTATE_FLIP_MIRROR) {
			vin_n.flip = true;
			vin_n.mirror = true;
		} else {
			APP_TRACE("unsupported rotate degree: %d", vin->rotate.degree);
			return -1;
		}
	}

	if (NETSDK_conf_vin_ch_set(chn + 1, &vin_n) ) {
		netsdk_vin_ch_set(chn + 1, &vin_n);
		return 0;
	}else
		return -1;
}

static int _get_video_encode_option(lpNVP_VENC_OPTIONS venc, int chn)
{
	ST_NSDK_VIN_CH vin_n;

	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);

	ST_NSDK_VENC_CH venc_n;

	memset(&venc_n, 0, sizeof(venc_n));
	NETSDK_conf_venc_ch_get((chn + 1) * 100 + venc->index + 1, &venc_n);

	// FIM Me
	venc->enc_fps.min = 3;
	venc->enc_fps.max = 15;

	venc->enc_gov.min = 1;
	venc->enc_gov.max = 100;

	venc->enc_interval.min = 1;
	venc->enc_interval.max = 1;
	
	venc->enc_quality.min = 0;
	venc->enc_quality.max = 4;


	char resolution_opt[512] = { 0 };
	char width[10] = { 0 };
	char height[10] = { 0 };
	char opt[128] = { 0 };
	int number = 0;
	if (venc_n.resolutionProperty.opt) {
            sscanf(venc_n.resolutionProperty.opt, "[%511[^]]", resolution_opt);
            if (strlen(resolution_opt) > 0) {
                    char *token = strtok(resolution_opt, ",");
                    while (token != NULL) {
                        NK_STR_TRIM(token, opt, sizeof(opt));
                        sscanf(opt, "\"%127[^\"]", opt);
                        if (2 == sscanf(opt, "%9[^x]x%9[^;#]", width, height)) {
                            NVP_SET_SIZE(&venc->resolution[number], atoi(width), atoi(height));
                            number++;
                        }
                        token = strtok(NULL, ",");
                    }
                    venc->resolution_nr = number;
            }
	}
//	if (venc->index == 0) {
//		venc->enc_bps.min  = 512;
//		venc->enc_bps.max = 5120;
//
//
//	} else if (venc->index == 1) {
//		venc->enc_bps.min  = 32;
//		venc->enc_bps.max = 1500;
//
//	} else {
//		venc->enc_bps.min  = 32;
//		venc->enc_bps.max = 512;
//
//	}

	venc->enc_bps.min = venc_n.constantBitRateProperty.min;
	venc->enc_bps.max = venc_n.constantBitRateProperty.max;

	venc->enc_profile_nr = 1;
	venc->enc_profile[0] = NVP_H264_PROFILE_MAIN;

	venc->coding_fmt_nr = 1;//for zw_protocol
	return 0;
}

static int _get_video_encode(lpNVP_VENC_CONFIG venc, int chn)
{
	ST_NSDK_VENC_CH venc_n;

	memset(&venc_n, 0, sizeof(venc_n));
	NETSDK_conf_venc_ch_get((chn + 1)*100+ venc->index + 1, &venc_n);

	if (venc_n.freeResolution == true) {
		venc->width = venc_n.resolutionWidth;
		venc->height =  venc_n.resolutionHeight;
	} else {
		venc->width = (venc_n.resolution >> 16) & 0xffff;
		venc->height =  venc_n.resolution & 0xffff;
	}
	venc->enc_bps = venc_n.constantBitRate;
	venc->enc_fps = venc_n.frameRate;
	venc->enc_gov = venc_n.keyFrameInterval;
	venc->enc_interval = 1;	
	venc->enc_quality = 3;//venc_n.definitionType;
	
	if (venc_n.bitRateControlType == kNSDK_BR_CONTROL_CBR) {
		venc->quant_mode = NVP_QUANT_CBR;
	} else {
		venc->quant_mode = NVP_QUANT_VBR;
	}

//	if(0 == strcmp(venc->name, ZW_PROTOCOL_FLAG)){
//		if (venc_n.codecType == kNSDK_CODEC_TYPE_H264) {
//			venc->enc_type = NVP_VENC_H264;
//		} else if(venc_n.codecType == kNSDK_CODEC_TYPE_H265){
//			venc->enc_type = NVP_VENC_H265;
//		} else {
//			venc->enc_type = NVP_VENC_H264;
//			APP_ASSERT(0, "err: unknown code type: %d  @id=%d,%d", venc_n.codecType, chn, venc->index);
//		}
//	}else{
		if (venc_n.codecType == kNSDK_CODEC_TYPE_H264) {
			venc->enc_type = NVP_VENC_H264;
		} else if(venc_n.codecType == kNSDK_CODEC_TYPE_H265){
			venc->enc_type = NVP_VENC_H265;
		} else {
			venc->enc_type = NVP_VENC_H264;
			APP_ASSERT(0, "err: unknown code type: %d  @id=%d,%d", venc_n.codecType, chn, venc->index);
		}
//	}
	// FIX me
	venc->enc_profile = NVP_H264_PROFILE_MAIN;//venc_n.h264Profile - 1;
	if (venc->index == 0)
		venc->user_count = 4;
	else
		venc->user_count = 6;
	
	venc->option.index = venc->index;
	strcpy(venc->option.token, venc->token);
	strcpy(venc->option.enc_token, venc->enc_token);
	_get_video_encode_option(&venc->option, chn);

	return 0;
}

static int _set_video_encode(lpNVP_VENC_CONFIG venc, int chn)
{
	ST_NSDK_VENC_CH venc_n;
	memset(&venc_n, 0, sizeof(venc_n));
	NETSDK_conf_venc_ch_get((chn + 1)*100+ venc->index + 1, &venc_n);
	
	if (venc_n.freeResolution == true) {
		 venc_n.resolutionWidth = venc->width;
		 venc_n.resolutionHeight = venc->height;
	} else {
		//for XM NVR set Resolution
		if(720 == venc->width && 480 == venc->height){
			venc->width  = 720;
			venc->height = 576;
		}else if(352 == venc->width && 240 == venc->height){
			venc->width  = 352;
			venc->height = 288;
		}
		venc_n.resolution = ((venc->width & 0xffff) << 16) | (venc->height & 0xffff);
	}
	if (venc->quant_mode == NVP_QUANT_CBR) {
		venc_n.bitRateControlType = kNSDK_BR_CONTROL_CBR;
	} else {
		venc_n.bitRateControlType = kNSDK_BR_CONTROL_VBR;
	}
	venc_n.frameRate = venc->enc_fps;
	if(venc->enc_bps > venc_n.constantBitRateProperty.max){
		venc_n.constantBitRate = venc_n.constantBitRateProperty.max;
	}else{
		venc_n.constantBitRate = venc->enc_bps;
	}
	venc_n.keyFrameInterval = venc->enc_gov;	
	venc_n.keyFrameInterval = venc->enc_gov;
//	venc_n.definitionType = venc->enc_quality;

//	if(0 == strcmp(venc->name, ZW_PROTOCOL_FLAG)){
		if (venc->enc_type == NVP_VENC_H264) {
			venc_n.codecType = kNSDK_CODEC_TYPE_H264;
		}else if (venc->enc_type == NVP_VENC_H265) {
			venc_n.codecType = kNSDK_CODEC_TYPE_H265;
		}else {
			APP_TRACE("err: unsupported code type: %d", venc->enc_type);
			return -1;
		}
//	}

	if (NETSDK_conf_venc_ch_set((chn + 1)*100+ venc->index + 1, &venc_n)){
	//	NETSDK_venc_ch_delay_set((chn + 1)*100+ venc->index + 1, (void*)&venc_n);
		netsdk_venc_ch_changed((chn + 1)*100+ venc->index + 1, (LP_NSDK_VENC_CH)&venc_n);
		return 0;
	}else
		return -1;
}

static int _get_audio_input(lpNVP_AIN_CONFIG ain, int chn)
{
	return 0;
}

static int _set_audio_input(lpNVP_AIN_CONFIG ain, int chn)
{
	return -1;
}


static int _get_audio_encode(lpNVP_AENC_CONFIG aenc, int chn)
{
	//FIX me
	aenc->channel = 1;
	aenc->enc_type = NVP_AENC_G711;
	aenc->sample_size = 8;
	aenc->sample_rate = 8000;
	
	aenc->user_count = 2;
	return 0;
}

static int _set_audio_encode(lpNVP_AENC_CONFIG aenc, int chn)
{
	return -1;
}

static void grid_map_set(bool grid[ONVIF_MD_MAX_ROW][ONVIF_MD_MAX_COLUM], lpNVP_MD_GRANULARITY mgrid, int row, int colum)
{
    int i, j;
    //bool grid[18][22] = { false };
	int nn;
	unsigned char *pgridData = (unsigned char *)mgrid->granularity;

	row = (row >= ONVIF_MD_MAX_ROW) ? ONVIF_MD_MAX_ROW : row;
	colum = (colum >= ONVIF_MD_MAX_COLUM) ? ONVIF_MD_MAX_COLUM : colum;

	for(i = 0; i < row; i++){
		for(j = 0; j < colum; j ++){
			nn = i*colum + j;
			grid[i][j] = pgridData[nn/8] & (0x1 << (7 - (nn%8)));
		}
	}
}

static void grid_map_get(bool grid[ONVIF_MD_MAX_ROW][ONVIF_MD_MAX_COLUM], lpNVP_MD_GRANULARITY mgrid, int row, int colum)
{
	int i, j, nn;
	unsigned char *pgridData = (unsigned char *)mgrid->granularity;

	row = (row >= ONVIF_MD_MAX_ROW) ? ONVIF_MD_MAX_ROW : row;
	colum = (colum >= ONVIF_MD_MAX_COLUM) ? ONVIF_MD_MAX_COLUM : colum;

	for(i = 0; i < row; i ++){
        for(j = 0; j < colum; j ++){
           if(grid[i][j]){
              nn = i*colum + j;
              pgridData[nn/8] |= 0x1 << (7 - (nn%8));
			} 
         }
     }
}

static int _get_motion_detection(lpNVP_MD_CONFIG md, int chn)
{
	ST_NSDK_MD_CH md_n;
	
	memset(&md_n, 0, sizeof(md_n));
	NETSDK_conf_md_ch_get(chn + 1, &md_n);
	APP_TRACE("motion detection type: %d", md_n.detectionType);
	md->enable = md_n.enabled;
	if (kNSDK_MD_TYPE_GRID == md_n.detectionType) {
		md->type = NVP_MD_TYPE_GRID;
		md->grid.columnGranularity = md_n.detectionGrid.columnGranularity;
		md->grid.rowGranularity = md_n.detectionGrid.rowGranularity;
		md->grid.sensitivity = md_n.detectionGrid.sensitivityLevel;

		int i, ii;
		bool des_grid[ONVIF_MD_MAX_ROW][ONVIF_MD_MAX_COLUM];
		for(i = 0; i < md_n.detectionGrid.rowGranularity; ++i){
			for(ii = 0; ii < md_n.detectionGrid.columnGranularity; ++ii){
				LP_NSDK_MD_GRID detectionGrid = &md_n.detectionGrid;
				des_grid[i][ii] = detectionGrid->getGranularity(detectionGrid, i, ii);
				printf("%c", des_grid[i][ii] ? '#' : '.');
			}
			printf("\n");
		}
		grid_map_get(des_grid, &(md->grid), md_n.detectionGrid.rowGranularity, md_n.detectionGrid.columnGranularity);
		md->grid.threshold = 5;
	}
	if(0 == strcmp(md->module_name, ZW_PROTOCOL_FLAG)) {
		//md->type = NVP_MD_TYPE_REGION;
		//md->region.ch[0].sensitivity = md_n.detectionRegion.ch[0].sensitivityLevel;
		md->region.ch[0].index = 1;

		ST_NSDK_VENC_CH venc_ch;
		NETSDK_conf_venc_ch_get(101,&venc_ch);

		int venc0_width = (venc_ch.resolution >> 16) & 0xffff;
		int venc0_height = venc_ch.resolution & 0xffff;
		float regionX, regionY, regionWidth, regionHeight;
		int region1X, region1Y, region1Width, region1Height;
		int region2X, region2Y, region2Width, region2Height;
		int i, j;
		for(i = 1; i < 4; i++){
			region1X = (int)md_n.detectionRegion.ch[i].regionX;
			region1Y = (int)md_n.detectionRegion.ch[i].regionY;
			region1Width = (int)md_n.detectionRegion.ch[i].regionWidth;
			region1Height = (int)md_n.detectionRegion.ch[i].regionHeight;
			for(j = 0; j < i; j++){
				region2X = (int)md_n.detectionRegion.ch[j].regionX;
				region2Y = (int)md_n.detectionRegion.ch[j].regionY;
				region2Width = (int)md_n.detectionRegion.ch[j].regionWidth;
				region2Height = (int)md_n.detectionRegion.ch[j].regionHeight;
				if((region2X || region2Y || region2Width || region2Height)
					&& region2X == region1X
					&& region2Y == region1Y
					&& region2Width == region1Width
					&& region2Height == region1Height){
					md_n.detectionRegion.ch[i].regionX = 0.0;
					md_n.detectionRegion.ch[i].regionY = 0.0;
					md_n.detectionRegion.ch[i].regionWidth = 0.0;
					md_n.detectionRegion.ch[i].regionHeight = 0.0;
				}
			}
		}
		for(i = 0; i < 4; i++){
			regionX = md_n.detectionRegion.ch[i].regionX;
			regionY = md_n.detectionRegion.ch[i].regionY;
			regionWidth = md_n.detectionRegion.ch[i].regionWidth;
			regionHeight = md_n.detectionRegion.ch[i].regionHeight;
			md->region.ch[i].nX = regionX * venc0_width / 100;
			md->region.ch[i].nY = regionY * venc0_height / 100;
			md->region.ch[i].nWidth = regionWidth * venc0_width / 100;
			md->region.ch[i].nHeight = regionHeight * venc0_height / 100;
		}
	}

	// FIX me
	md->delay_off_alarm = 300;
	md->delay_on_alarm = 200;

	return 0;
}


static int _set_motion_detection(lpNVP_MD_CONFIG md, int chn)
{
	ST_NSDK_MD_CH md_n;

	memset(&md_n, 0, sizeof(md_n));
	NETSDK_conf_md_ch_get(1, &md_n);

	md_n.enabled = md->enable;
	if(NVP_MD_TYPE_GRID == md->type){
	md_n.detectionType = kNSDK_MD_TYPE_GRID;
	md_n.detectionGrid.columnGranularity = md->grid.columnGranularity;
	md_n.detectionGrid.rowGranularity = md->grid.rowGranularity;
	md_n.detectionGrid.sensitivityLevel = md->grid.sensitivity;
		
		int i, ii;
		bool des_grid[ONVIF_MD_MAX_ROW ][ONVIF_MD_MAX_COLUM];
	grid_map_set(des_grid, &(md->grid), md_n.detectionGrid.rowGranularity, md_n.detectionGrid.columnGranularity);
	for(i = 0; i < md_n.detectionGrid.rowGranularity; ++i){
		for(ii = 0; ii < md_n.detectionGrid.columnGranularity; ++ii){
			LP_NSDK_MD_GRID detectionGrid = &md_n.detectionGrid;
			detectionGrid->setGranularity(detectionGrid, i, ii, des_grid[i][ii]);
			printf("%c", des_grid[i][ii] ? '#' : '.');
		}
		printf("\n");
		}
	}else {
		APP_TRACE("err: unsupported motion detection type: %d", md->type);
		return -1;
	}
	if(0 == strcmp(md->module_name, ZW_PROTOCOL_FLAG)){
		//md_n.detectionType = kNSDK_MD_TYPE_REGION;
		//md_n.detectionRegion.ch[0].sensitivityLevel = md->region.ch[0].sensitivity;

		ST_NSDK_VENC_CH venc_ch;
		NETSDK_conf_venc_ch_get(101,&venc_ch);

		int venc0_width = (venc_ch.resolution >> 16) & 0xffff;
		int venc0_height = venc_ch.resolution & 0xffff;
		float nX, nY, nWidth, nHeight;
		int i;
		for(i = 0; i < 4; i++){
			nX = md->region.ch[i].nX;
			nY = md->region.ch[i].nY;
			nWidth = md->region.ch[i].nWidth;
			nHeight = md->region.ch[i].nHeight;
			md_n.detectionRegion.ch[i].enabled = md->enable;
			md_n.detectionRegion.ch[i].regionX = nX * 100 / venc0_width;
			md_n.detectionRegion.ch[i].regionY = nY * 100 / venc0_height;
			md_n.detectionRegion.ch[i].regionWidth = nWidth * 100 /venc0_width;
			md_n.detectionRegion.ch[i].regionHeight = nHeight * 100 /venc0_height;
		}
	}

	NETSDK_conf_md_ch_set(1, &md_n);
	
	return 0;
}

static int _get_video_analytic(lpNVP_VAN_CONFIG van, int chn)
{
	return 0;
}

static int _set_video_analytic(lpNVP_VAN_CONFIG van, int chn)
{
	return 0;
}


static int _get_ptz(lpNVP_PTZ_CONFIG ptz, int chn)
{
	return 0;
}

static int _set_ptz(lpNVP_PTZ_CONFIG ptz, int chn)
{
	return 0;
}

static int _get_profile(lpNVP_PROFILE_CHN profile)
{
	int i;
	// FIX me
	profile->profile_nr = 2;
	profile->venc_nr = 2;
	profile->aenc_nr = 1;
	//
	for (i = 0; i < profile->venc_nr; i++) {
		_get_video_encode(&profile->venc[i], profile->index);
	}
	for (i = 0; i < profile->aenc_nr; i++) {
		_get_audio_encode(&profile->aenc[i], profile->index);
	}
	_get_video_source(&profile->v_source, profile->index);
	for (i = 0; i < profile->vin_conf_nr; i++) {
		_get_video_input_conf(&profile->vin[i], profile->index);
	}
	_get_audio_input(&profile->ain, profile->index);
	_get_ptz(&profile->ptz, profile->index);
	_get_video_analytic(&profile->van, profile->index);
	_get_motion_detection(&profile->md, profile->index);

	return 0;
}

static int _set_profile(lpNVP_PROFILE_CHN profile)
{
	int i;
	int ret = 0;
	//
	for (i = 0; i < profile->venc_nr; i++) {
		if (_set_video_encode(&profile->venc[i], profile->index) < 0)
			ret = -1;
	}
	for (i = 0; i < profile->aenc_nr; i++) {
		if (_set_audio_encode(&profile->aenc[i], profile->index) < 0)
			ret = -1;
	}
	if (_set_video_source(&profile->v_source, profile->index) < 0)
		ret = -1;
	for (i = 0; i < profile->vin_conf_nr; i++) {
		if (_set_video_input_conf(&profile->vin[i], profile->index) < 0)
			ret = -1;
	}
	if (_set_audio_input(&profile->ain, profile->index) < 0)
		ret = -1;
	if (_set_ptz(&profile->ptz, profile->index) < 0)
		ret = -1;
	if (_set_video_analytic(&profile->van, profile->index) < 0)
		ret = -1;
	if (_set_motion_detection(&profile->md, profile->index) < 0)
		ret = -1;

	return ret;
}

static int _get_profiles(lpNVP_PROFILE profiles)
{
	int i;

	profiles->chn = NVP_MAX_CH;
	//
	for ( i = 0; i < profiles->chn; i++) {
		_get_profile(&profiles->profile[i]);
	}
	return 0;
}

static int _set_profiles(lpNVP_PROFILE profiles)
{
	int i;
	int ret = 0;
	//
	for ( i = 0; i < profiles->chn; i++) {
		if(_set_profile(&profiles->profile[i]) < 0)
			ret = -1;
	}
	return ret;
}


static int _get_all(lpNVP_ENV env)
{
	_get_system_information(&env->devinfo);
	_get_date_time(&env->systime);
	_get_interface(&env->ether);
	_get_profiles(&env->profiles);

	return 0;
}

static int _set_all(lpNVP_ENV env)
{
	int ret = 0;

	if (_get_system_information(&env->devinfo) < 0)
		ret = -1;
	if (_get_date_time(&env->systime) < 0)
		ret = -1;
	if (_get_interface(&env->ether) < 0)
		ret = -1;
	if (_get_profiles(&env->profiles) < 0)
		ret = -1;

	return ret;
}


/********************************************************************************
* system command interfaces
*********************************************************************************/
static void _cmd_system_boot(long l, void *r)
{
	TICKER_del_task(_cmd_system_boot);
	APP_TRACE("system reboot now...");
	exit(0);
}

static int _cmd_ptz(lpNVP_CMD cmd, const char *module, int keyid)
{
	const char *ptz_cmd_name[] =
	{
		"PTZ_CMD_UP",
		"PTZ_CMD_DOWN",
		"PTZ_CMD_LEFT",
		"PTZ_CMD_RIGHT",
		"PTZ_CMD_LEFT_UP",
		"PTZ_CMD_RIGHT_UP",
		"PTZ_CMD_LEFT_DOWN",
		"PTZ_CMD_RIGHT_DOWN",
		"PTZ_CMD_AUTOPAN",
		"PTZ_CMD_IRIS_OPEN",
		"PTZ_CMD_IRIS_CLOSE",
		"PTZ_CMD_ZOOM_IN",
		"PTZ_CMD_ZOOM_OUT",
		"PTZ_CMD_FOCUS_FAR",
		"PTZ_CMD_FOCUS_NEAR",
		"PTZ_CMD_STOP",
		"PTZ_CMD_WIPPER_ON",
		"PTZ_CMD_WIPPER_OFF",
		"PTZ_CMD_LIGHT_ON",
		"PTZ_CMD_LIGHT_OFF",
		"PTZ_CMD_POWER_ON",
		"PTZ_CMD_POWER_OFF",
		"PTZ_CMD_GOTO_PRESET",
		"PTZ_CMD_SET_PRESET",
		"PTZ_CMD_CLEAR_PRESET",
		"PTZ_CMD_TOUR",
	};

	int speed = 0;
	int ret = -1;

	APP_TRACE("%s(%d)", ptz_cmd_name[cmd->ptz.cmd], cmd->ptz.cmd);

#if defined(UART_PROTOCOL)

	ST_NSDK_PTZ_CFG stPtzConfig;
	memset(&stPtzConfig, 0, sizeof(stPtzConfig));
	if(0 != cmd->ptz.speed){
		NETSDK_conf_ptz_ch_get(&stPtzConfig);
		stPtzConfig.stPtzExternalConfig.nSpeed = cmd->ptz.speed*8;
	
		if(stPtzConfig.stPtzExternalConfig.nSpeed < 1){
			stPtzConfig.stPtzExternalConfig.nSpeed = 1;
		}
		
		NETSDK_conf_ptz_ch_set(&stPtzConfig);

		speed = ptz_speed_level_switch(stPtzConfig.stPtzExternalConfig.nSpeed);
	}	
	
	switch(cmd->ptz.cmd) 
	{
		case NVP_PTZ_CMD_LEFT:			
			ret = PTZ_Send(0, PTZ_CMD_LEFT, speed);
			break;
		case NVP_PTZ_CMD_RIGHT:
			ret = PTZ_Send(0, PTZ_CMD_RIGHT, speed);
			break;
		case NVP_PTZ_CMD_UP:
			ret = PTZ_Send(0, PTZ_CMD_UP, speed);
			break;
		case NVP_PTZ_CMD_DOWN:
			ret = PTZ_Send(0, PTZ_CMD_DOWN, speed);
			break;
		case NVP_PTZ_CMD_LEFT_UP:			
			ret = PTZ_Send(0, PTZ_CMD_LEFT_UP, speed);
			break;
		case NVP_PTZ_CMD_RIGHT_UP:
			ret = PTZ_Send(0, PTZ_CMD_RIGHT_UP, speed);
			break;
		case NVP_PTZ_CMD_LEFT_DOWN:
			ret = PTZ_Send(0, PTZ_CMD_LEFT_DOWN, speed);
			break;
		case NVP_PTZ_CMD_RIGHT_DOWN:
			ret = PTZ_Send(0, PTZ_CMD_RIGHT_DOWN, speed);
			break;
		case NVP_PTZ_CMD_AUTOPAN:
			if(0 == strcmp(stPtzConfig.stPtzExternalConfig.strptzCustomTpye,"BEISIDE")){
				ret = PTZ_Send(0, PTZ_CMD_GOTO_PRESET,99);
			}else{
				ret = PTZ_Send(0, PTZ_CMD_AUTOPAN, speed);
			}
			
			break;
		case NVP_PTZ_CMD_ZOOM_IN:
			ret = PTZ_Send(0, PTZ_CMD_ZOOM_IN, speed);
			break;
		case NVP_PTZ_CMD_ZOOM_OUT:
			ret = PTZ_Send(0, PTZ_CMD_ZOOM_OUT, speed);
			break;
		case NVP_PTZ_CMD_FOCUS_FAR:
			ret = PTZ_Send(0, PTZ_CMD_FOCUS_FAR, speed);
			break;
		case NVP_PTZ_CMD_FOCUS_NEAR:
			ret = PTZ_Send(0, PTZ_CMD_FOCUS_NEAR, speed);
			break;	
		case NVP_PTZ_CMD_SET_PRESET:			
			ret = PTZ_Send(0, PTZ_CMD_SET_PRESET, cmd->ptz.index);
			break;
		case NVP_PTZ_CMD_GOTO_PRESET:
			ret = PTZ_Send(0, PTZ_CMD_GOTO_PRESET, cmd->ptz.index);
			break;
		case NVP_PTZ_CMD_CLEAR_PRESET:
			ret = PTZ_Send(0, PTZ_CMD_CLEAR_PRESET, cmd->ptz.index);
			break;
		case NVP_PTZ_CMD_STOP:
			ret = PTZ_Send(0, PTZ_CMD_STOP, 0);
			break;
		default:
			break;
	}
#endif

	return ret;
}


static int _get_user(lpNVP_USER user)
{
	if(0 == strcmp(user->snapshot_url, ZW_PROTOCOL_FLAG)){
		int venc1_width;
		int venc1_height;
		ST_NSDK_VENC_CH venc_ch;
		NETSDK_conf_venc_ch_get(102,&venc_ch);
		if(!venc_ch.freeResolution){
			venc1_width = (venc_ch.resolution >> 16) & 0xffff;
			venc1_height = (venc_ch.resolution >> 0) & 0xffff;
		}else{
			venc1_width = venc_ch.resolutionWidth;
			venc1_height = venc_ch.resolutionHeight;
		}

		NETSDK_venc_snapshot(1, kNSDK_SNAPSHOT_IMAGE_JPEG, venc1_width, venc1_height, user->snapshot_url);
		//must be call after calling NETSDK_venc_snapshot()
		//NETSDK_venc_free_snapshot(user->snapshot_url);
	}else{
	ST_NSDK_NETWORK_PORT port_n;
	ifconf_interface_t irf;
	char *ether_iface = getenv("DEF_ETHER");
	memset(&port_n, 0, sizeof(port_n));

	if(!user){
		return -1;
	}

	NETSDK_conf_port_get(1, &port_n);
	ifconf_get_interface(ether_iface, &irf);

	snprintf(user->snapshot_url, sizeof(user->snapshot_url), "http://%d.%d.%d.%d:%d/snapshot.jpg", 
			irf.ipaddr.s_b[0], irf.ipaddr.s_b[1], irf.ipaddr.s_b[2], irf.ipaddr.s_b[3], port_n.value);
	}

	return 0;
}

static int _get_privmask(lpNVP_PRIVACYMASKS_CONFIG primask, int chn)
{
	int i;
	float n1X, n1Y, n2X, n2Y, n3X, n3Y, n4X, n4Y;
	ST_NSDK_VIN_CH vin_n;

	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);

	primask->privacyMask[0].enable = vin_n.privacyMask[0].enabled;

	for(i = 0; i < 4; i++){
		n1X = vin_n.privacyMask[i].regionX * 2 / 100 - 1;
		n1Y = vin_n.privacyMask[i].regionY * 2 / 100 - 1;
		n2X = (vin_n.privacyMask[i].regionX + vin_n.privacyMask[i].regionWidth) * 2 / 100 - 1;
		n2Y = n1Y;
		n3X = n2X;
		n3Y = (vin_n.privacyMask[i].regionY + vin_n.privacyMask[i].regionHeight) * 2 / 100 - 1;
		n4X = n1X;
		n4Y = n3Y;
		primask->privacyMask[i].point1.nX = n1X;
		primask->privacyMask[i].point1.nY = n1Y;
		primask->privacyMask[i].point2.nX = n2X;
		primask->privacyMask[i].point2.nY = n2Y;
		primask->privacyMask[i].point3.nX = n3X;
		primask->privacyMask[i].point3.nY = n3Y;
		primask->privacyMask[i].point4.nX = n4X;
		primask->privacyMask[i].point4.nY = n4Y;
	}

	return 0;
}

static int _set_privmask(lpNVP_PRIVACYMASKS_CONFIG primask, int chn)
{
	int i;
	float n1X, n1Y, n3X, n3Y;
	ST_NSDK_VIN_CH vin_n;

	memset(&vin_n, 0, sizeof(vin_n));
	NETSDK_conf_vin_ch_get(chn + 1, &vin_n);

	////erase all privacyMask areas
	for(i = 0; i < 4; i++){
		vin_n.privacyMask[i].enabled = false;
	}
	if (NETSDK_conf_vin_ch_set(chn + 1, &vin_n) ){
		netsdk_vin_ch_set(chn + 1, &vin_n);
	}

	for(i = 0; i < 4; i++){
		n1X = primask->privacyMask[i].point1.nX;
		n1Y = primask->privacyMask[i].point1.nY;
		n3X = primask->privacyMask[i].point3.nX;
		n3Y = primask->privacyMask[i].point3.nY;
		vin_n.privacyMask[i].enabled = primask->privacyMask[0].enable;
		vin_n.privacyMask[i].regionX = (n1X + 1) * 100 / 2;
		vin_n.privacyMask[i].regionY = (n1Y + 1) * 100 / 2;
		vin_n.privacyMask[i].regionWidth = (n3X - n1X) * 100 / 2;
		vin_n.privacyMask[i].regionHeight = (n3Y - n1Y) * 100 / 2;
	}

	if (NETSDK_conf_vin_ch_set(chn + 1, &vin_n) ){
		netsdk_vin_ch_set(chn + 1, &vin_n);
		return 0;
	}else{
		return -1;
	}
}

static int _get_osd(lpNVP_ENV env,int id){
	ST_NSDK_VENC_CH venc_ch;
	NETSDK_conf_venc_ch_get(101,&venc_ch);

	env->profiles.profile[0].time_ol.enable = venc_ch.datetimeOverlay.o.enabled;
	if(kNSDK_DATETIME_FMT_SLASH_YYYYMMDD  == venc_ch.datetimeOverlay.dateFormat){
		env->profiles.profile[0].time_ol.date_type  =1;
	}else if(kNSDK_DATETIME_FMT_SLASH_MMDDYYYY == venc_ch.datetimeOverlay.dateFormat){
		env->profiles.profile[0].time_ol.date_type  =2;
	}else if(kNSDK_DATETIME_FMT_SLASH_DDMMYYYY == venc_ch.datetimeOverlay.dateFormat){
		env->profiles.profile[0].time_ol.date_type  =3;
	}else if(kNSDK_DATETIME_FMT_DASH_YYYYMMDD == venc_ch.datetimeOverlay.dateFormat){
		env->profiles.profile[0].time_ol.date_type  =4;
	}else if(kNSDK_DATETIME_FMT_DASH_MMDDYYYY == venc_ch.datetimeOverlay.dateFormat){
		env->profiles.profile[0].time_ol.date_type  = 5;
	}else if(kNSDK_DATETIME_FMT_DASH_DDMMYYYY == venc_ch.datetimeOverlay.dateFormat){
		env->profiles.profile[0].time_ol.date_type  =6;
	}else{
		env->profiles.profile[0].time_ol.date_type  =1;
	}

	if(12 == venc_ch.datetimeOverlay.dateFormat){
		env->profiles.profile[0].time_ol.time_type  =1;
	}else {
		env->profiles.profile[0].time_ol.time_type  =2;
	}

	int xpos_time = (int)venc_ch.datetimeOverlay.o.regionX;
	int ypos_time = (int)venc_ch.datetimeOverlay.o.regionY;
	if(50 > xpos_time && 50 > ypos_time){
		env->profiles.profile[0].time_ol.where.location = NVP_TOPLEFT;
	}else if(50 > xpos_time && 50 < ypos_time){
		env->profiles.profile[0].time_ol.where.location = NVP_BOTTOMLEFT;
	}else if(50 < xpos_time && 50 > ypos_time){
		env->profiles.profile[0].time_ol.where.location = NVP_TOPRIGHT;
	}else if(50 < xpos_time && 50 < ypos_time){
		env->profiles.profile[0].time_ol.where.location = NVP_BOTTOMRIGHT;
	}

	env->profiles.profile[0].title_ol.enable = venc_ch.channelNameOverlay.o.enabled;
	if(0 == strcmp(env->profiles.profile[0].venc[0].name, ZW_PROTOCOL_FLAG)){
		strcpy(env->profiles.profile[0].title_ol.alias, venc_ch.channelName);
	}else{
		bool isgbk = FALSE;
		char utf8[64] = {0};
		isgbk = gb2312_to_utf8(false, venc_ch.channelName, utf8, sizeof(utf8));
		if(TRUE == isgbk){//有简体中文字符
			strcpy(env->profiles.profile[0].title_ol.alias, utf8);
		}else{//没简体中文字符
			strcpy(env->profiles.profile[0].title_ol.alias, venc_ch.channelName);
		}
	}

	int xpos_title = (int)venc_ch.channelNameOverlay.o.regionX;
	int ypos_title = (int)venc_ch.channelNameOverlay.o.regionY;
	if(50 > xpos_title && 50 > ypos_title){
		env->profiles.profile[0].title_ol.where.location = NVP_TOPLEFT;
	}else if(50 > xpos_title && 50 < ypos_title){
		env->profiles.profile[0].title_ol.where.location = NVP_BOTTOMLEFT;
	}else if(50 < xpos_title && 50 > ypos_title){
		env->profiles.profile[0].title_ol.where.location = NVP_TOPRIGHT;
	}else if(50 < xpos_title && 50 < ypos_title){
		env->profiles.profile[0].title_ol.where.location = NVP_BOTTOMRIGHT;
	}
	return 0;
}

static int _set_osd(lpNVP_ENV env,int id){
	ST_NSDK_VENC_CH venc_ch;
	NETSDK_conf_venc_ch_get(101,&venc_ch);
	venc_ch.datetimeOverlay.o.enabled = env->profiles.profile[0].time_ol.enable;
	if(1 == env->profiles.profile[0].time_ol.date_type ){
		venc_ch.datetimeOverlay.dateFormat = kNSDK_DATETIME_FMT_SLASH_YYYYMMDD;
	}else if(2 == env->profiles.profile[0].time_ol.date_type ){
		venc_ch.datetimeOverlay.dateFormat = kNSDK_DATETIME_FMT_SLASH_MMDDYYYY;
	}else if(3 == env->profiles.profile[0].time_ol.date_type ){
		venc_ch.datetimeOverlay.dateFormat = kNSDK_DATETIME_FMT_SLASH_DDMMYYYY;
	}else if(4 == env->profiles.profile[0].time_ol.date_type ){
		venc_ch.datetimeOverlay.dateFormat = kNSDK_DATETIME_FMT_DASH_YYYYMMDD;
	}else if(5 == env->profiles.profile[0].time_ol.date_type ){
		venc_ch.datetimeOverlay.dateFormat = kNSDK_DATETIME_FMT_DASH_MMDDYYYY;
	}else if(6 == env->profiles.profile[0].time_ol.date_type ){
		venc_ch.datetimeOverlay.dateFormat = kNSDK_DATETIME_FMT_DASH_DDMMYYYY;
	}
	if(1 == env->profiles.profile[0].time_ol.time_type){
		venc_ch.datetimeOverlay.timeFormat = 12;
	}else if(2 == env->profiles.profile[0].time_ol.time_type){
		venc_ch.datetimeOverlay.timeFormat = 24;
	}

	int time_lo = env->profiles.profile[0].time_ol.where.location;
	int title_lo = env->profiles.profile[0].title_ol.where.location;
	if(time_lo != title_lo){
		if(NVP_TOPLEFT == env->profiles.profile[0].time_ol.where.location){
			venc_ch.datetimeOverlay.o.regionX = 8.80;
			venc_ch.datetimeOverlay.o.regionY = 3.10;
		}else if(NVP_BOTTOMLEFT == env->profiles.profile[0].time_ol.where.location){
			venc_ch.datetimeOverlay.o.regionX = 8.80;
			venc_ch.datetimeOverlay.o.regionY = 95.1;
		}else if(NVP_TOPRIGHT == env->profiles.profile[0].time_ol.where.location){
			venc_ch.datetimeOverlay.o.regionX = 60.0;
			venc_ch.datetimeOverlay.o.regionY = 3.10;
		}else if(NVP_BOTTOMRIGHT == env->profiles.profile[0].time_ol.where.location){
			venc_ch.datetimeOverlay.o.regionX = 60.0;
			venc_ch.datetimeOverlay.o.regionY = 95.1;
		}

		if(NVP_TOPLEFT == env->profiles.profile[0].title_ol.where.location){
			venc_ch.channelNameOverlay.o.regionX = 8.80;
			venc_ch.channelNameOverlay.o.regionY = 3.10;
		}else if(NVP_BOTTOMLEFT == env->profiles.profile[0].title_ol.where.location){
			venc_ch.channelNameOverlay.o.regionX = 8.80;
			venc_ch.channelNameOverlay.o.regionY = 95.1;
		}else if(NVP_TOPRIGHT == env->profiles.profile[0].title_ol.where.location){
			venc_ch.channelNameOverlay.o.regionX = 60.0;
			venc_ch.channelNameOverlay.o.regionY = 3.10;
		}else if(NVP_BOTTOMRIGHT == env->profiles.profile[0].title_ol.where.location){
			venc_ch.channelNameOverlay.o.regionX = 60.0;
			venc_ch.channelNameOverlay.o.regionY = 95.1;
		}
	}

	venc_ch.channelNameOverlay.o.enabled = env->profiles.profile[0].title_ol.enable;
	strcpy(venc_ch.channelName,env->profiles.profile[0].title_ol.alias);
	printf("****************[CHN]:%s****************\n", venc_ch.channelName);
	if(NULL != NETSDK_conf_venc_ch_set(101, &venc_ch)){
		netsdk_venc_ch_changed(101, &venc_ch);
	}
	return 0;
}

static int _get_auth(lpNVP_AUTHORIZATIONS_CONFIG auth, int id){
	if(NULL == auth){
		return -1;
	}
	memset(auth, 0, sizeof(stNVP_AUTHORIZATIONS_CONFIG));
	
	int usersnumber = USRM_Users_Get_Count();
	int i_u =0;
	if(usersnumber){
		auth->usernum = usersnumber;
		for(i_u = 0; i_u < usersnumber; i_u ++){
			char usern[64] = {0};
			char pwdn[64] = {0};
			int userRet = USRM_Users_Get_User_PWD(i_u,usern,sizeof(usern), pwdn, sizeof(pwdn));
			if(userRet == 0){
//				printf("user %s 	pwd %s \n",usern,pwdn);
				snprintf(auth->users[i_u].username, sizeof(auth->users[i_u].username), usern);
				snprintf(auth->users[i_u].password, sizeof(auth->users[i_u].password), pwdn);
			}
		}
	}else{
		snprintf(auth->users[0].username, sizeof(auth->users[0].username), "admin");
		auth->usernum = 1;
	}
	
	return 0;
}

/********************************************************************************
* external interfaces
*********************************************************************************/


int NVP_env_load(lpNVP_ENV env, const char *module, int keyid)
{
	char temp[512] = {0};
	char *ptr = NULL, *pbuf = NULL;
	char *saveptr = NULL;
	int ret;
	int chn, id;

	chn = keyid/100;
	id = keyid%100;
	strncpy(temp, module, 511);
	pbuf = temp;
	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_ALL)) {
			ret = _get_all(env);
			break;
		}else if (OM_MATCH(ptr, OM_PROFILE)) {
			ret = _get_profile(&env->profiles.profile[chn]);
		}else if (OM_MATCH(ptr, OM_PROFILES)) {
			ret = _get_profiles(&env->profiles);
		}else if (OM_MATCH(ptr, OM_INFO)) {
			ret = _get_system_information(&env->devinfo);
		}else if (OM_MATCH(ptr, OM_DTIME)) {
			ret = _get_date_time(&env->systime);
		}else if (OM_MATCH(ptr, OM_NET)) {
			ret = _get_interface(&env->ether);
		}  else if (OM_MATCH(ptr, OM_VENC)) {
			ret = _get_video_encode(&env->profiles.profile[chn].venc[id], chn);
		}  else if (OM_MATCH(ptr, OM_VSRC)) {
			ret = _get_video_source(&env->profiles.profile[chn].v_source, chn);
		}  else if (OM_MATCH(ptr, OM_VINC)) {
			ret = _get_video_input_conf(&env->profiles.profile[chn].vin[id], chn);
		}  else if (OM_MATCH(ptr, OM_AENC)) {
			ret = _get_audio_encode(&env->profiles.profile[chn].aenc[id], chn);
		}  else if (OM_MATCH(ptr, OM_AIN)) {
			ret = _get_audio_input(&env->profiles.profile[chn].ain, chn);
		}  else if (OM_MATCH(ptr, OM_COLOR)) {
			ret = _get_color(&env->profiles.profile[chn].v_source.image.color, chn);
		}  else if (OM_MATCH(ptr, OM_IMG)) {
			ret = _get_image(&env->profiles.profile[chn].v_source.image, chn);
		}  else if (OM_MATCH(ptr, OM_MD)) {
			ret = _get_motion_detection(&env->profiles.profile[chn].md, chn);
		}  else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _get_ptz(&env->profiles.profile[chn].ptz, chn);
		}  else if (OM_MATCH(ptr, OM_USER)) {
			ret = _get_user(&env->user);
		}  else if (OM_MATCH(ptr, OM_PRIVMASK)) {
			ret = _get_privmask(&env->profiles.profile[chn].privMask, chn);
		}  else if (OM_MATCH(ptr, OM_OSD)) {
			ret = _get_osd(env,id);
		}  else if (OM_MATCH(ptr, OM_AUTH)) {
			ret = _get_auth(&env->profiles.profile[chn].auth, id);
		}
		else {
			APP_TRACE("unknown env module: %s", ptr);
		}
		//
		pbuf = NULL;
	}

	return 0;
}

int NVP_env_save(lpNVP_ENV env, const char *module, int keyid)
{
	char temp[512] = {0};
	char *ptr = NULL, *pbuf = NULL;
	char *saveptr = NULL;
	int ret;
	int f_ret = 0;
	int chn, id;

	chn = keyid / 100;
	id = keyid % 100;
	strncpy(temp, module, 511);
	pbuf = temp;
	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_ALL)) {
			ret = _set_all(env);
			break;
		}else if (OM_MATCH(ptr, OM_PROFILE)) {
			ret = _set_profile(&env->profiles.profile[chn]);
		}else if (OM_MATCH(ptr, OM_PROFILES)) {
			ret = _set_profiles(&env->profiles);
		}else if (OM_MATCH(ptr, OM_INFO)) {
			//
		}else if (OM_MATCH(ptr, OM_DTIME)) {
			ret = _set_date_time(&env->systime);
		}else if (OM_MATCH(ptr, OM_NET)) {
			ret = _set_interface(&env->ether);
		}  else if (OM_MATCH(ptr, OM_VENC)) {
			ret = _set_video_encode(&env->profiles.profile[chn].venc[id], chn);
		}  else if (OM_MATCH(ptr, OM_VSRC)) {
			ret = _set_video_source(&env->profiles.profile[chn].v_source, chn);
		}  else if (OM_MATCH(ptr, OM_VINC)) {
			ret = _set_video_input_conf(&env->profiles.profile[chn].vin[id], chn);
		}  else if (OM_MATCH(ptr, OM_AENC)) {
			ret = _set_audio_encode(&env->profiles.profile[chn].aenc[id], chn);
		}  else if (OM_MATCH(ptr, OM_AIN)) {
			ret = _set_audio_input(&env->profiles.profile[chn].ain, chn);
		}  else if (OM_MATCH(ptr, OM_COLOR)) {
			ret = _set_color(&env->profiles.profile[chn].v_source.image.color, chn);
		}  else if (OM_MATCH(ptr, OM_IMG)) {
			ret = _set_image(&env->profiles.profile[chn].v_source.image, chn);
		}  else if (OM_MATCH(ptr, OM_MD)) {
			ret = _set_motion_detection(&env->profiles.profile[chn].md, chn);
		}  else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _set_ptz(&env->profiles.profile[chn].ptz, chn);
		}else if (OM_MATCH(ptr, OM_PRIVMASK)) {
			ret = _set_privmask(&env->profiles.profile[chn].privMask, chn);
		}else if (OM_MATCH(ptr, OM_OSD)) {
			ret = _set_osd(env,id);
		}else {
			APP_TRACE("unknown env module: %s", ptr);
			f_ret = -1;
		}

		if (ret < 0)
			f_ret = -1;

		//
		pbuf = NULL;
	}

	return f_ret;
}

int NVP_env_cmd(lpNVP_CMD cmd, const char *module, int keyid)
{
	char temp[512] = {0};
	char *ptr = NULL, *pbuf = NULL;
	char *saveptr = NULL;
	int ret;
	int f_ret = 0;
	int chn, id;

	chn = keyid / 100;
	id = keyid % 100;
	strncpy(temp, module, 511);
	pbuf = temp;
	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_REBOOT)) {
			ret = TICKER_add_task(_cmd_system_boot, 1, false);
		}else if (OM_MATCH(ptr, OM_SYS_RESET)) {
			APP_TRACE("unknown env module: %s", ptr);
		}else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _cmd_ptz(cmd, module, keyid);
		}else if(OM_MATCH(ptr, OM_IFRAME)){
			ret = SDK_ENC_request_stream_keyframe(0, keyid);
		} else {
			APP_TRACE("unknown env module: %s", ptr);
			f_ret = -1;
		}

		if (ret < 0)
			f_ret = -1;

		//
		pbuf = NULL;
	}

	return f_ret;
}

#undef ZW_PROTOCOL_FLAG


