#include "app_debug.h"
#include "netsdk_json.h"
#include "netsdk_private.h"
#include "custom.h"
#include "generic.h"

static const ST_NSDK_MAP_STR_DEC fixMode_map[] = {
	{"wall", eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL},
	{"cell", eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL},
	{"table", eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE},
	{"none", eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE},
};	

static const ST_NSDK_MAP_STR_DEC promptSoundType_map[] = {
	{"chinese", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_CHINESE},
	{"english", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_ENGLISH},
	{"german", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_GERMAN},
	{"korean", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_KOREAN},
	{"portuguese", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_PORTUGUESE},
	{"russian", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_RUSSIAN},
	{"spanish", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_SPANISH}
};

static const ST_NSDK_MAP_STR_DEC irCutControlMode_map[] = {
	{"hardware", kNSDK_IMAGE_IRCUT_CONTROL_MODE_HARDWARE},
	{"software", kNSDK_IMAGE_IRCUT_CONTROL_MODE_SOFTWARE},
};

static const ST_NSDK_MAP_STR_DEC productType_map[] = {
	{"PX", eNSDK_PRODUCT_TYPE_PX},
	{"CX", eNSDK_PRODUCT_TYPE_CX},
};

static const ST_NSDK_MAP_STR_DEC pirTrigger_map[] = {
	{"fallingEdge", kNSDK_PIR_MANAGER_TRIGGER_FALLING_EDGE},
	{"risingEdge", kNSDK_PIR_MANAGER_TRIGGER_RISING_EDGE},
};

#define CUSTOM_SETTING_FILE_PATH "/media/conf/custom.conf"
#define CUSTOM_SETTING_DEFAULT_FILE_PATH "/media/custom/custom.conf"
//#define CUSTOM_SETTING_DEFAULT_FILE_PATH "/root/nfs/git_ipc/server_ipc/git_ipc/app_rebulid/bin/custom.conf"

typedef struct _custom
{
	ST_CUSTOM_SETTING custom;
	pthread_mutex_t mutex;
}ST_CUSTOM,*LP_CUSTOM;

static LP_CUSTOM custom_attr = NULL;

static void custom_setting_dump(LP_CUSTOM_SETTING custom_conf)
{
	//APP_TRACE("function.audioInputGain:%d", custom_conf->function.audioInputGain);
	//APP_TRACE("function.audioInputVolume:%d", custom_conf->function.audioInputVolume);
	//APP_TRACE("function.audioOutputGain:%d", custom_conf->function.audioOutputGain);
	APP_TRACE("function.fixMode:%d", custom_conf->function.fixMode);
	//APP_TRACE("function.ipAdapted:%d", custom_conf->function.ipAdapted);
	APP_TRACE("function.promptSoundType:%d", custom_conf->function.promptSoundType);
	APP_TRACE("function.irCutControlMode:%d", custom_conf->function.irCutControlMode);
	//APP_TRACE("function.p2pServerIp:%s", custom_conf->function.p2pServerIp);
	/*APP_TRACE("function.irCutControlThresh:%d,%d,%d,%d,%d,%d,%d,%d", 
		custom_conf->function.irCutControlThresh[0],
		custom_conf->function.irCutControlThresh[1],
		custom_conf->function.irCutControlThresh[2],
		custom_conf->function.irCutControlThresh[3],
		custom_conf->function.irCutControlThresh[4],
		custom_conf->function.irCutControlThresh[5],
		custom_conf->function.irCutControlThresh[6],
		custom_conf->function.irCutControlThresh[7]);*/
	//APP_TRACE("function.imageStyle:%d", custom_conf->function.imageStyle);
    //APP_TRACE("function.audioHwSpec:%d", custom_conf->function.audioHwSpec);
	//APP_TRACE("function.ledPwmEnabled:%d", custom_conf->function.ledPwmEnabled);
	//APP_TRACE("function.ledPwmChannelCount:%d", custom_conf->function.ledPwmChannelCount);
    //APP_TRACE("function.vendor:%s", custom_conf->function.vendor);
    //APP_TRACE("function.powerLineFrequencyMode:%d", custom_conf->function.powerLineFrequencyMode);
    APP_TRACE("function.model:%s", custom_conf->function.model);
    //APP_TRACE("function.audioInput:%d", custom_conf->function.audioInput);
    //APP_TRACE("function.audioOutput:%d", custom_conf->function.audioOutput);
    //APP_TRACE("function.lightControl:%d", custom_conf->function.lightControl);
    //APP_TRACE("function.bulbControl:%d", custom_conf->function.bulbControl);
    //APP_TRACE("function.ptz:%d", custom_conf->function.ptz);
    //APP_TRACE("function.sdCard:%d", custom_conf->function.sdCard);
    //APP_TRACE("function.fisheye:%d", custom_conf->function.fisheye);
    //APP_TRACE("function.pirEnabled:%d", custom_conf->function.pirEnabled);
    //APP_TRACE("function.pirTrigger:%d", custom_conf->function.pirTrigger);
    //APP_TRACE("function.flipEnabled:%d", custom_conf->function.flipEnabled);
    //APP_TRACE("function.mirrorEnabled:%d", custom_conf->function.mirrorEnabled);
    //APP_TRACE("function.customAlarmSound:%d", custom_conf->function.customAlarmSound);
	//APP_TRACE("protocol.hikvision:%s", (custom_conf->protocol.hikvision == 1)?"True":"False");
	//APP_TRACE("model.oemNumber:%s", custom_conf->model.oemNumber);
    //APP_TRACE("model.productType:%d", custom_conf->model.productType);

    APP_TRACE("st_ptzAttr.nAddress:%d", custom_conf->st_ptzAttr.nAddress);
    APP_TRACE("st_ptzAttr.nBaudRate:%d", custom_conf->st_ptzAttr.nBaudRate);
    APP_TRACE("st_ptzAttr.nDateBit:%d", custom_conf->st_ptzAttr.nDateBit);
    APP_TRACE("st_ptzAttr.nStopBit:%d", custom_conf->st_ptzAttr.nStopBit);
    APP_TRACE("st_ptzAttr.strPtzCustomTpye:%s", custom_conf->st_ptzAttr.strPtzCustomTpye);
    APP_TRACE("motor.motorEnabled:%d", custom_conf->motor.motorEnabled);
}

