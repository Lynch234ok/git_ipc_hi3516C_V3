
#include "hikvisionv10.h"
#include "web_server.h"
#include "socket_tcp.h"
#include "app_debug.h"


int HIKv10_set_namespace(ezxml_t xml)
{
	if(ezxml_set_attr_d(xml, "xmlns", "http://www.hikvision.com/ver10/XMLSchema") == xml){
		return 0;
	}
	return -1;
}

int HIKv10_set_version(ezxml_t xml)
{
	if(ezxml_set_attr_d(xml, "version", "1.0") == xml){
		return 0;
	}
	return -1;
}

ezxml_t HIKv10_xs_string(const char *name, const char *str)
{
	if(NULL != name && NULL != str){
		ezxml_t xml = ezxml_new_d(name);
		if(NULL != xml){
			ezxml_set_txt_d(xml, str);
			return xml;
		}
	}
	return NULL;
}

ezxml_t HIKv10_xs_integer(const char *name, int content)
{
	if(NULL != name){
		char str[32] = {""};
		if(sprintf(str, "%d", content) < sizeof(str)){
			return HIKv10_xs_string(name, str);
		}
	}
	return NULL;
}

ezxml_t HIKv10_xs_boolean(const char *name, bool content)
{
	if(NULL != name){
		char str[32] = {""};
		if(snprintf(str, sizeof(str), "%s", content ? "true" : "false") < sizeof(str)){
			return HIKv10_xs_string(name, str);
		}
	}
	return NULL;
}


int HIKv10_response(int sock, HTTP_CSTR_t content)
{
	int ret = 0;
	int reponse_max = 0;
	char *response_buf = NULL;
	LP_HTTP_HEAD_FIELD http_header = NULL;
	int header_len = 0;
	int content_len = 0;
	stSOCKET_TCP sock_tcp;
	lpSOCKET_TCP tcp = socket_tcp2_r(sock, &sock_tcp);
	if(NULL != content){
		content_len = strlen(content);
	}
	reponse_max = 4096 + content_len;
	response_buf = alloca(reponse_max);

	http_header = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	http_header->add_tag_text(http_header, "Server", "IE/10.0", true);
	http_header->add_tag_text(http_header, "Cache-Control", "no-cache", true);
	http_header->add_tag_text(http_header, "Content-Type", "application/xml", true);
	http_header->add_tag_text(http_header, "Connection", "close", true);      
	http_header->add_tag_int(http_header, "Content-Length", content_len, true);
	header_len = http_header->to_text(http_header, response_buf, reponse_max);
	http_header->free(http_header);
	http_header = NULL;

	strcat(response_buf, (char *)content);
	APP_TRACE(response_buf);

	//ret = tcp->send(tcp, response_buf, strlen(response_buf), 0);
	ret = send(sock, response_buf, strlen(response_buf), 0);
	if(ret < 0){
		APP_TRACE("NETSDK send data failed, error:%s, %d ", strerror(errno), sock);
	}
	return 0;
}


int HIKVISIOv10_init()
{
	int const n_vin_id = 1;
	int const n_stream_id = 3;

	int i = 0, ii = 0;
	char url_text[1024] = {""};
	
	WEBS_add_cgi("/Video/inputs", HIKVISIONv10_video_inputs, kH_METH_GET);
	

	// 8.9 streaming
	WEBS_add_cgi("/Streaming/channels", HIKVISIONv10_streaming_channels, kH_METH_GET);
	for(i = 0; i < n_vin_id; ++i){
		for(ii = 0; ii < n_stream_id; ++ii){
			int id = ii + 1;
			if(n_vin_id > 1){
				id += (n_vin_id + 1) * 100; // page 76
			}
			// streaming picture
			snprintf(url_text, sizeof(url_text), "/Streaming/channels/%d", id);
			WEBS_add_cgi(url_text, HIKVISIONv10_streaming_channels_id, kH_METH_GET);
			snprintf(url_text, sizeof(url_text), "/Streaming/channels/%d/capabilities", id);
			WEBS_add_cgi(url_text, HIKVISIONv10_streaming_channels_id_capabilities, kH_METH_GET);
			snprintf(url_text, sizeof(url_text), "/Streaming/channels/%d/status", id);
			WEBS_add_cgi(url_text, HIKVISIONv10_streaming_channels_id_status, kH_METH_GET);
			snprintf(url_text, sizeof(url_text), "/Streaming/channels/%d/picture", id);
			WEBS_add_cgi(url_text, HIKVISIONv10_streaming_channels_id_picture, kH_METH_GET);
			snprintf(url_text, sizeof(url_text), "/Streaming/channels/%d/requestKeyFrame", id);
			WEBS_add_cgi(url_text, HIKVISIONv10_streaming_channels_id_requestkeyframe, kH_METH_GET);
		}
	}

	
}

void HIKVISIONV10_destroy()
{
}


