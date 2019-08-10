
#include <secure_chip.h>
#include "generic.h"
#include "app_debug.h"
#include "cgi_bin.h"
#include "netsdk.h"
#include "socket_tcp.h"
#include "global_runtime.h"
#include "bsp/bsp.h"
#include "bsp/keytime.h"
#include "custom.h"
#include "netsdk_private.h"
#include "led_pwm.h"
#include "sound.h"
#include "sensor.h"

#define FACTORY_TEST_CMD_CHECK_ENCTYPTION_CHIP "CheckEncryptionChip"
#define FACTORY_TEST_CMD_GET_RESET_KEY_INFO "GetResetKeyInfo"
#define FACTORY_TEST_CMD_GET_KEY_PRESS_STATUS "KeyPressStatus"
#define FACTORY_TEST_CMD_LED_MODE  "LEDMODE"
#define FACTORY_TEST_CMD_GET_LANGUAGE_PROPERTY	"GetLanguageProperty"
#define FACTORY_TEST_CMD_LIGHT_MODE "LIGHTMODE"

#define KEY_STATUS_UP       "up"
#define KEY_STATUS_DOWN     "down"

typedef struct LEDMODE_config_t
{
	struct
	{
		int id;
		char led_mode[20];
		int keeptime;
	}param[5];

}stLEDMODE_config,*lpLEDMODE_config;
static lpLEDMODE_config _led_config = NULL;

const ST_NSDK_MAP_STR_DEC led_mode_map[] = {
	{"flicker fastly", LED_MIN_MODE},
	{"flicker slowly", LED_MAX_MODE},
	{"dark", LED_DARK_MODE},
	{"light", LED_LIGHT_MODE},
};

typedef struct KeyAttr
{
    int id;
    char keyName[32];
    char status[8];
}stKeyAttr, *lpKeyAttr;

static stKeyInfo keyInfo = {
    KEY_MAX_NUM,
    {1, 1, 1, 1, 1, 1, 1, 1},
};

static stKeyAttr keyAttr[] = {
        {0, "Reset", KEY_STATUS_UP},
        {1, "Video", KEY_STATUS_UP},
};

extern int custom_conf_match();

