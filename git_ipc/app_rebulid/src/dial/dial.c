#if defined(DIAL)


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <dial.h>
#include <base/ja_process.h>
#include <uart2.h>
#include <app_debug.h>


/*
 * Type
 */
typedef enum {

    emDIAL_UART_REQ_CIMI,
    emDIAL_UART_REQ_CREG,

    emDIAL_UART_RES_OK,
    emDIAL_UART_RES_CREG,

} emDIAL_UART_CMD;

typedef struct {

    bool trigger;
    bool running;
    pthread_t pid;

} stDIAL_thr_params;

typedef struct {

    stDIAL_thr_params deamon_thr;

    pthread_mutex_t init_lock;

} stDIAL_params;


/*
 * Macro
 */
#define DIAL_SCRIPT_PATH IPCAM_ENV_HOME_DIR"/shell/ppp/"
#define DIAL_TTYUSB_PREFIX "/dev/ttyUSB"
#define DIAL_NET_CHECK_INTERVAL_S 10
#define DIAL_IF_NAME "ppp0"

/*
 * Global
 */
static emDIAL_ISP gs_dial_isp = emDIAL_ISP_NONE;

static char *gs_prefixes[] = {"46000", "46001",
                              "46002", "46003",
                              "46005", "46006",
                              "46007", "46011"};
static emDIAL_ISP gs_isps[] = {emDIAL_ISP_MOBILE_GSM, emDIAL_ISP_UNICOM_GSM,
                               emDIAL_ISP_MOBILE_TDS1, emDIAL_ISP_TELECOM_CDMA1,
                               emDIAL_ISP_TELECOM_CDMA2, emDIAL_ISP_UNICOM_WCDMA,
                               emDIAL_ISP_MOBILE_TDS2, emDIAL_ISP_TELECOM_FDDLTE};

static stDIAL_params gs_dial_params = {
        .deamon_thr.trigger = false,
        .deamon_thr.running = false,
        .init_lock = PTHREAD_MUTEX_INITIALIZER,
};


/*
 * Prototype
 */
static void *dial_thr_deamon(void *arg);
static ssize_t dial_recv_from_uart_with_timeo(Uart2* uart,
                                              uint8_t* recvBuf, size_t recvBufLen,
                                              uint32_t timeo_ms);


/*
 * Helper function
 */
static void printf_bin(char *buf, size_t buf_len)
{
    int i;

    printf("%02X", (uint8_t)(buf[0]));

    for (i = 1; i < buf_len; i++) {
        printf(" %02X", (uint8_t)(buf[i]));
    }
}

static void dial_sleep_us(uint32_t timeout_us)
{
    uint32_t wait_once_us = 30 * 1000;
    uint32_t wait_total_us = 0;

    if (timeout_us <= (2 * wait_once_us)) {
        usleep(timeout_us);
        return;
    } else {
        while (gs_dial_params.deamon_thr.trigger) {
            usleep(wait_once_us);
            wait_total_us += wait_once_us;
            if (wait_total_us >= timeout_us) {
                return;
            }
        }
    }
}

static const char *dial_get_ip_str(const struct sockaddr *sa,
                                   char *str_buf, socklen_t str_buf_len)
{
    const char *ret;
    switch(sa->sa_family) {
        case AF_INET:
            ret = inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                            str_buf, str_buf_len);
            if (NULL == ret) {
                APP_TRACE("%s: Failed to get IPv4 string", __FUNCTION__);
            }
            break;

        case AF_INET6:
            ret = NULL;
            APP_TRACE("%s: IPv6 is not supported", __FUNCTION__);
            break;

        default:
            ret = NULL;
            APP_TRACE("%s: Unsupported address family: %d",
                      __FUNCTION__, sa->sa_family);
    }

    return ret;
}

/*
 * Function do some thing
 */
