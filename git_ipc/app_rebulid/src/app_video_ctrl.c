#ifdef VIDEO_CTRL

#include "app_video_ctrl.h"
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <NkUtils/log.h>
#include <NkUtils/assert.h>
#include <time.h>
#include <base/ja_process.h>
#include <app_debug.h>
#include "netsdk.h"
#include "sdk/sdk_api.h"
#include <sys/prctl.h>

typedef struct App_Video_Ctrl_Data{
	int ate_static_open;			// 0  没有ate设置,1 有ate设置
	int ate_static_time;			//0  没有时间限制，>0 为限制更新时间，秒单位
	time_t ate_set_time;			//设置ate时的时间
	int tf_card_status;			// 0 没有tf卡，1有tf卡	
	pthread_t app_ctrl_pid;			//线程id
	int app_ctrl_trigger;
	int current_event;			//0 当前没有事件，1 当前有事件
	time_t event_set_time;		//有事件时的时间
}stApp_Video_Ctrl_Data,*lpApp_Video_Ctrl_Data;

lpApp_Video_Ctrl_Data __app_video_ctrl_data = NULL;

#define APP_VIDEO_CTRL_BPS_LOW 256
#define APP_VIDEO_CTRL_GOP_LOW 20
#define APP_VIDEO_CTRL_FPS_LOW 2

#define APP_VIDEO_CTRL_BPS_HIGH 2048
#define APP_VIDEO_CTRL_GOP_HIGH 50
#define APP_VIDEO_CTRL_FPS_HIGH 25


static NK_Boolean onCheckVideoParam(lpVIDEO_PARAM_T videoParam)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((videoParam != NULL),-1);
//	APP_TRACE("onCheckVideoParam is ok");
	ST_NSDK_VENC_CH venc_ch ;
	LP_NSDK_VENC_CH lpvenc_ch = NULL;
	memset(&venc_ch,0,sizeof(ST_NSDK_VENC_CH));
	lpvenc_ch = NETSDK_conf_venc_ch_get(101, &venc_ch);
	int check_ret =1;
	if(lpvenc_ch){
		if((videoParam->bps > lpvenc_ch->constantBitRate) ||
			(videoParam->gop >  lpvenc_ch->keyFrameInterval) ||
			(videoParam->fps > lpvenc_ch->frameRate)){
			check_ret =0;
		}
		if((videoParam->bps < APP_VIDEO_CTRL_BPS_LOW) ||
			(videoParam->gop < APP_VIDEO_CTRL_GOP_LOW) ||
			(videoParam->fps < APP_VIDEO_CTRL_FPS_LOW)){
			check_ret =0;
		}		
	}
	if(check_ret){
		return NK_True;
	}else {
		return NK_False;
	}
}

static NK_Int onGetVideoParam(lpVIDEO_PARAM_T videoParam)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((videoParam != NULL),-1);
	int ret =0;
	int vin =0;
	int stream =0;
	ST_SDK_ENC_STREAM_ATTR stream_attr;;
	memset(&stream_attr,0,sizeof(ST_SDK_ENC_STREAM_ATTR));
	ret = SDK_ENC_get_stream(vin, stream, &stream_attr);
	if(0 == ret){
		if(kSDK_ENC_BUF_DATA_H264 == stream_attr.enType){
			videoParam->bps = stream_attr.H264_attr.bps;
			videoParam->gop = stream_attr.H264_attr.gop;
			videoParam->fps = stream_attr.H264_attr.fps;
			videoParam->updateI = NK_False;
		}else if(kSDK_ENC_BUF_DATA_H265 == stream_attr.enType){
			videoParam->bps = stream_attr.H265_attr.bps;
			videoParam->gop =  stream_attr.H264_attr.gop;
			videoParam->fps = stream_attr.H264_attr.fps;
			videoParam->updateI = NK_False;
		}		
	}else {
		return -1;
	}
	return 0;
}

