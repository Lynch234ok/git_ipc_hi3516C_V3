#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NkUtils/types.h"
#include <NkUtils/assert.h>
#include <base/ja_process.h>
#include "app_debug.h"
#include "generic.h"
#include "netsdk_json.h"
#include "fisheye.h"
#include "model_conf.h"
#include "netsdk_private.h"
#include "app_gsensor.h"

#define FISHEYE_CONFIG_FILE_PATH    "/media/conf/fisheye.json"
#define FISHEYE_LENS_PARAM_FILE_PATH    "/media/custom/lens/"
#define FISHEYE_LENS_INFO_FILE_PATH    "/media/conf/lens_info.json"

static lpFISHEYE_config _fisheye_config = NULL;

typedef struct fisheye_lens_config_t {
    LP_FISHEYE_LENS_PARAM lens_param;
    int lens_param_count;
    pthread_mutex_t mutex;
}st_FISHEYE_LENS_CONFIG, *lp_FISHEYE_LENS_CONFIG;
static st_FISHEYE_LENS_CONFIG _fisheye_lens_config;

static void fisheye_dump()
{
	int i;
	if(_fisheye_config){
		 for(i = 0;i < (sizeof(_fisheye_config->param) /sizeof(_fisheye_config->param[0]));i++){
			printf("param%d:CenterCoordinateX:%d\n", i, _fisheye_config->param[i].CenterCoordinateX);
			printf("param%d:CenterCoordinateY:%d\n", i, _fisheye_config->param[i].CenterCoordinateY);
			printf("param%d:Radius:%d\n", i, _fisheye_config->param[i].Radius);
			printf("param%d:AngleX:%d\n", i, _fisheye_config->param[i].AngleX);
			printf("param%d:AngleY:%d\n", i, _fisheye_config->param[i].AngleY);
			printf("param%d:AngleZ:%d\n", i, _fisheye_config->param[i].AngleZ);

            printf("param%d:CenterCoordinateX:%f\n", i, _fisheye_config->param2[i].CenterCoordinateX);
			printf("param%d:CenterCoordinateY:%f\n", i, _fisheye_config->param2[i].CenterCoordinateY);
			printf("param%d:Radius:%f\n", i, _fisheye_config->param2[i].Radius);
			printf("param%d:AngleX:%f\n", i, _fisheye_config->param2[i].AngleX);
			printf("param%d:AngleY:%f\n", i, _fisheye_config->param2[i].AngleY);
			printf("param%d:AngleZ:%f\n", i, _fisheye_config->param2[i].AngleZ);
        }
	}
}

static NK_Int fisheye_config_load()
{
    LP_JSON_OBJECT obj = json_object_from_file(FISHEYE_CONFIG_FILE_PATH);
    NK_Int i = 0;
	char text[64];

    if((int)obj < 0){
        APP_TRACE("fisheye config load error");
        return -1;
    }

    if(NETSDK_json_check_child(obj, "LensName")) {
        NETSDK_json_get_string(obj, "LensName", _fisheye_config->lensName, sizeof(_fisheye_config->lensName));
    }
    else {
        sprintf(_fisheye_config->lensName, "m109");  // 默认使用m109
    }

	if(NETSDK_json_check_child(obj, "FixMode")) {
		if(NULL != NETSDK_json_get_string(obj, "FixMode", text, sizeof(text))) {
			if(!strcmp(text, "wall")){
				_fisheye_config->fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL;
			}else if(!strcmp(text, "cell")){
				_fisheye_config->fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL;
			}else if(!strcmp(text, "table")){
				_fisheye_config->fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE;
			}else{
				_fisheye_config->fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE;
			}
		}
	}
    LP_JSON_OBJECT fixParam = NETSDK_json_get_child(obj, "FixParam");
    if(fixParam){
        for(i = 0;
                i < (sizeof(_fisheye_config->param) /sizeof(_fisheye_config->param[0]));
                i++)
        {
                LP_JSON_OBJECT param = json_object_array_get_idx(fixParam, i);
                if(param){
                    /* 只判断第一个数据，如果是整形则按int存储，如果是浮点则按float存储 */
                    /* 必须保证文件中两组数据都是同一种类型 */
                    if(json_type_int == json_object_get_type(json_object_object_get(param, "CenterCoordinateX"))) {
                        _fisheye_config->type = true;
                        _fisheye_config->param[i].CenterCoordinateX = NETSDK_json_get_int(param, "CenterCoordinateX");
                        _fisheye_config->param[i].CenterCoordinateY = NETSDK_json_get_int(param, "CenterCoordinateY");
                        _fisheye_config->param[i].Radius = NETSDK_json_get_int(param, "Radius");
                        _fisheye_config->param[i].AngleX = NETSDK_json_get_int(param, "AngleX");
                        _fisheye_config->param[i].AngleY = NETSDK_json_get_int(param, "AngleY");
                        _fisheye_config->param[i].AngleZ = NETSDK_json_get_int(param, "AngleZ");
                    }
                    else {
                        _fisheye_config->type = false;
                        _fisheye_config->param2[i].CenterCoordinateX = NETSDK_json_get_float(param, "CenterCoordinateX");
                        _fisheye_config->param2[i].CenterCoordinateY = NETSDK_json_get_float(param, "CenterCoordinateY");
                        _fisheye_config->param2[i].Radius = NETSDK_json_get_float(param, "Radius");
                        _fisheye_config->param2[i].AngleX = NETSDK_json_get_float(param, "AngleX");
                        _fisheye_config->param2[i].AngleY = NETSDK_json_get_float(param, "AngleY");
                        _fisheye_config->param2[i].AngleZ = NETSDK_json_get_float(param, "AngleZ");
                    }
                }
        }
    }

    return 0;
}

