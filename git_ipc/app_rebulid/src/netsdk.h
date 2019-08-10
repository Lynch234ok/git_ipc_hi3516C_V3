
#include <time.h>
#include <sys/timeb.h>
#include "web_server.h"
#include "netsdk_json.h"
#include "netsdk_def.h"

#ifndef NETSDK_H_
#define NETSDK_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kNSDK_INS_RET_CONTENT_READY (2L)
#define kNSDK_INS_RET_CONTENT_SENT (1L)
#define kNSDK_INS_RET_OK (0L)
#define kNSDK_INS_RET_DEVICE_BUSY (-1L)
#define kNSDK_INS_RET_DEVICE_ERROR (-2L)
#define kNSDK_INS_RET_DEVICE_NOT_SUPPORT (-3L)
#define kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT (-4L)
#define kNSDK_INS_RET_INVALID_OPERATION (-5L)
#define kNSDK_INS_RET_INVALID_DOCUMENT (-6L)
#define kNSDK_INS_RET_UNKNOWN_ERROR (-7L)

extern int NETSDK_video_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri, char *content, int content_max);
extern void NETSDK_video_conf_save();
extern void NETSDK_video_conf_save2();

extern int NETSDK_http_io_service(LP_HTTP_CONTEXT context, HTTP_CSTR_t cap_uri);

extern int NETSDK_http_service(LP_HTTP_CONTEXT context);

extern void NETSDK_conf_system_save();
extern void NETSDK_conf_system_save2();

extern LP_NSDK_SYSTEM_TIME NETSDK_conf_system_get_time(LP_NSDK_SYSTEM_TIME sys_time);
extern LP_NSDK_SYSTEM_TIME NETSDK_conf_system_set_time(LP_NSDK_SYSTEM_TIME sys_time);

extern void NETSDK_conf_audio_save();
extern void NETSDK_conf_audio_save2();

extern void NETSDK_conf_video_save();
extern void NETSDK_conf_video_save2();
extern void NETSDK_venc_ch_delay_set(int id, void* venc_ch);


extern LP_NSDK_SYSTEM_DEVICE_INFO NETSDK_conf_system_get_device_info(LP_NSDK_SYSTEM_DEVICE_INFO device_info);
extern LP_NSDK_SYSTEM_DEVICE_INFO NETSDK_conf_system_set_device_info(LP_NSDK_SYSTEM_DEVICE_INFO device_info);
extern void NETSDK_conf_system_operation(EM_NSDK_SYSTEM_OPERATION operation);
extern LP_NSDK_SYSTEM_SETTING NETSDK_conf_system_get_setting_info(LP_NSDK_SYSTEM_SETTING info);
extern LP_NSDK_SYSTEM_SETTING NETSDK_conf_system_set_setting_info(LP_NSDK_SYSTEM_SETTING info);
extern LP_NSDK_SYSTEM_DST NETSDK_conf_system_get_DST_info(LP_NSDK_SYSTEM_DST info);
extern LP_NSDK_SYSTEM_DST NETSDK_conf_system_set_DST_info(LP_NSDK_SYSTEM_DST info);
extern LP_NSDK_SYSTEM_REC_MANAGER NETSDK_conf_system_get_record_info(LP_NSDK_SYSTEM_REC_MANAGER recManager);
extern LP_NSDK_SYSTEM_REC_MANAGER NETSDK_conf_system_set_record_info(LP_NSDK_SYSTEM_REC_MANAGER recManager);

extern LP_NSDK_VIN_CH NETSDK_conf_vin_ch_get(int id, LP_NSDK_VIN_CH vin_ch);
extern LP_NSDK_VIN_CH NETSDK_conf_vin_ch_set(int id, LP_NSDK_VIN_CH vin_ch);
extern LP_NSDK_AIN_CH NETSDK_conf_ain_ch_get(int id, LP_NSDK_AIN_CH ain_ch);
extern LP_NSDK_AIN_CH NETSDK_conf_ain_ch_set(int id, LP_NSDK_AIN_CH ain_ch);
extern LP_NSDK_MD_CH NETSDK_conf_md_ch_get(int id, LP_NSDK_MD_CH md_ch);
extern LP_NSDK_MD_CH NETSDK_conf_md_ch_set(int id, LP_NSDK_MD_CH md_ch);
extern LP_NSDK_VENC_CH NETSDK_conf_venc_ch_get(int id, LP_NSDK_VENC_CH venc_ch);
extern LP_NSDK_VENC_CH NETSDK_conf_venc_ch_set(int id, LP_NSDK_VENC_CH venc_ch);
extern LP_NSDK_VENC_CH NETSDK_conf_venc_ch_set2(int id, LP_NSDK_VENC_CH venc_ch, bool immediate);
extern LP_NSDK_AIN_CH NETSDK_conf_ain_ch_get(int id, LP_NSDK_AIN_CH ain_ch);
extern LP_NSDK_AIN_CH NETSDK_conf_ain_ch_set(int id, LP_NSDK_AIN_CH ain_ch);
extern LP_NSDK_AENC_CH NETSDK_conf_aenc_ch_get(int id, LP_NSDK_AENC_CH aenc_ch);
extern LP_NSDK_AENC_CH NETSDK_conf_aenc_ch_set(int id, LP_NSDK_AENC_CH aenc_ch);
extern int NETSDK_venc_snapshot(int id, int type, int width, int height, char *file_path);
//must be call after calling NETSDK_venc_snapshot()
extern void NETSDK_venc_free_snapshot(char *file_path);
extern int NETSDK_venc_get_channels();


