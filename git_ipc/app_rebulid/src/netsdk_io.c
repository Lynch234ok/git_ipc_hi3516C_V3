
#include "netsdk.h"
#include "netsdk_util.h"
#include "netsdk_private.h"
#include "generic.h"
#include "sdk/sdk_api.h"
#include "app_overlay.h"
#include "ticker.h"
#include "generic.h"
#include "app_debug.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// NetSDK general opertaions
/////////////////////////////////////////////////////////////////////////////////////////////////

static inline int IO_ENTER_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }
	return pthread_rwlock_wrlock(&netsdk->io_sync);
}

static int IO_LEAVE_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }
	return pthread_rwlock_unlock(&netsdk->io_sync);
}

void NETSDK_conf_io_save()
{
	if(0 == IO_ENTER_CRITICAL()){
		APP_TRACE("NetSDK IO Conf Save!!");
		NETSDK_conf_save(netsdk->io_conf, "io");
		IO_LEAVE_CRITICAL();
	}
}

void NETSDK_conf_io_save2()
{
	if(netsdk->io_conf_save){
		netsdk->io_conf_save(eNSDK_CONF_SAVE_JUST_SAVE,  1);
	}else{
		NETSDK_conf_io_save();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////
const char *alarmIOPortStatusJSON = {
	"{"
		"\"type\" : \"input\","
		"\"state\" : \"high\","
		"\"active\" : true,"
	"}"
};
/////////////////////////////////////////////////////////////////////////////////////////////////

static LP_JSON_OBJECT io_lookup_channel(LP_JSON_OBJECT channelListJSON, int channelID)
{
	int i = 0;
	int const channelListLen = json_object_array_length(channelListJSON);
	// one channel
	for(i = 0; i < channelListLen; ++i){
		LP_JSON_OBJECT channelJSON = json_object_array_get_idx(channelListJSON, i);
		int const lookupID = json_object_get_int(json_object_object_get(channelJSON, "id"));
		if(lookupID == channelID){
			return channelJSON;
		}
	}
	return NULL;
}

/*
/NetSDK/IO/alarmInput/channels
/NetSDK/IO/alarmInput/channels/properties
/NetSDK/IO/alarmInput/channels/ID
/NetSDK/IO/alarmInput/channels/ID/properties
/NetSDK/IO/alarmInput/channels/ID/portStatus
*/

static int io_alarm_input_channel_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT channelListJSON, LP_JSON_OBJECT formJSON, int id, char *content, int content_max)
{
	int i = 0, ii = 0, ret = kNSDK_INS_RET_INVALID_OPERATION;
	char text[128] = {""};
	const char *token = NULL;
	LP_JSON_OBJECT channelJSON = NULL;
	LP_JSON_OBJECT activeJSON = NULL;

	if(id > 0){
		channelJSON = io_lookup_channel(channelListJSON, id);io_lookup_channel(channelListJSON, id);
		activeJSON = NETSDK_json_get_child(channelJSON, "active");
	}

	if(NSDK_SUBURI_MATCH(token, subURI, "/PORTSTATUS")){
		if(HTTP_IS_GET(context)){
			if(netsdk->alarmInputChannelPortStatus){
				ST_NSDK_ALARM_IN_CH alarmInCh;
				int ioPortState = kNSDK_IO_STATE_LOW;
				if(NETSDK_conf_alarm_in_ch_get(id, &alarmInCh)
					&& 0 == netsdk->alarmInputChannelPortStatus(id, &alarmInCh, &ioPortState)){
					LP_JSON_OBJECT alarmStatusJSON = NETSDK_json_parse(alarmIOPortStatusJSON);
					if(NULL != alarmStatusJSON){
						NETSDK_json_set_string2(alarmStatusJSON, "type", "input");
						NETSDK_json_set_string2(alarmStatusJSON, "state", kNSDK_IO_STATE_LOW == ioPortState ? "low" : "high");
						NETSDK_json_set_boolean2(alarmStatusJSON, "active", ioPortState == alarmInCh.activeState);
						snprintf(content, content_max, "%s", json_object_to_json_string(alarmStatusJSON));
						json_object_put(alarmStatusJSON);
						alarmStatusJSON = NULL;
						ret = kNSDK_INS_RET_CONTENT_READY;
					}
				}else{
					ret = kNSDK_INS_RET_DEVICE_ERROR;
				}
			}else{
				ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
			}
			
			
		}
	}else{
		if(HTTP_IS_GET(context)){
			if(0 == id){
				snprintf(content, content_max, "%s", json_object_to_json_string(channelListJSON));
				ret = kNSDK_INS_RET_CONTENT_READY;
			}else if(id > 0){
				if(NULL != channelJSON){
					snprintf(content, content_max, "%s", json_object_to_json_string(channelJSON));
					ret = kNSDK_INS_RET_CONTENT_READY;
				}
			}
		}else if(HTTP_IS_PUT(context)){
			if(!formJSON){
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}else{
				if(0 == IO_ENTER_CRITICAL()){
					if(NULL != channelJSON){
						LP_JSON_OBJECT formActiveJSON = NETSDK_json_get_child(formJSON, "active");
						
						// content -> channel
						//NETSDK_json_copy_child(docChannel, refChannel, "id");
						if(NULL != formActiveJSON){
							NETSDK_json_copy_child(formActiveJSON, activeJSON, "defaultState");
							NETSDK_json_copy_child(formActiveJSON, activeJSON, "activeState");
						}
						ret = kNSDK_INS_RET_OK;
					}
					
					IO_LEAVE_CRITICAL();
					if(kNSDK_INS_RET_OK == ret){
						NETSDK_conf_io_save2();
					}
				}	
			}
		}
	}
	//APP_TRACE(content);	
	return ret;
}

static void io_alarm_input_remove_properties(LP_JSON_OBJECT alarm_out)
{
	int i = 0, ii = 0;
	LP_JSON_OBJECT channels = NETSDK_json_get_child(alarm_out, "alarmInputChannel");
	int const n_channels = json_object_array_length(channels);
	
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		LP_JSON_OBJECT channelActive = NETSDK_json_get_child(channel, "active");
		NETSDK_json_remove_properties(channelActive);
		NETSDK_json_remove_properties(channel);
	}
}

