

#include "netsdk_json.h"
#include <pthread.h>
#include "netsdk_def.h"

#ifndef NETSDKV10_PRIVATE_H_
#define NETSDKV10_PRIVATE_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct NSDK_PRIVATE {
	char confDirectory[128], defConfDirectory[128];

	LP_JSON_OBJECT jsonSystem;
	LP_JSON_OBJECT audio_conf;
	LP_JSON_OBJECT video_conf;
	LP_JSON_OBJECT io_conf;
	LP_JSON_OBJECT ptz_conf;
	LP_JSON_OBJECT stream_conf;
	LP_JSON_OBJECT network_conf;
	LP_JSON_OBJECT image_conf;

	pthread_rwlock_t system_sync, audio_sync, video_sync, io_sync, ptz_sync, network_sync, image_sync;
    int lock_sync_enabled;
	fNSDK_CONF_SAVE system_conf_save;
	fNSDK_CONF_SAVE audio_conf_save;
	fNSDK_CONF_SAVE video_conf_save;
	fNSDK_CONF_SAVE io_conf_save;
	fNSDK_CONF_SAVE ptz_conf_save;
	fNSDK_CONF_SAVE network_conf_save;
	fNSDK_CONF_SAVE image_conf_save;

	// callback function
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


}ST_NSDK_PRIVATE, *LP_NSDK_PRIVATE;

extern LP_NSDK_PRIVATE netsdk;
extern LP_NSDK_PRIVATE NETSDK_private_get();
extern void NETSDK_private_put();

typedef struct NSDK_MAP_STR_DEC {
	const char *str;
	int dec;
}ST_NSDK_MAP_STR_DEC, *LP_NSDK_MAP_STR_DEC;

extern int NETSDK_map_str2dec(const ST_NSDK_MAP_STR_DEC map[], int map_items, const char *str_key, int def_val);
extern const char *NETSDK_map_dec2str(const ST_NSDK_MAP_STR_DEC map[], int map_items, int dec_key, const char *def_val);

#define NETSDK_MAP_STR2DEC(__map,__str_key,__def_val) \
	NETSDK_map_str2dec(__map,sizeof(__map)/sizeof(__map[0]),__str_key, __def_val)
#define NETSDK_MAP_DEC2STR(__map,__dec_key,__def_val) \
	NETSDK_map_dec2str(__map,sizeof(__map)/sizeof(__map[0]),__dec_key,__def_val)

extern int NETSDK_private_read_lock(pthread_rwlock_t *sync);
extern int NETSDK_private_try_read_lock(pthread_rwlock_t *sync);
extern int NETSDK_private_write_lock(pthread_rwlock_t *sync);
extern int NETSDK_private_try_write_lock(pthread_rwlock_t *sync);
extern int NETSDK_private_unlock(pthread_rwlock_t *sync);

#ifdef __cplusplus
extern "C" {
#endif
#endif //NETSDKV10_PRIVATE_H_

