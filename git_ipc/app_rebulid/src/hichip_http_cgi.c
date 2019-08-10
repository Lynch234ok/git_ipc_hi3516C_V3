#include "sdk/sdk_api.h"
#include "media_buf.h"
#include "generic.h"
#include "http_common.h"
#include "sensor.h"
#include "sysconf.h"
#include "hichip_debug.h"
#include "hichip.h"
#include "timertask.h"
#include "sdk/sdk_vin.h"
#include "app_motion_detect.h"
#include "sdk/sdk_isp_def.h"
#include "app_overlay.h"
#include "cgi_hash.h"
#include "hichip_http_cgi.h"
#include "app_debug.h"
#include "socket_tcp.h"
#include <string.h>
#include "ptz.h"
#include "netsdk.h"
#include "usrm.h"
#include "nk_stream_rw.h"
#include "../api/include/http_auth/_md5.h"
#include "hichip_alarm.h"
#include "fisheye.h"
#include "model_conf.h"
#include <sys/prctl.h>
#include "ipcam_timer.h"
#include "global_runtime.h"
#include "base/ja_process.h"
#include "custom.h"

#define HICHIP_SERVER		"JuanServer"
#include <linux/tcp.h>
#define HICHIP_PARAM_TO_SET (true)
#define HICHIP_PARAM_TO_GET (false)
#define HICHIP_DEFAULT_WIDTH	1280.0
#define HICHIP_DEFAULT_HEIGHT	720.0

#define HICHIP_FRAMEHEAD_FRAMETYPE_OOB 1;

enum {
    HICHIP_FISHEYE_FRAME_TYPE_FISHEYE_PARAM = 0,
    HICHIP_FISHEYE_FRAME_TYPE_LENS_PARAM,
    HICHIP_FISHEYE_FRAME_TYPE_GSENSOR_PARAM,
    HICHIP_FISHEYE_FRAME_TYPE_CNT,
};



#define HICHIP_PARAM_SUFFIX(src, dst, set_get)					\
	do{															\
		char *ptr = NULL;										\
		if(ptr = strstr(src, "get")){							\
			dst = strdupa(src + 3);								\
			set_get = HICHIP_PARAM_TO_GET;						\
		}														\
		else if(ptr = strstr(src, "set")){						\										
			dst = strdupa(src + 3);								\
			set_get = HICHIP_PARAM_TO_SET;						\
		}														\
		else{													\
			dst = src;											\
			set_get = HICHIP_PARAM_TO_GET;						\
		}														\
	}while(0)													\

typedef struct fisheyeParam
{
	struct
	{
		int CenterCoordinateX;///// Center alignment. Relative to frame width
		int CenterCoordinateY;// Relative to frame height
		int Radius;//Radius of circle, relative to the width of a picture
		int AngleX;//
		int AngleY;//   The angle of multi camera installation
		int AngleZ;//
	}param[2];
		int fixMode;
        int version;        // 0x01000003
        int Reverse;
}ST_FISHEYE_PARAM, *LP_FISHEYE_PARAM;
		
		
typedef struct byPassFrame{
		int frameType;
		int dataSize;
}ST_BYPASS_FRAME, *LP_BYPASS_FRAME;


struct HICHIP_RTP_HDR
{
	// byte 0 
	unsigned short cc :4; // CSRC count 
	unsigned short x :1; // header extension flag 
	unsigned short p :1; // padding flag 
	unsigned short version :2; // protocol version 
	// byte 1 
	unsigned short pt :7; // payload type 
	unsigned short marker :1; // marker bit 
	// bytes 2, 3 
	unsigned short seqno :16; // sequence number 
	// bytes 4-7 
	unsigned int ts; // timestamp in ms 
	// bytes 8-11 
	unsigned int ssrc; // synchronization source 
};

struct HICHIP_RTSP_ITLEAVED_HDR
{
	unsigned char dollar; //8, $:dollar sign(24 decimal)
	unsigned char channelid; //8, channel id
	unsigned short resv; //16, reseved
	unsigned int payloadLen; //32, payload length, the sum with the frame size and the 'rtpHead'
	struct HICHIP_RTP_HDR rtpHead; //rtp head
};



static bool g_bHashInit = false;
static CGI_hashtable *g_htHiChip = NULL;
extern lpSDK_VIN_API sdk_vin;

typedef struct HICHIP_MEDIA_SESSION{
	int media_type;
}ST_HICHIP_MEDIA_SESSION,*LP_HICHIP_MEDIA_SESSION;

#define HICHIP_MEDIA_VIDEO	(1<<0)//��Ƶ 
#define HICHIP_MEDIA_AUDIO	(1<<4)//��Ƶ
#define HICHIP_MEDIA_DATA	(1<<8)//����
#define HICHIP_MEDIA_VIDEO_AUDIO	(HICHIP_MEDIA_VIDEO | HICHIP_MEDIA_AUDIO)
#define HICHIP_MEDIA_VIDEO_DATA	(HICHIP_MEDIA_VIDEO | HICHIP_MEDIA_DATA)
#define HICHIP_MEDIA_AUDIO_DATA	(HICHIP_MEDIA_AUDIO | HICHIP_MEDIA_DATA)
#define HICHIP_MEDIA_VIDEO_AUDIO_DATA (HICHIP_MEDIA_VIDEO | HICHIP_MEDIA_AUDIO | HICHIP_MEDIA_DATA)

static ST_HICHIP_MAP_STR_DEC hichip_media_map[] = {
	{"video", HICHIP_MEDIA_VIDEO},
	{"audio", HICHIP_MEDIA_AUDIO},
	{"data", HICHIP_MEDIA_DATA},
	{"video_audio", HICHIP_MEDIA_VIDEO_AUDIO},
	{"video_data", HICHIP_MEDIA_VIDEO_DATA},
	{"audio_data", HICHIP_MEDIA_AUDIO_DATA},
	{"video_audio_data", HICHIP_MEDIA_VIDEO_AUDIO_DATA},
};

static char * hichip_uri_suffix2cgi(AVal suffix, AVal param){
	char *strCgi = NULL;
	const char *strQueryParam = AVAL_STRDUPA(param);
	const char *strUriSuff = AVAL_STRDUPA(suffix);
	if(NULL != strstr(strUriSuff, "param") && NULL != (strCgi = strstr(strQueryParam, "cmd="))){
		char *tmp = strstr(strQueryParam, "&");
		int len = 0;
		if(tmp){
			len = tmp - strCgi - strlen("cmd=");
		}
		else{
			len = strlen(strCgi) - strlen("cmd=");
		}
		if(len > 0){
			tmp = calloc(sizeof(char), len  + strlen(".cgi") + 1);
			strncat(tmp, strCgi + strlen("cmd="), len);
			strcat(tmp, ".cgi");
			return tmp;
		}
	}
	else if(NULL != (strCgi = strstr(strUriSuff, "/cgi-bin/hi3510/"))){
		char * tmp = strdup(strCgi + strlen("/cgi-bin/hi3510/"));
		return tmp;
	}
	else if(NULL != (strCgi = strstr(strUriSuff, "/livestream"))){
		char *tmp = strdup(strCgi);
		return tmp;
	}
	else{
		return NULL;
	}
}



static void param_success(char* strContent, ssize_t const nLength, const char* text)
{
	snprintf(strContent, nLength, "[Success] %s"CRLF, text ? text : "");
}

static void param_error(char* strContent, ssize_t const nLength, const char* text)
{
	snprintf(strContent, nLength, "[Error] %s"CRLF, text ? text : "");
}
static int hichip_read_query2param(const char* strQuery, const char* key, AVal *ret)
{
	if(strQuery && key && strlen(key) > 0){
		char* key_with_equal = alloca(strlen(key) + 2);
		char* str_ptr = NULL;
		char *str_ch = NULL;
		ret->av_val = NULL;
		ret->av_len = 0;
		sprintf(key_with_equal, "%s=", key);
		str_ptr = strcasestr(strQuery, key_with_equal);
		if(str_ptr){
			ret->av_val = str_ptr + strlen(key_with_equal);
			str_ch = strchr(str_ptr, '&');
			if(str_ch){
				ret->av_len = str_ch - str_ptr - strlen(key_with_equal);
			}
			else{
				ret->av_len = strlen(ret->av_val);
			}

			return 0;
		}
	}
	return -1;
}


typedef struct 
{
	int max;
	int val;
} RatioValue_t;

static void hichip_ConvertParam(char *str, RatioValue_t *val)
{
	char value[128];
	char *max_value = NULL;
	strcpy(value,str);
	max_value = strstr(value, "/");
	if(max_value)
	{
		*max_value = 0;
		max_value += strlen("/");
		val->max = atoi(max_value);
		char *tmp = strstr(value, ",");
		if(tmp){
			val->val = atoi(tmp + strlen(","));
		}
	}
	else{
		val->val = -1;
		val->val = -1;
	}
}


int hichip_livemedia_cgi(const char* strQuery, char* strContent, ssize_t const nLength, int w, int h){
	uint32_t const session_id = rand();
	uint32_t const ssrc = session_id;
	snprintf(strContent, nLength,
		"Session: %d"CRLF
		"Cseq: 1"CRLF
		"m=video 96 H264/90000/%d/%d"CRLF
		"m=audio 97 G726/8000/1"CRLF
		"Transport: RTP/AVP/TCP;unicast;hisiinterleaved=0-1;ssrc=%x"CRLF
		CRLF,
		1840064387,
		w, h,
		1840064387);
	return 0;
}
int hichip_livemedia11_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	return hichip_livemedia_cgi(strQuery, strContent, nLength, 1280, 720);
}

int hichip_livemedia12_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	return hichip_livemedia_cgi(strQuery, strContent, nLength, 640, 360);
}

int hichip_identify_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{	
	SYSCONF_t *pConf = SYSCONF_dup();

	snprintf(strContent, nLength, "var productid=\"C1F0S9Z3N0P0L0\";"CRLF
										"var vendorid=\"Juan\";"CRLF);
    


	return 0;
}
int hichip_vdisplayattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){

	snprintf(strContent, nLength, "var powerfreq=\"50\";"CRLF);
	
	return 0;
}


// GET /cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=up&-speed=34 HTTP/1.1
int hichip_ptzctrl_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	AVal av_step, av_act, av_speed, av_chn;
	int n_step = -1, n_speed = -1, chn = 1,ptz_cmd = PTZ_CMD_STOP,speed_level = 0;	
	char *act = NULL;
	if(0 == hichip_read_query2param(strQuery, "step", &av_step)){
		n_step = atoi(AVAL_STRDUPA(av_step));
	}
	if(0 == hichip_read_query2param(strQuery, "act", &av_act)){
		act = AVAL_STRDUPA(av_act);
	}
	if(0 == hichip_read_query2param(strQuery, "speed", &av_speed)){
		speed_level = atoi(AVAL_STRDUPA(av_speed));
		if(speed_level > 8){
			speed_level = 8;
		}
	}
	if(0 == hichip_read_query2param(strQuery, "chn", &av_chn)){
		chn = atoi(AVAL_STRDUPA(av_chn));
	}
	if(chn < 1){
		chn = 0;
	}else{
		chn --;
	}

#if defined(UART_PROTOCOL)

	ptz_cmd = PTZ_Cmd_str2int(act);

	ST_NSDK_PTZ_CFG stPtzConfig;
	memset(&stPtzConfig, 0, sizeof(stPtzConfig));
	NETSDK_conf_ptz_ch_get(&stPtzConfig);

	if(0 == speed_level){
		speed_level = stPtzConfig.stPtzExternalConfig.nSpeed;

	}else{//speed_level != 0
		stPtzConfig.stPtzExternalConfig.nSpeed = speed_level;
		NETSDK_conf_ptz_ch_set(&stPtzConfig);
	}

	n_speed = ptz_speed_level_switch(speed_level);
	
	APP_TRACE("PTZ control channel=%d step=%d act=%s speed=%d",
		chn, n_step, act, n_speed);

		
	if((PTZ_CMD_AUTOPAN == ptz_cmd) && (0 == strcmp(stPtzConfig.stPtzExternalConfig.strptzCustomTpye,"BEISIDE"))){//for beiside
		PTZ_Send(chn, PTZ_CMD_GOTO_PRESET,99);
	}else{
		PTZ_Send(chn, ptz_cmd, n_speed);
	}