static int io_alarm_input_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT ioRefJSON, LP_JSON_OBJECT ioDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int content_max)
{
	int id = 0;
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	HTTP_CSTR_t token = NULL;
	char text[32] = {""};
	LP_JSON_OBJECT channelListJSON = NULL;

	if(H_IS_GET(context->request_header->method) && !NSDK_PROPERTIES(subURI)){
		io_alarm_input_remove_properties(ioDupJSON);
	}
	APP_TRACE("subURI: %s", subURI);

	// get channel list
	if(HTTP_IS_GET(context)){
		channelListJSON = NETSDK_json_get_child(ioDupJSON, "alarmInputChannel");
	}else{
		channelListJSON = NETSDK_json_get_child(ioRefJSON, "alarmInputChannel");
	}
	
	if(NSDK_SUBURI_MATCH(token, subURI, "/CHANNELS")){
		ret = io_alarm_input_channel_instance(context, subURI, channelListJSON, formJSON, 0, content, content_max);
	}else if(token = "/CHANNEL/%d", 1 == sscanf(subURI, token, &id)){
		subURI += snprintf(text, sizeof(text), token, id);
		ret = io_alarm_input_channel_instance(context, subURI, channelListJSON, formJSON, id, content, content_max);
	}
	return ret;
}


/*
/NetSDK/IO/alarmOutput/channels
/NetSDK/IO/alarmOutput/channels/properties
/NetSDK/IO/alarmOutput/channel/ID
/NetSDK/IO/alarmOutput/channel/ID/properties
/NetSDK/IO/alarmOutput/channel/ID/properties
/NetSDK/IO/alarmOutput/channel/ID/portStatus
/NetSDK/IO/alarmOutput/channel/ID/trigger
*/

