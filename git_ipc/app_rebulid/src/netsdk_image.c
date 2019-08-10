
#include "netsdk.h"
#include "netsdk_util.h"
#include "netsdk_private.h"
#include "generic.h"
#include "sdk/sdk_api.h"
#include "app_debug.h"
#include "fisheye.h"
/////////////////////////////////////////////////////////////////////////////////////////////////
// NetSDK general opertaions
/////////////////////////////////////////////////////////////////////////////////////////////////

//image
const ST_NSDK_MAP_STR_DEC sceneMode_map[] = {
		{"auto", kNSDK_IMAGE_SCENE_MODE_AUTO},
		{"indoor", kNSDK_IMAGE_SCENE_MODE_INDOOR},
		{"outdoor", kNSDK_IMAGE_SCENE_MODE_OUTDOOR},
	};

const ST_NSDK_MAP_STR_DEC irCutControlMode_map[] = {
	{"hardware", kNSDK_IMAGE_IRCUT_CONTROL_MODE_HARDWARE},
	{"software", kNSDK_IMAGE_IRCUT_CONTROL_MODE_SOFTWARE},
};

const ST_NSDK_MAP_STR_DEC irCutMode_map[] = {
	{"auto", kNSDK_IMAGE_IRCUT_MODE_AUTO},
	{"daylight", kNSDK_IMAGE_IRCUT_MODE_DAYLIGHT},
	{"night", kNSDK_IMAGE_IRCUT_MODE_NIGHT},
	{"light", kNSDK_IMAGE_IRCUT_MODE_LIGHTMODE},
	{"smart", kNSDK_IMAGE_IRCUT_MODE_SMARTMODE},
};

const ST_NSDK_MAP_STR_DEC exposureMode_map[] = {
	{"auto", kNSDK_IMAGE_EXPOSURE_MODE_AUTO},
	{"bright", kNSDK_IMAGE_EXPOSURE_MODE_BRIGHT},
	{"dark", kNSDK_IMAGE_EXPOSURE_MODE_DARK},
};

const ST_NSDK_MAP_STR_DEC awbMode_map[] = {
	{"auto", kNSDK_IMAGE_AWB_MODE_AUTO},
	{"indoor", kNSDK_IMAGE_AWB_MODE_BRIGHT},
	{"outdoor", kNSDK_IMAGE_AWB_MODE_DARK},
};

const ST_NSDK_MAP_STR_DEC lowlightMode_map[] = {
	{"close", kNSDK_IMAGE_LOWLIGHT_MODE_CLOSE},
	{"only night", kNSDK_IMAGE_LOWLIGHT_MODE_NIGHT},
	{"day-night", kNSDK_IMAGE_LOWLIGHT_MODE_ALLDAY},
	{"auto", kNSDK_IMAGE_LOWLIGHT_MODE_AUTO},
};	

const ST_NSDK_MAP_STR_DEC lghtNhbtdMode_map[] = {
	{"auto", kNSDK_IMAGE_lghtNhbtd_MODE_AUTO},
	{"on", kNSDK_IMAGE_lghtNhbtd_MODE_ON},
	{"off", kNSDK_IMAGE_lghtNhbtd_MODE_OFF},
};

const ST_NSDK_MAP_STR_DEC BLcompensationMode_map[] = {
	{"auto", kNSDK_IMAGE_BLcompensation_MODE_AUTO},
	{"on", kNSDK_IMAGE_BLcompensation_MODE_ON},
	{"off", kNSDK_IMAGE_BLcompensation_MODE_OFF},
};

const ST_NSDK_MAP_STR_DEC fixMode_map[] = {
	{"wall", eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL},
	{"cell", eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL},
	{"table", eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE},
	{"none", eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE},
};	

const ST_NSDK_MAP_STR_DEC wallMode_map[] = {
	{"origin", eNSDK_IMAGE_FISHEYE_MODE_WALL_ORIGIN},
	{"180", eNSDK_IMAGE_FISHEYE_MODE_WALL_180},
	{"split", eNSDK_IMAGE_FISHEYE_MODE_WALL_SPLIT},
	{"wall_split", eNSDK_IMAGE_FISHEYE_MODE_WALL_WALL_SPLIT},
	{"4R", eNSDK_IMAGE_FISHEYE_MODE_WALL_4R},
	{"kitR", eNSDK_IMAGE_FISHEYE_MODE_WALL_KITR},
	{"kitO", eNSDK_IMAGE_FISHEYE_MODE_WALL_KITO},
};	

