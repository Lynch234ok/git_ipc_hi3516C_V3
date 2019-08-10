#if defined(BLUETOOTH)


#include <stdio.h>
#include <stdint.h>
#include <thread_db.h>
#include <stdbool.h>
#include <app_debug.h>
#include <sys/prctl.h>
#include <securedat.h>
#include <bluetooth.h>
#include <uart2.h>
#include "cycle_buffer.h"


#define kBT_HOTSPOT_NAME_MAXLEN 31
#define kBT_RECVBUF_SIZE 2048

#define kBT_SERIAL_PORT_PATH "/dev/ttyAMA1"


#define kBT_WAIT_THR_QUIT_TIMEO_MS 5000

#define kBT_REQ_PREFIX "\n\rAT+"
#define kBT_RES_PREFIX "\n\rRES+"
#define kBT_SUFFIX "\r\n"

/*
 * data types
 */

typedef enum {

    emBT_UART_REQ_VER = 1100,
    emBT_UART_REQ_GET_NAME = 1101,
    emBT_UART_REQ_SET_NAME = 1102,
    emBT_UART_REQ_WAKE = 1103,
    emBT_UART_REQ_SLEEP = 1104,

    emBT_UART_RES_OK = 2100,
    emBT_UART_RES_FAIL = 2101,
    emBT_UART_RES_VER = 2200,
    emBT_UART_RES_NAME = 2201,

} emBT_UART_CMD;

typedef struct {

    bool running;
    bool trigger;
    pthread_t pid;

} bt_thread;

typedef struct {

    bool is_inited;
    bool is_hotspot_on;
    char hotspot_name[kBT_HOTSPOT_NAME_MAXLEN + 1];
    pthread_mutex_t bt_lock;
    pthread_mutex_t w_lock;  // BT_write is directly to uart, so need lock;
                             // BT_read is from buffer where already has lock.
    Uart2 *bt_uart;
    H_CYCLE_BUFFER recv_buf;
    bt_thread recv_thr;

} bt_param;





/*
 * globals
 */

static bt_param gs_bt_param = {
        .is_inited = false,
        .is_hotspot_on = false,
        .hotspot_name = {0},
        .bt_lock = PTHREAD_MUTEX_INITIALIZER,
        .w_lock = PTHREAD_MUTEX_INITIALIZER,
        .bt_uart = NULL,
        .recv_buf = NULL,
        .recv_thr = {
                .running = false,
                .trigger = false
        }
};




/*
 * helpers
 */
static int bt_uart_send_fixsz_data(Uart2 *uart, uint8_t *toSendBytes, size_t nSize)
{
    size_t totalSent = 0;
    ssize_t sentLen = 0;
    unsigned char *sendBuf;
    struct timespec tp;
    uint64_t startTimeMs, nowTimeMs;
    uint64_t timeoutMs;


    timeoutMs = 2000;
    sendBuf = toSendBytes;

    if (0 != clock_gettime(CLOCK_MONOTONIC, &tp)) {
        APP_TRACE("clock_gettime failed!!");
        return -1;
    }
    startTimeMs = ((uint64_t)tp.tv_sec * 1000) + ((uint64_t)tp.tv_nsec / 1000000);

    while (totalSent < nSize) {

        clock_gettime(CLOCK_MONOTONIC, &tp);
        nowTimeMs = ((uint64_t)tp.tv_sec * 1000) + ((uint64_t)tp.tv_nsec / 1000000);
        if ((nowTimeMs - startTimeMs) > timeoutMs) {
            break;
        }

        sentLen = uart->Send(uart, sendBuf, nSize - totalSent);
        if (sentLen > 0) {
            totalSent = totalSent + sentLen;
            sendBuf = sendBuf + sentLen;
        } else {
            if (0 == sentLen) {
                // sleep 20 ms
                usleep(20000);
            } else {
                APP_TRACE("%s: uart send error!", __FUNCTION__);
                break;
            }
        }
    }

    if (totalSent == nSize) {
        return 0;
    } else {
        APP_TRACE("%s: send error!! sent: %zu, want send: %zu",
                  __FUNCTION__, totalSent, nSize);
        return -1;
    }
}

