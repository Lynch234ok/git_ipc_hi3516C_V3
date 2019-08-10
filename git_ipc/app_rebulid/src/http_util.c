
/*
 *	IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *	By downloading, copying, installing or using the software you agree to this license.
 *	If you do not agree to this license, do not download, install,
 *	Copy or use the software.
 *
 *	Copyright (C) 2012, JUAN, Co, All Rights Reserved.
 *
 *	Project Name: HTTP utilities
 *	File Name:
 *
 *	Writed by Frank Law at 2013 - 06 - 03 JUAN,Guangzhou,Guangdong,China
 *
 */

#include "http_util.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_tcp.h"


#define HTTP_TAGS_SORT_BY_NAME

static void text_to_upper(HTTP_STR_t text)
{
	char *ch = text;
	while('\0' != *ch){
		int ch_dw = (int)(*ch);
		ch_dw = toupper(ch_dw);
		*ch++ = (char)(ch_dw);
	}
}

static HTTP_STR_t text_trim(HTTP_STR_t text)
{
	char *text_offset = text + strlen(text);
	// trim the end space
	while(' ' == *text_offset){
		*text_offset-- = '\0';
	}
	// trim the head space
	text_offset = text;
	while(' ' == *text_offset){
		++text_offset;
	}
	return text_offset;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

static LP_HTTP_TAG tag_new(HTTP_CSTR_t tag_name, HTTP_CSTR_t tag_val)
{
	if(NULL != tag_name && NULL != tag_val){
		LP_HTTP_TAG tag = calloc(sizeof(ST_HTTP_TAG), 1);
		tag->name = strdup(text_trim(strdupa(tag_name)));
		tag->val = strdup(text_trim(strdupa(tag_val)));
		tag->next = NULL;
		return tag;
	}
	return NULL;
}

static void tag_free(LP_HTTP_TAG tag)
{
	free(tag->name);
	free(tag->val);
	free(tag);
}

static int tag_list_insert(LP_HTTP_TAG *tag_list, HTTP_CSTR_t tag_name, HTTP_CSTR_t tag_val, bool over_write)
{
	LP_HTTP_TAG new_tag = tag_new(tag_name, tag_val);
	if(NULL != new_tag){
		LP_HTTP_TAG prev_tag = NULL;
		LP_HTTP_TAG tag = *tag_list;
#ifdef HTTP_TAGS_SORT_BY_NAME
		if(NULL == *tag_list){
			// no tags before
			// add a new tag for http header
			*tag_list = new_tag;
		}else{
			// there is a read tag in header
			// insert to the htad
			while(NULL != tag){
				if(over_write
					&& strlen(tag->name) == strlen(new_tag->name)
					&& 0 == strcasecmp(tag->name, new_tag->name)){
					// check reduplicate
					// update the value
					free(tag->val);
					tag->val = strdup(new_tag->val);
					tag_free(new_tag);
					break;
				}else if(strcasecmp(tag->name, new_tag->name) >= 0){
					// insert here
					if(NULL == prev_tag){
						*tag_list = new_tag;
					}else{
						prev_tag->next = new_tag;
					}
					new_tag->next = tag;
					break;
				}
				prev_tag = tag;
				tag = tag->next; // to check the next tag
			}
			// insert to the tail
			if(NULL == tag){
				prev_tag->next = new_tag;
			}

		}
#else
		// insert at head
		tag->next = *tag_list;
		*tag_list = tag;
#endif
		return 0;
	}
	return -1;
}

static int tag_list_remove(LP_HTTP_TAG *tag_list, HTTP_CSTR_t tag_name)
{
	LP_HTTP_TAG tag = *tag_list;
    LP_HTTP_TAG prev_tag = NULL;
	
    while(NULL != tag){
		if(strlen(tag->name) == strlen(tag_name)
			&& 0 == strcasecmp(tag->name, tag_name)){
			// this tag to remove
			//printf("remove \"%s=%s\"\r\n", tag->name, tag->val);
			if(!prev_tag){
				// this is the first tag in list
				*tag_list = tag->next; // the head of the list mark the next tag location
				tag_free(tag); // then free the memory of tag
			}else{
				prev_tag->next = tag->next; // <- backup the next tag
				tag_free(tag); // then free the memory of tag
			}
			// success to remove a tag from list
			return 0;
		}
		// to find the next
		prev_tag = tag;
		tag = tag->next;
	}
	// failed to remove a tag
	return -1;
}

static HTTP_CSTR_t tag_list_lookup(LP_HTTP_TAG *tag_list, HTTP_CSTR_t tag_name)
{
	LP_HTTP_TAG tag = *tag_list;
	while(NULL != tag){
		if(strlen(tag->name) == strlen(tag_name)
			&& 0 == strcasecmp(tag->name, tag_name)){
			// bingo!!
			return tag->val;
		}
		tag = tag->next;
	}
	return NULL;
}

static void tag_list_free(LP_HTTP_TAG *tag_list)
{
	while(NULL != *tag_list){
		tag_list_remove(tag_list, (*tag_list)->name);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


static HTTP_CSTR_t query_para_list_read(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_CSTR_t name)
{
	if(NULL != name){
		return tag_list_lookup(&thiz->paras, name);
	}
	return NULL;
}

static int query_para_list_add(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_STR_t name, HTTP_STR_t val)
{
	return tag_list_insert(&thiz->paras, name, val, true);
}

static int query_para_list_del(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_STR_t name)
{
	if(NULL != name){
		return tag_list_remove(&thiz->paras, name);
	}
	return -1;
}
	
static void query_para_list_dump(struct HTTP_QUERY_PARA_LIST *const thiz)
{
	char text[8192] = {""};
	if(thiz->to_text(thiz, text, sizeof(text))){
		printf("%s\r\n", text);
	}
}

static ssize_t query_para_list_to_text(struct HTTP_QUERY_PARA_LIST *const thiz, char *text, size_t text_size)
{
	int ret = 0;
	char *text_offset = text;
	size_t text_left = text_size - 1;
	LP_HTTP_TAG para = thiz->paras;
	while(NULL != para){
		ret = snprintf(text_offset, text_left,
			"%s=%s%s", para->name, para->val, kQUERY_DELIMIT);
		//printf("Para %s=%s\n", para->name, para->val);
		// increase
		text_offset += ret;
		text_left -= ret;
		// to next para
		para = para->next;
	}
	// remove the last 
	text[strlen(text) - strlen(kQUERY_DELIMIT)] = '\0';
	return strlen(text);
}

static void query_para_list_free(struct HTTP_QUERY_PARA_LIST *const thiz)
{
	// free all the paras
	tag_list_free(&thiz->paras);
	// free myself
	free(thiz);
}

LP_HTTP_QUERY_PARA_LIST HTTP_UTIL_parse_query_as_para(HTTP_STR_t query_text)
{
	LP_HTTP_QUERY_PARA_LIST para_list = NULL;
	para_list = calloc(sizeof(ST_HTTP_QUERY_PARA_LIST), 1);
	para_list->paras = NULL;
	para_list->read = query_para_list_read;
	para_list->add = query_para_list_add;
	para_list->del = query_para_list_del;
	para_list->dump = query_para_list_dump;
	para_list->to_text = query_para_list_to_text;
	para_list->free = query_para_list_free;
	
	if(NULL != query_text){
		char *query_string = strdupa(query_text);
		char *token = NULL, *para_exp = NULL;
		para_exp = strtok_r(query_string, kQUERY_DELIMIT, &token);
		while(NULL != para_exp){
			char *const locate_equ = strchr(para_exp, '=');
			// add this para to the list
			if(NULL != locate_equ){
				char *const para_name = strndupa(para_exp, locate_equ - para_exp);
				char *const para_val = strdupa(locate_equ + 1);
				para_list->add(para_list, para_name, para_val);
			}else{
				para_list->add(para_list, para_exp, "");
			}
			
			// to find next expression
			para_exp = strtok_r(NULL, kQUERY_DELIMIT, &token);
		}
	}
	return para_list;
}
//
//// header tags operations
//static int http_add_tag_text(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t tag_name, HTTP_CSTR_t tag_val, bool over_write)
//{
//	return tag_list_insert(&thiz->tags, tag_name, tag_val, over_write);
//}
//
//static int http_add_tag_int(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t tag_name, int tag_val, bool over_write)
//{
//	if(NULL != tag_name){
//		char str_val[32] = {""};
//		sprintf(str_val, "%d", tag_val);
//		return thiz->add_tag_text(thiz, tag_name, str_val, over_write);
//	}
//	return -1;
//}
//
//static int http_del_tag(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t tag_name)
//{
//	if(NULL != tag_name){
//		return tag_list_remove(&thiz->tags, tag_name);
//	}
//	return -1;
//}
//
//// add & delete tag in special
//static int http_add_tag_server(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t server_name)
//{
//	return thiz->add_tag_text(thiz, "Server", server_name, true);
//}
//
//static int http_del_tag_server(struct HTTP_HEAD_FIELD *const thiz)
//{
//	return thiz->del_tag(thiz, "Server");
//}
//
//static int http_add_tag_date(struct HTTP_HEAD_FIELD *const thiz, time_t t)
//{
//	char date_buf[128] = {""};
//	return thiz->add_tag_text(thiz, "Date", HTTP_UTIL_date_header(date_buf, sizeof(date_buf), t), true);
//}
//
//static int http_del_tag_date(struct HTTP_HEAD_FIELD *const thiz)
//{
//	return thiz->del_tag(thiz, "Date");
//}
//
//
//static HTTP_CSTR_t http_read_tag(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t tag_name)
//{
//	if(NULL != thiz->tags){
//		return tag_list_lookup(&thiz->tags, tag_name);
//	}
//	return NULL;
//}
//
//static void http_dump(struct HTTP_HEAD_FIELD *const thiz)
//{
//	char buf[2048] = {""};
//	thiz->to_text(thiz, buf, sizeof(buf));
//	//printf(buf);
//}
//
//static ssize_t http_to_text(struct HTTP_HEAD_FIELD *const thiz, char *text, size_t stack_len)
//{
//	int ret = 0;
//	LP_HTTP_TAG tag = thiz->tags;
//	char *text_offset = text;
//	size_t text_left = stack_len;
//
//	if(!text){
//		return -1;
//	}
//	if(thiz->request_flag){
//		bool const query_valid = (NULL != thiz->query && strlen(thiz->query) >  0);
//		// request mode
//		if(query_valid){
//			ret = snprintf(text_offset, text_left, "%s %s?%s %s/%s" kCRLF,
//				thiz->method,
//				thiz->uri, query_valid ? thiz->query : "",
//				thiz->protocol, thiz->version);
//		}else{
//			ret = snprintf(text_offset, text_left, "%s %s %s/%s" kCRLF,
//				thiz->method,
//				thiz->uri,
//				thiz->protocol, thiz->version);
//		}
//	}else{
//		// response mode
//		ret = snprintf(text_offset, text_left, "%s/%s %d %s" kCRLF,
//			thiz->protocol, thiz->version, thiz->status_code, thiz->reason_phrase);
//	}
//	text_offset += ret;
//	text_left -= ret;
//	while(NULL != tag){
//		ret = snprintf(text_offset, text_left, "%s: %s" kCRLF,
//			tag->name, tag->val);
//		text_offset += ret;
//		text_left -= ret;
//		tag = tag->next;
//	}
//	ret = snprintf(text_offset, text_left, "%s", kCRLF); // end of header
//	text_offset += ret;
//	text_left -= ret;
//	return (ssize_t)(text_offset - text);
//}
//
//// free the memory allocation of this header, this header would be invalid once free
//static void http_free(struct HTTP_HEAD_FIELD *const thiz)
//{
//	thiz->magic = 0;
//	free(thiz->protocol);
//	free(thiz->version);
//	if(thiz->request_flag){
//		free(thiz->method);
//		free(thiz->uri);
//		free(thiz->query);
//	}else{
//		free(thiz->reason_phrase);
//	}
//	// free the tags of header
//	tag_list_free(&thiz->tags);
//	// free my self
//	free(thiz);
//}
//
//static LP_HTTP_HEAD_FIELD http_header_new()
//{
//	LP_HTTP_HEAD_FIELD header = calloc(sizeof(ST_HTTP_HEAD_FIELD), 1);
//	// interfaces
//	header->add_tag_text = http_add_tag_text;
//	header->add_tag_int = http_add_tag_int;
//	header->del_tag = http_del_tag;
//	header->add_tag_server = http_add_tag_server;
//	header->del_tag_server = http_del_tag_server;
//	header->add_tag_date = http_add_tag_date;
//	header->del_tag_date = http_del_tag_date;
//	header->read_tag = http_read_tag;
//	header->dump = http_dump;
//	header->to_text = http_to_text;
//	header->free = http_free;
//	return header;
//}
//
//LP_HTTP_HEAD_FIELD HTTP_UTIL_new_request_header(HTTP_CSTR_t protocol, HTTP_CSTR_t version, HTTP_CSTR_t method, HTTP_CSTR_t uri, HTTP_CSTR_t query)
//{
//	LP_HTTP_HEAD_FIELD req_header = http_header_new();
//	// the magic
//	req_header->magic = kHTTP_UTIL_MAGIC;
//	req_header->request_flag = true; // very important
//	req_header->protocol = NULL == protocol ? strdup("HTTP") : strdup(protocol);
//	text_to_upper(req_header->protocol); // toupper
//	req_header->version = NULL == version ? strdup("1.1") : strdup(version);
//	req_header->method = strdup(method);
//	text_to_upper(req_header->method); // toupper
//	if(NULL == uri || 0 == strlen(uri)){
//		req_header->uri = strdup("/");
//	}else{
//		req_header->uri = strdup(uri);
//	}
//	req_header->query = (NULL == query) ? strdup("") : strdup(query);
//	if(NULL != query && strlen(query) > 0){
//		size_t query_enc_len = strlen(req_header->query);
//		size_t query_dec_len = 0;
//		HTTP_STR_t query_decode = alloca(query_enc_len + 1);
//		// decode the query string
//		query_dec_len = HTTP_UTIL_url_decode(req_header->query, query_enc_len, query_decode, query_enc_len);
//		free(req_header->query);
//		req_header->query = strndup(query_decode, query_dec_len);
//	}
//	req_header->tags = NULL;
//	return req_header;
//}
//
//LP_HTTP_HEAD_FIELD HTTP_UTIL_new_response_header(HTTP_CSTR_t protocol, HTTP_CSTR_t version, int status_code, HTTP_CSTR_t reason_phrase)
//{
//	LP_HTTP_HEAD_FIELD resp_header = http_header_new();
//	// the magic
//	resp_header->magic = kHTTP_UTIL_MAGIC;
//	resp_header->request_flag = false; // very important
//	resp_header->protocol = NULL == protocol ? strdup("HTTP") : strdup(protocol);
//	text_to_upper(resp_header->protocol); // toupper
//	resp_header->version = NULL == version ? strdup("1.1") : strdup(version);
//	resp_header->status_code = status_code;
//	if(!reason_phrase){
//		resp_header->reason_phrase = strdup(HTTP_UTIL_reason_phrase(resp_header->status_code));
//	}else{
//		resp_header->reason_phrase = strdup(reason_phrase);
//	}
//	resp_header->tags = NULL;
//	return resp_header;
//}
//
//LP_HTTP_HEAD_FIELD HTTP_UTIL_new_response_head_200(int tags, int keepAlive, const char *contentType, int contentLength)
//{
//	LP_HTTP_HEAD_FIELD httpHeadField = HTTP_UTIL_new_response_header(NULL, NULL, 200, NULL);
//	if(NULL != httpHeadField){
//		if(tags & kH_TAG_DATE){
//			httpHeadField->add_tag_date(httpHeadField, 0);
//		}
//		if(tags & kH_TAG_SERVER){
//			httpHeadField->add_tag_server(httpHeadField, "JAWS/1.0");
//		}
//		if(keepAlive > 0){
//			httpHeadField->add_tag_text(httpHeadField, "Connection", "keep-alive", true);
//			httpHeadField->add_tag_int(httpHeadField, "Keep-Alive", keepAlive, true);
//		}else{
//			httpHeadField->add_tag_text(httpHeadField, "Connection", "close", true);
//		}
//		if(NULL != contentType){
//			httpHeadField->add_tag_text(httpHeadField, "Content-Type", contentType, true);
//		}
//		if(contentLength >= 0){
//			httpHeadField->add_tag_int(httpHeadField, "Content-Length", contentLength, true);
//		}
//		return httpHeadField;
//	}
//	return NULL;
//}
//
//static int http_tag_parse(LP_HTTP_HEAD_FIELD header, HTTP_CSTR_t header_text)
//{
//	int ret = -1;
//	char response_line[1024];
//	char *brk = NULL;
//	HTTP_CSTR_t header_offset = header_text; // base offset
//	int tag_cnt = 0;
//	while(NULL != (brk = strpbrk(header_offset, kCRLF))){
//		header_offset = brk + strlen(kCRLF);
//		ret = sscanf(header_offset, "%[^"kCRLF"]", response_line);
//		if(ret > 0){
//			char *token = NULL;
//			char *tag_name = strtok_r(response_line, ":", &token);
//			char *tag_val = strtok_r(NULL, kCRLF, &token); // trim the ending SP
//			header->add_tag_text(header, tag_name, tag_val, false);
//			++tag_cnt; // increase the tags
//		}else{
//			break;
//		}
//	}
//	return tag_cnt;
//}
//
//LP_HTTP_HEAD_FIELD HTTP_UTIL_parse_request_header(HTTP_STR_t header_text, size_t header_len)
//{
//	int ret = 0;
//	char url[8192] = {""}, method[32] = {""}, protocol_version[32] = {""};
//	// read the response line
//	ret = sscanf(header_text, "%s %s %s[^"kCRLF"]", method, url, protocol_version);
//	if(3 == ret){
//		char *token = NULL;
//		char *protocol = strtok_r(protocol_version, "/", &token);
//		char *version = strtok_r(NULL, " ", &token);
//		if(NULL != version){
//			LP_HTTP_HEAD_FIELD http_header = NULL;
//			char *uri = NULL, *query = NULL;
//			// separate the uri and query in url
//			uri = strtok_r(url, "?", &token);
//			if(NULL != uri){
//				query = strtok_r(NULL, " " kCRLF, &token);
//			}
//			http_header = HTTP_UTIL_new_request_header(protocol, version, method, uri, query);
//			if(http_header){
//				ret = http_tag_parse(http_header, strstr(header_text, kCRLF));
//				return http_header;
//			}
//		}
//	}
//	return NULL;
//}
//
//LP_HTTP_HEAD_FIELD HTTP_UTIL_parse_response_header(HTTP_STR_t header_text, size_t header_len)
//{
//	int ret = 0;
//	char response_line[256];
//	int status_code = 0;
//
//	// read the response line
//	ret = sscanf(header_text, "%[^"kCRLF"]", response_line);
//	if(ret > 0){
//		char *token = NULL;
//		char version[32] = {""}, reason_phrase[128] = {""};
//		char *protocol = strtok_r(response_line, "/", &token);
//		if(NULL != protocol){
//			// parse the version, status code and reason phrase
//			ret = sscanf(response_line + strlen(protocol) + strlen("/"),
//				"%s %d %s", version, &status_code, reason_phrase);
//			if(3 == ret){
//				// create a response header of http
//				LP_HTTP_HEAD_FIELD http_header = HTTP_UTIL_new_response_header(protocol, version, status_code, reason_phrase);
//				ret = http_tag_parse(http_header, header_text);
//				return http_header;
//			}
//		}
//	}
//	return NULL;
//}
//
//static ssize_t read_header_from_sock(int sock, HTTP_STR_t buf, size_t stack_len)
//{
//	int n_recv = 0, ret = 0;
//	// peek the receive buffer and check the completed header firstly
//	n_recv = recv(sock, buf, stack_len, MSG_PEEK);
//	if(n_recv < 0){
//		// something error
//		return -1;
//	}
//	// receive the http header at actual
//	n_recv = HTTP_UTIL_check_header(buf, n_recv);
//	ret = recv(sock, buf, n_recv, 0);
//	if(ret != n_recv){
//		// something error
//		return -1;
//	}
//	return n_recv;
//}
//
//#include "app_debug.h" // FIXME:
//
//LP_HTTP_HEAD_FIELD HTTP_UTIL_recv_request_header(int sock)
//{
//	int ret = 0;
//	char sock_buf[4096] = {""};
//	LP_HTTP_HEAD_FIELD http_header = NULL;
//	ret = read_header_from_sock(sock, sock_buf, sizeof(sock_buf) - 1);
//	if(ret <= 0){
//		return NULL;
//	}
//	sock_buf[ret] = 0;
//	http_header = HTTP_UTIL_parse_request_header(sock_buf, ret);
//	if(NULL != http_header){
//		// success to parse the request header
//		// check proxy
//		//char uri_proxy[128] = {""};
//		const char *uri_prefix = "http://";
//		if(0 == strncasecmp(uri_prefix, http_header->uri, strlen(uri_prefix))){
//			struct sockaddr_in sock_addr;
//			socklen_t sock_addrlen = sizeof(sock_addr);
//
//			if(0 == getsockname(sock, &sock_addr, &sock_addrlen)){
//				char uri_path[128] = {""};
//				const char *uri_addr = strdupa(inet_ntoa(sock_addr.sin_addr));
//				int const uri_port = ntohs(sock_addr.sin_port);
//				char *abs_path = NULL;
//
//				sprintf(uri_path, "http://%s:%d/", uri_addr, uri_port);
//				if(0 == strncasecmp(uri_path, http_header->uri, strlen(uri_path))){
//					abs_path = strdup(http_header->uri + strlen(uri_path) - 1);
//
//				}else if(80 == uri_port){
//					// may has the defaut port cut
//					sprintf(uri_path, "http://%s/", uri_addr);
//					if(0 == strncasecmp(uri_path, http_header->uri, strlen(uri_path))){
//						abs_path = strdup(http_header->uri + strlen(uri_path) - 1);
//					}
//				}
//				// it's not a proxy
//				if(NULL != abs_path){
//					// reset the http request uri
//					free(http_header->uri);
//					http_header->uri = abs_path;
//					return http_header;
//				}
//			}
//			http_header->free(http_header);
//			http_header = NULL;
//			// return invalid header here
//		}
//		return http_header;
//	}
//	return NULL;
//}
//
//LP_HTTP_HEAD_FIELD HTTP_UTIL_recv_response_header(int sock)
//{
//	int ret = 0;
//	char sock_buf[4096] = {""};
//	ret = read_header_from_sock(sock, sock_buf, sizeof(sock_buf) - 1);
//	if(ret <= 0){
//		return NULL;
//	}
//	printf("%s\n", sock_buf);
//	return HTTP_UTIL_parse_response_header(sock_buf, ret);
//}
//
//int HTTP_UTIL_recv_response_message(int sock, char *ret_buf, int buf_size)
//{
//	int ret = 0, receive_size, sign_length;
//	char length_str[8];
//	LP_HTTP_HEAD_FIELD recv_header;
//	char sock_buf[4096] = {""};
//	ret = read_header_from_sock(sock, sock_buf, sizeof(sock_buf) - 1);
//	if(ret <= 0){
//		return -1;
//	}
//	printf("%s\n", sock_buf);
//	recv_header = HTTP_UTIL_parse_response_header(sock_buf, ret);
//	if(200 == recv_header->status_code){
//		char *pRev_length = recv_header->read_tag(recv_header, "Content-Length");
//		char *Transfer_Encoding = recv_header->read_tag(recv_header, "Transfer-Encoding");
//		if(NULL != Transfer_Encoding && !strcmp(Transfer_Encoding, "chunked")){
//			ret = recv(sock, sock_buf, sizeof(sock_buf - 1), MSG_PEEK);
//			if(NULL != strstr(sock_buf, "\r\n")){
//				sign_length = strlen("\r\n");
//				ret = sscanf(sock_buf, "%x\r\n%s\r\n0\r\n", &receive_size, ret_buf);
//			}else{//"\n"
//				sign_length = strlen("\n");
//				ret = sscanf(sock_buf, "%x\n%s\n0\n", &receive_size, ret_buf);
//			}
//			sprintf(length_str, "%x", receive_size);
//			ret = recv(sock, sock_buf, strlen(length_str)+ sign_length, 0);
//			ret = recv(sock, sock_buf, receive_size, 0);
//			if(ret == receive_size){
//				memcpy(ret_buf, sock_buf, receive_size);
//			}
//			ret = recv(sock, sock_buf, receive_size, 0);
//			if(NULL == strstr(sock_buf, "0")){
//				goto recv_error;
//			}
//		}else if(NULL != pRev_length){
//			// receive the content of http request
//			receive_size = atoi(pRev_length);
//			if(0 == receive_size){
//				goto recv_error;
//			}
//			ret = recv(sock, ret_buf, buf_size, 0);
//			if(ret != receive_size){
//				goto recv_error;
//			}
//		}
//	}else{
//		goto recv_error;
//	}
//
//	if(recv_header){
//		recv_header->free(recv_header);
//	}
//	return 0;
//
//recv_error:
//	if(recv_header){
//		recv_header->free(recv_header);
//	}
//	return -1;
//}
//
//
//int HTTP_UTIL_recv_response_file(int sock, char *file)
//{
//	int ret = 0, receive_file_size, sign_length, receive_size_cnt;
//	char length_str[8];
//	LP_HTTP_HEAD_FIELD recv_header;
//	char sock_buf[4096] = {""};
//	ret = read_header_from_sock(sock, sock_buf, sizeof(sock_buf) - 1);
//	if(ret <= 0){
//		return -1;
//	}
//	printf("%s\n", sock_buf);
//	recv_header = HTTP_UTIL_parse_response_header(sock_buf, ret);
//	if(NULL != recv_header && 200 == recv_header->status_code){
//		char *pRev_length = recv_header->read_tag(recv_header, "Content-Length");
//		char *Transfer_Encoding = recv_header->read_tag(recv_header, "Transfer-Encoding");
//		if(NULL != Transfer_Encoding && !strcmp(Transfer_Encoding, "chunked")){
//			ret = recv(sock, sock_buf, sizeof(sock_buf - 1), MSG_PEEK);
//			if(NULL != strstr(sock_buf, "\r\n")){
//				sign_length = strlen("\r\n");
//				ret = sscanf(sock_buf, "%x\r\n%s\r\n0\r\n", &receive_file_size);
//			}else{//"\n"
//				sign_length = strlen("\n");
//				ret = sscanf(sock_buf, "%x\n%s\n0\n", &receive_file_size);
//			}
//			sprintf(length_str, "%x", receive_file_size);
//			ret = recv(sock, sock_buf, strlen(length_str)+ sign_length, 0);
//			FILE *fd = fopen(file, "w+b");
//			if(fd){
//				receive_size_cnt = 0;
//				while(receive_file_size > receive_size_cnt){
//					if(receive_file_size - receive_size_cnt > sizeof(sock_buf)-1){
//						ret = recv(sock, sock_buf, sizeof(sock_buf)-1, 0);
//					}else{
//						ret = recv(sock, sock_buf, receive_file_size - receive_size_cnt, 0);
//					}
//					if(ret < 0){
//						printf("receive file error!\n");
//						break;
//					}
//					receive_size_cnt += ret;
//					ret = fwrite(sock_buf, 1, ret, fd);
//					printf("receive_size:%d/%d\n", receive_size_cnt, receive_file_size);
//				}
//				fclose(fd);
//
//				if(ret < 0){
//					goto recv_error;
//				}
//
//				ret = recv(sock, sock_buf, receive_file_size, 0);
//				if(NULL == strstr(sock_buf, "0")){
//					goto recv_error;
//				}
//			}
//		}else if(NULL != pRev_length){
//			// receive the content of http request
//			receive_file_size = atoi(pRev_length);
//			if(0 == receive_file_size){
//				goto recv_error;
//			}
//			FILE *fd = fopen(file, "w+b");
//			if(fd){
//				receive_size_cnt = 0;
//				while(receive_file_size > receive_size_cnt){
//					if(receive_file_size - receive_size_cnt > sizeof(sock_buf)-1){
//						ret = recv(sock, sock_buf, sizeof(sock_buf)-1, 0);
//					}else{
//						ret = recv(sock, sock_buf, receive_file_size - receive_size_cnt, 0);
//					}
//					if(ret < 0){
//						printf("receive file error!\n");
//						break;
//					}
//					receive_size_cnt += ret;
//					ret = fwrite(sock_buf, 1, ret, fd);
//					printf("receive_size:%d/%d\n", receive_size_cnt, receive_file_size);
//				}
//				fclose(fd);
//				if(ret < 0){
//					printf("receive file error!\n");
//					goto	recv_error;
//				}
//			}else{
//				goto	recv_error;
//			}
//		}else{
//			//unknow error
//			goto	recv_error;
//		}
//	}else{
//		goto recv_error;
//	}
//
//	if(recv_header){
//		recv_header->free(recv_header);
//	}
//	return 0;
//
//recv_error:
//	if(recv_header){
//		recv_header->free(recv_header);
//	}
//	return -1;
//}
//
//
////define at app_hichip.c
extern char g_hichip_nonce[36];
int HTTP_UTIL_send_401_digest(int sock, LP_HTTP_HEAD_FIELD request_header)
{
	LP_HTTP_HEAD_FIELD response_header = NULL;
	char response_buf[1024] = {""};
	char nonce_str[128] = {""};
	size_t response_len = 0;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp2_r(sock, &sockTCP);
	int ret;

	response_header = HTTP_UTIL_new_response_header(request_header->protocol,
		request_header->version, 401, NULL);
	response_header->add_tag_server(response_header, "JAWS/1.0");
	response_header->add_tag_date(response_header, 0);
	response_header->add_tag_int(response_header, "Content-Length", 0, true);
	response_header->add_tag_text(response_header, "Connection", "close", true);
	sprintf(nonce_str, "Digest realm=\"IPCAM\" nonce=\"%s\"", g_hichip_nonce);
	response_header->add_tag_text(response_header, "WWW-Authenticate", nonce_str, true);
	response_len = response_header->to_text(response_header, response_buf, sizeof(response_buf));
	response_header->dump(response_header);
	response_header->free(response_header);
	response_header = NULL;
	printf("%s:%d\r\n", __FUNCTION__, __LINE__);
	ret = tcp->send(tcp, response_buf, response_len, 0);
	if(ret < 0){
		// do something!
		printf("send 401 failed %d-%s!\r\n", errno, strerror(errno));
		usleep(5000);
	}
	return 0;
}

int HTTP_UTIL_send_503(int sock, LP_HTTP_HEAD_FIELD request_header)
{
	LP_HTTP_HEAD_FIELD response_header = NULL;
	char response_buf[1024] = {""};
	char nonce_str[128] = {""};
	size_t response_len = 0;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp2_r(sock, &sockTCP);
	int ret;

	response_header = HTTP_UTIL_new_response_header(request_header->protocol,
		request_header->version, 503, NULL);
	response_header->add_tag_server(response_header, "JAWS/1.0");
	response_header->add_tag_date(response_header, 0);
	response_header->add_tag_int(response_header, "Content-Length", 0, true);
	response_header->add_tag_text(response_header, "Connection", "close", true);
	response_len = response_header->to_text(response_header, response_buf, sizeof(response_buf));
	response_header->dump(response_header);
	response_header->free(response_header);
	response_header = NULL;
	printf("%s:%d\r\n", __FUNCTION__, __LINE__);
	ret = tcp->send(tcp, response_buf, response_len, 0);
	if(ret < 0){
		// do something!
		printf("send 503 failed %d-%s!\r\n", errno, strerror(errno));
		usleep(5000);
	}
	return 0;
}

int HTTP_UTIL_send_500(int sock, LP_HTTP_HEAD_FIELD request_header)
{
	LP_HTTP_HEAD_FIELD response_header = NULL;
	char response_buf[1024] = {""};
	char nonce_str[128] = {""};
	size_t response_len = 0;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp2_r(sock, &sockTCP);
	int ret;

	response_header = HTTP_UTIL_new_response_header(request_header->protocol,
		request_header->version, 500, NULL);
	response_header->add_tag_server(response_header, "JAWS/1.0");
	response_header->add_tag_date(response_header, 0);
	response_header->add_tag_int(response_header, "Content-Length", 0, true);
	response_header->add_tag_text(response_header, "Connection", "close", true);
	response_len = response_header->to_text(response_header, response_buf, sizeof(response_buf));
	response_header->dump(response_header);
	response_header->free(response_header);
	response_header = NULL;
	printf("%s:%d\r\n", __FUNCTION__, __LINE__);
	ret = tcp->send(tcp, response_buf, response_len, 0);
	if(ret < 0){
		// do something!
		printf("send 503 failed %d-%s!\r\n", errno, strerror(errno));
		usleep(5000);
	}
	return 0;
}