const ST_NSDK_MAP_STR_DEC cellMode_map[] = {
	{"origin", eNSDK_IMAGE_FISHEYE_MODE_CELL_ORIGIN},
	{"360", eNSDK_IMAGE_FISHEYE_MODE_CELL_360},
	{"split", eNSDK_IMAGE_FISHEYE_MODE_CELL_SPLIT},
	{"4R", eNSDK_IMAGE_FISHEYE_MODE_CELL_4R},
	{"wall_split", eNSDK_IMAGE_FISHEYE_MODE_CELL_WALL_SPLIT},
	{"180", eNSDK_IMAGE_FISHEYE_MODE_CELL_180},
	{"kitR", eNSDK_IMAGE_FISHEYE_MODE_CELL_KITR},
	{"kitO", eNSDK_IMAGE_FISHEYE_MODE_CELL_KITO},
};	

const ST_NSDK_MAP_STR_DEC tableMode_map[] = {
	{"origin", eNSDK_IMAGE_FISHEYE_MODE_TABLE_ORIGIN},
	{"360", eNSDK_IMAGE_FISHEYE_MODE_TABLE_360},
	{"split", eNSDK_IMAGE_FISHEYE_MODE_TABLE_SPLIT},
	{"4R", eNSDK_IMAGE_FISHEYE_MODE_TABLE_4R},
	{"VR", eNSDK_IMAGE_FISHEYE_MODE_TABLE_VR},
	{"kitR", eNSDK_IMAGE_FISHEYE_MODE_TABLE_KITR},
	{"kitO", eNSDK_IMAGE_FISHEYE_MODE_TABLE_KITO},
};

static inline int IMAGE_ENTER_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	return pthread_rwlock_wrlock(&netsdk->image_sync);
}

static int IMAGE_LEAVE_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	return pthread_rwlock_unlock(&netsdk->image_sync);
}



void NETSDK_conf_image_save()
{
    if(0 == IMAGE_ENTER_CRITICAL())
    {
        APP_TRACE("NetSDK Image Conf Save!!");
        NETSDK_conf_save(netsdk->image_conf, "image");
        IMAGE_LEAVE_CRITICAL();
    }
}

void NETSDK_conf_image_save2()
{
	if(netsdk->image_conf_save){
		netsdk->image_conf_save(eNSDK_CONF_SAVE_JUST_SAVE, 2);
	}else{
		NETSDK_conf_image_save();
	}
}

static void image_remove_properties(LP_JSON_OBJECT image)
{	
	if(image){
		NETSDK_json_remove_properties(image);
		LP_JSON_OBJECT irCutFilter = NETSDK_json_get_child(image, "irCutFilter");
		if(irCutFilter != NULL){
			NETSDK_json_remove_properties(irCutFilter);
		}

		LP_JSON_OBJECT manualSharpness = NETSDK_json_get_child(image, "manualSharpness");
		if(manualSharpness != NULL){
			NETSDK_json_remove_properties(manualSharpness);
		}

		LP_JSON_OBJECT denoise3d = NETSDK_json_get_child(image, "denoise3d");
		if(denoise3d != NULL){
			NETSDK_json_remove_properties(denoise3d);
		}
		
		LP_JSON_OBJECT WDR = NETSDK_json_get_child(image, "WDR");
		if(WDR != NULL){
			NETSDK_json_remove_properties(WDR);
		}

		LP_JSON_OBJECT videoMode = NETSDK_json_get_child(image, "videoMode");
		if(videoMode != NULL){
			NETSDK_json_remove_properties(videoMode);
		}
	}
}

static int image_ircutfilter(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT inputRefJSON, LP_JSON_OBJECT inputDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;

	if(HTTP_IS_GET(context)){
		snprintf(content, contentMax, "%s", json_object_to_json_string(inputDupJSON));
		ret = kNSDK_INS_RET_CONTENT_READY;
	}
	else if(HTTP_IS_PUT(context)){
		if(0 == IMAGE_ENTER_CRITICAL()){
			if(formJSON != NULL){	
				ret = kNSDK_INS_RET_OK;
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"irCutControlMode") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"irCutMode") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}
				IMAGE_LEAVE_CRITICAL();
				if(netsdk->imageChanged){
					ST_NSDK_IMAGE image;
					if(NETSDK_conf_image_get(&image) && ret == kNSDK_INS_RET_OK){
						if(0 == netsdk->imageChanged(&image)){
							ret = kNSDK_INS_RET_OK;
						}else{
							ret = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}
				}else{
					ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
				}
				if(ret == kNSDK_INS_RET_OK){
					NETSDK_conf_image_save2();
				}		
			}else{
				IMAGE_LEAVE_CRITICAL();
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}
		}
	}	
	return ret;
}

