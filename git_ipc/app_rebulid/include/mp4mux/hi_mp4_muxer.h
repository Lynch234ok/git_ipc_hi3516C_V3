/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_mp4_muxer.h
* @brief     mp4muxer module header file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#ifndef __HI_MP4_MUXER_H__
#define __HI_MP4_MUXER_H__
#include "hi_error_def.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef enum hiAPP_MP4_ERR_CODE_E
{
    /*general error code*/
    APP_MP4_ERR_HANDLE_INVALID = 0x40,                       /**<mp4 handle invalid*/
    APP_MP4_ERR_INVALID_ARG = 0x41,                          /**<param is null or invalid*/
    APP_MP4_ERR_MALLOC_FAIL = 0x42,                          /**<malloc memory fail*/
    APP_MP4_ERR_CREATE_MUXER = 0x43,                         /**<create mp4 fail*/
    APP_MP4_ERR_DESTROY_MUXER = 0x44,                        /**<destory mp4  fail*/
    APP_MP4_ERR_WRITE_HEAD = 0x45,                           /**<start mp4 fail*/
    APP_MP4_ERR_WRITE_TAIL = 0x46,                           /**<stop mp4 fail*/
    APP_MP4_ERR_CREATE_AGAIN  = 0x47,                        /**<mp4 re created*/
    APP_MP4_ERR_NOT_CREATE = 0x48,                           /**<mp4 not created*/
    APP_MP4_ERR_READ_FRAME  = 0x49,                          /**<read frame fail*/
    APP_MP4_ERR_WRITE_FRAME  = 0x50,                         /**<write frame fail*/
    APP_MP4_ERR_REPAIR  = 0x51,                              /**<write frame fail*/
    APP_MP4_ERR_CREATE_STREAM = 0x52,                        /**<create stream fail*/
    APP_MP4_ERR_DESTROY_STREAM = 0x53,                       /**<destory stream  fail*/
    APP_MP4_ERR_CREATE_BACK = 0x54,                          /**<create back fail*/
    APP_MP4_ERR_DESTROY_BACK = 0x55,                         /**<destory back  fail*/
    APP_MP4_ERR_CREATED_BACK = 0x56,                          /**<create back again*/
    APP_MP4_ERR_NOT_CREATE_BACK = 0x57,                         /**<not create back*/
    APP_MP4_ERR_ADD_ATOM = 0x58,                         /**<not create back*/

    /*file related error code*/
    APP_MP4_ERR_OPETATION_FAIL  = 0x61,     /**<mp4 repair fail*/
    APP_MP4_ERR_UNSUPPORT_CODEC = 0x62,     /**<not support codec*/
    APP_MP4_ERR_OPEN_FILE  = 0x63,          /**<open file error*/
    APP_MP4_ERR_CLOSE_FILE  = 0x64,         /**<close file error*/
    APP_MP4_ERR_READ_FILE  = 0x65,          /**<read file error*/
    APP_MP4_ERR_SEEK_FILE  = 0x66,          /**<seek file error*/
    APP_MP4_ERR_WRITE_FILE  = 0x67,         /**<write file error*/
    APP_MP4_ERR_REMOVE_FILE  = 0x68,        /**<remove file error*/

    APP_MP4_ERR_BUTT = 0xFF
} HI_APP_MP4_ERR_CODE_E;


