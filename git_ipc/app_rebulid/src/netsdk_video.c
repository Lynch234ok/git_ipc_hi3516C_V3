
#include "netsdk.h"
#include "netsdk_util.h"
#include "netsdk_private.h"
#include "generic.h"
#include "sdk/sdk_api.h"
#include "app_overlay.h"
#include "ticker.h"
#include "generic.h"
#include "app_debug.h"
#include "gb2312utf8.h"
#include "sensor.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// NetSDK general opertaions
/////////////////////////////////////////////////////////////////////////////////////////////////

//video
const ST_NSDK_MAP_STR_DEC resolution_map[] = {
	{"2592x1944", kNSDK_RES_2592X1944}, 
	{"2160x2160", kNSDK_RES_2160X2160},
	{"1944x1944", kNSDK_RES_1944X1944}, 	
	{"2592x1520", kNSDK_RES_2592X1520},
	{"2048x1520", kNSDK_RES_2048X1520}, {"1520x1520", kNSDK_RES_1520X1520},
	{"1920x1440", kNSDK_RES_1920X1440}, {"1920x1080", kNSDK_RES_1920X1080},
	{"1280x960", kNSDK_RES_1280X960}, {"1280x720", kNSDK_RES_1280X720},
	{"1024x768", kNSDK_RES_1024X768}, {"1024x576", kNSDK_RES_1024X576},
	{"960x576", kNSDK_RES_960X576}, {"960x480", kNSDK_RES_960X480},
	{"720x576", kNSDK_RES_720X576}, {"720x480", kNSDK_RES_720X480},
	{"640x480", kNSDK_RES_640X480}, {"640x360", kNSDK_RES_640X360},
	{"352x288", kNSDK_RES_352X288}, {"352x240", kNSDK_RES_352X240},
	{"320x240", kNSDK_RES_320X240}, {"320x180", kNSDK_RES_320X180},
	{"176x144", kNSDK_RES_176X144}, {"176x120", kNSDK_RES_176X120},
	{"160x120", kNSDK_RES_160X120}, {"160x90", kNSDK_RES_160X90},
	{"1280x1280", kNSDK_RES_1280X1280},{"1408x1408", kNSDK_RES_1408X1408},
	{"1536x1280", kNSDK_RES_1536X1280},{"1536x1536", kNSDK_RES_1536X1536},
	{"1280x1024", kNSDK_RES_1280X1024},{"960x720", kNSDK_RES_960X720},
	{"1440x1440", kNSDK_RES_1440X1440},{"320x320", kNSDK_RES_320X320},
	{"640x640", kNSDK_RES_640X640},{"1536x784", kNSDK_RES_1536X784},
};

const ST_NSDK_MAP_STR_DEC date_format_map[] = {
	{"YYYY/MM/DD", kNSDK_DATETIME_FMT_SLASH_YYYYMMDD},
	{"MM/DD/YYYY", kNSDK_DATETIME_FMT_SLASH_MMDDYYYY},
	{"DD/MM/YYYY", kNSDK_DATETIME_FMT_SLASH_DDMMYYYY},
	{"YYYY-MM-DD", kNSDK_DATETIME_FMT_DASH_YYYYMMDD},
	{"MM-DD-YYYY", kNSDK_DATETIME_FMT_DASH_MMDDYYYY},
	{"DD-MM-YYYY", kNSDK_DATETIME_FMT_DASH_DDMMYYYY},
};

const ST_NSDK_MAP_STR_DEC bitrate_control_type_map[] = {
	{"CBR", kNSDK_BR_CONTROL_CBR},
	{"VBR", kNSDK_BR_CONTROL_VBR},
};

const ST_NSDK_MAP_STR_DEC profile_type_map[] = {
	{"baseline", kSDK_ENC_H264_PROFILE_BASELINE},
	{"main", kSDK_ENC_H264_PROFILE_MAIN},
	{"high",kSDK_ENC_H264_PROFILE_HIGH},
};

const ST_NSDK_MAP_STR_DEC codec_type[] = {
	{"H.264", kNSDK_CODEC_TYPE_H264},
	{"H.265", kNSDK_CODEC_TYPE_H265},
};

const ST_NSDK_MAP_STR_DEC definition_type[] = {
	{"auto", kNSDK_DEFINITION_AUTO},
	{"fluency", kNSDK_DEFINITION_FLUENCY},
	{"BD", kNSDK_DEFINITION_BD},
	{"HD", kNSDK_DEFINITION_HD},
};


static inline int VIDEO_ENTER_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }
	return pthread_rwlock_wrlock(&netsdk->video_sync);
}

static int VIDEO_LEAVE_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }
	return pthread_rwlock_unlock(&netsdk->video_sync);
}

void NETSDK_conf_video_save()
{
	if(0 == VIDEO_ENTER_CRITICAL()){
		APP_TRACE("NetSDK Video Conf Save!!");
		NETSDK_conf_save(netsdk->video_conf, "video");
		VIDEO_LEAVE_CRITICAL();
	}
}

void NETSDK_conf_video_save2()
{
	if(netsdk->video_conf_save){
		netsdk->video_conf_save(eNSDK_CONF_SAVE_JUST_SAVE, 1);
	}else{
		NETSDK_conf_video_save();
	}
}
///////////////////

static bool motion_detection_get_granularity(struct NSDK_MD_GRID *const _this, int row, int column)
{
	int const offset = _this->columnGranularity * row + column;
	int const offsetByte = offset / (sizeof(_this->granularity[0]) * 8);
	int const offsetBit = offset % (sizeof(_this->granularity[0]) * 8);
	return (_this->granularity[offsetByte] & (1<<offsetBit)) ? true : false;
}

static void motion_detection_set_granularity(struct NSDK_MD_GRID *const _this, int row, int column, bool flag)
{
	int const offset = _this->columnGranularity * row + column;
	int const offsetByte = offset / (sizeof(_this->granularity[0]) * 8);
	int const offsetBit = offset % (sizeof(_this->granularity[0]) * 8);

	if(flag){
		_this->granularity[offsetByte] |= (1<<offsetBit);
	}else{
		_this->granularity[offsetByte] &= ~(1<<offsetBit);
	}
}

static LP_JSON_OBJECT video_find_channel(LP_JSON_OBJECT channels, int id)
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

static void video_input_remove_properties(LP_JSON_OBJECT vin)
{
	int i = 0, ii = 0;
	LP_JSON_OBJECT channels = NETSDK_json_get_child(vin, "videoInputChannel");
	int const n_channels = json_object_array_length(channels);
	
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		LP_JSON_OBJECT privacy_msk = NETSDK_json_get_child(channel, "privacyMask");
		int const n_regions = json_object_array_length(privacy_msk);
	
		for(ii = 0; ii < n_regions; ++ii){
			LP_JSON_OBJECT regin = json_object_array_get_idx(privacy_msk, ii);
			NETSDK_json_remove_properties(regin);
		}
		NETSDK_json_remove_properties(channel);
	}
	
}

static int video_input_channel_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT channelsJSON, LP_JSON_OBJECT formJSON,
	int id, char *content, int contentMax)
{
	int i = 0,ii = 0;
	int netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;

	if(HTTP_IS_GET(context)){
		if(0 == id){
			snprintf(content, contentMax, "%s", json_object_to_json_string(channelsJSON));
			netsdkRet = kNSDK_INS_RET_CONTENT_READY;
		}else if(id > 0){
			LP_JSON_OBJECT channelJSON = video_find_channel(channelsJSON, id);
			if(channelJSON != NULL){
				snprintf(content, contentMax, "%s", json_object_to_json_string(channelJSON));
				netsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}else{
				netsdkRet = kNSDK_INS_RET_INVALID_DOCUMENT;
			}
		}
	}else if(HTTP_IS_PUT(context)){
		LP_JSON_OBJECT channelJSON = video_find_channel(channelsJSON, id);
		
		if(NULL == formJSON){
			netsdkRet = kNSDK_INS_RET_INVALID_DOCUMENT;
		}else{
			//APP_TRACE(json_object_to_json_string(formJSON));	
			// FIXME: lock or enter critical here
			if(0 == id){
				netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
			}else if(id > 0){
				// one channel
				LP_JSON_OBJECT privacyMasksJSON = NETSDK_json_get_child(channelJSON, "privacyMask");
				LP_JSON_OBJECT formPrivacyMasksJSON = NETSDK_json_get_child(formJSON, "privacyMask");
				
				if(channelJSON != NULL){
					NETSDK_json_copy_child(formJSON, channelJSON, "id");
					NETSDK_json_copy_child(formJSON, channelJSON, "powerLineFrequencyMode");
					NETSDK_json_copy_child(formJSON, channelJSON, "brightnessLevel");
					NETSDK_json_copy_child(formJSON, channelJSON, "contrastLevel");
					NETSDK_json_copy_child(formJSON, channelJSON, "sharpnessLevel");
					NETSDK_json_copy_child(formJSON, channelJSON, "saturationLevel");
					NETSDK_json_copy_child(formJSON, channelJSON, "hueLevel");
					NETSDK_json_copy_child(formJSON, channelJSON, "flipEnabled");
					NETSDK_json_copy_child(formJSON, channelJSON, "mirrorEnabled");
					//APP_TRACE("channelJSON:%s", json_object_to_json_string(channelJSON));
					//APP_TRACE("fromJSON:%s", json_object_to_json_string(formJSON));
	
					if(NULL != privacyMasksJSON && NULL != formPrivacyMasksJSON){
						int const privacyMasksLength = json_object_array_length(privacyMasksJSON);
						int const formPrivacyMasksLength = json_object_array_length(formPrivacyMasksJSON);
						
						for(i = 0; i < formPrivacyMasksLength; ++i){
							LP_JSON_OBJECT formPrivacyMaskJSON = json_object_array_get_idx(formPrivacyMasksJSON, i);
							for(ii = 0; ii < privacyMasksLength; ++ii){
								LP_JSON_OBJECT privacyMaskJSON = json_object_array_get_idx(privacyMasksJSON, ii);
								if(NETSDK_json_get_int(formPrivacyMaskJSON,"id") == NETSDK_json_get_int(privacyMaskJSON,"id")){
									//NETSDK_json_copy_child(from_mask,mask,"id");
									NETSDK_json_copy_child(formPrivacyMaskJSON, privacyMaskJSON, "enabled");
									NETSDK_json_copy_child(formPrivacyMaskJSON, privacyMaskJSON, "regionX");
									NETSDK_json_copy_child(formPrivacyMaskJSON, privacyMaskJSON, "regionY");
									NETSDK_json_copy_child(formPrivacyMaskJSON, privacyMaskJSON, "regionWidth");
									NETSDK_json_copy_child(formPrivacyMaskJSON, privacyMaskJSON, "regionHeight");
									NETSDK_json_copy_child(formPrivacyMaskJSON, privacyMaskJSON, "regionColor");
								}
							}
						}
					}

					// setup input with new configure
					if(netsdk->videoInputChannelChanged){
						ST_NSDK_VIN_CH vin_ch;
						int ret_val = 0;
						if(NETSDK_conf_vin_ch_get(id, &vin_ch)){
                            if(0 == VIDEO_ENTER_CRITICAL())
                            {
                                ret_val = netsdk->videoInputChannelChanged(id, &vin_ch);
                                VIDEO_LEAVE_CRITICAL();
                            }
							if(0 == ret_val){
								// save to file
								NETSDK_conf_video_save2();
								// response
								netsdkRet = kNSDK_INS_RET_OK;
							}else{
								netsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
							}
						}
					}else{
						netsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
					}
				}	
			}
		}
	}

	return netsdkRet;
}