static int io_alarm_output_channel_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT channelListJSON, LP_JSON_OBJECT formJSON, int id, char *content, int content_max)
{
	int i = 0, ii = 0, netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	char text[128] = {""};
	const char *token = NULL;
	LP_JSON_OBJECT channelJSON = NULL;
	LP_JSON_OBJECT activeJSON = NULL; // 

	APP_TRACE("subURI: %s", subURI);

	if(id > 0){
		channelJSON = io_lookup_channel(channelListJSON, id);io_lookup_channel(channelListJSON, id);
		activeJSON = NETSDK_json_get_child(channelJSON, "active");
	}

	if(NSDK_SUBURI_MATCH(token, subURI, "/PORTSTATUS")){
		if(HTTP_IS_GET(context)){
			if(netsdk->alarmOutputChannelPortStatus){
				ST_NSDK_ALARM_OUT_CH alarmOutCh;
				int ioPortState = kNSDK_IO_STATE_LOW;
				if(NETSDK_conf_alarm_out_ch_get(id, &alarmOutCh)
					&& 0 == netsdk->alarmOutputChannelPortStatus(id, &alarmOutCh, &ioPortState)){
					LP_JSON_OBJECT alarmStatusJSON = NETSDK_json_parse(alarmIOPortStatusJSON);
					if(NULL != alarmStatusJSON){
						NETSDK_json_set_string2(alarmStatusJSON, "type", "output");
						NETSDK_json_set_string2(alarmStatusJSON, "state", kNSDK_IO_STATE_LOW == ioPortState ? "low" : "high");
						NETSDK_json_set_boolean2(alarmStatusJSON, "active", ioPortState == alarmOutCh.activeState);
						snprintf(content, content_max, "%s", json_object_to_json_string(alarmStatusJSON));
						json_object_put(alarmStatusJSON);
						alarmStatusJSON = NULL;
						netsdkRet = kNSDK_INS_RET_CONTENT_READY;
					}
				}else{
					netsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
				}
			}else{
				netsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
			}
		}
	}else if(NSDK_SUBURI_MATCH(token, subURI, "/TRIGGER")){
		if(HTTP_IS_PUT(context)){					
			LP_HTTP_QUERY_PARA_LIST queryList = HTTP_UTIL_parse_query_as_para(context->request_header->query);
			if(NULL != queryList){
				const char *outputState = queryList->read(queryList, "outputState");
				ST_NSDK_ALARM_OUT_CH_TRIGGER_METHOD triggerMethod;
				bool triggerValid = true;

				APP_TRACE("outputState: %s", outputState);
				
				if(NULL != outputState){
					if(0 == strcasecmp("high", outputState)){
						triggerMethod.outputState = kNSDK_IO_STATE_HIGH;
					}else if(0 == strcasecmp("low", outputState)){
						triggerMethod.outputState = kNSDK_IO_STATE_LOW;
					}else if(0 == strcasecmp("pulse", outputState)){
						triggerMethod.outputState = kNSDK_IO_STATE_PULSE;
						triggerMethod.pulseState = kNSDK_IO_STATE_HIGH;
						triggerMethod.pulseDuration = 1;
						
						token = queryList->read(queryList, "pulseState");
						if(NULL != token){
							if(0 == strcasecmp("high", token)){
								triggerMethod.pulseState = kNSDK_IO_STATE_HIGH;
							}else if(0 == strcasecmp("low", token)){
								triggerMethod.pulseState = kNSDK_IO_STATE_LOW;
							}
						}

						token = queryList->read(queryList, "pulseDuration");
						if(NULL != token){
							triggerMethod.pulseDuration = atoi(token);
						}
					}else{
						triggerValid = false;
					}
					
					if(triggerValid){
						if(netsdk->alarmOutputChannelTrigger
							&& 0 == netsdk->alarmOutputChannelTrigger(id, &triggerMethod)){
							netsdkRet = kNSDK_INS_RET_OK;
						}else{
							netsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}
				}
				queryList->free(queryList);
				queryList = NULL;
			}	
		}
	}else{
		if(HTTP_IS_GET(context)){
			if(0 == id){
				snprintf(content, content_max, "%s", json_object_to_json_string(channelListJSON));
				netsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}else if(id > 0){
				if(NULL != channelJSON){
					snprintf(content, content_max, "%s", json_object_to_json_string(channelJSON));
					netsdkRet = kNSDK_INS_RET_CONTENT_READY;
				}
			}
		}else if(HTTP_IS_PUT(context)){
			if(!formJSON){
				netsdkRet = kNSDK_INS_RET_INVALID_DOCUMENT;
			}else{
				if(NULL != channelJSON){
					if(0 == IO_ENTER_CRITICAL()){
						LP_JSON_OBJECT activeJSON = NETSDK_json_get_child(channelJSON, "active");
						LP_JSON_OBJECT formActiveJSON = NETSDK_json_get_child(formJSON, "active");
						
						// form -> channel
						NETSDK_json_copy_child(formJSON, channelJSON, "id");
						NETSDK_json_copy_child(formJSON, channelJSON, "powerOnState");
						NETSDK_json_copy_child(formJSON, channelJSON, "pulseDuration");
						NETSDK_json_copy_child(formActiveJSON, activeJSON, "defaultState");
						NETSDK_json_copy_child(formActiveJSON, activeJSON, "activeState");
						netsdkRet = kNSDK_INS_RET_OK;
						IO_LEAVE_CRITICAL();
						// save
						NETSDK_conf_io_save2();
					}
				}
			}
		}
	}
	return netsdkRet;
}

static void io_alarm_output_remove_properties(LP_JSON_OBJECT alarm_out)
{
	int i = 0, ii = 0;
	LP_JSON_OBJECT channels = NETSDK_json_get_child(alarm_out, "alarmOutputChannel");
	int const n_channels = json_object_array_length(channels);
	
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		LP_JSON_OBJECT channelActive = NETSDK_json_get_child(channel, "active");
		NETSDK_json_remove_properties(channelActive);
		NETSDK_json_remove_properties(channel);
	}
}

