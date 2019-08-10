
#include "model_conf.h"
#include "app_debug.h"
#include "netsdk_private.h"
#include "netsdk.h"
#include "sensor.h"
#include "custom.h"
#include "global_runtime.h"
#include "app_gsensor.h"
#include "app_bluetooth.h"

#define MODEL_CONF_FILE_DIR "/media/custom/model_conf"

#define MODEL_CONF_FILE_100 "100.json"
#define MODEL_CONF_FILE_130 "130.json"
#define MODEL_CONF_FILE_200 "200.json"
#define MODEL_CONF_FILE_300 "300.json"
#define MODEL_CONF_FILE_400 "400.json"
#define MODEL_CONF_FILE_500 "500.json"
#define MODEL_CONF_FILE_400_imx326 "400_imx326.json"
#define MODEL_CONF_FILE_F5_300 "f5_300.json"


LP_MODEL_CONF model_conf_attr = NULL;

static char * strupr(char *sOrigin)
{
	char *pRead = sOrigin;
	while (*pRead)
	{
		if (*pRead>=0x61 && *pRead <= 0x7A)
		{
			*pRead -= 0x20;
		}
		pRead ++;
	}
	return NULL;
}

static uint32_t str2hex(char* str)
{
	uint32_t var=0;
	uint32_t t;
	
	if (var > 8){
		return -1;
	}
	strupr(str);
	for (; *str; str++){
		if (*str>='A' && *str <='F'){
			t = *str-55;//a-fÖ®¼äµÄasciiÓë¶ÔÓ¦ÊýÖµÏà²î55Èç'A'Îª65,65-55¼´ÎªA
		}else{
			t = *str-48;
		}
		var<<=4;
		var|=t;
	}
	return var;
}

static void model_conf_dump(LP_MODEL_CONF model_conf)
{
	if(model_conf){
		//APP_TRACE("model_conf->video[0].bps:%d", model_conf->video[0].bps);
		//APP_TRACE("model_conf->video[0].fps:%d", model_conf->video[0].fps);
		//APP_TRACE("model_conf->video[0].resolution:%x", model_conf->video[0].resolution);
		//APP_TRACE("model_conf->video[0].resolutionProperty.opt:%s", model_conf->video[0].resolutionProperty.opt);
		//APP_TRACE("model_conf->video[1].bps:%d", model_conf->video[1].bps);
		//APP_TRACE("model_conf->video[1].fps:%d", model_conf->video[1].fps);
		//APP_TRACE("model_conf->video[1].resolution:%x", model_conf->video[1].resolution);
		//APP_TRACE("model_conf->video[1].resolutionProperty.opt:%s", model_conf->video[1].resolutionProperty.opt);
		//APP_TRACE("model_conf->fixmode:%d", model_conf->fixmode);
		APP_TRACE("model_conf->snumber.chipModel:0x%x", model_conf->snumber.chipModel);
		APP_TRACE("model_conf->snumber.productModel:0x%x", model_conf->snumber.productModel);
		APP_TRACE("model_conf->modelName:%s", model_conf->modelName);
        //APP_TRACE("model_conf->ircutFilter.irCutControlMode:%d", model_conf->ircutFilter.irCutControlMode);
	}else{
		APP_TRACE("media conf is NULL!");
	}
}

static LP_JSON_OBJECT model_conf_video_find_channel(LP_JSON_OBJECT channels, int id)
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

static LP_JSON_OBJECT model_conf_load_json(const char *fileName)
{
	LP_JSON_OBJECT confJSON = NULL;

	confJSON = NETSDK_json_load(fileName);
	if(NULL != confJSON){
		return confJSON;
	}else{

	}
	return NULL;
}