extern LP_NSDK_ALARM_IN_CH NETSDK_conf_alarm_in_ch_get(int id, LP_NSDK_ALARM_IN_CH alarmInCh);
extern LP_NSDK_ALARM_IN_CH NETSDK_conf_alarm_int_ch_set(int id, LP_NSDK_ALARM_IN_CH alarmInCh);
extern LP_NSDK_ALARM_OUT_CH NETSDK_conf_alarm_out_ch_get(int id, LP_NSDK_ALARM_OUT_CH alarmOutCh);
extern LP_NSDK_ALARM_OUT_CH NETSDK_conf_alarm_out_ch_set(int id, LP_NSDK_ALARM_OUT_CH alarmOutCh);

extern LP_NSDK_NETWORK_INTERFACE NETSDK_conf_interface_get(int id, LP_NSDK_NETWORK_INTERFACE n_interface);
extern LP_NSDK_NETWORK_INTERFACE NETSDK_conf_interface_set_by_delay(int id, LP_NSDK_NETWORK_INTERFACE n_interface, EM_NSDK_CONF_SAVE_OPERATION opteration, int delay);
extern LP_NSDK_NETWORK_INTERFACE NETSDK_conf_interface_set(int id, LP_NSDK_NETWORK_INTERFACE n_interface, EM_NSDK_CONF_SAVE_OPERATION opteration);
extern LP_NSDK_NETWORK_PORT NETSDK_conf_port_get(int id, LP_NSDK_NETWORK_PORT port);
extern LP_NSDK_NETWORK_PORT NETSDK_conf_port_set_by_delay(int id, LP_NSDK_NETWORK_PORT port, EM_NSDK_CONF_SAVE_OPERATION opteration, int delay);
extern LP_NSDK_NETWORK_PORT NETSDK_conf_port_set(int id, LP_NSDK_NETWORK_PORT port,  EM_NSDK_CONF_SAVE_OPERATION opteration);
extern int NETSDK_tmp_interface_set(int id, LP_NSDK_NETWORK_INTERFACE n_interface, EM_NSDK_CONF_SAVE_OPERATION opteration);
extern int NETSDK_tmp_interface_get(int id, LP_NSDK_NETWORK_INTERFACE n_interface);

extern LP_NSDK_PTZ_CFG NETSDK_conf_ptz_ch_get(LP_NSDK_PTZ_CFG pst_PtzConfig);
extern inline  LP_NSDK_PTZ_CFG NETSDK_conf_ptz_ch_set(LP_NSDK_PTZ_CFG pst_PtzConfig);


extern void NETSDK_conf_io_save();
extern void NETSDK_conf_io_save2();

extern void NETSDK_conf_ptz_save();
extern void NETSDK_conf_ptz_save2();
extern void NETSDK_fix_resolution_opt(int sensor_type);

extern void NETSDK_conf_image_save();
extern LP_NSDK_IMAGE NETSDK_conf_image_get(LP_NSDK_IMAGE image);
extern LP_NSDK_IMAGE NETSDK_conf_image_set(LP_NSDK_IMAGE image);
extern LP_NSDK_IMAGE NETSDK_conf_image_set2(LP_NSDK_IMAGE image, bool immediate);


#define NSDK_PROPERTIES(__str) (NULL != strstr(__str, "/PROPERTIES"))
#define NSDK_SUBURI_MATCH(__token,__subURI,__prefix) \
	((__token = __prefix, 0 == strncmp((__token), (__subURI), strlen(__token))) ?\
	(__subURI += strlen(__token), true) : false)

typedef struct NSDK_INTERFACE {
	fNSDK_SYS_DEV_INFO systemDeviceInfo;
	fNSDK_VIN_CH_CHG videoInputChannelChanged;
	fNSDK_AIN_CH_CHG audioInputChannelChanged;
	fNSDK_MD_CH_CHG motionDetectionChannelChanged;
	fNSDK_MD_CH_STATUS motionDetectionChannelStatus;
	fNSDK_VENC_CH_CHG videoEncodeChannelChanged;
	fNSDK_AENC_CH_CHG audioEncodeChannelChanged;
	fNSDK_VENC_REQUEST_KEYFRAME videoEncodeRequestKeyFrame;
	fNSDK_VENC_SNAPSHOT videoEncodeSnapShot;
	fNSDK_ALARM_IN_CH_CHG alarmInputChannelChanged;
	fNSDK_ALARM_OUT_CH_CHG alarmOutputChannelChanged;
	fNSDK_ALARM_OUT_CH_TRIGGER alarmOutputChannelTrigger;

	// callback to delay save
	fNSDK_CONF_SAVE audio_conf_save;
	fNSDK_CONF_SAVE video_conf_save;
	fNSDK_CONF_SAVE io_conf_save;
	fNSDK_CONF_SAVE system_conf_save;
}ST_NSDK_INTERFACE, *LP_NSDK_INTERFACE;


