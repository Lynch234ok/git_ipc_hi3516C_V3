
#include <stdbool.h>
#include <stdint.h>

#ifndef NETSDKV10_DEF_H_
#define NETSDKV10_DEF_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct NSDK_PARA_PROPERTY {
	char opt[512];
}ST_NSDK_PARA_PROPERTY, LP_NSDK_PARA_PROPERTY;

typedef struct NSDK_CONSTBITRATE_PROPERTY {
	int min;
	int max;
}ST_NSDK_VENC_CONSTBITRATE_PROPERTY, LP_NSDK_VENC_CONSTBITRATE_PROPERTY;



typedef struct NSDK_AIN_CH {
	int id;
#define kNSDK_AIN_WORK_MODE_INPUT (1<<0)
#define kNSDK_AIN_WORK_MODE_OUTPUT (1<<1)
#define kNSDK_AIN_WORK_MODE_IO (kNSDK_AIN_WORK_MODE_INPUT | kNSDK_AIN_WORK_MODE_OUTPUT)
	int workMode;
	int sampleRate, sampleBitWidth;
	int inputVolume, outputVolume;
	
#define kNSDK_ACTIVE_PICKUP (0)
#define kNSDK_PASSIVE_MIC (1)
	int microphoneType;

}ST_NSDK_AIN_CH, *LP_NSDK_AIN_CH;

typedef struct NSDK_AENC_CH {
	int id;
	bool enabled;
	int audioInputChannelID;
#define kNSDK_AENC_CODEC_TYPE_PCM (1<<0)
#define kNSDK_AENC_CODEC_TYPE_G711A (1<<1)
#define kNSDK_AENC_CODEC_TYPE_G711U (1<<2)
#define kNSDK_AENC_CODEC_TYPE_AAC (1<<3)
	int codecType;
}ST_NSDK_AENC_CH, *LP_NSDK_AENC_CH;

typedef struct NSDK_VIN_PRIVACY_MASK_REGION {
	int id;
	bool enabled;
	float regionX, regionY, regionWidth, regionHeight;
	uint32_t regionColor;
}ST_NSDK_VIN_PRIVACY_MASK_REGION, *LP_NSDK_VIN_PRIVACY_MASK_REGION;

typedef struct NSDK_VIN_CH {
	int id;
	int powerLineFrequencyMode;
	int captureWidth, captureHeight;
	int captureFrameRate;
	int brightnessLevel, contrastLevel, sharpnessLevel, saturationLevel, hueLevel;
	bool flip, mirror;
	ST_NSDK_VIN_PRIVACY_MASK_REGION privacyMask[4];
}ST_NSDK_VIN_CH, *LP_NSDK_VIN_CH;

typedef struct NSDK_VENC_OVERLAY {
	bool enabled;
	float regionX, regionY;
}ST_NSDK_VENC_OVERLAY, *LP_NSDK_VENC_OVERLAY;

typedef struct NSDK_VENC_CHANNEL_NAME_OVERLAY {
	ST_NSDK_VENC_OVERLAY o;
}ST_NSDK_VENC_CHANNEL_NAME_OVERLAY, *LP_NSDK_VENC_CHANNEL_NAME_OVERLAY;

typedef struct NSDK_VENC_DATETIME_OVERLAY {
	ST_NSDK_VENC_OVERLAY o;
#define kNSDK_DATETIME_FMT_SLASH_YYYYMMDD (1<<0)
#define kNSDK_DATETIME_FMT_SLASH_MMDDYYYY (1<<1)
#define kNSDK_DATETIME_FMT_SLASH_DDMMYYYY (1<<2)
#define kNSDK_DATETIME_FMT_DASH_YYYYMMDD (1<<3)
#define kNSDK_DATETIME_FMT_DASH_MMDDYYYY (1<<4)
#define kNSDK_DATETIME_FMT_DASH_DDMMYYYY (1<<5)
	int dateFormat;
	int timeFormat; // 12, 24
	bool displayWeek;
}ST_NSDK_VENC_DATETIME_OVERLAY, *LP_NSDK_VENC_DATETIME_OVERLAY;

typedef struct NSDK_VENC_DEVICE_ID_OVERLAY {
	ST_NSDK_VENC_OVERLAY o;
}ST_NSDK_VENC_DEVICE_ID_OVERLAY, *LP_NSDK_VENC_DEVICE_ID_OVERLAY;

typedef struct NSDK_VENC_TEXT_OVERLAY {
	ST_NSDK_VENC_OVERLAY o;
	int id;
	char message[64 + 1];
}ST_NSDK_VENC_TEXT_OVERLAY, *LP_NSDK_VENC_TEXT_OVERLAY;