#endif

	param_success(strContent, nLength, "ptz ok");
    

	
	return 0;
}

// GET /cgi-bin/hi3510/param.cgi?cmd=getosd&-chn=1&-region=1&cmd=getosd&-chn=1&-region=0& HTTP/1.1
int hichip_osd_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	int ret = 0;

	ret = snprintf(strContent, nLength, 
		"var x_1=\"%d\";" CRLF
		"var y_1=\"%d\";" CRLF
		"var w_1=\"%d\";" CRLF
		"var h_1=\"%d\";" CRLF
		"var show_1=\"%d\";" CRLF
		"name=\"%s\";" CRLF
		"var x_0=\"%d\";" CRLF
		"var y_0=\"%d\";" CRLF
		"var w_0=\"%d\";" CRLF
		"var h_0=\"%d\";" CRLF
		"var show_0=\"%d\";" CRLF,
		0, 0, 64, 32, 1, "CAM1",
		976, 0, 304, 32, 1);
    
	
	return 0;
}

int hichip_servertime_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	int ret = 0;
	AVal server_time;

	if(bOption){
		if(0 == hichip_read_query2param(strQuery, "time", &server_time)){
			//		int year, month, mday, hour, min, sec;
			struct tm cur_time;
			const char* const str_server_time = AVAL_STRDUPA(server_time);
			ret = sscanf(str_server_time, "%04d.%02d.%02d.%02d.%02d.%02d",
				&cur_time.tm_year, &cur_time.tm_mon, &cur_time.tm_mday, &cur_time.tm_hour, &cur_time.tm_min, &cur_time.tm_sec);

			time_t timet;
			struct timeval tv;
			cur_time.tm_year -= 1900;
			cur_time.tm_mon -= 1;
			timet = mktime(&cur_time);
			tv.tv_sec = timet;
			tv.tv_usec = 0;
			settimeofday(&tv,NULL);
			param_success(strContent, nLength, "set server time"CRLF);
		}
	}
	else{
		time_t tNow;
		struct tm *tmNow;
		time(&tNow);
		SYSCONF_t *pConf = SYSCONF_dup();
		
		tmNow = localtime(&tNow);
		snprintf(strContent, nLength, "var time=\"%04d.%02d.%02d.%02d.%02d.%02d\";"CRLF
			"var timeZone=\"%d\";"CRLF
			"var dstmode=\"off\";"CRLF, tmNow->tm_year+1900, tmNow->tm_mon+1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, 
			tmNow->tm_sec, pConf->ipcam.date_time.time_zone.val);
		
	}
    
	return 0;
}



// param.cgi?cmd=getimageattr
int hichip_imageattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	RatioValue_t hue, brightness, saturation, contrast;
	AVal avHue, avBrightness, avSaturation, avContrast;
	AVal av_scene; // auto, indorr, outdoor
	AVal av_flip, av_mirror; // on, off
	char strQuery_decode[512] = {0};

	http_url_decode(strQuery, strlen(strQuery), strQuery_decode, sizeof(strQuery_decode));

	hichip_read_query2param(strQuery_decode, "hue", &avHue);
	hichip_ConvertParam(AVAL_STRDUPA(avHue), &hue);
	hichip_read_query2param(strQuery_decode, "brightness", &avBrightness);
	hichip_ConvertParam(AVAL_STRDUPA(avBrightness), &brightness);
	hichip_read_query2param(strQuery_decode, "saturation", &avSaturation);
	hichip_ConvertParam(AVAL_STRDUPA(avSaturation), &saturation);
	hichip_read_query2param(strQuery_decode, "contrast", &avContrast);
	hichip_ConvertParam(AVAL_STRDUPA(avContrast), &contrast);
	
	hichip_read_query2param(strQuery, "scene", &av_scene);
	hichip_read_query2param(strQuery, "flip", &av_flip);
	hichip_read_query2param(strQuery, "mirror", &av_mirror);
	
	

	SYSCONF_t* sysconf = SYSCONF_dup();
	if(bOption){
		// to set
		int n_hue = -1, n_brightness = -1, n_saturation = -1, n_contrast = -1, n_flip = -1, n_mirror = -1;
#if 1
		if(hue.val >= 0 && hue.val <= hue.max){
			APP_TRACE("hue.val = %d, hue.max = %d, sysmax=%d\n", hue.val, hue.max, sysconf->ipcam.vin[0].hue.max);
			n_hue = hue.val* sysconf->ipcam.vin[0].hue.max / hue.max;
			if(n_hue <= sysconf->ipcam.vin[0].hue.max){
				sysconf->ipcam.vin[0].hue.val = (SYS_U16_t)n_hue;
				SENSOR_hue_set(n_hue);
			}
		}
		//brightness Maybe overflow!!!!  so get it a limit! 
		if(brightness.val >= 0 && brightness.val <= brightness.max && brightness.val <65535){
			n_brightness = brightness.val * sysconf->ipcam.vin[0].brightness.max / brightness.max;
			if(n_brightness <= sysconf->ipcam.vin[0].brightness.max){
				sysconf->ipcam.vin[0].brightness.val = n_brightness;
				//				printf("brightness:%d\r\n", n_brightness);
				//APP_TRACE("ipcam n_brightness:%d\n", n_brightness);
				SENSOR_brightness_set(n_brightness);
			}
		}
		if(saturation.val >= 0 && saturation.val <= saturation.max){
			/*printf("saturation:%d/%d--%d/%d\r\n", 
			saturation.val, saturation.max, 
			sysconf->ipcam.vin[0].saturation.val, 
			sysconf->ipcam.vin[0].saturation.max);		*/	
			n_saturation = saturation.val * sysconf->ipcam.vin[0].saturation.max / saturation.max;
			if(n_saturation <= sysconf->ipcam.vin[0].saturation.max){
				sysconf->ipcam.vin[0].saturation.val = n_saturation;
				//APP_TRACE("ipcam n_saturation:%d\n", n_saturation);
				SENSOR_saturation_set(n_saturation);
			}
		}
		if(contrast.val >= 0 && contrast.val <= contrast.max){
			/*printf("contrast:%d/%d--%d/%d\r\n", 
			contrast.val, contrast.max, 
			sysconf->ipcam.vin[0].contrast.val, 
			sysconf->ipcam.vin[0].contrast.max);*/
			n_contrast = contrast.val * sysconf->ipcam.vin[0].contrast.max /contrast.max;
			if(n_contrast <= sysconf->ipcam.vin[0].contrast.max){
				sysconf->ipcam.vin[0].contrast.val = n_contrast;
				//APP_TRACE("ipcam n_contrast:%d\n", n_contrast);
				SENSOR_contrast_set(n_contrast);
			}
		}
		if(av_flip.av_len){
			n_flip = (0 == strcasecmp(AVAL_STRDUPA(av_flip), "on"));
			sysconf->ipcam.isp.image_attr.flip = n_flip;
			if(!n_flip){
				//				SENSOR_mirror_flip(MODE_UNFLIP);
			}else{
				//				SENSOR_mirror_flip(MODE_FLIP);
			}
		}
		if(av_mirror.av_len){
			n_mirror = (0 == strcasecmp(AVAL_STRDUPA(av_mirror), "on"));
			sysconf->ipcam.isp.image_attr.mirror = n_mirror;
			if(!n_flip){
				//				SENSOR_mirror_flip(MODE_UNMIRROR);
			}else{
				//				SENSOR_mirror_flip(MODE_MIRROR);
			}
		}
		if(n_hue != -1 && n_brightness != -1 && n_saturation != -1 
			&& n_contrast != -1 && n_flip != -1 && n_mirror != -1
			&& n_flip != -1 && n_mirror != -1){
				SYSCONF_save(sysconf);
		}
		param_success(strContent, nLength, "set image attr ok");
#endif

	}else{
		// to get

		snprintf(strContent, nLength, "var hue=\"%d\",\"%d\"/\"%d\";"CRLF
												"var brightness=\"%d\",\"%d\"/\"%d\";"CRLF
												"var saturation=\"%d\",\"%d\"/\"%d\";"CRLF
												"var contrast=\"%d\",\"%d\"/\"%d\";"CRLF
												"var scene=\"%s\";"CRLF
												"var flip=\"%s\";"CRLF
												"var mirror=\"%s\";"CRLF, 
												sysconf->ipcam.vin[0].hue.val,
												sysconf->ipcam.vin[0].hue.val,
												sysconf->ipcam.vin[0].hue.max,
												sysconf->ipcam.vin[0].brightness.val*256/sysconf->ipcam.vin[0].brightness.max,
												sysconf->ipcam.vin[0].brightness.val,
												sysconf->ipcam.vin[0].brightness.max,
												sysconf->ipcam.vin[0].saturation.val*256/sysconf->ipcam.vin[0].saturation.max,
												sysconf->ipcam.vin[0].saturation.val,
												sysconf->ipcam.vin[0].saturation.max,
												sysconf->ipcam.vin[0].contrast.val*8/sysconf->ipcam.vin[0].contrast.max,
												sysconf->ipcam.vin[0].contrast.val,
												sysconf->ipcam.vin[0].contrast.max,
												"auto",
												sysconf->ipcam.isp.image_attr.flip ? "on" : "off",
												sysconf->ipcam.isp.image_attr.mirror ? "on" : "off");
	}
    
	return 0;
}



int hichip_videoattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal videomode, vinorm;
	SYSCONF_t *pSysConf = SYSCONF_dup();
	int nVideoMode;

	
	if(bOption){
		hichip_read_query2param(strQuery, "videomode", &videomode);
		hichip_read_query2param(strQuery, "vinorm", &vinorm);
		int main_stream;
		int sub_stream;
		if(videomode.av_len){
			switch(atoi(AVAL_STRDUPA(videomode))){
				case 18:
					main_stream = SYS_VIN_SIZE_VGA; sub_stream = SYS_VIN_SIZE_QVGA;
					break;
				case 19:
					main_stream = SYS_VIN_SIZE_D1; sub_stream = SYS_VIN_SIZE_QCIF;
					break;
				case 21:
					main_stream = sub_stream = SYS_VIN_SIZE_QVGA;
					break;
				case 22:
					main_stream = SYS_VIN_SIZE_CIF; sub_stream = SYS_VIN_SIZE_QCIF;
					break;
				case 24:
					main_stream = SYS_VIN_SIZE_QCIF;  sub_stream = SYS_VIN_SIZE_CIF;
					break;
				case 25:
					main_stream = SYS_VIN_SIZE_QCIF; sub_stream = SYS_VIN_SIZE_QCIF;
					break;
				default:
					main_stream = SYS_VIN_SIZE_720P; sub_stream = SYS_VIN_SIZE_720P;
					break;
			}
			pSysConf->ipcam.vin[0].enc_h264[0].stream[0].size.val = main_stream;
			pSysConf->ipcam.vin[0].enc_h264[0].stream[1].size.val = sub_stream;
			SYSCONF_save(pSysConf);
			param_success(strContent, nLength, "set video attr ok");
		}
	}
	else{
		switch (pSysConf->ipcam.vin[0].enc_h264[0].stream[0].size.val | pSysConf->ipcam.vin[0].enc_h264[0].stream[1].size.val)
		{
		case SYS_VIN_SIZE_VGA | SYS_VIN_SIZE_QVGA:
		case SYS_VIN_SIZE_CIF | SYS_VIN_SIZE_D1:
			nVideoMode = 18;
			break;
		case SYS_VIN_SIZE_D1 | SYS_VIN_SIZE_QCIF:
			nVideoMode = 19;
			break;
		case SYS_VIN_SIZE_QVGA | SYS_VIN_SIZE_QVGA:
		case SYS_VIN_SIZE_CIF | SYS_VIN_SIZE_CIF:
			nVideoMode = 21;
			break;
		case SYS_VIN_SIZE_CIF | SYS_VIN_SIZE_QCIF:
			nVideoMode = 22;
			break;
			// 		case SYS_VIN_SIZE_QCIF | SYS_VIN_SIZE_CIF:
			// 			nVideoMode = 24;
			// 			break;
		case SYS_VIN_SIZE_QCIF | SYS_VIN_SIZE_QCIF:
			nVideoMode = 25;
			break;
		default:
			nVideoMode = 31;
		}
		snprintf(strContent, nLength, "var videomode=\"%d\";"CRLF, nVideoMode);
			
	}
    
	return 0;
}