typedef struct NSDK_INIT {
	char confDirectory[128], defConfDirectory[128];

	fNSDK_SYS_DEV_INFO systemDeviceInfo;
	fNSDK_SYS_OPERATION systemOperation;
	fNSDK_VIN_CH_CHG videoInputChannelChanged;
	fNSDK_AIN_CH_CHG audioInputChannelChanged;
	fNSDK_MD_CH_CHG motionDetectionChannelChanged;
	fNSDK_MD_CH_STATUS motionDetectionChannelStatus;
	fNSDK_VENC_CH_CHG videoEncodeChannelChanged;
	fNSDK_AENC_CH_CHG audioEncodeChannelChanged;
	fNSDK_VENC_REQUEST_KEYFRAME videoEncodeRequestKeyFrame;
	fNSDK_VENC_SNAPSHOT videoEncodeSnapShot;
	fNSDK_SYSTEM_CHG systemChanged;
	fNSDK_SYSTEM_DST_CHG systemDSTChanged;
	fNSDK_WIFI_STATUS networkWirelessStatus;
	fNSDK_IMAGE_CHG imageChanged;
	fNSDK_IMAGE_AF_STATUS AutoFocusStatus;
	fNSDK_SYS_REMOTE_UPGRADE_STATUS remoteUpgradeStatus;
	fNSDK_SYS_REMOTE_UPGRADE_ERROR remoteUpgradeError;

	fNSDK_ALARM_IN_CH_PORT_STATUS alarmInputChannelPortStatus;
	fNSDK_ALARM_OUT_CH_PORT_STATUS alarmOutputChannelPortStatus;

	fNSDK_ALARM_IN_CH_CHG alarmInputChannelChanged;
	fNSDK_ALARM_OUT_CH_CHG alarmOutputChannelChanged;
	fNSDK_ALARM_OUT_CH_TRIGGER alarmOutputChannelTrigger;
	fNSDK_PTZ_UART_CONFIG_CHANGED ptzUartConfigChanged;

	// callback to delay save
	fNSDK_CONF_SAVE audio_conf_save;
	fNSDK_CONF_SAVE video_conf_save;
	fNSDK_CONF_SAVE io_conf_save;
	fNSDK_CONF_SAVE system_conf_save;
	fNSDK_CONF_SAVE network_conf_save;
	fNSDK_CONF_SAVE image_conf_save;
}ST_NSDK_INIT, *LP_NSDK_INIT;

extern int NETSDK_conf_save(LP_JSON_OBJECT json, const char *fileName);

extern int NETSDK_init(LP_NSDK_INIT para);
extern void NETSDK_destroy();

/**
	 *载入配置、解析配置文件
	 *
	 * @param[in]	reload 是否重载

	 * @return		返回0
	 */
extern int NETSDK_conf_load(bool reload);

/**
	 *获取多配置模式
	 *
	 * @param[in]		ret_mode			返回模式变量(字符串)
	 	sNSDK_MULTI_CONF_MODE_INSIDE
	 	sNSDK_MULTI_CONF_MODE_OUTSIDE
	 	sNSDK_MULTI_CONF_MODE_CUSTOM
	 * @return		返回模式整形变量(整形)
	      kSDK_MULTI_CONF_MODE_INSIDE
	      kSDK_MULTI_CONF_MODE_OUTSIDE
	      kSDK_MULTI_CONF_MODE_CUSTOM
	 */
extern int NETSDK_get_multi_conf_mode(char *ret_mode);

/**
	 *设置多配置模式

	 * @param[in]		mode		设置配置模式(字符串)
	 *  sNSDK_MULTI_CONF_MODE_INSIDE
	 	sNSDK_MULTI_CONF_MODE_OUTSIDE
	 	sNSDK_MULTI_CONF_MODE_CUSTOM

	 * @return		写入成功，返回0，写入失败返回 -1。
	 */
extern int NETSDK_set_multi_conf_mode(char *mode);

// FIXME:
extern int NETSDK_sdcard_media_search(LP_HTTP_CONTEXT context);
extern int NETSDK_sdcard_media_playback_flv(LP_HTTP_CONTEXT context);
extern int NETSDK_sdcard_format(LP_HTTP_CONTEXT context);
extern int NETSDK_sdcard_status(LP_HTTP_CONTEXT context);
extern int NETSDK_sdcard_checkRW(LP_HTTP_CONTEXT context);
extern int NETSDK_sdcard_file(LP_HTTP_CONTEXT context);


#ifdef __cplusplus
};
#endif
#endif //NETSDK_H_