typedef struct NSDK_VENC_CH {
	int id;
	char channelName[64 + 1];
	bool enabled;
	int videoInputChannelID;
#define kNSDK_CODEC_TYPE_H264 (0)
#define kNSDK_CODEC_TYPE_H265 (1)
	int codecType;
	char codecTypeOpt[128];
#define kSDK_ENC_H264_PROFILE_BASELINE (1)
#define kSDK_ENC_H264_PROFILE_MAIN (2)
#define kSDK_ENC_H264_PROFILE_HIGH (3)

#define kSDK_ENC_H265_PROFILE_MAIN (0)

	int h264Profile;

#define kNSDK_RES_2592X1944 ((2592<<16)+1944)
#define kNSDK_RES_2160X2160 ((2160<<16)+2160)
#define kNSDK_RES_1944X1944 ((1944<<16)+1944)
#define kNSDK_RES_2592X1520 ((2592<<16)+1520)
#define kNSDK_RES_2048X1520 ((2048<<16)+1520)
#define kNSDK_RES_1920X1440 ((1920<<16)+1440)
#define kNSDK_RES_1920X1080 ((1920<<16)+1080)
#define kNSDK_RES_1280X960 ((1280<<16)+960)
#define kNSDK_RES_1280X720 ((1280<<16)+720)
#define kNSDK_RES_1024X768 ((1024<<16)+768)
#define kNSDK_RES_1024X576 ((1024<<16)+576)
#define kNSDK_RES_960X576 ((960<<16)+576)
#define kNSDK_RES_960X480 ((960<<16)+480)
#define kNSDK_RES_720X576 ((720<<16)+576)
#define kNSDK_RES_720X480 ((720<<16)+480)
#define kNSDK_RES_640X480 ((640<<16)+480)
#define kNSDK_RES_640X360 ((640<<16)+360)
#define kNSDK_RES_352X288 ((352<<16)+288)
#define kNSDK_RES_352X240 ((352<<16)+240)
#define kNSDK_RES_320X240 ((320<<16)+240)
#define kNSDK_RES_320X180 ((320<<16)+180)
#define kNSDK_RES_176X144 ((176<<16)+144)
#define kNSDK_RES_176X120 ((176<<16)+120)
#define kNSDK_RES_160X120 ((160<<16)+120)
#define kNSDK_RES_160X90 ((160<<16)+90)
#define kNSDK_RES_1280X1024 ((1280<<16)+1024)
#define kNSDK_RES_1280X1280 ((1280<<16)+1280)
#define kNSDK_RES_1408X1408 ((1408<<16)+1408)
#define kNSDK_RES_1536X1280 ((1536<<16)+1280)
#define kNSDK_RES_1520X1520 ((1520<<16)+1520)
#define kNSDK_RES_1536X1536 ((1536<<16)+1536)
#define kNSDK_RES_960X720 ((960<<16)+720)
#define kNSDK_RES_1440X1440 ((1440<<16)+1440)
#define kNSDK_RES_320X320 ((320<<16)+320)
#define kNSDK_RES_640X640 ((640<<16)+640)
#define kNSDK_RES_1536X784 ((1536<<16)+784)



	int resolution;
	ST_NSDK_PARA_PROPERTY resolutionProperty;
	bool freeResolution;
	int resolutionWidth, resolutionHeight;
#define kNSDK_BR_CONTROL_CBR (0)
#define kNSDK_BR_CONTROL_VBR (1)
	int bitRateControlType;
	int constantBitRate; // in kbps
	ST_NSDK_VENC_CONSTBITRATE_PROPERTY constantBitRateProperty;
	int frameRate;
	int keyFrameInterval;
#define kNSDK_SNAPSHOT_IMAGE_JPEG (0)
	int snapShotImageType;

#define kNSDK_DEFINITION_AUTO (0)
#define kNSDK_DEFINITION_FLUENCY (1)
#define kNSDK_DEFINITION_BD (2)
#define kNSDK_DEFINITION_HD (3)

	int definitionType;
	ST_NSDK_VENC_CHANNEL_NAME_OVERLAY channelNameOverlay;
	ST_NSDK_VENC_DATETIME_OVERLAY datetimeOverlay;
	ST_NSDK_VENC_DEVICE_ID_OVERLAY deviceIDOverlay;
	ST_NSDK_VENC_TEXT_OVERLAY textOverlay[2];
	int ImageTransmissionModel; 
	
}ST_NSDK_VENC_CH, *LP_NSDK_VENC_CH;

typedef struct NSDK_MD_GRID {
	int rowGranularity, columnGranularity; // read-only
	uint32_t granularity[64]; // hot-bit actived
	int sensitivityLevel; // 0 - 100

	bool (*getGranularity)(struct NSDK_MD_GRID *const _this, int row, int column);
	void (*setGranularity)(struct NSDK_MD_GRID *const _this, int row, int column, bool flag);
}ST_NSDK_MD_GRID, *LP_NSDK_MD_GRID;

typedef struct NSDK_MD_REGION_CH {
	int id; // read-only
	bool enabled;
	float regionX, regionY, regionWidth, regionHeight; // 0 - 100
	int sensitivityLevel;
}ST_NSDK_MD_REGION_CH, *LP_NSDK_MD_REGION_CH;

typedef struct NSDK_MD_REGION {
	ST_NSDK_MD_REGION_CH ch[4];
}ST_NSDK_MD_REGION, *LP_NSDK_MD_REGION;

typedef struct NSDK_MD_CH {
	int id;
	bool enabled;
#define kNSDK_MD_TYPE_GRID (1<<0)
#define kNSDK_MD_TYPE_REGION (1<<1)
	int detectionType;
	ST_NSDK_MD_GRID detectionGrid;
	ST_NSDK_MD_REGION detectionRegion;
}ST_NSDK_MD_CH, *LP_NSDK_MD_CH;