static int video_input_channel_instance_privacymask_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	int id, int id_msk, bool properties, char *content, int contentMax)
{
	int i = 0, ii = 0, ret = kNSDK_INS_RET_INVALID_OPERATION;
	//char *json_text = NULL;
	LP_JSON_OBJECT video = json_object_get(netsdk->video_conf);
	LP_JSON_OBJECT videoRefJSON = NETSDK_json_get_child(video, "videoInput");
	LP_JSON_OBJECT videoDupJSON = NULL;
	LP_JSON_OBJECT channels = NULL;
	LP_JSON_OBJECT masks = NULL;

	if(HTTP_IS_GET(context)){
		videoDupJSON = NETSDK_json_dup(videoRefJSON);
		channels = NETSDK_json_get_child(videoDupJSON, "videoInputChannel");
		if(!properties){
			video_input_remove_properties(videoDupJSON);
		}
		if(0 == id){
			ret = kNSDK_INS_RET_INVALID_OPERATION;
		}else if(id > 0){
			LP_JSON_OBJECT channel = video_find_channel(channels,id);
			LP_JSON_OBJECT mask=NULL;
			if(channel != NULL){
				masks = NETSDK_json_get_child(channel,"privacyMask");
				if(0 == id_msk){
					snprintf(content, contentMax, "%s", json_object_to_json_string(masks));
					ret = kNSDK_INS_RET_CONTENT_READY;
				}else if(id_msk > 0){
					mask=video_find_channel(masks,id_msk);
					if(mask != NULL){
						snprintf(content, contentMax, "%s", json_object_to_json_string(mask));
						ret = kNSDK_INS_RET_CONTENT_READY;
					}
				}
			}
		}
		// release json
		json_object_put(videoDupJSON);
		videoDupJSON = NULL;
	}
	else if(kH_METH_POST == context->request_method || kH_METH_PUT == context->request_method){
		LP_JSON_OBJECT from = NETSDK_json_parse(context->request_content);
		channels = NETSDK_json_get_child(videoRefJSON, "videoInputChannel");

		if(NULL != from){
			//APP_TRACE(json_object_to_json_string(from));	
			// FIXME: lock or enter critical here
			if(0 == id){
				ret = kNSDK_INS_RET_INVALID_OPERATION;
			}else if(id > 0){
				LP_JSON_OBJECT channel = video_find_channel(channels,id);
				LP_JSON_OBJECT mask=NULL;
				if(channel != NULL){
					masks = NETSDK_json_get_child(channel, "privacyMask");
					if(0 == id_msk){
						int const privacyMasksLength = json_object_array_length(masks);
						int const formPrivacyMasksLength = json_object_array_length(from);
						LP_JSON_OBJECT from_mask = NULL;
						for(ii = 0; ii < formPrivacyMasksLength; ii++){
							from_mask = json_object_array_get_idx(from, ii);
							for(i = 0; i < privacyMasksLength; ++i){
								LP_JSON_OBJECT mask = json_object_array_get_idx(masks, i);
								if(NETSDK_json_get_int(from_mask,"id")==NETSDK_json_get_int(mask,"id")){
									//NETSDK_json_copy_child(from_mask,mask,"id");
									NETSDK_json_copy_child(from_mask,mask,"enabled");
									NETSDK_json_copy_child(from_mask,mask,"regionX");
									NETSDK_json_copy_child(from_mask,mask,"regionY");
									NETSDK_json_copy_child(from_mask,mask,"regionWidth");
									NETSDK_json_copy_child(from_mask,mask,"regionHeight");
									NETSDK_json_copy_child(from_mask,mask,"regionColor");
								}
							}
						}
						
					}else if(id_msk > 0){
						mask=video_find_channel(masks,id_msk);
						if(mask != NULL){
							//NETSDK_json_copy_child(from,mask,"id");
							NETSDK_json_copy_child(from,mask,"enabled");
							NETSDK_json_copy_child(from,mask,"regionX");
							NETSDK_json_copy_child(from,mask,"regionY");
							NETSDK_json_copy_child(from,mask,"regionWidth");
							NETSDK_json_copy_child(from,mask,"regionHeight");
							NETSDK_json_copy_child(from,mask,"regionColor");
								
						}
					}
					// setup input with new configure
					if(netsdk->videoInputChannelChanged){
						ST_NSDK_VIN_CH vin_ch;
						int ret_val = 0;
						if(NETSDK_conf_vin_ch_get(id, &vin_ch)){
                            if(0 == VIDEO_ENTER_CRITICAL())
                            {
                                ret_val = netsdk->videoInputChannelChanged(id, &vin_ch);
                                VIDEO_LEAVE_CRITICAL();
                            }
							if(0 == ret_val){
								// save to file
								NETSDK_conf_video_save2();
								// response
								ret = kNSDK_INS_RET_OK;
							}else{
								ret = kNSDK_INS_RET_DEVICE_ERROR;
							}
						}
					}else{
						ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
					}

						
				}				
			}
			json_object_put(from);
			from = NULL;
		}else{
			ret = kNSDK_INS_RET_INVALID_DOCUMENT;
		}
	}

	return ret;
}

static int video_input_channel_feature_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT videoInputChannelJSON, LP_JSON_OBJECT formJSON,
	const char *featureKey, char *content, int contentMax)
{
	int netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	LP_JSON_OBJECT featureJSON = NETSDK_json_get_child(videoInputChannelJSON, featureKey);
	if(NULL != featureJSON){
		if(HTTP_IS_GET(context)){
			snprintf(content, contentMax, "%s", json_object_to_json_string(featureJSON));
			netsdkRet = kNSDK_INS_RET_CONTENT_READY;
		}else{
			int featureValue = json_object_get_int(formJSON);
			NETSDK_json_set_int2(videoInputChannelJSON, featureKey, featureValue);
			netsdkRet = kNSDK_INS_RET_OK;
		}
	}
	return netsdkRet;
}


/*
/NetSDK/Video/input/channels
/NetSDK/Video/input/channels/properties
/NetSDK/Video/input/channel/ID
/NetSDK/Video/input/channel/ID/properties
/NetSDK/Video/input/channel/ID/PrivacyMasks
/NetSDK/Video/input/channel/ID/PrivacyMasks/properties
/NetSDK/Video/input/channel/ID/PrivacyMask/ID
/NetSDK/Video/input/channel/ID/PrivacyMask/ID/properties
*/
static int video_input_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT inputRefJSON, LP_JSON_OBJECT inputDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int id = 0, id_msk = 0;
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;
	LP_JSON_OBJECT channelsJSON = NULL;

	if(HTTP_IS_GET(context)){
		channelsJSON = NETSDK_json_get_child(inputDupJSON, "videoInputChannel");
	}else{
		channelsJSON = NETSDK_json_get_child(inputRefJSON, "videoInputChannel");
	}

	// check properties necessary when GET method
	if(HTTP_IS_GET(context) && !NSDK_PROPERTIES(subURI)){
		video_input_remove_properties(inputDupJSON);
	}
	
	if(prefix = "/CHANNELS", 0 == strncmp(prefix, subURI, strlen(prefix))){
		// get all channels
		subURI += strlen(prefix);
		ret = video_input_channel_instance(context, subURI, channelsJSON, formJSON, 0, content, contentMax);
		
	}else if(prefix = "/CHANNEL/%d", 1 == sscanf(subURI, prefix, &id)){
		LP_JSON_OBJECT channelJSON = video_find_channel(channelsJSON, id);
		subURI += snprintf(content, sizeof(content), prefix, id);
		//APP_TRACE("URI : %s", subURI);

		if(prefix = "/BRIGHTNESSLEVEL", 0 == strncmp(subURI, prefix, strlen(prefix))){
			ret = video_input_channel_feature_instance(context, subURI, channelJSON, formJSON, "brightnessLevel", content, contentMax);
			if(kNSDK_INS_RET_OK == ret){
				ret = video_input_channel_instance(context, subURI, channelsJSON, channelJSON, id, content, contentMax);
			}
		}else if(prefix = "/SHARPNESSLEVEL", 0 == strncmp(subURI, prefix, strlen(prefix))){
			ret = video_input_channel_feature_instance(context, subURI, channelJSON, formJSON, "sharpnessLevel", content, contentMax);
			if(kNSDK_INS_RET_OK == ret){
				ret = video_input_channel_instance(context, subURI, channelsJSON, channelJSON, id, content, contentMax);
			}
		}else if(prefix = "/SATURATIONLEVEL", 0 == strncmp(subURI, prefix, strlen(prefix))){
			ret = video_input_channel_feature_instance(context, subURI, channelJSON, formJSON, "saturationLevel", content, contentMax);
			if(kNSDK_INS_RET_OK == ret){
				ret = video_input_channel_instance(context, subURI, channelsJSON, channelJSON, id, content, contentMax);
			}
		}else if(prefix = "/HUELEVEL", 0 == strncmp(subURI, prefix, strlen(prefix))){
			ret = video_input_channel_feature_instance(context, subURI, channelJSON, formJSON, "hueLevel", content, contentMax);
			if(kNSDK_INS_RET_OK == ret){
				ret = video_input_channel_instance(context, subURI, channelsJSON, channelJSON, id, content, contentMax);
			}
		}else if(prefix = "/CONTRASTLEVEL", 0 == strncmp(subURI, prefix, strlen(prefix))){
			ret = video_input_channel_feature_instance(context, subURI, channelJSON, formJSON, "contrastLevel", content, contentMax);
			if(kNSDK_INS_RET_OK == ret){
				ret = video_input_channel_instance(context, subURI, channelsJSON, channelJSON, id, content, contentMax);
			}
		}else if(prefix = "/PRIVACYMASKS", 0 == strncmp(subURI, prefix, strlen(prefix))){
			subURI += strlen(prefix);
			if(strlen(subURI) == 0){
				ret = video_input_channel_instance_privacymask_instance(context, subURI, id, 0, false, content, contentMax);
			}else if(prefix = "/PROPERTIES", 0 == strncmp(subURI, prefix, strlen(prefix))){
				ret = video_input_channel_instance_privacymask_instance(context, subURI, id, 0, true, content, contentMax);
			}
		}else if(prefix = "/PRIVACYMASK", 0 == strncmp(subURI, prefix, strlen(prefix))){
			subURI += strlen(prefix);
			if(1 == sscanf(subURI, "/%d", &id_msk)){
				subURI += sprintf(content, "/%d", id_msk);
				APP_TRACE("URI : %s", subURI);
				if(strlen(subURI) == 0){
					ret = video_input_channel_instance_privacymask_instance(context, subURI, id, id_msk, false, content, contentMax);
				}else if(prefix = "/PROPERTIES", 0 == strncmp(subURI, prefix, strlen(prefix))){
					ret = video_input_channel_instance_privacymask_instance(context, subURI, id, id_msk, true, content, contentMax);
				}
			}
		}else{
			ret = video_input_channel_instance(context, subURI, channelsJSON, formJSON, id, content, contentMax);
		}
	}
	
	return ret;
}

static bool video_md_get_granularity(LP_JSON_OBJECT obj, int row, int column)
{
	bool flag = false;
	if(NULL != obj){
		LP_JSON_OBJECT rowsJSON = json_object_get(obj);
		LP_JSON_OBJECT rowJSON = json_object_array_get_idx(rowsJSON, row);
		if(NULL != rowJSON){
			LP_JSON_OBJECT columnsJSON = json_object_get(rowJSON);
			LP_JSON_OBJECT columnJSON = json_object_array_get_idx(columnsJSON, column);
			//APP_TRACE("%s", json_object_to_json_string(columnJSON));
			flag = json_object_get_boolean(columnJSON);
			json_object_put(columnsJSON);
			columnsJSON = NULL;
		}
		json_object_put(rowsJSON);
		rowsJSON = NULL;
	}
	return flag;
}

static void video_md_set_granularity(LP_JSON_OBJECT obj, int row, int column, bool flag)
{
	if(NULL != obj){
		LP_JSON_OBJECT rowsJSON = json_object_get(obj);
		LP_JSON_OBJECT rowJSON = json_object_array_get_idx(rowsJSON, row);
		if(NULL != rowJSON){
			LP_JSON_OBJECT columnsJSON = json_object_get(rowJSON);
			LP_JSON_OBJECT columnJSON = json_object_array_get_idx(columnsJSON, column);
			if(NULL != columnJSON){
				columnJSON = json_object_new_boolean(flag ? TRUE : FALSE);
				json_object_array_put_idx(columnsJSON, column, columnJSON);
			}
			json_object_put(columnsJSON);
			columnsJSON = NULL;
		}
		json_object_put(rowsJSON);
		rowsJSON = NULL;
	}
}


static int video_md_copy_grid_granularity(LP_JSON_OBJECT srcJSON, LP_JSON_OBJECT dstJSON)
{
	//unsigned int array[64];
	int i = 0, ii = 0;

	if(NULL != srcJSON && NULL != dstJSON){
		LP_JSON_OBJECT srcRowsJSON = json_object_get(srcJSON);
		LP_JSON_OBJECT dstRowsJSON = json_object_get(dstJSON);
		int const srcRowLen = json_object_array_length(srcRowsJSON);
		int const dstRowLen = json_object_array_length(dstRowsJSON);

		if(srcRowLen == dstRowLen){
			for(i = 0; i < srcRowLen; ++i){
				LP_JSON_OBJECT srcColumnsJSON = json_object_array_get_idx(srcRowsJSON, i);
				LP_JSON_OBJECT dstColumnsJSON = json_object_array_get_idx(dstRowsJSON, i);
				int const srcColumnLen = json_object_array_length(srcColumnsJSON);
				int const dstColumnLen = json_object_array_length(dstColumnsJSON);
				
				if(srcColumnLen == dstColumnLen){
					for(ii = 0; ii < srcColumnLen; ++ii){
						// get one item flag
						boolean flag = json_object_get_boolean(json_object_array_get_idx(srcColumnsJSON, ii));
						LP_JSON_OBJECT flagJSON = json_object_new_boolean(flag);
						json_object_array_put_idx(dstColumnsJSON, ii, flagJSON);
						
						flag = json_object_get_boolean(json_object_array_get_idx(dstColumnsJSON, ii));
						printf("%c", flag ? '#' : '.');
					}
				}
				printf("\r\n");
			}
		}
		json_object_put(srcRowsJSON);
		srcRowsJSON = NULL;
		json_object_put(dstRowsJSON);
		dstRowsJSON = NULL;
	}
	return 0;
}

