#ifndef __P2P_JSON_PARSE_H__
#define __P2P_JSON_PARSE_H__
#include "netsdk_util.h"
#include "netsdk_json.h"
#include "netsdk_def.h"

#ifdef __cplusplus
extern "C" {
#endif

extern bool p2p_setup_is_get(void *request);

extern ssize_t p2p_parse(void *request, void *response, int max_response_len);

extern int mtion_setsensitivityLevel(ST_NSDK_MD_CH *md_ch, char *sensitivityLevel);

extern int set_motinfo(LP_JSON_OBJECT motinfo, int sensitivityLevel);
#ifdef __cplusplus
}
#endif
#endif //__P2P_JSON_PARSE_H__

