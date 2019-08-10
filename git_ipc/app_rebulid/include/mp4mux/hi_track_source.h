/**
* Copyright (C), 2017-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file          hi_track_source.h
* @brief       track source define head file
* @author    HiMobileCam middleware develop team
* @date       2017.02.25
*/

#ifndef _HI_TRACK_SOURCE_H
#define _HI_TRACK_SOURCE_H
#include "hi_type.h"

typedef enum hiTrack_SourceType_E
{
    HI_TRACK_SOURCE_TYPE_PRIV,
    HI_TRACK_SOURCE_TYPE_VIDEO,
    HI_TRACK_SOURCE_TYPE_AUDIO
} HI_Track_SourceType_E;


typedef enum hiTrack_VideoCodec_E
{
    HI_TRACK_VIDEO_CODEC_H264,
    HI_TRACK_VIDEO_CODEC_H265,
    HI_TRACK_VIDEO_CODEC_MJPEG
} HI_Track_VideoCodec_E;


typedef enum hiTrack_AudioCodec_E
{
    HI_TRACK_AUDIO_CODEC_AAC,
    HI_TRACK_AUDIO_CODEC_MP3
} HI_Track_AudioCodec_E;


typedef struct hiTrack_VideoSourceInfo_S
{
    HI_Track_VideoCodec_E enCodecType;
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_U32 u32BitRate;
    HI_U32 u32FrameRate;
    HI_U32 u32Gop;
    HI_FLOAT fSpeed;
} HI_Track_VideoSourceInfo_S;

typedef struct hiTrack_AudioSourceInfo_S
{
    HI_Track_AudioCodec_E enCodecType;
    HI_U32 u32ChnCnt;
    HI_U32 u32SampleRate;
    HI_U32 u32AvgBytesPerSec;
    HI_U32 u32SamplesPerFrame;
    HI_U16 u16SampleBitWidth;
} HI_Track_AudioSourceInfo_S;

typedef struct hiTrack_PrivateSourceInfo_S
{
    HI_U32 u32PrivateData;
} HI_Track_PrivateSourceInfo_S;

typedef struct hiTrack_Source_S HI_Track_Source_S;
typedef HI_Track_Source_S* HI_Track_Source_Handle;

typedef HI_S32 (*HI_Track_Source_Start_FN)(HI_Track_Source_Handle pTrackSource);
typedef HI_S32 (*HI_Track_Source_Stop_FN)(HI_Track_Source_Handle pTrackSource);

struct hiTrack_Source_S
{
    HI_S32 s32PrivateHandle;  // venc/aenc/ handle
    HI_Track_Source_Start_FN pfnSourceStart;
    HI_Track_Source_Stop_FN pfnSourceStop;

    HI_Track_SourceType_E enTrackType;
    union
    {
        HI_Track_VideoSourceInfo_S stVideoInfo; /**<video track info*/
        HI_Track_AudioSourceInfo_S stAudioInfo; /**<audio track info*/
        HI_Track_PrivateSourceInfo_S stPrivInfo;/**<private track info*/
    } unTrackSourceAttr;

};

#endif
