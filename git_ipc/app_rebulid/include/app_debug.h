
#include <string.h>
#include <errno.h>

#ifndef APP_DEBUG_H_
#define APP_DEBUG_H_
#ifdef __cplusplus
extern "C" {
#endif

#define APP_DEBUG (1)

#if APP_DEBUG
#define APP_TRACE(fmt, arg...) \
	do{\
		const char* bname = basename(strdupa(__FILE__));\
		int const syntax_fg = ((bname[0] >> 8) + (bname[0] & 0xff)) % 8;\
		printf("\033[1;%dm[%12s:%4d]\033[0m ", 30 + syntax_fg, bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define APP_ASSERT(exp, fmt...)
#else
#define APP_TRACE(fmt...)
#define APP_ASSERT(exp, fmt...)
#endif

#define OS_USER_EXEC(__name, __exp) do{ if(0 == strcmp(__name, OS_USER)){ __exp;} }while(0)
#define FRANK_EXEC(__exp) do{ OS_USER_EXEC("Frank", __exp); }while(0)

#ifdef __cplusplus
}
#endif
#endif //APP_DEBUG_H_

