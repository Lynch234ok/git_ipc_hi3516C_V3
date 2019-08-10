#ifdef VIDEO_CTRL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <NkUtils/assert.h>
#include <base/ja_process.h>
#include "generic.h"
#include "app_debug.h"
#include "video_ctrl.h"
#include <sys/prctl.h>

typedef struct _STATIC_PARAM {
	NK_Boolean static_flag;
	stVIDEO_PARAM_T sta_conf;
}stSTATIC_PARAM;

typedef struct _DYNAMIC_PARAM {
	NK_Boolean trun_up_flag;
	NK_Int delay_us;
	stVIDEO_PARAM_T low_conf;
	stVIDEO_PARAM_T high_conf;
}stDYNAMIC_PARAM;

typedef struct _VIDEO_CTRL_T {
	pthread_t ctrl_pid;
	NK_Boolean ctrl_trigger;
	stSTATIC_PARAM static_param;
	stDYNAMIC_PARAM dynamic_param;
	NK_Int high_keep_on_us;
	emVIDEO_CTRL_STATUS_T status;
	stVIDEO_CTRL_FUNCTION eventSet;
}stVIDEO_CTRL_T,*lpVIDEO_CTRL_T;

/**
 * description: 该宏定义用于高帧状态持续时间与控制线程睡眠时间两者之间的匹配.
 * suggest value: 以系统线程调度时间为参考,建议最小值为10000us.
 * attention: 该值不可设置大于1000000us(1sec).
 */
#define SLEEP_TIME_US	(20000)

static lpVIDEO_CTRL_T _lpVideoCtrl = NULL;


static NK_Void ctrller()
{
	prctl(PR_SET_NAME, "ctrller");
	while(_lpVideoCtrl && _lpVideoCtrl->ctrl_trigger) {
		/*传参静态控制*/
		if(_lpVideoCtrl->static_param.static_flag) {
			if(0 == _lpVideoCtrl->eventSet.onSetVideoParam(&_lpVideoCtrl->static_param.sta_conf)){
				_lpVideoCtrl->static_param.static_flag = NK_False;
				_lpVideoCtrl->status = emVIDEO_CTRL_STATIC;
			}
			goto SLEEP_AND_NEXT_ROUND;
		}
		/*动态内部控制*/
		else if(_lpVideoCtrl->status != emVIDEO_CTRL_STATIC) {
			/*启动高帧参数*/
			if(_lpVideoCtrl->dynamic_param.trun_up_flag) {
				if(_lpVideoCtrl->status != emVIDEO_CTRL_DYNAMIC_HIGH) {
					if(0 == _lpVideoCtrl->eventSet.onSetVideoParam(&_lpVideoCtrl->dynamic_param.high_conf)) {
						_lpVideoCtrl->dynamic_param.trun_up_flag = NK_False;
						_lpVideoCtrl->dynamic_param.delay_us = _lpVideoCtrl->high_keep_on_us;
						_lpVideoCtrl->status = emVIDEO_CTRL_DYNAMIC_HIGH;
					}
				}else{
					_lpVideoCtrl->dynamic_param.trun_up_flag = NK_False;
					_lpVideoCtrl->dynamic_param.delay_us = _lpVideoCtrl->high_keep_on_us;
				}
			}
			/*高帧状态倒数计时*/
			else if(_lpVideoCtrl->status == emVIDEO_CTRL_DYNAMIC_HIGH) {
				if(_lpVideoCtrl->dynamic_param.delay_us > 0){
					--_lpVideoCtrl->dynamic_param.delay_us;
				}else{
					_lpVideoCtrl->status = emVIDEO_CTRL_DYNAMIC_LOW;
				}
			}
			/*启动低帧参数*/
			else if(_lpVideoCtrl->status == emVIDEO_CTRL_DYNAMIC_LOW) {
				if(0 == _lpVideoCtrl->eventSet.onSetVideoParam(&_lpVideoCtrl->dynamic_param.low_conf)) {
					_lpVideoCtrl->status = emVIDEO_CTRL_NONE;
				}
			}
			else{
				//do nothing
			}
		}

SLEEP_AND_NEXT_ROUND:
		/*attention:睡眠时间与高帧状态倒数计时相关*/
		usleep(SLEEP_TIME_US);
		//APP_TRACE("status : %d...%d...",_lpVideoCtrl->status,_lpVideoCtrl->dynamic_param.delay_us);
	}
}