static int bt_uart_send_data(Uart2 *uart, uint8_t *toSendBytes, size_t nSize)
{
    if (nSize <= 0) {
        APP_TRACE("%s: Invalid send size %zu", __FUNCTION__, nSize);
        return -1;
    }

    uart->Send(uart, toSendBytes, nSize);

    return 0;
}

static char *bt_get_hotspot_name(char *str_buf, size_t str_buf_len)
{
    char sn_str[32] = {0};
    if (0 == UC_SNumberGet(sn_str)) {
        APP_TRACE("%s: failed to get sn!", __FUNCTION__);
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

static int bt_res_get_value(char *val_ptr, ssize_t val_len,
                            char *out_str_buf, size_t out_buf_len)
{
    if (NULL == val_ptr) {
        APP_TRACE("%s: val_ptr can't be NULL!", __FUNCTION__);
        return -1;
    }
    if (NULL == out_str_buf) {
        APP_TRACE("%s: out_str_buf can't be NULL!", __FUNCTION__);
        return -1;
    }

    if (val_len <= 0) {
        APP_TRACE("%s: val_len should > 0! val_len: %zd", __FUNCTION__, val_len);
        return -1;
    }

    if (val_len >= out_buf_len) {
        APP_TRACE("%s: out buf size too small! value_len: %zd, out_buf_len: %zu",
                  __FUNCTION__, val_len, out_buf_len);
        return -1;
    }

    memcpy(out_str_buf, val_ptr, val_len);
    out_str_buf[val_len] = '\0';

    return 0;
}

static int bt_write_to_uart(uint8_t *p_data, size_t size)
{
    return bt_uart_send_data(gs_bt_param.bt_uart, p_data, size);
}

// peek min[all data bytes, data_buf_len] data; if no data, this function will not blocking, and *p_out_data_len == 0
static int bt_cycle_buf_try_peek_all(uint8_t *data_buf, size_t data_buf_len, size_t *p_out_data_len)
{
    size_t cycle_buffer_data_sz = 0;
    size_t out_data_sz = 0;

    if (0 != CYCLE_BUFFER_get_data_sz(gs_bt_param.recv_buf,
                                      &cycle_buffer_data_sz)) {
        // internal error
        APP_TRACE("%s: error occured while get data from buffer!", __FUNCTION__);
        return -1;
    } else {
        if (cycle_buffer_data_sz <= 0) {
            // buffer no data
            *p_out_data_len = 0;
            return 0;
        } else {
            if (cycle_buffer_data_sz <= data_buf_len) {
                out_data_sz = cycle_buffer_data_sz;
            } else {
                out_data_sz = data_buf_len;
                APP_TRACE("%s: %d bytes not peek. (peek: %d, total: %d)",
                          __FUNCTION__,
                          cycle_buffer_data_sz - out_data_sz,
                          out_data_sz, cycle_buffer_data_sz);
            }
            if (0 != CYCLE_BUFFER_peek(gs_bt_param.recv_buf,
                                       data_buf,
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


static int bt_uart_cmd_req(emBT_UART_CMD send_req, char *name)
{
    int ret;
    char *send_str;
    int str_buf_len = 64;
    char str_buf[str_buf_len];

    switch (send_req) {
        case emBT_UART_REQ_VER:
            send_str = kBT_REQ_PREFIX"VER=?"kBT_SUFFIX;
            break;

        case emBT_UART_REQ_GET_NAME:
            send_str = kBT_REQ_PREFIX"BTNAME=?"kBT_SUFFIX;
            break;

        case emBT_UART_REQ_SET_NAME:
            if (NULL == name) {
                APP_TRACE("%s: name should not be NULL!", __FUNCTION__);
                return -1;
            } else {
                ret = snprintf(str_buf, str_buf_len,
                               kBT_REQ_PREFIX"BTNAME=%s"kBT_SUFFIX,
                               name);
                if (ret < 0 || ret >= str_buf_len) {
                    APP_TRACE("%s: sprintf error, ret: %d, buf len: %d",
                              __FUNCTION__, ret, str_buf_len);
                    return -1;
                } else {
                    send_str = str_buf;
                    break;
                }
            }

        case emBT_UART_REQ_WAKE:
            send_str = kBT_REQ_PREFIX"WAKE"kBT_SUFFIX;
            break;

        case emBT_UART_REQ_SLEEP:
            send_str = kBT_REQ_PREFIX"SLEEP"kBT_SUFFIX;
            break;

        default:
            APP_TRACE("%s: Unsupported req: %d", __FUNCTION__, send_req);
            return -1;
    }


    return bt_write_to_uart(send_str, strlen(send_str));
}

// buffer size should equal to or bigger than (length of name + 1)
static int bt_uart_cmd_recv_res(emBT_UART_CMD *p_res_type,
                                int timeout_ms,
                                char *out_str_buf,
                                size_t out_buf_len)
{
    size_t data_buf_len = 2048;
    uint8_t data_buf[data_buf_len+1];

    size_t out_data_sz = 0;

    struct timespec tp;
    uint64_t curMs;
    uint64_t startMs;
    uint64_t endMs;

    char *SUFFIX = kBT_SUFFIX;

    char *tmp_ptr;
    char *suffix_ptr;
    ssize_t value_len;
    size_t res_prefix_len;
    size_t suffix_len;
    size_t to_parse_data_len;

    res_prefix_len = strlen(kBT_RES_PREFIX);
    suffix_len = strlen(SUFFIX);

    // 1. recv data
    if (0 != clock_gettime(CLOCK_MONOTONIC, &tp)) {
        APP_TRACE("%s: clock_gettime failed!!", __FUNCTION__);
        return -1;
    }
    curMs = ((uint64_t)tp.tv_sec) * 1000 + ((uint64_t)tp.tv_nsec) / 1000000;
    startMs = curMs;
    endMs = curMs + (uint64_t)timeout_ms;

    data_buf[0] = '\0';
    while (1) {
        if (0 != clock_gettime(CLOCK_MONOTONIC, &tp)) {
            APP_TRACE("%s: clock_gettime failed!!", __FUNCTION__);
            return -1;
        }
        curMs = ((uint64_t)tp.tv_sec) * 1000 + ((uint64_t)tp.tv_nsec) / 1000000;

        if (curMs > endMs) {
            APP_TRACE("%s: bt out data: ****%s****", __FUNCTION__, data_buf);
            APP_TRACE("%s: timeout to recv uart response! s: %llu, e: %llu, c: %llu",
                      __FUNCTION__, startMs, endMs, curMs);
            return -1;
        }

        if (0 != bt_cycle_buf_try_peek_all(data_buf, data_buf_len, &out_data_sz)) {
            APP_TRACE("%s: failed to get response!", __FUNCTION__);
            return -1;
        } else {
            if (out_data_sz <= 0) {
                data_buf[0] = '\0';
                usleep(50000);
            } else {
                data_buf[out_data_sz] = '\0';
                if (out_data_sz <= res_prefix_len) {
//                    APP_TRACE("%s: bt out data for response too short, wait", __FUNCTION__);
                    usleep(50000);
                } else {
                    suffix_ptr = strstr(((char *)data_buf) + res_prefix_len, SUFFIX);
                    if (NULL == suffix_ptr) {
//                        APP_TRACE("%s: bt out data no suffix CRLF, wait", __FUNCTION__);
                        usleep(50000);
                    } else {
                        APP_TRACE("%s: bt out data: ****%s****", __FUNCTION__, data_buf);
                        to_parse_data_len = suffix_ptr - (char *)data_buf;
                        CYCLE_BUFFER_skip(gs_bt_param.recv_buf, to_parse_data_len + suffix_len);
                        break;
                    }
                }
            }
        }
    }


    // 2. parse data
    if (0 != strncmp((char *)data_buf, kBT_RES_PREFIX, res_prefix_len)) {
        APP_TRACE("%s: Error response from bt model: %s", __FUNCTION__, data_buf);
        return -1;
    } else {

        // deal with reponses

        tmp_ptr = ((char *)data_buf) + res_prefix_len;

        // OK
        if (0 == strncmp(tmp_ptr, "OK", 2)) {
            *p_res_type = emBT_UART_RES_OK;
            return 0;

        // FAIL
        } else if (0 == strncmp(tmp_ptr, "FAIL", 4)) {
            *p_res_type = emBT_UART_RES_FAIL;
            return 0;

        // VER=XXX
        } else if (0 == strncmp(tmp_ptr, "VER=", 4)) {

            tmp_ptr = tmp_ptr + 4;
            value_len = suffix_ptr - tmp_ptr;
            if (0 != bt_res_get_value(tmp_ptr, value_len,
                                      out_str_buf, out_buf_len)) {
                APP_TRACE("%s: Failed to get value of VER. res: %s",
                          __FUNCTION__,
                          data_buf);
                return -1;
            }

            *p_res_type = emBT_UART_RES_VER;
            return 0;

        // NAME=XXX
        } else if (0 == strncmp(tmp_ptr, "NAME=", 5)) {

            tmp_ptr = tmp_ptr + 5;
            value_len = suffix_ptr - tmp_ptr;
            if (0 != bt_res_get_value(tmp_ptr, value_len,
                                      out_str_buf, out_buf_len)) {
                APP_TRACE("%s: Failed to get value of NAME. res: %s",
                          __FUNCTION__,
                          data_buf);
                return -1;
            }

            *p_res_type = emBT_UART_RES_NAME;
            return 0;

        } else {
            APP_TRACE("%s: Unsupported response: %s", __FUNCTION__, tmp_ptr);
            return -1;
        }
    }
}


/*
 * bt requests
 */

static int bt_req_get_version(char *out_buf, size_t out_buf_len)
{
    emBT_UART_CMD res_type;

    // clear buffer
    CYCLE_BUFFER_clear(gs_bt_param.recv_buf);

    // send VER=?
    if (0 != bt_uart_cmd_req(emBT_UART_REQ_VER, NULL)) {
        APP_TRACE("%s: Failed request to get bluetooth firmware version", __FUNCTION__);
        return -1;
    }

    // recv VER=XXX
    if (0 != bt_uart_cmd_recv_res(&res_type, 2000, out_buf, out_buf_len)) {
        APP_TRACE("%s: Failed to get response of get bluetooth firmware version", __FUNCTION__);
        return -1;
    }

    if (emBT_UART_RES_VER != res_type) {
        APP_TRACE("%s: Failed to get bluetooth firmware version", __FUNCTION__);
        return -1;
    } else {
        return 0;
    }
}

static int bt_req_get_name(char *out_buf, size_t out_buf_len)
{
    emBT_UART_CMD res_type;

    // clear buffer
    CYCLE_BUFFER_clear(gs_bt_param.recv_buf);

    // send BTNAME=?
    if (0 != bt_uart_cmd_req(emBT_UART_REQ_GET_NAME, NULL)) {
        APP_TRACE("%s: Failed request to get bluetooth name", __FUNCTION__);
        return -1;
    }

    // recv NAME=XXX
    if (0 != bt_uart_cmd_recv_res(&res_type, 2000, out_buf, out_buf_len)) {
        APP_TRACE("%s: Failed to get response of get bluetooth name", __FUNCTION__);
        return -1;
    }

    if (emBT_UART_RES_NAME != res_type) {
        APP_TRACE("%s: Failed to get bluetooth name", __FUNCTION__);
        return -1;
    } else {
        return 0;
    }
}

static int bt_req_set_name(char *set_name)
{
    emBT_UART_CMD res_type;

    // clear buffer
    CYCLE_BUFFER_clear(gs_bt_param.recv_buf);

    if (0 != bt_uart_cmd_req(emBT_UART_REQ_SET_NAME, set_name)) {
        APP_TRACE("%s: Failed request to set bluetooth name", __FUNCTION__);
        return -1;
    }

    // recv NAME=OK
    if (0 != bt_uart_cmd_recv_res(&res_type, 2000, NULL, 0)) {
        APP_TRACE("%s: Failed to get response of set bluetooth name", __FUNCTION__);
        return -1;
    }

    if (emBT_UART_RES_OK != res_type) {
        APP_TRACE("%s: Failed to set bluetooth name", __FUNCTION__);
        return -1;
    } else {
        return 0;
    }
}

static int bt_req_open_hotspot( void )
{
    emBT_UART_CMD res_type;

    // clear buffer
    CYCLE_BUFFER_clear(gs_bt_param.recv_buf);

    // send WAKE
    if (0 != bt_uart_cmd_req(emBT_UART_REQ_WAKE, NULL)) {
        APP_TRACE("%s: Failed request to wake bluetooth", __FUNCTION__);
        return -1;
    }

    // recv OK
    if (0 != bt_uart_cmd_recv_res(&res_type, 5000, NULL, 0)) {
        APP_TRACE("%s: Failed to get response of wake bluetooth", __FUNCTION__);
        return -1;
    }

    if (emBT_UART_RES_OK != res_type) {
        APP_TRACE("%s: Failed to wake bluetooth", __FUNCTION__);
        return -1;
    } else {
        return 0;
    }
}

static int bt_req_close_hotspot( void )
{
    emBT_UART_CMD res_type;

    // clear buffer
    CYCLE_BUFFER_clear(gs_bt_param.recv_buf);

    // send SLEEP
    if (0 != bt_uart_cmd_req(emBT_UART_REQ_SLEEP, NULL)) {
        APP_TRACE("%s: Failed request to sleep bluetooth", __FUNCTION__);
        return -1;
    }

    // recv OK
    if (0 != bt_uart_cmd_recv_res(&res_type, 1000, NULL, 0)) {
        APP_TRACE("%s: Failed to get response of sleep bluetooth", __FUNCTION__);
        return -1;
    }

    if (emBT_UART_RES_OK != res_type) {
        APP_TRACE("%s: Failed to get sleep bluetooth", __FUNCTION__);
        return -1;
    } else {
        return 0;
    }
}


/*
 * functions do some thing
 */
static void bt_stop_thr_recv( void )
{
    unsigned int wait_total_us = 0;
    unsigned int a_period_us = 0;
    unsigned int wait_us = 200 * 1000;

    unsigned int timeout_us = kBT_WAIT_THR_QUIT_TIMEO_MS * 1000;

    gs_bt_param.recv_thr.trigger = false;
    while (1) {
        if (gs_bt_param.recv_thr.running) {

            if (wait_total_us > timeout_us) {
                APP_TRACE("%s: Wait too long for recv thread to quit. timeout: %ums",
                          __FUNCTION__, timeout_us/1000);
                break;
            }

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

    gs_bt_param.recv_thr.running = false;
}


static int bt_ensure_right_name( void )
{
    size_t cur_name_buf_len = 64;
    char cur_name[cur_name_buf_len];

    size_t set_name_len;
    char *set_name;

    int try_time = 10;


    set_name_len = strlen(gs_bt_param.hotspot_name);
    if (0 == set_name_len) {
        APP_TRACE("%s: Length of bluetooth name should bigger than 0!", __FUNCTION__);
        return -1;
    } else {
        set_name = gs_bt_param.hotspot_name;
    }

    while (1) {

        if (try_time > 0) {
            try_time--;
        } else {
            return -1;
        }

        // get current bluetooth name
        if (0 != bt_req_get_name(cur_name, cur_name_buf_len)) {
            usleep(500000);
        } else {

            // if name right
            //     return 0;
            // if name wrong
            //     set name and recheck
            if (0 == strcmp(cur_name, set_name)) {
                return 0;
            } else {
                APP_TRACE("%s: Bluetooth hotspot name %s is wrong, should be: %s. setting it to right...",
                          __FUNCTION__, cur_name, set_name);

                if (0 != bt_req_set_name(set_name)) {
                    APP_TRACE("%s: failed to set bluetooth name",
                              __FUNCTION__);
                } else {
                    usleep(50000);
                }
            }
        }
    }
}


/*
 * thread
 */
static void *bt_thr_recv(void *arg)
{
    int ret;
    size_t buf_len = 2048;
    uint8_t buf[buf_len];
    ssize_t data_sz = 0;
    int recv_sz;

    pthread_detach(pthread_self());

    prctl(PR_SET_NAME, "bt_thr_recv");


    while (gs_bt_param.recv_thr.trigger) {

        data_sz = gs_bt_param.bt_uart->Recv_Nb(gs_bt_param.bt_uart, buf, buf_len - 1);
        if (data_sz > 0) {
            if (data_sz < buf_len) {
                recv_sz = data_sz;
            } else {
                recv_sz = buf_len - 1;
                APP_TRACE("%s: %d bytes discarded! (save: %d, total: %d)",
                          __FUNCTION__,
                          data_sz - recv_sz,
                          recv_sz, data_sz);
            }
            buf[recv_sz] = '\0';
//            APP_TRACE("%s: %d bytes come in: %s", __FUNCTION__, data_sz, buf);

            ret = CYCLE_BUFFER_write(gs_bt_param.recv_buf,
                                          buf,
                                          recv_sz);
            if (0 != ret) {
                APP_TRACE("%s: Failed write data to cycle buffer!", __FUNCTION__);
            } else {
                usleep(50000);
            }
        } else {
            usleep(50000);
        }
    }

    gs_bt_param.recv_thr.running = false;
    return NULL;
}

static int bt_ensure( void )
{
    // ensure hotspot name is right
    if (0 != bt_ensure_right_name()) {
        APP_TRACE("%s: failed ensure bluetooth name right!", __FUNCTION__);
        return -1;
    }

    return 0;

}

/*
 * Interfaces
 */

int BT_init( void )
{
    int ret;

    size_t version_buf_len = 32;
    char version_buf[version_buf_len];


    pthread_mutex_lock(&gs_bt_param.bt_lock);

    if (gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is already inited", __FUNCTION__);
        goto NORMAL_RETURN;
    }


    // init cycle buffer
    gs_bt_param.recv_buf = CYCLE_BUFFER_new(kBT_RECVBUF_SIZE);
    if (NULL == gs_bt_param.recv_buf) {
        APP_TRACE("%s: failed to create recv buffer!", __FUNCTION__);
        goto FAIL_RETURN;
    }

    // init bluetooth serial communication
    gs_bt_param.bt_uart = PUART_Struct(kBT_SERIAL_PORT_PATH);
    if (NULL == gs_bt_param.bt_uart) {
        APP_TRACE("%s: failed to open bluetooth serial port!", __FUNCTION__);
        goto FAIL_RETURN;
    } else {
        Uart2 *uart;
        uart = gs_bt_param.bt_uart;
        APP_TRACE("Set baudrate 115200");
        APP_TRACE("Set databit 8");
        APP_TRACE("Set stopbit 1");
        APP_TRACE("Set parity NONE");
        uart->SetBaud(uart, 115200);
        uart->SetDatabit(uart, 8);
        uart->SetStopbit(uart, 1);
        uart->SetParity(uart, 0);
    }

    // create recv thread
    gs_bt_param.recv_thr.trigger = true;
    gs_bt_param.recv_thr.running = true;
    ret = pthread_create(&(gs_bt_param.recv_thr.pid), NULL, bt_thr_recv, NULL);
    if (0 != ret) {
        gs_bt_param.recv_thr.trigger = false;
        gs_bt_param.recv_thr.running = false;
        APP_TRACE("%s: failed to create recv thread!", __FUNCTION__);
        goto FAIL_RETURN;
    }

    // get bt firmware version
    if (0 != bt_req_get_version(version_buf, version_buf_len)) {
        APP_TRACE("%s: failed to get bluetooth firmware version!", __FUNCTION__);
    } else {
        APP_TRACE("%s: Bluetooth firmware version: %s", __FUNCTION__, version_buf);
    }


    gs_bt_param.is_inited = true;

NORMAL_RETURN:

    pthread_mutex_unlock(&gs_bt_param.bt_lock);
    return 0;

FAIL_RETURN:

    bt_stop_thr_recv();

    if (NULL != gs_bt_param.bt_uart) {
        PUART_Destruct(&gs_bt_param.bt_uart);
        gs_bt_param.bt_uart = NULL;
    }

    if (NULL != gs_bt_param.recv_buf) {
        if (0 != CYCLE_BUFFER_free(gs_bt_param.recv_buf)) {
            APP_TRACE("%s: failed to free recv buffer!!", __FUNCTION__);
        }
        gs_bt_param.recv_buf = NULL;
    }

    pthread_mutex_unlock(&gs_bt_param.bt_lock);
    return -1;
}
int BT_deinit( void )
{
    pthread_mutex_lock(&gs_bt_param.bt_lock);

    if (!gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is not inited", __FUNCTION__);
        goto NORMAL_RETURN;
    }

    gs_bt_param.is_inited = false;

    // close hotspot
    if (0 != bt_req_close_hotspot()) {
        APP_TRACE("%s: failed to close bluetooth hotspot!", __FUNCTION__);
    }
    gs_bt_param.is_hotspot_on = false;

    // stop recv thread
    bt_stop_thr_recv();

    // close serial port
    PUART_Destruct(&gs_bt_param.bt_uart);
    gs_bt_param.bt_uart = NULL;

    // free recv buffer
    if (0 != CYCLE_BUFFER_free(gs_bt_param.recv_buf)) {
        APP_TRACE("%s: failed to free recv buffer!!", __FUNCTION__);
    }
    gs_bt_param.recv_buf = NULL;



NORMAL_RETURN:

    pthread_mutex_unlock(&gs_bt_param.bt_lock);
    return 0;
}

bool BT_is_init()
{
    return gs_bt_param.is_inited;

}

bool BT_is_hotspot_on()
{
    return gs_bt_param.is_hotspot_on;

}

// hotspot_name can't be NULL
int BT_open_hotspot(char *hotspot_name)
{
    int ret;
    size_t in_name_len;
    size_t name_buf_len;

    if (NULL == hotspot_name) {
        APP_TRACE("%s: hotspot_name can't be NULL!", __FUNCTION__);
        return -1;
    }

    in_name_len = strlen(hotspot_name);
    if (in_name_len > kBT_HOTSPOT_NAME_MAXLEN) {
        APP_TRACE("%s: hotspot_name(%s) too long! max length allowed: %d",
                  __FUNCTION__, hotspot_name, kBT_HOTSPOT_NAME_MAXLEN);
        return -1;
    }

    pthread_mutex_lock(&gs_bt_param.bt_lock);

    // in lock, is inited?
    if (!gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is not inited", __FUNCTION__);
        goto FAIL_RETURN;
    }

    // in lock, is hotspot opened?
    if (gs_bt_param.is_hotspot_on) {
        APP_TRACE("%s: bluetooth hotspot is already on", __FUNCTION__);
        goto NORMAL_RETURN;
    }

    // copy hotspot name
    name_buf_len = sizeof(gs_bt_param.hotspot_name);
    ret = snprintf(gs_bt_param.hotspot_name, name_buf_len,
                   "%s", hotspot_name);
    if (ret >= name_buf_len) {
        APP_TRACE("%s: snprintf failed!", __FUNCTION__);
        goto FAIL_RETURN;
    }

    if(0 != bt_ensure()) {
        goto FAIL_RETURN;

    }

    // in lock, open hotspot
    if (0 != bt_req_open_hotspot()) {
        goto FAIL_RETURN;

    }

    gs_bt_param.is_hotspot_on = true;

NORMAL_RETURN:

    pthread_mutex_unlock(&gs_bt_param.bt_lock);
    return 0;

FAIL_RETURN:

    pthread_mutex_unlock(&gs_bt_param.bt_lock);
    return -1;
}
int BT_close_hotspot( void )
{
    pthread_mutex_lock(&gs_bt_param.bt_lock);

    // in lock, is inited?
    if (!gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is not inited", __FUNCTION__);
        goto FAIL_RETURN;
    }

    // in lock, is hotspot opened?
    if (!gs_bt_param.is_hotspot_on) {
        APP_TRACE("%s: bluetooth hotspot is not on", __FUNCTION__);
        goto NORMAL_RETURN;
    }

    gs_bt_param.is_hotspot_on = false;

    // in lock, close hotspot
    bt_req_close_hotspot();


NORMAL_RETURN:

    pthread_mutex_unlock(&gs_bt_param.bt_lock);
    return 0;

FAIL_RETURN:

    pthread_mutex_unlock(&gs_bt_param.bt_lock);
    return -1;
}

ssize_t BT_get_recvbuf_data_sz( void )
{
    size_t data_sz;

    // is inited?
    if (!gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is not inited", __FUNCTION__);
        return -1;
    }

    // is hotspot opened?
    if (!gs_bt_param.is_hotspot_on) {
        APP_TRACE("%s: bluetooth hotspot is not on", __FUNCTION__);
        return -1;
    }


    if (0 == CYCLE_BUFFER_get_data_sz(gs_bt_param.recv_buf, &data_sz)) {
        return data_sz;
    } else {
        return -1;
    }
}

int BT_read(uint8_t *p_data, size_t read_size)
{
    int ret;

    // is inited?
    if (!gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is not inited", __FUNCTION__);
        return -1;
    }

    // is hotspot opened?
    if (!gs_bt_param.is_hotspot_on) {
        APP_TRACE("%s: bluetooth hotspot is not on", __FUNCTION__);
        return -1;
    }

    // read
    ret = CYCLE_BUFFER_read(gs_bt_param.recv_buf, p_data, read_size);
    if (0 == ret) {
        return 0;
    } else {
        return -1;
    }
}
int BT_peek(uint8_t *p_data, size_t peek_size)
{
    int ret;

    // is inited?
    if (!gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is not inited", __FUNCTION__);
        return -1;
    }

    // is hotspot opened?
    if (!gs_bt_param.is_hotspot_on) {
        APP_TRACE("%s: bluetooth hotspot is not on", __FUNCTION__);
        return -1;
    }

    // peek
    ret = CYCLE_BUFFER_peek(gs_bt_param.recv_buf, p_data, peek_size);
    if (0 == ret) {
        return 0;
    } else {
        return -1;
    }
}
int BT_skip(size_t skip_size)
{
    int ret;

    // is inited?
    if (!gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is not inited", __FUNCTION__);
        return -1;
    }

    // is hotspot opened?
    if (!gs_bt_param.is_hotspot_on) {
        APP_TRACE("%s: bluetooth hotspot is not on", __FUNCTION__);
        return -1;
    }

    // skip
    ret = CYCLE_BUFFER_skip(gs_bt_param.recv_buf, skip_size);
    if (0 == ret) {
        return 0;
    } else {
        return -1;
    }
}

int BT_write(uint8_t *p_data, size_t write_size)
{
    int ret;
    size_t written_size = 0;
    size_t once_write_size = 0;
    const size_t once_fix_size = 64;

    // is inited?
    if (!gs_bt_param.is_inited) {
        APP_TRACE("%s: bluetooth is not inited", __FUNCTION__);
        return -1;
    }

    // is hotspot opened?
    if (!gs_bt_param.is_hotspot_on) {
        APP_TRACE("%s: bluetooth hotspot is not on", __FUNCTION__);
        return -1;
    }

    pthread_mutex_lock(&gs_bt_param.w_lock);

    while (true) {

        if (written_size + once_fix_size > write_size) {
            once_write_size = write_size - written_size;
        } else {
            once_write_size = once_fix_size;
        }

        ret = bt_write_to_uart(p_data + written_size, once_write_size);
        if (0 != ret) {
            break;
        } else {
            written_size += once_write_size;
            if (written_size >= write_size) {
                break;
            } else {
                usleep(50000);
            }
        }
    }

    pthread_mutex_unlock(&gs_bt_param.w_lock);

    if (0 == ret) {
        return 0;
    } else {
        APP_TRACE("%s: failed write to serial port", __FUNCTION__);
        return -1;
    }
}




#endif //defined(BLUETOOTH)
