


#include <NkUtils/types.h>

#include <string.h>
#ifdef _WIN32
//#include <unistd.h> //linux
#include <io.h>  //windows
#include <process.h> //windows 
#include <windows.h>
#else
#include <unistd.h> 
#endif
#include <stdio.h>
#include <pthread.h>
 
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <stdlib.h>
#ifdef _WIN32
//#include <unistd.h> //linux
#include <io.h>  //windows
#include <process.h> //windows 
#include <windows.h>
#else
#include <unistd.h> 
#endif
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#include "http_util.h"

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kHTTP_SERV_KEEP_ALIVE_DURATION (60)

#define kH_METH_OPTIONS (1<<0)
#define kH_METH_GET (1<<1)
#define kH_METH_HEAD (1<<2)
#define kH_METH_POST (1<<3)
#define kH_METH_PUT (1<<4)
#define kH_METH_DELETE (1<<5)
#define kH_METH_CONNECT (1<<6)
#define kH_METH_EXTENSION (1<<7)
#define kH_METH_ALL (kH_METH_OPTIONS|kH_METH_GET|kH_METH_HEAD|kH_METH_PUT|kH_METH_DELETE|kH_METH_CONNECT|kH_METH_EXTENSION)

#define H_IS_OPTIONS(__s) (!strncasecmp(__s,"OPTIONS",6))
#define H_IS_GET(__s) (!strncasecmp(__s,"GET",3))
#define H_IS_HEAD(__s) (!strncasecmp(__s,"HEAD",4))
#define H_IS_POST(__s) (!strncasecmp(__s,"POST",4))
#define H_IS_PUT(__s) (!strncasecmp(__s,"PUT",3))
#define H_IS_DELETE(__s) (!strncasecmp(__s,"DELETE",6))
#define H_IS_CONNECT(__s) (!strncasecmp(__s,"CONNECT",7))

#define HTTP_IS_OPTIONS(__ctx) (kH_METH_OPTIONS == __ctx->request_method)
#define HTTP_IS_GET(__ctx) (kH_METH_GET == __ctx->request_method)
#define HTTP_IS_HEAD(__ctx) (kH_METH_HEAD == __ctx->request_method)
#define HTTP_IS_POST(__ctx) (kH_METH_POST == __ctx->request_method)
#define HTTP_IS_PUT(__ctx) (kH_METH_PUT == __ctx->request_method)
#define HTTP_IS_DELETE(__ctx) (kH_METH_DELETE == __ctx->request_method)
#define HTTP_IS_CONNECT(__ctx) (kH_METH_CONNECT == __ctx->request_method)


typedef struct HTTP_CONTEXT {
	int sock;
	NK_Boolean *trigger;
	int keep_alive; // keep alive duration, if 0 means close
	// request info
	LP_HTTP_HEAD_FIELD request_header;
	size_t request_content_len;
	void *request_content;

	int request_method;
	int accept_method;
}ST_HTTP_CONTEXT, *LP_HTTP_CONTEXT;

typedef int (*fHTTP_SERV_CGI_SERVICE)(LP_HTTP_CONTEXT context);

typedef struct HTTP_SERV {

	// set the resource directory
	int (*set_resource_dir)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t directory);

	// start / stop the server
	int (*start_listener)(struct HTTP_SERV *const http_serv);
	int (*stop_listener)(struct HTTP_SERV *const http_serv);

	// session loop
	int (*session_loop)(struct HTTP_SERV *const http_serv, NK_Boolean *session_trigger, int session_sock);

	// user operation
	int (*add_user)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t username, HTTP_CSTR_t password);
	int (*del_user)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t username);
	int (*empty_user)(struct HTTP_SERV *const http_serv);
	void (*dump_user)(struct HTTP_SERV *const http_serv);

	// cgi operation
	int (*add_cgi)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t uri, fHTTP_SERV_CGI_SERVICE service, int accept_method);
	int (*del_cgi)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t uri);
	int (*empty_cgi)(struct HTTP_SERV *const http_serv);
	void (*dump_cgi)(struct HTTP_SERV *const http_serv);
	
}ST_HTTP_SERV, *LP_HTTP_SERV;

extern LP_HTTP_SERV HTTP_SERV_create();
extern void HTTP_SERV_release(LP_HTTP_SERV http_serv);
extern NK_Boolean HTTP_SERV_probe(HTTP_CSTR_t request_packet, size_t packet_len);

#ifdef __cplusplus
}
#endif
#endif //HTTP_SERVER_H_

