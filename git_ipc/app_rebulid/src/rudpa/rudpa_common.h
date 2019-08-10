/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	rudpa_common.h
 * Describle:
 * History:		rename from rudpa_common.h, now will put some 
				common funtion 
 * Last modified: 2014-02-21 16:56
=============================================================*/
#ifndef __RUDPA_COMMON_H__
#define __RUDPA_COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif



/****************Debug  Macro**************************************/
#include <assert.h>

#define _RUDPA_DEBUG_LEVEL  3
#define _RUDPA_PRINT(syntax,fmt,arg...)  \
	do{	\
		printf("\033[%sm""[RUDPA |%s:%d ]\033[0m",syntax,basename(__FILE__),__LINE__); \
		printf(fmt,##arg); \
	}while(0);

#if _RUDPA_DEBUG_LEVEL <= 1
#define _RUDPA_DEBUG(fmt, arg...) _RUDPA_PRINT("34",fmt,##arg)
#else
#define _RUDPA_DEBUG(fmt,arg...)
#endif

#if _RUDPA_DEBUG_LEVEL <= 2
#define _RUDPA_STUB(fmt, arg...) _RUDPA_PRINT("33",fmt,##arg)
#else
#define _RUDPA_STUB(fmt,arg...)
#endif


#if _RUDPA_DEBUG_LEVEL <= 3
#define _RUDPA_TRACE(fmt, arg...) _RUDPA_PRINT("32",fmt,##arg)
#else
#define _RUDPA_TRACE(fmt, arg...)
#endif


#if _RUDPA_DEBUG_LEVEL <= 4
#define _RUDPA_ERROR(fmt, arg...) _RUDPA_PRINT("31",fmt,##arg)
#else
#define _RUDPA_ERROR(fmt, arg...)

#endif

#define _RUDPA_ASSERT(exp, fmt, arg...)
/*
#define _RUDPA_ASSERT(exp, fmt, arg...) \
	{\
	if(!(exp)){\
				printf("[RUDPA | %s:%d ]",basename(__FILE__),__LINE__);\
				printf(fmt, ##arg);\
				printf("\r\n");\
			}\
	}
*/
#define DBG(fmt,arg...) _RUDPA_DEBUG(fmt,##arg)

#define TRACE(fmt,arg...) _RUDPA_TRACE(fmt,##arg)


/****************note some important log to tmp filesystem********/
/*
	log: log str
	log_file: file to store the log  (eg. /tmp/rudpa.port)
	mode : w+, a+,..... mode of open the log_file
*/
extern uint32_t rudpa_log( char *log_file, char *mode, char *log_fmt, ...);




#ifdef __cplusplus
}
#endif
#endif