static NK_Int onSetVideoParam(lpVIDEO_PARAM_T videoParam)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((videoParam != NULL),-1);
	int ret =0;
	int vin =0;
	int stream =0;
	ST_SDK_ENC_STREAM_ATTR stream_attr;
	memset(&stream_attr,0,sizeof(ST_SDK_ENC_STREAM_ATTR));
	ret = SDK_ENC_get_stream(vin, stream, &stream_attr);
	if(0 == ret){
		if(kSDK_ENC_BUF_DATA_H264 == stream_attr.enType){
			stream_attr.H264_attr.fps = videoParam->fps;
			stream_attr.H264_attr.gop = videoParam->gop;
			stream_attr.H264_attr.bps = videoParam->bps ;
		}else if(kSDK_ENC_BUF_DATA_H265 == stream_attr.enType){
			stream_attr.H265_attr.fps = videoParam->fps;
			stream_attr.H265_attr.gop = videoParam->gop;
			stream_attr.H265_attr.bps = videoParam->bps ;
		}
	}else {
		return  -1;
	}
	APP_TRACE("videl stream change to: fps:%d gop:%d bps:%d", videoParam->fps, videoParam->gop, videoParam->bps);
	ret = SDK_ENC_set_stream (vin, stream, &stream_attr);
	if(0 != ret){
		return  -1;
	}

	return 0;
}

static NK_Int initCustomConf(lpVIDEO_PARAM_T highConf,lpVIDEO_PARAM_T lowConf)
{
	ST_NSDK_VENC_CH venc_ch ;
	LP_NSDK_VENC_CH lpvenc_ch = NULL;
	memset(&venc_ch,0,sizeof(ST_NSDK_VENC_CH));
	lpvenc_ch = NETSDK_conf_venc_ch_get(101, &venc_ch);
	if(lpvenc_ch){
		highConf->bps = lpvenc_ch->constantBitRate;
		highConf->gop =  lpvenc_ch->keyFrameInterval;
		highConf->fps = lpvenc_ch->frameRate;
		highConf->updateI = NK_True;
	}else{
		highConf->bps = APP_VIDEO_CTRL_BPS_HIGH;
		highConf->gop = APP_VIDEO_CTRL_GOP_HIGH;
		highConf->fps = APP_VIDEO_CTRL_FPS_HIGH;
		highConf->updateI = NK_True;
	}
	

	lowConf->bps = APP_VIDEO_CTRL_BPS_LOW;
	lowConf->gop = APP_VIDEO_CTRL_GOP_LOW;
	lowConf->fps = APP_VIDEO_CTRL_FPS_LOW;
	lowConf->updateI = NK_False;

	return 0;
}

/*
*负责策略检查
*时间检查
*/
void* app_video_ctrl_threader(void* arg){
	prctl(PR_SET_NAME, "app_video_ctrl_threader");
	if(NULL == arg){
		return ;
	}
	lpApp_Video_Ctrl_Data data = (lpApp_Video_Ctrl_Data)arg;
	data->app_ctrl_trigger = 1;
	while(data->app_ctrl_trigger){
		if(data->ate_static_open){
			if(data->ate_static_time >0){	//有时间限时
				time_t current_time = time(NULL);
				if(current_time - data->ate_set_time > data->ate_static_time){
					VIDEO_CTRL_stop_static_conf();
					data->ate_static_open = 0;
					data->ate_static_time = 0;
				}
			}else{
				// 无时间限制,不处理
			}
		}else if(data->current_event){
			NK_VIDEO_CTRL_trun_up_conf();
			data->event_set_time = time(NULL);
			data->current_event =0;
		}else {

		}

		//for ATE?
		if(data->current_event){
			time_t current_time = time(NULL);
			if((current_time -data->event_set_time) >3 ){
				data->current_event =0;
				data->event_set_time = time(NULL);
			}
		}
		usleep(20000);
	}
	
	
	
}