int WEB_CGI_factory_test(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	const char *chip_name[] = {
		"NONE",
		"AT88SC",
		"LIC2",
		"24C02",
	};

	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	const char *cmd = NULL;
	LP_HTTP_HEAD_FIELD httpHeadField;
	char httpHeadBuf[512];
	int httpHeadLength = 0;
	char httpContentBuf[512];
	int httpContentLength = 0;
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(session->sock, &sockTCP);
	pHead= HTTP_UTIL_parse_query_as_para(session->request_header->query);

	if(NULL == pHead){
		goto http_head_parse_failed;
	}

	cmd = pHead->read(pHead, "cmd");
	if(NULL == cmd){
		goto http_param_parse_failed;
	}

	if(STR_THE_SAME_N(cmd, FACTORY_TEST_CMD_CHECK_ENCTYPTION_CHIP, strlen(FACTORY_TEST_CMD_CHECK_ENCTYPTION_CHIP))){

		LP_JSON_OBJECT resBodyJson = NULL;
		LP_JSON_OBJECT chipTextJson = NULL;				// object
		LP_JSON_OBJECT paraListJson = NULL;			    // array
		LP_JSON_OBJECT paraUidJson = NULL;			    // object
		char schip_md5[33];
		char sn[33];
		char uid[33];
		int revmark = -1;

		bool errOccur = false;

		do {

			if (!SECURE_CHIP_is_init_success()) {
				APP_TRACE("%s: Secure chip is not init success!", __FUNCTION__);
				errOccur = true;
				break;
			}

			if (!g_authorized) {
				APP_TRACE("%s: Product is not authorized!", __FUNCTION__);
				errOccur = true;
				break;
			}

			resBodyJson = json_object_new_object();
			if (NULL == resBodyJson) {
				APP_TRACE("%s: Failed to get new json object!", __FUNCTION__);
				errOccur = true;
				break;
			}

			// Result 字段
			json_object_object_add(resBodyJson, "Result", json_object_new_string("Success"));

			// Chip Type 字段
			json_object_object_add(resBodyJson, "Chip Type", json_object_new_string(chip_name[g_encryp_chip_type]));

			// RevMark 字段
			if (0 != SECURE_CHIP_get_revmark(&revmark)) {
				APP_TRACE("%s: Failed to get revmark!", __FUNCTION__);
				errOccur = true;
				break;
			}
			json_object_object_add(resBodyJson, "RevMark", json_object_new_int(revmark));

			// ChipText 字段
			chipTextJson = json_object_new_object();
			if (NULL == chipTextJson) {
				APP_TRACE("%s: Failed to get new json object!", __FUNCTION__);
				errOccur = true;
				break;
			}
            json_object_object_add(resBodyJson, "ChipText", chipTextJson);

            // ChipText.md5
            if (0 != SECURE_CHIP_get_data(SECURE_CHIP_MD5, schip_md5, sizeof(schip_md5))) {
                APP_TRACE("%s: Failed to get md5!", __FUNCTION__);
                errOccur = true;
                break;
            }
			json_object_object_add(chipTextJson, "md5", json_object_new_string(schip_md5));

			// ChipText.SN
			if (0 != SECURE_CHIP_get_data(SECURE_CHIP_DATA_SN, sn, sizeof(sn))) {
                APP_TRACE("%s: Failed to get SN!", __FUNCTION__);
                errOccur = true;
                break;
            }
			json_object_object_add(chipTextJson, "SN", json_object_new_string(sn));

			// ChipText.ParamList
            paraListJson = json_object_new_array();
            if (NULL == paraListJson) {
                APP_TRACE("%s: Failed to get new json array!", __FUNCTION__);
                errOccur = true;
                break;
            }
            json_object_object_add(chipTextJson, "ParamList", paraListJson);

            // ChipText.ParamList[].Key uid
            paraUidJson = json_object_new_object();
            if (NULL == paraUidJson) {
                APP_TRACE("%s: Failed to get new json object!", __FUNCTION__);
                errOccur = true;
                break;
            }
            json_object_array_add(paraListJson, paraUidJson);

            if (0 != SECURE_CHIP_get_data(SECURE_CHIP_DATA_UID, uid, sizeof(uid))) {
                APP_TRACE("%s: Failed to get uid!", __FUNCTION__);
                errOccur = true;
                break;
            }
            json_object_object_add(paraUidJson, "Key", json_object_new_string("uid"));
            json_object_object_add(paraUidJson, "ParamList", json_object_new_string(uid));

        } while(0);

		if (errOccur) {
			snprintf(httpContentBuf, sizeof(httpContentBuf),
					 "{\"Result\":\"%s\", \"Chip Type\":\"%s\", \"RevMark\":0, \"ChipText\": {}}",
					 "Fail", chip_name[g_encryp_chip_type]);
		} else {
			snprintf(httpContentBuf, sizeof(httpContentBuf), "%s", json_object_to_json_string(resBodyJson));
		}

		if (NULL != resBodyJson) {
			json_object_put(resBodyJson);
		}

	}else if(STR_THE_SAME_N(cmd, FACTORY_TEST_CMD_GET_RESET_KEY_INFO, strlen(FACTORY_TEST_CMD_GET_RESET_KEY_INFO))){
		time_t time_begin = time(NULL), time_end = 0;
        int keyVal[KEY_MAX_NUM];
        while((time_end = time(NULL)) - time_begin <= 10){
            BSP_Get_Key_Val(keyVal);
			if(BSP_KEY_PRESS == keyVal[0]){
				ret = 1;
				break;
			}
			usleep(50000);//50ms
		}
		snprintf(httpContentBuf, sizeof(httpContentBuf),
		"{\"Result\":\"%s\"}",
		ret?"Success":"Fail");
    }else if(STR_THE_SAME_N(cmd, FACTORY_TEST_CMD_GET_KEY_PRESS_STATUS, strlen(FACTORY_TEST_CMD_GET_KEY_PRESS_STATUS))){
        if(HTTP_IS_GET(session)) {
            int i = 0;
            int numTmp = 0;
            /* 检查keyInfo.keyNum个按键是否已有被按下的记录 */
            for(i = 0; i < keyInfo.keyNum; i++) {
                if(keyInfo.keyVal[i] == BSP_KEY_PRESS) {
                    numTmp++;
                    snprintf(keyAttr[i].status, sizeof(keyAttr[i].status), "%s", KEY_STATUS_DOWN);
                }
            }

            /* numTmp == keyInfo.keyNum可判断出全部按键已有被按下的记录 */
    		if(numTmp == keyInfo.keyNum) {
            }
            else {
    			KEY_startGetKeyAttr(&keyInfo);
            }

            // FIXME
            snprintf(httpContentBuf, sizeof(httpContentBuf),
            "{\"KeyStatus\":[{\"id\":%d,\"KeyName\":\"%s\",\"Status\":\"%s\"},{\"id\":%d,\"KeyName\":\"%s\",\"Status\":\"%s\"}]}",
            keyAttr[0].id, keyAttr[0].keyName, keyAttr[0].status,
            keyAttr[1].id, keyAttr[1].keyName, keyAttr[1].status);
        }
        else {
            /* 重置按键检测状态 */
            if(session->request_content != NULL) {
                char actionVal[16];
                int i = 0;
                LP_JSON_OBJECT key_json = NETSDK_json_parse(session->request_content);
                if(key_json) {
                    NETSDK_json_get_string(key_json, "Action", actionVal, sizeof(actionVal));
                }
                if(!strcmp(actionVal, "Reset")) {
                    KEY_stopGetKeyAttr();
                    for(i = 0; i < keyInfo.keyNum; i++) {
                        keyInfo.keyVal[i] = BSP_KEY_RELEASE;
                        snprintf(keyAttr[i].status, sizeof(keyAttr[i].status), "%s", KEY_STATUS_UP);
                    }
                    ret = 1;
                }
                else {
                    ret = 0;
                }
            }
            snprintf(httpContentBuf, sizeof(httpContentBuf),
		        "{\"Result\":\"%s\"}",
		        ret?"true":"false");
        }
    }else if(STR_THE_SAME_N(cmd,FACTORY_TEST_CMD_LED_MODE,strlen(FACTORY_TEST_CMD_LED_MODE))){
        if(HTTP_IS_GET(session)) {
            if(!_led_config) {
                _led_config = calloc(1, sizeof(stLEDMODE_config));
            }

            LP_JSON_OBJECT led_mode_json = json_object_new_object();
            LP_JSON_OBJECT ledMode = json_object_new_array();
            LP_JSON_OBJECT param = NULL;
            int i,LedMode;
            char * str = NULL;

            for(i = 0; i < (sizeof(_led_config->param) / sizeof(_led_config->param[0])); i++)
            {
                param = json_object_new_object();

                _led_config->param[i].id = i;
                LedMode = KEY_LED_get_mode(_led_config->param[i].id);
                str = NETSDK_MAP_DEC2STR(led_mode_map,LedMode,"dark");
                memcpy(_led_config->param[i].led_mode,str,sizeof(_led_config->param[i].led_mode));
                _led_config->param[i].keeptime = KEY_LED_get_keep_time(_led_config->param[i].id);

                NETSDK_json_set_int(param,"id",_led_config->param[i].id);
                NETSDK_json_set_string(param,"mode",_led_config->param[i].led_mode);
                NETSDK_json_set_int(param,"keeptime",_led_config->param[i].keeptime);

                json_object_array_put_idx(ledMode, i, param);
            }

            json_object_object_add(led_mode_json, "LEDMODE", ledMode);

            snprintf(httpContentBuf, sizeof(httpContentBuf), "%s", json_object_to_json_string(led_mode_json));

            if(led_mode_json) {
                json_object_put(led_mode_json);
                led_mode_json = NULL;

                json_object_put(param);
                param = NULL;
            }

            free(_led_config);
            _led_config = NULL;

        }else{
            if(session->request_content != NULL) {
                LP_JSON_OBJECT led_mode_json = NETSDK_json_parse(session->request_content);
                if(!_led_config){
                    _led_config = calloc(1, sizeof(stLEDMODE_config));
                }

                if(led_mode_json) {

                    LP_JSON_OBJECT child_ledMode = NETSDK_json_get_child(led_mode_json, "LEDMODE");

                    if(child_ledMode) {

                        int const led_array_num = json_object_array_length(child_ledMode);
                        int i, LedMode;

                        for(i = 0 ; i < led_array_num; i++){
                            LP_JSON_OBJECT led_json = json_object_array_get_idx(child_ledMode,i);

                            _led_config->param[i].id =  NETSDK_json_get_int(led_json, "id");
                            NETSDK_json_get_string(led_json, "mode", _led_config->param[i].led_mode, sizeof(_led_config->param[i].led_mode));
                            _led_config->param[i].keeptime = NETSDK_json_get_int(led_json, "keeptime");

                            LedMode = NETSDK_MAP_STR2DEC(led_mode_map,_led_config->param[i].led_mode,LED_DARK_MODE);
                            KEY_LED_set_keep_time(_led_config->param[i].keeptime);
                            if(KEY_LED_set_mode(_led_config->param[i].id,true,LedMode) == 0) {
                                ret = 1;
                            }
                        }
                    }
                }
                free(_led_config);
                _led_config = NULL;

                snprintf(httpContentBuf, sizeof(httpContentBuf),
		        "{\"Result\":\"%s\"}",
		        ret?"Success":"Fail");
            }
        }
    }
	else if(STR_THE_SAME_N(cmd, FACTORY_TEST_CMD_GET_LANGUAGE_PROPERTY, strlen(FACTORY_TEST_CMD_GET_LANGUAGE_PROPERTY))){
        if(HTTP_IS_GET(session)) {
			ST_NSDK_SYSTEM_SETTING sysInfo;
			NETSDK_conf_system_get_setting_info(&sysInfo);
			snprintf(httpContentBuf, sizeof(httpContentBuf),
						"{\"languageDefault\":\"%s\",\"languageProperty\":{\"type\":\"string\",\"mode\":\"ro\",\"opt\":%s }}",
						sysInfo.promptSound.soundTypeStr, sysInfo.promptSound.soundTypeOpt);
		}

	}
	else if(STR_THE_SAME_N(cmd,FACTORY_TEST_CMD_LIGHT_MODE,strlen(FACTORY_TEST_CMD_LIGHT_MODE))){
			bool set_light_success = false;
			BSP_WHITE_LIGHT_Led(true);
#if  defined(HI3516E_V1)
			LED_PWM_duty_cycle_set(1, 100);
#endif
			usleep(200000);
			BSP_WHITE_LIGHT_Led(false);
#if  defined(HI3516E_V1)
			LED_PWM_duty_cycle_set(1, 0);
#endif
			set_light_success = true;

			snprintf(httpContentBuf, sizeof(httpContentBuf),
			"{\"Result\":\"%s\"}",
			set_light_success?"Success":"Fail");
	}
	else{
		snprintf(httpContentBuf, sizeof(httpContentBuf),
		"{\"Error_description\":\"wrong interface\"}");
	}

	
	httpContentLength = strlen(httpContentBuf);

	httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
	httpHeadField->add_tag_date(httpHeadField, 0);
	httpHeadField->add_tag_text(httpHeadField, "Content-Type", "application/json", true);
	httpHeadField->add_tag_int(httpHeadField, "Content-Length", httpContentLength, true);

	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;
	
	// send out http head when situation not 200 ok
	if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)){
		// send out http content
		if(httpContentLength > 0){
			if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)){
				// send
			}
		}
	}
	