static int image_sharpness(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT inputRefJSON, LP_JSON_OBJECT inputDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;

	if(HTTP_IS_GET(context)){
		snprintf(content, contentMax, "%s", json_object_to_json_string(inputDupJSON));
		ret = kNSDK_INS_RET_CONTENT_READY;
	}
	else if(HTTP_IS_PUT(context)){
		if(0 == IMAGE_ENTER_CRITICAL()){
			if(formJSON != NULL){	
				ret = kNSDK_INS_RET_OK;
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"enabled") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"sharpnessLevel") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}	
				IMAGE_LEAVE_CRITICAL();
				if(netsdk->imageChanged){
					ST_NSDK_IMAGE image;
					if(NETSDK_conf_image_get(&image) && ret == kNSDK_INS_RET_OK){
						if(0 == netsdk->imageChanged(&image)){
							ret = kNSDK_INS_RET_OK;
						}else{
							ret = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}
				}else{
					ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
				}			
				if(ret == kNSDK_INS_RET_OK){
					NETSDK_conf_image_save2();
				}
			}else{
				IMAGE_LEAVE_CRITICAL();
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}
		}
	}	
	return ret;
}


static int image_denoise3d(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT inputRefJSON, LP_JSON_OBJECT inputDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;

	if(HTTP_IS_GET(context)){
		snprintf(content, contentMax, "%s", json_object_to_json_string(inputDupJSON));
		ret = kNSDK_INS_RET_CONTENT_READY;
	}
	else if(HTTP_IS_PUT(context)){
		if(0 == IMAGE_ENTER_CRITICAL()){
			if(formJSON != NULL){	
				ret = kNSDK_INS_RET_OK;
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"enabled") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"denoise3dStrength") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}	
				IMAGE_LEAVE_CRITICAL();
				if(netsdk->imageChanged){
					ST_NSDK_IMAGE image;
					if(NETSDK_conf_image_get(&image) && ret == kNSDK_INS_RET_OK){
						if(0 == netsdk->imageChanged(&image)){
							ret = kNSDK_INS_RET_OK;
						}else{
							ret = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}
				}else{
					ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
				}
				if(ret == kNSDK_INS_RET_OK){
					NETSDK_conf_image_save2();
				}
			}else{
				IMAGE_LEAVE_CRITICAL();
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}
		}
	}	
	return ret;
}


static int image_wdr(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT inputRefJSON, LP_JSON_OBJECT inputDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;

	if(HTTP_IS_GET(context)){
		snprintf(content, contentMax, "%s", json_object_to_json_string(inputDupJSON));
		ret = kNSDK_INS_RET_CONTENT_READY;
	}
	else if(HTTP_IS_PUT(context)){
		if(0 == IMAGE_ENTER_CRITICAL()){
			if(formJSON != NULL){	
				ret = kNSDK_INS_RET_OK;
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"enabled") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"WDRStrength") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}	
				IMAGE_LEAVE_CRITICAL();
				if(netsdk->imageChanged){
					ST_NSDK_IMAGE image;
					if(NETSDK_conf_image_get(&image) && ret == kNSDK_INS_RET_OK){
						if(0 == netsdk->imageChanged(&image)){
							ret = kNSDK_INS_RET_OK;
						}else{
							ret = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}
				}else{
					ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
				}
				
				if(kNSDK_INS_RET_OK == ret){
					// save to file
					NETSDK_conf_image_save2();
				}
			}else{
				IMAGE_LEAVE_CRITICAL();
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}
		}
	}	
	return ret;
}

static int image_autofocus(LP_HTTP_CONTEXT context, char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	if(HTTP_IS_GET(context)){
		APP_TRACE("%d", context->keep_alive);
		netsdk->AutoFocusStatus(context->sock, false);
		snprintf(content,contentMax, "%s", "ok");
		ret = kNSDK_INS_RET_CONTENT_READY;
	}else if(HTTP_IS_DELETE(context)){
		netsdk->AutoFocusStatus(context->sock, true);
		ret = kNSDK_INS_RET_OK;
	}
	return ret;
}

static int image_get_fisheye_param(LP_JSON_OBJECT dst_json)
{
	LP_JSON_OBJECT fixParam = json_object_new_array();
	LP_JSON_OBJECT param =NULL;
	stFISHEYE_config conf = {0};
	NK_Int i = 0;
	char *text = NULL;
	if(-1 != FISHEYE_config_get(&conf)){
		text = NETSDK_MAP_DEC2STR(fixMode_map, FISHEYE_get_fix_mode(), "cell");
		NETSDK_json_set_string2(dst_json, "fixMode", text);
        NETSDK_json_set_string2(dst_json, "LensName", conf.lensName);
        for(i = 0;i < (sizeof(conf.param) / sizeof(conf.param[0]));i++){
			param = json_object_new_object();
			NETSDK_json_set_int2(param, "id", i);
            if(conf.type) {
                NETSDK_json_set_int2(param, "CenterCoordinateX", conf.param[i].CenterCoordinateX);
                NETSDK_json_set_int2(param, "CenterCoordinateY", conf.param[i].CenterCoordinateY);
                NETSDK_json_set_int2(param, "Radius", conf.param[i].Radius);
                NETSDK_json_set_int2(param, "AngleX", conf.param[i].AngleX);
                NETSDK_json_set_int2(param, "AngleY", conf.param[i].AngleY);
                NETSDK_json_set_int2(param, "AngleZ", conf.param[i].AngleZ);
            }
            else {
                NETSDK_json_set_float2(param, "CenterCoordinateX", conf.param2[i].CenterCoordinateX);
                NETSDK_json_set_float2(param, "CenterCoordinateY", conf.param2[i].CenterCoordinateY);
                NETSDK_json_set_float2(param, "Radius", conf.param2[i].Radius);
                NETSDK_json_set_float2(param, "AngleX", conf.param2[i].AngleX);
                NETSDK_json_set_float2(param, "AngleY", conf.param2[i].AngleY);
                NETSDK_json_set_float2(param, "AngleZ", conf.param2[i].AngleZ);
            }
			json_object_array_put_idx(fixParam, i, param);
		}
		json_object_object_add(dst_json, "FixParam", fixParam);
	}
	return 0;
}

