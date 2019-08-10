
#include "hichipv10.h"
#include "netsdk.h"
#include "app_debug.h"


int HICHIPV10_response_text(int sock, HTTP_CSTR_t text)
{
	const char response_format[] = {
		"HTTP/1.1 200 OK" kCRLF
		"Server:IE/10.0" kCRLF
		"Cache-Control:no-cache" kCRLF
		"Connection:keep-alive" kCRLF
		"Content-Type:text/html" kCRLF
		"Content-Length:%d"kCRLF
		kCRLF
		"%s" // <-- text
	};
	
	int ret = 0;
	int const text_size = (NULL == text) ? 0 : strlen(text);
	int const response_max = sizeof(response_format) + 32 + text_size;
	char *response_buf = alloca(response_max);

	sprintf(response_buf, response_format, text_size, (NULL == text) ? "" : text);
	ret = send(sock, response_buf, strlen(response_buf), 0);
	if(ret < 0){
		APP_TRACE("NETSDK send data failed, error:%s", strerror(errno));
	}
	return ret;
}

int HICHIPV10_get_identify(LP_HTTP_CONTEXT context)
{
	char content[4096] = {""};
	NETSDK_json_add_text(content, sizeof(content), "productid", "C1F0S9Z3N0P0L0");
	NETSDK_json_add_text(content, sizeof(content), "vendorid", "JUAN");
	HICHIPV10_response_text(context->sock, content);
	return 0;
}

int HICHIPV10_get_video_displayattr(LP_HTTP_CONTEXT context)
{
	char content[4096] = {""};
	NETSDK_json_add_dec(content, sizeof(content), "powerfreq", 50);
	HICHIPV10_response_text(context->sock, content);
	return 0;
}


#include "httpd.h"
int HICHIPV10_compat(LP_HTTP_CONTEXT context)
{
	int ret = 0;
	HTTPD_SESSION_t session;
	HTTP_REQUEST_LINE_t *request_line = &session.request_line;
	// re-make a header to compat the old version
	memset(&session, 0, sizeof(session));

	// the request header
	session.request_sz = context->request_header->to_text(context->request_header, session.request_buf, sizeof(session.request_buf));
	// socket and context
	session.sock = context->sock;
	session.trigger = context->trigger;
	session.keep_alive = context->keep_alive;
	session.auth_counter = 0;
	// request line
	request_line->method.av_val = context->request_header->method;
	request_line->method.av_len = strlen(request_line->method.av_val);
	request_line->uri.av_val = context->request_header->uri; // not use in old version
	request_line->uri.av_len = strlen(request_line->uri.av_val); // not use in old version
	request_line->uri_host.av_val = ""; // not use in old version
	request_line->uri_host.av_len = 0; // not use in old version
	request_line->uri_hostname.av_val = ""; // not use in old version
	request_line->uri_hostname.av_len = 0; // not use in old version
	request_line->uri_suffix.av_val = context->request_header->uri;
	request_line->uri_suffix.av_len = strlen(request_line->uri_suffix.av_val);
	request_line->uri_suffix_extname.av_val = ""; // not use in old version
	request_line->uri_suffix_extname.av_len = 0; // not use in old version
	request_line->uri_query_string.av_val = context->request_header->query;
	request_line->uri_query_string.av_len = strlen(request_line->uri_query_string.av_val);
	request_line->version.av_val = context->request_header->version;
	request_line->version.av_len = strlen(request_line->version.av_val);

	ret = HICHIP_http_cgi(&session);
	return ret;
}

