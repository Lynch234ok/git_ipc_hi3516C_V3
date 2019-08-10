#ifndef __CROSS_THREAD_HEAD_FILE__
#define __CROSS_THREAD_HEAD_FILE__
#include "stdinc.h"


#ifdef WIN32
typedef DWORD THREAD_ID_t;

#define exitThread_c(x) _endthread()
#define detatchThread_c(x)
#define cancelThread_c(x)       TerminateThread(x, 0)

typedef HANDLE THREAD_HANDLE;
typedef DWORD THREAD_RETURN;
#else
typedef pthread_t THRAED_ID_t;

#define exitThread_c(x) pthread_exit(x);
#define detatchThread_c(x) pthread_detach(x);
#define cancelThread_c(x)       pthread_cancel(x)

typedef pthread_t THREAD_HANDLE;
typedef pthread_t THREAD_ID_t;
typedef void * THREAD_RETURN;
#endif

typedef THREAD_RETURN (*THREAD_PROC)(void *);

#ifdef __cplusplus
extern "C" {
#endif

extern THREAD_ID_t currentThreadId_c();

extern int initThread_c(THREAD_HANDLE *hThread,THREAD_PROC proc,void *ThreadParam);

extern int joinThread_c(THREAD_HANDLE hThread);

#define JA_THREAD_init		initThread_c
#define JA_THREAD_join		joinThread_c
#define JA_CUR_THREAD_ID	currentThreadId_c

extern int JA_THREAD_init0(THREAD_HANDLE *hThread,THREAD_PROC proc,void *ThreadParam, const char *thName, 
	int detached, void *stackBuf, int stackSize, int priority);

extern int JA_THREAD_set_name(THREAD_HANDLE hThread, const char *name);
extern int JA_THREAD_get_name(THREAD_HANDLE hThread, char *name, int outSize);

#ifdef __cplusplus
};
#endif

#endif
