#ifndef HICHIP_HTTP_CGI_H_
#define HICHIP_HTTP_CGI_H_

#include "httpd.h"

void HICHIP_http_init();
void HICHIP_http_destroy();
int HICHIP_http_cgi(HTTPD_SESSION_t* session);

#include "web_server.h"
extern int HICHIP_live_stream(LP_HTTP_CONTEXT session);
extern int HICHIP_set_md_status(bool flag);
extern bool HICHIP_get_md_status();


#endif

