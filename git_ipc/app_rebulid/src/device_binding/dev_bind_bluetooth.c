#if defined(BLUETOOTH)

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <device_binding/device_binding.h>
#include <app_debug.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <json/json.h>
#include <securedat.h>
#include <bluetooth.h>
#include <base64.h>

// struct for json protocol version 1
typedef struct {

    int ver;
    int id;
    char method[32];

} stBT_req_hdr_v1, *lpBT_req_hdr_v1;

typedef struct {

    char ssid[33];
    char pass[257];
    char mode[33];
    char token[512];

} stBT_req_put_info_v1_param, *lpBT_put_info_v1_param;

typedef enum {

    emBT_ERR_CODE_OK = 0,
    emBT_ERR_CODE_REQ_ERR = 100,
    emBT_ERR_CODE_METHOD_ERR = 101,
    emBT_ERR_CODE_PARAM_ERR = 200,
    emBT_ERR_CODE_VER_ERR = 300,
    emBT_ERR_CODE_DEV_ERR = 400,

} emBT_ERR_CODE;



typedef struct {

    bool triger;
    bool running;
    pthread_t pid;

} stBT_thr_params;

typedef struct {

    stBT_thr_params work_thr;

    pthread_mutex_t init_lock;

} stBT_params;

typedef enum {
    emBT_DEAL_REQ_ERR = -1,
    emBT_DEAL_REQ_PUT_INFO,
    emBT_DEAL_REQ_GET_NEAR_AP,

} emBT_DEAL_REQ;

static const unsigned int thread_stop_timeout_us =  (10 * 1000 * 1000);


static stBT_params gs_bt_params = {

        .work_thr.triger = false,
        .work_thr.running = false,

        .init_lock = PTHREAD_MUTEX_INITIALIZER,

};

static DEV_BIND_get_near_ap bt_get_near_ap = NULL;



static char *bt_get_hotspot_name(char *str_buf, size_t str_buf_len)
{
    char sn_str[32] = {0};
    if (0 == UC_SNumberGet(sn_str)) {
        snprintf(str_buf,
                 str_buf_len,
                 "IPC%s",
                 sn_str);
    } else {
        snprintf(str_buf,
                 str_buf_len,
                 "IPC123456");
    }

    return str_buf;
}


// json request/response

// only key exist and is jsonType, return the value; else return NULL
static json_object *bt_json_get_typed_key(json_object *jsonObj,
                                   char *key,
                                   json_type jsonType)
{
    json_object *valObj;
    json_type valType;

    valObj = json_object_object_get(jsonObj, key);
    if (NULL == valObj) {
        APP_TRACE("%s: no \"%s\" key in json object!", __FUNCTION__, key);
        return NULL;
    } else {
        valType = json_object_get_type(valObj);
        if (valType != jsonType) {
            APP_TRACE("%s: \"%s\" value has wrong type! type: %d, expected: %d",
                      __FUNCTION__, key, valType, jsonType);
            return NULL;
        } else {
            return valObj;
        }
    }
}

// @return -1, request error have id; -2, request error no id; 0, ok
static int bt_json_get_ver_and_id(json_object *reqObj, int *p_ver, int *p_id)
{
    json_object *valJson;
    bool errOccur = false;
    bool haveId = false;
    char *key;

    // root.ver
    key = "ver";
    valJson = bt_json_get_typed_key(reqObj, key, json_type_int);
    if (NULL != valJson) {
        *p_ver = json_object_get_int(valJson);
    } else {
        errOccur = true;
    }

    // root.id
    key = "id";
    valJson = bt_json_get_typed_key(reqObj, key, json_type_int);
    if (NULL != valJson) {
        *p_id = json_object_get_int(valJson);
        haveId = true;
    } else {
        errOccur = true;
    }

    if (errOccur) {
        if (haveId) {
            return -1;
        } else {
            return -2;
        }
    } else {
        return 0;
    }
}

// **************** protocol version 1 ************** //
static int bt_json_get_hdr_v1(json_object *reqObj, lpBT_req_hdr_v1 lpHdr_v1)
{

    json_object *valJson;
    bool errOccur = false;
    char *key;


    // root.ver
    lpHdr_v1->ver = 1;

    // root.id
    key = "id";
    valJson = bt_json_get_typed_key(reqObj, key, json_type_int);
    if (NULL != valJson) {
        lpHdr_v1->id = json_object_get_int(valJson);
    } else {
        errOccur = true;
    }

    // root.method
    key = "method";
    valJson = bt_json_get_typed_key(reqObj, key, json_type_string);
    if (NULL != valJson) {
        snprintf(lpHdr_v1->method, sizeof(lpHdr_v1->method),
                 "%s", json_object_get_string(valJson));
    } else {
        errOccur = true;
    }

    if (errOccur) {
        return -1;
    } else {
        return 0;
    }
}