#define kNSDK_IO_STATE_LOW (1<<0)
#define kNSDK_IO_STATE_HIGH (1<<1)
#define kNSDK_IO_STATE_PULSE (1<<2)

typedef struct NSDK_ALARM_IN_CH {
	int id;
	int defaultState;
	int activeState;
}ST_NSDK_ALARM_IN_CH, *LP_NSDK_ALARM_IN_CH;

typedef struct NSDK_ALARM_OUT_CH {
	int id;
	int defaultState, activeState;
#define kNSDK_IO_PWR_ON_METHOD_CONT (0)
#define kNSDK_IO_PWR_ON_METHOD_PULSE (1)
	int powerOnMethod;
	int pulseDuration;
}ST_NSDK_ALARM_OUT_CH, *LP_NSDK_ALARM_OUT_CH;

typedef struct NSDK_ALARM_OUT_CH_TRIGGER_METHOD {
	int outputState;
	int pulseDuration; // if output state pulse, indicate the output pulse duration
	int pulseState; // if high trigger a pulse _|~|_, else trigger a pulse ~|_|~
}ST_NSDK_ALARM_OUT_CH_TRIGGER_METHOD, *LP_NSDK_ALARM_OUT_CH_TRIGGER_METHOD;

typedef struct NSDK_NETWORK_PORT{
	int id;
	char portName[32];
	int value;
}ST_NSDK_NETWORK_PORT, *LP_NSDK_NETWORK_PORT;

typedef struct NSDK_NETWORK_LAN{
#define kNSDK_NETWORK_LAN_IP_VERSION_V4 (0)
#define kNSDK_NETWORK_LAN_IP_VERSION_V6 (1)
	int ipVersion;
#define kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC (0)
#define kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC (1)
	int addressingType;
	char staticIP[64];
	char staticNetmask[64];
	char staticGateway[64];
}ST_NSDK_NETWORK_LAN, *LP_NSDK_NETWORK_LAN;

typedef struct NSDK_NETWORK_VIRTUALLAN{
	char staticIP[32];
	char staticNetmask[32];
}ST_NSDK_NETWORK_VIRTUALLAN, *LP_NSDK_NETWORK_VIRTUALLAN;

typedef struct NSDK_NETWORK_DEFAULTLAN{
	char staticIP[32];
	char staticNetmask[32];
}ST_NSDK_NETWORK_DEFAULTLAN, *LP_NSDK_NETWORK_DEFAULTLAN;

typedef struct NSDK_NETWORK_PPPOE{
	bool enabled;
	char pppoeUserName[64+1];
	char pppoePassword[64+1];
}ST_NSDK_NETWORK_PPPOE, *LP_NSDK_NETWORK_PPPOE;

typedef struct NSDK_NETWORK_DDNS{
	bool enabled;
#define kNSDK_NETWORK_DDNS_PROVIDER_DYNDDNS (1<<0)
#define kNSDK_NETWORK_DDNS_PROVIDER_NOIP (1<<1)
#define kNSDK_NETWORK_DDNS_PROVIDER_3322 (1<<2)
#define kNSDK_NETWORK_DDNS_PROVIDER_CHANGEIP (1<<3)
#define kNSDK_NETWORK_DDNS_PROVIDER_POPDVR (1<<4)
#define kNSDK_NETWORK_DDNS_PROVIDER_SKYBEST (1<<5)
#define kNSDK_NETWORK_DDNS_PROVIDER_DVRTOP (1<<6)
	int ddnsProvider;
	char ddnsUrl[64+1];
	char ddnsUserName[64+1];
	char ddnsPassword[64+1];
}ST_NSDK_NETWORK_DDNS, *LP_NSDK_NETWORK_DDNS;

typedef struct NSDK_NETWORK_WIRELESS_STAMODE{
	char wirelessStaMode[32];
	char wirelessApBssId[48+1];
	char wirelessApEssId[64];
	char wirelessApPsk[256];
	bool wirelessFixedBpsModeEnabled;
}ST_NSDK_NETWORK_WIRELESS_STAMODE, *LP_NSDK_NETWORK_WIRElESS_STAMODE;

typedef struct NSDK_NETWORK_WIRELESS_APMODE{
	char wirelessBssId[48+1];
	char wirelessEssId[64];
	char wirelessPsk[128];
#define NSDK_NETWORK_WIRELESS_APMODE_80211B (1<<0)
#define NSDK_NETWORK_WIRELESS_APMODE_80211G (1<<1)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N (1<<2)
#define NSDK_NETWORK_WIRELESS_APMODE_80211BG (1<<3)
#define NSDK_NETWORK_WIRELESS_APMODE_80211BGN (1<<4)
	int wireLessApMode;
	bool wireLessEssIdBroadcastingEnabled;
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_AUTO 	(0)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_1		(1)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_2		(2)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_3		(3)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_4		(4)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_5		(5)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_6		(6)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_7		(7)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_8		(8)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_9		(9)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_10		(10)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_11		(11)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_12		(12)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_13		(13)
#define NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_14		(14)
	int wirelessApMode80211nChannel;
#define NSDK_NETWORK_WIRELESS_WPAMODE_WPA_PSK (1<<0)
#define NSDK_NETWORK_WIRELESS_WPAMODE_WPA2_PSK (1<<1)
	int wirelessWpaMode;
}ST_NSDK_NETWORK_WIRELESS_APMODE, *LPNSDK_NETWORK_WIRELESS_APMODE;