static NK_Int fisheye_lens_param_load(const char *file) {
    LP_JSON_OBJECT obj = json_object_from_file(file);
    NK_Int i = 0;

    if((int)obj < 0){
        APP_TRACE("fisheye lens param load error");
        return -1;
    }

    int len = json_object_array_length(obj);
    _fisheye_lens_config.lens_param_count = len;
    for(i = 0; i < len; i++) {
        LP_JSON_OBJECT param = json_object_array_get_idx(obj, i);
        _fisheye_lens_config.lens_param[i].angle = NETSDK_json_get_float(param, "angle");
        _fisheye_lens_config.lens_param[i].height = NETSDK_json_get_float(param, "height");
    }

    json_object_put(obj);
    obj = NULL;

    return 0;

}

NK_Int FISHEYE_config_init()
{
    NK_Int i = 0;
    int len;
    LP_JSON_OBJECT obj;

    if(!_fisheye_config){
        _fisheye_config = calloc(1, sizeof(stFISHEYE_config));
        if(IS_FILE_EXIST(FISHEYE_CONFIG_FILE_PATH)){
            fisheye_config_load();
            //fisheye_dump();
        }else{
        /* 使用浮点数区别开Px-720和其它单镜头设备 */
#if defined(PX_720)
            _fisheye_config->type = false;
#else
            _fisheye_config->type = true;
#endif
            sprintf(_fisheye_config->lensName, "m109");    // 默认使用m109
			_fisheye_config->fixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE;
			for(i = 0;
                    i < (sizeof(_fisheye_config->param) /sizeof(_fisheye_config->param[0]));
                    i++)
            {
                _fisheye_config->param[i].CenterCoordinateX = 0;
                _fisheye_config->param[i].CenterCoordinateY = 0;
                _fisheye_config->param[i].Radius = 0;
                _fisheye_config->param[i].AngleX = 0;
                _fisheye_config->param[i].AngleY = 0;
                _fisheye_config->param[i].AngleZ = 0;
                _fisheye_config->param2[i].CenterCoordinateX = 0.0;
                _fisheye_config->param2[i].CenterCoordinateY = 0.0;
                _fisheye_config->param2[i].Radius = 0.0;
                _fisheye_config->param2[i].AngleX = 0.0;
                _fisheye_config->param2[i].AngleY = 0.0;
                _fisheye_config->param2[i].AngleZ = 0.0;
            }
        }

        pthread_mutex_init(&_fisheye_config->mutex, NULL);

        char lens_param_file[32];
        sprintf(lens_param_file, "%s%s%s", FISHEYE_LENS_PARAM_FILE_PATH, _fisheye_config->lensName, ".json");
        pthread_mutex_init(&_fisheye_lens_config.mutex, NULL);
        /* fisheye lens param init */
        if(!_fisheye_lens_config.lens_param) {
            if(IS_FILE_EXIST(lens_param_file)) {
                obj = json_object_from_file(lens_param_file);
                if(obj) {
                    len = json_object_array_length(obj);
                    _fisheye_lens_config.lens_param = calloc(len, sizeof(ST_FISHEYE_LENS_PARAM));
                    fisheye_lens_param_load(lens_param_file);
                    json_object_put(obj);
                    obj = NULL;
                }
            }
        }
        //APP_TRACE("fisheye init success");

        return 0;
    }

    APP_TRACE("fisheye has inited");
    return 0;
}