static int bt_json_get_put_info_v1_param(json_object *reqObj,
                                         lpBT_put_info_v1_param lpParam)
{
    json_object *valJson;
    json_object *paramObj;
    bool errOccur = false;
    char *key;

    // root.params
    key = "params";
    valJson = bt_json_get_typed_key(reqObj, key, json_type_object);
    if (NULL != valJson) {
        paramObj = valJson;
    } else {
        return -1;
    }

    // root.params.ssid
    key = "ssid";
    valJson = bt_json_get_typed_key(paramObj, key, json_type_string);
    if (NULL != valJson) {
        snprintf(lpParam->ssid, sizeof(lpParam->ssid),
                 "%s", json_object_get_string(valJson));
    } else {
        errOccur = true;
    }

    // root.params.pass
    key = "pass";
    valJson = bt_json_get_typed_key(paramObj, key, json_type_string);
    if (NULL != valJson) {
        snprintf(lpParam->pass, sizeof(lpParam->pass),
                 "%s", json_object_get_string(valJson));
    } else {
        errOccur = true;
    }

    // root.params.mode
    key = "mode";
    valJson = bt_json_get_typed_key(paramObj, key, json_type_string);
    if (NULL != valJson) {
        snprintf(lpParam->mode, sizeof(lpParam->mode),
                 "%s", json_object_get_string(valJson));
    } else {
        errOccur = true;
    }

    // root.params.token
    key = "token";
    valJson = bt_json_get_typed_key(paramObj, key, json_type_string);
    if (NULL != valJson) {
        snprintf(lpParam->token, sizeof(lpParam->token),
                 "%s", json_object_get_string(valJson));
    } else {
        errOccur = true;
    }


    if (errOccur) {
        return -1;
    } else {
        return 0;
    }
}

static void bt_json_res_dev_err(int id)
{
    char dev_err_str[64];

    snprintf(dev_err_str,
             sizeof(dev_err_str),
             "{\"id\":%d,\"res\":%d,\"params\":{}}\r\n",
             id, emBT_ERR_CODE_DEV_ERR);

    APP_TRACE("res: %s", dev_err_str);

    BT_write((unsigned char*)dev_err_str, strlen(dev_err_str));
}

static int bt_json_res_set_id(json_object *retObj, int id)
{
    char *key;
    json_object *tmpJson = NULL;

    // root.id
    key = "id";
    tmpJson = json_object_new_int(id);
    if (NULL == tmpJson) {
        APP_TRACE("%s: Failed to create \"%s\" field!", __FUNCTION__, key);
        return -1;
    } else {
        json_object_object_add(retObj, key, tmpJson);
    }

    return 0;

}

static int bt_json_res_set_res(json_object *retObj, int res)
{
    char *key;
    json_object *tmpJson = NULL;

    // root.res
    key = "res";
    tmpJson = json_object_new_int(res);
    if (NULL == tmpJson) {
        APP_TRACE("%s: Failed to create \"%s\" field!", __FUNCTION__, key);
        return -1;
    } else {
        json_object_object_add(retObj, key, tmpJson);
    }

    return 0;

}

static int bt_json_res_set_avai_ver(json_object *retObj)
{
    char *key;
    json_object *tmpJson = NULL;
    json_object *avai_verArry = NULL;

    // root.params.avai_ver[]
    key = "avai_ver";
    avai_verArry = json_object_new_array();
    if (NULL == avai_verArry) {
        APP_TRACE("%s: Failed to create \"%s\" field!", __FUNCTION__, key);
        return -1;
    } else {
        json_object_object_add(retObj, key, avai_verArry);
    }

    // root.params.avai_ver[1] = 1
    tmpJson = json_object_new_int(1);
    if (NULL == tmpJson) {
        APP_TRACE("%s: Failed to create json int!", __FUNCTION__);
        return -1;
    } else {
        json_object_array_add(avai_verArry, tmpJson);
    }

    return 0;

}