int hichip_vencattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal chn, bps, fps, brmode, imagegrade, gop;
	SYSCONF_t *pSysConf = SYSCONF_dup();

	hichip_read_query2param(strQuery, "chn", &chn);
	int stream_ch = atoi(AVAL_STRDUPA(chn)) - 11;

	if(bOption){
		ST_NSDK_VENC_CH venc_ch;
		hichip_read_query2param(strQuery, "bps", &bps);
		hichip_read_query2param(strQuery, "fps", &fps);
		hichip_read_query2param(strQuery, "brmode", &brmode);
		hichip_read_query2param(strQuery, "imagegrade", &imagegrade);
		hichip_read_query2param(strQuery, "gop", &gop);
		if(NETSDK_conf_venc_ch_get(100+stream_ch +1, &venc_ch)){
			if(bps.av_len){
				venc_ch.constantBitRate = atoi(AVAL_STRDUPA(bps));
				APP_TRACE("BPS:%d", venc_ch.constantBitRate);
			}
			if(fps.av_len){
				venc_ch.frameRate = atoi(AVAL_STRDUPA(fps));
			}
			if(brmode.av_len){
				venc_ch.bitRateControlType = atoi(AVAL_STRDUPA(brmode));
			}
			if(imagegrade.av_len){
				//FIX ME
			}
			if(gop.av_len){
				//FIX ME
			}

			NETSDK_conf_venc_ch_set(100+stream_ch +1, &venc_ch);
			netsdk_venc_ch_changed(100+stream_ch +1, &venc_ch);
			param_success(strContent, nLength, "set venc attr success");
		}else{
			param_error(strContent, nLength, "set venc attr error");
		}		
	}
	else{
		int width, height;
		APP_TRACE("stream_ch:%d\n", stream_ch);
		ST_NSDK_VENC_CH venc_ch;
		if(NETSDK_conf_venc_ch_get(100+stream_ch +1, &venc_ch)){
			if(venc_ch.freeResolution){
				width = venc_ch.resolutionWidth;
				height = venc_ch.resolutionHeight;
			}else{
				width = (venc_ch.resolution >> 16) & 0xffff;
				height = (venc_ch.resolution >> 0) & 0xffff;
			}
			snprintf(strContent, nLength, "var bps_%d=\"%d\";"CRLF
																"var fps_%d=\"%d\";"CRLF
																"var gop_%d=\"%d\";"CRLF
																"var brmode_%d=\"%d\";"CRLF
																"var imagegrade_%d=\"%d\";"CRLF
																"var width_%d=\"%d\";"CRLF
																"var height_%d=\"%d\";"CRLF, stream_ch+1, 
																venc_ch.constantBitRate, 
																stream_ch+1, venc_ch.frameRate, 
																stream_ch+1, venc_ch.keyFrameInterval, 
																stream_ch+1, venc_ch.bitRateControlType, 
																stream_ch+1, 1, 
																stream_ch+1, width,
																stream_ch+1, height);
		}
// 		sprintf(strContent, "HTTP/1.0 200 OK\r\n"
// 			"Server: HiIpcam\r\n"
// 			"Cache-Control: no-cache\r\n"
// 			"Content-Type:text/html\r\n"
// 			"Connection: close\r\n"
// 			"Content-Length: %d\r\n\r\n", strlen(item));

	}
	
	return 0;
}

int hichip_aencattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] aencattr ok");
	
	return 0;
}

int hichip_audioinvolume_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] audioinvolume ok");
	
	return 0;
}

static void hichip_channel_name_set()
{
	TICKER_del_task(hichip_channel_name_set); // only run once!
	APP_OVERLAY_create_title(0);
}

static int hichip_channel_name_create()
{
	TICKER_del_task(hichip_channel_name_set);
	TICKER_add_task(hichip_channel_name_set, 3, false);
}

int hichip_channelName_set()
{
	int vin = 0;
	APP_OVERLAY_release_title(vin);
	hichip_channel_name_create();
	return 0;
	//}
}


int hichip_overlayattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	AVal av_region, av_show, av_name;
	int n_region = 0;
	int n_show = 0;
	char* str_name = NULL;
	SYSCONF_t* sysconf = SYSCONF_dup();

	if(bOption){
		if(0 == hichip_read_query2param(strQuery, "region", &av_region)){
			n_region = atoi(AVAL_STRDUPA(av_region));
		}
		if(0 == hichip_read_query2param(strQuery, "show", &av_show)){
			n_show = atoi(AVAL_STRDUPA(av_show));
		}
		if(0 == n_region){
			//APP_OVERLAY_show_clock(0, 0, n_show);
		}
		else{
			//APP_OVERLAY_show_title(0, 0, n_show);
		}
		// FIXME: remove temprarily
		
		if(0 == hichip_read_query2param(strQuery, "name", &av_name)){
			str_name = AVAL_STRDUPA(av_name);APP_TRACE("setoverlay_channel\n");
			int stream_vin = 1,i = 0;
			APP_TRACE("channel name: %s", str_name);
			for(i = 0; i < stream_vin; i++){
				ST_NSDK_VENC_CH venc_ch;
				if(NETSDK_conf_venc_ch_get(100+ i +1, &venc_ch)){
					strncpy(venc_ch.channelName, str_name, sizeof(venc_ch.channelName));
					NETSDK_conf_venc_ch_set(100+i +1, &venc_ch);
					hichip_channelName_set();
				}
			}
		}
		param_success(strContent, nLength, " set overlay attr ok");
	}
	else{
		// FIXME: remove temprarily
		//snprintf(strContent, nLength, "var show=\"1\";"CRLF"name=\"%s\";"CRLF, sysconf->ipcam.vin[0].channel_name);
	}
    
	return 0;
}


int hichip_netattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	SYSCONF_t *pSysConf = SYSCONF_dup();
	AVal dhcpflag, ip, netmask, gateway, fdnsip, sdnsip;

	if(bOption){
		hichip_read_query2param(strQuery, "dhcp", &dhcpflag);
		hichip_read_query2param(strQuery, "ipaddr", &ip);
		hichip_read_query2param(strQuery, "netmask", &netmask);
		hichip_read_query2param(strQuery, "gateway", &gateway);
		hichip_read_query2param(strQuery, "fdnsip", &fdnsip);
		hichip_read_query2param(strQuery, "sdnsip", &sdnsip);
		if(dhcpflag.av_len){
			pSysConf->ipcam.network.lan.dhcp = (0 == strcasecmp(AVAL_STRDUPA(dhcpflag), "on"));
		}
		if(ip.av_len){
			pSysConf->ipcam.network.lan.static_ip.s_addr = inet_addr(AVAL_STRDUPA(ip));
		}
		if(netmask.av_len){
			pSysConf->ipcam.network.lan.static_netmask.s_addr = inet_addr(AVAL_STRDUPA(netmask));
		}
		if(gateway.av_len){
			pSysConf->ipcam.network.lan.static_gateway.s_addr = inet_addr(AVAL_STRDUPA(gateway));
		}
		if(fdnsip.av_len){
			pSysConf->ipcam.network.lan.static_preferred_dns.s_addr = inet_addr(AVAL_STRDUPA(fdnsip));
		}
		if(sdnsip.av_len){
			pSysConf->ipcam.network.lan.static_alternate_dns.s_addr = inet_addr(AVAL_STRDUPA(sdnsip));
		}
		SYSCONF_save(pSysConf);
		param_success(strContent, nLength, "set net attr success");
	}
	else{
		char mac[18] = {0};
		sprintf(mac, "%x:%x:%x:%x:%x:%x", pSysConf->ipcam.network.mac.s[0], 
											pSysConf->ipcam.network.mac.s[1],
											pSysConf->ipcam.network.mac.s[2],
											pSysConf->ipcam.network.mac.s[3],
											pSysConf->ipcam.network.mac.s[4],
											pSysConf->ipcam.network.mac.s[5]);
		char strIP[16] = {0}; snprintf(strIP, sizeof(strIP), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_ip.in_addr));
		char strNetmask[16] = {0}; snprintf(strNetmask, sizeof(strNetmask), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_netmask.in_addr));
		char strGateway[16] = {0}; snprintf(strGateway, sizeof(strGateway), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_gateway.in_addr));
		char strFdns[16] = {0}; snprintf(strFdns, sizeof(strFdns), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_preferred_dns.in_addr));
		char strSdns[16] = {0}; snprintf(strSdns, sizeof(strSdns), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_alternate_dns.in_addr));
		
		snprintf(strContent, nLength, "var dhcpflag=\"%s\";"CRLF
										"var ip=\"%s\";"CRLF
										"var netmask=\"%s\";"CRLF
										"var gateway=\"%s\";"CRLF
										"var fdnsip=\"%s\";"CRLF
										"var sdnsip=\"%s\";"CRLF
										"var macaddress=\"%s\";"CRLF, 
										pSysConf->ipcam.network.lan.dhcp ? "on" : "off",
										strIP,
										strNetmask,
										strGateway,
										strFdns,
										strSdns, mac);
	}
    
	return 0;
}

int hichip_httpport_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal httpport;
	SYSCONF_t *pSysConf = SYSCONF_dup();

	if(bOption){
		hichip_read_query2param(strQuery, "httpport", &httpport);
		pSysConf->ipcam.network.lan.port[0].value = atoi(AVAL_STRDUPA(httpport));
		SYSCONF_save(pSysConf);
		exit(0);
		param_success(strContent, nLength, "set httpport ok");
	}
	else{
		snprintf(strContent, nLength, "var httpport=\"%d\";"CRLF, pSysConf->ipcam.network.lan.port[0].value);
	}
    
	return 0;
}

int hichip_rtspport_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal rtspport;
	SYSCONF_t *pSysConf = SYSCONF_dup();

	if(bOption){
		hichip_read_query2param(strQuery, "rtspport", &rtspport);
		pSysConf->ipcam.network.lan.port[0].value = atoi(AVAL_STRDUPA(rtspport));
		SYSCONF_save(pSysConf);
		exit(0);
		param_success(strContent, nLength, "set rtspport ok");
	}
	else{
		snprintf(strContent, nLength, "var rtspport=\"%d\";"CRLF, pSysConf->ipcam.network.lan.port[0].value);
	}
    
	return 0;
}

int hichip_infrared_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal infrared;
	SYSCONF_t *pConf = SYSCONF_dup();


	if(bOption){
		hichip_read_query2param(strQuery, "infraredstat", &infrared);
		int mode = 0;
		if(AVSTRMATCH(&infrared, "auto")){
			mode = ISP_IRCUT_MODE_AUTO;
		}
		else if(AVSTRCASEMATCH(&infrared, "open")){
			mode = ISP_IRCUT_MODE_NIGHT;
		}
		else if(AVSTRCASEMATCH(&infrared, "close")){
			mode = ISP_IRCUT_MODE_DAYLIGHT;
		}
		pConf->ipcam.isp.day_night_mode.ircut_mode = mode;
		SENSOR_ircut_mode_set(mode);
		SYSCONF_save(pConf);
		param_success(strContent, nLength, "set infrared ok!");
	}
	else{
		char mode[8] = {0};
		switch(pConf->ipcam.isp.day_night_mode.ircut_mode){
			case ISP_IRCUT_MODE_AUTO:
				strcat(mode, "auto"); break;
			case ISP_IRCUT_MODE_NIGHT:
				strcat(mode, "open"); break;
			case ISP_IRCUT_MODE_DAYLIGHT:
				strcat(mode, "close"); break;
			default:
				break;
		}
		snprintf(strContent, nLength, "var infraredstat=\"%s\";"CRLF, mode);
	}
    
    return 0;
}