static bool _dial_is_ok(void)
{
    int fd;
    bool ret;
    struct ifreq ifr;
    char *if_ppp_name = DIAL_IF_NAME;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        APP_TRACE("%s: Failed to create socket! errno: %d",
                  __FUNCTION__, errno);
        return false;
    }

    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", if_ppp_name);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        APP_TRACE("%s: ioctl for %s failed, errno: %d",
                  __FUNCTION__, if_ppp_name, errno);
        ret = false;
    } else if (0 == (ifr.ifr_flags & IFF_UP)) {
        APP_TRACE("%s: interface %s DOWN", __FUNCTION__, if_ppp_name);
        ret = false;
    } else if (0 == (ifr.ifr_flags & IFF_RUNNING)) {
        APP_TRACE("%s: interface %s NOT RUNNING", __FUNCTION__, if_ppp_name);
        ret = false;
    } else {
        ret = true;
    }

    close(fd);
    return ret;
}

static int dial_check_with_timeo(struct sockaddr *p_sa, uint32_t timeout_ms)
{
    int fd;
    struct ifreq ifr;
    char *if_ppp_name = DIAL_IF_NAME;
    bool is_ok = false;

    uint32_t wait_once_us = 500 * 1000;
    uint32_t wait_total_us = 0;
    uint32_t timeout_us = timeout_ms * 1000;

    while (gs_dial_params.deamon_thr.trigger) {
        if (_dial_is_ok()) {
            is_ok = true;
            break;
        } else {
            if (wait_total_us > timeout_us) {
                return -1;
            } else {
                dial_sleep_us(wait_once_us);
                wait_total_us += wait_once_us;
            }
        }
    }

    if (!is_ok) {
        return -1;
    }

    // get ip
    int ret = -1;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        APP_TRACE("%s: Failed to create socket! errno: %d",
                  __FUNCTION__, errno);
        return -1;
    }

    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", if_ppp_name);
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        APP_TRACE("%s: ioctl for %s failed, errno: %d",
                  __FUNCTION__, if_ppp_name, errno);
        ret = -1;
    } else {
        *p_sa = ifr.ifr_addr;
        ret = 0;
    }

    close(fd);
    return ret;
}

static ssize_t dial_recv_from_uart_with_timeo(Uart2* uart,
                                              uint8_t* recvBuf, size_t recvBufLen,
                                              uint32_t timeo_ms)
{
    ssize_t ret;
    uint32_t timeo_us = timeo_ms * 1000;
    uint32_t wait_once_us = 50 * 1000;
    uint32_t wait_total_us = 0;

    while (gs_dial_params.deamon_thr.trigger)
    {
        ret = uart->Recv_Nb(uart, recvBuf, recvBufLen);
        if (ret != 0) {
            return ret;
        } else {
            if (wait_total_us > timeo_us) {
                return ret;
            } else {
                usleep(wait_once_us);
                wait_total_us += wait_once_us;
            }
        }
    }

    return -1;
}


/*
 * Request and response
 */
static int dial_uart_req(Uart2* uart, emDIAL_UART_CMD send_req)
{
    ssize_t ret;
    char *send_str;

    switch (send_req) {
        case emDIAL_UART_REQ_CIMI:
            send_str = "AT+CIMI\r\n";
            break;

        case emDIAL_UART_REQ_CREG:
            send_str = "AT+CREG?\r\n";
            break;

        default:
            APP_TRACE("%s: Unsupported req: %d", __FUNCTION__, send_req);
            return -1;
    }

    ret = uart->Send(uart, (uint8_t *)send_str, strlen(send_str));
    if (ret <= 0) {
        return -1;
    } else {
        return 0;
    }
}