/*general error code*/
#define HI_ERR_MP4_NULL_PTR       HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_ERR_NULL_PTR)
#define HI_ERR_MP4_HANDLE_INVALID HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_HANDLE_INVALID)
#define HI_ERR_MP4_INVALIDARG     HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_INVALID_ARG)
#define HI_ERR_MP4_MALLOC         HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_MALLOC_FAIL)
#define HI_ERR_MP4_CREATE_MUXER   HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_CREATE_MUXER)
#define HI_ERR_MP4_DESTROY_MUXER  HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_DESTROY_MUXER)
#define HI_ERR_MP4_WRITE_HEAD     HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_WRITE_HEAD)
#define HI_ERR_MP4_WRITE_TAIL     HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_WRITE_TAIL)
#define HI_ERR_MP4_CREATE_AGAIN   HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_CREATE_AGAIN)
#define HI_ERR_MP4_NOT_CREATE     HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_NOT_CREATE)
#define HI_ERR_MP4_READ_FRAME     HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_READ_FRAME)
#define HI_ERR_MP4_WRITE_FRAME    HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_WRITE_FRAME)
#define HI_ERR_MP4_REPAIR_UPDATE  HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_REPAIR)
#define HI_ERR_MP4_CREATE_STREAM  HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_CREATE_STREAM)
#define HI_ERR_MP4_DESTROY_STREAM HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_DESTROY_STREAM)
#define HI_ERR_MP4_CREATE_BACK    HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_CREATE_BACK)
#define HI_ERR_MP4_DESTROY_BACK   HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_DESTROY_BACK)
#define HI_ERR_MP4_CREATED_BACK    HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_CREATED_BACK)
#define HI_ERR_MP4_NOT_CREATE_BACK   HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_NOT_CREATE_BACK)
#define HI_ERR_MP4_ADD_ATOM   HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_ADD_ATOM)

/*FILE related error code*/
#define HI_ERR_MP4_OPERATION_FAILED HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_OPETATION_FAIL)
#define HI_ERR_MP4_UNSUPPORT_CODEC  HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_UNSUPPORT_CODEC)
#define HI_ERR_MP4_OPEN_FILE        HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_OPEN_FILE)
#define HI_ERR_MP4_END_FILE         HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_CLOSE_FILE)
#define HI_ERR_MP4_READ_FILE        HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_READ_FILE)
#define HI_ERR_MP4_SEEK_FILE        HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_SEEK_FILE)
#define HI_ERR_MP4_WRITE_FILE       HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_WRITE_FILE)
#define HI_ERR_MP4_REMOVE_FILE      HI_APP_DEF_ERR(HI_APPID_MP4,APP_ERR_LEVEL_ERROR,APP_MP4_ERR_REMOVE_FILE)

#define MODULE_NAME_MP4  "MP4MUXER"
#define HI_MP4_MAX_FILE_NAME (256) /*file name path max*/
#define HI_MP4_MAX_BOX_PATH (64)/*atom path  max length*/
#define HI_MP4_MAX_HDLR_NAME (128)/*handlr name max length*/
#define HI_MP4_TYPE_SIZE (4) /*box type length*/
#define HI_MP4_BOX_MAX_DATALEN (500*1024) /*user define box datalen max*/
#define HI_MP4_VBUF_MAX_SIZE (5*1024*1024) /*VBUF size max*/
#define HI_MP4_PREALLOC_MAX_SIZE (100*1024*1024) /*pre allocate unit size max*/

typedef enum hiMP4_TRACK_TYPE_E
{
    HI_MP4_STREAM_VIDEO = 1,
    HI_MP4_STREAM_AUDIO,
    HI_MP4_STREAM_DATA,
    HI_MP4_STREAM_BUTT
} HI_MP4_TRACK_TYPE_E;

typedef enum hiMP4_CODECID_E
{
    HI_MP4_CODEC_ID_H264 = 0,
    HI_MP4_CODEC_ID_H265,
    HI_MP4_CODEC_ID_MJPEG,
    HI_MP4_CODEC_ID_AACLC ,
    HI_MP4_CODEC_ID_MP3 ,
    HI_MP4_CODEC_ID_G726 ,
    HI_MP4_CODEC_ID_G711_A ,
    HI_MP4_CODEC_ID_G711_M ,
    HI_MP4_CODEC_ID_DATA,
    HI_MP4_CODEC_ID_BUTT
} HI_MP4_CODECID_E;

typedef struct hiMP4_VIDEOINFO_S
{
    HI_U32 u32Width;/*video width*/
    HI_U32 u32Height;/*video height*/
    HI_U32        u32BitRate;/*bitrate bps*/
    HI_U32        u32FrameRate;/*frame rate fps*/
    HI_MP4_CODECID_E enCodecID;/*codec type*/
} HI_MP4_VIDEOINFO_S;