typedef struct NSDK_NETWORK_WIRELESS_REPEATER{
#define NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_AUTO (0)
#define NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_10M (1)
#define NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_100M (2)
#define NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_1000M (3)
        int wiredBandWidth;
        int wiredMaxConn;
        int wirelessMaxConn;
#define NSDK_NETWORK_REPEAER_MODE_AUTO (0)
#define NSDK_NETWORK_REPEAER_MODE_WIRED (1)
#define NSDK_NETWORK_REPEAER_MODE_WIRELESS (2)
        int repeaterMode;
#define NSDK_NETWORK_REPEAER_WORK_MODE_AUTO (0)
#define NSDK_NETWORK_REPEAER_WORK_MODE_BRIDGE (1)
#define NSDK_NETWORK_REPEAER_WORK_MODE_BOND (2)
        int repeaterWorkMode;
}ST_NSDK_NETWORK_WIRELESS_REPEATER, *LPNSDK_NETWORK_WIRELESS_REPEATER;

typedef struct NSDK_NETWORK_WIRELESS_DHCP_SERVER{
	bool enabled;
	bool dhcpAutoSettingEnabled;
	char dhcpIpRange[64+1];
	char dhcpIpNumber[32+1];
	char dhcpIpDns[32+1];
	char dhcpIpGateway[32+1];
}ST_NSDK_NETWORK_WIRELESS_DHCP_SERVER, *LP_NSDK_NETWORK_WIRELESS_DHCP_SERVER;

typedef struct NSDK_NETWORK_WIRELESS{
#define NSDK_NETWORK_WIRELESS_MODE_NONE (1<<0)
#define NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT (1<<1)
#define NSDK_NETWORK_WIRELESS_MODE_STATIONMODE (1<<2)
#define NSDK_NETWORK_WIRELESS_MODE_REPEATER             (1<<3)
	int wirelessMode;
    char repeaterDevId[64];
	ST_NSDK_NETWORK_WIRELESS_STAMODE wirelessStaMode;
    ST_NSDK_NETWORK_WIRELESS_STAMODE wirelessStaModeBackup;
	ST_NSDK_NETWORK_WIRELESS_APMODE wirelessApMode;
	ST_NSDK_NETWORK_WIRELESS_DHCP_SERVER dhcpServer;
    ST_NSDK_NETWORK_WIRELESS_REPEATER repeater;
}ST_NSDK_NETWORK_WIRELESS, *LP_NSDK_NETWORK_WIRELESS;

typedef struct NSDK_NETWORK_ESEE{
	bool enabled;
}ST_NSDK_NETWORK_ESEE, *LP_NETSDK_NETWORK_ESEE;

typedef struct NSDK_NETWORK_DNS{
	char staticPreferredDns[64];
	char staticAlternateDns[64];
}ST_NSDK_NETWORK_DNS, *LP_NSDK_NETWORK_DNS;

typedef struct NSDK_NETWORK_UPNP{
	bool enabled;
}ST_NSDK_NETWORK_UPNP, *LP_NSDK_NETWORK_UPNP;

typedef struct NSDK_NETWORK_INTERFACE{
	int id;
	char interfaceName[16];
	ST_NSDK_NETWORK_LAN lan;
	ST_NSDK_NETWORK_UPNP upnp;
	ST_NSDK_NETWORK_PPPOE pppoe;
	ST_NSDK_NETWORK_DDNS ddns;
	ST_NSDK_NETWORK_WIRELESS wireless;
	ST_NSDK_NETWORK_DNS dns;
	ST_NSDK_NETWORK_ESEE esee;
}ST_NSDK_NETWORK_INTERFACE, *LP_NSDK_NETWORK_INTERFACE;

typedef struct NSDK_NETWORK_OSSCLD {
	bool isBound;	// is device bound
	int chNum;	// total channel be allowed
	struct {
		struct {
			bool enable;
			int type; // record type. 0 motion; 1 all day
		} stream[3];
	} channel[4];
} ST_NSDK_NETWORK_OSSCLD, *LP_NSDK_NETWORK_OSSCLD;

typedef struct NSDK_SYSTEM_TIME {
	int greenwichMeanTime; // if +08:30 keep as 8 x 100 + 30
#define kNSDK_CALENDARSTYLE_GENERAL (0)
#define kNSDK_CALENDARSTYLE_JALAALI (1)
	int calendarStyle;
	bool ntpEnabled;
	char ntpServerDomain[64];
	char ntpServerBackupOne[64];
	char ntpServerBackupTwo[64];
}ST_NSDK_SYSTEM_TIME, *LP_NSDK_SYSTEM_TIME;

//Daylight Saving Time
typedef struct NSDK_SYSTEM_DAYLIGHT_SAVING_TIME_WEEK
{
	char type[8];
	int month;
	int week;
	int weekday;
	int hour;
	int minute;
}ST_NSDK_SYSTEM_DST_WEEK, *LP_NSDK_SYSTEM_DST_WEEK;

