
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
/*static int audio_sync_read_lock()
{
	return NETSDK_private_read_lock(&netsdk->audio_sync);
}

static int audio_sync_try_read_lock()
{
	return NETSDK_private_try_read_lock(&netsdk->audio_sync);
}

static int audio_sync_write_lock()
{
	return NETSDK_private_write_lock(&netsdk->audio_sync);
}

static int audio_sync_try_write_lock()
{
	return NETSDK_private_try_write_lock(&netsdk->audio_sync);
}

static int audio_sync_unlock()
{
	return NETSDK_private_unlock(&netsdk->audio_sync);
}*/

const ST_NSDK_MAP_STR_DEC audio_type_map[] = {
	{"G.711alaw", kNSDK_AENC_CODEC_TYPE_G711A},
	{"G.711ulaw", kNSDK_AENC_CODEC_TYPE_G711U},
	{"AAC", kNSDK_AENC_CODEC_TYPE_AAC}
};

const ST_NSDK_MAP_STR_DEC microphoneType_map[] = {
	{"activePickup", kNSDK_ACTIVE_PICKUP},
	{"passiveMic", kNSDK_PASSIVE_MIC},
};

const ST_NSDK_MAP_STR_DEC audio_Input_workMode_map[] = {
	{"input", kNSDK_AIN_WORK_MODE_INPUT},
	{"output", kNSDK_AIN_WORK_MODE_OUTPUT},
	{"input&output", kNSDK_AIN_WORK_MODE_IO},
};

static inline int AUDIO_ENTER_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }
	return pthread_rwlock_wrlock(&netsdk->audio_sync);
}

static int AUDIO_LEAVE_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }
	return pthread_rwlock_unlock(&netsdk->audio_sync);
}



void NETSDK_conf_audio_save()
{
    if(0 == AUDIO_ENTER_CRITICAL())
    {
        APP_TRACE("NetSDK Audio Conf Save!!");
        NETSDK_conf_save(netsdk->audio_conf, "audio");
        AUDIO_LEAVE_CRITICAL();
    }
}

void NETSDK_conf_audio_save2()
{
	if(netsdk->audio_conf_save){
		netsdk->audio_conf_save(eNSDK_CONF_SAVE_JUST_SAVE, 1);
	}else{
		NETSDK_conf_audio_save();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/*
/NetSDK/Audio/input/channels
/NetSDK/Audio/input/channels/properties
/NetSDK/Audio/input/channel/ID
/NetSDK/Audio/input/channel/ID/properties
*/

static LP_JSON_OBJECT audio_find_channel(LP_JSON_OBJECT channels, int id)
{
	int i = 0;
	int const n_channels = json_object_array_length(channels);
	// one channel
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		if(json_object_get_int(json_object_object_get(channel, "id")) == id){
			return channel;
		}
	}
	return NULL;
}


static void audio_input_remove_properties(LP_JSON_OBJECT ain)
{	
	int i = 0, ii = 0;
	LP_JSON_OBJECT channels = NETSDK_json_get_child(ain, "audioInputChannel");
	int const n_channels = json_object_array_length(channels);
	
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		NETSDK_json_remove_properties(channel);
	}
}


static int audio_input_channels(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri,
	int id, bool properties, char *content, int content_max)
{
	int i = 0, ret = 0;
	char *json_text = NULL;
	LP_JSON_OBJECT audio = NETSDK_json_dup(netsdk->audio_conf);
	LP_JSON_OBJECT input = NETSDK_json_get_child(audio, "audioInput");
	LP_JSON_OBJECT channels = NETSDK_json_get_child(input, "audioInputChannel");
	
	if(!properties){
		audio_input_remove_properties(input);
	}

	if(0 == id){
		snprintf(content, content_max, "%s", json_object_to_json_string(channels));
		ret = kNSDK_INS_RET_CONTENT_READY;
	}else if(id > 0){
		int const n_channels = json_object_array_length(channels);
		// one channel
		for(i = 0; i < n_channels; ++i){
			LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
			if(json_object_get_int(json_object_object_get(channel, "id")) == id){
				snprintf(content, content_max, "%s", json_object_to_json_string(channel));
				ret = kNSDK_INS_RET_CONTENT_READY;
				break;
			}
		}
	}

	// release json
	json_object_put(audio);
	audio = NULL;

	return ret;
}