NK_Int NK_VIDEO_CTRL_init(lpVIDEO_CTRL_FUNCTION init,lpVIDEO_PARAM_T high,lpVIDEO_PARAM_T low,NK_Int high_keep_on_sec)
{
	NK_Int ret = 0;

	NK_EXPECT_VERBOSE_RETURN_VAL(((init != NULL) && (high != NULL) && (low != NULL)),-1);
	if(!_lpVideoCtrl) {
		_lpVideoCtrl = calloc(sizeof(stVIDEO_CTRL_T),1);
		_lpVideoCtrl->eventSet.onCheckVideoParam = init->onCheckVideoParam;
		_lpVideoCtrl->eventSet.onGetVideoParam = init->onGetVideoParam;
		_lpVideoCtrl->eventSet.onSetVideoParam = init->onSetVideoParam;
		/*attention:与ctrller线程睡眠等待时间相关,注意计算倍数关系*/
		if(high_keep_on_sec <= 0) {
			_lpVideoCtrl->high_keep_on_us = 10 * (1000000 / SLEEP_TIME_US);
		}else{
			_lpVideoCtrl->high_keep_on_us = high_keep_on_sec * (1000000 / SLEEP_TIME_US);
		}
		_lpVideoCtrl->status = emVIDEO_CTRL_DYNAMIC_HIGH;
		//init static conf
		_lpVideoCtrl->static_param.static_flag = NK_False;
		//init dynamic conf
		memcpy(&_lpVideoCtrl->dynamic_param.low_conf, low,sizeof(stVIDEO_PARAM_T));
		memcpy(&_lpVideoCtrl->dynamic_param.high_conf, high,sizeof(stVIDEO_PARAM_T));
		_lpVideoCtrl->dynamic_param.delay_us = _lpVideoCtrl->high_keep_on_us;
		_lpVideoCtrl->dynamic_param.trun_up_flag = NK_False;
		//start ctrl thread
		_lpVideoCtrl->ctrl_trigger = NK_True;
		ret = pthread_create(&_lpVideoCtrl->ctrl_pid, NULL, ctrller, NULL);
		if(0 != ret) {
			APP_TRACE("video ctrl ctrller thread create failed!");
			free(_lpVideoCtrl);
			_lpVideoCtrl = NULL;
			return -1;
		}
		//APP_TRACE("video ctrl module init success!");
	}else{
		APP_TRACE("video ctrl module has been inited!");
	}

	return 0;
}

NK_Int NK_VIDEO_CTRL_start_static_conf(lpVIDEO_PARAM_T videoParam)
{
	NK_EXPECT_VERBOSE_RETURN_VAL(((_lpVideoCtrl != NULL) && (_lpVideoCtrl->eventSet.onSetVideoParam != NULL) && (videoParam != NULL)),-1);
	NK_EXPECT_VERBOSE_RETURN_VAL(((_lpVideoCtrl->eventSet.onCheckVideoParam != NULL) && (_lpVideoCtrl->eventSet.onSetVideoParam != NULL)),-1);
	if(NK_True != _lpVideoCtrl->eventSet.onCheckVideoParam(videoParam)) {
		APP_TRACE("static param is illegality");
		return -1;
	}
	memcpy(&_lpVideoCtrl->static_param.sta_conf, videoParam, sizeof(stVIDEO_PARAM_T));
	_lpVideoCtrl->static_param.static_flag = NK_True;

	return 0;
}

NK_Int NK_VIDEO_CTRL_stop_static_conf()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_lpVideoCtrl != NULL),-1);
	_lpVideoCtrl->static_param.static_flag = NK_False;
	_lpVideoCtrl->dynamic_param.trun_up_flag = NK_False;
	_lpVideoCtrl->status = emVIDEO_CTRL_NONE;

	return 0;
}

NK_Int NK_VIDEO_CTRL_set_dynamic_conf(emVIDEO_CTRL_STATUS_T highOrLow,lpVIDEO_PARAM_T videoParam)
{
	NK_EXPECT_VERBOSE_RETURN_VAL(((_lpVideoCtrl != NULL) && (videoParam != NULL)),-1);
	NK_EXPECT_VERBOSE_RETURN_VAL(((_lpVideoCtrl->eventSet.onCheckVideoParam != NULL) && (_lpVideoCtrl->eventSet.onSetVideoParam != NULL)),-1);
	if(NK_True != _lpVideoCtrl->eventSet.onCheckVideoParam(videoParam)) {
		APP_TRACE("dynamic param is illegality");
		return -1;
	}
	if(highOrLow == emVIDEO_CTRL_DYNAMIC_HIGH) {
		memcpy(&_lpVideoCtrl->dynamic_param.high_conf, videoParam, sizeof(stVIDEO_PARAM_T));
	}else if(highOrLow == emVIDEO_CTRL_DYNAMIC_LOW) {
		memcpy(&_lpVideoCtrl->dynamic_param.low_conf, videoParam, sizeof(stVIDEO_PARAM_T));
	}

	return 0;
}

NK_Int NK_VIDEO_CTRL_trun_up_conf()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_lpVideoCtrl != NULL),-1);
	_lpVideoCtrl->dynamic_param.trun_up_flag = NK_True;

	return 0;
}

NK_Int NK_VIDEO_CTRL_get_cur_conf(lpVIDEO_PARAM_T videoParam)
{
	NK_EXPECT_VERBOSE_RETURN_VAL(((_lpVideoCtrl != NULL) && (_lpVideoCtrl->eventSet.onGetVideoParam != NULL) && (videoParam != NULL)),-1);
	return _lpVideoCtrl->eventSet.onGetVideoParam(videoParam);
}

emVIDEO_CTRL_STATUS_T NK_VIDEO_CTRL_get_cur_conf_status()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_lpVideoCtrl != NULL),-1);
	return _lpVideoCtrl->status;
}

NK_Int NK_VIDEO_CTRL_destroy()
{
	if(_lpVideoCtrl){
		_lpVideoCtrl->ctrl_trigger = NK_False;
		pthread_join(_lpVideoCtrl->ctrl_pid, NULL);
		free(_lpVideoCtrl);
	}
	APP_TRACE("NK tfcard destroy success!");
	_lpVideoCtrl = NULL;

	return 0;
}

#endif

