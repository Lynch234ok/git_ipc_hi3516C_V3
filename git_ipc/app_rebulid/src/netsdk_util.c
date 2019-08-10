
#include "netsdk_util.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "http_util.h"
#include "socket_tcp.h"
#include "app_debug.h"

int NETSDK_response(int sock, HTTP_CSTR_t content)
{
	int ret = 0;
	int response_max = 4096;
	char *response_buf = NULL;
	LP_HTTP_HEAD_FIELD http_header = NULL;
	int header_len = 0;
	int content_len = 0;
	ST_SOCKET_TCP sock_tcp;
	LP_SOCKET_TCP tcp = socket_tcp2_r(sock, &sock_tcp);
	
	if(NULL != content){
		content_len = strlen(content);
		response_max += content_len;
	}

	response_buf = alloca(response_max);
	http_header = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	http_header->add_tag_server(http_header, "IE/10.0");
	http_header->add_tag_date(http_header, 0);
	http_header->add_tag_text(http_header, "Cache-Control", "no-cache", true);
	//http_header->add_tag_text(http_header, "Content-Type", "text/json", true);
	//http_header->add_tag_text(http_header, "Content-Type", "application/x-javascript", true);
	http_header->add_tag_text(http_header, "Content-Type", "application/json", true);
	//http_header->add_tag_text(http_header, "Content-Type", "text/html", true);
	//http_header->add_tag_text(http_header, "Content-Type", "text/xml", true);
	http_header->add_tag_text(http_header, "Connection", "close", true);      
	http_header->add_tag_int(http_header, "Content-Length", content_len, true);
	header_len = http_header->to_text(http_header, response_buf, response_max);
	http_header->free(http_header);
	http_header = NULL;

	strcat(response_buf, (char *)content);
	//APP_TRACE(response_buf);

	ret = tcp->send2(tcp, response_buf, strlen(response_buf), 0);
	if(ret < 0){
		APP_TRACE("NETSDK send data failed, error:%s, %d ", strerror(errno), sock);
		return -1;
	}
	return 0;
}

int NETSDK_response_file(int sock, HTTP_CSTR_t mime, const char *file_path)
{
	FILE *fid = NULL;
	fid = fopen(file_path, "rb");
	if(NULL != fid){
		struct stat file_stat;
		int ret = 0;
		int response_max = 4096;
		char *response_buf = NULL;
		LP_HTTP_HEAD_FIELD http_header = NULL;
		int header_len = 0;
		int content_len = 0;
		stSOCKET_TCP sock_tcp;
		lpSOCKET_TCP tcp = socket_tcp2_r(sock, &sock_tcp);
		int read_size = 0;
		char *file_offset = NULL;

		fstat(fileno(fid), &file_stat);
		content_len = file_stat.st_size;
		response_max += content_len; // including the file size
		
		response_buf = alloca(response_max);
		http_header = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
		http_header->add_tag_text(http_header, "Server", "IE/10.0", true);
		http_header->add_tag_text(http_header, "Cache-Control", "no-cache", true);
		http_header->add_tag_text(http_header, "Content-Type", mime, true);
		http_header->add_tag_text(http_header, "Connection", "close", true);      
		http_header->add_tag_int(http_header, "Content-Length", content_len, true);
		header_len = http_header->to_text(http_header, response_buf, response_max);
		http_header->free(http_header);
		http_header = NULL;

		file_offset = response_buf + header_len;
		while((read_size = fread(file_offset, 1, 1024, fid)) > 0){
			if(read_size < 0){
				fclose(fid);
				return -1;
			}
			// next block
			file_offset += read_size;
		}

		ret = tcp->send2(tcp, response_buf, file_offset - response_buf, 0);
		if(ret < 0){
			APP_TRACE("NETSDK send data failed, error:%s, %d ", strerror(errno), sock);
			return -1;
		}
		return 0;
	}
	return -1;
}


int NETSDK_return_ok(int sock, HTTP_CSTR_t mesg)
{
	char content[1024] = {"var result=\"ok\"" kCRLF};
	if(NULL != mesg){
		snprintf(content + strlen(content), sizeof(content) - strlen(content), "var mesg=\"%s\"" kCRLF, mesg); 
	}
	return NETSDK_response(sock, content);
}