static int io_alarm_output_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT ioRefJSON, LP_JSON_OBJECT ioDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int content_max)
{
	int id = 0;
	int netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	HTTP_CSTR_t token = NULL;
	char text[32] = {""};
	LP_JSON_OBJECT channelListJSON = NULL;

	if(HTTP_IS_GET(context) && !NSDK_PROPERTIES(subURI)){
		io_alarm_output_remove_properties(ioDupJSON);
	}
	APP_TRACE("subURI: %s", subURI);

	// get channel list
	if(HTTP_IS_GET(context)){
		channelListJSON = NETSDK_json_get_child(ioDupJSON, "alarmOutputChannel");
	}else{
		channelListJSON = NETSDK_json_get_child(ioRefJSON, "alarmOutputChannel");
	}
	
	if(NSDK_SUBURI_MATCH(token, subURI, "/CHANNELS")){
		netsdkRet = io_alarm_output_channel_instance(context, subURI, channelListJSON, formJSON, 0, content, content_max);
	}else if(token = "/CHANNEL/%d", 1 == sscanf(subURI, token, &id)){
		subURI += snprintf(text, sizeof(text), token, id);
		netsdkRet = io_alarm_output_channel_instance(context, subURI, channelListJSON, formJSON, id, content, content_max);
	}
	return netsdkRet;
}

int NETSDK_io_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI, char *content, int content_max)
{
	int netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	HTTP_CSTR_t token = NULL;
	LP_JSON_OBJECT ioJSON = json_object_get(netsdk->io_conf);
	LP_JSON_OBJECT subRefJSON = NULL;
	LP_JSON_OBJECT subDupJSON = NULL;
	LP_JSON_OBJECT formJSON = NULL;
	bool alarmOutPut = false, alarmInPut = false;

	if(NSDK_SUBURI_MATCH(token, subURI, "/ALARMINPUT")){
		alarmInPut = true;
		subRefJSON = NETSDK_json_get_child(ioJSON, "alarmInput");
	}else if(NSDK_SUBURI_MATCH(token, subURI, "/ALARMOUTPUT")){
		alarmOutPut = true;
		subRefJSON = NETSDK_json_get_child(ioJSON, "alarmOutput");
	}

	// duplicated the reference JSON if GET method
	if(NULL != subRefJSON){
		if(HTTP_IS_GET(context)){
			subDupJSON = NETSDK_json_dup(subRefJSON);
		}
	}

	// parse form JSON for content
	if(NULL != context->request_content && 0 != context->request_content_len){
		formJSON = NETSDK_json_parse(context->request_content);
		//APP_TRACE("Form JSON:\r\n%s", json_object_to_json_string(formJSON));
	}

	if(alarmInPut){
		netsdkRet = io_alarm_input_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, content_max);
	}else if(alarmOutPut){
		netsdkRet = io_alarm_output_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, content_max);
	}

	// put all the references
	if(NULL != formJSON){
		json_object_put(formJSON);
		formJSON = NULL;
	}
	if(NULL != subDupJSON){
		json_object_put(subDupJSON);
	}
	if(NULL != ioJSON){
		json_object_put(ioJSON);
		ioJSON = NULL;
	}
	return netsdkRet;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////////////////////////////