static int bt_json_res_send(const json_object *resObj)
{
    const char *send_str;
    int ret = 0;

    send_str = json_object_get_string(resObj);
    APP_TRACE("res: %s", send_str);
    ret = BT_write((unsigned char*)send_str, strlen(send_str));
    if (0 == ret) {
        BT_write((unsigned char*)"\r\n", 2);
    }

    return ret;

}

static int bt_json_res(int id, emBT_ERR_CODE err_code)
{
    json_object *resObj = NULL;
    json_object *tmpJson = NULL;
    char *key;
    int ret = 0;


    resObj = json_object_new_object();
    if (NULL == resObj) {
        APP_TRACE("%s: Failed to create json object!", __FUNCTION__);
        bt_json_res_dev_err(id);
        goto ERROR_RETURN;
    }

    // root.id
    if(0 != bt_json_res_set_id(resObj, id)) {
        bt_json_res_dev_err(id);
        goto ERROR_RETURN;
    }

    // root.res
    if(0 != bt_json_res_set_res(resObj, err_code)) {
        bt_json_res_dev_err(id);
        goto ERROR_RETURN;
    }

    // root.params
    key = "params";
    tmpJson = json_object_new_object();
    if (NULL == tmpJson) {
        APP_TRACE("%s: Failed to create \"%s\" field!", __FUNCTION__, key);
        bt_json_res_dev_err(id);
        goto ERROR_RETURN;
    } else {
        json_object_object_add(resObj, key, tmpJson);
        if (emBT_ERR_CODE_VER_ERR == err_code) {
            if(0 != bt_json_res_set_avai_ver(tmpJson)) {
                bt_json_res_dev_err(id);
                goto ERROR_RETURN;
            }
        }
    }

    // send
    bt_json_res_send(resObj);
    json_object_put(resObj);
    return ret;


ERROR_RETURN:

    if (NULL != resObj) {
        json_object_put(resObj);
    }

    return -1;
}

static int bt_json_near_ap_res(int id, lpDEV_NEAR_AP lpAPs, unsigned int nAPs, emBT_ERR_CODE err_code)
{
#define MAX_NUM_AP_RES  8

    json_object *resObj = NULL;
    json_object *tmpJson = NULL, *paramsJson = NULL;
    char *key;
    int ret = 0;
    int i = 0;
    int ap_req_num = 0;

    if(NULL == lpAPs) {
        return -1;
    }

    resObj = json_object_new_object();
    if (NULL == resObj) {
        APP_TRACE("%s: Failed to create json object!", __FUNCTION__);
        bt_json_res_dev_err(id);
        goto ERROR_RETURN;
    }

    // root.id
    if(0 != bt_json_res_set_id(resObj, id)) {
        bt_json_res_dev_err(id);
        goto ERROR_RETURN;
    }

    // root.res
    if(0 != bt_json_res_set_res(resObj, err_code)) {
        bt_json_res_dev_err(id);
        goto ERROR_RETURN;
    }

    // root.params
    key = "params";
    if(emBT_ERR_CODE_VER_ERR == err_code) {
        tmpJson = json_object_new_object();
        if(0 != bt_json_res_set_avai_ver(tmpJson)) {
            bt_json_res_dev_err(id);
            goto ERROR_RETURN;
        }
        json_object_object_add(resObj, key, tmpJson);
    } else {
       paramsJson = json_object_new_array();
       if (NULL == paramsJson) {
           APP_TRACE("%s: Failed to create \"%s\" field!", __FUNCTION__, key);
           bt_json_res_dev_err(id);
           goto ERROR_RETURN;
       } else {
            ap_req_num = (nAPs > MAX_NUM_AP_RES) ? MAX_NUM_AP_RES : nAPs;
            for(i = 0; i < ap_req_num; i++) {
               json_object *info = json_object_array_get_idx(paramsJson, i);
               if(info == NULL) {
                   char en_base64[64] = {0};
                   info = json_object_new_object();
                   base64_encode(lpAPs[i].ssid, en_base64, strlen(lpAPs[i].ssid));
                   tmpJson = json_object_new_string(en_base64);
                   json_object_object_add(info, "ssid", tmpJson);
                   tmpJson = json_object_new_int(lpAPs[i].rssi);
                   json_object_object_add(info, "rssi", tmpJson);
                   /* 因为蓝牙发送问题，暂时屏蔽以下协议内容，后面再做扩展 */
                   /*base64_encode(lpAPs[i].bssid, en_base64, strlen(lpAPs[i].bssid));
                   tmpJson = json_object_new_string(en_base64);
                   json_object_object_add(info, "bssid", tmpJson);
                   tmpJson = json_object_new_string(lpAPs[i].encrytype);
                   json_object_object_add(info, "encrypt", tmpJson);*/
                   json_object_array_put_idx(paramsJson, i, info);
               }
           }
           json_object_object_add(resObj, key, paramsJson);
       }
    }

    // send
    bt_json_res_send(resObj);
    json_object_put(resObj);
    return ret;


ERROR_RETURN:

    if (NULL != resObj) {
        json_object_put(resObj);
    }

    return -1;
}