NK_Int VIDEO_CTRL_start_static_conf(lpVIDEO_PARAM_T videoParam, NK_Boolean autoStopFlag)
{
	if(NULL == videoParam){
		return APP_VIDEO_CTRL_ERROR_PARAM;
	}
	if(NULL == __app_video_ctrl_data){
		return APP_VIDEO_CTRL_ERROR_NOINIT;
	}
	__app_video_ctrl_data->ate_static_open = 1;
	if(__app_video_ctrl_data->ate_static_open){
		__app_video_ctrl_data->ate_static_time = 30;
	}
	__app_video_ctrl_data->ate_set_time = time(NULL);

	return NK_VIDEO_CTRL_start_static_conf(videoParam);
}

NK_Int VIDEO_CTRL_stop_static_conf()
{
	if(NULL == __app_video_ctrl_data){
		return APP_VIDEO_CTRL_ERROR_NOINIT;
	}
	__app_video_ctrl_data->ate_static_open = 0;
	__app_video_ctrl_data->ate_static_time = 0;
	__app_video_ctrl_data->ate_set_time = 0;
	return NK_VIDEO_CTRL_stop_static_conf();
}

NK_Int VIDEO_CTRL_turnup_event(emVIDEO_CTRL_TURNUP_EVENT event)
{
	if(NULL == __app_video_ctrl_data){
		return APP_VIDEO_CTRL_ERROR_NOINIT;
	}
	__app_video_ctrl_data->current_event =1;
	__app_video_ctrl_data->event_set_time = time(NULL);
//	return NK_VIDEO_CTRL_trun_up_conf();
	return 0;
}

int  VIDEO_CTRL_init()
{
	APP_TRACE("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");

	//check the netsdk model
	ST_NSDK_VENC_CH venc_ch;
	NETSDK_conf_venc_ch_get(101, &venc_ch);
	if(venc_ch.ImageTransmissionModel == eNSDK_COMPATIBILITY_MODE){
		APP_TRACE("cause for the compatibility mode do not run video ctrl");
		return 0;
	}
	
	stVIDEO_CTRL_FUNCTION init;
	stVIDEO_PARAM_T highConf = {0},lowConf = {0};
	NK_Int highKeepOnSec;
	int ret;
	lpApp_Video_Ctrl_Data app_video_data = malloc(sizeof(stApp_Video_Ctrl_Data));
	if(NULL == app_video_data){
		return 0;
	}
	memset(app_video_data,0,sizeof(stApp_Video_Ctrl_Data));

	
	init.onCheckVideoParam = onCheckVideoParam;
	init.onGetVideoParam = onGetVideoParam;
	init.onSetVideoParam = onSetVideoParam;
	initCustomConf(&highConf,&lowConf);
	highKeepOnSec = 30;

	ret = NK_VIDEO_CTRL_init(&init,&highConf,&lowConf,highKeepOnSec);
	if(0!=ret){
		APP_TRACE("NK_VIDEO_CTRL_init init  failed!");
		free(app_video_data);
		app_video_data = NULL;
		return -1;
	}

	ret = pthread_create(&app_video_data->app_ctrl_pid,NULL,(void *)app_video_ctrl_threader,app_video_data);
	if(0 != ret){
		APP_TRACE("app video ctrl ctrller thread create failed!");
		NK_VIDEO_CTRL_destroy();
		free(app_video_data);
		app_video_data = NULL;
		return -1;
	}
	__app_video_ctrl_data = app_video_data;
	return 0;
}

NK_Int VIDEO_CTRL_destroy()
{
	NK_VIDEO_CTRL_destroy();
	if(__app_video_ctrl_data){
		__app_video_ctrl_data->app_ctrl_trigger = 0;
		pthread_join(__app_video_ctrl_data->app_ctrl_pid,NULL);
		free(__app_video_ctrl_data);
		__app_video_ctrl_data = NULL;
	}
	
	return 0;
}


#endif