static int dial_uart_recv_res(Uart2* uart,
                              uint32_t timeout_ms,
                              emDIAL_UART_CMD *p_res_type,
                              char *recvBuf,
                              size_t recvBufLen)
{
    ssize_t ret;

    ret = dial_recv_from_uart_with_timeo(uart, (uint8_t *)recvBuf, recvBufLen - 1, timeout_ms);
    if (ret <= 0) {
        APP_TRACE("dial_recv_from_uart_with_timeo error, ret: %zd", ret);
        return -1;
    }

    recvBuf[ret] = '\0';

    printf("recv: ****%s****\n", recvBuf);
    printf("data:\n****");
    printf_bin(recvBuf, (size_t)ret);
    printf("****\n");


    if (NULL != strstr(recvBuf, "+CREG:")) {
        *p_res_type = emDIAL_UART_RES_CREG;
        return 0;
    } else if (NULL != strstr(recvBuf, "OK\r\n")) {
        *p_res_type = emDIAL_UART_RES_OK;
        return 0;
    } else {
        APP_TRACE("%s: Unsupported response: %s", __FUNCTION__, recvBuf);
        return -1;
    }
}

static emDIAL_ISP dial_get_isp_from_response(char *response, size_t ary_len)
{
    char *p_num_start;
    char *p_num_end;
    ssize_t num_len;
    char *CRLF = "\r\n";
    emDIAL_ISP dial_isp = emDIAL_ISP_NONE;
    size_t i = 0;
    size_t prefix_len = 5;


    p_num_start = strstr(response, CRLF);
    if (NULL == p_num_start) {
        APP_TRACE("%s: Failed to first CRLF in res. res: %s", __FUNCTION__, response);
        return emDIAL_ISP_NONE;
    }
    p_num_start = p_num_start + 2;


    p_num_end = strstr(p_num_start, CRLF);
    if (NULL == p_num_end) {
        APP_TRACE("%s: Failed to second CRLF in res. res: %s", __FUNCTION__, response);
        return emDIAL_ISP_NONE;
    }

    num_len = p_num_end - p_num_start;
    if (num_len < prefix_len) {
        APP_TRACE("%s: num too short, at least be %zu. res: %s",
               __FUNCTION__, prefix_len, response);
        return emDIAL_ISP_NONE;
    }

    for (i = 0; i < ary_len; i++) {
        if (0 == strncmp(p_num_start, gs_prefixes[i], prefix_len)) {
            dial_isp = gs_isps[i];
            break;
        }
    }

    if (emDIAL_ISP_NONE == dial_isp) {
        APP_TRACE("Can't find ISP in the array. num_start: %s", p_num_start);
    }

    return dial_isp;
}

