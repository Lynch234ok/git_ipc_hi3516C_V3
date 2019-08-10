#ifndef __CIRCLEBUF_H__
#define __CIRCLEBUF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gnu_win.h"

#ifndef RET_OK
#define RET_OK 	(0)
#endif
#ifndef RET_FAIL
#define RET_FAIL (-1)
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#define CIRCLEBUF_WAITTIMEOUT		(5*1000*1000) //5s

typedef void* lpCircleBuf;  
typedef void (*pfn_CircleBufDestroy)(lpCircleBuf cb);
typedef void (*pfn_CircleBufWrite)(lpCircleBuf cb, const void* pSourceBuffer, const unsigned int iNumBytes);
typedef int (*pfn_CircleBufRead)(lpCircleBuf cb, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* pbBytesRead);
typedef unsigned int (*pfn_CircleGetUsedSpace)(lpCircleBuf cb);
typedef unsigned int (*pfn_CircleGetFreeSpace)(lpCircleBuf cb);
typedef void (*pfn_CircleFlush)(lpCircleBuf cb);

typedef struct _CircleBuf
{
    pfn_CircleBufWrite Write;
    pfn_CircleBufRead Read;
    pfn_CircleFlush Flush;
    pfn_CircleGetUsedSpace GetUsedSize;
    pfn_CircleGetFreeSpace GetFreeSize;
	//
	pfn_CircleBufDestroy Destroy;

    unsigned char* m_pBuffer;
    unsigned int m_iBufferSize;
    unsigned int m_iReadCursor;
    unsigned int m_iWriteCursor;
    EventHandle_t m_evtDataAvailable;
    Lock_t m_csCircleBuf;
} CircleBuf_t;
//
////////////////////////////////////////////////////////////////////////////////
CircleBuf_t* CircleBuf_new(const unsigned int iBufferSize);

#ifdef __cplusplus
}
#endif

#endif

