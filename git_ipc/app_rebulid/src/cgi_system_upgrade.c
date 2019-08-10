
#include "httpd.h"
#include "http_common.h"
#include "firmware.h"
#include "generic.h"
#include "app_debug.h"
#include "cgi.h"
#include "sdk/sdk_api.h"
#include "socket_tcp.h"
#include "bsp/keytime.h"
#include "wifi/ja_wifi_seek.h"
#include "app_cloud_rec.h"

static int get_post_boundary(const char* content_type, char* ret_bound)
{
	const char* const boundary_tag = "boundary=";
	void* bound_ptr = strcasestr(content_type, boundary_tag);
	ssize_t bound_len = 0;
	if(bound_ptr){
		bound_ptr += strlen(boundary_tag);
		bound_len = strcspn(bound_ptr, "\r\n");
		if(ret_bound){
			strcpy(ret_bound, "--");
			memcpy(ret_bound + strlen(ret_bound), bound_ptr, bound_len);
			return 0;
		}
	}
	return -1;
}


static bool _cgi_upgrading = false;
int CGI_system_upgrade(HTTPD_SESSION_t* session)
{
	int ret = 0;
	AVal user_agent;
	AVal content_length;
	AVal content_type;
	char* str_user_agent = NULL;
	char* str_content_length = NULL;
	char* str_content_type = NULL;
	ssize_t content_len;

	if(_cgi_upgrading){
		// FIXEME: response a http error
		return -1;
	}
#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
    NK_TFRECORD_Stop(0);
#else
#if defined(TFCARD)
	TFCARD_destroy();
#endif
#endif

#if defined(CLOUD_REC)
    NK_CLOUD_REC_Deinit();
#endif

	if(0 != http_read_header(session->request_buf, "User-Agent", &user_agent)){
		return -1;
	}
	if(0 != http_read_header(session->request_buf, "Content-Length", &content_length)){
		return -1;
	}
	if(0 != http_read_header(session->request_buf, "Content-Type", &content_type)){
		return -1;
	}

	str_user_agent = AVAL_STRDUPA(user_agent);
	str_content_length = AVAL_STRDUPA(content_length);
	str_content_type = AVAL_STRDUPA(content_type);
	_cgi_upgrading = true;

	APP_TRACE("\r\n"
		"User-Agent=\"%s\"\r\n"
		"Content-Length=\"%s\"\r\n"
		"Content-Type=\"%s\"",
		str_user_agent, str_content_length, str_content_type);

	content_len = atoi(str_content_length);
	APP_TRACE("Content size = %d", content_len);

#if defined(WIFI)
	NK_WIFI_adapter_monitor_thread_stop();
#endif

    GLOBAL_cpuDetectDestroy();

	if(content_len > 0){
		char boundary[128];
		void* file_buf;
		void* file_begin = NULL;
		void* file_end = NULL;
		ssize_t file_size = content_len;
		memset(boundary, 0, sizeof(boundary));
		
		// get boundary
		get_post_boundary(str_content_type, boundary);
		APP_TRACE("Post boundary=\"%s\"", boundary);

		// alloc memory to temprarily store file
		file_buf = calloc(content_len+4096, 1);
		APP_ASSERT(file_buf, "Alloc memory %d failed!", content_len);

		int rev_size = 0;
		do{
			ret = recv(session->sock, file_buf+rev_size, content_len, 0);
			if(ret < 0){
				// FIXME: what to do		
				break;
			}
			rev_size += ret;
		}while(rev_size < content_len);
		
		file_begin = memmem(file_buf, file_size, boundary, strlen(boundary));
		if(NULL != file_begin){
			// the file name form found
			file_begin += strlen(boundary);
			file_size = content_len - (file_begin - file_buf);
			APP_TRACE("Seek boundary file size = %d", file_size);
			// continue to seek the file form
			file_begin = memmem(file_begin, file_size, boundary, strlen(boundary));
			if(NULL != file_begin){
				// the file content found
				file_begin += strlen(boundary); // skip the boundary
				file_size = content_len - (file_begin - file_buf);
				APP_TRACE("Seek boundary file size = %d", file_size);
				// continue to seek the CLRF where is the symbol of content begin
				file_begin = memmem(file_begin, file_size, "\r\n\r\n", 4);
				if(NULL != file_begin){
					// success to seek the beginning of the file
					file_begin += 4;
					file_size -= 4;
					APP_TRACE("Seek boundary file size = %d", file_size);
					// then to seek the end of file
					file_end = memmem(file_begin, file_size, boundary, strlen(boundary));
					if(NULL != file_end){
						file_size = file_end - file_begin - 2; // excluding the CLRF at the end of file data
						if(file_size > 0){
							FILE* fid_post = fopen(FIRMWARE_IMPORT_FILE, "wb");
							if(!fid_post){
								// FIXME: what to do
								APP_ASSERT(fid_post, "Open file \"" FIRMWARE_IMPORT_FILE " \" error!");
							}
							ret = fwrite(file_begin, 1, file_size, fid_post);
							if(ret != file_size){
								APP_ASSERT(fid_post, "Write to file \"" FIRMWARE_IMPORT_FILE " \" error %d/%d!", ret, file_size);
							}
							fclose(fid_post);
							
							// free the file buf
							free(file_buf);
							file_buf = NULL;

							// try to upgrade this firmware
							do
							{
								// response the http ok
								const char* const response_msg = "HTTP/1.1 200 OK\r\n"
									"Content-Type: text/html; charset=UTF-8\r\n"
									"Content-length: 2\r\n"
									"Connection: keep-alive\r\n"
									"\r\n"
									"OK";

								ret = send(session->sock, response_msg, strlen(response_msg), 0);
								if(ret < 0){
									APP_TRACE("Response upgrade failed!");
									_cgi_upgrading = false;
									return 0;
								}

								// try to upgrade this firmware
								APP_TRACE("Get file size = %d", file_size);
								APP_TRACE("Start to upgrade!!");

								// prevent keep-alive
								session->keep_alive = 0;

								//set overlay caution
								APP_OVERLAY_create_caution(0);
								
								// start upgrade
								FIRMWARE_upgrade_start(1, false, FIRMWARE_IMPORT_FILE);
								_cgi_upgrading = false;
								return 0;
								
							}while(0);
						}
					}
				}
			}
		}
	}
	_cgi_upgrading = false;
	return -1;
}

