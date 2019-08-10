
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include "log.h"

#ifndef _REMOTE_UPGRADE_DEBUG_H_
#define _REMOTE_UPGRADE_DEBUG_H_
#ifdef __cplusplus
extern "C" {
#endif

#define REMOTE_UPGRADE_DEBUG (1)


#if REMOTE_UPGRADE_DEBUG
#define RU_TRACE(fmt, arg...) \
	do{\
			const char* bname = basename(strdupa(__FILE__));\
			int const syntax_fg = ((bname[0] >> 8) + (bname[0] & 0xff)) % 8;\
			printf("\033[1;%dm[%12s:%4d]\033[0m ", 31 + syntax_fg, bname, __LINE__);\
			printf(fmt, ##arg);\
			printf("\r\n");\
		}while(0)
#else
#define RU_TRACE(fmt...)
#endif

#ifdef __cplusplus
}
#endif
#endif //_REMOTE_UPGRADE_DEBUG_H_

