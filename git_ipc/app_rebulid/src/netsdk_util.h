
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "http_util.h"

#ifndef NETSDKV10_UTIL_H_
#define NETSDKV10_UTIL_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int NETSDK_response(int sock, HTTP_CSTR_t content);
extern int NETSDK_response_file(int sock, HTTP_CSTR_t mime, const char *file_path);

extern int NETSDK_return_ok(int sock, HTTP_CSTR_t mesg);
extern int NETSDK_return_failed(int sock, HTTP_CSTR_t mesg);

extern void NETSDK_json_add_text(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, HTTP_CSTR_t var_val, HTTP_CSTR_t comment);
extern void NETSDK_json_add_dec(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, int var_val, HTTP_CSTR_t comment);
extern void NETSDK_json_add_hex(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, unsigned int var_val, HTTP_CSTR_t comment);
extern void NETSDK_json_add_float(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, float var_val, HTTP_CSTR_t comment);
extern void NETSDK_json_add_percent(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, float var_val, HTTP_CSTR_t comment);
extern void NETSDK_json_add_boolean(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, bool var_val, HTTP_CSTR_t comment);

extern HTTP_CSTR_t NETSDK_read_text(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, HTTP_CSTR_t def_val);
extern int NETSDK_read_dec(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, int ref_max, int def_val);
extern unsigned int NETSDK_read_hex(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, unsigned int def_val);
extern float NETSDK_read_float(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, float def_val);
extern bool NETSDK_read_boolean(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, bool def_val);

#ifdef __cplusplus
};
#endif
#endif //NETSDKV10_UTIL_H_