static int custom_setting_save(LP_JSON_OBJECT json)
{
	json_object_to_file(CUSTOM_SETTING_FILE_PATH, json);
	return 0;
}

static void custom_setting_parse_thresh_array8(char *text, int *array)
{
	if(text && array){
		int _array[8];
		memset(_array, 0, sizeof(_array));
		if(8 == sscanf(text, "%d,%d,%d,%d,%d,%d,%d,%d",
			&_array[0], &_array[1], &_array[2], &_array[3], &_array[4], &_array[5], &_array[6], &_array[7])){
			memcpy(array, _array, sizeof(_array));
		}else{
			APP_TRACE("Wrong array format");
		}
	}
}

static void custom_setting_array2str(char *text, int *array)
{
	if(array){
		int _array[8];
		memcpy(_array, array, sizeof(_array));
		sprintf(text, "%d,%d,%d,%d,%d,%d,%d,%d",
			_array[0], _array[1], _array[2], _array[3], _array[4], _array[5], _array[6], _array[7]);
	}
}

static void custom_setting_init_struct(LP_CUSTOM_SETTING custom_conf)
{
	if(custom_conf){
		//function
		custom_conf->function.audioInputGain = -1;
		custom_conf->function.audioInputVolume = -1;
		custom_conf->function.audioOutputGain = -1;
		custom_conf->function.fixMode = -1;
		custom_conf->function.imageStyle = -1;
		custom_conf->function.ipAdapted = -1;
		custom_conf->function.promptSoundType = -1;
		custom_conf->function.irCutControlMode = -1;
		memset(custom_conf->function.irCutControlThresh, 0, sizeof(custom_conf->function.irCutControlThresh));
		memset(custom_conf->function.p2pServerIp, 0, sizeof(custom_conf->function.p2pServerIp));
        custom_conf->function.audioHwSpec = -1;
		custom_conf->function.ledPwmEnabled = -1;
		custom_conf->function.ledPwmChannelCount = -1;
        memset(custom_conf->function.vendor, 0, sizeof(custom_conf->function.vendor));
        custom_conf->function.powerLineFrequencyMode = -1;
        memset(custom_conf->function.model, 0, sizeof(custom_conf->function.model));
        custom_conf->function.audioInput = -1;
        custom_conf->function.audioOutput = -1;
        custom_conf->function.lightControl = -1;
        custom_conf->function.bulbControl = -1;
        custom_conf->function.ptz = -1;
        custom_conf->function.sdCard = -1;
        custom_conf->function.fisheye = -1;
        custom_conf->function.pirEnabled = -1;
        custom_conf->function.pirTrigger = -1;
        custom_conf->function.flipEnabled = -1;
        custom_conf->function.mirrorEnabled = -1;
        custom_conf->function.customAlarmSound = -1;

		//module

		//protocol
		custom_conf->protocol.hikvision = -1;

        //ptz
        custom_conf->st_ptzAttr.nAddress = -1;
        custom_conf->st_ptzAttr.nBaudRate = -1;
        custom_conf->st_ptzAttr.nDateBit = -1;
        custom_conf->st_ptzAttr.nStopBit = -1;
		memset(custom_conf->st_ptzAttr.strPtzCustomTpye, 0, sizeof(custom_conf->st_ptzAttr.strPtzCustomTpye));

        //motor
        custom_conf->motor.motorEnabled = -1;
        custom_conf->motor.motorType = -1;

		//model
		memset(custom_conf->model.oemNumber, 0, sizeof(custom_conf->model.oemNumber));
        custom_conf->model.productType = -1;
	}
}