static int video_md_channel(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT videoRefJSON, LP_JSON_OBJECT videoDupJSON, LP_JSON_OBJECT formJSON,
	int id, char *content, int contentMax)
{
	int i = 0, ii = 0, netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	char *json_text = NULL;
	char md_type[16];
	char text[128] = {""};
	

	if(HTTP_IS_GET(context)){
		LP_JSON_OBJECT channelsJSON = NETSDK_json_get_child(videoDupJSON, "motionDetectionChannel");
		
		if(0 == id){
			// get all the channels attributes
			snprintf(content, contentMax, "%s", json_object_to_json_string(channelsJSON));
			netsdkRet = kNSDK_INS_RET_CONTENT_READY;
		}else if(id > 0){
			LP_JSON_OBJECT channelJSON = video_find_channel(channelsJSON, id);
			if(channelJSON != NULL){
				snprintf(content, contentMax, "%s", json_object_to_json_string(channelJSON));
				netsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}
		}
	}else if(HTTP_IS_PUT(context)){
		if(NULL != formJSON){
			LP_JSON_OBJECT channelsJSON = NETSDK_json_get_child(videoRefJSON, "motionDetectionChannel");
			//APP_TRACE(json_object_to_json_string(formJSON));	
			// FIXME: lock or enter critical here
			if(0 == id){
				netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
			}else if(id > 0){
				LP_JSON_OBJECT channelJSON = video_find_channel(channelsJSON, id);
				
				if(channelJSON != NULL){
					// form elements
					LP_JSON_OBJECT formEnabledJSON = NETSDK_json_get_child(formJSON, "enabled");
					LP_JSON_OBJECT formDetectionTypeJSON = NETSDK_json_get_child(formJSON, "detectionType");
					LP_JSON_OBJECT formRegionsJSON = NETSDK_json_get_child(formJSON, "detectionRegion");
					LP_JSON_OBJECT formGridJSON = NETSDK_json_get_child(formJSON, "detectionGrid");
					// local elements
					LP_JSON_OBJECT regionsJSON = NETSDK_json_get_child(channelJSON,"detectionRegion");
					LP_JSON_OBJECT gridJSON = NETSDK_json_get_child(channelJSON,"detectionGrid");

					if(0 == VIDEO_ENTER_CRITICAL()){
						// marked it
						NETSDK_json_copy_child(formJSON, channelJSON, "enabled");
						NETSDK_json_copy_child(formJSON,channelJSON, "detectionType");

						// region mode setup
						if(NULL != formRegionsJSON){
							int const regionsLength = json_object_array_length(regionsJSON);
							int const formRegionsLength = json_object_array_length(formRegionsJSON);
							for(i = 0; i < formRegionsLength; ++i){
								LP_JSON_OBJECT formRegionJSON = json_object_array_get_idx(formRegionsJSON, i);
								for(ii = 0; ii < regionsLength; ++ii){
									LP_JSON_OBJECT regionJSON = json_object_array_get_idx(regionsJSON, ii);
									if(NETSDK_json_get_int(formRegionJSON,"id") == NETSDK_json_get_int(regionJSON,"id")){
										NETSDK_json_copy_child(formRegionJSON, regionJSON, "enabled");
										NETSDK_json_copy_child(formRegionJSON, regionJSON, "regionX");
										NETSDK_json_copy_child(formRegionJSON, regionJSON, "regionY");
										NETSDK_json_copy_child(formRegionJSON, regionJSON, "regionWidth");
										NETSDK_json_copy_child(formRegionJSON, regionJSON, "regionHeight");
										NETSDK_json_copy_child(formRegionJSON, regionJSON, "sensitivityLevel");
									}
								}
							}
						}

						// grid mode setup
						if(NULL != formGridJSON){
							LP_JSON_OBJECT granularityJSON = NETSDK_json_get_child(gridJSON, "granularity");
							LP_JSON_OBJECT formGranularityJSON = NETSDK_json_get_child(formGridJSON, "granularity");

							//APP_TRACE("Source Granularity\r\n%s", json_object_to_json_string(formGranularityJSON));
							//APP_TRACE("Destination Granularity\r\n%s", json_object_to_json_string(granularityJSON));

							// mark sensitivity
							NETSDK_json_copy_child(formGridJSON, gridJSON, "sensitivityLevel");
							video_md_copy_grid_granularity(formGranularityJSON, granularityJSON);
							
							//APP_TRACE(json_object_to_json_string(grid));	
						}
						VIDEO_LEAVE_CRITICAL();
					}
						
					// setup input with new configure
					if(netsdk->motionDetectionChannelChanged){
						ST_NSDK_MD_CH md_ch;
						int ret_val = 0;
						if(NETSDK_conf_md_ch_get(id, &md_ch)){
                            if(0 == VIDEO_ENTER_CRITICAL())
                            {
                                ret_val = netsdk->motionDetectionChannelChanged(id, &md_ch);
                                VIDEO_LEAVE_CRITICAL(); 
                            }
							if(0 == ret_val){
								// save to file
								// response
								netsdkRet = kNSDK_INS_RET_OK;
							}else{
								netsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
							}
						}
					}else{
						netsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
					}
					
					if(kNSDK_INS_RET_OK == netsdkRet){
						NETSDK_conf_video_save2();
					}			
				}
			}
		}else{
			netsdkRet = kNSDK_INS_RET_INVALID_DOCUMENT;
		}
	}

	return netsdkRet;
}

/*
/NetSDK/Video/MotionDetection/channels
/NetSDK/Video/MotionDetection/channels/properties
/NetSDK/Video/MotionDetection/channel/ID
/NetSDK/Video/MotionDetection/channel/ID/properties
/NetSDK/Video/MotionDetection/channel/ID/detectionGrid
/NetSDK/Video/MotionDetection/channel/ID/detectionGrid/properties
/NetSDK/Video/MotionDetection/channel/ID/detectionRegions
/NetSDK/Video/MotionDetection/channel/ID/detectionRegions/properties
/NetSDK/Video/MotionDetection/channel/ID/detectionRegion/ID
/NetSDK/Video/MotionDetection/channel/ID/detectionRegion/ID/properties
*/

static void video_md_remove_properties(LP_JSON_OBJECT md)
{
	int i = 0, ii = 0;
	LP_JSON_OBJECT channels = NETSDK_json_get_child(md, "motionDetectionChannel");
	int const n_channels = json_object_array_length(channels);
	
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		LP_JSON_OBJECT md_grid = NETSDK_json_get_child(channel, "detectionGrid");
		LP_JSON_OBJECT md_region = NETSDK_json_get_child(channel, "detectionRegion");
		int const n_regions = json_object_array_length(md_region);
	
		for(ii = 0; ii < n_regions; ++ii){
			LP_JSON_OBJECT regin = json_object_array_get_idx(md_region, ii);
			NETSDK_json_remove_properties(regin);
		}
		NETSDK_json_remove_properties(md_grid);
		NETSDK_json_remove_properties(channel);
	}
}

static int video_motiondetecion_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT mdRefJSON, LP_JSON_OBJECT mdDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int channelID = 0, netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;

	if(HTTP_IS_GET(context) && !NSDK_PROPERTIES(subURI)){
		video_md_remove_properties(mdDupJSON);
	}

	//APP_TRACE("URI : %s", subURI);
	if(prefix = "/CHANNELS", 0 == strncmp(prefix, subURI, strlen(prefix))){
		// get all channels
		subURI += strlen(prefix);
		netsdkRet = video_md_channel(context, subURI, mdRefJSON, mdDupJSON, formJSON, 0, content, contentMax);
	}else if(prefix = "/CHANNEL/%d", 1 == sscanf(subURI, prefix, &channelID)){
		if(channelID > 0){
			subURI += sprintf(content, prefix, channelID);
			//APP_TRACE("URI : %s", subURI);

			if(prefix = "/STATUS", 0 == strncmp(subURI, prefix, strlen(prefix))){
				if(netsdk->motionDetectionChannelStatus){
					if(HTTP_IS_GET(context)){
						if(netsdk->motionDetectionChannelStatus(channelID, false)){
							snprintf(content,contentMax, "%s", "true");
						}else{
							snprintf(content,contentMax, "%s", "false");
						}
						netsdkRet = kNSDK_INS_RET_CONTENT_READY;
						
					}else if(HTTP_IS_DELETE(context)){
						netsdk->motionDetectionChannelStatus(channelID, true);
						netsdkRet = kNSDK_INS_RET_OK;
					}else{
						netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
					}
				}else{
					netsdkRet = kNSDK_INS_RET_UNKNOWN_ERROR;
				}
			}else{
				netsdkRet = video_md_channel(context, subURI, mdRefJSON, mdDupJSON, formJSON, channelID, content, contentMax);
			}
		}
	}
	return netsdkRet;
}

/*
/NetSDK/Video/encode/channels
/NetSDK/Video/encode/channels/ID
/NetSDK/Video/encode/channels/ID/properties
/NetSDK/Video/encode/channels/ID/snapShot
/NetSDK/Video/encode/channels/ID/requestKeyFrame
/NetSDK/Video/encode/channels/ID/channelNameOverlay
/NetSDK/Video/encode/channels/ID/channelNameOverlay/properties
/NetSDK/Video/encode/channels/ID/datetimeOverlay
/NetSDK/Video/encode/channels/ID/datetimeOverlay/properties
/NetSDK/Video/encode/channels/ID/textOverlay
/NetSDK/Video/encode/channels/ID/textOverlay/properties
/NetSDK/Video/encode/channels/ID/textOverlay/ID
/NetSDK/Video/encode/channels/ID/textOverlay/ID/properties
*/

static void video_encode_remove_properties(LP_JSON_OBJECT venc)
{
	int i = 0, ii = 0;
	LP_JSON_OBJECT channels = NETSDK_json_get_child(venc, "videoEncodeChannel");
	int const n_channels = json_object_array_length(channels);
	
	for(i = 0; i < n_channels; ++i){
		LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
		LP_JSON_OBJECT channel_name_overlay = NETSDK_json_get_child(channel, "channelNameOverlay");
		LP_JSON_OBJECT datetime_overlay = NETSDK_json_get_child(channel, "datetimeOverlay");
		LP_JSON_OBJECT deviceid_overlay = NETSDK_json_get_child(channel, "deviceIDOverlay");
		LP_JSON_OBJECT text_overlay = NETSDK_json_get_child(channel, "textOverlay");
		int const n_region = json_object_array_length(text_overlay);

		for(ii = 0; ii < n_region; ++ii){
			LP_JSON_OBJECT region = json_object_array_get_idx(text_overlay, ii);
			NETSDK_json_remove_properties(region);
		}

		NETSDK_json_remove_properties(deviceid_overlay);
		NETSDK_json_remove_properties(datetime_overlay);
		NETSDK_json_remove_properties(channel_name_overlay);
		NETSDK_json_remove_properties(channel);
	}
}

