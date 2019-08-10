#if defined(BLUETOOTH)

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <app_debug.h>
#include <json/json.h>
#include <bluetooth.h>
#include "app_bluetooth.h"


#define APP_BLUETOOTH_STATUS_WORKING       "working"
#define APP_BLUETOOTH_STATUS_NONWORKING    "nonworking"

emAPP_BLUETOOTH_STATUS APP_BLUETOOTH_status(char *status)
{
    emAPP_BLUETOOTH_STATUS emStatus = emAPP_BLUETOOTH_STATUS_NONWORKING;

    if(BT_is_hotspot_on()) {
        emStatus = emAPP_BLUETOOTH_STATUS_WORKING;
    }
    else {
        emStatus = emAPP_BLUETOOTH_STATUS_NONWORKING;
    }

    if(NULL != status) {
        sprintf(status, "%s", (emStatus == emAPP_BLUETOOTH_STATUS_WORKING) ? APP_BLUETOOTH_STATUS_WORKING : APP_BLUETOOTH_STATUS_NONWORKING);
    }

    return emStatus;

}

int APP_BLUETOOTH_get_to_json(struct json_object *bluetoothJson, bool properties_flag)
{
    struct json_object *tmpJson = NULL;
    struct json_object *statusJson = NULL;
    struct json_object *propertiesJson = NULL;
    struct json_object *optArrJson = NULL;
    int ret = emAPP_BLUETOOTH_ERR_CODE_OK;
    char strStatus[16] = {0};


    if(NULL != bluetoothJson) {
        // version
        tmpJson = json_object_new_string("1.0.0");  // 报文版本号
        json_object_object_add(bluetoothJson, "version", tmpJson);
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
            json_object_object_add(bluetoothJson, "versionProperty", propertiesJson);
        }

        // status
        memset(strStatus, 0, sizeof(strStatus));
        APP_BLUETOOTH_status(strStatus);
        tmpJson = json_object_new_string_len(strStatus, strlen(strStatus));
        json_object_object_add(bluetoothJson, "status", tmpJson);
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
            optArrJson = json_object_new_array();
            if(NULL != optArrJson) {
                tmpJson = json_object_new_string(APP_BLUETOOTH_STATUS_WORKING);
                json_object_array_add(optArrJson, tmpJson);
                tmpJson = json_object_new_string(APP_BLUETOOTH_STATUS_NONWORKING);
                json_object_array_add(optArrJson, tmpJson);
                json_object_object_add(propertiesJson, "opt", optArrJson);
            }
            json_object_object_add(bluetoothJson, "statusProperty", propertiesJson);
        }

    }

    return ret;

ERROR_RETURN:

    if(NULL != statusJson) {
        json_object_put(statusJson);
    }

    if(NULL != propertiesJson) {
        json_object_put(propertiesJson);
    }

    if(NULL != optArrJson) {
        json_object_put(optArrJson);
    }

    return emAPP_BLUETOOTH_ERR_CODE_NOT_SUPPORT;

}

int APP_BLUETOOTH_init()
{
    return BT_init();

}

bool APP_BLUETOOTH_is_support()
{
    return BT_is_init();

}

#endif //defined(BLUETOOTH)