static int image_set_fisheye_param(LP_JSON_OBJECT src_json, const char *lensName, EM_NSDK_IMAGE_FISHEYE_FIX_MODE fixMode)
{
	int i;
	stFISHEYE_config conf;
    bool type = false;

    sprintf(conf.lensName, lensName);
	conf.fixMode = fixMode;
	for(i = 0; i < (sizeof(conf.param) /sizeof(conf.param[0]));i++){
        LP_JSON_OBJECT param = json_object_array_get_idx(src_json, i);
        if(param){
            /* 只判断第一个数据，如果是整形则按int存储，如果是浮点则按float存储 */
            if(json_type_int == json_object_get_type(json_object_object_get(param, "CenterCoordinateX"))) {
                conf.param[i].CenterCoordinateX = NETSDK_json_get_int(param, "CenterCoordinateX");
                conf.param[i].CenterCoordinateY = NETSDK_json_get_int(param, "CenterCoordinateY");
                conf.param[i].Radius = NETSDK_json_get_int(param, "Radius");
                conf.param[i].AngleX = NETSDK_json_get_int(param, "AngleX");
                conf.param[i].AngleY = NETSDK_json_get_int(param, "AngleY");
                conf.param[i].AngleZ = NETSDK_json_get_int(param, "AngleZ");
                type = true;
            }
            else {
                conf.param2[i].CenterCoordinateX = NETSDK_json_get_float(param, "CenterCoordinateX");
                conf.param2[i].CenterCoordinateY = NETSDK_json_get_float(param, "CenterCoordinateY");
                conf.param2[i].Radius = NETSDK_json_get_float(param, "Radius");
                conf.param2[i].AngleX = NETSDK_json_get_float(param, "AngleX");
                conf.param2[i].AngleY = NETSDK_json_get_float(param, "AngleY");
                conf.param2[i].AngleZ = NETSDK_json_get_float(param, "AngleZ");
                type = false;
            }
        }
	}
	FISHEYE_config_set(&conf, type);
	return 0;
}

