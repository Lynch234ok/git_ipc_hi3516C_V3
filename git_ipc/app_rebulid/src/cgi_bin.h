#ifndef _CGI_BIN_H_
#define _CGI_BIN_H_

#define CRLF "\r\n"

#ifdef _cplusplus
extern "C"{
#endif
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "http_util.h"
#include "http_server.h"
extern int WEB_CGI_moo(LP_HTTP_CONTEXT session);
extern int WEB_CGI_whoami(LP_HTTP_CONTEXT session);
extern int WEB_CGI_shell(LP_HTTP_CONTEXT session);
extern int WEB_CGI_snapshot(LP_HTTP_CONTEXT session);
extern int WEB_CGI_cpu_temperature(LP_HTTP_CONTEXT session);
extern int WEB_CGI_mjpeg_html(LP_HTTP_CONTEXT session);
extern int WEB_CGI_mjpeg(LP_HTTP_CONTEXT session);
extern int WEB_CGI_sdk_reg_rw(LP_HTTP_CONTEXT session);
extern int WEB_CGI_focus_measure(LP_HTTP_CONTEXT session);
extern int WEB_CGI_get_dana_id_QRCode(LP_HTTP_CONTEXT session);
extern int WEB_CGI_dana_uid(LP_HTTP_CONTEXT session);
extern int WEB_CGI_isp_cfg(LP_HTTP_CONTEXT session);
extern int WEB_CGI_cal_defect_pixel(LP_HTTP_CONTEXT session);
extern int WEB_CGI_factory_test(LP_HTTP_CONTEXT session);
extern int WEB_CGI_factory_setting(LP_HTTP_CONTEXT session);
#ifdef _cplusplus
}
#endif
#endif//_CGI_BIN_