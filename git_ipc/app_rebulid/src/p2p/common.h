/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:		common.h
 * Describle:
 * History:		rename from common.h, now will put some 
				common funtion or common struct 
 * Last modified:	2014-07-16 17:24
=============================================================*/
#ifndef JAP2P_COMMON_H
#define JAP2P_COMMON_H

#include <stdio.h>
#include <stdbool.h>


typedef unsigned char UINT8;
typedef signed char SINT8;
typedef char CHAR;  //for char* 's signedness warnings
typedef unsigned short UINT16;
typedef signed short SINT16;
typedef unsigned int UINT32;
typedef signed int SINT32;
typedef unsigned long long UINT64;
typedef signed long long SINT64;


/****************Debug  Macro**************************************/
#include <assert.h>

#define MODULE "JAP2P"
#define DEBUG_LEVEL 7


#define PRINT(syntax,fmt,arg...)  \
	do{	\
		printf("\033[%sm[%s|%s:%d]", syntax, MODULE, __FILE__, __LINE__); \
		printf(fmt,##arg); \
		printf("\033[0m");\
	}while(0);

#if DEBUG_LEVEL >= 0
#define JAEMERG(fmt, arg...) PRINT("1;31",fmt,##arg)
#else
#define JAEMERG(fmt, arg...) 
#endif

#if DEBUG_LEVEL >= 1
#define JACRIT(fmt, arg...) PRINT("1;31",fmt,##arg)
#else
#define JACRIT(fmt, arg...) 
#endif

#if DEBUG_LEVEL >= 2
#define JAALERT(fmt, arg...) PRINT("1;31",fmt,##arg)
#else
#define JAALERT(fmt, arg...) 
#endif

#if DEBUG_LEVEL >= 3
#define JAERR(fmt, arg...) PRINT("1;31",fmt,##arg)
#else
#define JAERR(fmt, arg...) 
#endif

#if DEBUG_LEVEL >= 4
#define JAWARN(fmt, arg...) PRINT("1;31",fmt,##arg)
#else
#define JAWARN(fmt, arg...) 
#endif

#if DEBUG_LEVEL >= 5
#define JANOTICE(fmt, arg...) PRINT("1;32",fmt,##arg)
#else
#define JANOTICE(fmt, arg...) 
#endif
#if DEBUG_LEVEL >= 6
#define JAINFO(fmt, arg...) PRINT("1;33",fmt,##arg)
#else
#define JAINFO(fmt, arg...) 
#endif
#if DEBUG_LEVEL >= 7
#define JATRACE(fmt, arg...) PRINT("0",fmt,##arg)
#else
#define JATRACE(fmt, arg...) 
#endif


#if DEBUG_LEVEL >= 7
#define JAASSERT(exp, fmt, arg...) \
	do{\
	if(!(exp)){\
				printf("\033[1;31m[%s|%s:%d] assert(\"%s\")",MODULE, __FILE__,__LINE__,#exp);\
				printf(fmt, ##arg);\
				printf("\n");\
				printf("\033[0m");\
				exit(1);\
			}\
	}while(0);

#else
#define JAASSERT(exp, fmt, arg...)
#endif


/****************note some important log to tmp filesystem********/
/*
	log: log str
	log_file: file to store the log  (eg. /tmp/rudpa.port)
	mode : w+, a+,..... mode of open the log_file
*/
SINT32 JALOG(const CHAR *log_file, const CHAR *mode, const CHAR *log_fmt, ...);
SINT32 JALOG_BB(const CHAR *log_file, const CHAR *mode, const CHAR *log_fmt, ...);

#endif