static void NK_MEMCPY(char* dst, char* src, int size)
{
	if(src && dst && size>0){
		while(0 < size--){
			*dst++ = *src++;
		}
	}
}

int CGI_system_upgrade_get_rate(HTTPD_SESSION_t* session)
{
	int ret = 0;
	char response_buf[1024];
	char content[32] = {""};;
	const char* const query_string = AVAL_STRDUPA(session->request_line.uri_query_string);
	AVal av_cmd;

	if(0 == http_read_query_string(query_string, "cmd", &av_cmd)){
		if(AVSTRCASEMATCH(&av_cmd, "upgrade_rate")){
			int upgrade_rate = FIRMWARE_upgrade_get_rate();
			APP_TRACE("FIRMWARE_upgrade_get_rate=%d", upgrade_rate);
			sprintf(content, "%d", upgrade_rate);
			
		}else if(AVSTRCASEMATCH(&av_cmd, "http_import_rate")){
			int import_rate = FIRMWARE_import_get_rate();
			APP_TRACE("FIRMWARE_import_get_rate=%d", import_rate);
			sprintf(content, "%d", import_rate);
			
		}else if(AVSTRCASEMATCH(&av_cmd, "http_import")){
			AVal av_ip, av_port;
			strcpy(content, "1");
			
			if(0 == http_read_query_string(query_string, "ip", &av_ip)
				&& 0 == http_read_query_string(query_string, "port", &av_port)){

				const char* str_ip = AVAL_STRDUPA(av_ip);
				uint16_t n_port = atoi(AVAL_STRDUPA(av_port));
				
				if(n_port > 0 && n_port < 65535 && strlen(str_ip) > 0)	{
					if(0 == FIRMWARE_import_from_http(str_ip, n_port)){
						FIRMWARE_upgrade_start(1, false, FIRMWARE_IMPORT_FILE);
						strcpy(content, "0");
					}
				}
			}
		}
		
	}

	ret = snprintf(response_buf, ARRAY_ITEM(response_buf),
		"HTTP/1.1 200 OK"CRLF
		"Content-Type: text/html; charset=UTF-8"CRLF
		"Content-Length: %d"CRLF
		"Connection: keep-alive"CRLF
		CRLF
		"%s",
		strlen(content), content);
	return send(session->sock, response_buf, strlen(response_buf), 0);
}