static int custom_conf_json_parse(LP_JSON_OBJECT json, LP_CUSTOM_SETTING custom_conf)
{
	char text[64];
	LP_JSON_OBJECT child_json = NULL;
	
	if(json && custom_conf){
		//Function
		child_json = NETSDK_json_get_child(json, "Function");
		if(child_json){
			if(NETSDK_json_check_child(child_json, "audioInputGain")){
				custom_conf->function.audioInputGain = NETSDK_json_get_int(child_json, "audioInputGain");
			}
			if(NETSDK_json_check_child(child_json, "audioInputVolume")){
				custom_conf->function.audioInputVolume = NETSDK_json_get_int(child_json, "audioInputVolume");
			}
			if(NETSDK_json_check_child(child_json, "audioOutputGain")){
				custom_conf->function.audioOutputGain= NETSDK_json_get_int(child_json, "audioOutputGain");
			}
			if(NETSDK_json_check_child(child_json, "ipAdapted")){
				custom_conf->function.ipAdapted = (int)NETSDK_json_get_boolean(child_json, "ipAdapted");
			}
			if(NETSDK_json_check_child(child_json, "fixMode")){
				if(NULL != NETSDK_json_get_string(child_json, "fixMode", text, sizeof(text))){
					custom_conf->function.fixMode = NETSDK_MAP_STR2DEC(fixMode_map,text,eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL);
				}
			}
			if(NETSDK_json_check_child(child_json, "promptSoundType")){
				if(NULL != NETSDK_json_get_string(child_json, "promptSoundType", text, sizeof(text))){
					custom_conf->function.promptSoundType = NETSDK_MAP_STR2DEC(promptSoundType_map,text,kNSDK_SYSTEM_PROMPT_SOUND_TYPE_CHINESE);
				}
			}
			if(NETSDK_json_check_child(child_json, "p2pServerIp")){
				NETSDK_json_get_string(child_json, "p2pServerIp", custom_conf->function.p2pServerIp, sizeof(custom_conf->function.p2pServerIp));
				//FIX ME:  防止OEM配置文件人为出错，写入局域网IP地址
				if(NULL != strstr(custom_conf->function.p2pServerIp, "192.168")){
					//avoid wrong serverIP
					APP_TRACE("avoid wrong serverIP:%s", custom_conf->function.p2pServerIp);
					memset(custom_conf->function.p2pServerIp, 0, sizeof(custom_conf->function.p2pServerIp));
				}
			}
			if(NETSDK_json_check_child(child_json, "irCutControlMode")){
				if(NULL != NETSDK_json_get_string(child_json, "irCutControlMode", text, sizeof(text))){
					custom_conf->function.irCutControlMode = NETSDK_MAP_STR2DEC(irCutControlMode_map,text,kNSDK_IMAGE_IRCUT_CONTROL_MODE_HARDWARE);
				}
			}
			if(NETSDK_json_check_child(child_json, "irCutControlThresh")){
				if(NULL != NETSDK_json_get_string(child_json, "irCutControlThresh", text, sizeof(text))){
					custom_setting_parse_thresh_array8(text, custom_conf->function.irCutControlThresh);
				}
			}
			if(NETSDK_json_check_child(child_json, "imageStyle")){
				custom_conf->function.imageStyle = NETSDK_json_get_int(child_json, "imageStyle");
			}
            if(NETSDK_json_check_child(child_json, "audioHwSpec")){
				custom_conf->function.audioHwSpec = NETSDK_json_get_int(child_json, "audioHwSpec");
			}

			if(NETSDK_json_check_child(child_json, "ledPwmEnabled")){
				custom_conf->function.ledPwmEnabled = (int)NETSDK_json_get_boolean(child_json, "ledPwmEnabled");
			}
			if(NETSDK_json_check_child(child_json, "ledPwmChannelCount")){
				custom_conf->function.ledPwmChannelCount = NETSDK_json_get_int(child_json, "ledPwmChannelCount");
			}
            if(NETSDK_json_check_child(child_json, "vendor")){
                NETSDK_json_get_string(child_json, "vendor", custom_conf->function.vendor, sizeof(custom_conf->function.vendor));
            }
            if(NETSDK_json_check_child(child_json, "powerLineFrequencyMode")){
                custom_conf->function.powerLineFrequencyMode = NETSDK_json_get_int(child_json, "powerLineFrequencyMode");
            }
            if(NETSDK_json_check_child(child_json, "model"))
            {
                NETSDK_json_get_string(child_json, "model", custom_conf->function.model, sizeof(custom_conf->function.model));
            }
            if(NETSDK_json_check_child(child_json, "audioInput"))
            {
                custom_conf->function.audioInput = NETSDK_json_get_int(child_json, "audioInput");
            }
            if(NETSDK_json_check_child(child_json, "audioOutput"))
            {
                custom_conf->function.audioOutput = NETSDK_json_get_int(child_json, "audioOutput");
            }
            if(NETSDK_json_check_child(child_json, "lightControl"))
            {
                custom_conf->function.lightControl = NETSDK_json_get_int(child_json, "lightControl");
            }
            if(NETSDK_json_check_child(child_json, "bulbControl"))
            {
                custom_conf->function.bulbControl = NETSDK_json_get_int(child_json, "bulbControl");
            }
            if(NETSDK_json_check_child(child_json, "ptz"))
            {
                custom_conf->function.ptz = NETSDK_json_get_int(child_json, "ptz");
            }
            if(NETSDK_json_check_child(child_json, "sdCard"))
            {
                custom_conf->function.sdCard = NETSDK_json_get_int(child_json, "sdCard");
            }
            if(NETSDK_json_check_child(child_json, "fisheye"))
            {
                custom_conf->function.fisheye = NETSDK_json_get_int(child_json, "fisheye");
            }
            if(NETSDK_json_check_child(child_json, "pirEnabled")){
                custom_conf->function.pirEnabled = (int)NETSDK_json_get_boolean(child_json, "pirEnabled");
            }
            if(NETSDK_json_check_child(child_json, "pirTrigger")){
                if(NULL != NETSDK_json_get_string(child_json, "pirTrigger", text, sizeof(text))){
                    custom_conf->function.pirTrigger = NETSDK_MAP_STR2DEC(pirTrigger_map, text, kNSDK_PIR_MANAGER_TRIGGER_FALLING_EDGE);
                }
            }
            if(NETSDK_json_check_child(child_json, "flipEnabled")){
                custom_conf->function.flipEnabled = (int)NETSDK_json_get_boolean(child_json, "flipEnabled");
            }
            if(NETSDK_json_check_child(child_json, "mirrorEnabled")){
                custom_conf->function.mirrorEnabled = (int)NETSDK_json_get_boolean(child_json, "mirrorEnabled");
            }
            if(NETSDK_json_check_child(child_json, "customAlarmSound")){
                custom_conf->function.customAlarmSound = (int)NETSDK_json_get_boolean(child_json, "customAlarmSound");
            }
		}

		//Module
		child_json = NETSDK_json_get_child(json, "Module");		
		if(child_json){
			
		}

		//Protocol
		child_json = NETSDK_json_get_child(json, "Protocol");
		if(child_json){

		}

        //ptz
		child_json = NETSDK_json_get_child(json, "externalControl");
		if(child_json){
            if(NETSDK_json_check_child(child_json, "address")){
				custom_conf->st_ptzAttr.nAddress = NETSDK_json_get_int(child_json, "address");
			}
            if(NETSDK_json_check_child(child_json, "baudrate")){
				custom_conf->st_ptzAttr.nBaudRate  = NETSDK_json_get_int(child_json, "baudrate");
			}
            if(NETSDK_json_check_child(child_json, "databit")){
				custom_conf->st_ptzAttr.nDateBit = NETSDK_json_get_int(child_json, "databit");
			}
            if(NETSDK_json_check_child(child_json, "stopbit")){
				custom_conf->st_ptzAttr.nStopBit = NETSDK_json_get_int(child_json, "stopbit");
			}

            if(NETSDK_json_check_child(child_json, "ptzCustomTpye")){				
				NETSDK_json_get_string(child_json, "ptzCustomTpye", custom_conf->st_ptzAttr.strPtzCustomTpye, sizeof(custom_conf->st_ptzAttr.strPtzCustomTpye));
			}

			
		}

        child_json = NETSDK_json_get_child(json, "motorControl");
        if(child_json){
            if(NETSDK_json_get_child(child_json, "motorEnabled")){
                custom_conf->motor.motorEnabled = NETSDK_json_get_boolean(child_json, "motorEnabled");
            }
            if(NETSDK_json_get_child(child_json, "motorType")){
                custom_conf->motor.motorType = NETSDK_json_get_boolean(child_json, "motorType");
            }
        }

		//Model
		child_json = NETSDK_json_get_child(json, "Model");
		if(child_json){
            if(NETSDK_json_check_child(child_json, "productType")){
                if(NULL != NETSDK_json_get_string(child_json, "productType", text, sizeof(text))){
                    custom_conf->model.productType = NETSDK_MAP_STR2DEC(productType_map,text,eNSDK_PRODUCT_TYPE_PX);
                }
            }
			if(NETSDK_json_check_child(child_json, "oemNumber")){
				NETSDK_json_get_string(child_json, "oemNumber", custom_conf->model.oemNumber, sizeof(custom_conf->model.oemNumber));
			}else{
				return -1;
			}
		}else{
			return -1;
		}
	}
	return 0;
}