typedef struct NSDK_SYSTEM_DAYLIGHT_SAVING_TIME
{
	bool enable;
    char country[64];
	unsigned int offset;
	ST_NSDK_SYSTEM_DST_WEEK week[2];
}ST_NSDK_SYSTEM_DST, *LP_NSDK_SYSTEM_DST;

typedef struct NSDK_SYSTEM_DEVICE_INFO {
	char deviceName[64 + 1];
	int deviceAddress;
	char deviceDescription[256 + 1];
	char model[32 + 1];
	char soc[16+1];
	char serialNumber[64 + 1];
	char macAddress[32 + 1];
	char firmwareVersion[32 + 1];
	char firmwareReleaseDate[32 + 1];
	char hardwareVersion[32 + 1];
    char productCode[32 + 1];
}ST_NSDK_SYSTEM_DEVICE_INFO, *LP_NSDK_SYSTEM_DEVICE_INFO;

typedef struct NSDK_SYSTEM_PROMPT_SOUND{
	bool enabled;
#define kNSDK_SYSTEM_PROMPT_SOUND_TYPE_CHINESE (0)
#define kNSDK_SYSTEM_PROMPT_SOUND_TYPE_ENGLISH (1)
#define kNSDK_SYSTEM_PROMPT_SOUND_TYPE_GERMAN  (2)
#define kNSDK_SYSTEM_PROMPT_SOUND_TYPE_KOREAN  (3)
#define kNSDK_SYSTEM_PROMPT_SOUND_TYPE_PORTUGUESE (4)
#define kNSDK_SYSTEM_PROMPT_SOUND_TYPE_RUSSIAN  (5)
#define kNSDK_SYSTEM_PROMPT_SOUND_TYPE_SPANISH (6)
	int soundType;
	char soundTypeStr[16];
	char soundTypeOpt[128];
}ST_NSDK_SYSTEM_PROMPT_SOUND, *LP_NSDK_SYSTEM_PROMPT_SOUND;

typedef struct NSDK_SYSTEM_MD_ALARM {
    bool MotionWarningTone;
    char WarningToneType[32];
}ST_NSDK_SYSTEM_MD_ALARM, *LP_NSDK_SYSTEM_MD_ALARM;

typedef enum
{
    EN_NSDK_FISHEYE_LENS_TYPE_NONE  = 0,
    EN_NSDK_FISHEYE_LENS_TYPE_180   = 180,
    EN_NSDK_FISHEYE_LENS_TYPE_360   = 360,
    EN_NSDK_FISHEYE_LENS_TYPE_720   = 720
}enNSDK_FISHEYE_LENS_TYPE;

typedef enum
{
    EN_NSDK_LIGHT_CTL_TYPE_NONE = 0,    // 非特殊枪机灯控
    EN_NSDK_LIGHT_CTL_TYPE_DOUBLE,      // 双光源枪机灯控
    EN_NSDK_LIGHT_CTL_TYPE_WARMTH       // 暖光黑夜全彩枪机灯控
}enNSDK_LIGHT_CTL_TYPE;

typedef enum
{
    EN_NSDK_BULB_CTL_NONE = 0,  // 非灯泡设备
    EN_NSDK_BULB_CTL_SGL,       // 纯白光非调光灯泡设备
    EN_NSDK_BULB_CTL_SGL_PWM,   // 纯白光可调光灯泡设备
    EN_NSDK_BULB_CTL_DBL,       // 双光源白光非调光灯泡设备
    EN_NSDK_BULB_CTL_DBL_PWM    // 双光源白光可调光灯泡设备
}enNSDK_BULB_CTL;

typedef struct NSDK_SYSTEM_CAPABILITY_SET
{
    unsigned int version;
    unsigned int maxChannel;
    char model[64];
    bool powerBattery;
    bool audioInput;
    bool audioOutput;
    bool bluetooth;
    int lightControl;
    int bulbControl;
    bool ptz;
    bool sdCard;
    bool lte;
    bool wifi;
    bool rj45;
    bool rtc;
    int fisheye;
    bool wifiStationCanSet;
    bool pir;
}ST_NSDK_SYSTEM_CAPABILITY_SET, *LP_NSDK_SYSTEM_CAPABILITY_SET;

//pir Manager
typedef struct NSDK_SYSTEM_PIR_MANAGER
{
#define kNSDK_PIR_MANAGER_TRIGGER_FALLING_EDGE (0)
#define kNSDK_PIR_MANAGER_TRIGGER_RISING_EDGE (1)

    int pirTrigger;
}ST_NSDK_SYSTEM_PIR_MANAGER, *LP_NSDK_SYSTEM_PIR_MANAGER;