// request interfaces
static emDIAL_ISP dial_req_get_isp(char *out_ttyBuf, size_t out_ttfBufLen)
{
    int i;
    int ret;
    Uart2 *uart = NULL;
    size_t usbDevBufLen = 32;
    char usbDevBuf[usbDevBufLen];
    size_t recvBufLen = 1024;
    char recvBuf[recvBufLen];
    size_t prefix_ary_len;
    size_t isp_ary_len;
    emDIAL_ISP dial_isp = emDIAL_ISP_NONE;
    emDIAL_UART_CMD dial_uart_res;


    prefix_ary_len = sizeof(gs_prefixes)/sizeof(char *);
    isp_ary_len = sizeof(gs_isps)/sizeof(emDIAL_ISP);
    if (0 == prefix_ary_len
        || 0 == isp_ary_len) {
        APP_TRACE("%s: prefix or isp array can't be empty! prefix_ary_len: %zu, isp_ary_len: %zu",
                  __FUNCTION__, prefix_ary_len, isp_ary_len);
        goto FAIL_RETURN;
    }

    if (prefix_ary_len != isp_ary_len) {
        APP_TRACE("%s: members in prefix and isp arrays should be equal! prefix_ary_len: %zu, isp_ary_len: %zu",
                  __FUNCTION__, prefix_ary_len, isp_ary_len);
        goto FAIL_RETURN;
    }


    i = 1;
    snprintf(usbDevBuf, usbDevBufLen, "%s%d", DIAL_TTYUSB_PREFIX, i);
    while (1) {
        uart = PUART_Struct(usbDevBuf);
        if (NULL != uart) {
            break;
        } else {
            APP_TRACE("%s: PUART_Struct %s failed", __FUNCTION__, usbDevBuf);
            if (i >= 5) {
                goto FAIL_RETURN;
            } else {
                i++;
            }
            snprintf(usbDevBuf, usbDevBufLen, "%s%d", DIAL_TTYUSB_PREFIX, i);
            APP_TRACE("%s: Retry %s", __FUNCTION__, usbDevBuf);
        }
    }

    uart->SetBaud(uart, 115200);
    uart->SetDatabit(uart, 8);
    uart->SetStopbit(uart, 1);
    uart->SetParity(uart, 0);

    ret = dial_uart_req(uart, emDIAL_UART_REQ_CIMI);
    if (0 != ret) {
        APP_TRACE("%s: dial_uart_req failed", __FUNCTION__);
        goto FAIL_RETURN;
    }

    ret = dial_uart_recv_res(uart, 3000, &dial_uart_res, recvBuf, recvBufLen);
    if (0 != ret) {
        APP_TRACE("%s: dial_uart_recv_res failed", __FUNCTION__);
        goto FAIL_RETURN;
    }

    if (emDIAL_UART_RES_OK != dial_uart_res) {
        APP_TRACE("%s: Res is not OK, dial_uart_res: %d. res: %s", __FUNCTION__, dial_uart_res, recvBuf);
        goto FAIL_RETURN;
    }

    dial_isp = dial_get_isp_from_response(recvBuf, prefix_ary_len);
    if (emDIAL_ISP_NONE != dial_isp) {
        snprintf(out_ttyBuf, out_ttfBufLen, "%s", usbDevBuf);
    }

    PUART_Destruct(&uart);
    return dial_isp;


FAIL_RETURN:

    if (NULL != uart) {
        PUART_Destruct(&uart);
    }

    return emDIAL_ISP_NONE;
}

// @return  -1, error; 0 not ready; 1 ready
static int dial_req_is_module_net_ready(char *ttyName)
{
    int ret = -1;
    Uart2 *uart = NULL;
    size_t recvBufLen = 1024;
    char recvBuf[recvBufLen + 1];
    emDIAL_UART_CMD dial_uart_res;
    char *str_ptr;
    char *tmp_ptr;
    int creg_stat;

    uart = PUART_Struct(ttyName);
    if (NULL == uart) {
        APP_TRACE("%s: PUART_Struct %s failed", __FUNCTION__, ttyName);
        return -1;
    }

    uart->SetBaud(uart, 115200);
    uart->SetDatabit(uart, 8);
    uart->SetStopbit(uart, 1);
    uart->SetParity(uart, 0);

    ret = dial_uart_req(uart, emDIAL_UART_REQ_CREG);
    if (0 != ret) {
        APP_TRACE("%s: dial_uart_req failed, ret: %d", __FUNCTION__, ret);
        goto FAIL_RETURN;
    }

    ret = dial_uart_recv_res(uart, 3000, &dial_uart_res, recvBuf, recvBufLen);
    if (0 != ret) {
        APP_TRACE("%s: dial_uart_recv_res failed, ret: %d", __FUNCTION__, ret);
        goto FAIL_RETURN;
    }

    if (emDIAL_UART_RES_CREG != dial_uart_res) {
        APP_TRACE("%s: Res is not ok for CREG. res: %s", __FUNCTION__, recvBuf);
        goto FAIL_RETURN;
    }

    str_ptr = strstr(recvBuf, "+CREG:");
    if (NULL == str_ptr) {
        APP_TRACE("%s: Failed to find \"+CREG:\". res: %s", __FUNCTION__, recvBuf);
        goto FAIL_RETURN;
    }

    tmp_ptr = strstr(str_ptr, ",");
    if (NULL == str_ptr) {
        APP_TRACE("%s: Failed to find \",\". res: %s", __FUNCTION__, recvBuf);
        goto FAIL_RETURN;
    }
    tmp_ptr = tmp_ptr + 1;

    creg_stat = atoi(tmp_ptr);
    if (1 != creg_stat
        && 5 != creg_stat) {
        APP_TRACE("%s: Module net not ready, CREG stat: %d. res: %s", __FUNCTION__, creg_stat, recvBuf);
        ret = 0;
    } else {
        ret = 1;
    }

    PUART_Destruct(&uart);
    return ret;


FAIL_RETURN:

    if (NULL != uart) {
        PUART_Destruct(&uart);
    }

    return -1;
}