int CGI_system_upgrade2(LP_HTTP_CONTEXT context)
{
	int ret = 0;
	//AVal user_agent;
	//AVal content_length;
	//AVal content_type;
	const char *user_agent = NULL;
	const char *content_length = NULL;
	const char *content_type = NULL;
	LP_HTTP_HEAD_FIELD const http = context->request_header;
	void *upfile_mem = NULL;
	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	ssize_t content_len;
	ssize_t offset = 0;
	const char *cmd = NULL;
	bool force_upgrade_flag = false, downgrade_flag = false;

	if(_cgi_upgrading){
		// FIXEME: response a http error
		return -1;
	}

	user_agent = http->read_tag(http, "User-Agent");
	if(!user_agent){
		return -1;
	}
	content_length = http->read_tag(http, "Content-Length");
	if(!content_length){
		return -1;
	}
	content_type = http->read_tag(http, "Content-Type");
	if(!content_type){
		return -1;
	}

	pHead= HTTP_UTIL_parse_query_as_para(context->request_header->query);
	if(NULL != pHead){
		cmd = pHead->read(pHead, "oem_force");
		if(NULL != cmd){
			if(!strcmp(cmd, "true")){
				force_upgrade_flag = true;
			}
		}else{

		}
		cmd = pHead->read(pHead, "downgrade");
		if(NULL != cmd){
			if(!strcmp(cmd, "true")){
				downgrade_flag = true;
			}
		}else{

		}
		pHead->free(pHead);
		pHead = NULL;
	}

	_cgi_upgrading = true;

	APP_TRACE("\r\n"
		"User-Agent=\"%s\"\r\n"
		"Content-Length=\"%s\"\r\n"
		"Content-Type=\"%s\"",
		user_agent, content_length, content_type);

	content_len = atoi(content_length);	

#if defined(HI3518E)
	//FIX lack of memery in HI3518E inception
	uint32_t chip_id;
	if(!sdk_sys){
		APP_TRACE("sdk_sys not init!");
		return -1;
	}
	sdk_sys->get_chip_id(&chip_id);
	if(SDK_HI3518E_V100 == chip_id){
		FIRMWARE_import_release_memery();
	}
#endif

	GLOBAL_before_upgrade_destroy();

	printf("%s() line:%d\n", __FUNCTION__, __LINE__);
	
	upfile_mem = FIRMWARE_upgrade_env_prepare(&content_len);
	printf("%s() line:%d\n", __FUNCTION__, __LINE__);
	if(NULL == upfile_mem)
	{
		printf("%s() error line:%d\n", __FUNCTION__, __LINE__);
		APP_TRACE("FIRMWARE_upgrade_env_prepare error!");
        GLOBAL_reboot_system();
	}
	memset(upfile_mem, 0, content_len);
	
	ssize_t ret_rev;
	ST_SOCKET_TCP sock_tcp;
	lpSOCKET_TCP tcp = socket_tcp2_r(context->sock, &sock_tcp);

	printf("%s() line:%d\n", __FUNCTION__, __LINE__);

	if (context->request_content_len > 0 && !context->request_content) {

		tcp->set_recv_timeout(tcp, 120, 0);
		if ((ret_rev = tcp->recv2(tcp, upfile_mem, context->request_content_len, 0))
						!= context->request_content_len) {
			//FIXME
			FIRMWARE_import_recover_memery();
			APP_TRACE("UPGRADE RECV FALSE!");
            GLOBAL_reboot_system();
		}
		APP_TRACE("request =%d recv = %d",context->request_content_len,ret_rev);
	}
	
	printf("%s() line:%d\n", __FUNCTION__, __LINE__);
	APP_TRACE("Content size = %d", content_len);

#if defined(WIFI)
	NK_WIFI_adapter_monitor_thread_stop();//stop wifi monitor detection
#endif
	printf("%s() line:%d\n", __FUNCTION__, __LINE__);

	if(content_len > 0){
		char boundary[128];
		void* file_buf = NULL;
		void* file_begin = NULL;
		void* file_end = NULL;
		ssize_t file_size = content_len;
		memset(boundary, 0, sizeof(boundary));
		
		// get boundary
		get_post_boundary(content_type, boundary);
		APP_TRACE("Post boundary=\"%s\"", boundary);

		// pick file out of content
//		file_buf = context->request_content;		
		file_buf = upfile_mem;
		
		printf("%s() line:%d\n", __FUNCTION__, __LINE__);
		file_begin = memmem(file_buf, file_size, boundary, strlen(boundary));
		if(NULL != file_begin){
			// the file name form found
			file_begin += strlen(boundary);
			file_size = content_len - (file_begin - file_buf);
			APP_TRACE("Seek boundary file size = %d", file_size);
			// continue to seek the file form
			file_begin = memmem(file_begin, file_size, boundary, strlen(boundary));
			if(NULL != file_begin){
				// the file content found
				file_begin += strlen(boundary); // skip the boundary
				file_size = content_len - (file_begin - file_buf);
				APP_TRACE("Seek boundary file size = %d", file_size);
				// continue to seek the CLRF where is the symbol of content begin
				file_begin = memmem(file_begin, file_size, "\r\n\r\n", 4);
				if(NULL != file_begin){
					// success to seek the beginning of the file
					file_begin += 4;
					file_size -= 4;
					APP_TRACE("Seek boundary file size = %d", file_size);
					// then to seek the end of file
					file_end = memmem(file_begin, file_size, boundary, strlen(boundary));
					if(NULL != file_end){
						file_size = file_end - file_begin - 2; // excluding the CLRF at the end of file data
						if(file_size > 0){							
						//	memcpy(upfile_mem, file_begin, file_size);
							// try to upgrade this firmware
							do
							{
								// response the http ok
								const char* const response_msg = "HTTP/1.1 200 OK\r\n"
									"Content-Type: text/html; charset=UTF-8\r\n"
									"Content-length: 2\r\n"
									"Connection: keep-alive\r\n"
									"\r\n"
									"OK";

								ret = send(context->sock, response_msg, strlen(response_msg), 0);
								if(ret < 0){
									APP_TRACE("Response upgrade failed!");
									_cgi_upgrading = false;
									FIRMWARE_import_recover_memery();
									return 0;
								}

								// try to upgrade this firmware
								APP_TRACE("Get file size = %d", file_size);
								APP_TRACE("Start to upgrade!!");

								// prevent keep-alive
								context->keep_alive = 0;


								// start upgrade
							//	FIRMWARE_upgrade_start2(1, upfile_mem, file_size);
								printf("%s() upfile_mem = %p, file_begin = %p\n", __FUNCTION__, upfile_mem, file_begin);
								NK_MEMCPY((char*)upfile_mem, (char*)file_begin, file_size);
								offset = (ssize_t)((unsigned int)file_begin - (unsigned int)upfile_mem);
								offset = 0;
								FIRMWARE_upgrade_start2(force_upgrade_flag, downgrade_flag, 1, upfile_mem+offset, file_size);
								_cgi_upgrading = false;
								return 0;
								
							}while(0);
						}
					}
				}
			}
		}
	}
	FIRMWARE_import_recover_memery();
	_cgi_upgrading = false;	
    GLOBAL_reboot_system();
	return -1;
}