typedef struct NSDK_SYSTEM_SETTING{
	bool messagePushEnabled;
    bool timeRecordEnabled;
	bool MotionRecordEnabled;
	struct
	{
		//tBOOL enabled;
		struct
		{
			bool enabled;//ʹ��scheduleʱ���жϴ˱�����Ϊtrueʱ����ʹ��time/weekday

			struct
			{
				unsigned int hour, min, sec;
			} BeginTime, EndTime;

#define DUCAM_SETUP_WEEKDAY_SUN (1<<0)
#define DUCAM_SETUP_WEEKDAY_MON (1<<1)
#define DUCAM_SETUP_WEEKDAY_TUE (1<<2)
#define DUCAM_SETUP_WEEKDAY_WED (1<<3)
#define DUCAM_SETUP_WEEKDAY_THU (1<<4)
#define DUCAM_SETUP_WEEKDAY_FRI (1<<5)
#define DUCAM_SETUP_WEEKDAY_SAT (1<<6)

			unsigned int weekday;

		} Schedule[16];

	} AlarmNotification,TFcard_Record;

	ST_NSDK_SYSTEM_PROMPT_SOUND promptSound;
    char area[16];
    ST_NSDK_SYSTEM_MD_ALARM mdAlarm;
    ST_NSDK_SYSTEM_CAPABILITY_SET capabilitySet;
    ST_NSDK_SYSTEM_PIR_MANAGER pirManager;
}ST_NSDK_SYSTEM_SETTING,*LP_NSDK_SYSTEM_SETTING;

//Record Manager
typedef struct NSDK_SYSTEM_RECORD_MANAGER
{
	bool useIOAlarm;
	char recMode[16];
}ST_NSDK_SYSTEM_REC_MANAGER, *LP_NSDK_SYSTEM_REC_MANAGER;

typedef struct NSDK_IMAGE_IRCUTFILTER
{
#define kNSDK_IMAGE_IRCUT_CONTROL_MODE_HARDWARE (0)
#define kNSDK_IMAGE_IRCUT_CONTROL_MODE_SOFTWARE (1)
	
#define kNSDK_IMAGE_IRCUT_MODE_AUTO (0)
#define kNSDK_IMAGE_IRCUT_MODE_DAYLIGHT (1)
#define kNSDK_IMAGE_IRCUT_MODE_NIGHT (2)
#define kNSDK_IMAGE_IRCUT_MODE_LIGHTMODE (3)
#define kNSDK_IMAGE_IRCUT_MODE_SMARTMODE (4)

	 int irCutControlMode;
	 int irCutMode;
}ST_NSDK_IMAGE_IRCUTFILTER,*LP_NSDK_IMAGE_IRCUTFILTER;

typedef struct NSDK_IMAGE_MANUAL_SHARPNESS
{
	bool enabled;
	int sharpnessLevel;
}ST_NSDK_IMAGE_MANUAL_SHARPNESS, *LP_NSDK_IMAGE_MANUAL_SHARPNESS;

typedef struct NSDK_IMAGE_DENOISE3D
{
	bool enabled;
	int denoise3dStrength;
}ST_NSDK_IMAGE_DENOISE3D,*LP_NSDK_IMAGE_DENOISE3D;

typedef struct NSDK_IMAGE_WDR
{
	bool enabled;
	int WDRStrength;
}ST_NSDK_IMAGE_WDR,*LP_NSDK_IMAGE_WDR;

typedef enum NSDK_IMAGE_FISHEYE_FIX_MODE{
	eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL=0,
	eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL,
	eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE,
	eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE,
	eNSDK_IMAGE_FISHEYE_FIX_MODE_P720,
	eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE2,
}EM_NSDK_IMAGE_FISHEYE_FIX_MODE;

typedef enum NSDK_IMAGE_FISHEYE_SHOW_MODE{
	eNSDK_IMAGE_FISHEYE_MODE_WALL_ORIGIN = 0x0,
	eNSDK_IMAGE_FISHEYE_MODE_WALL_180,
	eNSDK_IMAGE_FISHEYE_MODE_WALL_SPLIT,
	eNSDK_IMAGE_FISHEYE_MODE_WALL_WALL_SPLIT,
	eNSDK_IMAGE_FISHEYE_MODE_WALL_4R,
	eNSDK_IMAGE_FISHEYE_MODE_WALL_KITR,
	eNSDK_IMAGE_FISHEYE_MODE_WALL_KITO,

	eNSDK_IMAGE_FISHEYE_MODE_CELL_ORIGIN = 0x10,
	eNSDK_IMAGE_FISHEYE_MODE_CELL_360,
	eNSDK_IMAGE_FISHEYE_MODE_CELL_SPLIT,
	eNSDK_IMAGE_FISHEYE_MODE_CELL_4R,
	eNSDK_IMAGE_FISHEYE_MODE_CELL_WALL_SPLIT,
	eNSDK_IMAGE_FISHEYE_MODE_CELL_180,
	eNSDK_IMAGE_FISHEYE_MODE_CELL_KITR,
	eNSDK_IMAGE_FISHEYE_MODE_CELL_KITO,

	eNSDK_IMAGE_FISHEYE_MODE_TABLE_ORIGIN= 0x20,
	eNSDK_IMAGE_FISHEYE_MODE_TABLE_360,
	eNSDK_IMAGE_FISHEYE_MODE_TABLE_SPLIT,
	eNSDK_IMAGE_FISHEYE_MODE_TABLE_4R,
	eNSDK_IMAGE_FISHEYE_MODE_TABLE_VR,
	eNSDK_IMAGE_FISHEYE_MODE_TABLE_KITR,
	eNSDK_IMAGE_FISHEYE_MODE_TABLE_KITO,

	eNSDK_IMAGE_FISHEYE_MODE_NONE = 0x30,
}EM_NSDK_IMAGE_FISHEYE_SHOW_MODE;