NK_Int FISHEYR_config_destroy()
{
    if(_fisheye_config){
        free(_fisheye_config);
    }
    _fisheye_config = NULL;

    return 0;
}



NK_Int FISHEYE_config_get(lpFISHEYE_config fisheye_config)
{
    NK_Int i = 0;

    if(!_fisheye_config || !fisheye_config){
        APP_TRACE("_fisheye_config not init or fisheye_config null");
        return -1;
    }

    pthread_mutex_lock(&_fisheye_config->mutex);
    sprintf(fisheye_config->lensName, "%s", _fisheye_config->lensName);
	fisheye_config->fixMode = _fisheye_config->fixMode;
    fisheye_config->type = _fisheye_config->type;
    for(i = 0;
            i < (sizeof(_fisheye_config->param) / sizeof(_fisheye_config->param[0]));
            i++)
    {
        fisheye_config->param[i].CenterCoordinateX = _fisheye_config->param[i].CenterCoordinateX;
        fisheye_config->param[i].CenterCoordinateY = _fisheye_config->param[i].CenterCoordinateY;
        fisheye_config->param[i].Radius = _fisheye_config->param[i].Radius;
        fisheye_config->param[i].AngleX = _fisheye_config->param[i].AngleX;
        fisheye_config->param[i].AngleY = _fisheye_config->param[i].AngleY;
        fisheye_config->param[i].AngleZ = _fisheye_config->param[i].AngleZ;

        fisheye_config->param2[i].CenterCoordinateX = _fisheye_config->param2[i].CenterCoordinateX;
        fisheye_config->param2[i].CenterCoordinateY = _fisheye_config->param2[i].CenterCoordinateY;
        fisheye_config->param2[i].Radius = _fisheye_config->param2[i].Radius;
        fisheye_config->param2[i].AngleX = _fisheye_config->param2[i].AngleX;
        fisheye_config->param2[i].AngleY = _fisheye_config->param2[i].AngleY;
        fisheye_config->param2[i].AngleZ = _fisheye_config->param2[i].AngleZ;
    }
	//fisheye_dump();
    pthread_mutex_unlock(&_fisheye_config->mutex);

    return 0;
}

