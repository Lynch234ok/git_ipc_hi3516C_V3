#if defined(GSENSOR)

#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdbool.h>
#include <pthread.h>
#include <app_debug.h>
#include <json/json.h>
#include <gsensor.h>
#include "socket_tcp.h"
#include "global_runtime.h"
#include "app_gsensor.h"

#define GSENSOR_CALIBRATION_ANGLES_FILE     "/media/conf/gsensor_calibration_angles"

typedef struct {

    bool triger;
    pthread_t pid;
    pthread_mutex_t init_lock;

    bool calibration;
} stGsensor_params;

static stGsensor_params gsensor_params = {
    .triger = false,
    .pid = (pthread_t)NULL,
    .init_lock = PTHREAD_MUTEX_INITIALIZER,
    .calibration = false,
};

static stGsensor_angles cur_gsensor_angles = {
    .angle_x = 0.0,
    .angle_y = 0.0,
    .angle_z = 0.0,
};

int APP_GSENSOR_get_angles(lpGsensor_angles angles)
{
    double cur_angle_x, cur_angle_y, cur_angle_z;

    if(GSENSOR_is_init()) {
        GSENSOR_get_angles(&cur_angle_x, &cur_angle_y, &cur_angle_z);
        angles->angle_x = cur_angle_x;
        angles->angle_y = cur_angle_y;
        angles->angle_z = cur_angle_z;
    }
    else {
        return -1;
    }

    return 0;

}

static void *gsensor_work(void *arg)
{
    double cur_angle_x, cur_angle_y, cur_angle_z;

    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "gsensor_work");

    while(gsensor_params.triger) {

        GSENSOR_get_angles(&cur_angle_x, &cur_angle_y, &cur_angle_z);
        pthread_mutex_lock(&gsensor_params.init_lock);

        cur_gsensor_angles.angle_x = cur_angle_x;
        cur_gsensor_angles.angle_y = cur_angle_y;
        cur_gsensor_angles.angle_z = cur_angle_z;

        pthread_mutex_unlock(&gsensor_params.init_lock);
        //sleep(1);
    }

    return NULL;

}