static LP_NSDK_ALARM_IN_CH netsdk_conf_alarm_in_ch(bool setFlag, int channelID, LP_NSDK_ALARM_IN_CH alarmInCh)
{
	char text[64] = {""};
	bool setComplete = false;
	LP_NSDK_ALARM_IN_CH netsdkRet = NULL;
	
	if(NULL != alarmInCh && 0 == IO_ENTER_CRITICAL()){
		LP_JSON_OBJECT ioJSON = json_object_get(netsdk->io_conf);
		if(NULL != ioJSON){
			LP_JSON_OBJECT alarmInPutJSON = NETSDK_json_get_child(ioJSON, "alarmInput");
			//APP_TRACE("alarmInPutJSON:\r\n%s", json_object_to_json_string(alarmInPutJSON));
			if(NULL != alarmInPutJSON){
				LP_JSON_OBJECT channelListJSON = NETSDK_json_get_child(alarmInPutJSON, "alarmInputChannel");
				if(NULL != channelListJSON){
					LP_JSON_OBJECT channelJSON = io_lookup_channel(channelListJSON, channelID);
					if(NULL != channelJSON){
						LP_JSON_OBJECT activeJSON = NETSDK_json_get_child(channelJSON, "active");
						if(NULL != activeJSON){
							const char *defaultState = strdupa(NETSDK_json_get_string(activeJSON, "defaultState", text, sizeof(text)));
							const char *activeState = strdupa(NETSDK_json_get_string(activeJSON, "activeState", text, sizeof(text)));
								
							if(setFlag){		
								if(kNSDK_IO_STATE_LOW == alarmInCh->defaultState){
									defaultState = "low";
								}else{
									defaultState = "high";
								}
								if(kNSDK_IO_STATE_LOW == alarmInCh->activeState){
									activeState = "low";
								}else{
									activeState = "high";
								}
								NETSDK_json_set_string2(activeJSON, "defaultState", defaultState);
								NETSDK_json_set_string2(activeJSON, "activeState", activeState);
								setComplete = true;
							}else{
								alarmInCh->id = NETSDK_json_get_int(channelJSON, "id");
								if(STR_CASE_THE_SAME(defaultState, "low")){
									alarmInCh->defaultState = kNSDK_IO_STATE_LOW;
								}else{
									alarmInCh->defaultState = kNSDK_IO_STATE_HIGH;
								}
								if(STR_CASE_THE_SAME(activeState, "low")){
									alarmInCh->activeState = kNSDK_IO_STATE_LOW;
								}else{
									alarmInCh->activeState = kNSDK_IO_STATE_HIGH;
								}
								APP_TRACE("defaultState: \"%s\" activeState: \"%s\"", defaultState, activeState);
							}
							netsdkRet = alarmInCh;
						}
					}
				}
			}
			json_object_put(ioJSON);
			ioJSON = NULL;
		}
		IO_LEAVE_CRITICAL();
		if(setComplete){
			NETSDK_conf_io_save2();
		}
	}
	return netsdkRet;
}

LP_NSDK_ALARM_IN_CH NETSDK_conf_alarm_in_ch_get(int id, LP_NSDK_ALARM_IN_CH alarmInCh)
{	
	return netsdk_conf_alarm_in_ch(false, id, alarmInCh);
}

LP_NSDK_ALARM_IN_CH NETSDK_conf_alarm_int_ch_set(int id, LP_NSDK_ALARM_IN_CH alarmInCh)
{
	return netsdk_conf_alarm_in_ch(true, id, alarmInCh);
}