int hichip_upnpattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal upnp;
	SYSCONF_t *pSysConf = SYSCONF_dup();

	if(bOption){
		hichip_read_query2param(strQuery, "upm_enable", &upnp);
		pSysConf->ipcam.network.lan.upnp = atoi(AVAL_STRDUPA(upnp));
		SYSCONF_save(pSysConf);
		param_success(strContent, nLength, "set upnp_enable ok");
	}
	else{
		snprintf(strContent, nLength, "var upm_enable=\"%d\";"CRLF, pSysConf->ipcam.network.lan.upnp);
	}
    
	return 0;
}

int hichip_3thddnsattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal enable, service, uname, passwd, domain;
	SYSCONF_t *pConf = SYSCONF_dup();

	if(bOption){
		hichip_read_query2param(strQuery, "d3th_enable", &enable);
		hichip_read_query2param(strQuery, "d3th_service", &service);
		hichip_read_query2param(strQuery, "d3th_uname", &uname);
		hichip_read_query2param(strQuery, "d3th_passwd", &passwd);
		hichip_read_query2param(strQuery, "d3th_domain", &domain);
		pConf->ipcam.network.ddns.enable = atoi(AVAL_STRDUPA(enable));
		snprintf(pConf->ipcam.network.ddns.username, sizeof(pConf->ipcam.network.ddns.username), AVAL_STRDUPA(uname));
		snprintf(pConf->ipcam.network.ddns.password, sizeof(pConf->ipcam.network.ddns.password), AVAL_STRDUPA(passwd));
		snprintf(pConf->ipcam.network.ddns.url, sizeof(pConf->ipcam.network.ddns.url), AVAL_STRDUPA(domain));
		SYSCONF_save(pConf);
		param_success(strContent, nLength, "set ddns ok");
	}
	else{
		snprintf(strContent, nLength, "var d3th_enable=\"%d\";"CRLF
											"var d3th_service=\"%d\";"CRLF
											"var d3th_uname=\"%s\";"CRLF
											"var d3th_passwd=\"%s\";"CRLF
											"var d3th_domain=\"%s\";"CRLF, 
											pConf->ipcam.network.ddns.enable,
											pConf->ipcam.network.ddns.provider,
											pConf->ipcam.network.ddns.username,
											pConf->ipcam.network.ddns.password,
											pConf->ipcam.network.ddns.url);
	}
    
	return 0;
}

int hichip_serverinfo_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){


	SYSCONF_t *pConf = SYSCONF_dup();
	snprintf(strContent, nLength, "var model=\"%s\";"CRLF
										"var hardVersion=\"%s\";"CRLF
										"var softVersion=\"%s\";"CRLF
										"var name=\"%s\";"CRLF,
										pConf->ipcam.info.device_model,
										pConf->ipcam.info.hardware_version,
										pConf->ipcam.info.software_version,
										pConf->ipcam.info.device_name);
    
	return 0;
}

int hichip_mdattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal avEnable, avSensitivity, avName, avX, avY, avWidth, avHeight;
	SYSCONF_t *pConf = SYSCONF_dup();

	char strQuery_decode[512] = {0};
	http_url_decode(strQuery, strlen(strQuery), strQuery_decode, sizeof(strQuery_decode));
	if(bOption){
		hichip_read_query2param(strQuery_decode, "enable", &avEnable);
		hichip_read_query2param(strQuery_decode, "s", &avSensitivity);
		hichip_read_query2param(strQuery_decode, "name", &avName);
		hichip_read_query2param(strQuery_decode, "x", &avX);
		hichip_read_query2param(strQuery_decode, "y", &avY);
		hichip_read_query2param(strQuery_decode, "w", &avWidth);
		hichip_read_query2param(strQuery_decode, "h", &avHeight);
		RatioValue_t ratioX, ratioY, ratioWdith, ratioHeight;
		hichip_ConvertParam(AVAL_STRDUPA(avX), &ratioX);
		hichip_ConvertParam(AVAL_STRDUPA(avY), &ratioY);
		hichip_ConvertParam(AVAL_STRDUPA(avWidth), &ratioWdith);
		hichip_ConvertParam(AVAL_STRDUPA(avHeight), &ratioHeight);

		if(1 == atoi(AVAL_STRDUPA(avEnable))){
			float x = (float)ratioX.val / 1280.0;
			float y = (float)ratioY.val / 720.0;
			float Width = (float)ratioWdith.val / 1280.0;
			float Height = (float)ratioHeight.val / 720.0;
			float Threshold = 1.0 / (float)atoi(AVAL_STRDUPA(avSensitivity));

			APP_TRACE("x:%f, y:%f, w:%f, h:%f, s:%d\n", x, y, Width, Height, atoi(AVAL_STRDUPA(avSensitivity)));
			//APP_MD_clear_mask(0);
			//APP_MD_add_rect_mask(0, x, y, Width, Height);
			//if(0 == sdk_vin->check_md_rect(0, AVAL_STRDUPA(avName), NULL, NULL, NULL, NULL, NULL)){
				if(0 != sdk_vin->add_md_rect(0, AVAL_STRDUPA(avName), x, y, Width, Height, Threshold)){
					param_error(strContent, nLength, "set mdattr error");
				}
			//}

		}
		param_success(strContent, nLength, "set mdattr ok");
	}
	else{
		char item[256] = {0};
		int i;
		for(i = 0; i < 4; ++i){
			char strName[5] = {0};
			snprintf(strName, sizeof(strName), "%d", i + 1);
			float xRatio, yRatio, wRatio, hRatio, Threshold;
			int nX, nY, nWidth, nHeight;
			int nSensitivity;
			if(1 == sdk_vin->check_md_rect(0, strName, &xRatio, &yRatio, &wRatio, &hRatio, &Threshold)){
				nSensitivity = 1.0 / Threshold;
				nX = xRatio * 1280;
				nY = yRatio * 720;
				nWidth = wRatio * 1280;
				nHeight = hRatio * 720;
				memset(item, 0, sizeof(item));
				snprintf(item, sizeof(item), "var m%d_enable=\"%d\";"CRLF
											"var m%d_x=\"%d,%d/1280\";"CRLF
											"var m%d_y=\"%d,%d/720\";"CRLF
											"var m%d_w=\"%d,%d/1280\";"CRLF
											"var m%d_h=\"%d,%d/720\";"CRLF
											"var m%d_sensitivity=\"%d\";"CRLF,
											i + 1, 1,
											i + 1, nX, nX,
											i + 1, nY, nY,
											i + 1, nWidth, nWidth, 
											i + 1, nHeight, nHeight,
											i + 1, nSensitivity);
				strncat(strContent, item, nLength - strlen(strContent));					
				APP_TRACE("name:%s, item:%s:\n", strName, item);
			}
			else{
				APP_TRACE("check md failed\n");
			}
		}
		if(0 == strlen(strContent)){
			strncat(strContent, "no motion detect", nLength);
		}
	}
	
    
    return 0;
}
int hichip_ioattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal avEnable, avFlag;

	if(bOption){
		hichip_read_query2param(strQuery, "io_enable", &avEnable);
		hichip_read_query2param(strQuery, "io_flag", &avFlag);

		param_success(strContent, nLength, "set ioattr ok");
	}
	else{
		param_success(strContent, nLength, "get ioattr ok");
		/*snprintf(content, , "var io_enable=\"%d\";"CRLF
											"var io_flag=\"%d\";"CRLF,
											, , );
*/
	}
	
	return 0;
}
int hichip_mdalarm_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal avAname, avEmailSwitch, avEmailSnap, avSnapSwitch, avRecordSwitch, avFtpSwitch, avRelaySwitch;

	if(bOption){
		hichip_read_query2param(strQuery, "aname", &avAname);
		hichip_read_query2param(strQuery, "md_email_switch", &avEmailSwitch);
		hichip_read_query2param(strQuery, "md_emailsnap_switch", &avEmailSnap);
		hichip_read_query2param(strQuery, "md_snap_switch", &avSnapSwitch);
		hichip_read_query2param(strQuery, "md_record_switch", &avRecordSwitch);
		hichip_read_query2param(strQuery, "md_ftprec_switch", &avFtpSwitch);
		hichip_read_query2param(strQuery, "md_relay_switch", &avRelaySwitch);

		param_success(strContent , nLength, "set mdalarm ok");
	} 
	else{
		snprintf(strContent, nLength, "get mdalarm ok");
	}
	
	return 0;
}
int hichip_schedule_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "no schedule");
	
	return 0;
}
int hichip_devtype_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	uint32_t type = 0x0007 << 8;
	snprintf(strContent, nLength, "var devtype=\"%04x\";", type);
	
	return 0;
}
int hichip_coverattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal avEnable, avName, avX, avY, avWidth, avHeight, avColor;
	SYSCONF_t *pConf = SYSCONF_dup();
	char strQuery_decode[512] = {0};
	http_url_decode(strQuery, strlen(strQuery), strQuery_decode, sizeof(strQuery_decode));
	if(bOption){
		hichip_read_query2param(strQuery_decode, "enable", &avEnable);
		hichip_read_query2param(strQuery_decode, "name", &avName);
		hichip_read_query2param(strQuery_decode, "x", &avX);
		hichip_read_query2param(strQuery_decode, "y", &avY);
		hichip_read_query2param(strQuery_decode, "w", &avWidth);
		hichip_read_query2param(strQuery_decode, "h", &avHeight);
		hichip_read_query2param(strQuery_decode, "color", &avColor);
		RatioValue_t ratioX, ratioY, ratioWdith, ratioHeight;
		hichip_ConvertParam(AVAL_STRDUPA(avX), &ratioX);
		hichip_ConvertParam(AVAL_STRDUPA(avY), &ratioY);
		hichip_ConvertParam(AVAL_STRDUPA(avWidth), &ratioWdith);
		hichip_ConvertParam(AVAL_STRDUPA(avHeight), &ratioHeight);
		if(1 == atoi(AVAL_STRDUPA(avEnable))){
			float x = (float)ratioX.val / 1280.0; // FIXME:
			float y = (float)ratioY.val / 720.0; // FIMXE:
			float Width = (float)ratioWdith.val / 1280.0; // FIMXE:
			float Height = (float)ratioHeight.val / 720.0; // FIMXE:
			//char *pColor = strlwr(AVAL_STRDUPA(avColor));
			uint32_t nColor = 0;
			sscanf(AVAL_STRDUPA(avColor), "%06x", &nColor);
			APP_TRACE("%f, %f, %f, %f, color#%s:%06x\n", x, y, Width, Height, AVAL_STRDUPA(avColor), nColor);
			// FIXME:
			ST_SDK_VIN_COVER_ATTR cover_attr;
			cover_attr.x = x;
			cover_attr.y = y;
			cover_attr.width = Width;
			cover_attr.height = Height;
			cover_attr.color = nColor;
			if(0 != sdk_vin->set_cover(0, atoi(AVAL_STRDUPA(avName)), &cover_attr)){
				param_error(strContent, nLength, "set coverattr error");
			}

		}
		param_success(strContent, nLength, "set coverattr ok");
	}
	else{
		char item[256] = {0};
		int i;
		for(i = 0; i < 4; ++i){
			char strName[5] = {0};
			snprintf(strName, sizeof(strName), "%d", i + 1);
			float xRatio, yRatio, wRatio, hRatio;
			int nX, nY, nWidth, nHeight;
			uint32_t nColor;
			ST_SDK_VIN_COVER_ATTR cover_attr;
			if(0 == sdk_vin->get_cover(0, atoi(strName), &cover_attr)){
				nX = xRatio * 1280;
				nY = yRatio * 720;
				nWidth = wRatio * 1280;
				nHeight = hRatio * 720;
				memset(item, 0, sizeof(item));
				snprintf(item, sizeof(item), "var c%d_enable=\"%d\";"CRLF
											"var c%d_x=\"%d,%d/1280\";"CRLF
											"var c%d_y=\"%d,%d/720\";"CRLF
											"var c%d_w=\"%d,%d/1280\";"CRLF
											"var c%d_h=\"%d,%d/720\";"CRLF
											"var c%d_color=\"%06x\";"CRLF,
											i + 1, 1,
											i + 1, nX, nX,
											i + 1, nY, nY,
											i + 1, nWidth, nWidth, 
											i + 1, nHeight, nHeight,
											i + 1, nColor);
				strncat(strContent, item, nLength - strlen(strContent));					
				APP_TRACE("name:%s, item:%s:\n", strName, item);

			}
		}
		if(0 == strlen(strContent)){
			strncat(strContent, "no cover", nLength);
		}
	}
    
    return 0;
}
int hichip_preset_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] preset ok");

	AVal av_status, av_act, av_number, av_chn;
	int n_status = -1, n_number = -1, chn = 1, ptz_cmd = PTZ_CMD_STOP;
	char *act = NULL;
	if(0 == hichip_read_query2param(strQuery, "status", &av_status)){
		n_status = atoi(AVAL_STRDUPA(av_status));
	}
	if(0 == hichip_read_query2param(strQuery, "act", &av_act)){
		act = AVAL_STRDUPA(av_act);
	}
	if(0 == hichip_read_query2param(strQuery, "number", &av_number)){
		n_number = atoi(AVAL_STRDUPA(av_number));
	}
	if(0 == hichip_read_query2param(strQuery, "chn", &av_chn)){
		chn = atoi(AVAL_STRDUPA(av_chn));
	}
	if(chn < 1){
		chn = 0;
	}else{
		chn --;
	}

	if(!strcmp("set", act)){
		if(n_status == 1){
			ptz_cmd = PTZ_CMD_SET_PRESET;
		}else if(n_status == 0){
			ptz_cmd = PTZ_CMD_CLEAR_PRESET;
		}else{
			
		}
	}else if(!strcmp("goto", act)){
			ptz_cmd = PTZ_CMD_GOTO_PRESET;
	}else if(!strcmp("auto", act)){
		ptz_cmd = PTZ_CMD_AUTOPAN;
	}else{

	}

	APP_TRACE("PTZ control channel=%d status=%d act=%s, n_number=%d",
		chn, n_status, act, n_number);


	
	ST_NSDK_PTZ_CFG stPtzConfig;
	memset(&stPtzConfig, 0, sizeof(stPtzConfig));
	NETSDK_conf_ptz_ch_get(&stPtzConfig);
	
	if((ptz_cmd == PTZ_CMD_AUTOPAN) && (0 == strcmp(stPtzConfig.stPtzExternalConfig.strptzCustomTpye,"BEISIDE"))){
		if(1 == n_status){
			PTZ_Send(chn, PTZ_CMD_GOTO_PRESET,99);
		}else if(0 == n_status){
			PTZ_Send(chn, PTZ_CMD_STOP,0);
		}	
	}else{
		PTZ_Send(chn, ptz_cmd, n_number);
	}

	param_success(strContent, nLength, "ptz ok");
    

	
	return 0;
}
int hichip_ptzcomattr(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzcomattr ok");
	
    return 0;
}
int hichip_ptzup_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzup ok");
	
    return 0;
}
int hichip_ptzdown_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzdown ok");
	
    return 0;
}
int hichip_ptzleft_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
 	snprintf(strContent, nLength, "[Success] ptzleft ok");
	
    return 0;
}
int hichip_ptzright_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzright ok");
	
    return 0;
}
int hichip_ptzzoomin_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzzoomin ok");
	
    return 0;
}
int hichip_ptzzoomout_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzzoomout ok");
	
    return 0;
}