static int model_conf_json_video_parse(LP_JSON_OBJECT json, LP_MODEL_CONF model_conf, int stream)
{
	const ST_NSDK_MAP_STR_DEC resolution_map[] = {
		{"2592x1944", kNSDK_RES_2592X1944}, 
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

	char text[32];
	if(json){
		model_conf->video[stream-1].bps= NETSDK_json_get_int(json, "Bps");
		model_conf->video[stream-1].fps= NETSDK_json_get_int(json, "Fps");
		if(NETSDK_json_get_string(json, "Resolution", text, sizeof(text))){
			// get resolution property
			LP_JSON_OBJECT propertyJSON = NETSDK_json_get_child(json, "ResolutionProperty");
			if(NULL != propertyJSON){
				LP_JSON_OBJECT optionJSON = NETSDK_json_get_child(propertyJSON, "opt");
				if(NULL != optionJSON){
					snprintf(model_conf->video[stream-1].resolutionProperty.opt, 
					sizeof(model_conf->video[stream-1].resolutionProperty.opt), 
					"%s",
					json_object_to_json_string(optionJSON));
				}
			}
			model_conf->video[stream-1].resolution= NETSDK_MAP_STR2DEC(resolution_map,text,kNSDK_RES_640X360);
		}else{
			APP_TRACE("Parse json error!");
			goto parse_error;
		}
	}else{
		APP_TRACE("Parse json error!");
		goto parse_error;
	}
	return 0;

parse_error:
	return -1;

}

static LP_MODEL_CONF model_conf_json_parse(LP_JSON_OBJECT json, LP_MODEL_CONF model_conf)
{
	char text[32];
	LP_JSON_OBJECT sn_json, video_json, video_channel_main, video_channel_sub, audio_json, osd_json;
	const ST_NSDK_MAP_STR_DEC fixMode_map[] = {
		{"wall", eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL},
		{"cell", eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL},
		{"table", eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE},
		{"none", eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE},
	};
    const ST_NSDK_MAP_STR_DEC irCutControlMode_map[] = {
        {"hardware", kNSDK_IMAGE_IRCUT_CONTROL_MODE_HARDWARE},
        {"software", kNSDK_IMAGE_IRCUT_CONTROL_MODE_SOFTWARE},
    };
	
	NETSDK_json_get_string(json, "FixMode", text, sizeof(text));
	model_conf->fixmode = NETSDK_MAP_STR2DEC(fixMode_map, text, eNSDK_IMAGE_FISHEYE_FIX_MODE_NONE);

	if(NULL == NETSDK_json_get_string(json, "ModelName", model_conf->modelName, sizeof(model_conf->modelName))){
		snprintf(model_conf->modelName, sizeof(model_conf->modelName), "IPCAM");
	}
	
	sn_json = NETSDK_json_get_child(json, "SN");
	video_json = NETSDK_json_get_child(json, "Video.videoChannel");
	audio_json = NETSDK_json_get_child(json, "Audio");
	osd_json = NETSDK_json_get_child(json, "OSD");
	
	if(sn_json){
		NETSDK_json_get_string(sn_json, "ChipModel", text, sizeof(text));
		model_conf->snumber.chipModel = str2hex(text);
		NETSDK_json_get_string(sn_json, "ProductModel", text, sizeof(text));
		model_conf->snumber.productModel= str2hex(text);
	}else{
		APP_TRACE("Parse json error!");
		goto parse_error;
	}
	
	if(video_json){
		video_channel_main = model_conf_video_find_channel(video_json, MODEL_CONF_VIDEO_MAIN_STREAM_ID); // ä¸»ç æµ
		video_channel_sub = model_conf_video_find_channel(video_json, MODEL_CONF_VIDEO_SUB_STREAM_ID);   // å­ç æµï¼ˆç”¨æ¥æŠ“å›¾ï¼Œå°ºå¯¸ç›¸å¯¹è¾ƒå°ï¼‰
		//main stream
		if(-1 == model_conf_json_video_parse(video_channel_main, model_conf, MODEL_CONF_VIDEO_MAIN_STREAM_ID)){
			APP_TRACE("Parse json error!");
			goto parse_error;
		}
		//sub stream
		if(-1 == model_conf_json_video_parse(video_channel_sub, model_conf, MODEL_CONF_VIDEO_SUB_STREAM_ID)){
			APP_TRACE("Parse json error!");
			goto parse_error;
		}
		
	}else{
		APP_TRACE("Parse json error!");
		goto parse_error;
	}

	if(osd_json){
        if(json_object_object_get(osd_json, "osdStreamNum") == NULL) {
            model_conf->osd.osd_stream_num = -1;
        }
        else {
            model_conf->osd.osd_stream_num = NETSDK_json_get_int(osd_json, "osdStreamNum");
        }

        if(json_object_object_get(osd_json, "osdRadio") == NULL) {
            model_conf->osd.osd_radio = -1;
        }
        else {
            model_conf->osd.osd_radio= NETSDK_json_get_int(osd_json, "osdRadio");
        }

        if(json_object_object_get(osd_json, "osdTimeX") == NULL) {
            model_conf->osd.timeX = -1;
        }
        else {
            model_conf->osd.timeX = NETSDK_json_get_int(osd_json, "osdTimeX");
        }

        if(json_object_object_get(osd_json, "osdTimeY") == NULL) {
            model_conf->osd.timeY = -1;
        }
        else {
            model_conf->osd.timeY = NETSDK_json_get_int(osd_json, "osdTimeY");
        }
	}else{
		model_conf->osd.osd_stream_num = -1;
		model_conf->osd.osd_radio = -1;
		model_conf->osd.timeX = -1;
		model_conf->osd.timeY = -1;
	}

	if(audio_json){

	}else{
		APP_TRACE("Parse json error!");
		goto parse_error;
	}	

	NETSDK_json_get_string(json, "irCutControlMode", text, sizeof(text));
	model_conf->ircutFilter.irCutControlMode = NETSDK_MAP_STR2DEC(irCutControlMode_map, text, kNSDK_IMAGE_IRCUT_CONTROL_MODE_SOFTWARE);

	return model_conf;
parse_error:
	return NULL;
}

static int model_conf_load(const char *fileName, LP_MODEL_CONF model_conf)
{
	LP_JSON_OBJECT json_conf =  model_conf_load_json(fileName);
	if(NULL != json_conf){
		if(NULL != model_conf_json_parse(json_conf, model_conf)){
			return 0;
		}
	}
	
	return -1;
}

static void model_conf_make_file_path(emSENSOR_MODEL sensor_type, char *file_path)
{
	char *file = NULL;
	switch(sensor_type){
		default:
		case SENSOR_MODEL_SC1145:
		case SENSOR_MODEL_SC1045:
			file = MODEL_CONF_FILE_100;
			break;
		case SENSOR_MODEL_APTINA_AR0130:
		case SENSOR_MODEL_SC1035:
		case SENSOR_MODEL_IMX225:
		case SENSOR_MODEL_SC1135:
		case SENSOR_MODEL_SC1235:
			file = MODEL_CONF_FILE_130;
			break;
		case SENSOR_MODEL_SMARTSENS_SC2045:
		case SENSOR_MODEL_SMARTSENS_SC2035:
		case SENSOR_MODEL_OV2710:
		case SENSOR_MODEL_APTINA_AR0237:
		case SENSOR_MODEL_SC2135:
		case SENSOR_MODEL_IMX291:
		case SENSOR_MODEL_PS5230:
		case SENSOR_MODEL_SC2235:
		case SENSOR_MODEL_SC2232:
        case SENSOR_MODEL_IMX307:
			file = MODEL_CONF_FILE_200;
			break;
		case SENSOR_MODEL_APTINA_AR0330:
		case SENSOR_MODEL_SC3035:
		case SENSOR_MODEL_PS5270: {
#if defined(GSENSOR)
            if(0 == APP_GSENSOR_start()) {
                file = MODEL_CONF_FILE_F5_300;
                break;
            }
#endif
			file = MODEL_CONF_FILE_300;
			break;
        }
		case SENSOR_MODEL_OV_OV4689:
			file = MODEL_CONF_FILE_400;
			break;
		case SENSOR_MODEL_IMX326:
			file = MODEL_CONF_FILE_400_imx326;
			break;
		case SENSOR_MODEL_SONY_IMX178:
		case SENSOR_MODEL_OV5658:
		case SENSOR_MODEL_OS05A:
			file = MODEL_CONF_FILE_500;
			break;
		
	}

    /* ¸ù¾Ýproduct typeÈ·¶¨model_confÂ·¾¶ */
    /* Ã»ÓÐ¶Áµ½product type£¬ÔòÄ¬ÈÏ MODEL_CONF_FILE_DIR/Px */
	if(NK_False == GLOBAL_sn_front()){
		 sprintf(file_path, "%s/Cx/%s", MODEL_CONF_FILE_DIR, file);
	}else{
		 sprintf(file_path, "%s/Px/%s", MODEL_CONF_FILE_DIR, file);
	}
	//APP_TRACE("model config file:%s", file_path);
}

LP_MODEL_CONF MODEL_CONF_get(LP_MODEL_CONF model_conf)
{
	if(model_conf && model_conf_attr){
		memcpy(model_conf, model_conf_attr, sizeof(ST_MODEL_CONF));
	}
	return model_conf_attr;
}

bool MODEL_CONF_check_int_valid(int param)
{
	return (-1 == param)? false:true;
}

LP_MODEL_CONF MODEL_CONF_init(int sensor_type)
{
	char file_path[128];
	int ret = 0;
	if(NULL == model_conf_attr){
		model_conf_attr = (LP_MODEL_CONF)calloc(sizeof(ST_MODEL_CONF), 1);
		model_conf_make_file_path((emSENSOR_MODEL)sensor_type, file_path);
		ret = model_conf_load(file_path, model_conf_attr);
		if((0 != ret) && (NULL != model_conf_attr)){
			APP_TRACE("LOAD file failed!");
			APP_TRACE("file_path =%sLOAD file failed!",file_path);
			
			free(model_conf_attr);
			model_conf_attr = NULL;
		}
		model_conf_dump(model_conf_attr);
	}
	return model_conf_attr;
}

int MODEL_CONF_destory()
{
	if(model_conf_attr){
		free(model_conf_attr);
		model_conf_attr = NULL;
		return 0;
	}
	return -1;
}