static int custom_setting_file_to_string(char *str)
{
	LP_JSON_OBJECT json_conf =  NETSDK_json_load(CUSTOM_SETTING_FILE_PATH);
	if(NULL != json_conf){
		snprintf(str, 1024, "%s", json_object_to_json_string(json_conf));
		json_object_put(json_conf);
		return 0;
	}else{
	
	}
	return -1;
}

static int custom_setting_load(const char *fileName, LP_CUSTOM_SETTING custom_conf)
{
	int ret = -1;
	LP_JSON_OBJECT json_conf =  NETSDK_json_load(fileName);
    APP_TRACE("custom load file %s", fileName);
	custom_setting_init_struct(custom_conf);
	if(NULL != json_conf){
		ret = custom_conf_json_parse(json_conf, custom_conf);
		json_object_put(json_conf);
		if(-1 == ret){
			APP_TRACE("NO OEM number");
			return -1;
		}
	}else{
        APP_TRACE("NO OEM file in media conf");
		return -1;
	}
    APP_TRACE("custom load finish!!!");

	return 0;
}

static int custom_match_struct(LP_CUSTOM_SETTING attr)
{
	ST_CUSTOM_SETTING custom_conf;
	custom_setting_load(CUSTOM_SETTING_FILE_PATH, &custom_conf);
	//model
	if(CUSTOM_check_string_valid(custom_conf.model.oemNumber)){
		sprintf(attr->model.oemNumber, "%s", custom_conf.model.oemNumber);
	}
	if(CUSTOM_check_int_valid(custom_conf.model.productType)){
		attr->model.productType = custom_conf.model.productType;
	}

	//function
	if(CUSTOM_check_int_valid(custom_conf.function.audioInputGain)){
		attr->function.audioInputGain  = custom_conf.function.audioInputGain;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.audioInputVolume)){
		attr->function.audioInputVolume = custom_conf.function.audioInputVolume;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.audioOutputGain)){
		attr->function.audioOutputGain  = custom_conf.function.audioOutputGain;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.fixMode)){
		attr->function.fixMode = custom_conf.function.fixMode;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.imageStyle)){
		attr->function.imageStyle = custom_conf.function.imageStyle;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.promptSoundType)){
		attr->function.promptSoundType = custom_conf.function.promptSoundType;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.ipAdapted)){
		attr->function.ipAdapted = custom_conf.function.ipAdapted;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.irCutControlMode)){
		attr->function.irCutControlMode = custom_conf.function.irCutControlMode;
	}
	if(CUSTOM_check_string_valid(custom_conf.function.p2pServerIp)){
		sprintf(attr->function.p2pServerIp, "%s", custom_conf.function.p2pServerIp);
	}
	if(CUSTOM_check_int_valid(custom_conf.function.audioHwSpec)){
		attr->function.audioHwSpec  = custom_conf.function.audioHwSpec;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.ledPwmEnabled)){
		attr->function.ledPwmEnabled  = custom_conf.function.ledPwmEnabled;
	}
	if(CUSTOM_check_int_valid(custom_conf.function.ledPwmChannelCount)){
		attr->function.ledPwmChannelCount  = custom_conf.function.ledPwmChannelCount;
	}
    if(CUSTOM_check_string_valid(custom_conf.function.vendor)){
        sprintf(attr->function.vendor, "%s", custom_conf.function.vendor);
    }
    if(CUSTOM_check_int_valid(custom_conf.function.powerLineFrequencyMode)){
        attr->function.powerLineFrequencyMode  = custom_conf.function.powerLineFrequencyMode;
    }
    if(CUSTOM_check_string_valid(custom_conf.function.model))
    {
        snprintf(attr->function.model, sizeof(attr->function.model), "%s", custom_conf.function.model);
    }
    if(CUSTOM_check_int_valid(custom_conf.function.audioInput))
    {
        attr->function.audioInput = custom_conf.function.audioInput;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.audioOutput))
    {
        attr->function.audioOutput = custom_conf.function.audioOutput;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.lightControl))
    {
        attr->function.lightControl = custom_conf.function.lightControl;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.bulbControl))
    {
        attr->function.bulbControl = custom_conf.function.bulbControl;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.ptz))
    {
        attr->function.ptz = custom_conf.function.ptz;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.sdCard))
    {
        attr->function.sdCard = custom_conf.function.sdCard;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.fisheye))
    {
        attr->function.fisheye = custom_conf.function.fisheye;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.pirEnabled))
    {
        attr->function.pirEnabled = custom_conf.function.pirEnabled;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.pirTrigger))
    {
        attr->function.pirTrigger = custom_conf.function.pirTrigger;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.flipEnabled))
    {
        attr->function.flipEnabled = custom_conf.function.flipEnabled;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.mirrorEnabled))
    {
        attr->function.mirrorEnabled = custom_conf.function.mirrorEnabled;
    }
    if(CUSTOM_check_int_valid(custom_conf.function.customAlarmSound))
    {
        attr->function.customAlarmSound = custom_conf.function.customAlarmSound;
    }

	//protocal
	if(CUSTOM_check_int_valid(custom_conf.protocol.hikvision)){
		attr->protocol.hikvision = custom_conf.protocol.hikvision;
	}

    //ptz
	if(CUSTOM_check_int_valid(custom_conf.st_ptzAttr.nAddress)){
		attr->st_ptzAttr.nAddress = custom_conf.st_ptzAttr.nAddress;
	}
    if(CUSTOM_check_int_valid(custom_conf.st_ptzAttr.nBaudRate)){
		attr->st_ptzAttr.nBaudRate = custom_conf.st_ptzAttr.nBaudRate;
	}
    if(CUSTOM_check_int_valid(custom_conf.st_ptzAttr.nDateBit)){
		attr->st_ptzAttr.nDateBit = custom_conf.st_ptzAttr.nDateBit;
	}
    if(CUSTOM_check_int_valid(custom_conf.st_ptzAttr.nStopBit)){
		attr->st_ptzAttr.nStopBit = custom_conf.st_ptzAttr.nStopBit;
	}
    if(CUSTOM_check_string_valid(custom_conf.st_ptzAttr.strPtzCustomTpye)){
		strncpy(attr->st_ptzAttr.strPtzCustomTpye, custom_conf.st_ptzAttr.strPtzCustomTpye, sizeof(attr->st_ptzAttr.strPtzCustomTpye));
	}

    //motor
    if(CUSTOM_check_int_valid(custom_conf.motor.motorEnabled)){
        attr->motor.motorEnabled = custom_conf.motor.motorEnabled;
    }
    if(CUSTOM_check_int_valid(custom_conf.motor.motorType)){
        attr->motor.motorType = custom_conf.motor.motorType;
    }

	return 0;
}

