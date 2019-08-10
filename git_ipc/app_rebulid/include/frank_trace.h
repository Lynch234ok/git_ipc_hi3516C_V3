
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef FRANK_TRACE_H_
#define FRANK_TRACE_H_
#ifdef __cplusplus
extern "C" {
#endif

#define _DEBUG (0)
#if _DEBUG
#define _TRACE(fmt, arg...) \
	do{\
		const char* bname = basename(strdupa(__FILE__));\
		int syntax_fg = 0;\
		int i = 0;\
		for(i = 0; i < (typeof(i))(strlen(bname)); ++i){ syntax_fg ^= bname[i]; }\
		printf("\033[1;%dm[%12s:%4d]\033[0m ", 30 + (syntax_fg % 7), bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define _ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = basename(strdupa(__FILE__));\
			int const syntax_fg = bname[0] % 8;\
			printf("\033[1;%dm[%12s:%4d]\033[0m assert(\"%s\") ", 30 + (syntax_fg % 7), bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define _TRACE(fmt...)
#define _ASSERT(exp, fmt...)
#endif

#ifdef __cplusplus
}
#endif
#endif //FRANK_TRACE_H_

