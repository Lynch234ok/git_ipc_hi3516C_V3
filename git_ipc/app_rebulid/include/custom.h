#ifndef _CUSTOM_H
#define _CUSTOM_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _custom_setting_function
{
	int ipAdapted;
	int fixMode;    // 产测时设置的安装方式是跟圆心矫正参数一起传给设备，所以在fisheye.json中保存，不保存在这里
	int imageStyle;
	int promptSoundType;
	int audioInputGain;
	int audioInputVolume;
	int audioOutputGain;
	int irCutControlMode;
	int irCutControlThresh[8];
	char p2pServerIp[32];
    int audioHwSpec;
	int ledPwmEnabled;
	int ledPwmChannelCount;
    char vendor[16];
    int powerLineFrequencyMode;
    int pirEnabled;
    int pirTrigger;
    int flipEnabled;
    int mirrorEnabled;
    int customAlarmSound;

    /**
     * 鍏宠仈capability set
     */
    char model[64];
    int audioInput;
    int audioOutput;
    int lightControl;
    int bulbControl;
    int ptz;
    int sdCard;
    int fisheye;
}ST_CUSTOM_FUNCTION, *LP_CUSTOM_FUNCTION;

typedef struct _custom_setting_protocol
{
	int hikvision;
}ST_CUSTOM_PROTOCOL, *LP_CUSTOM_PROTOCOL;

typedef struct _custom_setting_module
{
	int reverse;
}ST_CUSTOM_MODULE,*LP_CUSTOM_MODULE;

typedef struct _custom_setting_model
{
	char oemNumber[16];
    int productType;
}ST_CUSTOM_MODEL, *LP_CUSTOM_MODEL;

typedef struct PTZ_ATTR{//浜戝彴閰嶇疆
    int  nAddress;
    int  nBaudRate;
    int  nDateBit;
    int  nStopBit;    
	char strPtzCustomTpye[16 + 1];
}st_PTZ_ATTR, *pst_PTZ_ATTR;

typedef struct MOTOR_ATTR{
    int motorEnabled;
    int motorType;
}ST_MOTOR_ATTR, *LP_MOTOR_ATTR;

typedef struct _custom_setting
{
	ST_CUSTOM_FUNCTION function;
	ST_CUSTOM_PROTOCOL protocol;
	ST_CUSTOM_MODULE module;
	ST_CUSTOM_MODEL model;
    st_PTZ_ATTR st_ptzAttr;
    ST_MOTOR_ATTR motor;
}ST_CUSTOM_SETTING, *LP_CUSTOM_SETTING;

extern int CUSTOM_set(LP_CUSTOM_SETTING attr);
extern int CUSTOM_get(LP_CUSTOM_SETTING attr);
extern int CUSTOM_set_json_string(char * json_str);
extern int CUSTOM_get_json_string(char * json_str);
extern bool CUSTOM_check_int_valid(int input);
extern bool CUSTOM_check_string_valid(char *input);
extern bool CUSTOM_check_array_valid(int *input, int length);
extern int CUSTOM_init(void);
extern void CUSTOM_destory(void);


#ifdef __cplusplus
};
#endif
#endif /*_CUSTOM_H*/