
/*
*	IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
*
*	By downloading, copying, installing or using the software you agree to this license.
*	If you do not agree to this license, do not download, install,
*	Copy or use the software.
*
*	Copyright (C) 2012, JUAN, Co, All Rights Reserved.
*
*	Project Name: HTTP utilies
*	File Name:
*
*	Writed by Frank Law at 2013 - 06 - 03 JUAN,Guangzhou,Guangdong,China
*
*	ChangeLog:
*		Add interfaces at 2013-06-24 Frank Law
*
*/

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>


#include <NkUtils/http_utils.h>

#ifndef HTTP_UTIL_H_
#define HTTP_UTIL_H_
#ifdef __cplusplus
extern "C" {
#endif


#define HTTP_UTIL_file_mime(__file_ext) NK_HTTPUtils_FileMIME(strrchr(__file_ext, '.'))


#define kCRLF "\r\n"
#define kQUERY_DELIMIT "&"

#define kHTTP_UTIL_MAGIC (('H'<<24)|('T'<<16)|('T'<<8)|('P'<<0))

#define kHTTP_STAT_CONTINUE (100)
#define kHTTP_STAT_SWITCHING_PROTOCOLS (101)
#define kHTTP_STAT_OK (200)
#define kHTTP_STAT_CREATED (201)
#define kHTTP_STAT_ACCEPTED (202)
#define kHTTP_STAT_NON_AUTHORITATIVE_INFOMATION (203)
#define kHTTP_STAT_NO_CONTENT (204)
#define kHTTP_STAT_RESET_CONTENT (205)
#define kHTTP_STAT_PARTIAL_CONTENT (206)
#define kHTTP_STAT_MULTIPLE_CHOICES (300)
#define kHTTP_STAT_MOVED_PERMANENTLY (301)
#define kHTTP_STAT_FOUND (302)
#define kHTTP_STAT_SEE_OTHER (303)
#define kHTTP_STAT_NOT_MODIFIED (304)
#define kHTTP_STAT_USE_PROXY (305)
#define kHTTP_STAT_TEMPORARY_REDIRECT (307)
#define kHTTP_STAT_BAD_REQUEST (400)
#define kHTTP_STAT_UNAUTHORIZED (401)
#define kHTTP_STAT_PAYMENT_REQUIRED (402)
#define kHTTP_STAT_FORBIDDEN (403)
#define kHTTP_STAT_NOT_FOUND (404)
#define kHTTP_STAT_METHOD_NOT_ALLOWED (405)
#define kHTTP_STAT_NOT_ACCEPTABLE (406)
#define kHTTP_STAT_PROXY_AUTHENTICATION_REQUIRED (407)
#define kHTTP_STAT_REQUEST_TIME_OUT (408)
#define kHTTP_STAT_CONFLICT (409)
#define kHTTP_STAT_GONE (410)
#define kHTTP_STAT_LENGTH_REQUIRED (411)
#define kHTTP_STAT_PRECONDITION_FAILED (412)
#define kHTTP_STAT_REQUEST_ENTITY_TOO_LARGE (413)
#define kHTTP_STAT_REQUEST_URI_TOO_LARGE (414)
#define kHTTP_STAT_UNSUPPORTED_MEDIA_TYPE (415)
#define kHTTP_STAT_REQUESTED_RANGE_NOT_SATISFIABLE (416)
#define kHTTP_STAT_EXPECTATION_FAILED (417)
#define kHTTP_STAT_INTERNAL_SERVER_ERROR (500)
#define kHTTP_STAT_NOT_IMPLEMENTED (501)
#define kHTTP_STAT_BAD_GATEWAY (502)
#define kHTTP_STAT_SERVICE_UNAVAILABLE (503)
#define kHTTP_STAT_GATEWAY_TIME_OUT (504)
#define kHTTP_STAT_HTTP_VERSION_NOT_SUPPORTED (505)

typedef char* HTTP_STR_t;
typedef const char* HTTP_CSTR_t;

typedef struct HTTP_TAG {
	HTTP_STR_t name;
	HTTP_STR_t val;
	struct HTTP_TAG *next;
}ST_HTTP_TAG, *LP_HTTP_TAG;

typedef struct HTTP_QUERY_PARA_LIST {
	LP_HTTP_TAG paras;
	
	HTTP_CSTR_t (*read)(struct HTTP_QUERY_PARA_LIST *const list, HTTP_CSTR_t name);

	int (*add)(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_STR_t name, HTTP_STR_t val);
	int (*del)(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_STR_t name);
	
	void (*dump)(struct HTTP_QUERY_PARA_LIST *const thiz);
	ssize_t (*to_text)(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_STR_t text, size_t text_size);
	
	void (*free)(struct HTTP_QUERY_PARA_LIST *const thiz);
}ST_HTTP_QUERY_PARA_LIST, *LP_HTTP_QUERY_PARA_LIST;

extern LP_HTTP_QUERY_PARA_LIST HTTP_UTIL_parse_query_as_para(HTTP_STR_t query_text);

typedef struct HTTP_HEAD_FIELD {
	uint32_t magic; // == "HTTP"
	bool request_flag; // if true, this is a request header, else is a response header
	HTTP_STR_t protocol; // HTTP, RTSP ...1.0, 1.1...
	HTTP_STR_t version;
	union {
		// if request_flag is true
		// 5.1 Request-Line = Method SP Request-URI SP HTTP-VERSION CRLF
		// 5.1.2 Request-URI = "*"|absoluteURI|abs_path|authority
		struct {
			HTTP_STR_t method;
			HTTP_STR_t uri;
			HTTP_STR_t query;
		};
		// if request_flag = false
		struct {
			int status_code;
			HTTP_STR_t reason_phrase;
		};
	};
	LP_HTTP_TAG tags;
	
	// add & delete the tag
	int (*add_tag_text)(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t tag_name, HTTP_CSTR_t tag_val, bool over_write);
	int (*add_tag_int)(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t tag_name, int tag_val, bool over_write);
	int (*del_tag)(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t tag_name);
	// add & delete tag in special
	int (*add_tag_server)(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t server_name);
	int (*del_tag_server)(struct HTTP_HEAD_FIELD *const thiz);
	int (*add_tag_date)(struct HTTP_HEAD_FIELD *const thiz, time_t t);
	int (*del_tag_date)(struct HTTP_HEAD_FIELD *const thiz);
	
	HTTP_CSTR_t (*read_tag)(struct HTTP_HEAD_FIELD *const thiz, HTTP_CSTR_t tag_name);
	
	void (*dump)(struct HTTP_HEAD_FIELD *const thiz);
	ssize_t (*to_text)(struct HTTP_HEAD_FIELD *const thiz, HTTP_STR_t text, size_t text_size);

	// free the memory allocation of this header, this header would be invalid once free
	void (*free)(struct HTTP_HEAD_FIELD *const thiz);
}ST_HTTP_HEAD_FIELD, *LP_HTTP_HEAD_FIELD;

// filter the double slash in url
extern void HTTP_UTIL_url_filter(HTTP_STR_t url);

// uri encode / decode
extern ssize_t HTTP_UTIL_url_encode(HTTP_CSTR_t in_text, size_t in_size, HTTP_STR_t out_text, size_t out_size);
extern ssize_t HTTP_UTIL_url_decode(HTTP_CSTR_t in_text, size_t in_size, HTTP_STR_t out_text, size_t out_size);

// check whether there is an http header in text
extern ssize_t HTTP_UTIL_check_header(HTTP_STR_t text, size_t stack_len);

// http response reason phrase
extern HTTP_CSTR_t HTTP_UTIL_reason_phrase(int status_code);

// http date header value
extern HTTP_CSTR_t HTTP_UTIL_date_header(HTTP_STR_t buf, size_t buf_len, time_t t);

// new a request / response header of http
extern LP_HTTP_HEAD_FIELD HTTP_UTIL_new_request_header(HTTP_CSTR_t protocol, HTTP_CSTR_t version, HTTP_CSTR_t method, HTTP_CSTR_t uri, HTTP_CSTR_t query);
extern LP_HTTP_HEAD_FIELD HTTP_UTIL_new_response_header(HTTP_CSTR_t protocol, HTTP_CSTR_t version, int status_code, HTTP_CSTR_t reason_phrase);

#define kH_TAG_SERVER (1<<0)
#define kH_TAG_DATE (1<<1)
extern LP_HTTP_HEAD_FIELD HTTP_UTIL_new_response_head_200(int tags, int keepAlive, const char *contentType, int contentLength);


// parse a request / response header of http from text
extern LP_HTTP_HEAD_FIELD HTTP_UTIL_parse_request_header(HTTP_STR_t header_text, size_t header_len);
extern LP_HTTP_HEAD_FIELD HTTP_UTIL_parse_response_header(HTTP_STR_t header_text, size_t header_len);

// receive a request / response header of http from socket
extern LP_HTTP_HEAD_FIELD HTTP_UTIL_recv_request_header(int sock);
extern LP_HTTP_HEAD_FIELD HTTP_UTIL_recv_response_header(int sock);
extern int HTTP_UTIL_send_401_digest(int sock, LP_HTTP_HEAD_FIELD request_header);
extern int HTTP_UTIL_send_500(int sock, LP_HTTP_HEAD_FIELD request_header);
extern int HTTP_UTIL_send_503(int sock, LP_HTTP_HEAD_FIELD request_header);


#ifdef __cplusplus
};
#endif
#endif //HTTP_UTIL_H_