static int audio_input_channel(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri,
	int id, bool properties, char *content, int content_max)
{
	int i = 0, ret = 0;
	char *json_text = NULL;
	LP_JSON_OBJECT audio = json_object_get(netsdk->audio_conf);
	LP_JSON_OBJECT input_ref = NETSDK_json_get_child(audio, "audioInput");
	LP_JSON_OBJECT input_dup = NETSDK_json_dup(input_ref);
	LP_JSON_OBJECT channels = NETSDK_json_get_child(input_dup, "audioInputChannel");

	if(H_IS_GET(context->request_header->method)){
		if(!properties){
			audio_input_remove_properties(input_dup);
		}

		if(0 == id){
			snprintf(content, content_max, "%s", json_object_to_json_string(channels));
			ret = kNSDK_INS_RET_CONTENT_READY;
		}else if(id > 0){
			LP_JSON_OBJECT channel = audio_find_channel(channels, id);
			// one channel
			snprintf(content, content_max, "%s", json_object_to_json_string(channel));
			ret = kNSDK_INS_RET_CONTENT_READY;
		}
	}else if(H_IS_PUT(context->request_header->method)){
		if(NULL != context->request_content){
			LP_JSON_OBJECT form = NETSDK_json_parse(context->request_content);
			APP_TRACE("Content : %s", context->request_content);
			if(!form){
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}else{
				LP_JSON_OBJECT channels = NETSDK_json_get_child(input_ref, "audioInputChannel");
				APP_TRACE(json_object_to_json_string(form));	
				if(0 == id){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}else if(id > 0){
					LP_JSON_OBJECT channel = audio_find_channel(channels, id);
					if(NULL != channel){
						//NETSDK_json_copy_child(form, channel, "id");
						NETSDK_json_copy_child(form, channel, "inputVolume");
						NETSDK_json_copy_child(form, channel, "outputVolume");
						NETSDK_json_copy_child(form, channel, "microphoneType");
						//FIX ME   do callback to setting audio


					#if 1
						if(netsdk->audioInputChannelChanged){
							ST_NSDK_AIN_CH  ain_ch;
							NETSDK_conf_ain_ch_get(id, &ain_ch);
							if(netsdk->audioInputChannelChanged(id, &ain_ch)){
								// save to file
								NETSDK_conf_audio_save2();
								// response
								ret = kNSDK_INS_RET_OK;

							}else{
								ret = kNSDK_INS_RET_DEVICE_ERROR;
							}
						}
					#else

						NETSDK_conf_audio_save2();
						ret = kNSDK_INS_RET_OK;

					#endif
					}
				}
				json_object_put(form);
				form = NULL;
			}
		}
	}
		
	// release json
	json_object_put(input_dup);
	json_object_put(audio);
	audio = NULL;

	return ret;
}



static int audio_input_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri, char *content, int content_max)
{
	int id = 0, id_msk = 0;
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;
	
	if(prefix = "/CHANNELS", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		sub_uri += strlen(prefix);
		if(0 == strlen(sub_uri)){
			ret = audio_input_channels(context, sub_uri, 0, false, content, content_max);
		}else if(prefix = "/PROPERTIES", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
			ret = audio_input_channels(context, sub_uri, 0, true, content, content_max);
		}
	}else if(prefix = "/CHANNEL/%d", 1 == sscanf(sub_uri, prefix, &id)){
		if(id > 0){
			ret = sprintf(content, prefix, id);
			sub_uri += ret;
		//	APP_TRACE("URI : %s", sub_uri);

			if(0 == strlen(sub_uri)){
				ret = audio_input_channel(context, sub_uri, id, false, content, content_max);
			}else if(prefix = "/PROPERTIES", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
				ret = audio_input_channel(context, sub_uri, id, true, content, content_max);
			}
		}
	}
	//ret = kNSDK_INS_RET_CONTENT_READY;
	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/*
/NetSDK/Audio/encode/channels
/NetSDK/Audio/encode/channels/properties
/NetSDK/Audio/encode/channel/ID
/NetSDK/Audio/encode/channel/ID/properties
*/

static void audio_encode_remove_properties(LP_JSON_OBJECT aenc)
{
	int i = 0, ii = 0;
	LP_JSON_OBJECT channels = NETSDK_json_get_child(aenc, "audioEncodeChannel");
	int const n_channels = json_object_array_length(channels);
	
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		NETSDK_json_remove_properties(channel);
	}

}