static LP_JSON_OBJECT custom_create_object(LP_CUSTOM_SETTING attr)
{
	LP_JSON_OBJECT obj = json_object_new_object();
	LP_JSON_OBJECT param =NULL;
	int model_flag = 0, function_flag = 0, protocol_flag = 0, module_flag = 0, ptz_flag = 0, motor_flag = 0;
	char *text;

	APP_TRACE("custom create json object");

	//model
	param = json_object_new_object();
	if(CUSTOM_check_string_valid(attr->model.oemNumber)){
		NETSDK_json_set_string2(param, "oemNumber", attr->model.oemNumber);
		model_flag = 1;
	}
    if(CUSTOM_check_int_valid(attr->model.productType)){
		text = NETSDK_MAP_DEC2STR(productType_map, attr->model.productType, eNSDK_PRODUCT_TYPE_PX);
		NETSDK_json_set_string(param, "productType", text);
		model_flag = 1;
	}
	if(model_flag){
		json_object_object_add(obj, "Model", param);
	}else{
		//needn't to add to json file
		json_object_put(param);
		param = NULL;
	}

    
    //ptz
    param = json_object_new_object();
    if(CUSTOM_check_int_valid(attr->st_ptzAttr.nAddress)){
        NETSDK_json_set_int(param, "address", attr->st_ptzAttr.nAddress);
        ptz_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->st_ptzAttr.nBaudRate)){
        NETSDK_json_set_int(param, "baudrate", attr->st_ptzAttr.nBaudRate);
        ptz_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->st_ptzAttr.nDateBit)){
        NETSDK_json_set_int(param, "databit", attr->st_ptzAttr.nDateBit);
        ptz_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->st_ptzAttr.nStopBit)){
        NETSDK_json_set_int(param, "stopbit", attr->st_ptzAttr.nStopBit);
        ptz_flag = 1;
    }

	if(CUSTOM_check_string_valid(attr->st_ptzAttr.strPtzCustomTpye)){
		NETSDK_json_set_string2(param, "ptzCustomTpye", attr->st_ptzAttr.strPtzCustomTpye);
		ptz_flag = 1;
	}

	
    if(ptz_flag){
        json_object_object_add(obj, "externalControl", param);
    }else{
        //needn't to add to json file
        json_object_put(param);
        param = NULL;
    }

    //motor
    param = json_object_new_object();
    if(CUSTOM_check_int_valid(attr->motor.motorEnabled)){
        NETSDK_json_set_int(param, "motorEnabled", attr->motor.motorEnabled);
        motor_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->motor.motorType)){
        NETSDK_json_set_boolean(param, "motorType", attr->motor.motorType);
        motor_flag = 1;
    }

    if(motor_flag){
        json_object_object_add(obj, "motorControl", param);
    }else{
        //needn't to add to json file
        json_object_put(param);
        param = NULL;
    }
    
	//function
	param = json_object_new_object();
	if(CUSTOM_check_int_valid(attr->function.audioInputGain)){
		NETSDK_json_set_int(param, "audioInputGain", attr->function.audioInputGain);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.audioInputVolume)){
		NETSDK_json_set_int(param, "audioInputVolume", attr->function.audioInputVolume);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.audioOutputGain)){
		NETSDK_json_set_int(param, "audioOutputGain", attr->function.audioOutputGain);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.fixMode)){
		text = NETSDK_MAP_DEC2STR(fixMode_map, attr->function.fixMode,eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL);
		NETSDK_json_set_string(param, "fixMode", text);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.imageStyle)){
		NETSDK_json_set_int(param, "imageStyle", attr->function.imageStyle);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.promptSoundType)){
		text = NETSDK_MAP_DEC2STR(promptSoundType_map, attr->function.promptSoundType,kNSDK_SYSTEM_PROMPT_SOUND_TYPE_CHINESE);
		NETSDK_json_set_string(param, "promptSoundType", text);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.ipAdapted)){
		NETSDK_json_set_int(param, "ipAdapted", attr->function.ipAdapted);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.irCutControlMode)){
		text = NETSDK_MAP_DEC2STR(irCutControlMode_map, attr->function.irCutControlMode,kNSDK_IMAGE_IRCUT_CONTROL_MODE_HARDWARE);
		NETSDK_json_set_string(param, "irCutControlMode", text);
		function_flag = 1;
	}
	if(CUSTOM_check_array_valid(attr->function.irCutControlThresh, 8)){
		char array_str[32];
		custom_setting_array2str(array_str, attr->function.irCutControlThresh);
		NETSDK_json_set_string(param, "irCutControlThresh", array_str);
		function_flag = 1;
	}
	if(CUSTOM_check_string_valid(attr->function.p2pServerIp)){
		NETSDK_json_set_string(param, "p2pServerIp", attr->function.p2pServerIp);
		function_flag = 1;
	}
    if(CUSTOM_check_int_valid(attr->function.audioHwSpec)){
		NETSDK_json_set_int(param, "audioHwSpec", attr->function.audioHwSpec);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.ledPwmEnabled)){
		NETSDK_json_set_boolean(param, "ledPwmEnabled", attr->function.ledPwmEnabled);
		function_flag = 1;
	}
	if(CUSTOM_check_int_valid(attr->function.ledPwmChannelCount)){
		NETSDK_json_set_int(param, "ledPwmChannelCount", attr->function.ledPwmChannelCount);
		function_flag = 1;
	}
    if(CUSTOM_check_string_valid(attr->function.vendor)){
        NETSDK_json_set_string(param, "vendor", attr->function.vendor);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.powerLineFrequencyMode)){
        NETSDK_json_set_int(param, "powerLineFrequencyMode", attr->function.powerLineFrequencyMode);
        function_flag = 1;
    }
    if(CUSTOM_check_string_valid(attr->function.model))
    {
        NETSDK_json_set_string(param, "model", attr->function.model);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.audioInput))
    {
        NETSDK_json_set_int(param, "audioInput", attr->function.audioInput);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.audioOutput))
    {
        NETSDK_json_set_int(param, "audioOutput", attr->function.audioOutput);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.lightControl))
    {
        NETSDK_json_set_int(param, "lightControl", attr->function.lightControl);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.bulbControl))
    {
        NETSDK_json_set_int(param, "bulbControl", attr->function.bulbControl);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.ptz))
    {
        NETSDK_json_set_int(param, "ptz", attr->function.ptz);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.sdCard))
    {
        NETSDK_json_set_int(param, "sdCard", attr->function.sdCard);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.fisheye))
    {
        NETSDK_json_set_int(param, "fisheye", attr->function.fisheye);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.pirEnabled)){
        NETSDK_json_set_boolean(param, "pirEnabled", attr->function.pirEnabled);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.pirTrigger)){
        text = NETSDK_MAP_DEC2STR(pirTrigger_map, attr->function.pirTrigger, "fallingEdge");
        NETSDK_json_set_string(param, "pirTrigger", text);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.flipEnabled)){
        NETSDK_json_set_boolean(param, "flipEnabled", attr->function.flipEnabled);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.mirrorEnabled)){
        NETSDK_json_set_boolean(param, "mirrorEnabled", attr->function.mirrorEnabled);
        function_flag = 1;
    }
    if(CUSTOM_check_int_valid(attr->function.customAlarmSound)){
        NETSDK_json_set_boolean(param, "customAlarmSound", attr->function.customAlarmSound);
        function_flag = 1;
    }
	if(function_flag){
		json_object_object_add(obj, "Function", param);
	}else{
		//needn't to add to json file
		json_object_put(param);
		param = NULL;
	}

	//protocol
	param = json_object_new_object();
	if(CUSTOM_check_int_valid(attr->protocol.hikvision)){
		NETSDK_json_set_boolean(param, "hikvision", attr->protocol.hikvision);
		protocol_flag = 1;
	}
	if(protocol_flag){
		json_object_object_add(obj, "Protocol", param);
	}else{
		//needn't to add to json file
		json_object_put(param);
		param = NULL;
	}

	//module

	if((0 == model_flag) &&
		(0 == function_flag) &&
		(0 == protocol_flag) &&
		(0 == module_flag) &&
		(0 == ptz_flag) &&
		(0 == motor_flag)){
		json_object_put(obj);
		obj = NULL;
	}
	
	return obj;
}