static int image_videoMode(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT inputRefJSON, LP_JSON_OBJECT inputDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;

	if(HTTP_IS_GET(context)){
		image_get_fisheye_param(inputDupJSON);
		snprintf(content, contentMax, "%s", json_object_to_json_string(inputDupJSON));
		ret = kNSDK_INS_RET_CONTENT_READY;
	}
	else if(HTTP_IS_PUT(context)){
		if(0 == IMAGE_ENTER_CRITICAL()){
			if(formJSON != NULL){	
				ret = kNSDK_INS_RET_OK;
				if(NETSDK_json_copy_child(formJSON,inputRefJSON,"fixMode") == -1){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}
				char text[32];
				EM_NSDK_IMAGE_FISHEYE_FIX_MODE fixMode;
				if(NULL != NETSDK_json_get_string(formJSON, "fixMode", text, sizeof(text))){
					fixMode = NETSDK_MAP_STR2DEC(fixMode_map, text, eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE);
					if(!strcmp("wall", text)){
						if(NETSDK_json_copy_child(formJSON,inputRefJSON,"wallMode") == -1){
							ret = kNSDK_INS_RET_INVALID_DOCUMENT;
						}
					}
					if(!strcmp("cell", text)){
						if(NETSDK_json_copy_child(formJSON,inputRefJSON,"cellMode") == -1){
							ret = kNSDK_INS_RET_INVALID_DOCUMENT;
						}
					}
					if(!strcmp("table", text)){
						if(NETSDK_json_copy_child(formJSON,inputRefJSON,"tableMode") == -1){
							ret = kNSDK_INS_RET_INVALID_DOCUMENT;
						}
					}
				}
                char lensName[16];
				LP_JSON_OBJECT fixparam = NULL;

				if(NETSDK_json_check_child(formJSON, "LensName")) {
                    NETSDK_json_get_string(formJSON, "LensName", lensName, sizeof(lensName));
                }
                else {
                    sprintf(lensName, "m109");
                }

                fixparam = NETSDK_json_get_child(formJSON, "FixParam");
				if(fixparam){
					image_set_fisheye_param(fixparam, lensName, fixMode);
				}
				IMAGE_LEAVE_CRITICAL();
				if(netsdk->imageChanged){
					ST_NSDK_IMAGE image;
					if(NETSDK_conf_image_get(&image) && ret == kNSDK_INS_RET_OK){
						if(0 == netsdk->imageChanged(&image)){
							ret = kNSDK_INS_RET_OK;
						}else{
							ret = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}
				}else{
					ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
				}
				
				if(kNSDK_INS_RET_OK == ret){
					// save to file
					NETSDK_conf_image_save2();
				}
			}else{
				IMAGE_LEAVE_CRITICAL();
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}
		}
	}	
	return ret;
}


static int image_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT inputRefJSON, LP_JSON_OBJECT inputDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;

	if(HTTP_IS_GET(context)){
		snprintf(content, contentMax, "%s", json_object_to_json_string(inputDupJSON));
		ret = kNSDK_INS_RET_CONTENT_READY;
	}
	else if(HTTP_IS_PUT(context)){
		if(0 == IMAGE_ENTER_CRITICAL()){
			if(formJSON != NULL){
				LP_JSON_OBJECT irCutFilterJSON = NETSDK_json_get_child(inputRefJSON, "irCutFilter");
				LP_JSON_OBJECT manualSharpnessJSON = NETSDK_json_get_child(inputRefJSON, "manualSharpness");
				LP_JSON_OBJECT denoise3dJSON = NETSDK_json_get_child(inputRefJSON, "denoise3d");
				LP_JSON_OBJECT wdrJSON = NETSDK_json_get_child(inputRefJSON, "WDR");
				LP_JSON_OBJECT videoModeJSON = NETSDK_json_get_child(inputRefJSON, "videoMode");

				LP_JSON_OBJECT formirCutFilterJSON = NETSDK_json_get_child(formJSON, "irCutFilter");
				LP_JSON_OBJECT formmanualSharpnessJSON = NETSDK_json_get_child(formJSON, "manualSharpness");
				LP_JSON_OBJECT formdenoise3dJSON = NETSDK_json_get_child(formJSON, "denoise3d");
				LP_JSON_OBJECT formwdrJSON = NETSDK_json_get_child(formJSON, "WDR");
				LP_JSON_OBJECT formvideoModeJSON = NETSDK_json_get_child(formJSON, "videoMode");

				NETSDK_json_copy_child(formJSON,inputRefJSON,"sceneMode");
				NETSDK_json_copy_child(formJSON,inputRefJSON,"exposureMode");
				NETSDK_json_copy_child(formJSON,inputRefJSON,"awbMode");
				NETSDK_json_copy_child(formJSON,inputRefJSON,"lowlightMode");
				NETSDK_json_copy_child(formJSON,inputRefJSON,"imageStyle");
				NETSDK_json_copy_child(formJSON,inputRefJSON,"lghtNhbtdMode");
				NETSDK_json_copy_child(formJSON,inputRefJSON,"BLcompensationMode");

				if(irCutFilterJSON != NULL && formirCutFilterJSON != NULL){
					NETSDK_json_copy_child(formirCutFilterJSON, irCutFilterJSON, "irCutControlMode");
					NETSDK_json_copy_child(formirCutFilterJSON, irCutFilterJSON, "irCutMode");
				}
				if(formmanualSharpnessJSON != NULL && manualSharpnessJSON != NULL){
					NETSDK_json_copy_child(formmanualSharpnessJSON, manualSharpnessJSON, "enabled");
					NETSDK_json_copy_child(formmanualSharpnessJSON, manualSharpnessJSON, "sharpnessLevel");
				}
				if(formdenoise3dJSON!= NULL && denoise3dJSON!= NULL){
					NETSDK_json_copy_child(formdenoise3dJSON, denoise3dJSON, "enabled");
					NETSDK_json_copy_child(formdenoise3dJSON, denoise3dJSON, "denoise3dStrength");
				}
				if(formwdrJSON!= NULL && wdrJSON!= NULL){
					NETSDK_json_copy_child(formwdrJSON, wdrJSON, "enabled");
					NETSDK_json_copy_child(formwdrJSON, wdrJSON, "WDRStrength");			
				}

				if(formvideoModeJSON!= NULL && videoModeJSON!= NULL){
					NETSDK_json_copy_child(formvideoModeJSON, videoModeJSON, "fixMode");
					NETSDK_json_copy_child(formvideoModeJSON, videoModeJSON, "wallMode");	
					NETSDK_json_copy_child(formvideoModeJSON, videoModeJSON, "cellMode");		
					NETSDK_json_copy_child(formvideoModeJSON, videoModeJSON, "tableMode");		
				}

				IMAGE_LEAVE_CRITICAL();	
				if(netsdk->imageChanged){
					ST_NSDK_IMAGE image;
					if(NETSDK_conf_image_get(&image)){
						if(0 == netsdk->imageChanged(&image)){
							ret = kNSDK_INS_RET_OK;
						}else{
							ret = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}
				}else{
					ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
				}
				
				if(kNSDK_INS_RET_OK == ret){
					// save to file
					NETSDK_conf_image_save2();
				}
			}else{
				IMAGE_LEAVE_CRITICAL();
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}		
		}
	}	
	return ret;
}


static LP_NSDK_IMAGE netsdk_conf_image(bool set_flag, LP_NSDK_IMAGE image, bool immediate)
{
	int i = 0;
	char text[128] = {""};
	char *str = NULL;	

	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}

	if(image){
		if(0 == IMAGE_ENTER_CRITICAL()){
			LP_JSON_OBJECT fileJson = json_object_get(netsdk->image_conf);
			LP_JSON_OBJECT imageJson = NETSDK_json_get_child(fileJson, "image");
			LP_JSON_OBJECT irCutFilterJSON = NETSDK_json_get_child(imageJson, "irCutFilter");
			LP_JSON_OBJECT manualSharpnessJSON = NETSDK_json_get_child(imageJson, "manualSharpness");
			LP_JSON_OBJECT denoise3dJSON = NETSDK_json_get_child(imageJson, "denoise3d");
			LP_JSON_OBJECT wdrJSON = NETSDK_json_get_child(imageJson, "WDR");
			LP_JSON_OBJECT videoModeJSON = NETSDK_json_get_child(imageJson, "videoMode");
			if(set_flag){//set
				str = NETSDK_MAP_DEC2STR(sceneMode_map,image->sceneMode,"auto");
				NETSDK_json_set_string2(imageJson, "sceneMode", str);
				str = NETSDK_MAP_DEC2STR(lghtNhbtdMode_map,image->lghtNhbtdMode,"auto");
				NETSDK_json_set_string2(imageJson, "lghtNhbtdMode", str);
				str = NETSDK_MAP_DEC2STR(BLcompensationMode_map,image->BLcompensationMode,"auto");
				NETSDK_json_set_string2(imageJson, "BLcompensationMode", str);
				str = NETSDK_MAP_DEC2STR(exposureMode_map,image->exposureMode,"auto");
				NETSDK_json_set_string2(imageJson, "exposureMode", str);
				str = NETSDK_MAP_DEC2STR(awbMode_map,image->awbMode,"auto");
				NETSDK_json_set_string2(imageJson, "awbMode", str);
				str = NETSDK_MAP_DEC2STR(lowlightMode_map,image->lowlightMode,"close");
				NETSDK_json_set_string2(imageJson, "lowlightMode", str);
				str = NETSDK_MAP_DEC2STR(irCutControlMode_map,image->irCutFilter.irCutControlMode,"hardware");
				NETSDK_json_set_string2(irCutFilterJSON, "irCutControlMode", str);
				str = NETSDK_MAP_DEC2STR(irCutMode_map,image->irCutFilter.irCutMode,"auto");
				NETSDK_json_set_string2(irCutFilterJSON, "irCutMode", str);
				NETSDK_json_set_boolean2(manualSharpnessJSON, "enabled", image->manualSharpness.enabled);
				NETSDK_json_set_int2(manualSharpnessJSON, "sharpnessLevel", image->manualSharpness.sharpnessLevel);
				NETSDK_json_set_boolean2(denoise3dJSON, "enabled", image->denoise3d.enabled);
				NETSDK_json_set_int2(denoise3dJSON, "denoise3dStrength", image->denoise3d.denoise3dStrength);
				NETSDK_json_set_boolean2(wdrJSON, "enabled", image->wdr.enabled);
				NETSDK_json_set_int2(wdrJSON, "WDRStrength", image->wdr.WDRStrength);		
				NETSDK_json_set_int2(imageJson, "imageStyle", image->imageStyle);
				str = NETSDK_MAP_DEC2STR(fixMode_map, image->videoMode.fixType,"cell");
				NETSDK_json_set_string2(videoModeJSON, "fixMode", str);
				switch(image->videoMode.fixType){
					default:
					case eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL:
						str = NETSDK_MAP_DEC2STR(cellMode_map, image->videoMode.showMode,"origin");
						NETSDK_json_set_string2(videoModeJSON, "cellMode", str);
					break;
					case eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL:
						str = NETSDK_MAP_DEC2STR(wallMode_map, image->videoMode.showMode,"origin");
						NETSDK_json_set_string2(videoModeJSON, "wallMode", str);
					break;
					case eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE:
						str = NETSDK_MAP_DEC2STR(tableMode_map, image->videoMode.showMode,"origins");
						NETSDK_json_set_string2(videoModeJSON, "tableMode", str);
					break;
				}
			}else{//get	
				NETSDK_json_get_string(imageJson, "sceneMode", text, sizeof(text));
				image->sceneMode = NETSDK_MAP_STR2DEC(sceneMode_map, text, kNSDK_IMAGE_SCENE_MODE_AUTO);

				NETSDK_json_get_string(imageJson, "lghtNhbtdMode", text, sizeof(text));
				image->lghtNhbtdMode = NETSDK_MAP_STR2DEC(lghtNhbtdMode_map, text, kNSDK_IMAGE_lghtNhbtd_MODE_AUTO);

				NETSDK_json_get_string(imageJson, "BLcompensationMode", text, sizeof(text));
				image->BLcompensationMode = NETSDK_MAP_STR2DEC(BLcompensationMode_map, text, kNSDK_IMAGE_BLcompensation_MODE_AUTO);


				NETSDK_json_get_string(imageJson, "exposureMode", text, sizeof(text));
				image->exposureMode = NETSDK_MAP_STR2DEC(exposureMode_map, text, kNSDK_IMAGE_EXPOSURE_MODE_AUTO);

				NETSDK_json_get_string(imageJson, "awbMode", text, sizeof(text));
				image->awbMode = NETSDK_MAP_STR2DEC(awbMode_map, text, kNSDK_IMAGE_AWB_MODE_AUTO);
				
				NETSDK_json_get_string(imageJson, "lowlightMode", text, sizeof(text));
				image->lowlightMode= NETSDK_MAP_STR2DEC(lowlightMode_map, text, kNSDK_IMAGE_LOWLIGHT_MODE_CLOSE);

				NETSDK_json_get_string(irCutFilterJSON, "irCutControlMode", text, sizeof(text));
				image->irCutFilter.irCutControlMode= NETSDK_MAP_STR2DEC(irCutControlMode_map, text, kNSDK_IMAGE_IRCUT_CONTROL_MODE_HARDWARE);
				
				NETSDK_json_get_string(irCutFilterJSON, "irCutMode", text, sizeof(text));
				image->irCutFilter.irCutMode= NETSDK_MAP_STR2DEC(irCutMode_map, text, kNSDK_IMAGE_IRCUT_MODE_AUTO);
				
				image->manualSharpness.enabled = NETSDK_json_get_boolean(manualSharpnessJSON, "enabled");
				image->manualSharpness.sharpnessLevel = NETSDK_json_get_int(manualSharpnessJSON, "sharpnessLevel");
				image->denoise3d.enabled = NETSDK_json_get_boolean(denoise3dJSON, "enabled");
				image->denoise3d.denoise3dStrength = NETSDK_json_get_int(denoise3dJSON, "denoise3dStrength");
				image->wdr.enabled = NETSDK_json_get_boolean(wdrJSON, "enabled");
				image->wdr.WDRStrength = NETSDK_json_get_int(wdrJSON, "WDRStrength");
				image->imageStyle = NETSDK_json_get_int(imageJson, "imageStyle");

				NETSDK_json_get_string(videoModeJSON, "fixMode", text, sizeof(text));
				image->videoMode.fixType= NETSDK_MAP_STR2DEC(fixMode_map, text, eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE);

				switch(image->videoMode.fixType){
					default:
					case eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL:
						NETSDK_json_get_string(videoModeJSON, "cellMode", text, sizeof(text));
						image->videoMode.showMode = NETSDK_MAP_STR2DEC(cellMode_map, text, eNSDK_IMAGE_FISHEYE_MODE_CELL_ORIGIN);
					break;
					case eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL:
						NETSDK_json_get_string(videoModeJSON, "wallMode", text, sizeof(text));
						image->videoMode.showMode = NETSDK_MAP_STR2DEC(wallMode_map, text, eNSDK_IMAGE_FISHEYE_MODE_WALL_ORIGIN);
					break;
					case eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE:
						NETSDK_json_get_string(videoModeJSON, "tableMode", text, sizeof(text));
						image->videoMode.showMode = NETSDK_MAP_STR2DEC(tableMode_map, text, eNSDK_IMAGE_FISHEYE_MODE_TABLE_ORIGIN);
					break;
				}
			}
			// release
			json_object_put(fileJson);
			fileJson = NULL;
			IMAGE_LEAVE_CRITICAL();
			if(set_flag){
				// save to file
                if(immediate) {
                    NETSDK_conf_image_save();
                }
                else {
                    NETSDK_conf_image_save2();
                }
			}
		//NETSDK_image_dump(image);
		}
		return image;
	}

	return NULL;
}