static LP_NSDK_ALARM_IN_CH netsdk_conf_alarm_out_ch(bool setFlag, int channelID, LP_NSDK_ALARM_OUT_CH alarmOutCh)
{
	char text[64] = {""};
	bool setComplete = false;
	LP_NSDK_ALARM_IN_CH netsdkRet = NULL;

	int id;
	int defaultState, activeState;
#define kNSDK_IO_PWR_ON_METHOD_CONT (0)
#define kNSDK_IO_PWR_ON_METHOD_PULSE (1)
	int powerOnMethod;
	int pulseDuration;

	if(NULL != alarmOutCh && 0 == IO_ENTER_CRITICAL()){
		LP_JSON_OBJECT ioJSON = json_object_get(netsdk->io_conf);
		if(NULL != ioJSON){
			LP_JSON_OBJECT alarmInPutJSON = NETSDK_json_get_child(ioJSON, "alarmOutput");
			//APP_TRACE("alarmInPutJSON:\r\n%s", json_object_to_json_string(alarmInPutJSON));
			if(NULL != alarmInPutJSON){
				LP_JSON_OBJECT channelListJSON = NETSDK_json_get_child(alarmInPutJSON, "alarmOutputChannel");
				if(NULL != channelListJSON){
					LP_JSON_OBJECT channelJSON = io_lookup_channel(channelListJSON, channelID);
					if(NULL != channelJSON){
						LP_JSON_OBJECT activeJSON = NETSDK_json_get_child(channelJSON, "active");
						const char *powerOnMethod = strdupa(NETSDK_json_get_string(channelJSON, "powerOnState", text, sizeof(text)));
						int pulseDuration = NETSDK_json_get_int(channelJSON, "pulseDuration");
						
						if(NULL != activeJSON){
							const char *defaultState = strdupa(NETSDK_json_get_string(activeJSON, "defaultState", text, sizeof(text)));
							const char *activeState = strdupa(NETSDK_json_get_string(activeJSON, "activeState", text, sizeof(text)));
							if(setFlag){		
								if(kNSDK_IO_STATE_LOW == alarmOutCh->defaultState){
									defaultState = "low";
								}else{
									defaultState = "high";
								}
								if(kNSDK_IO_STATE_LOW == alarmOutCh->activeState){
									activeState = "low";
								}else{
									activeState = "high";
								}
								NETSDK_json_set_string2(activeJSON, "defaultState", defaultState);
								NETSDK_json_set_string2(activeJSON, "activeState", activeState);
								setComplete = true;
							}else{
								if(STR_CASE_THE_SAME(defaultState, "low")){
									alarmOutCh->defaultState = kNSDK_IO_STATE_LOW;
								}else{
									alarmOutCh->defaultState = kNSDK_IO_STATE_HIGH;
								}
								if(STR_CASE_THE_SAME(activeState, "low")){
									alarmOutCh->activeState = kNSDK_IO_STATE_LOW;
								}else{
									alarmOutCh->activeState = kNSDK_IO_STATE_HIGH;
								}
							}
						}

						if(setFlag){
							if(kNSDK_IO_PWR_ON_METHOD_PULSE == alarmOutCh->powerOnMethod){
								powerOnMethod = "pulse";
							}else{
								powerOnMethod = "continuous";
							}
							NETSDK_json_set_string2(channelJSON, "powerOnMethod", powerOnMethod);
							NETSDK_json_set_string2(channelJSON, "pulseDuration", alarmOutCh->pulseDuration);
							setComplete = true;
						}else{
							alarmOutCh->id = NETSDK_json_get_int(channelJSON, "id");
							if(STR_CASE_THE_SAME(powerOnMethod, "pulse")){
								alarmOutCh->powerOnMethod = kNSDK_IO_PWR_ON_METHOD_PULSE;
							}else{
								alarmOutCh->powerOnMethod = kNSDK_IO_PWR_ON_METHOD_CONT;
							}
							alarmOutCh->pulseDuration = pulseDuration;
						}
						netsdkRet = alarmOutCh;
					}
				}
			}
			json_object_put(ioJSON);
			ioJSON = NULL;
		}
		IO_LEAVE_CRITICAL();
		if(setComplete){
			NETSDK_conf_io_save2();
		}
	}
	
	return netsdkRet;
}


LP_NSDK_ALARM_OUT_CH NETSDK_conf_alarm_out_ch_get(int id, LP_NSDK_ALARM_OUT_CH alarmOutCh)
{
	return netsdk_conf_alarm_out_ch(false, id, alarmOutCh);
}

LP_NSDK_ALARM_OUT_CH NETSDK_conf_alarm_out_ch_set(int id, LP_NSDK_ALARM_OUT_CH alarmOutCh)
{
	return netsdk_conf_alarm_out_ch(true, id, alarmOutCh);
}