typedef struct NSDK_VIDEO_MODE{
	EM_NSDK_IMAGE_FISHEYE_FIX_MODE fixType;
	EM_NSDK_IMAGE_FISHEYE_SHOW_MODE showMode;
}ST_NSDK_VIDEO_MODE, *LP_NSDK_VIDEO_MODE;

typedef struct NSDK_IMAGE
{
#define kNSDK_IMAGE_SCENE_MODE_AUTO (0)
#define kNSDK_IMAGE_SCENE_MODE_INDOOR (1)
#define kNSDK_IMAGE_SCENE_MODE_OUTDOOR (2)

#define kNSDK_IMAGE_EXPOSURE_MODE_AUTO (0)
#define kNSDK_IMAGE_EXPOSURE_MODE_BRIGHT (1)
#define kNSDK_IMAGE_EXPOSURE_MODE_DARK (2)

#define kNSDK_IMAGE_AWB_MODE_AUTO (0)
#define kNSDK_IMAGE_AWB_MODE_BRIGHT (1)
#define kNSDK_IMAGE_AWB_MODE_DARK (2)

#define kNSDK_IMAGE_LOWLIGHT_MODE_CLOSE (0)
#define kNSDK_IMAGE_LOWLIGHT_MODE_ALLDAY (1)
#define kNSDK_IMAGE_LOWLIGHT_MODE_NIGHT (2)
#define kNSDK_IMAGE_LOWLIGHT_MODE_AUTO	(3)

#define kNSDK_IMAGE_lghtNhbtd_MODE_AUTO (0)
#define kNSDK_IMAGE_lghtNhbtd_MODE_ON (1)
#define kNSDK_IMAGE_lghtNhbtd_MODE_OFF (2)

#define kNSDK_IMAGE_BLcompensation_MODE_AUTO (0)
#define kNSDK_IMAGE_BLcompensation_MODE_ON (1)
#define kNSDK_IMAGE_BLcompensation_MODE_OFF (2)

	int sceneMode;
	int exposureMode;
	int awbMode;
	int lowlightMode;
	int lghtNhbtdMode;
	int BLcompensationMode;
	ST_NSDK_IMAGE_IRCUTFILTER irCutFilter;
	ST_NSDK_IMAGE_MANUAL_SHARPNESS manualSharpness;
	ST_NSDK_IMAGE_DENOISE3D denoise3d;
	ST_NSDK_IMAGE_WDR wdr;
	int imageStyle;
	ST_NSDK_VIDEO_MODE videoMode;
}ST_NSDK_IMAGE,*LP_NSDK_IMAGE;

typedef enum NSDK_SYSTEM_OPERATION
{
	eNSDK_SYSTEM_OPERATION_REBOOT = 0,
	eNSDK_SYSTEM_OPERATION_DEFAULT,
	eNSDK_SYSTEM_OPERATION_REMOTE_UPGRADE,
}EM_NSDK_SYSTEM_OPERATION;

typedef enum NSDK_NETWORK_WIFI_STATUS_TYPE
{
	eNSDK_NETWORK_WIFI_STATUS_TYPE_NONE = -1,
	eNSDK_NETWORK_WIFI_STATUS_TYPE_MODEL_EXIST = 0,
	eNSDK_NETWORK_WIFI_STATUS_TYPE_STATION_SIGNAL,
}EM_NSDK_NETWORK_WIFI_STATUS_TYPE;

typedef struct NSDK_NETWORK_WIFI_STATUS_ATTR
{
	EM_NSDK_NETWORK_WIFI_STATUS_TYPE statusType;
	union
	{
		bool modelExist;
		int stationSignal;
	};
}ST_NSDK_NETWORK_WIFI_STATUS_ATTR, *LP_NSDK_NETWORK_WIFI_STATUS_ATTR;


typedef enum NSDK_PRODUCT_TYPE
{
    eNSDK_PRODUCT_TYPE_PX = 0,
    eNSDK_PRODUCT_TYPE_CX,
    eNSDK_PRODUCT_TYPE_NONE,
};

typedef struct NSDK_PTZ_EXTERNAL_CFG {
	char strProtocolName[32 + 1];
	int	 nAddress;
	int	 nSpeed;
	int  nBaudRate;
	int  nDateBit;
	int  nStopBit;
	char strParity;
	char strptzCustomTpye[16 + 1];
}ST_NSDK_PTZ_EXTERNAL_CFG, *LP_NSDK_PTZ_EXTERNAL_CFG;

typedef struct NSDK_PTZ__CFG{
	int nSerialPortID;
	int nVideoInputID;
	char strDuplexMode[32 + 1]; //half  full
	char strControlType[32 + 1]; //internal external
	ST_NSDK_PTZ_EXTERNAL_CFG stPtzExternalConfig;
}ST_NSDK_PTZ_CFG, *LP_NSDK_PTZ_CFG;

