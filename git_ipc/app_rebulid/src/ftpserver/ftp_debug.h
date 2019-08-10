#ifndef __FTP_DEBUG_H__
#define __FTP_DEBUG_H__

#define FTP_DEBUG (1)
#if FTP_DEBUG
#define FTP_SYNTAX "1;31"
#define FTP_TRACE(fmt, arg...) \
	do{\
		const char* bname = strdupa(basename(__FILE__));\
		printf("\033["FTP_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define FTP_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = strdupa(basename(__FILE__));\
			printf("\033["FTP_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define FTP_TRACE(fmt...)
#define FTP_ASSERT(exp, fmt, arg...)
#endif


#endif 