typedef struct hiMP4_AUDIOINFO_S
{
    HI_U32          u32Channels;/*audio channel num 2 */
    HI_U32          u32SampleRate;/*audio sample rate 48k*/
    HI_U32          u32SamplePerFrame;/*audio sample per frame*/
    HI_U16          u16SampleSize;/*bit per sample , 16*/
    HI_MP4_CODECID_E enCodecID;/*codec type*/
} HI_MP4_AUDIOINFO_S;

typedef struct hiMP4_DATAINFO_S
{
    HI_U32 u32Width;/*meta data width*/
    HI_U32 u32Height;/*meta data height*/
    HI_MP4_CODECID_E enCodecID;/*codec type*/
} HI_MP4_DATAINFO_S;

typedef struct hiMP4_TRACK_INFO_S
{
    HI_MP4_TRACK_TYPE_E enTrackType;/*stream type*/
    HI_U32   u32TimeScale;/*time scale for each trak*/
    HI_FLOAT        fSpeed;/*play speed, (0,1]for slow,(1,~) for fast*/
    HI_CHAR        aszHdlrName[HI_MP4_MAX_HDLR_NAME];/*manufacturer  name */
    union
    {
        HI_MP4_VIDEOINFO_S    stVideoInfo;/*video info*/
        HI_MP4_AUDIOINFO_S    stAudioInfo;/*audio info*/
        HI_MP4_DATAINFO_S     stDataInfo;/*metadata info*/
    };
} HI_MP4_TRACK_INFO_S;

typedef struct hiMP4_FRAME_DATA_S
{
    HI_U64 u64TimeStamp ;/*frame timestamp*/
    HI_U8* pu8DataBuffer;/*frame data buffer*/
    HI_U32 u32DataLength;/*frame date len*/
    HI_BOOL bKeyFrameFlag;/*key frame flag*/
} HI_MP4_FRAME_DATA_S;

typedef struct hiMP4_ATOM_INFO_S
{
    HI_U32 u32DataLen;/*user define atom data len */
    HI_CHAR aszType[HI_MP4_TYPE_SIZE + 1];/*user define atom type*/
    HI_U8* pu8DataBuf;/*user define atom data buffer*/
} HI_MP4_ATOM_INFO_S;

typedef struct hiMP4_REPAIR_OPT_S
{
    HI_S32 (*pfnRepairCreate)(HI_HANDLE* pHandle, const HI_CHAR* pszFileName);/*repair backup create callback*/
    HI_S32 (*pfnRepairUpdate)(HI_HANDLE hHandle,  HI_U8* buffer, HI_U32 u32DataLen);/*repair backup update callback*/
    HI_S32 (*pfnRepairDestroy)(HI_HANDLE hHandle, HI_BOOL bCleanBackup); /*repair back destroy call back*/
} HI_MP4_REPAIR_OPT_S;

/**
 * @brief set duration callback.
 * @param[in] hHANDLE HI_HANDLE :  muxer handle
 * @param[in] u32Timescale HI_U32 : timescale for muxer
 * @param[in,out] pu64Duration HI_U64* : duration for muxer
 * @return   0 success
 * @return  err num  failure
 */
typedef  HI_S32 (*HI_MP4_SETDUTAION_FN)(HI_HANDLE hHandle ,HI_U32 u32Timescale, HI_U64* pu64Duration);/*get and set duration callback*/

typedef struct hiMP4_REPAIR_CONFIG_S
{
    HI_S32 s32RepairFrame;/*repair I frame num before exception [1,60]*/
    HI_MP4_REPAIR_OPT_S stRepairOpt;/*repair backup callback options*/
} HI_MP4_REPAIR_CONFIG_S;

typedef struct hiMP4_MXUER_CONFIG_S
{
    HI_HANDLE hRepairHandle;/*repair handle , -1 for not repair*/
    HI_CHAR aszFileName[HI_MP4_MAX_FILE_NAME];/*file path and file name*/
    HI_U32 u32PreAllocUnit;/*pre allocate size in bytes, [0,100M],0 for not use pre allocate function,suggest 20M, unit :byte*/
    HI_U32 u32VBufSize;/*set the vbuf size for fwrite (0,5M] unit :byte*/
    HI_BOOL bCo64Flag;/*if true;use co64,or use stco*/
    HI_BOOL bConstantFps;/*if true, use constant framerate to calculate time,or use pst delta */
} HI_MP4_MUXER_CONFIG_S;