int NETSDK_return_failed(int sock, HTTP_CSTR_t mesg)
{
	char content[1024] = {"var result=\"failed\"" kCRLF};
	if(NULL != mesg){
		snprintf(content + strlen(content), sizeof(content) - strlen(content), "var mesg=\"%s\"" kCRLF, mesg); 
	}else{
		strcat(content, "var mesg=\"Unkwon error!\"" kCRLF);
	}
	return NETSDK_response(sock, content);
}

#define JS_ENDL kCRLF

#define NETSDKV10_JSON_ADD_VAR(__buf, __buf_size, __format, __var_name, __var_val, __comment) \
	do{\
		int const _buf_len = strlen(__buf);\
		snprintf(__buf + _buf_len, __buf_size - _buf_len,\
			__format, __var_name, __var_val, !__comment ? "" : "//", !__comment ? "" : __comment);\
	}while(0)

void NETSDK_json_add_text(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, HTTP_CSTR_t var_val, HTTP_CSTR_t comment)
{
	NETSDKV10_JSON_ADD_VAR(buf, buf_size, "var %s=\"%s\"; %s %s"JS_ENDL, var_name, var_val, comment);
}

void NETSDK_json_add_dec(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, int var_val, HTTP_CSTR_t comment)
{
	NETSDKV10_JSON_ADD_VAR(buf, buf_size, "var %s=\"%d\"; %s %s"JS_ENDL, var_name, var_val, comment);
}

void NETSDK_json_add_hex(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, unsigned int var_val, HTTP_CSTR_t comment)
{
	NETSDKV10_JSON_ADD_VAR(buf, buf_size, "var %s=\"%08x\"; %s %s"JS_ENDL, var_name, var_val, comment);
}

void NETSDK_json_add_float(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, float var_val, HTTP_CSTR_t comment)
{
	NETSDKV10_JSON_ADD_VAR(buf, buf_size, "var %s=\"%.5f\"; %s %s"JS_ENDL, var_name, var_val, comment);
}

void NETSDK_json_add_percent(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, float var_val, HTTP_CSTR_t comment)
{
	NETSDKV10_JSON_ADD_VAR(buf, buf_size, "var %s=\"%.3f%%\"; %s %s"JS_ENDL, var_name, var_val * 100.0, comment);
}

void NETSDK_json_add_boolean(HTTP_STR_t buf, size_t buf_size, HTTP_CSTR_t var_name, bool var_val, HTTP_CSTR_t comment)
{
	NETSDKV10_JSON_ADD_VAR(buf, buf_size, "var %s=\"%s\"; %s %s"JS_ENDL, var_name, var_val ? "yes" : "no", comment);
}

// netsdk v1.0 read para
HTTP_CSTR_t NETSDK_read_text(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, HTTP_CSTR_t def_val)
{
	if(NULL != list){
		HTTP_CSTR_t const para_val = list->read(list, var_name);
		if(NULL != para_val){
			return para_val;
		}
	}
	return def_val;
}

int NETSDK_read_dec(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, int ref_max, int def_val)
{
	if(NULL != list){
		HTTP_CSTR_t const para_val = list->read(list, var_name);
		if(NULL != para_val){
			if(0 != ref_max && NULL != strrchr(para_val, '%')){
				return (int)((float)ref_max * atof(para_val) / 100.0);
			}
			return atoi(para_val);
		}
	}
	return def_val;
}

unsigned int NETSDK_read_hex(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, unsigned int def_val)
{
	if(NULL != list){
		HTTP_CSTR_t para_val = list->read(list, var_name);
		unsigned int val = 0;
		if(NULL != para_val){
			//list->dump(list);
			if(1 == sscanf(para_val, "%08x", &val)){
				return val;
			}
		}
	}
	return def_val;
}


float NETSDK_read_float(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, float def_val)
{
	if(NULL != list){
		HTTP_CSTR_t const para_val = list->read(list, var_name);
		if(NULL != para_val){
			return atof(para_val);
		}
	}
	return def_val;
}

bool NETSDK_read_boolean(LP_HTTP_QUERY_PARA_LIST const list, HTTP_CSTR_t var_name, bool def_val)
{
	if(NULL != list){
		HTTP_CSTR_t const para_val = list->read(list, var_name);
		if(NULL != para_val){
			return ('y' == para_val[0]
				|| 'Y' == para_val[0]
				|| '1' == para_val[0]) ? true : false;
		}
	}
	return def_val;
}