static int video_encode_channel_limit_check(LP_JSON_OBJECT channelJSON, uint32_t soc_inception)
{
#define HI3518E_MAX_ABILITY (1280*720*25)
#define HI3518C_MAX_ABILITY (1280*960*25)
#define HI3516C_MAX_ABILITY (1920*1080*30)
#define HI3518EV2_MAX_ABILITY (1920*1080*15)
#define HI3516CV2_MAX_ABILITY (1920*1080*35)

	char text[128] = {""};
	int limit_framerate, max_framerate;
	ST_NSDK_VENC_CH venc_ch;
	NETSDK_json_get_string(channelJSON, "resolution", text, sizeof(text));
	venc_ch.resolution = NETSDK_MAP_STR2DEC(resolution_map,text,kNSDK_RES_640X480);
	venc_ch.freeResolution = NETSDK_json_get_boolean(channelJSON, "freeResolution");
	if(venc_ch.freeResolution){
		// user free resolution
		venc_ch.resolutionWidth = NETSDK_json_get_int(channelJSON, "resolutionWidth");
		venc_ch.resolutionHeight = NETSDK_json_get_int(channelJSON, "resolutionHeight");
	}else{
		venc_ch.resolutionWidth = (venc_ch.resolution >> 16) & 0xffff;
		venc_ch.resolutionHeight = (venc_ch.resolution >> 0) & 0xffff;
	} 

	if(SDK_HI3518C_V100 == soc_inception || SDK_HI3518A_V100 == soc_inception){//3518C&3518A
		limit_framerate = HI3518C_MAX_ABILITY/(venc_ch.resolutionWidth*venc_ch.resolutionHeight);		
		LP_JSON_OBJECT propertyJSON = NETSDK_json_get_child(channelJSON, "frameRateProperty");
		if(NULL != propertyJSON){
			max_framerate = NETSDK_json_get_int(propertyJSON, "max");
			if(limit_framerate <= max_framerate){
				max_framerate = limit_framerate;
			}else{
				max_framerate = (limit_framerate > 30)? 30 : limit_framerate;
			}
			NETSDK_json_set_int2(propertyJSON, "max", max_framerate);			
			venc_ch.frameRate = NETSDK_json_get_int(channelJSON, "frameRate");
			if(venc_ch.frameRate > max_framerate){		
				NETSDK_json_set_int2(channelJSON, "frameRate", max_framerate);	
			}
			//APP_TRACE("%s--%d", json_object_to_json_string(propertyJSON), venc_ch.frameRate);
		}		
	}else if(SDK_HI3518E_V100 == soc_inception){//3518E
		limit_framerate = HI3518E_MAX_ABILITY/(venc_ch.resolutionWidth*venc_ch.resolutionHeight);	
		if(limit_framerate > 15 && limit_framerate < 25){
			//it must be 960P
			limit_framerate = 15;
		}
		LP_JSON_OBJECT propertyJSON = NETSDK_json_get_child(channelJSON, "frameRateProperty");
		if(NULL != propertyJSON){
			max_framerate = NETSDK_json_get_int(propertyJSON, "max");
			if(limit_framerate <= max_framerate){
				max_framerate = limit_framerate;
			}else{
				max_framerate = (limit_framerate > 25)? 25 : limit_framerate;
			}
			NETSDK_json_set_int2(propertyJSON, "max", max_framerate);			
			venc_ch.frameRate = NETSDK_json_get_int(channelJSON, "frameRate");
			if(venc_ch.frameRate > max_framerate){		
				NETSDK_json_set_int2(channelJSON, "frameRate", max_framerate);	
			}
			//APP_TRACE("%s--%d", json_object_to_json_string(propertyJSON), venc_ch.frameRate);
		}		
	}else if(SDK_HI3516C_V200 == soc_inception){
		limit_framerate = HI3516CV2_MAX_ABILITY/(venc_ch.resolutionWidth*venc_ch.resolutionHeight);
		LP_JSON_OBJECT propertyJSON = NETSDK_json_get_child(channelJSON, "frameRateProperty");
		if(NULL != propertyJSON){
			max_framerate = NETSDK_json_get_int(propertyJSON, "max");
			if(limit_framerate <= max_framerate){
				max_framerate = limit_framerate;
			}else{
				max_framerate = (limit_framerate > 30)? 30 : limit_framerate;
			}
			NETSDK_json_set_int2(propertyJSON, "max", max_framerate);			
			venc_ch.frameRate = NETSDK_json_get_int(channelJSON, "frameRate");
			if(venc_ch.frameRate > max_framerate){		
				NETSDK_json_set_int2(channelJSON, "frameRate", max_framerate);	
			}
			//APP_TRACE("%s--%d", json_object_to_json_string(propertyJSON), venc_ch.frameRate);
		}		
	}else if(SDK_HI3518E_V200 == soc_inception || SDK_HI3516E_V100 == soc_inception){
		limit_framerate = HI3518EV2_MAX_ABILITY/(venc_ch.resolutionWidth*venc_ch.resolutionHeight);
		LP_JSON_OBJECT propertyJSON = NETSDK_json_get_child(channelJSON, "frameRateProperty");
		if(NULL != propertyJSON){
			max_framerate = NETSDK_json_get_int(propertyJSON, "max");
			if(limit_framerate <= max_framerate){
				max_framerate = limit_framerate;
			}else{
				max_framerate = (limit_framerate > 30)? 30 : limit_framerate;
			}
			NETSDK_json_set_int2(propertyJSON, "max", max_framerate);			
			venc_ch.frameRate = NETSDK_json_get_int(channelJSON, "frameRate");
			if(venc_ch.frameRate > max_framerate){		
				NETSDK_json_set_int2(channelJSON, "frameRate", max_framerate);	
			}
			//APP_TRACE("%s--%d", json_object_to_json_string(propertyJSON), venc_ch.frameRate);
		}		
	}else if(SDK_M388C1G == soc_inception){//m388c2g/m388c1g
		//do something
	}
	else{//another SOC

	}

	venc_ch.ImageTransmissionModel = NETSDK_json_get_int(channelJSON, "ImageTransmissionModel");
	if(eNSDK_COMPATIBILITY_MODE == venc_ch.ImageTransmissionModel){
		APP_TRACE("Set the gop 2 times of fps");
		venc_ch.keyFrameInterval = venc_ch.frameRate*2;
		NETSDK_json_set_int2(channelJSON, "keyFrameInterval", venc_ch.keyFrameInterval);
	}
	else if(eNSDK_LOW_BPS_MODEL == venc_ch.ImageTransmissionModel){
		APP_TRACE("Set the gop 10 times of fps");
		venc_ch.keyFrameInterval = venc_ch.frameRate*10;
		NETSDK_json_set_int2(channelJSON, "keyFrameInterval", venc_ch.keyFrameInterval);
	}

	return 0;
}

static int video_encode_channel(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT encRefJSON, LP_JSON_OBJECT encDupJSON, LP_JSON_OBJECT formJSON, int id, char *content, int contentMax)
{
	int i = 0, ii = 0, netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	char text[128] = {""};

	if(HTTP_IS_GET(context)){
		LP_JSON_OBJECT channelsJSON = NETSDK_json_get_child(encDupJSON, "videoEncodeChannel");
        const char * userAgent = NULL;
		userAgent = context->request_header->read_tag(context->request_header, "User-Agent");
		if(userAgent == NULL || !strcmp(userAgent, "HTTP_USER_AGENT")){//fix hikvision
			LP_JSON_OBJECT mainChannelJSON = video_find_channel(channelsJSON, 101);
			char channelNameUtf8[64],channelNameGb2312[64];
			NETSDK_json_get_string(mainChannelJSON, "channelName", channelNameUtf8, sizeof(channelNameUtf8));
			utf8_to_gb2312(channelNameUtf8, channelNameGb2312,sizeof(channelNameGb2312));
			NETSDK_json_set_string2(mainChannelJSON, "channelName", channelNameGb2312);
		}else{
			
		}
		if(0 == id){
			snprintf(content, contentMax, "%s", json_object_to_json_string(channelsJSON));
			netsdkRet = kNSDK_INS_RET_CONTENT_READY;
		}else if(id > 0){
			LP_JSON_OBJECT channelJSON = video_find_channel(channelsJSON, id);
			if(NULL != channelJSON){
				snprintf(content, contentMax, "%s", json_object_to_json_string(channelJSON));
				netsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}
		}
		
	}else if(HTTP_IS_PUT(context)){
		if(0 == VIDEO_ENTER_CRITICAL()){
			LP_JSON_OBJECT channels = NETSDK_json_get_child(encRefJSON, "videoEncodeChannel");
			//APP_TRACE("Content : %s", context->request_content);
			
			if(!formJSON){
				netsdkRet = kNSDK_INS_RET_INVALID_DOCUMENT;
			}else{	
				// FIXME: lock or enter critical here
				if(0 == id){
					netsdkRet = kNSDK_INS_RET_INVALID_DOCUMENT;
				}else if(id > 0){
					LP_JSON_OBJECT channelJSON = video_find_channel(channels, id);
					if(NULL != channelJSON){
						LP_JSON_OBJECT channelNameOverlayJSON = NETSDK_json_get_child(channelJSON, "channelNameOverlay");
						LP_JSON_OBJECT datetimeOverlayJSON = NETSDK_json_get_child(channelJSON, "datetimeOverlay");
						LP_JSON_OBJECT deviceIDOverlayJSON = NETSDK_json_get_child(channelJSON, "deviceIDOverlay");
						LP_JSON_OBJECT textOverlaysJSON = NETSDK_json_get_child(channelJSON, "textOverlay");

						LP_JSON_OBJECT formChannelNameOverlayJSON = NETSDK_json_get_child(formJSON, "channelNameOverlay");
						LP_JSON_OBJECT formDatetimeOverlayJSON = NETSDK_json_get_child(formJSON, "datetimeOverlay");
						LP_JSON_OBJECT formDeviceIDOverlayJSON = NETSDK_json_get_child(formJSON, "deviceIDOverlay");
						LP_JSON_OBJECT formTextOverlaysJSON = NETSDK_json_get_child(formJSON, "textOverlay");
						LP_JSON_OBJECT formchannelNameJSON =NETSDK_json_get_child(formJSON, "channelName");
						
						// jso -> channel
						NETSDK_json_copy_child(formJSON, channelJSON, "id");
						NETSDK_json_copy_child(formJSON, channelJSON, "enabled");
						NETSDK_json_copy_child(formJSON, channelJSON, "videoInputChannelID");
						NETSDK_json_copy_child(formJSON, channelJSON, "codecType");
						NETSDK_json_copy_child(formJSON, channelJSON, "h264Profile");
						NETSDK_json_copy_child(formJSON, channelJSON, "resolution");
						NETSDK_json_copy_child(formJSON, channelJSON, "freeResolution");
						NETSDK_json_copy_child(formJSON, channelJSON, "resolutionWidth");
						NETSDK_json_copy_child(formJSON, channelJSON, "resolutionHeight");
						NETSDK_json_copy_child(formJSON, channelJSON, "bitRateControlType");
						NETSDK_json_copy_child(formJSON, channelJSON, "constantBitRate");
						NETSDK_json_copy_child(formJSON, channelJSON, "frameRate");
						NETSDK_json_copy_child(formJSON, channelJSON, "keyFrameInterval");
						NETSDK_json_copy_child(formJSON, channelJSON, "snapShotImageType");
						NETSDK_json_copy_child(formJSON, channelJSON, "definitionType");
                        NETSDK_json_copy_child(formJSON, channelJSON, "ImageTransmissionModel");
						//fix resolution and fps limit in different soc
						uint32_t chip_id;
						if(!sdk_sys){
							APP_TRACE("sdk_sys not init!");							
							return kNSDK_INS_RET_DEVICE_ERROR;
						}
						sdk_sys->get_chip_id(&chip_id);
						video_encode_channel_limit_check(channelJSON, chip_id);

						if(NULL != formchannelNameJSON){
							char channelNameUtf8[64],channelNameGb2312[64];
							NETSDK_json_get_string(formJSON, "channelName", channelNameGb2312, sizeof(channelNameGb2312));
							if(strlen(channelNameGb2312) > 0){
								NETSDK_json_copy_child(formJSON, channelJSON, "channelName");
								NETSDK_json_get_string(channelJSON, "channelName", channelNameGb2312, sizeof(channelNameGb2312));						
								if(strlen(channelNameGb2312) > 0){
									if(gb2312_to_utf8(0, channelNameGb2312, channelNameUtf8, sizeof(channelNameUtf8))){
										NETSDK_json_set_string2(channelJSON, "channelName", channelNameUtf8);
									}else{
										//do nothing
									}		
								}	
							}
						}

						if(NULL != formChannelNameOverlayJSON && NULL != channelNameOverlayJSON){
							NETSDK_json_copy_child(formChannelNameOverlayJSON, channelNameOverlayJSON, "enabled");
							NETSDK_json_copy_child(formChannelNameOverlayJSON, channelNameOverlayJSON, "regionX");
							NETSDK_json_copy_child(formChannelNameOverlayJSON, channelNameOverlayJSON, "regionY");
						}
						if(NULL != formDatetimeOverlayJSON && NULL != datetimeOverlayJSON){
							NETSDK_json_copy_child(formDatetimeOverlayJSON, datetimeOverlayJSON, "enabled");
							NETSDK_json_copy_child(formDatetimeOverlayJSON, datetimeOverlayJSON, "regionX");
							NETSDK_json_copy_child(formDatetimeOverlayJSON, datetimeOverlayJSON, "regionY");
							NETSDK_json_copy_child(formDatetimeOverlayJSON, datetimeOverlayJSON, "dateFormat");
							NETSDK_json_copy_child(formDatetimeOverlayJSON, datetimeOverlayJSON, "timeFormat");
							NETSDK_json_copy_child(formDatetimeOverlayJSON, datetimeOverlayJSON, "displayWeek");
						}
						if(NULL != formDeviceIDOverlayJSON && NULL != deviceIDOverlayJSON){
							NETSDK_json_copy_child(formDeviceIDOverlayJSON, deviceIDOverlayJSON, "enabled");
							NETSDK_json_copy_child(formDeviceIDOverlayJSON, deviceIDOverlayJSON, "regionX");
							NETSDK_json_copy_child(formDeviceIDOverlayJSON, deviceIDOverlayJSON, "regionY");
							// FIXME: device ID
						}
						if(NULL != formTextOverlaysJSON && NULL != textOverlaysJSON){
							int const formTextOverlaysLength = json_object_array_length(formTextOverlaysJSON);
							int const textOverlaysLength = json_object_array_length(textOverlaysJSON);
							for(i = 0; i < formTextOverlaysLength; ++i){
								// find the match and set
								LP_JSON_OBJECT formTextOverlayJSON = json_object_array_get_idx(formTextOverlaysJSON, i);
								for(ii = 0; ii < textOverlaysLength; ++ii){
									LP_JSON_OBJECT textOverlay = json_object_array_get_idx(textOverlaysJSON, ii);
									if(NETSDK_json_get_int(formTextOverlayJSON, "id") == NETSDK_json_get_int(textOverlay, "id")){
										NETSDK_json_copy_child(formTextOverlayJSON, textOverlay, "enabled");
										NETSDK_json_copy_child(formTextOverlayJSON, textOverlay, "regionX");
										NETSDK_json_copy_child(formTextOverlayJSON, textOverlay, "regionY");
										NETSDK_json_copy_child(formTextOverlayJSON, textOverlay, "message");
										break;
									}
								}
							}
						}
					}
				}
			}
			VIDEO_LEAVE_CRITICAL();

			// caution: interface NETSDK_conf_venc_ch_get would enter critical
			if(netsdk->videoEncodeChannelChanged){
				ST_NSDK_VENC_CH venc_ch;
				if(NETSDK_conf_venc_ch_get(id, &venc_ch)){
					if(0 == netsdk->videoEncodeChannelChanged(id, &venc_ch)){
						netsdkRet = kNSDK_INS_RET_OK;
					}else{
						netsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
					}
				}
			}else{
				netsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
			}
			
			if(kNSDK_INS_RET_OK == netsdkRet){
				// save to file
				NETSDK_conf_video_save2();
			}
		}
	}

	//APP_TRACE(content);	
	return netsdkRet;
}