static int audio_encode_channel(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri,
	LP_JSON_OBJECT json_ref, LP_JSON_OBJECT json_dup, int id, char *content, int content_max)
{
	int i = 0, ii = 0, ret = kNSDK_INS_RET_INVALID_OPERATION;
	char text[128] = {""};
	if(H_IS_GET(context->request_header->method)){
		LP_JSON_OBJECT channels = NETSDK_json_get_child(json_dup, "audioEncodeChannel");
		if(0 == id){
			snprintf(content, content_max, "%s", json_object_to_json_string(channels));
			ret = kNSDK_INS_RET_CONTENT_READY;
		}else if(id > 0){
			LP_JSON_OBJECT channel = audio_find_channel(channels, id);
			if(NULL != channel){
				snprintf(content, content_max, "%s", json_object_to_json_string(channel));
				ret = kNSDK_INS_RET_CONTENT_READY;
			}
		}		
	}else if(H_IS_PUT(context->request_header->method)){
		if(!context->request_content || 0 == context->request_content_len){
			ret = kNSDK_INS_RET_INVALID_DOCUMENT;
		}else{
			LP_JSON_OBJECT form = NETSDK_json_parse(context->request_content);
			if(!form){
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}else{
				LP_JSON_OBJECT channels = NETSDK_json_get_child(json_ref, "audioEncodeChannel");
				APP_TRACE(json_object_to_json_string(form));	
				if(0 == id){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}else if(id > 0){
					LP_JSON_OBJECT channel = audio_find_channel(channels, id);
					if(NULL != channel){
						NETSDK_json_copy_child(form, channel, "id");
						NETSDK_json_copy_child(form, channel, "enabled");
						//NETSDK_json_copy_child(form, channel, "audioInputChannelID");
						NETSDK_json_copy_child(form, channel, "codecType");

						//FIX ME   do callback to setting audio
						#if 1
						if(netsdk->audioEncodeChannelChanged){						
							ST_NSDK_AENC_CH aenc_ch;
							if(NETSDK_conf_aenc_ch_get(id, &aenc_ch)){
								if(0 == netsdk->audioEncodeChannelChanged(id, &aenc_ch)){
									// save to file
									NETSDK_conf_audio_save2();
									// response
									ret = kNSDK_INS_RET_OK;
								}else{
									ret = kNSDK_INS_RET_DEVICE_ERROR;
								}
							}
						}else{
							ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
						}
						#else
						NETSDK_conf_audio_save2();
						ret = kNSDK_INS_RET_OK;
						#endif
						
					}
				}
					
				json_object_put(form);
				form = NULL;
			}
		}
	}

	APP_TRACE(content);	
	return ret;
}

static int audio_encode_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri, char *content, int content_max)
{
	int id = 0;
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;
	LP_JSON_OBJECT video = json_object_get(netsdk->audio_conf);
	LP_JSON_OBJECT json_ref = NETSDK_json_get_child(video, "audioEncode");
	LP_JSON_OBJECT json_dup = NETSDK_json_dup(json_ref);

	// check properties necessary when GET method
	if(kH_METH_GET == context->request_method && !NSDK_PROPERTIES(sub_uri)){
		audio_encode_remove_properties(json_dup);
	}
	APP_TRACE("URI : %s", sub_uri);
	if(prefix = "/CHANNELS", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		// get all channels
		sub_uri += strlen(prefix);
		ret = audio_encode_channel(context, sub_uri, json_ref, json_dup, 0, content, content_max);
	}else if(prefix = "/CHANNEL/%d", 1 == sscanf(sub_uri, prefix, &id)){
		if(id > 0){
			ret = sprintf(content, prefix, id);
			sub_uri += ret;

			if(id < 100){
				id += 100;
			}
			//if(H_IS_POST(context->request_header->method) || H_IS_PUT(context->request_header->method)){
			ret = audio_encode_channel(context, sub_uri, json_ref, json_dup, id, content, content_max);
			//}else{

			//}
		}
	}

	// put the both json
	json_object_put(json_dup);
	json_object_put(video);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////////////////////////////

void NETSDK_conf_ain_ch_dump(LP_NSDK_AIN_CH ain_ch)
{
}

static LP_NSDK_AIN_CH netsdk_conf_ain_ch(bool set_flag, int id, LP_NSDK_AIN_CH ain_ch)
{
	char text[128] = {""};
	char *str = NULL;

	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}

	if(ain_ch){
		if(0 == AUDIO_ENTER_CRITICAL()){
			LP_JSON_OBJECT audioJSON = json_object_get(netsdk->audio_conf);
			LP_JSON_OBJECT channelListJSON = NETSDK_json_get_child(audioJSON, "audioInput.audioInputChannel");
			LP_JSON_OBJECT channel = audio_find_channel(channelListJSON, id);

			if(NULL != channel){
				if(set_flag){//set

					str = NETSDK_MAP_DEC2STR(audio_Input_workMode_map,ain_ch->workMode,"input");
					NETSDK_json_set_string2(channel, "workMode", str);

					NETSDK_json_set_int2(channel, "sampleRate", ain_ch->sampleRate);
					NETSDK_json_set_int2(channel, "sampleBitWidth", ain_ch->sampleBitWidth);
					NETSDK_json_set_int2(channel, "inputVolume", ain_ch->inputVolume);
					NETSDK_json_set_int2(channel, "outputVolume", ain_ch->outputVolume);

					str = NETSDK_MAP_DEC2STR(microphoneType_map,ain_ch->microphoneType,"activePickup");
					NETSDK_json_set_string2(channel, "microphoneType", str);

				}else{//get

					ain_ch->id  = NETSDK_json_get_int(channel, "id");
					NETSDK_json_get_string(channel, "workMode", text, sizeof(text));
					ain_ch->workMode  = NETSDK_MAP_STR2DEC(audio_Input_workMode_map, text, kNSDK_AIN_WORK_MODE_INPUT);
					ain_ch->sampleRate = NETSDK_json_get_int(channel, "sampleRate");
					ain_ch->sampleBitWidth = NETSDK_json_get_int(channel, "sampleBitWidth");
					ain_ch->inputVolume = NETSDK_json_get_int(channel, "inputVolume");
					ain_ch->outputVolume = NETSDK_json_get_int(channel, "outputVolume");

					NETSDK_json_get_string(channel, "microphoneType", text, sizeof(text));
					ain_ch->microphoneType = NETSDK_MAP_STR2DEC(microphoneType_map, text, kNSDK_ACTIVE_PICKUP);

				}
			}
			AUDIO_LEAVE_CRITICAL();
			NETSDK_conf_audio_save2();			

			return ain_ch;
		}
	}
	return NULL;

}