//define in app_hichip.c
extern char g_hichip_nonce[36];
int hichip_get_nonce_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "%s", g_hichip_nonce);
	APP_TRACE("%s", strContent);
	return 0;
}


void HICHIP_http_init(){
	g_htHiChip = CGI_hashAdd(g_htHiChip, "identify.cgi", hichip_identify_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "vdisplayattr.cgi", hichip_vdisplayattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "/livestream/11", hichip_livemedia11_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "/livestream/12", hichip_livemedia12_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzctrl.cgi", hichip_ptzctrl_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "servertime.cgi", hichip_servertime_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "imageattr.cgi", hichip_imageattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "videoattr.cgi", hichip_videoattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "vencattr.cgi", hichip_vencattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "aencattr.cgi", hichip_aencattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "audioinvolume.cgi", hichip_audioinvolume_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "overlayattr.cgi", hichip_overlayattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "netattr.cgi", hichip_netattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "httpport.cgi", hichip_httpport_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "rtspport.cgi", hichip_rtspport_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "infrared.cgi", hichip_infrared_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "upnpattr.cgi", hichip_upnpattr_cgi);	
	g_htHiChip = CGI_hashAdd(g_htHiChip, "3thddnsattr.cgi", hichip_3thddnsattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "serverinfo.cgi", hichip_serverinfo_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "mdattr.cgi", hichip_mdattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ioattr.cgi", hichip_ioattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "mdalarm.cgi", hichip_mdalarm_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "schedule.cgi", hichip_schedule_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "devtype.cgi", hichip_devtype_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "coverattr.cgi", hichip_coverattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "preset.cgi", hichip_preset_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzcomattr.cgi", hichip_ptzcomattr);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzup.cgi", hichip_ptzup_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzdown.cgi", hichip_ptzdown_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzleft.cgi", hichip_ptzleft_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzright.cgi", hichip_ptzright_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzzoomin.cgi", hichip_ptzzoomin_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzzoomout.cgi", hichip_ptzzoomout_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "nonce.cgi", hichip_get_nonce_cgi);
}

void HICHIP_http_destroy(){
	CGI_hashDestroy(g_htHiChip);
	g_htHiChip = NULL;
}
int HICHIP_http_cgi(HTTPD_SESSION_t* session)
{
	session->keep_alive = 0;
	int ret = 0;
	char body[2048] = {0};
	char *dstCmd = NULL;
	char *strCgi = hichip_uri_suffix2cgi(session->request_line.uri_suffix, session->request_line.uri_query_string);
	//APP_TRACE("suffix:%s, param:%s\n", AVAL_STRDUPA(session->request_line.uri_suffix),AVAL_STRDUPA(session->request_line.uri_query_string));
	if(strCgi){
		int set_get = 2;
		HICHIP_PARAM_SUFFIX(strCgi, dstCmd, set_get);
		if(0 == strcmp(strCgi, "preset.cgi")){
			dstCmd = strCgi;
		}
		CGI_hashtable *pHashItem = CGI_hashFind(g_htHiChip, dstCmd);
		if(pHashItem){
			pHashItem->pCgiFunc(AVAL_STRDUPA(session->request_line.uri_query_string), body, ARRAY_ITEM(body), set_get);
		}
		else{
			APP_TRACE("cgi not found!\n");
		}
		
	}
	else{
		APP_TRACE("get cgi failed\n");
	}

	// send the response content
	APP_TRACE("%s:%d", strCgi, strlen(body));
	if(strCgi && strlen(body) > 0){
		char content[2048*2] = {0};
		int bLiveMedia = (NULL != strstr(dstCmd, "/livestream")) ? 1 : 0; 
		HTTP_HEADER_t* httpHeader = NULL;
		SYSCONF_t *pConf = SYSCONF_dup();
		if(1 == bLiveMedia){
			httpHeader = http_response_header_new("1.1", 200, NULL);                      
			httpHeader->add_tag_text(httpHeader, "Host", inet_ntoa(pConf->ipcam.network.lan.static_ip.in_addr));
			httpHeader->add_tag_text(httpHeader, "Connection", "Keep-Alive");		           
			httpHeader->add_tag_text(httpHeader, "Server", HICHIP_SERVER);
			httpHeader->add_tag_text(httpHeader, "Cache-Control", "no-cache");
			httpHeader->add_tag_text(httpHeader, "Accept-Ranges", "Bytes");
			httpHeader->add_tag_text(httpHeader, "Content-Type", "application/octet-stream");
			httpHeader->add_tag_text(httpHeader, "Connection", "close");
		}
		else{
			httpHeader = http_response_header_new("1.1", 200, NULL);                      
			httpHeader->add_tag_text(httpHeader, "Server", HICHIP_SERVER);
			httpHeader->add_tag_text(httpHeader, "Cache-Control", "no-cache");  
			httpHeader->add_tag_text(httpHeader, "Content-Type", "text/html");
			httpHeader->add_tag_text(httpHeader, "Connection", "close");      
			httpHeader->add_tag_int(httpHeader, "Content-Length", strlen(body));     
		}
		httpHeader->to_text(httpHeader, content, sizeof(content));                  	   
		http_response_header_free(httpHeader);                                        
		httpHeader = NULL;                                                            
		strncat(content, body, strlen(body));                     		   		
		ret = send(session->sock, content, strlen(content), 0);
		*session->trigger = false;
		free(strCgi);
		strCgi = NULL;
		if(ret < 0){
			APP_TRACE("Response  error : %s", strerror(errno));
			return -1;
		}
		return 0;
	}
	return -1;
}

#define HICHIP_SEND_MD_STATUS_TIMES (3)
#define HICHIP_SEND_IO_ALARM_STATUS_TIMES (1)
#define HICHIP_IO_ALARM_STATUS_DURATION (3)

static bool gs_md_status = false;
time_t gs_md_timestamp = 0;
static int hichip_media_type = HICHIP_MEDIA_VIDEO;
static bool gs_io_alarm_status = false;
time_t gs_io_alarm_timestamp = 0;

int HICHIP_set_md_status(bool flag)
{
	gs_md_status = flag;
	gs_md_timestamp = time(NULL);
}

bool HICHIP_get_md_status()
{
	return gs_md_status;
}

bool HICHIP_get_io_alarm_status()
{
	return gs_io_alarm_status;
}

int HICHIP_set_io_alarm_status(bool flag)
{
	
	if(!flag || !gs_io_alarm_status){
		//if pir is off before	
		gs_io_alarm_timestamp = time(NULL);
	}else{
		
	}
	gs_io_alarm_status = flag;
	return 0;
}

static int hichip_send_md_status(lpSOCKET_TCP tcp, 
								struct HICHIP_RTSP_ITLEAVED_HDR frameHeader, 
								int x, int y, int w, int h)
{
	char md_status_str[128];
	int ret = 0;
	sprintf(md_status_str, "1-%d-%d-%d-%d", x, y, w, h);
	int rawDataLen = strlen(md_status_str);
	struct HICHIP_RTP_HDR* const srtpHeader = &frameHeader.rtpHead;
	srtpHeader->pt = 100;
	frameHeader.payloadLen = htonl(strlen(md_status_str) + sizeof(frameHeader.rtpHead));
	//APP_TRACE("send md status!");
	if(0 != srtpHeader->pt){
		ret = tcp->send2(tcp, &frameHeader, sizeof(frameHeader), 0);
		if(ret < 0){
			// FIXME:
			APP_TRACE("send frameheader failed  %d", frameHeader.payloadLen);
			return 0;
		}
		ret = tcp->send2(tcp, md_status_str, strlen(md_status_str), 0);
		if(ret < 0){
			// FIXME:
			APP_TRACE("send payload failed");
		}
	}
	return 0;
}

static int hichip_send_io_alarm_status(lpSOCKET_TCP tcp, 
								struct HICHIP_RTSP_ITLEAVED_HDR frameHeader, int alarm_index,
								char *alarm_type)
{
	char io_status_str[128];
	int ret = 0;
	snprintf(io_status_str, sizeof(io_status_str), "%d-%s", alarm_index, alarm_type);
	int rawDataLen = strlen(io_status_str);
	struct HICHIP_RTP_HDR* const srtpHeader = &frameHeader.rtpHead;
	srtpHeader->pt = 101;//io alarm type
	frameHeader.payloadLen = htonl(strlen(io_status_str) + sizeof(frameHeader.rtpHead));
	APP_TRACE("send %s status!", alarm_type);
	if(0 != srtpHeader->pt){
		ret = tcp->send2(tcp, &frameHeader, sizeof(frameHeader), 0);
		if(ret < 0){
			// FIXME:
			APP_TRACE("send frameheader failed  %d", frameHeader.payloadLen);
			return 0;
		}
		ret = tcp->send2(tcp, io_status_str, strlen(io_status_str), 0);
		if(ret < 0){
			// FIXME:
			APP_TRACE("send payload failed");
		}
	}
	return 0;
}

