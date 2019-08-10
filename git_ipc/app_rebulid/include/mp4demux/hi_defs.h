/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file        hi_defs.h
  * @brief      common define.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */


#ifndef __HI_DEFS_H__
#define __HI_DEFS_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifndef __GNUC__
#define __asm__    asm
#define __inline__ inline
#endif

#define DO_NOTHING

#if HI_OS_TYPE == HI_OS_WIN32
    #ifndef INLINE
        #define INLINE __inline
    #endif
#elif HI_OS_TYPE == HI_OS_LINUX
#define INLINE inline
#endif

/*use for parameter INPUT, *DO NOT Modify the value* */
#define IN
/* use for parameter OUTPUT, the value maybe change when return from the function
 * the init value is ingore in the function.*/
#define OUT
/*use for parameter INPUT and OUTPUT*/
#define IO

/* --------------------------------  */
#ifndef EXTERN
#define EXTERN extern
#endif

#define STATIC static

#define LOCALVAR static
#define GLOBALVAR extern


/**for declaring global variable*/
#define DECLARE_GLOBALVAR

/**for using global variable*/
#define USE_GLOBALVAR extern


#define LOCALFUNC    static
#define EXTERNFUNC   extern

/* -------- Standard input/output/err *****/
#define STDIN  stdin
#define STDOUT stdout
#define STDERR stderr

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

/**conculate the aligned start address,according to base address and align byte counts
 * eg. 4 byte aligned,0x80001232 ALIGN_START(0x80001232,4) = 0x80001230
 */
#define ALIGN_START(v,a) (((v)) & (~((a)-1)))

/**conculate the aligned end address,according to base address and align byte counts
 * eg. 4 byte aligned,0x80001232 ALIGN_START(0x80001232,4) = 0x80001230
 */
#define ALIGN_END(v,a) (((v) & ~((a)-1)) + ((a)-1) )

/**conculate the aligned next address,according to base address and align byte counts
 * eg. 4 byte aligned,0x80001232 ALIGN_START(0x80001232,4) = 0x80001230
 */
#define ALIGN_NEXT(v,a) ((((v) + ((a)-1)) & (~((a)-1))))

#define ALIGN_LENGTH(l, a) ALIGN_NEXT(l, a)

#define ALIGNTYPE_1BYTE  1
/*zsp*/
#define ALIGNTYPE_2BYTE  2
/*x86... default*/
#define ALIGNTYPE_4BYTE  4

#define ALIGNTYPE_8BYTE  8
/*1 Page*/
#define ALIGNTYPE_4K     4096

#define ALIGNTYPE_ZSP    ALIGNTYPE_2BYTE

#define ALIGNTYPE_VIDEO  ALIGNTYPE_8BYTE

#define PACK_ONE_BYTE  __attribute__((__packed__))

/**middleware module id*/
typedef enum hiAPPMOD_ID_E
{
    HI_APPID_REC = 0x0B,                    /**< recoder */
    HI_APPID_SNAP = 0x0C,                   /**< snap */
    HI_APPID_FILEMGR = 0x16,                /**< file manager */
    HI_APPID_STORAGE = 0x18,                /**< storage*/
    HI_APPID_LOCALPLAYER = 0x20,            /**< local player*/
    HI_APPID_RTSPSERVER = 0x22,             /**<rtsp server*/
    HI_APPID_HTTPSERVER = 0x24,             /**<http server*/
    HI_APPID_MBUF = 0x25,                   /**< mbuffer manager */
    HI_APPID_LIVESTREAM = 0x26,             /**< livestream*/
    HI_APPID_RTSPCLIENT = 0x27,             /**< rtsp client */
    HI_APPID_DEMUXER = 0x28,                /**< demuxer */
    HI_APPID_MP4 = 0x29,                    /**< demuxer */
    HI_APPID_DTCF = 0x2A,                   /**< dtcf */
    HI_APPID_LOG = 0x1F,                    /**< log*/
    HI_APPID_BUTT = 0xFF
}APPMOD_ID_E;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HI_DEFS_H__ */