static int video_encode_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT encRefJSON, LP_JSON_OBJECT encDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int id = 0;
	int netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;
	bool overlayChanged = false;
	int text_overlay_id = -1;

	// check properties necessary when GET method
	if(HTTP_IS_GET(context) && !NSDK_PROPERTIES(subURI)){
		video_encode_remove_properties(encDupJSON);
	}
	
	if(prefix = "/CHANNELS", 0 == strncmp(prefix, subURI, strlen(prefix))){
		// get all channels
		subURI += strlen(prefix);
		netsdkRet = video_encode_channel(context, subURI, encRefJSON, encDupJSON, formJSON, 0, content, contentMax);
	}else if(prefix = "/CHANNEL/%d", 1 == sscanf(subURI, prefix, &id)){
		if(id > 0){
			LP_JSON_OBJECT channelsJSON = NULL;
			LP_JSON_OBJECT channelJSON = NULL;
			
			subURI += sprintf(content, prefix, id);
			//APP_TRACE("URI : %s", subURI);

			if(id < 100){
				id += 100;
			}

			if(HTTP_IS_GET(context)){
				channelsJSON = NETSDK_json_get_child(encDupJSON, "videoEncodeChannel");
			}else{//else if(HTTP_IS_PUT(context)){
				channelsJSON = NETSDK_json_get_child(encRefJSON, "videoEncodeChannel");
			}
			channelJSON = video_find_channel(channelsJSON, id);
			//APP_TRACE("%s", json_object_to_json_string(channelJSON));
			
			if(prefix = "/SNAPSHOT", 0 == strncmp(subURI, prefix, strlen(prefix))){
				if(HTTP_IS_GET(context)){
					// get method only
					if(netsdk->videoEncodeSnapShot){
						char filePath[256] = {""};
						LP_HTTP_QUERY_PARA_LIST query_list = HTTP_UTIL_parse_query_as_para(context->request_header->query);
						const char *token = NULL;
						int resolutionWidth = (token = query_list->read(query_list, "resolutionWidth")) ? atoi(token) : 0;
						int resolutionHeight = (token = query_list->read(query_list, "resolutionHeight")) ? atoi(token) : 0;

						if(!resolutionWidth || !resolutionHeight){
							ST_NSDK_VENC_CH venc_ch;
							if(NETSDK_conf_venc_ch_get(id, &venc_ch)){
								if(!venc_ch.freeResolution){
									resolutionWidth = (venc_ch.resolution >> 16) & 0xffff;
									resolutionHeight = (venc_ch.resolution >> 0) & 0xffff;
								}else{
									resolutionWidth = venc_ch.resolutionWidth;
									resolutionHeight = venc_ch.resolutionHeight;
								}
							}
						}
						
						if(0 == netsdk->videoEncodeSnapShot(id, kNSDK_SNAPSHOT_IMAGE_JPEG, resolutionWidth, resolutionHeight, filePath)){
							NETSDK_response_file(context->sock, "image/jpeg", filePath);
							netsdkRet = kNSDK_INS_RET_CONTENT_SENT;
						}else{
							netsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
						}

						// clear the temp image file
						unlink(filePath);
						remove(filePath);
						// clear query list
						query_list->free(query_list);
						query_list = NULL;
					}else{
						netsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
					}
				}
			}else if(prefix = "/REQUESTKEYFRAME", 0 == strncmp(subURI, prefix, strlen(prefix))){
				if(HTTP_IS_GET(context) || HTTP_IS_PUT(context)){
					if(netsdk->videoEncodeRequestKeyFrame){
						if(0 == netsdk->videoEncodeRequestKeyFrame(id)){
							netsdkRet = kNSDK_INS_RET_OK;
						}else{
							netsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}else{
						netsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
					}
				}
				
			}else if(prefix = "/CHANNELNAMEOVERLAY", 0 == strncmp(subURI, prefix, strlen(prefix))){
				LP_JSON_OBJECT channelNameOverlayJSON = NETSDK_json_get_child(channelJSON, "channelNameOverlay");
				
				if(HTTP_IS_GET(context)){
					snprintf(content, contentMax, "%s", json_object_to_json_string(channelNameOverlayJSON));
					netsdkRet = kNSDK_INS_RET_CONTENT_READY;
				}else if(HTTP_IS_PUT(context)){
					if(0 == VIDEO_ENTER_CRITICAL()){
						NETSDK_json_copy_child(formJSON, channelNameOverlayJSON, "enabled");
						NETSDK_json_copy_child(formJSON, channelNameOverlayJSON, "regionX");
						NETSDK_json_copy_child(formJSON, channelNameOverlayJSON, "regionY");
						VIDEO_LEAVE_CRITICAL();
						netsdkRet = video_encode_channel(context, subURI, encRefJSON, encDupJSON, channelJSON, id, content, contentMax);
					}
				}
				
			}else if(prefix = "/DATETIMEOVERLAY", 0 == strncmp(subURI, prefix, strlen(prefix))){
				LP_JSON_OBJECT datetimeOverlayJSON = NETSDK_json_get_child(channelJSON, "datetimeOverlay");
				
				if(HTTP_IS_GET(context)){
					snprintf(content, contentMax, "%s", json_object_to_json_string(datetimeOverlayJSON));
					netsdkRet = kNSDK_INS_RET_CONTENT_READY;
				}else if(HTTP_IS_PUT(context)){
					if(0 == VIDEO_ENTER_CRITICAL()){
						NETSDK_json_copy_child(formJSON, datetimeOverlayJSON, "enabled");
						NETSDK_json_copy_child(formJSON, datetimeOverlayJSON, "regionX");
						NETSDK_json_copy_child(formJSON, datetimeOverlayJSON, "regionY");
						NETSDK_json_copy_child(formJSON, datetimeOverlayJSON, "dateFormat");
						NETSDK_json_copy_child(formJSON, datetimeOverlayJSON, "timeFormat");
						NETSDK_json_copy_child(formJSON, datetimeOverlayJSON, "displayWeek");
						VIDEO_LEAVE_CRITICAL();
						netsdkRet = video_encode_channel(context, subURI, encRefJSON, encDupJSON, channelJSON, id, content, contentMax);	
					}
				}
				
			}else if(prefix = "/DEVICEIDOVERLAY", 0 == strncmp(subURI, prefix, strlen(prefix))){
				LP_JSON_OBJECT deviceIDOverlayJSON = NETSDK_json_get_child(channelJSON, "deviceIDOverlay");
				
				if(HTTP_IS_GET(context)){
					snprintf(content, contentMax, "%s", json_object_to_json_string(deviceIDOverlayJSON));
					netsdkRet = kNSDK_INS_RET_CONTENT_READY;
				}else if(HTTP_IS_PUT(context)){
					if(0 == VIDEO_ENTER_CRITICAL()){
						NETSDK_json_copy_child(formJSON, deviceIDOverlayJSON, "enabled");
						NETSDK_json_copy_child(formJSON, deviceIDOverlayJSON, "regionX");
						NETSDK_json_copy_child(formJSON, deviceIDOverlayJSON, "regionY");
						VIDEO_LEAVE_CRITICAL();
						netsdkRet = video_encode_channel(context, subURI, encRefJSON, encDupJSON, channelJSON, id, content, contentMax);
					}
				}
				
			}else if(prefix = "/TEXTOVERLAYS", 0 == strncmp(subURI, prefix, strlen(prefix))){
				// FIXME:
				netsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
			}else if(prefix = "/TEXTOVERLAY/%d", 1 == sscanf(subURI, prefix, &text_overlay_id)){
				// FIXME:
				netsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
			}else{
				// particular encode channel configuration
				netsdkRet = video_encode_channel(context, subURI, encRefJSON, encDupJSON, formJSON, id, content, contentMax);
			}
		}
	}
	return netsdkRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////////////////////////////

void NETSDK_conf_vin_ch_dump(LP_NSDK_VIN_CH vin_ch)
{
	int i = 0;
	printf("id=%d\r\n"
		"powerLineFrequencyMode=%d\r\n"
		"captureWidth=%d\r\n"
		"captureHeight=%d\r\n"
		"captureFrameRate=%d\r\n"
		"brightnessLevel=%d\r\n"
		"contrastLevel=%d\r\n"
		"sharpnessLevel=%d\r\n"
		"saturationLevel=%d\r\n"
		"hueLevel=%d\r\n"
		"flipEnabled=%d\r\n"
		"mirrorEnabled=%d\r\n",
		vin_ch->id,
		vin_ch->powerLineFrequencyMode,
		vin_ch->captureWidth,
		vin_ch->captureHeight,
		vin_ch->captureFrameRate,
		vin_ch->brightnessLevel,
		vin_ch->contrastLevel,
		vin_ch->sharpnessLevel,
		vin_ch->saturationLevel,
		vin_ch->hueLevel,
		vin_ch->flip,
		vin_ch->mirror);

	for(i = 0; i < (sizeof(vin_ch->privacyMask)/sizeof(vin_ch->privacyMask[0])); ++i){
		printf("privacyMask[%d]\r\n"
			"\tid=%d\r\n"
			"\tenabled=%s\r\n"
			"\tregionX=%f\r\n"
			"\tregionY=%f\r\n"
			"\tregionWidth=%f\r\n"
			"\tregionHeight=%f\r\n"
			"\tregionColor=#%08x\r\n",
			i,
			vin_ch->privacyMask[i].id,
			vin_ch->privacyMask[i].enabled ? "true" : "false",
			vin_ch->privacyMask[i].regionX,
			vin_ch->privacyMask[i].regionY,
			vin_ch->privacyMask[i].regionWidth,
			vin_ch->privacyMask[i].regionHeight,
			vin_ch->privacyMask[i].regionColor);
	}
}

static LP_NSDK_VIN_CH netsdk_conf_vin_ch(bool set, int id, LP_NSDK_VIN_CH vin_ch)
{
	int i = 0;

	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}

    if(NULL == vin_ch)
    {
        return NULL;
    }

    if(0 == VIDEO_ENTER_CRITICAL())
    {
    	LP_JSON_OBJECT jsonVideo = json_object_get(netsdk->video_conf);
    	LP_JSON_OBJECT channels = NETSDK_json_get_child(jsonVideo, "videoInput.videoInputChannel");
    	LP_JSON_OBJECT channel = NULL;
    	
    	for(i = 0; i < json_object_array_length(channels); ++i){
    		channel = json_object_array_get_idx(channels, i);
    		if(json_object_get_int(json_object_object_get(channel, "id")) == id){
    			break;
    		}else{
    			// clear
    			channel = NULL;
    		}
    	}

    	if(channel && vin_ch){
    		char text[128] = {""};
    		char *str = NULL;

    		if(set){
    			LP_JSON_OBJECT privacy_mask = NETSDK_json_get_child(channel, "privacyMask");

    			NETSDK_json_set_int2(channel, "id", 1);
    			NETSDK_json_set_int2(channel, "powerLineFrequencyMode", vin_ch->powerLineFrequencyMode);
    			NETSDK_json_set_int2(channel, "captureWidth", vin_ch->captureWidth);
    			NETSDK_json_set_int2(channel, "captureHeight", vin_ch->captureHeight);
    			NETSDK_json_set_int2(channel, "captureFrameRate", vin_ch->captureFrameRate);
    			NETSDK_json_set_int2(channel, "brightnessLevel", vin_ch->brightnessLevel);
    			NETSDK_json_set_int2(channel, "contrastLevel", vin_ch->contrastLevel);
    			NETSDK_json_set_int2(channel, "sharpnessLevel", vin_ch->sharpnessLevel);
    			NETSDK_json_set_int2(channel, "saturationLevel", vin_ch->saturationLevel);
    			NETSDK_json_set_int2(channel, "hueLevel", vin_ch->hueLevel);
    			NETSDK_json_set_boolean2(channel, "flipEnabled", vin_ch->flip);
    			NETSDK_json_set_boolean2(channel, "mirrorEnabled", vin_ch->mirror);
    			APP_TRACE("flipEnabled:%d", vin_ch->flip);
    			APP_TRACE("mirrorEnabled:%d", vin_ch->mirror);

    			for(i = 0; i < json_object_array_length(privacy_mask)
    				&& i < (sizeof(vin_ch->privacyMask)/sizeof(vin_ch->privacyMask[0])); ++i){				
    				LP_JSON_OBJECT region = json_object_array_get_idx(privacy_mask, i);
    				NETSDK_json_set_int2(channel, "id", vin_ch->privacyMask[i].id);
    				NETSDK_json_set_boolean2(region,"enabled",vin_ch->privacyMask[i].enabled);
    				NETSDK_json_set_float2(region,"regionX",vin_ch->privacyMask[i].regionX);
    				NETSDK_json_set_float2(region,"regionY",vin_ch->privacyMask[i].regionY);
    				NETSDK_json_set_float2(region,"regionWidth",vin_ch->privacyMask[i].regionWidth);
    				NETSDK_json_set_float2(region,"regionHeight",vin_ch->privacyMask[i].regionHeight);
    				sprintf(text,"%08x",vin_ch->privacyMask[i].regionColor);
    				NETSDK_json_set_string2(region,"regionColor",text);
    			}
    			// save to file
    			NETSDK_conf_video_save2();

    		}else{
    			LP_JSON_OBJECT privacy_mask = NETSDK_json_get_child(channel, "privacyMask");
    					
    			vin_ch->id = id;
    			vin_ch->powerLineFrequencyMode = NETSDK_json_get_int(channel, "powerLineFrequencyMode");
    			vin_ch->captureWidth = NETSDK_json_get_int(channel, "captureWidth");
    			vin_ch->captureHeight = NETSDK_json_get_int(channel, "captureHeight");
    			vin_ch->captureFrameRate = NETSDK_json_get_int(channel, "captureFrameRate");
    			vin_ch->brightnessLevel = NETSDK_json_get_int(channel, "brightnessLevel");
    			vin_ch->contrastLevel = NETSDK_json_get_int(channel, "contrastLevel");
    			vin_ch->sharpnessLevel = NETSDK_json_get_int(channel, "sharpnessLevel");
    			vin_ch->saturationLevel = NETSDK_json_get_int(channel, "saturationLevel");
    			vin_ch->hueLevel = NETSDK_json_get_int(channel, "hueLevel");
    			vin_ch->flip = NETSDK_json_get_boolean(channel, "flipEnabled");
    			vin_ch->mirror = NETSDK_json_get_boolean(channel, "mirrorEnabled");
    			// read privacy mask

    			for(i = 0; i < json_object_array_length(privacy_mask)
    				&& i < (sizeof(vin_ch->privacyMask)/sizeof(vin_ch->privacyMask[0])); ++i){				
    				LP_JSON_OBJECT region = json_object_array_get_idx(privacy_mask, i);

    				vin_ch->privacyMask[i].id = NETSDK_json_get_int(region, "id");
    				vin_ch->privacyMask[i].enabled = NETSDK_json_get_boolean(region, "enabled");
    				vin_ch->privacyMask[i].regionX = NETSDK_json_get_float(region, "regionX");
    				vin_ch->privacyMask[i].regionY = NETSDK_json_get_float(region, "regionY");
    				vin_ch->privacyMask[i].regionWidth = NETSDK_json_get_float(region, "regionWidth");
    				vin_ch->privacyMask[i].regionHeight = NETSDK_json_get_float(region, "regionHeight");
    				NETSDK_json_get_string(region,"regionColor",text,sizeof(text));
    				sscanf(text,"%x",&vin_ch->privacyMask[i].regionColor);
    			}

    		}
    		//NETSDK_conf_vin_ch_dump(vin_ch);
    	}
    	
        VIDEO_LEAVE_CRITICAL();
    	json_object_put(jsonVideo);
    	jsonVideo = NULL;

    	return channel ? vin_ch : NULL;
    }

    return NULL;

}


