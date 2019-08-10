#ifdef VIDEO_CTRL

#ifndef __APP_VIDEO_CTRL_H__
#define __APP_VIDEO_CTRL_H__
#include "video_ctrl.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum VIDEO_CTRL_TURNUP_EVENT {
	VIDEO_CTRL_TURNUP_EVENT_NONE = 0,
	VIDEO_CTRL_TURNUP_EVENT_P2P,
	VIDEO_CTRL_TURNUP_EVENT_MOTION,
	VIDEO_CTRL_TURNUP_EVENT_ALL,
}emVIDEO_CTRL_TURNUP_EVENT;

typedef enum APP_VIDEO_CTRL_ERROR{
	APP_VIDEO_CTRL_NOERROR = 0,			//没有错误
	APP_VIDEO_CTRL_ERROR = -1,				//错误
	APP_VIDEO_CTRL_ERROR_PARAM = -2,		//参数错误
	APP_VIDEO_CTRL_ERROR_MEMORY = -3,		//内存错误
	APP_VIDEO_CTRL_ERROR_PTHREAD = -4,	//线程错误
	APP_VIDEO_CTRL_ERROR_NOINIT = -5,		// 没有初始化
}emAPP_VIDEO_CTRL_ERROR;

/*
*设置ATE的回调参数
*
*/
extern NK_Int VIDEO_CTRL_start_static_conf(lpVIDEO_PARAM_T videoParam,NK_Boolean autoStopFlag);
extern NK_Int VIDEO_CTRL_stop_static_conf();
extern NK_Int VIDEO_CTRL_turnup_event(emVIDEO_CTRL_TURNUP_EVENT event);
extern NK_Int VIDEO_CTRL_init();
extern NK_Int VIDEO_CTRL_destroy();




#ifdef __cplusplus
};
#endif
#endif/*_VIDEO_CTRL_H*/

#endif