typedef int (*fNSDK_SYS_DEV_INFO)(LP_NSDK_SYSTEM_DEVICE_INFO dev_info);
typedef int (*fNSDK_VIN_CH_CHG)(int vin, LP_NSDK_VIN_CH vin_ch);
typedef int (*fNSDK_AIN_CH_CHG)(int ain, LP_NSDK_AIN_CH ain_ch);
typedef int (*fNSDK_MD_CH_CHG)(int vin, LP_NSDK_MD_CH md_ch);
typedef bool (*fNSDK_MD_CH_STATUS)(int vin,bool b_get); // get / delete md status
typedef int (*fNSDK_VENC_CH_CHG)(int id, LP_NSDK_VENC_CH venc_ch);
typedef int (*fNSDK_AENC_CH_CHG)(int id, LP_NSDK_AENC_CH aenc_ch);
typedef int (*fNSDK_VENC_REQUEST_KEYFRAME)(int id);
typedef int (*fNSDK_VENC_SNAPSHOT)(int id, int type, int width, int height, char *file_path);
typedef int (*fNSDK_SYSTEM_CHG)(LP_NSDK_SYSTEM_TIME sys_time);
typedef int (*fNSDK_SYSTEM_DST_CHG)(LP_NSDK_SYSTEM_DST dst_time);
typedef bool (*fNSDK_WIFI_STATUS)(EM_NSDK_NETWORK_WIFI_STATUS_TYPE type, LP_NSDK_NETWORK_WIFI_STATUS_ATTR status_attr);
typedef int (*fNSDK_IMAGE_CHG)(LP_NSDK_IMAGE image);
typedef bool (*fNSDK_IMAGE_AF_STATUS)(int sock, bool b_get);
typedef int (*fNSDK_SYS_OPERATION)(EM_NSDK_SYSTEM_OPERATION operation);
typedef int (*fNSDK_SYS_REMOTE_UPGRADE_STATUS)(char *ret_status, int *rate);
typedef int (*fNSDK_SYS_REMOTE_UPGRADE_ERROR)(char *ret_error);



typedef int (*fNSDK_ALARM_IN_CH_PORT_STATUS)(int id, LP_NSDK_ALARM_IN_CH alarmIn, int *status);
typedef int (*fNSDK_ALARM_OUT_CH_PORT_STATUS)(int id, LP_NSDK_ALARM_OUT_CH alarmOUT, int *status);
typedef int (*fNSDK_ALARM_IN_CH_CHG)(int id, LP_NSDK_ALARM_IN_CH alarmIn);
typedef int (*fNSDK_ALARM_OUT_CH_CHG)(int id, LP_NSDK_ALARM_OUT_CH alarmOut);
typedef int (*fNSDK_ALARM_OUT_CH_TRIGGER)(int id, LP_NSDK_ALARM_OUT_CH_TRIGGER_METHOD triggerMethod);
typedef int (*fNSDK_PTZ_UART_CONFIG_CHANGED)(int nBaudRate, int nDateBit, char *strParity, int nStopBit);

typedef enum NSDK_CONF_SAVE_OPERATION
{
	eNSDK_CONF_SAVE_RESTART = 0,
	eNSDK_CONF_SAVE_REBOOT,
	eNSDK_CONF_SAVE_JUST_SAVE,
	eNSDK_CONF_SAVE_RESTART_WIRELESS,
}EM_NSDK_CONF_SAVE_OPERATION;
typedef int (*fNSDK_CONF_SAVE)(EM_NSDK_CONF_SAVE_OPERATION opteration, int delay);

typedef enum NSDK_ImageTransmissionModel
{
	eNSDK_LOW_BPS_MODEL = 0,
	eNSDK_COMPATIBILITY_MODE,
}EM_NSDK_ImageTransmissionModel;

#define sNSDK_MULTI_CONF_MODE_FILE_PATH "/media/conf/multi_conf_mode"
#define sNSDK_MULTI_CONF_MODE_INSIDE "inside"
#define sNSDK_MULTI_CONF_MODE_OUTSIDE "outside"
#define sNSDK_MULTI_CONF_MODE_CUSTOM "custom"
#define sNSDK_MULTI_CONF_MODE_DEFAULT sNSDK_MULTI_CONF_MODE_OUTSIDE
#define kNSDK_MULTI_CONF_MODE_INSIDE (0)
#define kNSDK_MULTI_CONF_MODE_OUTSIDE (1)
#define kNSDK_MULTI_CONF_MODE_CUSTOM (2)
#define kNSDK_MULTI_CONF_MODE_DEFAULT kNSDK_MULTI_CONF_MODE_OUTSIDE

#define NSDK_NETWORK_WIRED_ETH          			            "eth0"
#define NSDK_NETWORK_WIRELESS_MODE_STA_ETH			"wlan0"
#define NSDK_NETWORK_WIRELESS_MODE_AP_ETH			"wlan0"
#define NSDK_NETWORK_WIRELESS_MODE_REPEATER_STA_ETH	"wlan0"
#define NSDK_NETWORK_WIRELESS_MODE_REPEATER_AP_ETH	"wlan1"
#define NSDK_NETWORK_WIRELESS_MODE_REPEATER_ETH		"br0"


#ifdef __cplusplus
};
#endif
#endif //NETSDKV10_DEF_H_

