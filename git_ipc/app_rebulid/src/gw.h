/*
 * gw.h
 *
 *  Created on: 2011-8-29
 *      Author: root
 */

#ifndef __GW_H__
#define __GW_H__

#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>                 /*socket address struct*/
#include <arpa/inet.h>                  /*host to network convertion*/
#include <sys/socket.h>
#include <fcntl.h>

#include "httpd.h"

typedef enum
{
	JUAN_VERSION_010000,
	JUAN_VERSION_CNT,
}JUAN_VERSION;

typedef enum
{
	JUAN_ERRNO_NO_ERROR,
	JUAN_ERRNO_INPUT_TOO_LONG,
	JUAN_ERRNO_VERSION_INVALID,
	JUAN_ERRNO_DIRECTION_ERROR,
	JUAN_ERRNO_USR_OR_PWD_ERROR,
	JUAN_ERRNO_PARAM_ERROR,
	JUAN_ERRNO_APP_RESPONSE_ERROR,
	JUAN_ERRNO_USR_MNG_ERROR, //Υ���û��������
	JUAN_ERRNO_CNT,
}JUAN_ERRNO;

typedef enum
{
	JUAN_DIRECTION_CLIENT_TO_SERVER,
	JUAN_DIRECTION_SERVER_TO_CLIENT,
	JUAN_DIRECTION_CNT,
}JUAN_DIRECTION;

typedef enum
{
	JUAN_ENC_GB2312,
	JUAN_ENC_UTF8,
	JUAN_ENC_CNT,
}JUAN_ENC;

typedef enum
{
	JUAN_ENVLOAD_TYPE_READ,
	JUAN_ENVLOAD_TYPE_WRITE,
	JUAN_ENVLOAD_TYPE_CNT,
}JUAN_ENVLOAD_TYPE;

typedef enum
{
	JUAN_COPYG_TYPE_COLOR,
	JUAN_COPYG_TYPE_ENCODE,
	JUAN_COPYG_TYPE_DETECTION,
	JUAN_COPYG_TYPE_PTZ,
	JUAN_COPYG_TYPE_SENSOR,
	JUAN_COPYG_TYPE_SCREEN,
	JUAN_COPYG_TYPE_ENCODESUB,
	JUAN_COPYG_TYPE_CNT,
}JUAN_COPYG_TYPE;

typedef enum
{
	JUAN_USRMNG_ACTION_LIST,
	JUAN_USRMNG_ACTION_ADD,
	JUAN_USRMNG_ACTION_RM,
	JUAN_USRMNG_ACTION_MODIFY,
	JUAN_USRMNG_ACTION_CNT,
}JUAN_USRMNG_ACTION;

typedef enum
{
	JUAN_HDD_ACTION_LIST,
	JUAN_HDD_ACTION_CNT,
}JUAN_HDD_ACTION;

#define MAX_XML_BUF (10240)


extern int cgi_gw_main(HTTPD_SESSION_t* _session);

#include "web_server.h"

extern int cgi_gw_main2(LP_HTTP_CONTEXT context);


#endif  /*__GW_H__*/