/* 参数二，true表示set int，false 表示set float */
NK_Int FISHEYE_config_set(lpFISHEYE_config fisheye_config, bool type)
{
    NK_Char cmdLine[128] = {0};

    if(!_fisheye_config || !fisheye_config){
        APP_TRACE("_fisheye_config not init or fisheye_config null");
        return -1;
    }
    pthread_mutex_lock(&_fisheye_config->mutex);
    if(!IS_FILE_EXIST(FISHEYE_CONFIG_FILE_PATH)){
        memset(cmdLine, 0, sizeof(cmdLine));
        snprintf(cmdLine, sizeof(cmdLine), "touch %s", FISHEYE_CONFIG_FILE_PATH);
        NK_SYSTEM(cmdLine);
    }else{
        memset(cmdLine, 0, sizeof(cmdLine));
        snprintf(cmdLine, sizeof(cmdLine), "rm -rf %s", FISHEYE_CONFIG_FILE_PATH);
        NK_SYSTEM(cmdLine);
        pthread_mutex_unlock(&_fisheye_config->mutex);

        return FISHEYE_config_set(fisheye_config, type);
    }

    LP_JSON_OBJECT obj = json_object_new_object();
    LP_JSON_OBJECT fixParam = json_object_new_array();
    LP_JSON_OBJECT param =NULL;
    NK_Int i = 0;

    NETSDK_json_set_string2(obj, "LensName", fisheye_config->lensName);

	if(eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL == fisheye_config->fixMode){
		NETSDK_json_set_string2(obj,"FixMode","wall");
	}else if(eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL == fisheye_config->fixMode){
		NETSDK_json_set_string2(obj,"FixMode","cell");
	}else if(eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE == fisheye_config->fixMode){
		NETSDK_json_set_string2(obj,"FixMode","table");
	}else{
		NETSDK_json_set_string2(obj,"FixMode","none");
	}

    for(i = 0;
            i < (sizeof(_fisheye_config->param) / sizeof(_fisheye_config->param[0]));
            i++)
    {
        param = json_object_new_object();
        NETSDK_json_set_int2(param, "id", i);
        if(type) {
            NETSDK_json_set_int2(param, "CenterCoordinateX", fisheye_config->param[i].CenterCoordinateX);
            NETSDK_json_set_int2(param, "CenterCoordinateY", fisheye_config->param[i].CenterCoordinateY);
            NETSDK_json_set_int2(param, "Radius", fisheye_config->param[i].Radius);
            NETSDK_json_set_int2(param, "AngleX", fisheye_config->param[i].AngleX);
            NETSDK_json_set_int2(param, "AngleY", fisheye_config->param[i].AngleY);
            NETSDK_json_set_int2(param, "AngleZ", fisheye_config->param[i].AngleZ);
        }
        else {
            NETSDK_json_set_float2(param, "CenterCoordinateX", fisheye_config->param2[i].CenterCoordinateX);
            NETSDK_json_set_float2(param, "CenterCoordinateY", fisheye_config->param2[i].CenterCoordinateY);
            NETSDK_json_set_float2(param, "Radius", fisheye_config->param2[i].Radius);
            NETSDK_json_set_float2(param, "AngleX", fisheye_config->param2[i].AngleX);
            NETSDK_json_set_float2(param, "AngleY", fisheye_config->param2[i].AngleY);
            NETSDK_json_set_float2(param, "AngleZ", fisheye_config->param2[i].AngleZ);
        }
        json_object_array_put_idx(fixParam, i, param);
    }
    json_object_object_add(obj, "FixParam", fixParam);
    json_object_to_file(FISHEYE_CONFIG_FILE_PATH, obj);
    //fixme, is it need free?
    if(obj){
		json_object_put(obj);
	}
	obj = NULL;

    fisheye_config_load();
    pthread_mutex_unlock(&_fisheye_config->mutex);

    return 0;
}

NK_Int FISHEYE_lens_param_len_get() {

    NK_Int len;

    pthread_mutex_lock(&_fisheye_lens_config.mutex);
    len = _fisheye_lens_config.lens_param_count;
    pthread_mutex_unlock(&_fisheye_lens_config.mutex);

    return len;

}

LP_FISHEYE_LENS_PARAM FISHEYE_lens_param_get() {

    if(!_fisheye_lens_config.lens_param) {
        APP_TRACE("_fisheye_lens_config.lens_param not init or lens_param null");
        return NULL;
    }

    return _fisheye_lens_config.lens_param;
}

/* 获取安装方式，统一从此接口获取 */
/* 安装方式获取优先级 产测fisheye.json -> sensor判断(model_conf) -> netsdk_image */
EM_NSDK_IMAGE_FISHEYE_FIX_MODE FISHEYE_get_fix_mode(void)
{
	ST_NSDK_IMAGE image;
	ST_MODEL_CONF model_conf;
	int tmpFlag = -1;  // 临时标志
	EM_NSDK_IMAGE_FISHEYE_FIX_MODE retFixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL;

#ifdef GSENSOR
        int angle_z = 0;
        stGsensor_angles angles;
        if(0 == APP_GSENSOR_get_angles(&angles)) {
            angle_z = abs(angles.angle_z);
            if(angle_z >= 40) {
                retFixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL;
            }
            else {
                retFixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL;
            }
            return retFixMode;
        }

#endif

	// fisheye获取为none 则使用sensor, 如果sensor也没有则找netsdk_image,最终没有才没有
	if(_fisheye_config) {
		pthread_mutex_lock(&_fisheye_config->mutex);
		if((_fisheye_config->fixMode != eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE) && (tmpFlag == -1)) {
			retFixMode = _fisheye_config->fixMode;
			tmpFlag = 1;
		}
		pthread_mutex_unlock(&_fisheye_config->mutex);
	}
	if((MODEL_CONF_get(&model_conf) != NULL) && (tmpFlag == -1)) {
		retFixMode = model_conf.fixmode;
		tmpFlag = 1;
	}
	if((NETSDK_conf_image_get(&image) != NULL) && (tmpFlag == -1)) {
		retFixMode = image.videoMode.fixType;
		tmpFlag = 1;
	}
	if(tmpFlag == -1) {
		retFixMode = eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE;
		tmpFlag = 1;
	}

	return retFixMode;

}

