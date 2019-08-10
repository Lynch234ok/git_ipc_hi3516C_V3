
#include "netsdk_def.h"

#ifndef _MODEL_CONF_H_
#define _MODEL_CONF_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct model_conf_video
{
	int bps;
	int fps;
	int resolution;
	ST_NSDK_PARA_PROPERTY resolutionProperty;
}ST_MODEL_CONF_VIDEO,*LP_MODEL_CONF_VIDEL;

typedef struct model_conf_audio
{
	int audioInputVol;
	int audioOutputVol;
}ST_MODEL_CONF_AUDIO,*LP_MODEL_CONF_AUDIO;

typedef struct model_conf_sn
{
	uint32_t chipModel;
	uint32_t productModel;
}ST_MODEL_CONF_SN,*LP_MODEL_CONF_SN;


typedef struct model_conf_osd
{
	int osd_stream_num;
	int osd_radio;
	int timeX;
	int timeY;
}ST_MODEL_CONF_OSD, *LP_MODEL_CONF_OSD;

#define MODEL_CONF_VIDEO_MAIN_STREAM_ID (1)
#define MODEL_CONF_VIDEO_SUB_STREAM_ID (2)

typedef struct model_conf
{
	ST_MODEL_CONF_VIDEO video[3];
	ST_MODEL_CONF_AUDIO audio;
	ST_MODEL_CONF_SN snumber;
	EM_NSDK_IMAGE_FISHEYE_FIX_MODE fixmode;
	char modelName[32];
	ST_MODEL_CONF_OSD osd;
    ST_NSDK_IMAGE_IRCUTFILTER ircutFilter;
}ST_MODEL_CONF, *LP_MODEL_CONF;

extern LP_MODEL_CONF MODEL_CONF_init(int sensor_type);
extern int MODEL_CONF_destory();
extern LP_MODEL_CONF MODEL_CONF_get(LP_MODEL_CONF model_conf);
extern bool MODEL_CONF_check_int_valid(int param);

#ifdef __cplusplus
}
#endif
#endif //_MODEL_CONF_H_