static int dial_call(void)
{
    int ret;
    int i;
    emDIAL_ISP dial_isp = emDIAL_ISP_NONE;
    char ttyBuf[32];
    char cmd[128];
    int wait_net_timeo_s = 25;
    int waited_s;


    i = 0;
    while (gs_dial_params.deamon_thr.trigger) {
        dial_isp = dial_req_get_isp(ttyBuf, sizeof(ttyBuf));
        APP_TRACE("Got ISP: %d", dial_isp);
        if (emDIAL_ISP_NONE != dial_isp) {
            break;
        } else {
            if (i >= 3) {
                return emDIAL_ISP_NONE;
            } else {
                i++;
                dial_sleep_us(1000000);
            }
        }
    }

    waited_s = 0;
    while (gs_dial_params.deamon_thr.trigger) {
        ret = dial_req_is_module_net_ready(ttyBuf);
        if (1 == ret) {
            break;
        } else if (0 == ret) {
            if (waited_s >= wait_net_timeo_s) {
                APP_TRACE("%s: timeout waiting module net to ready, just to try dial",
                          __FUNCTION__);
                break;
            } else {
                dial_sleep_us(2000000);
                waited_s = waited_s + 2;
            }
        } else {
            return emDIAL_ISP_NONE;
        }
    }

    switch (dial_isp) {
        case emDIAL_ISP_MOBILE_GSM:
        case emDIAL_ISP_MOBILE_TDS1:
        case emDIAL_ISP_MOBILE_TDS2:
            snprintf(cmd, sizeof(cmd), "%sppp-bgn mo %s &",
                     DIAL_SCRIPT_PATH, ttyBuf);
            NK_SYSTEM(cmd);
            break;
        case emDIAL_ISP_UNICOM_GSM:
        case emDIAL_ISP_UNICOM_WCDMA:
            snprintf(cmd, sizeof(cmd), "%sppp-bgn un %s &",
                     DIAL_SCRIPT_PATH, ttyBuf);
            NK_SYSTEM(cmd);
            break;
        case emDIAL_ISP_TELECOM_CDMA1:
        case emDIAL_ISP_TELECOM_CDMA2:
        case emDIAL_ISP_TELECOM_FDDLTE:
            snprintf(cmd, sizeof(cmd), "%sppp-bgn te %s &",
                     DIAL_SCRIPT_PATH, ttyBuf);
            NK_SYSTEM(cmd);
            break;
        default:
            return -1;
    }

    gs_dial_isp = dial_isp;

    return 0;
}


static void dial_stop_program(void)
{
    NK_SYSTEM(DIAL_SCRIPT_PATH"ppp-end &");
    sleep(1);
}

static int dial_worker(void)
{
    socklen_t ip_buf_len = 64;
    char ip_buf[ip_buf_len];
    const char *ip_str;
    struct sockaddr sa;
    char cmd[256];


    if (0 != dial_call()) {
        APP_TRACE("%s: Failed to call", __FUNCTION__);
        return -1;
    }

    if (0 != dial_check_with_timeo(&sa, 15000)) {
        APP_TRACE("%s: Call is checked to failed", __FUNCTION__);
        return -1;
    }

    // set gateway
    ip_str = dial_get_ip_str(&sa, ip_buf, ip_buf_len);
    if (NULL == ip_str) {
        APP_TRACE("%s: Failed to get ip string", __FUNCTION__);
        return -1;
    }
    snprintf(cmd, sizeof(cmd),
             "route add default gw %s %s", ip_str, DIAL_IF_NAME);
    NK_SYSTEM(cmd);


    while (gs_dial_params.deamon_thr.trigger) {
        sleep(DIAL_NET_CHECK_INTERVAL_S);
        if (!_dial_is_ok()) {
            return -1;
        }
    }

    return 0;
}

