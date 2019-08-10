#ifndef _OBJECT_H
#define _OBJECT_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdbool.h> 
#include <pthread.h>

#define JA_Boolean 			bool
#define JA_Void				void
#define JA_PVoid			void*
#define JA_Int				int
#define JA_Float			float
#define JA_Nil				NULL
#define JA_Integer			int
#define JA_Char				char
#define JA_PChar			char*
#define JA_False			false
#define JA_True				true
#define JA_UInt32			unsigned int
//#define TJA_Mutex			pthread_mutex_t
/*
#define	FOCUS_SIZE_2812		1

#if	FOCUS_SIZE_2812
#define CALCULATE_INTER		(20)	//计算间隔，每隔CALCULATE_INTER个zoom步就计算一次focus步
#define LOW_ZOOM_STEP		(180)
#define HIGH_ZOOM_STEP		(2006)
#define LOW_FOCUS_STEP		(20)
#define HIGH_FOCUS_STEP		(2342)
#define ALPHA_STEP			(1.0295)
#define BELTA_STEP			(-5.4873)

#elif FOCUS_SIZE_2808

#endif
*/


#define JAE_ASSERT(expr)\
	do{\
		if( !(expr) )\
		{\
			printf("JAE_ASSERT Error!!! File: "__FILE__", Line: %05d\n", __LINE__);\
			abort();\
		}\
	}while(0)

#ifdef _DEBUG
#define JaLog(format,...)				printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__) 
//typedef  void (*TJA_MemAllocator)(void);
#else
#define JaLog(format,...)
#endif


typedef struct tagTJA_Mutex TJA_Mutex;

struct tagTJA_Mutex{
	int (*lock)(TJA_Mutex *);
	int (*unlock)(TJA_Mutex *);
	int (*trylock)(TJA_Mutex*);
};

typedef struct tagTJA_MemAllocator TJA_MemAllocator;

struct tagTJA_MemAllocator{
	int (*free)(TJA_MemAllocator*, void *);
};

typedef enum ObjType{

	OBJ_PAN = 0,
	OBJ_TILT,
}enObjType;
/*
#define JA_CPP_EXTERN_BEGIN \
	#ifdef __cplusplus \
	extern "C" { \
	#endif

#define	JA_CPP_EXTERN_END \
	#ifdef  __cplusplus \
		} \
	#endif
*/


#endif

