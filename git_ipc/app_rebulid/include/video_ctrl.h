#ifdef VIDEO_CTRL

#ifndef _NK_VIDER_CTRL_H
#define _NK_VIDER_CTRL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <NkUtils/types.h>


typedef enum {
	emVIDEO_CTRL_NONE = 0,
	emVIDEO_CTRL_STATIC,
	emVIDEO_CTRL_DYNAMIC_LOW,
	emVIDEO_CTRL_DYNAMIC_HIGH,
}emVIDEO_CTRL_STATUS_T;

/**
 * 视频控制参数
 */
typedef struct VIDEO_PARAM_T {
	NK_Size bps;
	NK_Size gop;
	NK_Size fps;
	NK_Boolean updateI;
}stVIDEO_PARAM_T,*lpVIDEO_PARAM_T;


/**
 * 检查视频参数合法性事件
 */
typedef NK_Boolean (*fCtrlOnCheckVideoParam)(lpVIDEO_PARAM_T videoParam);

/**
 * 获取视频参数事件
 */
typedef NK_Int (*fCtrlOnGetVideoParam)(lpVIDEO_PARAM_T videoParam);

/**
 * 设置视频参数事件
 */
typedef NK_Int (*fCtrlOnSetVideoParam)(lpVIDEO_PARAM_T videoParam);

/**
 * 音视频参数控制事件集
 **/
typedef struct VIDEO_CTRL_FUNCTION {
	fCtrlOnCheckVideoParam onCheckVideoParam;
	fCtrlOnGetVideoParam onGetVideoParam;
	fCtrlOnSetVideoParam onSetVideoParam;
}stVIDEO_CTRL_FUNCTION,*lpVIDEO_CTRL_FUNCTION;


extern NK_Int NK_VIDEO_CTRL_start_static_conf(lpVIDEO_PARAM_T videoParam);
extern NK_Int NK_VIDEO_CTRL_stop_static_conf();
extern NK_Int NK_VIDEO_CTRL_set_dynamic_conf(emVIDEO_CTRL_STATUS_T highOrLow,lpVIDEO_PARAM_T videoParam);
extern NK_Int NK_VIDEO_CTRL_trun_up_conf();
extern NK_Int NK_VIDEO_CTRL_get_cur_conf(lpVIDEO_PARAM_T videoParam);
extern emVIDEO_CTRL_STATUS_T NK_VIDEO_CTRL_get_cur_conf_status();
extern NK_Int NK_VIDEO_CTRL_init(lpVIDEO_CTRL_FUNCTION init,lpVIDEO_PARAM_T high,lpVIDEO_PARAM_T low,NK_Int high_keep_on_sec);
extern NK_Int NK_VIDEO_CTRL_destroy();


#ifdef __cplusplus
};
#endif
#endif /*_NK_VIDER_CTRL_H*/

#endif