int hichip_send_md_io_status(int md_flag, int io_alarm_flag, lpSOCKET_TCP tcp, struct HICHIP_RTSP_ITLEAVED_HDR frameHeader)
{
	stNK_HICHIP_ALARM_HEADER alarm_header;
	stNK_HICHIP_ALARM_BOUNDARY boundary;
	char data[1024];
	stNK_STREAM_RW rwops;
	int ret;
	char sum_buffer[100];
	struct HICHIP_RTP_HDR* const srtpHeader = &frameHeader.rtpHead;
	struct MD5Context stmd5_context;
	int total_size = 0;
	
	snprintf(sum_buffer, sizeof(sum_buffer), "%s", "_+N1ALARM+CSUM+_");
	MD5Init(&stmd5_context);
	MD5Update(&stmd5_context, sum_buffer, strlen(sum_buffer));

	memset(&alarm_header, 0, sizeof(alarm_header));
	memset(&rwops, 0, sizeof(stNK_STREAM_RW));
	NK_STREAM_RWFromMem(&rwops, data + sizeof(alarm_header), 
                    	sizeof(data) - sizeof(alarm_header), 0);
	if(md_flag){
		//md_data
		stNK_HICHIP_MD_DATA md_data;
		md_data.chn = htonl(0);
		
		boundary.type = htonl(NK_HICHIP_ALARM_MD);
		NK_STREAM_RWwrite(&rwops, &boundary, 1, sizeof(boundary));
		//fill md data
		NK_STREAM_RWwrite(&rwops, &md_data, 1, sizeof(md_data));

		alarm_header.type = alarm_header.type | NK_HICHIP_ALARM_MD;
	}

	if(io_alarm_flag){
		//io_alarm_data
		stNK_HICHIP_IO_DATA io_data;
		io_data.chn = htonl(0);
		io_data.type = htonl(NK_HICHIP_IO_PIR);
		
		boundary.type = htonl(NK_HICHIP_ALARM_IO);
		NK_STREAM_RWwrite(&rwops, &boundary, 1, sizeof(boundary));
		//fill io data
		NK_STREAM_RWwrite(&rwops, &io_data, 1, sizeof(io_data));

		alarm_header.type = alarm_header.type | NK_HICHIP_ALARM_IO;
	}

	/*
	if(alarm_header.type == 1){
		APP_TRACE("send md only");
	}else if(alarm_header.type == 2){
		APP_TRACE("send PIR only");
	}else if(alarm_header.type == 3){
		APP_TRACE("send md + PIR");
	}*/

	boundary.type = htonl(NK_HICHIP_ALARM_END);
	NK_STREAM_RWwrite(&rwops, &boundary, 1, sizeof(boundary));
	
	//alarm_header
	MD5Final(alarm_header.checksum, &stmd5_context);
	alarm_header.length = htonl(sizeof(alarm_header) - sizeof(alarm_header.magic) + NK_STREAM_RWsize(&rwops));
	alarm_header.magic = htonl(NK_HICHIP_ALARM_MAGIC);
	alarm_header.type = htonl(alarm_header.type);
	alarm_header.ver = htonl(NK_HICHIP_ALARM_VER_10);
	total_size = sizeof(alarm_header) + NK_STREAM_RWsize(&rwops);
	
	NK_STREAM_RWFromMem(&rwops, data, sizeof(alarm_header), 0);
	NK_STREAM_RWwrite(&rwops, &alarm_header, 1, sizeof(alarm_header));

	srtpHeader->pt = 102;
	frameHeader.payloadLen = htonl(total_size + sizeof(frameHeader.rtpHead));

	ret = tcp->send2(tcp, &frameHeader, sizeof(frameHeader), 0);
	if(ret < 0){
		// FIXME:
		APP_TRACE("send frameheader failed  %d", frameHeader.payloadLen);
		return 0;
	}

	//send the data
	ret = tcp->send2(tcp, data, total_size, 0);
	if(ret < 0){
		APP_TRACE("send the alarm data failed");
		return 0;
	}
	
	return ret;
}

int hichip_send_heart_beat(lpSOCKET_TCP tcp, struct HICHIP_RTSP_ITLEAVED_HDR frameHeader)
{
	stNK_HICHIP_ALARM_HEADER alarm_header;
	stNK_HICHIP_ALARM_BOUNDARY boundary;
	char data[1024];
	stNK_STREAM_RW rwops;
	int ret;
	char sum_buffer[100];
	struct HICHIP_RTP_HDR* const srtpHeader = &frameHeader.rtpHead;
	struct MD5Context stmd5_context;
	int total_size = 0;
	
	snprintf(sum_buffer, sizeof(sum_buffer), "%s", "_+N1ALARM+CSUM+_");
	MD5Init(&stmd5_context);
	MD5Update(&stmd5_context, sum_buffer, strlen(sum_buffer));

	memset(&alarm_header, 0, sizeof(alarm_header));
	memset(&rwops, 0, sizeof(stNK_STREAM_RW));
	NK_STREAM_RWFromMem(&rwops, data + sizeof(alarm_header), 
                    	sizeof(data) - sizeof(alarm_header), 0);
	//heartbeat_data
	stNK_HICHIP_HEARTBEAT_DATA heart_data;
	memset(&heart_data, 0, sizeof(heart_data));
	heart_data.status = (htonl)NK_HICHIP_HEARTBEAT_LIVE;
		
	boundary.type = htonl(NK_HICHIP_ALARM_HEARDBEAT);
	NK_STREAM_RWwrite(&rwops, &boundary, 1, sizeof(boundary));
	//fill  data
	NK_STREAM_RWwrite(&rwops, &heart_data, 1, sizeof(heart_data));

	alarm_header.type = alarm_header.type | NK_HICHIP_ALARM_HEARDBEAT;

	boundary.type = htonl(NK_HICHIP_ALARM_END);
	NK_STREAM_RWwrite(&rwops, &boundary, 1, sizeof(boundary));
	
	//alarm_header
	MD5Final(alarm_header.checksum, &stmd5_context);
	alarm_header.length = htonl(sizeof(alarm_header) - sizeof(alarm_header.magic) + NK_STREAM_RWsize(&rwops));
	alarm_header.magic = htonl(NK_HICHIP_ALARM_MAGIC);
	alarm_header.type = htonl(alarm_header.type);
	alarm_header.ver = htonl(NK_HICHIP_ALARM_VER_10);
	total_size = sizeof(alarm_header) + NK_STREAM_RWsize(&rwops);
	
	NK_STREAM_RWFromMem(&rwops, data, sizeof(alarm_header), 0);
	NK_STREAM_RWwrite(&rwops, &alarm_header, 1, sizeof(alarm_header));

	srtpHeader->pt = 102;
	frameHeader.payloadLen = htonl(total_size + sizeof(frameHeader.rtpHead));

	ret = tcp->send2(tcp, &frameHeader, sizeof(frameHeader), 0);
	if(ret < 0){
		// FIXME:
		APP_TRACE("send frameheader failed  %d", frameHeader.payloadLen);
		return ret;
	}

	//send the data
	ret = tcp->send2(tcp, data, total_size, 0);
	if(ret < 0){
		APP_TRACE("send the alarm data failed");
		return ret;
	}
	
	return ret;
	
}

static bool hichip_is_request_video(int media_type)
{
	return (media_type & (HICHIP_MEDIA_VIDEO)) ? true : false;
}

static bool hichip_is_request_audio(int media_type)
{
	return (media_type & (HICHIP_MEDIA_AUDIO)) ? true : false;
}

static bool hichip_is_request_data(int media_type)
{ 
	return (media_type & (HICHIP_MEDIA_DATA)) ? true : false;
}

static bool hichip_check_auth(LP_HTTP_CONTEXT context)
{
	const char *authTag = NULL;
	const char *prefix = NULL;
	authTag = context->request_header->read_tag(context->request_header, "Authorization");
	if(!authTag){
		return false;
		// do something
	}else{
		prefix = "Basic ";
		//compat for the old version NVR
		if(USRM_check_user("admin", "") == USRM_GREAT){
			APP_TRACE("compat for the old version NVR");
			return true;
		}
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
			APP_TRACE("%s/%s", username, password);
			if(USRM_check_user(username, password) == USRM_GREAT){
				return true;
			}
		}
	}
	return false;
}

static int hichip_setBypassFrame(void *rawFrame)
{
	ST_BYPASS_FRAME byPassFrame;
	ST_FISHEYE_PARAM fisheye;
	stFISHEYE_config fisheye_config;

    byPassFrame.frameType = HICHIP_FISHEYE_FRAME_TYPE_FISHEYE_PARAM;  //Fisheye parameter default frame type
    byPassFrame.dataSize = sizeof(ST_FISHEYE_PARAM); //Fisheye parameter data size
	
    /*  Get fisheye configuration */
	
    FISHEYE_config_get(&fisheye_config);
	memcpy(fisheye.param, fisheye_config.param, sizeof(fisheye_config.param));
	
	ST_MODEL_CONF model_conf;
	
	if(NULL != MODEL_CONF_get(&model_conf)){
		fisheye.fixMode = model_conf.fixmode;
	}else{
		fisheye.fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE;
	}
    fisheye.version = 0x01000003;//1.0.0.3
    fisheye.Reverse = 0;

	//copy to hichip buffer
	memcpy(rawFrame, &byPassFrame, sizeof(ST_BYPASS_FRAME));
	memcpy(rawFrame + sizeof(ST_BYPASS_FRAME), &fisheye,sizeof(ST_FISHEYE_PARAM));
	
    return sizeof(ST_BYPASS_FRAME) + sizeof(ST_FISHEYE_PARAM);
	
}

static int hichip_send_fisheye_oob_frame(lpSOCKET_TCP tcp, struct HICHIP_RTSP_ITLEAVED_HDR frameHeader)
{
    int ret = 0;
    int Framelen = 0;
    char rawFrame[128] = {0};
    struct HICHIP_RTP_HDR* const srtpHeader = &frameHeader.rtpHead;

    memset(rawFrame, 0, sizeof(rawFrame));
    Framelen = hichip_setBypassFrame((void *)rawFrame);
    srtpHeader->pt = 103;
    frameHeader.payloadLen = htonl(Framelen + sizeof(frameHeader.rtpHead));
    int tSize = 0;
    if(0 != srtpHeader->pt) {
        if(tcp->send2(tcp, &frameHeader, sizeof(frameHeader), 0) < 0) {
            APP_TRACE("send fisheye frameHeader failed!!:%s(%d)", strerror(errno), errno);
            ret = -1;
        }
        else {
            if(tcp->send2(tcp, rawFrame,Framelen, 0) < 0) {
                APP_TRACE("send fisheye rawFrame failed!!");
                ret = -1;
            }
        }
    }

    return ret;

}

/*
    关闭有线无线dhcp
*/
static void hichip_dhcp_close()
{
    ST_NSDK_NETWORK_INTERFACE lan0, wlan0;

    NETSDK_conf_interface_get(1, &lan0);
    NETSDK_conf_interface_get(4, &wlan0);
    if(lan0.lan.addressingType != kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC) {
        lan0.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC;
        NETSDK_conf_interface_set(1, &lan0, eNSDK_CONF_SAVE_JUST_SAVE);
        NK_SYSTEM("kill -9 `pidof udhcpc`");
    }
    if(wlan0.wireless.dhcpServer.dhcpAutoSettingEnabled == true) {
        wlan0.wireless.dhcpServer.dhcpAutoSettingEnabled = false;
        NETSDK_conf_interface_set(4, &wlan0, eNSDK_CONF_SAVE_JUST_SAVE);
        NK_SYSTEM("kill -9 `pidof udhcpc`");
    }

}