int CGI_system_upgrade_get_rate2(LP_HTTP_CONTEXT context)
{
	int ret = 0;
	char response_buf[1024];
	char content[32] = {""};;
	AVal av_cmd;

	LP_HTTP_QUERY_PARA_LIST query = HTTP_UTIL_parse_query_as_para(context->request_header->query);
	if(query){
		const char *cmd = query->read(query, "cmd");
		if(NULL != cmd){
			if(0 == strcasecmp("upgrade_rate", cmd)){
				int upgrade_rate = FIRMWARE_upgrade_get_rate();
				APP_TRACE("FIRMWARE_upgrade_get_rate=%d", upgrade_rate);
				sprintf(content, "%d", upgrade_rate);
			}else if(0 == strcasecmp("http_import_rate", cmd)){
				int import_rate = FIRMWARE_import_get_rate();
				APP_TRACE("FIRMWARE_import_get_rate=%d", import_rate);
				sprintf(content, "%d", import_rate);
			}else if(0 == strcasecmp("http_import", cmd)){
				const char *ip = query->read(query, "ip");
				const char *port = query->read(query, "port");

				if(NULL != ip && NULL != port){
					uint16_t n_port = atoi(port);
					
					if(n_port > 0 && n_port < 60000 && strlen(ip) > 0)	{
						if(0 == FIRMWARE_import_from_http(ip, n_port)){
							FIRMWARE_upgrade_start(1, false, FIRMWARE_IMPORT_FILE);
							strcpy(content, "0");
						}
					}
				}
			}
			
		}
	}
	ret = snprintf(response_buf, ARRAY_ITEM(response_buf),
		"HTTP/1.1 200 OK"CRLF
		"Content-Type: text/html; charset=UTF-8"CRLF
		"Content-Length: %d"CRLF
		"Connection: keep-alive"CRLF
		CRLF
		"%s",
		strlen(content), content);
	return send(context->sock, response_buf, strlen(response_buf), 0);
}