LP_NSDK_VIN_CH NETSDK_conf_vin_ch_get(int id, LP_NSDK_VIN_CH vin_ch)
{
	return netsdk_conf_vin_ch(false, id, vin_ch);
}

LP_NSDK_VIN_CH NETSDK_conf_vin_ch_set(int id, LP_NSDK_VIN_CH vin_ch)
{
	return netsdk_conf_vin_ch(true, id, vin_ch);
}

void NETSDK_conf_venc_ch_dump(LP_NSDK_VENC_CH venc_ch)
{
	printf("id=%d\r\n"
		"channelName=%s\r\n"
		"enabled=%s\r\n"
		"videoInputChannelID=%d\r\n"
		"codecType=%d\r\n"
		"resolution=%dx%d\r\n"
		"freeResolution=%s\r\n"
		"resolutionWidth=%d\r\n"
		"resolutionHeight=%d\r\n"
		"bitRateControlType=%d\r\n"
		"constantBitRate=%d\r\n"
		"frameRate=%d\r\n"
		"keyFrameInterval=%d\r\n"
		"snapShotImageType=%d\r\n",
		venc_ch->id,
		venc_ch->channelName,
		venc_ch->enabled ? "true" : "false",
		venc_ch->videoInputChannelID,
		venc_ch->codecType,
		(venc_ch->resolution >> 16) & 0xffff, (venc_ch->resolution >> 0) & 0xffff, 
		venc_ch->freeResolution ? "true" : "false",
		venc_ch->resolutionWidth,
		venc_ch->resolutionHeight,
		venc_ch->bitRateControlType,
		venc_ch->constantBitRate,
		venc_ch->frameRate,
		venc_ch->keyFrameInterval,
		venc_ch->snapShotImageType);
}