static void hichip_stream_compatibility_mode(char *user_agent)
{
    ST_NSDK_VENC_CH venc_ch;

    if(NULL == user_agent)
    {
        return;
    }

    if(!strncmp(user_agent, "HiIpcam/V100R003 VodClient/1.0.0", 32))
    {

    // 无线单品打开码流启用nvr兼容模式,关闭跳帧参考,gop改为2秒
#if defined(CX)
        if(false == GLOBAL_sn_front())
        {
            NETSDK_conf_venc_ch_get(101, &venc_ch);
            if(venc_ch.ImageTransmissionModel != eNSDK_COMPATIBILITY_MODE)
            {
                venc_ch.ImageTransmissionModel = eNSDK_COMPATIBILITY_MODE;
                NETSDK_conf_venc_ch_set(101, &venc_ch);
            }
            NETSDK_conf_venc_ch_get(102, &venc_ch);
            if(venc_ch.ImageTransmissionModel != eNSDK_COMPATIBILITY_MODE)
            {
                venc_ch.ImageTransmissionModel = eNSDK_COMPATIBILITY_MODE;
                NETSDK_conf_venc_ch_set(102, &venc_ch);
            }
        }
#endif
    }

}

#define HICHIP_HEARBEAT_STR "heartbeat"
#define HICHIP_PLAY_STR "play"
#define HICHIP_BITRATE_MODE_NORMAL_STR "normal"
#define HICHIP_BITRATE_MODE_LOW_STR "lowBitRate"
#define HICHIP_MD_IO_SEND_TIME_OUT 30