/**
 * @brief create mp4 muxer.
 * @param[out] pMp4Handle HI_HANDLE* : return MP4 handle
 * @param[in] hRepairHandle HI_HANDLE : repair handle , if invalid means not repair
 * @param[in] pszFileName HI_CHAR* : file name
 * @return   0 success
 * @return  err num  failure
 */
HI_S32  HI_MP4_MUXER_Create(HI_HANDLE* pMp4Handle, HI_MP4_MUXER_CONFIG_S* pstMuxerCfg);

/**
 * @brief destroy mp4 muxer.
 * @param[in] hMp4Handle HI_HANDLE :  MP4 handle
 * @param[out] pu64Duration HI_U64* :  file duration
 * @return   0 success
 * @return  err num  failure
 */

HI_S32 HI_MP4_MUXER_Destroy(HI_HANDLE hMp4Handle, HI_U64* pu64Duration);

/**
 * @brief create mp4 stream.
 * @param[in] pMp4Handle HI_HANDLE : MP4 handle
 * @param[out] pSteamHandle HI_HANDLE* : return stream handle
 * @param[in] pstStreamInfo HI_MP4_TRACK_INFO_S* :stream info
 * @return   0 success
 * @return  err num  failure
 */
HI_S32  HI_MP4_MUXER_CreateTrack(HI_HANDLE  hMp4Handle, HI_HANDLE* pTrackHandle, HI_MP4_TRACK_INFO_S* pstTrackInfo);

/**
 * @brief destroy all mp4 stream.
 * @param[in] hMp4Handle HI_HANDLE :  MP4 handle
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_MUXER_DestroyAllTracks(HI_HANDLE  hMp4Handle , HI_MP4_SETDUTAION_FN pfnSetDuration);


/**
 * @brief write mp4 frame data.
 * @param[in] hMp4Handle HI_S32 : MP4 handle
 * @param[in] hStreamHandle HI_HANDLE : stream handle
 * @param[in] u64TimeStamp HI_U64 : frame pts
 * @param[in] pu8DataBuffer HI_U8* : data buffer
 * @param[in] u32DataLength HI_U32 : data length
 * @param[in] bKeyFrameFlag HI_BOOL : key frame flag
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_MUXER_WriteFrame(HI_HANDLE hMp4Handle, HI_HANDLE hTrackHandle, HI_MP4_FRAME_DATA_S* pstFrameData);

/**
 * @brief end writing mp4 data.
 * /   top
 * /moov
 * /moov/trak[1]/mdia/tkhd
 * @param[in] hMp4Handle HI_S32 : MP4 handle
 * @param[in] pszPath HI_CHAR* : box path [1,64]
 * @param[in] pu8DataBuf HI_U8* : data buffer
 * @param[in] s32Type HI_U32 : box type
 * @param[in] s32Len HI_U32 : data length
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_MUXER_AddAtom(HI_HANDLE hMP4Handle, HI_CHAR* pszAtomPath, HI_MP4_ATOM_INFO_S* pstAtomInfo);

/**
 * @brief create mp4 data.
 * @param[out] pBackFileHandle HI_HANDLE* :  back file handle
 * @param[in] pstRepairCfg HI_MP4_REPAIR_CONFIG_S* : repair config  repairFrame
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_REPAIR_Create(HI_HANDLE* pBackHandle, HI_MP4_REPAIR_CONFIG_S* pstRepairCfg);


/**
 * @brief destroy mp4 data.
 * @param[in] hBackFileHandle HI_HANDLE :  MP4 handle
 * @return   0 success
 * @return  err num  failure
 */

HI_S32 HI_MP4_REPAIR_Destroy( HI_HANDLE hBackHandle);

/**
 * @brief repair mp4 data.
 * @param[in] filename HI_CHAR* : file name
 * @param[in] pu8Buffer HI_U8* : data buffer
 * @param[in] u32DataLen HI_U32 : data buffer len
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_REPAIR_File(HI_CHAR* aszFilename, HI_U8* pu8Buffer, HI_U32 u32DataLen);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MP4_MUXER_H__ */
