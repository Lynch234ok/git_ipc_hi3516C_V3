
#include <time.h>
#include <sys/timeb.h>
#include "web_server.h"
#include "json/json.h"

#ifndef NETSDKV10_JSON_H_
#define NETSDKV10_JSON_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct json_object ST_JSON_OBJECT,*LP_JSON_OBJECT;

extern LP_JSON_OBJECT NETSDK_json_parse(const char *str);

extern LP_JSON_OBJECT NETSDK_json_dup(LP_JSON_OBJECT jso);

extern bool NETSDK_json_check_child(LP_JSON_OBJECT jso, const char *key);
extern LP_JSON_OBJECT NETSDK_json_get_child(LP_JSON_OBJECT jso, const char *keys);
extern int NETSDK_json_copy_child(LP_JSON_OBJECT src_jso, LP_JSON_OBJECT dst_jso, const char *key);

extern const char *NETSDK_json_get_string(LP_JSON_OBJECT jso, const char *key, char *result, int result_max);
extern int NETSDK_json_get_int(LP_JSON_OBJECT jso, const char *key);
extern float NETSDK_json_get_float(LP_JSON_OBJECT jso, const char *key);
extern double NETSDK_json_get_dfloat(LP_JSON_OBJECT jso, const char *key);
extern bool NETSDK_json_get_boolean(LP_JSON_OBJECT jso, const char *key);

extern void NETSDK_json_set_string(LP_JSON_OBJECT jso, const char *key, const char *val);
extern void NETSDK_json_set_int(LP_JSON_OBJECT jso, const char *key, int val);
extern void NETSDK_json_set_float(LP_JSON_OBJECT jso, const char *key, float val);
extern void NETSDK_json_set_dfloat(LP_JSON_OBJECT jso, const char *key, double val);
extern void NETSDK_json_set_boolean(LP_JSON_OBJECT jso, const char *key, bool val);

extern int NETSDK_json_set_string2(LP_JSON_OBJECT jso, const char *key, const char *val);
extern void NETSDK_json_set_int2(LP_JSON_OBJECT jso, const char *key, int val);
extern void NETSDK_json_set_float2(LP_JSON_OBJECT jso, const char *key, float val);
extern void NETSDK_json_set_dfloat2(LP_JSON_OBJECT jso, const char *key, double val);
extern void NETSDK_json_set_boolean2(LP_JSON_OBJECT jso, const char *key, bool val);

extern void NETSDK_json_remove_properties(LP_JSON_OBJECT channel);
extern int NETSDK_json_remove_property_option(LP_JSON_OBJECT jso, const char *key,  const void *val_ref);
extern int NETSDK_json_get_md_array(LP_JSON_OBJECT obj,unsigned int array[],int *row,int *column);
extern int NETSDK_json_set_md_array(LP_JSON_OBJECT obj,unsigned int array[],int row,int column);
extern int NETSDK_json_copy_md_array(LP_JSON_OBJECT src_jso, LP_JSON_OBJECT dst_jso, const char *key);
extern void NETSDK_json_set_array2(LP_JSON_OBJECT dst_jso, LP_JSON_OBJECT src_jso, char *key);
extern void NETSDK_json_set_array(LP_JSON_OBJECT dst_jso, LP_JSON_OBJECT src_jso, char *val);

extern LP_JSON_OBJECT NETSDK_json_load(const char *filename);
extern int NETSDK_json_save(LP_JSON_OBJECT jso, const char *conf_path);


#ifdef __cplusplus
};
#endif
#endif //NETSDKV10_JSON_H_

