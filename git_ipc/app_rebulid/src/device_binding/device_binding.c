#include <stdio.h>
#include <stdbool.h>
#include <zconf.h>
#include <pthread.h>
#include <app_debug.h>
#include <device_binding/device_binding.h>
#include <base64.h>
#include "dev_bind_bluetooth.h"
#include "dev_bind_sound_wave.h"
#include "../netsdk_def.h"
#include "../netsdk.h"
#include "../sound.h"


static pthread_mutex_t gs_on_recved_data_lock = PTHREAD_MUTEX_INITIALIZER;
static bool gs_data_recved = false;

static DEV_BIND_CB_on_recved_data dev_bind_cb_on_recved_data = NULL;


static int dev_bind_cb_defalut_on_recved_data(char *ssid,
                                              char *pass,
                                              char *wifi_mode,
                                              char *token)
{
    int ret;
    char SSID_Buffer[256];
    char PASS_Buffer[256];
    ST_NSDK_NETWORK_INTERFACE wlan;

    APP_TRACE("??? ssid: %s", ssid);
    APP_TRACE("??? pass: %s", pass);
    APP_TRACE("??? wifi_mode: %s", wifi_mode);
    APP_TRACE("??? token: %s", token);


    ret = base64_decode(ssid, SSID_Buffer, strlen(ssid));
    if (ret < 0) {
        APP_TRACE("base64_decode failed for: %s !", ssid);
        return -1;
    }
    ret = base64_decode(pass, PASS_Buffer, strlen(pass));
    if (ret < 0) {
        APP_TRACE("base64_decode failed for: %s !", pass);
        return -1;
    }

    APP_TRACE("ssid decode: %s", SSID_Buffer);
    APP_TRACE("pass decode: %s", PASS_Buffer);

    return -1;
}

static int dev_bind_internal_cb_on_recved_data(char *ssid,
                                               char *pass,
                                               char *wifi_mode,
                                               char *token)
{
    int ret;
    pthread_mutex_lock(&gs_on_recved_data_lock);

    if (gs_data_recved) {
        pthread_mutex_unlock(&gs_on_recved_data_lock);
        return 0;
    } else {
        gs_data_recved = true;
    }

    pthread_mutex_unlock(&gs_on_recved_data_lock);

    if (NULL != dev_bind_cb_on_recved_data) {
        ret = dev_bind_cb_on_recved_data(ssid, pass, wifi_mode, token);
        if (0 != ret) {
            gs_data_recved = false;
        }
        return ret;
    } else {
        return -1;
    }
}


/**
 * 让设备准备好去被绑定
 */
int DEV_BIND_start(DEV_BIND_CB_on_recved_data on_recved_data, DEV_BIND_get_near_ap get_near_ap)
{

#if !defined(SOUND_WAVE) && !defined(BLUETOOTH) && !defined(WIFI)

    APP_TRACE("No way of communication to bind device!");
    return -1;

#else

    bool bind_rdy = false;
    int ret;

    gs_data_recved = false;

    if (NULL == on_recved_data) {
        APP_TRACE("%s: Callback can't be NULL!", __FUNCTION__);
//        return -1;
        dev_bind_cb_on_recved_data = dev_bind_cb_defalut_on_recved_data;
    } else {
        dev_bind_cb_on_recved_data = on_recved_data;
    }

    #if defined(BLUETOOTH)

        ret = DEV_BIND_BT_start(dev_bind_internal_cb_on_recved_data, get_near_ap);
        if (0 == ret) {
            bind_rdy = true;
        } else {
            APP_TRACE("Failed to start bluetooth for binding");
        }

    #endif // defined(BLUETOOTH)


    #if defined(SOUND_WAVE)

//        ret = DEV_BIND_SW_start(dev_bind_internal_cb_on_recved_data);
//        if (0 == ret) {
//            bind_rdy = true;
//        } else {
//            APP_TRACE("Failed to start sound wave for binding");
//        }

    #endif // defined(SOUND_WAVE)


    #if defined(WIFI)
        // todo
    #endif // defined(WIFI)


    if (bind_rdy) {
        return 0;
    } else {
        APP_TRACE("Failed to start bind");
        return -1;
    }

#endif // !defined(SOUND_WAVE) && !defined(BLUETOOTH) && !defined(WIFI)
}

/**
 * 在需要的时候，提前中断绑定过程
 */
int DEV_BIND_interrupt()
{
#if !defined(SOUND_WAVE) && !defined(BLUETOOTH) && !defined(WIFI)

    APP_TRACE("No way of communication to bind device, no need to interrupt");
    return 0;

#else

    #if defined(BLUETOOTH)

        DEV_BIND_BT_interrupt();

    #endif // defined(BLUETOOTH)


    #if defined(SOUND_WAVE)

        DEV_BIND_SW_interrupt();

    #endif // defined(SOUND_WAVE)


    #if defined(WIFI)
        // todo
    #endif // defined(WIFI)


    return 0;

#endif // !defined(SOUND_WAVE) && !defined(BLUETOOTH) && !defined(WIFI)
}