LP_NSDK_IMAGE NETSDK_conf_image_get(LP_NSDK_IMAGE image)
{
	return netsdk_conf_image(false, image, false);
}

LP_NSDK_IMAGE NETSDK_conf_image_set(LP_NSDK_IMAGE image)
{
	return netsdk_conf_image(true, image, false);
}

LP_NSDK_IMAGE NETSDK_conf_image_set2(LP_NSDK_IMAGE image, bool immediate)
{
	return netsdk_conf_image(true, image, immediate);
}

int NETSDK_image_dump(LP_NSDK_IMAGE image)
{
	printf("sceneMode: %d\r\n"
			"lghtNhbtdMode: %d\r\n"
			"BLcompensationMode: %d\r\n"
			"awbMode: %d\r\n"
			"ircutControlMode: %d\r\n"
			"ircutMode %d\r\n"
			"sharpness enable: %d\r\n"
			"sharpnessLevel: %d\r\n"
			"denoise enable: %d\r\n"
			"denoiseStrength: %d\r\n"
			"WDR enable: %d\r\n"
			"WDRStrength: %d\r\n"
			"exposureMode: %d\r\n"
			"lowlight: %d\r\n",
			"imageStyle: %d\r\n",
			image->sceneMode,
			image->lghtNhbtdMode,
			image->BLcompensationMode,
			image->awbMode, 
			image->irCutFilter.irCutControlMode,
			image->irCutFilter.irCutMode,
			image->manualSharpness.enabled,
			image->manualSharpness.sharpnessLevel,
			image->denoise3d.enabled,
			image->denoise3d.denoise3dStrength,
			image->wdr.enabled,
			image->wdr.WDRStrength,
			image->exposureMode,
			image->lowlightMode, 
			image->imageStyle);
	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////////////////////////////



static int netsdk_image_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT inputRefJSON, LP_JSON_OBJECT inputDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	HTTP_CSTR_t prefix = NULL;
	LP_JSON_OBJECT DupJSON = NULL;
	LP_JSON_OBJECT RefJSON = NULL;
	
	if(HTTP_IS_GET(context) && !NSDK_PROPERTIES(subURI)){
		image_remove_properties(inputDupJSON);
	}
	APP_TRACE("sub uri:%s", subURI);
	if(prefix = "/IRCUTFILTER", 0 == strncmp(prefix, subURI, strlen(prefix))){
		// get all channels
		RefJSON = NETSDK_json_get_child(inputRefJSON, "irCutFilter");
		DupJSON = NETSDK_json_get_child(inputDupJSON, "irCutFilter");
		ret = image_ircutfilter(context, subURI, RefJSON, DupJSON, formJSON, content, contentMax);
	}else if(prefix = "/MANUALSHARPNESS", 0 == strncmp(prefix, subURI, strlen(prefix))){
		RefJSON = NETSDK_json_get_child(inputRefJSON, "manualSharpness");
		DupJSON = NETSDK_json_get_child(inputDupJSON, "manualSharpness");
		ret = image_sharpness(context, subURI, RefJSON, DupJSON, formJSON, content, contentMax);
	}else if(prefix = "/DENOISE3D", 0 == strncmp(prefix, subURI, strlen(prefix))){
		RefJSON = NETSDK_json_get_child(inputRefJSON, "denoise3d");
		DupJSON = NETSDK_json_get_child(inputDupJSON, "denoise3d");
		ret = image_denoise3d(context, subURI, RefJSON, DupJSON, formJSON, content, contentMax);
	}else if(prefix = "/WDR", 0 == strncmp(prefix, subURI, strlen(prefix))){
		RefJSON = NETSDK_json_get_child(inputRefJSON, "WDR");
		DupJSON = NETSDK_json_get_child(inputDupJSON, "WDR");
		ret = image_wdr(context, subURI, RefJSON, DupJSON, formJSON, content, contentMax);
	}else if(prefix = "/AF", 0 == strncmp(prefix, subURI, strlen(prefix))){
		ret = image_autofocus(context, content, contentMax);
	}else if(prefix = "/VIDEOMODE", 0 == strncmp(prefix, subURI, strlen(prefix))){
		RefJSON = NETSDK_json_get_child(inputRefJSON, "videoMode");
		if(RefJSON){
			DupJSON = NETSDK_json_get_child(inputDupJSON, "videoMode");
			ret = image_videoMode(context, subURI, RefJSON, DupJSON, formJSON, content, contentMax);
		}
	}else{
		ret = image_instance(context, subURI, inputRefJSON, inputDupJSON, formJSON, content, contentMax);
	}
	return ret;
}


int NETSDK_image_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI, char *content, int content_max)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	LP_JSON_OBJECT imageJSON = json_object_get(netsdk->image_conf);
	LP_JSON_OBJECT subRefJSON = NETSDK_json_get_child(imageJSON, "image");
	LP_JSON_OBJECT subDupJSON = NETSDK_json_dup(subRefJSON); 
	LP_JSON_OBJECT formJSON = NULL;
	HTTP_CSTR_t prefix = NULL;
	
	if(NULL != context->request_content && 0 != context->request_content_len){
		formJSON = NETSDK_json_parse(context->request_content);
	}
	
	ret = netsdk_image_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, content_max);
	
	// put all the reference of JSON
	if(NULL != formJSON){
		json_object_put(formJSON);
	}

	json_object_put(subDupJSON);
	json_object_put(imageJSON);
	return ret;
}