int CUSTOM_set_json_string(char * json_str)
{
	int ret = -1;
	LP_JSON_OBJECT json = NULL;

    APP_TRACE("custom set json");
	if(custom_attr){
		pthread_mutex_lock(&custom_attr->mutex);
		if(NULL != (json = NETSDK_json_parse(json_str))){
			custom_setting_save(json);
			custom_setting_load(CUSTOM_SETTING_FILE_PATH, &custom_attr->custom);
			json_object_put(json);
			ret = 0;
		}else{
		    APP_TRACE("NETSDK_json_parse failed!!! parse json_str = %s", json_str);
			ret = -1;
		}
	}
    else
    {
        APP_TRACE("custom_attr NULL");
    }

	pthread_mutex_unlock(&custom_attr->mutex);
	return ret;
}

bool CUSTOM_check_int_valid(int input)
{
	return (-1 == input)? false:true;
}

bool CUSTOM_check_string_valid(char *input)
{	
	return (strlen(input) > 0)? true:false;
}

bool CUSTOM_check_array_valid(int *input, int length)
{
	int i,sum = 0;;
	for(i = 0; i < length; i++){
		sum += *input;
		input++;
	}
	return (sum > 0)? true:false;
}

int CUSTOM_set(LP_CUSTOM_SETTING attr)
{
	LP_JSON_OBJECT obj = NULL;
    APP_TRACE("custom set");
	obj = custom_create_object(attr);
    if(obj){
		custom_setting_save(obj);
        custom_setting_load(CUSTOM_SETTING_FILE_PATH, &custom_attr->custom);
		json_object_put(obj);
	}
	obj = NULL;
	return 0;
}