LP_NSDK_AIN_CH NETSDK_conf_ain_ch_get(int id, LP_NSDK_AIN_CH ain_ch)
{
	return netsdk_conf_ain_ch(false, id, ain_ch);
}

LP_NSDK_AIN_CH NETSDK_conf_ain_ch_set(int id, LP_NSDK_AIN_CH ain_ch)
{
	return netsdk_conf_ain_ch(true, id, ain_ch);
}

void NETSDK_conf_aenc_ch_dump(LP_NSDK_AENC_CH aenc_ch)
{
}

static LP_NSDK_AENC_CH netsdk_conf_aenc_ch(bool set_flag, int id, LP_NSDK_AENC_CH aenc_ch)
{
	char text[128] = {""};
	char *str = NULL;

	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}

	if(aenc_ch){
		if(0 == AUDIO_ENTER_CRITICAL()){
			LP_JSON_OBJECT audioJSON = json_object_get(netsdk->audio_conf);
			LP_JSON_OBJECT channelListJSON = NETSDK_json_get_child(audioJSON, "audioEncode.audioEncodeChannel");
			LP_JSON_OBJECT channel = audio_find_channel(channelListJSON, id);

			if(NULL != channel){
				if(set_flag){//set
					NETSDK_json_set_boolean2(channel,"enabled", aenc_ch->enabled);
					//NETSDK_json_set_int2(channel, "codecType", aenc_ch->enabled);
					const char *codetype_str = NETSDK_MAP_DEC2STR(audio_type_map, aenc_ch->codecType, "AAC");
					APP_TRACE("set the codetype string to json is %s ", codetype_str);
					NETSDK_json_set_string2(channel, "codecType", codetype_str);
				}else{//get
					aenc_ch->id = NETSDK_json_get_int(channel, "id");
					aenc_ch->enabled = NETSDK_json_get_boolean(channel, "enabled");
					aenc_ch->audioInputChannelID = NETSDK_json_get_int(channel, "audioInputChannelID");
					NETSDK_json_get_string(channel, "codecType", text, sizeof(text));
					aenc_ch->codecType = NETSDK_MAP_STR2DEC(audio_type_map, text, kNSDK_AENC_CODEC_TYPE_G711A);		

				}
			}
			AUDIO_LEAVE_CRITICAL();

            if(true == set_flag)
            {
                NETSDK_conf_audio_save2();
            }

			return aenc_ch;
		}
	}
	return NULL;
}

LP_NSDK_AENC_CH NETSDK_conf_aenc_ch_get(int id, LP_NSDK_AENC_CH aenc_ch)
{
	return netsdk_conf_aenc_ch(false, id, aenc_ch);
}

LP_NSDK_AENC_CH NETSDK_conf_aenc_ch_set(int id, LP_NSDK_AENC_CH aenc_ch)
{
    if(netsdk->audioEncodeChannelChanged) {
        netsdk->audioEncodeChannelChanged(id, aenc_ch);
    }
	return netsdk_conf_aenc_ch(true, id, aenc_ch);
}

int NETSDK_audio_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri, char *content, int content_max)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
		HTTP_CSTR_t prefix = NULL;
	if(prefix = "/INPUT", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		sub_uri += strlen(prefix);
		ret =audio_input_instance(context, sub_uri, content, content_max);
	}else if(prefix = "/ENCODE", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		sub_uri += strlen(prefix);
		ret = audio_encode_instance(context, sub_uri, content, content_max);
	}
	return ret;
}