int HICHIP_live_stream(LP_HTTP_CONTEXT context)
{
	int ret = 0;
	int mediaBufID = 0;
	int mediaBufSpeed = 0;
	uint32_t const ssrc = rand();
	uint32_t seqNumber = rand();
	int streamID = 0;
	const char *requestURI = strdupa(context->request_header->uri);
	const char *requestQuery = strdupa(context->request_header->query);
	char mediaBufName[16] = {""};
	lpMEDIABUF_USER mediaBufUser = NULL;
	ST_HICHIP_MEDIA_SESSION mediaSession;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp2_r(context->sock, &sockTCP);
	LP_HTTP_HEAD_FIELD httpHeader = NULL;
	int httpHeaderLen = 0;
	char httpBuf[1048] = {""};
	bool mediaSDPSent = false;
	const char * media_type_type = NULL;
    const char * action_type = NULL;
	const char * bitrate_model = NULL;
	int md_send_times = 0;
    int io_send_times = 0;
	bool io_alarm_status = false;
	bool md_status = false;
	time_t last_md_time = 0;
    time_t cur_heart_time  = 0;
	time_t last_heart_time = 0;
	time_t md_send_data_time = 0;
	bool md_io_data_out = false;
	bool bit_rate_open_flag = false;
	bool heart_beat_open_flag = false;
	ST_NSDK_IMAGE image;
    ST_NSDK_VENC_CH venc_ch;
	const char * audio_code_type = NULL;
	LP_HTTP_QUERY_PARA_LIST pHead = NULL;
	char threadName[16] = {0};
	unsigned char ip[4] = {0};
	struct sockaddr_in addr_in = {0};
	socklen_t addr_len = sizeof(addr_in);
    char *user_agent = NULL;
    bool hichip_bypass_frame_flag = false;
    ST_CUSTOM_SETTING custom;
	pthread_detach(pthread_self());

	// enable nodelay
	tcp->set_nodelay(tcp, true);
	tcp->set_send_timeout(tcp, 4, 0);
	tcp->set_recv_timeout(tcp, 4, 0);

	//user check
	if(hichip_check_auth(context) == false){
		HTTP_UTIL_send_401_digest(context->sock, context->request_header);
		APP_TRACE("No Authorization!");
		*context->trigger = false;
		return -1;
	}

    CUSTOM_get(&custom);

    user_agent = context->request_header->read_tag(context->request_header, "User-Agent");

	STR_TO_UPPER(requestURI);
	sscanf(requestURI, "/LIVESTREAM/%d", &streamID);
    //APP_TRACE("cgi buffer meassage : %s", requestQuery);
    tcp->getpeername(tcp, (struct sockaddr *)&addr_in, &addr_len);
    snprintf(httpBuf, sizeof(httpBuf), "%s", inet_ntoa(addr_in.sin_addr));
    APP_TRACE("peer ip = (%s)", httpBuf);
    if(strlen(httpBuf)){
        ipstr2uint8(ip, httpBuf);
        snprintf(threadName, sizeof(threadName), "LIVE%02d_%02X%02X%02X%02X", streamID, ip[0], ip[1], ip[2], ip[3]);
        prctl(PR_SET_NAME, (unsigned long)threadName);
        memset(httpBuf, 0, sizeof(httpBuf));
    }

    pHead = HTTP_UTIL_parse_query_as_para(requestQuery);

	//parse action type
	if(NULL != (action_type = pHead->read(pHead, "action"))){
		if(strncmp(action_type, HICHIP_PLAY_STR, strlen(HICHIP_PLAY_STR)) == 0 ){
			bit_rate_open_flag = true;
			heart_beat_open_flag = false;
			APP_TRACE("play mode");
		}else if(strncmp(action_type, HICHIP_HEARBEAT_STR, strlen(HICHIP_HEARBEAT_STR)) == 0){
			bit_rate_open_flag = false;
			heart_beat_open_flag = true;
			APP_TRACE("heart beat mode");
		}
	}

	if(NULL != (bitrate_model = pHead->read(pHead, "mode"))){
		if(strncmp(bitrate_model, HICHIP_BITRATE_MODE_LOW_STR, strlen(HICHIP_BITRATE_MODE_LOW_STR)) == 0){
			//set low bitrate mode
			APP_TRACE("set to low bitrate mode");
			NETSDK_conf_venc_ch_get(101, &venc_ch);
			if(venc_ch.ImageTransmissionModel != eNSDK_LOW_BPS_MODEL){
				venc_ch.ImageTransmissionModel = eNSDK_LOW_BPS_MODEL;
				NETSDK_conf_venc_ch_set(101, &venc_ch);
			}
			
			memset(&venc_ch, 0, sizeof(ST_NSDK_VENC_CH));
			NETSDK_conf_venc_ch_get(102, &venc_ch);
			if(venc_ch.ImageTransmissionModel != eNSDK_LOW_BPS_MODEL){
				venc_ch.ImageTransmissionModel = eNSDK_LOW_BPS_MODEL;
				NETSDK_conf_venc_ch_set(102, &venc_ch);
			}

			//for low bitrate mode ,set the wifi_bps mode to flase
			ST_NSDK_NETWORK_INTERFACE wlan;
			NETSDK_conf_interface_get(4, &wlan);
			if(wlan.wireless.wirelessStaMode.wirelessFixedBpsModeEnabled != false){
				wlan.wireless.wirelessStaMode.wirelessFixedBpsModeEnabled = false;
				NETSDK_conf_interface_set(4, &wlan, eNSDK_CONF_SAVE_JUST_SAVE);
			}
			
		}else if(strncmp(bitrate_model, HICHIP_BITRATE_MODE_NORMAL_STR, strlen(HICHIP_BITRATE_MODE_NORMAL_STR)) == 0){
			//set to normal bitrate mode
			APP_TRACE("set to normal mode");
			NETSDK_conf_venc_ch_get(101, &venc_ch);
			if(venc_ch.ImageTransmissionModel != eNSDK_COMPATIBILITY_MODE){
				venc_ch.ImageTransmissionModel = eNSDK_COMPATIBILITY_MODE;
				NETSDK_conf_venc_ch_set(101, &venc_ch);
			}
			
			memset(&venc_ch, 0, sizeof(ST_NSDK_VENC_CH));
			NETSDK_conf_venc_ch_get(102, &venc_ch);
			if(venc_ch.ImageTransmissionModel != eNSDK_COMPATIBILITY_MODE){
				venc_ch.ImageTransmissionModel = eNSDK_COMPATIBILITY_MODE;
				NETSDK_conf_venc_ch_set(102, &venc_ch);
			}

			//for COMPATIBILITY mode ,set the wifi_bps mode to true
			ST_NSDK_NETWORK_INTERFACE wlan;
			NETSDK_conf_interface_get(4, &wlan);
			if(wlan.wireless.wirelessStaMode.wirelessFixedBpsModeEnabled != true){
				wlan.wireless.wirelessStaMode.wirelessFixedBpsModeEnabled = true;
				NETSDK_conf_interface_set(4, &wlan, eNSDK_CONF_SAVE_JUST_SAVE);
				IPCAM_timer_open_wifi_modify_bps();
			}
		}
        else
        {
            hichip_stream_compatibility_mode(user_agent);
        }
	}
    else
    {
        hichip_stream_compatibility_mode(user_agent);
    }
	if(NULL != (audio_code_type = pHead->read(pHead, "audioFormat"))){
		 ST_NSDK_AENC_CH aenc_ch;
		 if(strncmp(audio_code_type, "AAC", strlen("AAC")) == 0){
		 	NETSDK_conf_aenc_ch_get(101, &aenc_ch);
			if(aenc_ch.codecType != kNSDK_AENC_CODEC_TYPE_AAC){
				aenc_ch.codecType = kNSDK_AENC_CODEC_TYPE_AAC;
				NETSDK_conf_aenc_ch_set(101, &aenc_ch);
			}
		 }else if(strncmp(audio_code_type, "G711", strlen("G711")) == 0){
			NETSDK_conf_aenc_ch_get(101, &aenc_ch);
			if(aenc_ch.codecType != kNSDK_AENC_CODEC_TYPE_G711A){
				aenc_ch.codecType = kNSDK_AENC_CODEC_TYPE_G711A;
				NETSDK_conf_aenc_ch_set(101, &aenc_ch);
			}
		 }
	}

	if(NULL != (media_type_type = pHead->read(pHead, "media"))){
		mediaSession.media_type = hichip_map_str2dec(hichip_media_map, sizeof(hichip_media_map)/sizeof(hichip_media_map[0]), media_type_type, HICHIP_MEDIA_VIDEO);
		APP_TRACE("media type:%s--%x", media_type_type, mediaSession.media_type);
	}else{
		APP_TRACE("no requery media type!");
		*context->trigger = false;
		return -1;
	}
	//mediaSession.media_type = HICHIP_MEDIA_VIDEO_AUDIO_DATA;
	//mediaSession.media_type = HICHIP_MEDIA_VIDEO_DATA;
	snprintf(mediaBufName, sizeof(mediaBufName), "ch%d_%d.264", streamID / 10 - 1, streamID % 10 - 1);
	APP_TRACE("mediaBufName = %s", mediaBufName);
	
	mediaBufID = MEDIABUF_lookup_byname(mediaBufName); // FIXME:
	if(mediaBufID < 0){
		APP_TRACE("MediaBuf not found!");
		*context->trigger = false;
		return -1;
	} 
	
	mediaBufSpeed = MEDIABUF_in_speed(mediaBufID);
	mediaBufUser = MEDIABUF_attach(mediaBufID);

    user_agent = context->request_header->read_tag(context->request_header, "User-Agent");
    if(!strncmp(user_agent, "HiIpcam/V100R003 VodClient/1.0.0", 32)) {
        hichip_dhcp_close();
    }

	if(NULL != mediaBufUser){
		httpHeader = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
		httpHeader->add_tag_text(httpHeader, "Connection", "Keep-Alive", true);			   
		httpHeader->add_tag_text(httpHeader, "Server", "IE/10.0", true);
		httpHeader->add_tag_text(httpHeader, "Cache-Control", "no-cache", true);
		httpHeader->add_tag_text(httpHeader, "Accept-Ranges", "Bytes", true);
		httpHeader->add_tag_text(httpHeader, "Content-Type", "application/octet-stream", true);
		httpHeader->add_tag_text(httpHeader, "Connection", "close", true);
		httpHeader->to_text(httpHeader, httpBuf, sizeof(httpBuf));
		httpHeader->free(httpHeader);
		httpHeader = NULL;

		//APP_TRACE(httpBuf);

		ret = tcp->send2(tcp, httpBuf, strlen(httpBuf), 0);
		if(ret < 0){
			APP_TRACE("Why !!!!!");
		}

        if(NETSDK_conf_image_get(&image)){
            switch(image.videoMode.fixType){
                case eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL:
                case eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL:
                case eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE:
                hichip_bypass_frame_flag = true;
                break;
                case eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE:
                hichip_bypass_frame_flag = false;
                break;

            }
        }

        GLOBAL_enter_live();

		unsigned long long old_timestamp = 0;
		while(*context->trigger){
            fd_set read_fds;
			struct timeval poll_wait;
			lpSDK_ENC_BUF_ATTR attr = NULL;
			int outDataLen = 0;
			void *rawData = NULL;
			int rawDataLen = 0;
	
			FD_ZERO(&read_fds);
			FD_SET(tcp->sock, &read_fds);
			poll_wait.tv_sec = 0;
			poll_wait.tv_usec = 0;
			ret = select(tcp->sock + 1, &read_fds, NULL, NULL, &poll_wait);
			if(ret > 0 && FD_ISSET(tcp->sock, &read_fds)){
				struct sockaddr_in from_addr;
				socklen_t addr_len = sizeof(from_addr);
				char buf[1024];
				ret = recvfrom(tcp->sock, buf, sizeof(buf), 0, (struct sockaddr *)&from_addr, &addr_len);
				if(ret > 0){
					//parse message 
					stNK_HICHIP_ALARM_DATA alarm_data;
					int parseret;
					char *pBuf = buf;
			_parse_next:
					memset(&alarm_data, 0, sizeof(alarm_data));
					parseret = HICHIP_alarm_parse(pBuf, ret, &alarm_data, 1);
					if(parseret > 0){
						switch(alarm_data.type){
							case NK_HICHIP_ALARM_HEARDBEAT:
								 break;
			
							case NK_HICHIP_ALARM_OPERATION:
								 if(alarm_data.operation.status == NK_HICHIP_OPERATION_START){
							 		//fix it
							 		bit_rate_open_flag = true;
									heart_beat_open_flag = false;
							 	 }else if(alarm_data.operation.status == NK_HICHIP_OPERATION_PAUSE){
									bit_rate_open_flag = false;
									heart_beat_open_flag = true;
									//fix it
							 	 }else if(alarm_data.operation.status == NK_HICHIP_OPERATION_STOP){
									//fix it
								 	bit_rate_open_flag = false;
									heart_beat_open_flag = true;
								 }else if(alarm_data.operation.status == NK_HICHIP_OPERATION_REQ_IDR){
									//
									APP_TRACE("HICHIP_live_stream request keyframe!");
									if(sdk_enc){
										sdk_enc->request_stream_h264_keyframe(0, 0);
									}
									bit_rate_open_flag = true;
									heart_beat_open_flag = false;
								 }
								 break;
								 
								default:
									break;
						}

					}	
					if(parseret > 0){
						char alarmBuf[128];
						int retSize = HICHIP_alarm_construct(alarmBuf, sizeof(alarmBuf), &alarm_data, 1);
						if(retSize > 0 && retSize < ret){
							pBuf += retSize;
							ret -= retSize;
							goto _parse_next;
						}
					}
				}
			}
			
			bool mediaBufOut = false;
			if(0 == MEDIABUF_out_try_lock(mediaBufUser)){
                struct HICHIP_RTSP_ITLEAVED_HDR frameHeader;
				struct HICHIP_RTP_HDR*  rtpHeader = NULL;
				int media_is_out = false;
                void *rawFrame = (void *)(httpBuf);
                if(bit_rate_open_flag || !mediaSDPSent || md_io_data_out || hichip_bypass_frame_flag){
    				if(0 == MEDIABUF_out(mediaBufUser, (void *)&attr, NULL, &outDataLen)){
    					rawData = (void *)(attr + 1);
    					rawDataLen = attr->data_sz;
    					rtpHeader = &frameHeader.rtpHead;

    					//memset(&frameHeader, 0, sizeof(frameHeader));
    					frameHeader.dollar = '$';
    					frameHeader.channelid = 0;
    					frameHeader.resv = 0;
    					frameHeader.payloadLen = htonl(rawDataLen + sizeof(frameHeader.rtpHead));
    					rtpHeader->cc = 0;
    					rtpHeader->x = 0;
    					rtpHeader->p = 0;
    					rtpHeader->version = 2;
    					rtpHeader->pt = 0;
    					rtpHeader->marker = 0;
    					rtpHeader->seqno = seqNumber++;
    					rtpHeader->ts = htonl(attr->timestamp_us);
    					rtpHeader->ssrc = ssrc;
                        media_is_out = true;
                    }

                    if(md_io_data_out == true){
						//check the time;
						if((time(NULL) - md_send_data_time) > HICHIP_MD_IO_SEND_TIME_OUT){
							md_io_data_out = false;
							if(!bit_rate_open_flag){
								media_is_out = false;
							}
						}
                    }
                }

				if(!mediaSDPSent && media_is_out){
                    // send SDP is necessary
                    char audioType[8] = {0};
                    if(kSDK_ENC_BUF_DATA_G711A == attr->type) {
                        snprintf(audioType, sizeof(audioType), "G711");
                    }
                    else if(kSDK_ENC_BUF_DATA_AAC == attr->type) {
                        snprintf(audioType, sizeof(audioType), "AAC");
                    }
				 	switch(attr->type){
						default:
						case kSDK_ENC_BUF_DATA_H264:
							{
							snprintf(httpBuf, sizeof(httpBuf),
								"Session: %d"kCRLF
								"Cseq: 1"kCRLF
								"m=video 96 H264/90000/%d/%d"kCRLF
								"m=audio 8 %s/8000/2"kCRLF
								"Transport: RTP/AVP/TCP;unicast;hisiinterleaved=0-1;ssrc=%x"kCRLF
								kCRLF,
								ssrc, attr->h264.width, attr->h264.height, audioType, ssrc);
					 		}
							break;
						case kSDK_ENC_BUF_DATA_H265:
							{
							snprintf(httpBuf, sizeof(httpBuf),
								"Session: %d"kCRLF
								"Cseq: 1"kCRLF
								"m=video 97 H265/90000/%d/%d"kCRLF
								"m=audio 8 %s/8000/2"kCRLF
								"Transport: RTP/AVP/TCP;unicast;hisiinterleaved=0-1;ssrc=%xs"kCRLF
								kCRLF,
								ssrc, attr->h265.width, attr->h265.height, audioType, ssrc);
					 		}
							break;
					}
					ret = tcp->send2(tcp, httpBuf, strlen(httpBuf), 0);
					if(ret < 0){
						// FIXME:
						APP_TRACE("send sdp failed!!");
						*context->trigger = false;
					}
					mediaSDPSent = true;
				}

				if( true == hichip_bypass_frame_flag && media_is_out){
                    if(hichip_send_fisheye_oob_frame(tcp, frameHeader) < 0) {
                        *context->trigger = false;
                    }
                    else {
                        hichip_bypass_frame_flag = false;
                    }
				}
					
					if(hichip_is_request_data(mediaSession.media_type)){
                        time_t cur_time = time(NULL);
#if defined(PIR_ALARM)
						//IO alarm
						io_alarm_status =  HICHIP_get_io_alarm_status();
						//APP_TRACE("check io alarm: %d", io_alarm_status);
						if(io_alarm_status){
							if(io_send_times < HICHIP_SEND_IO_ALARM_STATUS_TIMES){
								io_send_times++;
								gs_io_alarm_timestamp = cur_time;
							}
							else if(gs_io_alarm_timestamp < cur_time){
								gs_io_alarm_timestamp = cur_time;
								io_send_times = 0;
							}	
							else{
								io_alarm_status = false;
							}
						}
						else{
							io_send_times = 0;
						}

#endif
                        md_status = HICHIP_get_md_status();
						if(md_status){
                            last_md_time = time(NULL);
							if(md_send_times < HICHIP_SEND_MD_STATUS_TIMES){
								md_send_times++;
							}
							else if(gs_md_timestamp < cur_time){
								HICHIP_set_md_status(false);
								md_send_times = 0;
								gs_md_timestamp = cur_time;
							}else{
								md_status = false;
							}
						}
                        else{
							md_send_times = 0;
						}

                        if(io_alarm_status && ((cur_time - last_md_time) <= 1)){
                                md_status = true;
                        }

                        if(0 < custom.function.pirEnabled)
                        {
                            if(md_status && io_alarm_status){
                                NETSDK_conf_venc_ch_get(101, &venc_ch);
                                hichip_send_md_status(tcp, frameHeader, 0, 0, venc_ch.resolutionWidth, venc_ch.resolutionHeight);
                            }
                        }
                        else
                        {
                            if(md_status){
                                NETSDK_conf_venc_ch_get(101, &venc_ch);
                                hichip_send_md_status(tcp, frameHeader, 0, 0, venc_ch.resolutionWidth, venc_ch.resolutionHeight);
                            }
                        }
                        if(md_status || io_alarm_status){
                            hichip_send_md_io_status(md_status, io_alarm_status, tcp, frameHeader);
                            md_io_data_out = true;
                            md_send_data_time = time(NULL);
                        }

					}

                    if(heart_beat_open_flag){
						cur_heart_time  = time(NULL);
						if(cur_heart_time - last_heart_time >= 10){
							ret = hichip_send_heart_beat(tcp, frameHeader);
							if(ret < 0){
								// FIXME:
								APP_TRACE("send heat beat packet failed!!");
								*context->trigger = false;
								media_is_out = false;
							}
							last_heart_time = cur_heart_time;
						}	
					}

                    if(media_is_out){
					rtpHeader->pt = 0;
					if(kSDK_ENC_BUF_DATA_H264 == attr->type){
						if(hichip_is_request_video(mediaSession.media_type)){
							rtpHeader->pt = 96;
						}else{
							//continue;
						}
					}else if(kSDK_ENC_BUF_DATA_H265 == attr->type){
						if(hichip_is_request_video(mediaSession.media_type)){
							rtpHeader->pt = 97;
						}else{
							//continue;
						}	
					}else if(kSDK_ENC_BUF_DATA_G711A == attr->type){
						if(hichip_is_request_audio(mediaSession.media_type)){
							rtpHeader->pt = 8;
						}else{
							//continue;
						}	
					}else if(kSDK_ENC_BUF_DATA_AAC== attr->type){
						if(hichip_is_request_audio(mediaSession.media_type)){
							rtpHeader->pt = 104;
						}else{
							//continue;
						}
					}
                    else{
						//APP_TRACE("What insert to my buffer? (%d)", rtpHeader->pt);
					}

					if(0 != rtpHeader->pt){
						ret = tcp->send2(tcp, &frameHeader, sizeof(frameHeader), 0);
						if(attr->timestamp_us - old_timestamp > 120000 && streamID == 11){
							//APP_TRACE("%llu -- %llu == %llu", old_timestamp, attr->timestamp_us, attr->timestamp_us- old_timestamp);					
						}
						old_timestamp = attr->timestamp_us;	
						if(ret < 0){ 
							// FIXME:
							APP_TRACE("send frameHeader failed!!:%s(%d)", strerror(errno), errno);
							*context->trigger = false;
						}else{
							ret = tcp->send2(tcp, rawData, rawDataLen, 0);
							if(ret < 0){
								// FIXME:
								APP_TRACE("send rawData failed!!");
								*context->trigger = false;
							}
							mediaBufOut = true;
						}						
					}
				}
				MEDIABUF_out_unlock(mediaBufUser);
			}
			if(!mediaBufOut){
				usleep(1000);
			}
		}

		APP_TRACE("Exit HiChip Media Server");
		MEDIABUF_detach(mediaBufUser);
		mediaBufUser = NULL;
        GLOBAL_leave_live();
	}{
		HTTP_UTIL_send_503(context->sock, context->request_header);
		*context->trigger = false;
		return 0;
	}
	return -1;
}