int APP_GSENSOR_get_to_json(struct json_object *gsensorJson, bool properties_flag)
{
    struct json_object *tmpJson = NULL;
    struct json_object *calibrationJson = NULL;
    struct json_object *anglesJson = NULL;
    struct json_object *propertiesJson = NULL;
    struct json_object *optArrJson;
    double cur_angle_x = 0.0, cur_angle_y = 0.0, cur_angle_z = 0.0;
    double minAngleVal = -90.0, maxAngleVal = 90.0;
    int ret = emAPP_GSENSOR_ERR_CODE_OK;

    if(NULL != gsensorJson) {
        if(GSENSOR_is_init()) {
            GSENSOR_get_angles(&cur_angle_x, &cur_angle_y, &cur_angle_z);
        }
        else {
            APP_TRACE("Gsensor not support");
            return emAPP_GSENSOR_ERR_CODE_NOT_SUPPORT;
        }

        // version
        tmpJson = json_object_new_string("1.0.0");  // 报文版本号
        json_object_object_add(gsensorJson, "version", tmpJson);
        if(properties_flag) {
            propertiesJson = json_object_new_object();
            if(NULL == propertiesJson) {
                APP_TRACE("Failed to create json object");
                goto ERROR_RETURN;
            }
            tmpJson = json_object_new_string("string");
            json_object_object_add(propertiesJson, "type", tmpJson);
            tmpJson = json_object_new_string("ro");
            json_object_object_add(propertiesJson, "mode", tmpJson);
            tmpJson = json_object_new_int(64);
            json_object_object_add(propertiesJson, "max", tmpJson);
            json_object_object_add(gsensorJson, "versionProperty", propertiesJson);
        }

        // calibration mode
        calibrationJson = json_object_new_object();
        if(NULL == calibrationJson) {
            APP_TRACE("Failed to create json object");
            goto ERROR_RETURN;
        }
        tmpJson = json_object_new_boolean(gsensor_params.calibration);
        json_object_object_add(calibrationJson, "enabled", tmpJson);
        if(properties_flag) {
            propertiesJson = json_object_new_object();
            if(NULL == propertiesJson) {
                APP_TRACE("Failed to create json object");
                goto ERROR_RETURN;
            }
            tmpJson = json_object_new_string("boolean");
            json_object_object_add(propertiesJson, "type", tmpJson);
            tmpJson = json_object_new_string("rw");
            json_object_object_add(propertiesJson, "mode", tmpJson);
            optArrJson = json_object_new_array();
            if(NULL != optArrJson) {
                tmpJson = json_object_new_boolean(true);
                json_object_array_add(optArrJson, tmpJson);
                tmpJson = json_object_new_boolean(false);
                json_object_array_add(optArrJson, tmpJson);
                json_object_object_add(propertiesJson, "opt", optArrJson);
            }
            json_object_object_add(calibrationJson, "calibrationModeProperty", propertiesJson);
        }
        json_object_object_add(gsensorJson, "calibration", calibrationJson);

        // angles
        anglesJson = json_object_new_object();
        if(NULL != anglesJson) {
            tmpJson = json_object_new_double(cur_angle_x);
            json_object_object_add(anglesJson, "anglesX", tmpJson);
            if(properties_flag) {
                propertiesJson = json_object_new_object();
                if(NULL == propertiesJson) {
                    APP_TRACE("Failed to create json object");
                    goto ERROR_RETURN;
                }
                tmpJson = json_object_new_string("double");
                json_object_object_add(propertiesJson, "type", tmpJson);
                tmpJson = json_object_new_string("rw");
                json_object_object_add(propertiesJson, "mode", tmpJson);
                tmpJson = json_object_new_double(minAngleVal);
                json_object_object_add(propertiesJson, "min", tmpJson);
                tmpJson = json_object_new_double(maxAngleVal);
                json_object_object_add(propertiesJson, "max", tmpJson);
                json_object_object_add(anglesJson, "anglesXProperty", propertiesJson);
            }
            tmpJson = json_object_new_double(cur_angle_y);
            json_object_object_add(anglesJson, "anglesY", tmpJson);
            if(properties_flag) {
                propertiesJson = json_object_new_object();
                if(NULL == propertiesJson) {
                    APP_TRACE("Failed to create json object");
                    goto ERROR_RETURN;
                }
                tmpJson = json_object_new_string("double");
                json_object_object_add(propertiesJson, "type", tmpJson);
                tmpJson = json_object_new_string("rw");
                json_object_object_add(propertiesJson, "mode", tmpJson);
                tmpJson = json_object_new_double(minAngleVal);
                json_object_object_add(propertiesJson, "min", tmpJson);
                tmpJson = json_object_new_double(maxAngleVal);
                json_object_object_add(propertiesJson, "max", tmpJson);
                json_object_object_add(anglesJson, "anglesYProperty", propertiesJson);
            }
            tmpJson = json_object_new_double(cur_angle_z);
            json_object_object_add(anglesJson, "anglesZ", tmpJson);
            if(properties_flag) {
                propertiesJson = json_object_new_object();
                if(NULL == propertiesJson) {
                    APP_TRACE("Failed to create json object");
                    goto ERROR_RETURN;
                }
                tmpJson = json_object_new_string("double");
                json_object_object_add(propertiesJson, "type", tmpJson);
                tmpJson = json_object_new_string("rw");
                json_object_object_add(propertiesJson, "mode", tmpJson);
                tmpJson = json_object_new_double(minAngleVal);
                json_object_object_add(propertiesJson, "min", tmpJson);
                tmpJson = json_object_new_double(maxAngleVal);
                json_object_object_add(propertiesJson, "max", tmpJson);
                json_object_object_add(anglesJson, "anglesZProperty", propertiesJson);
            }
            json_object_object_add(gsensorJson, "angles", anglesJson);
        }
        else {
            APP_TRACE("Failed to create json object");
            goto ERROR_RETURN;
        }
    }
    else {
        ret = -1;
    }

    return ret;

ERROR_RETURN:

    if(NULL != anglesJson) {
        json_object_put(anglesJson);
    }

    if(NULL != propertiesJson) {
        json_object_put(propertiesJson);
    }
    if(NULL != calibrationJson) {
        json_object_put(calibrationJson);
    }

    return emAPP_GSENSOR_ERR_CODE_ERROR;

}