static int bt_response_near_ap_v1(stBT_req_hdr_v1 stHdr_v1)
{
    stDEV_NEAR_AP ap[128];
    unsigned int apSize = sizeof(ap);
    int ret = 0;

    memset(ap, 0, apSize);
    if(NULL != bt_get_near_ap) {
        bt_get_near_ap(ap, &apSize);
    }

    // apSize / bt_get_near_ap return -1 emBT_ERR_CODE_REQ_ERR
    bt_json_near_ap_res(stHdr_v1.id, ap, apSize, emBT_ERR_CODE_OK);

    return ret;

}

static int bt_json_deal_req_v1(json_object *reqObj, DEV_BIND_CB_on_recved_data on_recved_data)
{
    stBT_req_hdr_v1 hdr_v1;
    if (0 != bt_json_get_hdr_v1(reqObj, &hdr_v1)) {
        APP_TRACE("%s: Failed to get header v1 from request json",
                  __FUNCTION__);
        // response: request error
        bt_json_res(hdr_v1.id, emBT_ERR_CODE_REQ_ERR);
        return -1;
    } else {
        // deal with methods
        if (0 == strcasecmp(hdr_v1.method, "put_info")) {
            stBT_req_put_info_v1_param stParam;
            if (0 != bt_json_get_put_info_v1_param(reqObj, &stParam)) {
                APP_TRACE("%s: Wrong parameter of method %s",
                          __FUNCTION__, hdr_v1.method);
                // response: parameter error
                bt_json_res(hdr_v1.id, emBT_ERR_CODE_PARAM_ERR);
                return -1;
            } else {
                // response: ok
                bt_json_res(hdr_v1.id, emBT_ERR_CODE_OK);
                if(0 != on_recved_data(stParam.ssid, stParam.pass, stParam.mode, stParam.token)) {
                    return emBT_DEAL_REQ_ERR;
                }
                return emBT_DEAL_REQ_PUT_INFO;
            }

        } else if (0 == strcasecmp(hdr_v1.method, "get_near_ap")) {
            bt_response_near_ap_v1(hdr_v1);
            return emBT_DEAL_REQ_GET_NEAR_AP;
        } else {
            APP_TRACE("%s: Unsupported method: %s",
                      __FUNCTION__, hdr_v1.method);
            // response: Unsupported method
            bt_json_res(hdr_v1.id, emBT_ERR_CODE_METHOD_ERR);
            return -1;
        }
    }
}
// ************************************************** //

// @return 0, work compeleted, bluetooth can be turn off; -1 request or device error
static int bt_json_deal_req(json_object *reqObj, DEV_BIND_CB_on_recved_data on_recved_data)
{
    int ret;
    int ver;
    int id;

    ret = bt_json_get_ver_and_id(reqObj, &ver, &id);
    if (ret < 0) {
        // todo
        // response: request error. when request error, id is -1;
        if (-1 == ret) {
            bt_json_res(id, emBT_ERR_CODE_REQ_ERR);
        } else {
            bt_json_res(-1, emBT_ERR_CODE_REQ_ERR);
        }
        return -1;
    } else {
        switch (ver) {
            case 1:
                return bt_json_deal_req_v1(reqObj, on_recved_data);

            default:
                // response: version unsupported version
                bt_json_res(id, emBT_ERR_CODE_VER_ERR);
                return -1;
        }
    }
}

// peek min[all data bytes, data_buf_len] data; if no data, this function will blocking
static int bt_peek_all(uint8_t *data_buf, size_t data_buf_len, size_t *p_out_data_len)
{
    ssize_t peek_data_sz = 0;
    size_t out_data_sz = 0;
    while (gs_bt_params.work_thr.triger) {

        peek_data_sz = BT_get_recvbuf_data_sz();
        if (peek_data_sz < 0) {
            // internal error
            return -1;
        } else {
            if (peek_data_sz <= 0) {
                // buffer no data
                usleep(50000);
            } else {
                out_data_sz = (peek_data_sz < data_buf_len) ? peek_data_sz : data_buf_len;
                if (0 != BT_peek(data_buf,
                                 out_data_sz)) {
                    // failed to peek data
                    APP_TRACE("%s: failed to peek data!", __FUNCTION__);
                    usleep(50000);
                } else {
                    *p_out_data_len = out_data_sz;
                    return 0;
                }
            }
        }
    }

    return -1;
}

