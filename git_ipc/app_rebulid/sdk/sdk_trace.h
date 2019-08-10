
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SOC_TRACE_H_
#define SOC_TRACE_H_
#ifdef __cplusplus
extern "C" {
#endif

#define SOC_TRACE_LEVEL (0)
#define __SOC_TRACE(syntax,fmt,arg...) \
	do{\
		char *const bname = basename(strdupa(__FILE__));\
		printf("\033[%sm[%16s:%4d]\033[0m "fmt"\r\n", syntax, bname, __LINE__, ##arg);\
	}while(0)


#if (SOC_TRACE_LEVEL==0)
	#define SOC_EMERG(fmt,arg...) __SOC_TRACE("1;30",fmt,##arg)
#else
	#define SOC_EMERG(fmt,arg...)
#endif

#if (SOC_TRACE_LEVEL<=1)
	#define SOC_ALERT(fmt,arg...) __SOC_TRACE("1;31",fmt,##arg)
#else
	#define SOC_ALERT(fmt,arg...)
#endif

#if (SOC_TRACE_LEVEL<=2)
	#define SOC_CRIT(fmt,arg...) __SOC_TRACE("1;32",fmt,##arg)
#else
	#define SOC_CRIT(fmt,arg...)
#endif

#if (SOC_TRACE_LEVEL<=3)
	#define SOC_ERR(fmt,arg...) __SOC_TRACE("1;33",fmt,##arg)
#else
	#define SOC_ERR(fmt,arg...)
#endif

#if (SOC_TRACE_LEVEL<=4)
	#define SOC_WARNING(fmt,arg...) __SOC_TRACE("1;34",fmt,##arg)
#else
	#define SOC_WARNING(fmt,arg...)
#endif

#if (SOC_TRACE_LEVEL<=5)
	#define SOC_NOTICE(fmt,arg...) __SOC_TRACE("1;35",fmt,##arg)
#else
	#define SOC_NOTICE(fmt,arg...)
#endif

#if (SOC_TRACE_LEVEL<=6)
	#define SOC_INFO(fmt,arg...) __SOC_TRACE("1;36",fmt,##arg)
#else
	#define SOC_INFO(fmt,arg...)
#endif

#if (SOC_TRACE_LEVEL<=7)
	#define SOC_DEBUG(fmt,arg...) __SOC_TRACE("37",fmt,##arg)
#else
	#define SOC_DEBUG(fmt,arg...)
#endif

#define SOC_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			char *const bname = basename(strdupa(__FILE__));\
			printf("\033[1;30m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#define SOC_CHECK(exp) \
	do{\
		HI_S32 ret = exp;\
		if(HI_SUCCESS != ret){\
			char *const bname = basename(strdupa(__FILE__));\
			printf("\033[1;30m");\
			printf("%s @ [%s: %d] err: 0x%08x <%s>", #exp, bname, __LINE__, ret, sdk_strerror(ret));\
			printf("\033[0m\r\n");\
		}\
	}while(0)



#ifdef __cplusplus
};
#endif
#endif //SOC_TRACE_H_