int CUSTOM_get_json_string(char * json_str)
{
	custom_setting_file_to_string(json_str);
	return 0;
}

int CUSTOM_get(LP_CUSTOM_SETTING attr)
{
	if(attr && custom_attr){
		memcpy(attr, &custom_attr->custom, sizeof(ST_CUSTOM_SETTING));
		return 0;
	}
    APP_TRACE("custom get failed!!! custom_attr NULL");
	return -1;
}

int CUSTOM_init()
{
	int ret = 0;
    APP_TRACE("custom init");
	if(NULL == custom_attr){
		custom_attr = (LP_CUSTOM)calloc(sizeof(ST_CUSTOM), 1);
		ret = custom_setting_load(CUSTOM_SETTING_FILE_PATH, &custom_attr->custom);
		if((0 != ret)){
			//use default custom.conf in /media/custom
			ST_CUSTOM_SETTING custom_def;
			custom_setting_load(CUSTOM_SETTING_DEFAULT_FILE_PATH, &custom_def);
			custom_match_struct(&custom_def);
			CUSTOM_set(&custom_def);
			custom_setting_load(CUSTOM_SETTING_FILE_PATH, &custom_attr->custom);
		}else{
            APP_TRACE("custom settint load failed!!!");
		}
        custom_setting_dump(&custom_attr->custom);
		pthread_mutex_init(&custom_attr->mutex, NULL);
	}
	
	return 0;
}

void CUSTOM_destory()
{
    APP_TRACE("custom destory");
	if(custom_attr){
		free(custom_attr);
		custom_attr = NULL;
	}
}