static LP_NSDK_VENC_CH netsdk_conf_venc_ch(bool set_flag, int channelID, LP_NSDK_VENC_CH venc_ch, bool immediate)
{
	int i = 0;
	char text[128] = {""};
	char *str = NULL;
	
	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}
	if(venc_ch){
		if(0 == VIDEO_ENTER_CRITICAL()){
			LP_JSON_OBJECT videoJSON = json_object_get(netsdk->video_conf);
			LP_JSON_OBJECT channelListJSON = NETSDK_json_get_child(videoJSON, "videoEncode.videoEncodeChannel");
			LP_JSON_OBJECT vinchannels = NETSDK_json_get_child(videoJSON, "videoInput.videoInputChannel");
			LP_JSON_OBJECT channel = video_find_channel(channelListJSON, channelID);
			LP_JSON_OBJECT vinchannel = video_find_channel(vinchannels, 1);

			if(NULL != channel && NULL != vinchannel){
				if(set_flag){
					LP_JSON_OBJECT channelNameOverlayJSON = NETSDK_json_get_child(channel, "channelNameOverlay");
					LP_JSON_OBJECT datetimeOverlayJSON = NETSDK_json_get_child(channel, "datetimeOverlay");
					LP_JSON_OBJECT deviceIDOverlayJSON = NETSDK_json_get_child(channel, "deviceIDOverlay");
					LP_JSON_OBJECT textOverlaysJSON = NETSDK_json_get_child(channel, "textOverlay");
					int const textOverlaysLength = json_object_array_length(textOverlaysJSON);
					ST_NSDK_VIN_CH vin_ch;
					vin_ch.powerLineFrequencyMode = NETSDK_json_get_int(vinchannel, "powerLineFrequencyMode");
					NETSDK_json_set_int2(channel, "id", channelID);
					if(strlen(venc_ch->channelName)>0){
						char channelNameutf8[64];
						if(gb2312_to_utf8(0, venc_ch->channelName, channelNameutf8, sizeof(channelNameutf8))){
							NETSDK_json_set_string2(channel, "channelName", channelNameutf8);
						}else{
							NETSDK_json_set_string2(channel, "channelName", venc_ch->channelName);
						}
					}
					NETSDK_json_set_boolean2(channel, "enable", venc_ch->enabled);
					NETSDK_json_set_int2(channel, "videoInputChannelID", venc_ch->videoInputChannelID);
					str = NETSDK_MAP_DEC2STR(codec_type,venc_ch->codecType,"H.264");
					NETSDK_json_set_string2(channel, "codecType", str);	
					str = NETSDK_MAP_DEC2STR(bitrate_control_type_map, venc_ch->bitRateControlType, "CBR");
					NETSDK_json_set_string2(channel, "bitRateControlType", str);
					str = NETSDK_MAP_DEC2STR(profile_type_map,venc_ch->h264Profile,"main");
					NETSDK_json_set_string2(channel, "h264Profile", str);
					//get resolution option
					if(immediate){
						LP_JSON_OBJECT propertyJSON = NETSDK_json_get_child(channel, "resolutionProperty");
						LP_JSON_OBJECT optionFromJSON = NETSDK_json_parse(venc_ch->resolutionProperty.opt);
						if(NULL != propertyJSON && NULL != optionFromJSON){
							//NETSDK_json_set_array2(propertyJSON, optionFromJSON, "opt");
							NETSDK_json_set_array(propertyJSON, optionFromJSON , NULL);
						}
						if(optionFromJSON){
							//release object
							json_object_put(optionFromJSON);
						}
					}
					str = NETSDK_MAP_DEC2STR(resolution_map,venc_ch->resolution,"640x480");
					NETSDK_json_set_string2(channel, "resolution", str);
					NETSDK_json_set_boolean2(channel, "freeResolution", venc_ch->freeResolution);
					if(venc_ch->freeResolution){
						NETSDK_json_set_int2(channel, "resolutionWidth", venc_ch->resolutionWidth);
						NETSDK_json_set_int2(channel, "resolutionHeight", venc_ch->resolutionHeight);
					}
					NETSDK_json_set_int2(channel, "constantBitRate", venc_ch->constantBitRate);
					if(vin_ch.powerLineFrequencyMode/2 < venc_ch->frameRate){
						venc_ch->frameRate == vin_ch.powerLineFrequencyMode/2;
					}
					str = NETSDK_MAP_DEC2STR(definition_type,venc_ch->definitionType,"auto");
					NETSDK_json_set_string2(channel, "definitionType", str);
					NETSDK_json_set_int2(channel, "frameRate", venc_ch->frameRate);
					NETSDK_json_set_int(channel, "ImageTransmissionModel", venc_ch->ImageTransmissionModel);
					//fix resolution and fps limit in different soc
					uint32_t chip_id;
					if(!sdk_sys){
						APP_TRACE("sdk_sys not init!");							
						return NULL;
					}
					if(venc_ch->keyFrameInterval < venc_ch->frameRate){
						venc_ch->keyFrameInterval = venc_ch->frameRate;
					}
					NETSDK_json_set_int2(channel, "keyFrameInterval", venc_ch->keyFrameInterval);
					sdk_sys->get_chip_id(&chip_id);
					video_encode_channel_limit_check(channel, chip_id);
					//venc_ch->keyFrameInterval = venc_ch->frameRate*2;
					
					//NETSDK_json_set_int2(channel, "snapShotImageType", venc_ch->snapShotImageType);
					// channel name overlay
					NETSDK_json_set_boolean2(channelNameOverlayJSON, "enabled", venc_ch->channelNameOverlay.o.enabled);
					NETSDK_json_set_float2(channelNameOverlayJSON, "regionX", venc_ch->channelNameOverlay.o.regionX);
					NETSDK_json_set_float2(channelNameOverlayJSON, "regionY", venc_ch->channelNameOverlay.o.regionY);
					// date time overlay
					NETSDK_json_set_boolean2(datetimeOverlayJSON, "enabled", venc_ch->datetimeOverlay.o.enabled);
					NETSDK_json_set_float2(datetimeOverlayJSON, "regionX", venc_ch->datetimeOverlay.o.regionX);
					NETSDK_json_set_float2(datetimeOverlayJSON, "regionY", venc_ch->datetimeOverlay.o.regionY);
					str = NETSDK_MAP_DEC2STR(date_format_map, venc_ch->datetimeOverlay.dateFormat, "YYYY/MM/DD");
					NETSDK_json_set_string2(datetimeOverlayJSON, "dateFormat", str);
					NETSDK_json_set_int2(datetimeOverlayJSON, "timeFormat", venc_ch->datetimeOverlay.timeFormat);
					NETSDK_json_set_boolean2(datetimeOverlayJSON, "displayWeek", venc_ch->datetimeOverlay.displayWeek);
					// device id
					// FIXME: device ID
					NETSDK_json_set_boolean2(deviceIDOverlayJSON, "enabled", venc_ch->deviceIDOverlay.o.enabled);
					NETSDK_json_set_float2(deviceIDOverlayJSON, "regionX", venc_ch->deviceIDOverlay.o.regionX);
					NETSDK_json_set_float2(deviceIDOverlayJSON, "regionY", venc_ch->deviceIDOverlay.o.regionY);
					// text overlay
					for(i = 0; i < textOverlaysLength && i < (int)(sizeof(venc_ch->textOverlay)/sizeof(venc_ch->textOverlay[0])); ++i){
						LP_JSON_OBJECT textOverlay = json_object_array_get_idx(textOverlaysJSON, i);
						NETSDK_json_set_boolean2(textOverlay, "enabled", venc_ch->textOverlay[i].o.enabled);
						NETSDK_json_set_float2(textOverlay, "regionX", venc_ch->textOverlay[i].o.regionX);
						NETSDK_json_set_float2(textOverlay, "regionY", venc_ch->textOverlay[i].o.regionY);
						NETSDK_json_set_string2(textOverlay, "message", venc_ch->textOverlay[i].message);
					}
				}else{
					LP_JSON_OBJECT channelNameOverlayJSON = NETSDK_json_get_child(channel, "channelNameOverlay");
					LP_JSON_OBJECT datetimeOverlayJSON = NETSDK_json_get_child(channel, "datetimeOverlay");
					LP_JSON_OBJECT deviceIDOverlayJSON = NETSDK_json_get_child(channel, "deviceIDOverlay");
					LP_JSON_OBJECT textOverlaysJSON = NETSDK_json_get_child(channel, "textOverlay");
					int const textOverlaysLength = json_object_array_length(textOverlaysJSON);
					ST_NSDK_VIN_CH vin_ch;
					vin_ch.powerLineFrequencyMode = NETSDK_json_get_int(vinchannel, "powerLineFrequencyMode");

					venc_ch->id = channelID;
					char channelNameUtf8[64];
					NETSDK_json_get_string(channel, "channelName", channelNameUtf8, sizeof(channelNameUtf8));
					utf8_to_gb2312(channelNameUtf8, venc_ch->channelName,sizeof(venc_ch->channelName));
					venc_ch->enabled = NETSDK_json_get_int(channel, "enable");
					venc_ch->videoInputChannelID = NETSDK_json_get_int(channel, "videoInputChannelID");
					//codecType
					char codecType[16];
					NETSDK_json_get_string(channel, "codecType", codecType, sizeof(codecType));
					venc_ch->codecType = NETSDK_MAP_STR2DEC(codec_type,codecType,kNSDK_CODEC_TYPE_H264);
                    LP_JSON_OBJECT codecPropertyJSON = NETSDK_json_get_child(channel, "codecTypeProperty");
                    if(NULL != codecPropertyJSON)
                    {
                        LP_JSON_OBJECT optionJSON = NETSDK_json_get_child(codecPropertyJSON, "opt");
                        if(NULL != optionJSON)
                        {
                            snprintf(venc_ch->codecTypeOpt, sizeof(venc_ch->codecTypeOpt), "%s",
                                        json_object_to_json_string(optionJSON));
                        }
                    }
					//venc_ch->codecType = kNSDK_CODEC_TYPE_H265;
					//profile
					char h264Profile[16];
					NETSDK_json_get_string(channel, "h264Profile", h264Profile, sizeof(h264Profile));
					venc_ch->h264Profile = NETSDK_MAP_STR2DEC(profile_type_map,h264Profile,kSDK_ENC_H264_PROFILE_MAIN);

					//definition type
					char definitionType[16];
					NETSDK_json_get_string(channel, "definitionType", definitionType, sizeof(definitionType));
					venc_ch->definitionType= NETSDK_MAP_STR2DEC(definition_type, definitionType, kNSDK_DEFINITION_AUTO);
					
					// read resolution
					venc_ch->freeResolution = NETSDK_json_get_boolean(channel, "freeResolution");
					strcpy(venc_ch->resolutionProperty.opt, ""); // clear					
					if(NETSDK_json_get_string(channel, "resolution", text, sizeof(text))){
						// get resolution property
						LP_JSON_OBJECT propertyJSON = NETSDK_json_get_child(channel, "resolutionProperty");
						// clear 
						//APP_TRACE("%s", json_object_to_json_string(channel));
						if(NULL != propertyJSON){
							LP_JSON_OBJECT optionJSON = NETSDK_json_get_child(propertyJSON, "opt");
							//APP_TRACE("%s", json_object_to_json_string(propertyJSON));
							if(NULL != optionJSON){
								snprintf(venc_ch->resolutionProperty.opt, sizeof(venc_ch->resolutionProperty.opt), "%s",
									json_object_to_json_string(optionJSON));
								//APP_TRACE("Option: %s", venc_ch->resolutionProperty.opt);
							}
						}
					}
					venc_ch->resolution = NETSDK_MAP_STR2DEC(resolution_map,text,kNSDK_RES_640X480);
					if(venc_ch->freeResolution){
						// user free resolution
						venc_ch->resolutionWidth = NETSDK_json_get_int(channel, "resolutionWidth");
						venc_ch->resolutionHeight = NETSDK_json_get_int(channel, "resolutionHeight");
					}else{
						venc_ch->resolutionWidth = (venc_ch->resolution >> 16) & 0xffff;
						venc_ch->resolutionHeight = (venc_ch->resolution >> 0) & 0xffff;
					}
					LP_JSON_OBJECT brpropertyJSON = NETSDK_json_get_child(channel, "constantBitRateProperty");
					venc_ch->constantBitRateProperty.min = NETSDK_json_get_int(brpropertyJSON, "min");
					venc_ch->constantBitRateProperty.max = NETSDK_json_get_int(brpropertyJSON, "max");
					//APP_TRACE("%s", json_object_to_json_string(brpropertyJSON));

					// read bitrate
					NETSDK_json_get_string(channel, "bitRateControlType", text, sizeof(text));
					venc_ch->bitRateControlType = NETSDK_MAP_STR2DEC(bitrate_control_type_map,text,kNSDK_BR_CONTROL_CBR);
					venc_ch->constantBitRate = NETSDK_json_get_int(channel, "constantBitRate");
					venc_ch->frameRate = NETSDK_json_get_int(channel, "frameRate");
					if(vin_ch.powerLineFrequencyMode/2 < venc_ch->frameRate){
						venc_ch->frameRate == vin_ch.powerLineFrequencyMode/2;
					}
					venc_ch->ImageTransmissionModel = NETSDK_json_get_int(channel, "ImageTransmissionModel");
					//venc_ch->keyFrameInterval = venc_ch->frameRate*2;
					venc_ch->keyFrameInterval = NETSDK_json_get_int(channel, "keyFrameInterval");
					//APP_TRACE("venc_ch->keyFrameInterval = %d", venc_ch->keyFrameInterval);
					venc_ch->snapShotImageType = kNSDK_SNAPSHOT_IMAGE_JPEG;

					// channel name overlay
					venc_ch->channelNameOverlay.o.enabled = NETSDK_json_get_int(channelNameOverlayJSON, "enabled");
					venc_ch->channelNameOverlay.o.regionX = NETSDK_json_get_float(channelNameOverlayJSON, "regionX");
					venc_ch->channelNameOverlay.o.regionY = NETSDK_json_get_float(channelNameOverlayJSON, "regionY");

					// date time overlay
					venc_ch->datetimeOverlay.o.enabled = NETSDK_json_get_int(datetimeOverlayJSON, "enabled");
					venc_ch->datetimeOverlay.o.regionX = NETSDK_json_get_float(datetimeOverlayJSON, "regionX");
					venc_ch->datetimeOverlay.o.regionY = NETSDK_json_get_float(datetimeOverlayJSON, "regionY");
					NETSDK_json_get_string(datetimeOverlayJSON, "dateFormat", text, sizeof(text));
					venc_ch->datetimeOverlay.dateFormat = NETSDK_MAP_STR2DEC(date_format_map, text, kNSDK_DATETIME_FMT_SLASH_YYYYMMDD);
					venc_ch->datetimeOverlay.timeFormat = NETSDK_json_get_int(datetimeOverlayJSON, "timeFormat");
					venc_ch->datetimeOverlay.displayWeek = NETSDK_json_get_boolean(datetimeOverlayJSON, "displayWeek");

					// device id
					// FIXME: device ID
					venc_ch->deviceIDOverlay.o.enabled = NETSDK_json_get_int(deviceIDOverlayJSON, "enabled");
					venc_ch->deviceIDOverlay.o.regionX = NETSDK_json_get_float(deviceIDOverlayJSON, "regionX");
					venc_ch->deviceIDOverlay.o.regionY = NETSDK_json_get_float(deviceIDOverlayJSON, "regionY");
					// text overlay
					for(i = 0; i < textOverlaysLength && i < (int)(sizeof(venc_ch->textOverlay)/sizeof(venc_ch->textOverlay[0])); ++i){
						LP_JSON_OBJECT textOverlay = json_object_array_get_idx(textOverlaysJSON, i);
						venc_ch->textOverlay[i].o.enabled = NETSDK_json_get_boolean(textOverlay, "enabled");
						venc_ch->textOverlay[i].o.regionX = NETSDK_json_get_float(textOverlay, "regionX");
						venc_ch->textOverlay[i].o.regionY = NETSDK_json_get_float(textOverlay, "regionY");
						NETSDK_json_get_string(textOverlay, "message", venc_ch->textOverlay[i].message, sizeof(venc_ch->textOverlay[i].message));
					}
				}
			}
			// put JSON
			json_object_put(videoJSON);
			videoJSON = NULL;
			// remember to unlock
			VIDEO_LEAVE_CRITICAL();
		}
		//NETSDK_conf_venc_ch_dump(venc_ch);

		if(set_flag){
			// save to file
			if(immediate){
				NETSDK_conf_video_save();
			}else{
				NETSDK_conf_video_save2();
			}
		}
	}
	return venc_ch;
}

LP_NSDK_VENC_CH NETSDK_conf_venc_ch_get(int id, LP_NSDK_VENC_CH venc_ch)
{
	return netsdk_conf_venc_ch(false, id, venc_ch, false);
}

LP_NSDK_VENC_CH NETSDK_conf_venc_ch_set(int id, LP_NSDK_VENC_CH venc_ch)
{
	if(netsdk_conf_venc_ch(true, id, venc_ch, false)){//because of the encode ability limit
		netsdk_conf_venc_ch(false, id, venc_ch, false);
		if(netsdk->videoEncodeChannelChanged){
			netsdk->videoEncodeChannelChanged(id, venc_ch);
		}
		return venc_ch;
	}else{
		return NULL;
	}
}

LP_NSDK_VENC_CH NETSDK_conf_venc_ch_set2(int id, LP_NSDK_VENC_CH venc_ch, bool immediate)
{
	if(netsdk_conf_venc_ch(true, id, venc_ch, immediate)){//because of the encode ability limit
		return netsdk_conf_venc_ch(false, id, venc_ch, false);
	}else{
		return NULL;
	}
}


void NETSDK_conf_md_ch_dump(LP_NSDK_MD_CH md_ch)
{
	int i = 0;
	
/*	printf("id=%d\r\n"
		"enabled=%s\r\n"
		"detectionType=%s\r\n",
		md_ch->id,
		md_ch->enabled ? "true" : "false",
		(md_ch->detectionType == kNSDK_MD_TYPE_GRID) ? "grid" : "region");*/

	if(md_ch->detectionType == kNSDK_MD_TYPE_REGION){
		for(i = 0; i < (sizeof(md_ch->detectionRegion.ch)/sizeof(md_ch->detectionRegion.ch[0])); ++i){
/*			printf("mdRegion[%d]\r\n"
				"\tid=%d\r\n"
				"\tenabled=%s\r\n"
				"\tregionX=%f\r\n"
				"\tregionY=%f\r\n"
				"\tregionWidth=%f\r\n"
				"\tregionHeight=%f\r\n"
				"\tsensitivityLevel=%d\r\n",
				i,
				md_ch->detectionRegion.ch[i].id,
				md_ch->detectionRegion.ch[i].enabled ? "true" : "false",
				md_ch->detectionRegion.ch[i].regionX,
				md_ch->detectionRegion.ch[i].regionY,
				md_ch->detectionRegion.ch[i].regionWidth,
				md_ch->detectionRegion.ch[i].regionHeight,
				md_ch->detectionRegion.ch[i].sensitivityLevel);
			*/
		}
	}
	else{
		int x,y;
/*		printf("rowGranularity=%d\r\n"
			"columnGranularity=%d\r\n"
			"sensitivityLevel=%d\r\n"
			"granularity:\r\n",
			md_ch->detectionGrid.rowGranularity,
			md_ch->detectionGrid.columnGranularity,
			md_ch->detectionGrid.sensitivityLevel);
			*/
		for(y=0;y<md_ch->detectionGrid.rowGranularity;y++){
			for(x=0;x<md_ch->detectionGrid.columnGranularity;x++){
				//printf("%s, ",(md_ch->detectionGrid.get_granularity(&md_ch->detectionGrid,y,x)==true) ? "T" : "F");
				//bool flag = video_md_get_granularity(LP_JSON_OBJECT obj,int row,int column)

				
				//printf("%s, ",MD_GRANULARITY_GET(md_ch->detectionGrid.granularity,md_ch->detectionGrid.columnGranularity,y,x) ? "T" : "F");
			}
			//printf("\r\n");
		}
	}
//	printf("\r\n");
}