static int app_gsensor_save_calibration_to_file(char *file, int8_t cor_x, int8_t cor_y, int8_t cor_z)
{
	unsigned char crc;
	char crcStr[8] = {0};
    char anglesStr[64] = {0};
	FILE *fp = NULL;


	fp = fopen(file, "w");
	if (NULL == fp) {
		APP_TRACE("Open file %s error !", file);
		return -1;
	}

    memset(crcStr, 0, sizeof(crcStr));
    memset(anglesStr, 0, sizeof(anglesStr));

    sprintf(anglesStr, "%d|%d|%d", cor_x, cor_y, cor_z);
	crc = CRC_getByteCRC(anglesStr, strlen(anglesStr));
	sprintf(crcStr, "%d\n", crc);
	fwrite(crcStr, 1, sizeof(crcStr), fp);
	fwrite(anglesStr, 1, strlen(anglesStr), fp);
	fsync(fp);
	fclose(fp);

	return 0;

}

static int app_gsensor_get_calibration_to_file(const char *file, int8_t *cor_x, int8_t *cor_y, int8_t *cor_z)
{
	FILE *fp = NULL;
	char readData[8];
    char anglesStr[64];
	unsigned char crc1, crc2;
	int ret = 0;

	fp = fopen(file, "r");
	if (NULL == fp) {
		APP_TRACE("Open file %s error !", file);
		return -1;
	}

    memset(readData, 0, sizeof(readData));
    memset(anglesStr, 0, sizeof(anglesStr));

	fread(readData, 1, sizeof(readData), fp);
	fread(anglesStr, 1, sizeof(anglesStr), fp);
    fclose(fp);
	crc1 = atoi(readData);
	crc2 = CRC_getByteCRC(anglesStr, strlen(anglesStr));
	if(crc1 == crc2) {
        char *d = "|";
        char *tmp;
        tmp = strtok(anglesStr, d);
        if(NULL != tmp) {
            *cor_x = atoi(tmp);
            tmp = strtok(NULL, d);
            if(tmp != NULL) {
                *cor_y = atoi(tmp);
                tmp = strtok(NULL, d);
                if(tmp != NULL) {
                    *cor_z = atoi(tmp);
                }
            }
        }

        if(NULL == tmp) {
            ret = -1;
        }
        else {
            ret = 0;
        }
	}
	else {
		APP_TRACE("%s get %s failed\n", __FUNCTION__, file);
		ret = -1;
	}

	return ret;

}

static int app_gsensor_calibration(bool enable)
{
    int ret = 0;
    int8_t cor_x = 0, cor_y = 0, cor_z = 0;

    if(GSENSOR_is_init()) {
        if(enable) {
            GSENSOR_auto_correct(&cor_x, &cor_y, &cor_z);
            APP_TRACE("%s: Corrected. cor_x: %d, cor_y: %d, cor_z: %d\n",
                    __FUNCTION__,
                    cor_x, cor_y, cor_z);
            // save to file
            ret = app_gsensor_save_calibration_to_file(GSENSOR_CALIBRATION_ANGLES_FILE, cor_x, cor_y, cor_z);
            if(ret == 0) {
                gsensor_params.calibration = true;
            }
        }
        else {
            gsensor_params.calibration = false;
            APP_TRACE("Don't do anything!!!");
        }
    }
    else {
        ret = emAPP_GSENSOR_ERR_CODE_NOT_SUPPORT;
    }

    return ret;

}

int APP_GSENSOR_calibration(bool enable)
{
    int ret = 0;

    ret = app_gsensor_calibration(enable);

    return ret;

}

int APP_GSENSOR_set_for_json(struct json_object *gsensorJson)
{
    struct json_object *tmpJson = NULL;
    struct json_object *calibrationJson = NULL;
    int ret = -1;

    if(NULL == gsensorJson) {
        return ret;
    }

    tmpJson = json_object_object_get(gsensorJson, "version");
    if(NULL != tmpJson) {
        if(json_type_string == json_object_get_type(tmpJson)) {
            if(0 == strncmp("1.0.0", json_object_get_string(tmpJson), strlen("1.0.0"))) {
                calibrationJson = json_object_object_get(gsensorJson, "calibration");
                if(NULL != calibrationJson) {
                    tmpJson = json_object_object_get(calibrationJson, "enabled");
                    if(NULL != tmpJson) {
                        if(json_type_boolean == json_object_get_type(tmpJson)) {
                            ret = app_gsensor_calibration(json_object_get_boolean(tmpJson));
                        }
                    }
                }
            }
        }
    }

    return ret;

}