static int dial_start_deamon(void)
{
    int ret;

    gs_dial_params.deamon_thr.trigger = true;
    gs_dial_params.deamon_thr.running = true;
    ret = pthread_create(&gs_dial_params.deamon_thr.pid,
                         NULL,
                         dial_thr_deamon,
                         NULL);
    if (0 != ret) {
        gs_dial_params.deamon_thr.trigger = false;
        gs_dial_params.deamon_thr.running = false;
        ret = -1;
    }

    return ret;
}

static void dial_stop_deamon(uint32_t timeout_ms)
{
    uint32_t wait_total_us = 0;
    uint32_t a_period_us = 0;
    uint32_t wait_once_us = 20 * 1000;
    uint32_t timeout_us = timeout_ms * 1000;

    gs_dial_params.deamon_thr.trigger = false;
    while (1) {
        if (gs_dial_params.deamon_thr.running) {
            if (wait_total_us > timeout_us) {
                APP_TRACE("%s: Wait deamon_thr timeout to quit!", __FUNCTION__);
                break;
            } else {
                usleep(wait_once_us);
                wait_total_us += wait_once_us;
                a_period_us += wait_once_us;
                if (a_period_us > 5000000) {
                    APP_TRACE("%s: %u Second passed for work thread to quit...",
                              __FUNCTION__, wait_total_us/1000000);
                    a_period_us = 0;
                }
            }
        } else {
            break;
        }
    }

    gs_dial_params.deamon_thr.running = false;
}

// Thread
static void *dial_thr_deamon(void *arg)
{
    uint32_t sleep_us = 3 * 1000 * 1000;

    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "dial deamon");

    dial_stop_program();
    while (gs_dial_params.deamon_thr.trigger) {
        if (0 != dial_worker()) {
            dial_stop_program();
            dial_sleep_us(sleep_us);
        }
    }

    dial_stop_program();
    APP_TRACE("%s: dial deamon thread quit", __FUNCTION__);
    return NULL;
}


// Interface
int DIAL_deamon_run()
{
    int ret;

    pthread_mutex_lock(&gs_dial_params.init_lock);

    if (gs_dial_params.deamon_thr.running) {
        APP_TRACE("%s: Dial deamon already running", __FUNCTION__);
        pthread_mutex_unlock(&gs_dial_params.init_lock);
        return 0;
    } else {
        ret = dial_start_deamon();
        if (0 != ret) {
            APP_TRACE("%s: Failed to start deamon thread!", __FUNCTION__);
            pthread_mutex_unlock(&gs_dial_params.init_lock);
            return -1;
        } else {
            APP_TRACE("%s: Bluetooth for device binding started", __FUNCTION__);
            pthread_mutex_unlock(&gs_dial_params.init_lock);
            return 0;
        }
    }
}

void DIAL_deamon_stop()
{
    pthread_mutex_lock(&gs_dial_params.init_lock);

    if (!gs_dial_params.deamon_thr.running) {
        APP_TRACE("%s: Dial deamon not running yet", __FUNCTION__);
    } else {
        dial_stop_deamon(3000);
    }

    pthread_mutex_unlock(&gs_dial_params.init_lock);
}

bool DIAL_is_ok(void)
{
    return _dial_is_ok();
}

emDIAL_ISP DIAL_get_isp( void )
{
    return gs_dial_isp;
}


#endif //defined(DIAL)