http_param_parse_failed:
	pHead->free(pHead);
	pHead = NULL;

http_head_parse_failed:
	return 0;
}



extern void ipcam_alarm_in_init();
extern void ipcam_bsp_init(int val, int audioHwSpec);

int WEB_CGI_factory_setting(LP_HTTP_CONTEXT session)
{
	int ret = 0;

	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	const char *cmd = NULL;
	LP_HTTP_HEAD_FIELD httpHeadField;
	char httpHeadBuf[512];
	int httpHeadLength = 0;
	char httpContentBuf[1024];
	int httpContentLength = 0;
    ST_CUSTOM_SETTING custom;
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(session->sock, &sockTCP);
	if(HTTP_IS_GET(session)){
		CUSTOM_get_json_string(httpContentBuf);
	}else{
		ret = CUSTOM_set_json_string(session->request_content);       
		if(0 == ret){
#if defined (LED_PWM)
			if(0 == CUSTOM_get(&custom)) {
				/* 产测定制led pwm */
				if(CUSTOM_check_int_valid(custom.function.ledPwmEnabled) 
					&& CUSTOM_check_int_valid(custom.function.ledPwmChannelCount)) {
					ret = LED_PWM_custom(custom.function.ledPwmEnabled, custom.function.ledPwmChannelCount);
				}
			}
#endif
			custom_conf_match();
		}
		snprintf(httpContentBuf, sizeof(httpContentBuf),
		"{\"Result\":\"%s\"}",
		(0 == ret)?"Success":"Fail");
	}

	httpContentLength = strlen(httpContentBuf);
	
	httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
	httpHeadField->add_tag_date(httpHeadField, 0);
	httpHeadField->add_tag_text(httpHeadField, "Content-Type", "application/json", true);
	httpHeadField->add_tag_int(httpHeadField, "Content-Length", httpContentLength, true);

	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;
	
	// send out http head when situation not 200 ok
	if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)){
		// send out http content
		if(httpContentLength > 0){
			if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)){
				// send
			}
		}
	}

	if(HTTP_IS_PUT(session) || HTTP_IS_POST(session)) {
		ST_CUSTOM_SETTING custom;
        ST_NSDK_VIN_CH vin_ch;
		if(0 == CUSTOM_get(&custom)) {
			if(CUSTOM_check_int_valid(custom.function.irCutControlMode)) {
				ST_NSDK_IMAGE image;
				NETSDK_conf_image_get(&image);
				SENSOR_ircut_control_mode_set(image.irCutFilter.irCutControlMode);
			}

			if(CUSTOM_check_int_valid(custom.function.promptSoundType)) {
				SearchFileAndPlay(SOUND_Configuration_mode, NK_True); // 播放定制的语言提示音
			}

            if(CUSTOM_check_int_valid(custom.function.powerLineFrequencyMode)) {
                if(NETSDK_conf_vin_ch_get(1, &vin_ch)) {
                    if(netsdk->videoInputChannelChanged) {
                        netsdk->videoInputChannelChanged(1, &vin_ch);
                    }
                }
            }

#if defined(PIR_ALARM)
            if(true == CUSTOM_check_int_valid(custom.function.pirEnabled))
            {
#if defined(HI3516E_V1)
                ipcam_bsp_init(0, 0);
#endif
                ipcam_alarm_in_init();
            }
#endif

#if defined(UART_PROTOCOL)
            if(CUSTOM_check_int_valid(custom.motor.motorEnabled))
            {
                APP_MOTOR_Exit();
                APP_MOTOR_Init();
            }
#endif
		}
	}

http_head_parse_failed:
	return 0;
}