// peek min[all data bytes, data_buf_len] data; if no data, this function will not blocking, and *p_out_data_len == 0
static int bt_try_peek_all(uint8_t *data_buf, size_t data_buf_len, size_t *p_out_data_len)
{
    size_t peek_data_sz = 0;
    size_t out_data_sz = 0;
    while (gs_bt_params.work_thr.triger) {

        peek_data_sz = BT_get_recvbuf_data_sz();
        if (peek_data_sz < 0) {
            // internal error
            return -1;
        } else {
            if (peek_data_sz <= 0) {
                // buffer no data
                *p_out_data_len = 0;
                return 0;
            } else {
                out_data_sz = (peek_data_sz < data_buf_len) ? peek_data_sz : data_buf_len;
                if (0 != BT_peek(data_buf,
                                 out_data_sz)) {
                    // failed to peek data
                    APP_TRACE("%s: failed to peek data!", __FUNCTION__);
                    return -1;
                } else {
                    *p_out_data_len = out_data_sz;
                    return 0;
                }
            }
        }
    }

    return -1;
}


static void *bt_work_thr(void *arg)
{
    size_t data_buf_len = 2048;
    uint8_t data_buf[data_buf_len];
    char *tmp_data_start_pos = NULL;
    char *tmp_data_end_pos = NULL;

    struct json_tokener *tok = NULL;

    DEV_BIND_CB_on_recved_data on_recved_data;


    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "bt_work_thread");

    on_recved_data = (DEV_BIND_CB_on_recved_data)arg;

    tok = json_tokener_new();
    if (NULL == tok) {
        APP_TRACE("%s: Failed to create json_tokener", __FUNCTION__);
        goto THREAD_QUIT;
    }


    // init bluetooth
    if (0 != BT_init()) {
        APP_TRACE("%s: Failed to init bluetooth", __FUNCTION__);
        goto THREAD_QUIT;
    }

    // open bluetooth hotspot
    if (0 != BT_open_hotspot(bt_get_hotspot_name((char *)data_buf, data_buf_len))) {
        APP_TRACE("%s: Failed to open bluetooth hotspot", __FUNCTION__);
        goto THREAD_QUIT;
    }


    // recv and parse json request
    // 1. skip other to find '{'
    // 2. parse. if error, discard parsed
    // 3. got parsed json
    size_t out_data_sz;
    size_t discard_data_len = 0;
    size_t to_parse_data_len = 0;
    json_object *jsonObj = NULL;

    uint32_t wait_crlf_timeout_us  = 10 * 1000 * 1000;
    uint32_t wait_crlf_us_total = 0;
    uint32_t wait_crlf_us_once = 100000;

    char *SUFFIX = "\r\n";
    size_t suffix_len = strlen(SUFFIX);

    while (gs_bt_params.work_thr.triger) {

        if (0 != bt_peek_all(data_buf, data_buf_len - 1, &out_data_sz)) {
            APP_TRACE("%s: Error occured when peek data!!", __FUNCTION__);
            goto THREAD_QUIT;
        } else {

            data_buf[out_data_sz] = '\0';

            // get position of '{'
            tmp_data_start_pos = strstr((char *)data_buf, "{");
            if (NULL == tmp_data_start_pos) {
                APP_TRACE("%s: out_data: ****%s****", __FUNCTION__, data_buf);
                APP_TRACE("%s: can't find start position, discard data!", __FUNCTION__);
                wait_crlf_us_total = 0;
                BT_skip(out_data_sz);
            } else {

                // discard data before '{'
                discard_data_len = tmp_data_start_pos - (char *)data_buf;
                BT_skip(discard_data_len);

                tmp_data_end_pos = strstr(tmp_data_start_pos, SUFFIX);
                if (NULL == tmp_data_end_pos) {
                    if (wait_crlf_us_total < wait_crlf_timeout_us) {
                        usleep(wait_crlf_us_once);
                        wait_crlf_us_total += wait_crlf_us_once;
                    } else {
                        APP_TRACE("%s: out_data: ****%s****", __FUNCTION__, data_buf);
                        APP_TRACE("%s: wiat timeout for CRLF, discard data!", __FUNCTION__);
                        wait_crlf_us_total = 0;
                        BT_skip(out_data_sz - discard_data_len);
                    }
                } else {
                    APP_TRACE("%s: out_data: ****%s****", __FUNCTION__, data_buf);
                    wait_crlf_us_total = 0;

                    to_parse_data_len = tmp_data_end_pos - tmp_data_start_pos;
                    APP_TRACE("%s: to_parse_data_len: %u", __FUNCTION__, to_parse_data_len);

                    BT_skip(to_parse_data_len + suffix_len);

                    // parse json
                    jsonObj = json_tokener_parse_ex(tok, tmp_data_start_pos, (int)to_parse_data_len);
                    if (NULL != jsonObj) {
                        // deal with request
                        int ret = 0;
                        ret = bt_json_deal_req(jsonObj, on_recved_data);
                        if(emBT_DEAL_REQ_ERR == ret) {
                            APP_TRACE("%s: Error occured then deal with json request!", __FUNCTION__);
                        } else if(emBT_DEAL_REQ_PUT_INFO == ret) {
                            gs_bt_params.work_thr.triger = false;
                            sleep(5);
                        }
                    } else {
                        APP_TRACE("%s: json parse failed!", __FUNCTION__);
                    }

                    json_tokener_reset(tok);
                }
            }
        }
    }