static LP_NSDK_MD_CH netsdk_conf_md_ch(bool set_flag, int id, LP_NSDK_MD_CH md_ch)
{
	int i = 0, ii = 0;

	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}

    if(NULL == md_ch)
    {
        return NULL;
    }

    if(0 == VIDEO_ENTER_CRITICAL())
    {
    	LP_JSON_OBJECT jso = json_object_get(netsdk->video_conf);
    	LP_JSON_OBJECT channelsJSON = NETSDK_json_get_child(jso, "motionDetection.motionDetectionChannel");
    	LP_JSON_OBJECT channelJSON = video_find_channel(channelsJSON, id);

    	// init the interfaces
    	//memset(md_ch,0,sizeof(ST_NSDK_MD_CH));
    	md_ch->detectionGrid.getGranularity = motion_detection_get_granularity;
    	md_ch->detectionGrid.setGranularity = motion_detection_set_granularity;

    	if(NULL != channelJSON && NULL != md_ch){
    		char text[128] = {""};
    		char *str = NULL;

    		if(set_flag){
				LP_JSON_OBJECT regionsJSON = NETSDK_json_get_child(channelJSON, "detectionRegion");
				LP_JSON_OBJECT gridJSON = NETSDK_json_get_child(channelJSON, "detectionGrid");
				LP_JSON_OBJECT granularityJSON = NETSDK_json_get_child(gridJSON, "granularity");
				int const n_regions = json_object_array_length(regionsJSON);
				
				//NETSDK_json_set_int2(channel, "id", 1);
				NETSDK_json_set_boolean2(channelJSON, "enabled", md_ch->enabled);
				NETSDK_json_set_string2(channelJSON, "detectionType", 
					(md_ch->detectionType == kNSDK_MD_TYPE_GRID) ? "grid" : "region");

				for(i = 0; i < n_regions
					&& i < (sizeof(md_ch->detectionRegion.ch) / sizeof(md_ch->detectionRegion.ch[0])); ++i){				
					LP_JSON_OBJECT regionJSON = json_object_array_get_idx(regionsJSON, i);
					//NETSDK_json_set_int2(region, "id", md_ch->detectionRegion.ch[i].id);
					NETSDK_json_set_boolean2(regionJSON, "enabled", md_ch->detectionRegion.ch[i].enabled);
					NETSDK_json_set_float2(regionJSON, "regionX", md_ch->detectionRegion.ch[i].regionX);
					NETSDK_json_set_float2(regionJSON, "regionY", md_ch->detectionRegion.ch[i].regionY);
					NETSDK_json_set_float2(regionJSON, "regionWidth", md_ch->detectionRegion.ch[i].regionWidth);
					NETSDK_json_set_float2(regionJSON, "regionHeight", md_ch->detectionRegion.ch[i].regionHeight);
					NETSDK_json_set_int2(regionJSON, "sensitivityLevel", md_ch->detectionRegion.ch[i].sensitivityLevel);
				}

				//NETSDK_json_set_int2(grid, "rowGranularity", md_ch->detectionGrid.rowGranularity);
				//NETSDK_json_set_int2(grid,"columnGranularity",md_ch->detectionGrid.columnGranularity);
				APP_TRACE("%d", md_ch->detectionGrid.sensitivityLevel);
				NETSDK_json_set_int2(gridJSON, "sensitivityLevel",md_ch->detectionGrid.sensitivityLevel);
				for(i = 0; i < md_ch->detectionGrid.rowGranularity; ++i){
					for(ii = 0; ii < md_ch->detectionGrid.columnGranularity; ++ii){
						LP_NSDK_MD_GRID detectionGrid = &md_ch->detectionGrid;
						bool flag = detectionGrid->getGranularity(detectionGrid, i, ii);
						video_md_set_granularity(granularityJSON, i, ii, flag);
					}
				}

				if(netsdk->motionDetectionChannelChanged){
					netsdk->motionDetectionChannelChanged(id, md_ch);
				}
				NETSDK_conf_video_save2();
    		}else{
				LP_JSON_OBJECT detectionGridJSON = NETSDK_json_get_child(channelJSON,"detectionGrid");
				LP_JSON_OBJECT detectionRegionsJSON = NETSDK_json_get_child(channelJSON,"detectionRegion");
				
				md_ch->id = id;
				md_ch->enabled = NETSDK_json_get_boolean(channelJSON, "enabled");
				NETSDK_json_get_string(channelJSON, "detectionType", text, sizeof(text));

				
				if(0 == strcmp("grid", text)){
					md_ch->detectionType = kNSDK_MD_TYPE_GRID;
				}else if(0 == strcmp("region", text)){
					md_ch->detectionType = kNSDK_MD_TYPE_REGION;
				}else{
					md_ch->detectionType = kNSDK_MD_TYPE_GRID;
				}
			//	APP_TRACE("detectionType = %s/%d", text, md_ch->detectionType);
				
				if(NULL != detectionGridJSON){
					LP_JSON_OBJECT granularityJSON = NETSDK_json_get_child(detectionGridJSON, "granularity");
					int n_row=0,n_column=0;
					md_ch->detectionType = kNSDK_MD_TYPE_GRID;
					
					md_ch->detectionGrid.rowGranularity = NETSDK_json_get_int(detectionGridJSON, "rowGranularity");
					md_ch->detectionGrid.columnGranularity = NETSDK_json_get_int(detectionGridJSON, "columnGranularity");
					md_ch->detectionGrid.sensitivityLevel = NETSDK_json_get_int(detectionGridJSON, "sensitivityLevel");

					for(i = 0; i < md_ch->detectionGrid.rowGranularity; ++i){
						for(ii = 0; ii < md_ch->detectionGrid.columnGranularity; ++ii){
							LP_NSDK_MD_GRID detectionGrid = &md_ch->detectionGrid;
							bool flag = video_md_get_granularity(granularityJSON, i, ii);
							detectionGrid->setGranularity(detectionGrid, i, ii, flag);
						}
					}
				}
				if(NULL != detectionRegionsJSON){
					int const detectionRegionsLen = json_object_array_length(detectionRegionsJSON);
					
					for(i = 0; i < detectionRegionsLen
						&& i < (sizeof(md_ch->detectionRegion.ch)/sizeof(md_ch->detectionRegion.ch[0])); ++i){
						LP_JSON_OBJECT regionJSON = json_object_array_get_idx(detectionRegionsJSON, i);
						md_ch->detectionRegion.ch[i].id = NETSDK_json_get_int(regionJSON, "id");
						md_ch->detectionRegion.ch[i].enabled = NETSDK_json_get_boolean(regionJSON,"enabled");
						md_ch->detectionRegion.ch[i].regionX = NETSDK_json_get_float(regionJSON, "regionX");
						md_ch->detectionRegion.ch[i].regionY = NETSDK_json_get_float(regionJSON, "regionY");
						md_ch->detectionRegion.ch[i].regionWidth = NETSDK_json_get_float(regionJSON, "regionWidth");
						md_ch->detectionRegion.ch[i].regionHeight = NETSDK_json_get_float(regionJSON, "regionHeight");
						md_ch->detectionRegion.ch[i].sensitivityLevel = NETSDK_json_get_int(regionJSON, "sensitivityLevel");;
					}
				}
    			//APP_TRACE("md_ch->detectionType = %d", md_ch->detectionType);
    		}
    		NETSDK_conf_md_ch_dump(md_ch);
    	}

        VIDEO_LEAVE_CRITICAL();
    	json_object_put(jso);
    	jso = NULL;

    	return channelJSON ? md_ch : NULL;
    }

    return NULL;

}

LP_NSDK_MD_CH NETSDK_conf_md_ch_get(int id, LP_NSDK_MD_CH md_ch)
{
	return netsdk_conf_md_ch(false, id, md_ch);
}

LP_NSDK_MD_CH NETSDK_conf_md_ch_set(int id, LP_NSDK_MD_CH md_ch)
{
	return netsdk_conf_md_ch(true, id, md_ch);
}

int NETSDK_video_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI, char *content, int contentMax)
{
	int netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	LP_JSON_OBJECT videoJSON = json_object_get(netsdk->video_conf);
	LP_JSON_OBJECT subRefJSON = NULL;
	LP_JSON_OBJECT subDupJSON = NULL; // FIXME:
	LP_JSON_OBJECT formJSON = NULL;
	bool videoInput = false, videoEncode = false, motionDetection = false;
	HTTP_CSTR_t prefix = NULL;

	if(NULL != context->request_content && 0 != context->request_content_len){
		formJSON = NETSDK_json_parse(context->request_content);
		//APP_TRACE("Form JSON:\r\n%s", json_object_to_json_string(formJSON));
	}
	if(prefix = "/INPUT", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		videoInput = true;
		subRefJSON = NETSDK_json_get_child(videoJSON, "videoInput");
	}else if(prefix = "/ENCODE", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		videoEncode = true;
		subRefJSON = NETSDK_json_get_child(videoJSON, "videoEncode");
	}else if(prefix = "/MOTIONDETECTION", 0 == strncmp(prefix, subURI, strlen(prefix))){
		subURI += strlen(prefix);
		motionDetection = true;
		subRefJSON = NETSDK_json_get_child(videoJSON, "motionDetection");
	}

	if(NULL != subRefJSON){
		if(HTTP_IS_GET(context) && 0 == VIDEO_ENTER_CRITICAL()){
			subDupJSON = NETSDK_json_dup(subRefJSON);
			VIDEO_LEAVE_CRITICAL();
		}
	}
	
	if(videoInput){
		netsdkRet = video_input_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, contentMax);
	}else if(videoEncode){
		netsdkRet = video_encode_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, contentMax);
	}else if(motionDetection){
		netsdkRet = video_motiondetecion_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, contentMax);
	}

	// put all the reference of JSON
	if(NULL != formJSON){
		json_object_put(formJSON);
	}
	if(NULL != subDupJSON){
		json_object_put(subDupJSON);
		subDupJSON = NULL;
	}
	json_object_put(videoJSON);
	return netsdkRet;
}

void NETSDK_fix_resolution_opt(int sensor_type)
{
	LP_JSON_OBJECT videoJSON = json_object_get(netsdk->video_conf);
	LP_JSON_OBJECT subRefJSON = NULL;
	LP_JSON_OBJECT channelsJSON = NULL,channelJSON = NULL;
	bool videoInput = false, videoEncode = false, motionDetection = false;
	HTTP_CSTR_t prefix = NULL;

	subRefJSON = NETSDK_json_get_child(videoJSON, "videoEncode");
	channelsJSON = NETSDK_json_get_child(subRefJSON, "videoEncodeChannel");
	channelJSON = video_find_channel(channelsJSON, 101);
	if(sensor_type == SENSOR_MODEL_OV_OV9712PLUS ||
		sensor_type == SENSOR_MODEL_OV_OV9712 ||
		sensor_type == SENSOR_MODEL_SOI_H22){
		if(!NETSDK_json_remove_property_option(channelJSON, "resolution", "1280x960")){
        	NETSDK_conf_video_save();     
		}
	}else if(sensor_type == SENSOR_MODEL_OV_OV4689){
		if(!NETSDK_json_remove_property_option(channelJSON, "resolution", "2592x1944")){
        	NETSDK_conf_video_save();     
		}
	}else if((sensor_type == SENSOR_MODEL_SC1045 ||
		sensor_type == SENSOR_MODEL_SC1145) &&
		(!strcmp(SOC_MODEL,"HI3518E_V2"))){
		if(!NETSDK_json_remove_property_option(channelJSON, "resolution", "1920x1080") && 
			!NETSDK_json_remove_property_option(channelJSON, "resolution", "1280x960")){
        	NETSDK_conf_video_save();     
		}
	}else if((sensor_type == SENSOR_MODEL_SC1035 ||
		sensor_type == SENSOR_MODEL_APTINA_AR0130 ||
		sensor_type == SENSOR_MODEL_SC1135) &&
		(!strcmp(SOC_MODEL,"HI3518E_V2"))){
		if(!NETSDK_json_remove_property_option(channelJSON, "resolution", "1920x1080")){
        	NETSDK_conf_video_save();     
		}
	}
	
}

//ID		:VIDEO IN    	0
//TYPE	:REVERSE		0
//WIDTH:					-1
//HEIGHT					-1
//FILE_PATH:RETURN		PTR
int NETSDK_venc_snapshot(int id, int type, int width, int height, char *file_path)
{
	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}
	return netsdk->videoEncodeSnapShot(id, type, width, height, *file_path);
}

//must be call after calling NETSDK_venc_snapshot()
void NETSDK_venc_free_snapshot(char *file_path)
{
	char cmd[256];
	sprintf(cmd, "rm -rf %s", file_path);
	NK_SYSTEM(cmd);
}

int NETSDK_venc_get_channels()
{
	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return 0;
	}

	LP_JSON_OBJECT videoJSON = json_object_get(netsdk->video_conf);
	LP_JSON_OBJECT channelListJSON = NETSDK_json_get_child(videoJSON, "videoEncode.videoEncodeChannel");
	int n_channels = json_object_array_length(channelListJSON);

	//get actual stream count; do not get from video.json
	if(!IS_FILE_EXIST("/media/conf/actual_stream")){
		n_channels = 2;
	}else{
		//get from video.json
	}
	return n_channels;
}