int APP_GSENSOR_start(void)
{
    int8_t cor_x = 0, cor_y = 0, cor_z = 0;

    if (0 != GSENSOR_init()) {
        APP_TRACE("%s: ADXL345_Init failed!", __FUNCTION__);
        return -1;
    }

    // set calibration
    if(0 == app_gsensor_get_calibration_to_file(GSENSOR_CALIBRATION_ANGLES_FILE, &cor_x, &cor_y, &cor_z)) {
        gsensor_params.calibration = true;
        GSENSOR_set_correct_xyz(cor_x, cor_y, cor_z);
    }

    return 0;

}

void APP_GSENSOR_stop(void)
{
    GSENSOR_deinit();

    gsensor_params.triger = false;
    gsensor_params.pid = (pthread_t)NULL;

}

int APP_GSENSOR_web_cgi_get_angles(LP_HTTP_CONTEXT session)
{
    LP_HTTP_HEAD_FIELD httpHeadField;
    char httpHeadBuf[512];
    int httpHeadLength = 0;
    char httpContentBuf[512];
    int httpContentLength = 0;
    ST_SOCKET_TCP sockTCP;
    LP_SOCKET_TCP httpTCP = socket_tcp2_r(session->sock, &sockTCP);
    double angle_x, angle_y, angle_z;

    if(HTTP_IS_GET(session)) {
        GSENSOR_get_angles(&angle_x, &angle_y, &angle_z);
        snprintf(httpContentBuf, sizeof(httpContentBuf),
            "{\"version\":\"1.0.0\",\"angles\":{\"anglesX\":%lf,\"anglesY\":%lf,\"anglesZ\":%lf,\"anglesProperty\":{\"type\":\"double\",\"mode\":\"ro\",\"min\":-90.0,\"max\":90.0}}}",
            angle_x / 10, angle_y / 10, angle_z / 10);
        APP_TRACE("get angles...x = %lf y = %lf z = %lf", angle_x, angle_y, angle_z);
    }
    else {
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
    if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)) {
        // send out http content
        if(httpContentLength > 0) {
            if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)) {
            // send
            }
        }
    }

    return 0;

}


int APP_GSENSOR_get_angles_frame(void *frame, unsigned int *frame_size)
{
    ST_GSENSOR_FRAME gsensorFrame;
    ST_GSENSOR_ANGLES gsensorAngles;
    stGsensor_angles angles;
    double cur_angle_x, cur_angle_y, cur_angle_z;

    if(!GSENSOR_is_init()) {
        return -1;
    }

    if((NULL == frame) || (NULL == frame_size)) {
        return -1;
    }

    gsensorFrame.frameType = 2;
    gsensorFrame.dataSize = sizeof(ST_GSENSOR_ANGLES);
    gsensorAngles.param.min_angle_range = -90.0;
    gsensorAngles.param.max_angle_range = 90.0;
    GSENSOR_get_angles(&cur_angle_x, &cur_angle_y, &cur_angle_z);
    gsensorAngles.param.angle_x = cur_angle_x;
    gsensorAngles.param.angle_y = cur_angle_y;
    gsensorAngles.param.angle_z = cur_angle_z;

    gsensorAngles.version = 0x01000000; // 1.0.0.0
    gsensorAngles.reverse = 0;

    memcpy(frame, &gsensorFrame, sizeof(ST_GSENSOR_FRAME));
    memcpy(frame + sizeof(ST_GSENSOR_FRAME), &gsensorAngles, sizeof(ST_GSENSOR_ANGLES));

    *frame_size = sizeof(ST_GSENSOR_ANGLES) + sizeof(ST_GSENSOR_FRAME);

    return 0;

}

bool APP_GSENSOR_is_support()
{
    return GSENSOR_is_init();

}

#endif