THREAD_QUIT:

    if (NULL != tok) {
        json_tokener_free(tok);
    }

    BT_deinit();

    gs_bt_params.work_thr.triger = false;
    gs_bt_params.work_thr.running = false;

    APP_TRACE("%s: Bluetooth device binding thread quit", __FUNCTION__);
    return NULL;
}


// thread controllers
static int bt_start_work_thr(DEV_BIND_CB_on_recved_data on_recved_data, DEV_BIND_get_near_ap get_near_ap)
{
    int ret;

    bt_get_near_ap = get_near_ap;

    gs_bt_params.work_thr.triger = true;
    gs_bt_params.work_thr.running = true;
    ret = pthread_create(&gs_bt_params.work_thr.pid,
                         NULL,
                         bt_work_thr,
                         on_recved_data);
    if (0 != ret) {
        gs_bt_params.work_thr.triger = false;
        gs_bt_params.work_thr.running = false;
        ret = -1;
    }

    return ret;
}

static void bt_stop_work_thr()
{
    unsigned int wait_total_us = 0;
    unsigned int a_period_us = 0;
    unsigned int wait_us = 200 * 1000;

    gs_bt_params.work_thr.triger = false;
    while (1) {
        if (gs_bt_params.work_thr.running) {
            usleep(wait_us);
            wait_total_us += wait_us;
            a_period_us += wait_us;
            if (a_period_us > 5000000) {
                APP_TRACE("%s: %u Second passed for work thread to quit...",
                          __FUNCTION__, wait_total_us/1000000);
                a_period_us = 0;
            }
        } else {
            break;
        }
    }

    gs_bt_params.work_thr.running = false;
}



/**
 * 开启蓝牙，准备绑定
 */
int DEV_BIND_BT_start(DEV_BIND_CB_on_recved_data on_recved_data, DEV_BIND_get_near_ap get_near_ap)
{
    int ret;

    if (NULL == on_recved_data) {
        APP_TRACE("%s: on_recved_data can't be NULL!", __FUNCTION__);
        return -1;
    }

    pthread_mutex_lock(&gs_bt_params.init_lock);


    ret = bt_start_work_thr(on_recved_data, get_near_ap);
    if (0 != ret) {
        APP_TRACE("%s: Failed to start work thread!", __FUNCTION__);
        goto FAIL_RETURN;
    }


    pthread_mutex_unlock(&gs_bt_params.init_lock);

    APP_TRACE("%s: Bluetooth for device binding started", __FUNCTION__);
    return 0;


FAIL_RETURN:

    bt_stop_work_thr();

    pthread_mutex_unlock(&gs_bt_params.init_lock);
    return -1;
}

/**
 * 在需要的时候，提前中断蓝牙绑定
 */
int DEV_BIND_BT_interrupt()
{
    pthread_mutex_lock(&gs_bt_params.init_lock);

    bt_stop_work_thr();

    pthread_mutex_unlock(&gs_bt_params.init_lock);

    return 0;
}

#endif //defined(BLUETOOTH)
