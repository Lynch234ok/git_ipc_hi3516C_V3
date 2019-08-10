


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#include "http_util.h"
#include "http_server.h"
#include "spook/spook.h"

#ifndef WEB_SERVER_H_
#define WEB_SERVER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kWEBS_SERVER_NAME "IE/10.0"
#define kWEBS_USER_AGENT_NAME kWEBS_SERVER_NAME" "__DATE__

#define kWEBS_KEEP_ALIVE_DURATION (60)

extern int WEBS_init(const char* resource_dir);
extern void WEBS_destroy();

extern int WEBS_add_cgi(HTTP_CSTR_t uri, fHTTP_SERV_CGI_SERVICE service, int accept_method);
extern int WEBS_del_cgi(HTTP_CSTR_t uri);
extern void WEBS_empty_cgi();
extern void WEBS_dump_cgi();

extern int WEBS_add_user(HTTP_CSTR_t username, HTTP_CSTR_t password);
extern int WEBS_del_user(HTTP_CSTR_t username);
extern void WEBS_empty_user();
extern void WEBS_dump_user();

extern SPOOK_SESSION_PROBE_t WEBS_probe(const void* msg, size_t msg_sz);
extern SPOOK_SESSION_LOOP_t WEBS_loop(bool* trigger, int sock, time_t* read_pts);

#ifdef __cplusplus
}
#endif
#endif //WEB_SERVER_H_

