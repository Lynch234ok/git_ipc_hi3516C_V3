/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_ffmpeg_format.c
* @brief     hidemuxer module header file
* @author    HiMobileCam middleware develop team
* @date      2016.11.03  */


#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
//#include <sys/syscall.h>
#include <string.h>
#include <mp4demux/libavformat/avformat.h>
#include <mp4demux/libavutil/avstring.h>
#include <mp4demux/libavutil/buffer.h>
#include <mp4demux/libavutil/mem.h>
#include <mp4demux/hi_demuxer.h>
#include <mp4demux/hi_demuxer_err.h>
#include <mp4demux/hi_ffdemux_log.h>
#include <mp4demux/libavcodec/avcodec.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MODULE_NAME_DEMUX  "ffmpegDemux"
#define FFMPEG_TIME_UNIT      (1000)
#define FFMPEG_AAC_ADTS_LEN  (7)
#define FFMPEG_SAMPLE_RATE_NUM  (13)
#define FFMPEG_FRAME_BUF_DATA_LEN    (64 * 1024)
#define FFMPEG_DEFAULT_FPS (30.0)
/*define NULL handle for codedex check*/
#define HI_NULL_HANDLE (0)
#define HI_INVALID_HANDLE (-1)

#define CHECK_VALID_HANDLE(handle) \
    {\
        if(HI_INVALID_HANDLE == handle || HI_NULL_HANDLE == handle)\
        {\
            printf( "demux_handle:%d  is not valid \n", handle); \
            return HI_ERR_DEMUXER_INVALID_HANDLE;\
        }\
    }

#define FFDEMUX_LOCK(mutex) \
    do \
    {\
        (void)pthread_mutex_lock(&(mutex)); \
    }while(0)

#define FFDEMUX_UNLOCK(mutex) \
    do \
    {\
        (void)pthread_mutex_unlock(&(mutex));\
    }while(0)

static  pthread_mutex_t s_demuxMutex;
typedef struct hiFORMAT_MEMBER_S
{
    AVFormatContext*  pstFormatContext;
    AVPacket stPkt;
    HI_CHAR aszFilePath[PATH_MAX+1];
    HI_S32  s32VideoStreamIndex;
    HI_S32  s32AudioStreamIndex;
    HI_S32  s32DataIndex;
    AVStream* pstVideoStream;
    AVStream* pstAudioStream;
    AVStream* pstData;
    enum AVCodecID enVidCodecId;
    enum AVCodecID enAudCodecId;
    AVBitStreamFilterContext* pstH264StreamFilter;
    int64_t iLastReadPts;
    HI_U8 aszAudioData[FFMPEG_FRAME_BUF_DATA_LEN];
    HI_BOOL bHasAdts;
    HI_S64  s64FileSize;
} HI_FORMAT_MEMBER_S;

typedef enum hiFFMPEG_AAC_PROFILE_E
{
    AAC_PROFILE_MAIN = 0,
    AAC_PROFILE_LC,
    AAC_PROFILE_SSR,
    AAC_PROFILE_RESERVED,
} HI_FFMPEG_AAC_PROFILE_E;

typedef struct hiFFMPEG_ADTS_HEADER
{
    /* fixed */
    HI_U32  u32Sync;                           /* syncword */
    HI_U8   u8ID;                             /* MPEG bit - should be 1 */
    HI_U8   u8Layer;                          /* MPEG u8Layer - should be 0 */
    HI_U8   u8ProtectBit;                     /* 0 = CRC word follows, 1 = no CRC word */
    HI_U8   u8Profile;                        /* 0 = main, 1 = LC, 2 = SSR, 3 = reserved */
    HI_U8   u8SampRateIdx;                    /* sample rate index range = [0, 11] */
    HI_U8   u8PrivateBit;                     /* ignore */
    HI_U8   u8ChannelConfig;                  /* 0 = implicit, >0 = use default table */
    HI_U8   u8OrigCopy;                       /* 0 = copy, 1 = original */
    HI_U8   u8Home;                           /* ignore */

    /* variable */
    HI_U8   u8CopyBit;                        /* 1 bit of the 72-bit copyright ID (transmitted as 1 bit per frame) */
    HI_U8   u8CopyStart;                      /* 1 = this bit starts the 72-bit ID, 0 = it does not */
    HI_S32  s32FrameLength;                    /* length of frame */
    HI_S32  s32BufferFull;                     /* number of 32-bit words left in enc buffer, 0x7FF = VBR */
    HI_U8   u8NumRawDataBlocks;               /* number of raw data blocks in frame */

    /* CRC */
    HI_S32   s32CrcCheckWord;                   /* 16-bit CRC check word (present if u8ProtectBit == 0) */
} HI_FFMPEG_ADTS_HEADER_S;

static HI_U32 s_au32SampRateTab[FFMPEG_SAMPLE_RATE_NUM] =
{
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
};


static HI_VOID FFMPEG_GET_ADTSHeader(const HI_S32 sampleRateidx,             /*!< aacPlus sampling frequency (incl. SBR) */
                                     HI_S32     s32FrameLength,            /*!< raw data length */
                                     const HI_S32     s32Profile,                /* 0 = main, 1 = LC, 2 = SSR, 3 = reserved */
                                     const HI_S32     s32Ch,
                                     HI_U8*      pADTSHeadBuf)
{
    HI_S32  s32Head0 = 0, s32Head1 = 0;
    HI_FFMPEG_ADTS_HEADER_S stADTSHeader = {0};

    s32FrameLength += FFMPEG_AAC_ADTS_LEN;

    /* fixed fields - should not change from frame to frame */

    stADTSHeader.u32Sync =             0x0fff;          /* 12bit: */
    stADTSHeader.u8ID =               0;               /* 1bit: MPEG bit - should be 1 */
    stADTSHeader.u8Layer =            0;               /* 2bit: MPEG u8Layer - should be 0 */
    stADTSHeader.u8ProtectBit =       1;               /* 1bit: */
    stADTSHeader.u8Profile =          s32Profile;         /* 2bit: */
    stADTSHeader.u8SampRateIdx =      sampleRateidx;     /* 4bit: */
    stADTSHeader.u8PrivateBit =       0;               /* 1bit: */
    stADTSHeader.u8ChannelConfig =    s32Ch;              /* 3bit: */
    stADTSHeader.u8OrigCopy =         0;               /* 1bit: */
    stADTSHeader.u8Home =             0;               /* 1bit: */

    /* variable fields - can change from frame to frame */

    stADTSHeader.u8CopyBit =          0;              /* 1bit: */
    stADTSHeader.u8CopyStart =        0;              /* 1bit: */
    stADTSHeader.s32FrameLength =      s32FrameLength;    /* 13bit: */
    stADTSHeader.s32BufferFull =       0x07ff;         /* 11bit: */
    stADTSHeader.u8NumRawDataBlocks = 0;              /* 2bit: */

    s32Head0  = stADTSHeader.u32Sync       << (32 - 12);
    s32Head0 |= stADTSHeader.u8ID         << (32 - 12 - 1);
    s32Head0 |= stADTSHeader.u8Layer      << (32 - 12 - 1 - 2);
    s32Head0 |= stADTSHeader.u8ProtectBit << (32 - 12 - 1 - 2 - 1);

    s32Head0 |= stADTSHeader.u8Profile       << (32 - 12 - 1 - 2 - 1 - 2);
    s32Head0 |= stADTSHeader.u8SampRateIdx   << (32 - 12 - 1 - 2 - 1 - 2 - 4);
    s32Head0 |= stADTSHeader.u8PrivateBit    << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1);
    s32Head0 |= stADTSHeader.u8ChannelConfig << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3);
    s32Head0 |= stADTSHeader.u8OrigCopy      << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3 - 1);
    s32Head0 |= stADTSHeader.u8Home          << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3 - 1 - 1);
    s32Head0 |= stADTSHeader.u8CopyBit       << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3 - 1 - 1 - 1);
    s32Head0 |= stADTSHeader.u8CopyStart     << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3 - 1 - 1 - 1 - 1);
    s32Head0 |= ((stADTSHeader.s32FrameLength >> (13 - 2)) & 0x3);

    s32Head1  = stADTSHeader.s32FrameLength      << (32 - 13 + 2);
    s32Head1 |= stADTSHeader.s32BufferFull       << (32 - 13 + 2 - 11);
    s32Head1 |= stADTSHeader.u8NumRawDataBlocks << (32 - 13 + 2 - 11 - 2);

    pADTSHeadBuf[0] = (unsigned char)(s32Head0 >> 24) & 0xff;
    pADTSHeadBuf[1] = (unsigned char)(s32Head0 >> 16) & 0xff;
    pADTSHeadBuf[2] = (unsigned char)(s32Head0 >> 8) & 0xff;
    pADTSHeadBuf[3] = (unsigned char)(s32Head0 >> 0) & 0xff;

    pADTSHeadBuf[4] = (unsigned char)(s32Head1 >> 24) & 0xff;
    pADTSHeadBuf[5] = (unsigned char)(s32Head1 >> 16) & 0xff;
    pADTSHeadBuf[6] = (unsigned char)(s32Head1 >> 8) & 0xff;

    return;
}

static HI_BOOL FFMPEG_JUDGE_HasAdtsHeader(HI_U8* psrc, HI_U32 u32srcsize, HI_U8* pdst, HI_U32 u32dstsize)
{
    if (NULL == psrc
        || NULL == pdst
        || u32srcsize < FFMPEG_AAC_ADTS_LEN
        || u32dstsize < FFMPEG_AAC_ADTS_LEN)
    {
        return HI_FALSE;
    }

    /* adts fixed header
     * syncword: 12bits, fixed,      0xFFF
     * ID:       1bit,   non-fixed,  MPEG version, 0 for MPEG-4, 1 for MPEG-2.
     * ...
     * Compare 28bits is enough, and we must compare without 'ID', because it is non-fixed */

    if (psrc[0] != pdst[0]
        || (psrc[1] & 0xF7) != (pdst[1] & 0xF7)
        || psrc[2] != pdst[2]
        || (psrc[3] & 0xF0) != (pdst[3] & 0xF0))
    {
        return HI_FALSE;
    }

    return HI_TRUE;
}

static HI_S32  FFMPEG_FILTER_Packet(HI_FORMAT_MEMBER_S* pstFormatMember)
{
    AVPacket  stAVPacket = {0};

    HI_S32 s32Ret = 0;
    stAVPacket = pstFormatMember->stPkt;
    s32Ret = av_bitstream_filter_filter(pstFormatMember->pstH264StreamFilter, pstFormatMember->pstVideoStream->codec, NULL,
                                        &stAVPacket.data, &stAVPacket.size, pstFormatMember->stPkt.data, pstFormatMember->stPkt.size,
                                        pstFormatMember->stPkt.flags & AV_PKT_FLAG_KEY);

    if (s32Ret < 0)
    {
        printf( "H264 av_bitstream_filter_filter err size: %d pts: %lld s32Ret: %d\n",
            pstFormatMember->stPkt.size, pstFormatMember->stPkt.pts, s32Ret);
        av_packet_unref(&pstFormatMember->stPkt);
        /*filter err may come from that repaired file stream-end have invalid frames,
        so here just return EOF, ignore all frames at stream-end*/
        return HI_RET_FILE_EOF;
    }

    if (NULL == stAVPacket.buf)
    {
        printf( "H264 av_bitstream_filter_filter, receive packet buffer is NULL \n");
        av_packet_unref(&pstFormatMember->stPkt);
        return HI_FAILURE;
    }

    if (s32Ret >= 0)
    {
        if (0 == s32Ret)
        {
            if (stAVPacket.data != pstFormatMember->stPkt.data)
            {
                printf( "H264 av_bitstream_filter_filter return 0 while the receive addr is diff from send addr \n");
                av_packet_unref(&pstFormatMember->stPkt);
                return HI_FAILURE;
            }
        }

        av_packet_unref(&pstFormatMember->stPkt);
        stAVPacket.buf = av_buffer_create(stAVPacket.data, stAVPacket.size, av_buffer_default_free, NULL, 0);

        if (!stAVPacket.buf)
        {
            av_free(stAVPacket.data);
            stAVPacket.data = NULL;
            return AVERROR(ENOMEM);
        }
    }

    pstFormatMember->stPkt = stAVPacket;
    return HI_SUCCESS;
}

static HI_S32 FFMPEG_GET_Frame(HI_FORMAT_MEMBER_S* pstFormatMember)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* read frames from the file */
    s32Ret = av_read_frame(pstFormatMember->pstFormatContext, &pstFormatMember->stPkt);

    if (AVERROR_EOF == s32Ret)
    {
        printf( "read endof s32Ret: %d \n", s32Ret);
        return HI_RET_FILE_EOF;
    }
    else if (s32Ret < 0)
    {
        printf( "av_read_frame failed, ret:%d\n", s32Ret);
        return HI_ERR_DEMUXER_READ_PACKET;
    }

    if (pstFormatMember->stPkt.size <= 0)
    {
        printf( "HiffmpegDemux err: read stream len 0\n");
        return HI_ERR_DEMUXER_READ_PACKET;
    }

    return HI_SUCCESS;
}

static HI_S32 FFMPEG_SET_AudioInfo(HI_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    //get  adts header for aac aduio
    HI_U8  u8ADTSHeader[FFMPEG_AAC_ADTS_LEN] = {0};
    HI_U32 u32SampleRateIdx = 3;/* 3 means 48000 sample rate, we use it as the default */
    HI_U32 u32Profile = AAC_PROFILE_LC;
    HI_U32 u32Index = 0;
    HI_U32 u32Channel =  pstFormatMember->pstAudioStream->codec->channels;
    HI_BOOL bHasADTS = HI_FALSE;

    for ( u32Index = 0 ; u32Index < FFMPEG_SAMPLE_RATE_NUM ; u32Index++ )
    {
        if (s_au32SampRateTab[u32Index] == pstFormatMember->pstAudioStream->codec->sample_rate)
        {
            u32SampleRateIdx = u32Index;
            break;
        }
    }

    if (u32Index == FFMPEG_SAMPLE_RATE_NUM)
    {
        printf( " warning invalid s32SampleRate:%d,i guess as 48000HZ\n", pstFormatMember->pstAudioStream->codec->sample_rate);
    }

    /*
    0:Main profile
    1:Low Complexity profile(LC)
    2:Scalable Sampling Rate profile(SSR)
    3:(Reserved)
    */
    if (pstFormatMember->pstAudioStream->codec->profile < 0 || pstFormatMember->pstAudioStream->codec->profile >= 3)
    {
        u32Profile = AAC_PROFILE_LC;
//        printf(  " warning invalid profile:%d,i guess Low Complexity profile\n", pstFormatMember->pstAudioStream->codec->profile);
    }
    else
    {
        u32Profile = pstFormatMember->pstAudioStream->codec->profile;
    }

    FFMPEG_GET_ADTSHeader(u32SampleRateIdx, (pstFormatMember->stPkt.size), u32Profile, u32Channel, u8ADTSHeader);
    bHasADTS = FFMPEG_JUDGE_HasAdtsHeader(u8ADTSHeader, FFMPEG_AAC_ADTS_LEN, pstFormatMember->stPkt.data, pstFormatMember->stPkt.size);

    if (!bHasADTS)
    {
        if (pstFormatMember->stPkt.size + FFMPEG_AAC_ADTS_LEN > FFMPEG_FRAME_BUF_DATA_LEN)
        {
            printf( "audio data len beyond %d \n", FFMPEG_FRAME_BUF_DATA_LEN);
            return HI_FAILURE;
        }

        memset(pstFormatMember->aszAudioData, 0, FFMPEG_FRAME_BUF_DATA_LEN);
        memcpy(pstFormatMember->aszAudioData, u8ADTSHeader, FFMPEG_AAC_ADTS_LEN);
        memcpy(pstFormatMember->aszAudioData + FFMPEG_AAC_ADTS_LEN, pstFormatMember->stPkt.data, pstFormatMember->stPkt.size);
        pstFmtFrame->pu8Data = pstFormatMember->aszAudioData;
        pstFmtFrame->u32Size = pstFormatMember->stPkt.size + FFMPEG_AAC_ADTS_LEN;
    }
    else
    {
        pstFmtFrame->pu8Data = pstFormatMember->stPkt.data;
        pstFmtFrame->u32Size = pstFormatMember->stPkt.size;
    }

    pstFormatMember->bHasAdts = bHasADTS;
    pstFormatMember->iLastReadPts = av_rescale_q(pstFormatMember->stPkt.pts - pstFormatMember->pstAudioStream->start_time, pstFormatMember->pstAudioStream->time_base, AV_TIME_BASE_Q);
    return HI_SUCCESS;
}

static HI_S32 FFMPEG_SET_VideoInfo(HI_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if ((AV_CODEC_ID_H264 == pstFormatMember->enVidCodecId) || (AV_CODEC_ID_HEVC == pstFormatMember->enVidCodecId))
    {
        s32Ret = FFMPEG_FILTER_Packet(pstFormatMember);

        if (s32Ret != HI_SUCCESS)
        {
            printf( "exec FFMPEG_FILTER_Packet failed\n");
            return  s32Ret;
        }
    }

    //pkt.pts is time base on stream->timebase, need expressed in AV_TIME_BASE units
    pstFormatMember->iLastReadPts = av_rescale_q(pstFormatMember->stPkt.pts - pstFormatMember->pstVideoStream->start_time, pstFormatMember->pstVideoStream->time_base, AV_TIME_BASE_Q);
    pstFmtFrame->pu8Data = pstFormatMember->stPkt.data;
    pstFmtFrame->u32Size = pstFormatMember->stPkt.size;
    return HI_SUCCESS;
}

static HI_S32 FFMPEG_SET_DataInfo(HI_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    pstFmtFrame->pu8Data = pstFormatMember->stPkt.data;
    pstFmtFrame->u32Size = pstFormatMember->stPkt.size;

    pstFormatMember->iLastReadPts = av_rescale_q(pstFormatMember->stPkt.pts - pstFormatMember->pstData->start_time, pstFormatMember->pstData->time_base, AV_TIME_BASE_Q);
    return HI_SUCCESS;
}

static HI_S32 FFMPEG_GET_FileSize(HI_FORMAT_MEMBER_S* pstFormatMember, HI_S64*  ps64FileSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S64 s64FileSize = 0;
    FILE* pMediaFd = fopen(pstFormatMember->aszFilePath, "rb");

    if (NULL == pMediaFd)
    {
        printf( " pstFormatMember->aszFilePath:%s , err info:%s\n", pstFormatMember->aszFilePath, strerror(errno));
        return HI_FAILURE;
    }

    s32Ret = fseeko (pMediaFd, 0, SEEK_END);

    /*if error comes, fseek return  -1*/
    if (s32Ret == -1)
    {
        printf( " fseeko exec failed,  err info:%s\n", strerror(errno));
        fclose(pMediaFd);
        return HI_FAILURE;
    }

    s64FileSize = (HI_S64)ftello(pMediaFd);

    /*if error comes, ftell return  -1*/
    if (s64FileSize == -1)
    {
        printf( " ftello exec failed,  err info:%s\n", strerror(errno));
        fclose(pMediaFd);
        return HI_FAILURE;
    }

    *ps64FileSize = s64FileSize;
    fclose(pMediaFd);
    return HI_SUCCESS;
}


HI_S32 HI_FFMPEG_Open(HI_HANDLE* pFmtHandle, const HI_CHAR* pszFileName)
{
    HI_S32 s32Ret = HI_SUCCESS;
    AVFormatContext* pstFmtCtx = NULL;
    HI_FORMAT_MEMBER_S* pstFormatMember = (HI_FORMAT_MEMBER_S*)malloc(sizeof(HI_FORMAT_MEMBER_S));


    if (NULL ==  pstFormatMember)
    {
        printf( "malloc HI_FORMAT_MEMBER_S failed \n");
        return HI_ERR_DEMUXER_NULL_PTR;
    }
    memset(pstFormatMember, 0x00, sizeof(HI_FORMAT_MEMBER_S));

    pstFormatMember->s32AudioStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    pstFormatMember->s32VideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    pstFormatMember->s32DataIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    av_init_packet(&pstFormatMember->stPkt);

    /* register all formats and codecs */
    av_register_all();
    HI_S32 s32FileLen = strlen(pszFileName);

    if (s32FileLen < 1  || s32FileLen > PATH_MAX)
    {
        printf( "ffmpeg demux  file len is %d beyond [1,%d] \n", s32FileLen, PATH_MAX);
        free(pstFormatMember);
        pstFormatMember = NULL;
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    if(NULL == realpath(pszFileName, pstFormatMember->aszFilePath))
    {
        printf( "realpath %s ret failed errno: %d!\n", pszFileName, errno);
        free(pstFormatMember);
        pstFormatMember = NULL;
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    /* open input file, and allocate format context */
    s32Ret =  avformat_open_input(&pstFmtCtx, pstFormatMember->aszFilePath, NULL, NULL);

    if (s32Ret < 0)
    {
        printf( "Could not open source file %s, s32Ret:%d \n",
            pstFormatMember->aszFilePath, s32Ret);
        free(pstFormatMember);
        pstFormatMember = NULL;
        return HI_ERR_DEMUXER_OPEN_FILE;
    }


    pstFormatMember->pstFormatContext = pstFmtCtx;
    pthread_mutex_init(&s_demuxMutex, NULL);
    *pFmtHandle = (HI_HANDLE) pstFormatMember;
    printf( "input format: %s\n", pstFmtCtx->iformat->name);
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Read(HI_HANDLE fmtHandle, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    CHECK_VALID_HANDLE(fmtHandle);
    HI_FORMAT_MEMBER_S* pstFormatMember = (HI_FORMAT_MEMBER_S*)fmtHandle;

    if (NULL == pstFmtFrame)
    {
        printf( "pstFmtFrame is null \n");
        return HI_ERR_DEMUXER_NULL_PTR;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    HI_S32 s32Ret = HI_SUCCESS ;
    HI_BOOL bGetPaket = HI_FALSE;
    FFDEMUX_LOCK(s_demuxMutex);

    while (bGetPaket != HI_TRUE)
    {
        s32Ret = FFMPEG_GET_Frame(pstFormatMember);

        if (s32Ret != HI_SUCCESS)
        {
            FFDEMUX_UNLOCK(s_demuxMutex);
            return s32Ret;
        }

        if (pstFormatMember->stPkt.stream_index == pstFormatMember->s32AudioStreamIndex)
        {

            s32Ret = FFMPEG_SET_AudioInfo(pstFormatMember, pstFmtFrame);

            if (s32Ret != HI_SUCCESS)
            {
                printf( "FFMPEG_SET_AudioInfo exec failed \n");
                av_packet_unref(&pstFormatMember->stPkt);
                FFDEMUX_UNLOCK(s_demuxMutex);
                return HI_ERR_DEMUXER_READ_PACKET;
            }

        }
        else if (pstFormatMember->stPkt.stream_index ==  pstFormatMember->s32VideoStreamIndex)
        {
            s32Ret = FFMPEG_SET_VideoInfo(pstFormatMember, pstFmtFrame);

            if(s32Ret == HI_RET_FILE_EOF)
            {
                av_packet_unref(&pstFormatMember->stPkt);
                FFDEMUX_UNLOCK(s_demuxMutex);
                return s32Ret;
            }
            else if (s32Ret != HI_SUCCESS)
            {
                printf( "FFMPEG_SET_VideoInfo exec failed \n");
                av_packet_unref(&pstFormatMember->stPkt);
                FFDEMUX_UNLOCK(s_demuxMutex);
                return HI_ERR_DEMUXER_READ_PACKET;
            }

        }
        else if (pstFormatMember->stPkt.stream_index ==  pstFormatMember->s32DataIndex)
        {
            s32Ret = FFMPEG_SET_DataInfo(pstFormatMember, pstFmtFrame);

            if (s32Ret != HI_SUCCESS)
            {
                printf( "FFMPEG_SET_DataInfo exec failed \n");
                av_packet_unref(&pstFormatMember->stPkt);
                FFDEMUX_UNLOCK(s_demuxMutex);
                return HI_ERR_DEMUXER_READ_PACKET;
            }

        }
        else
        {
            printf( "have other stream in input file, just ignore it\n");
            continue;
        }

        bGetPaket = HI_TRUE;
    }

    pstFmtFrame->s64Pts = pstFormatMember->iLastReadPts / 1000;
    pstFmtFrame->s64Dts = pstFormatMember->stPkt.dts;
    pstFmtFrame->bKeyFrame = (AV_PKT_FLAG_KEY == (AV_PKT_FLAG_KEY & pstFormatMember->stPkt.flags)) ? HI_TRUE : HI_FALSE;
    pstFmtFrame->u32StreamIndex = pstFormatMember->stPkt.stream_index;
    pstFmtFrame->s64Duration = pstFormatMember->stPkt.duration;
    pstFmtFrame->s64Pos = pstFormatMember->stPkt.pos;
    pstFmtFrame->pu8Header = pstFormatMember->pstVideoStream->codec->extradata;
    pstFmtFrame->u32HeaderLen = pstFormatMember->pstVideoStream->codec->extradata_size;
    FFDEMUX_UNLOCK(s_demuxMutex);
    return HI_SUCCESS;
}


HI_S32 HI_FFMPEG_Close(HI_HANDLE fmtHandle)
{
    CHECK_VALID_HANDLE(fmtHandle);
    HI_FORMAT_MEMBER_S* pstFormatMember = (HI_FORMAT_MEMBER_S*)fmtHandle;

    FFDEMUX_LOCK(s_demuxMutex);
    avformat_close_input(&pstFormatMember->pstFormatContext);

    if (pstFormatMember->pstH264StreamFilter)
    {
        av_bitstream_filter_close(pstFormatMember->pstH264StreamFilter);
    }

    free(pstFormatMember);
    pstFormatMember = NULL;
    FFDEMUX_UNLOCK(s_demuxMutex);
    pthread_mutex_destroy(&(s_demuxMutex));
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Free(HI_HANDLE fmtHandle, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    CHECK_VALID_HANDLE(fmtHandle);
    HI_FORMAT_MEMBER_S* pstFormatMember = (HI_FORMAT_MEMBER_S*)fmtHandle;

    if (NULL == pstFmtFrame)
    {
        printf( "pstFmtFrame is null \n");
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    FFDEMUX_LOCK(s_demuxMutex);

    if ( NULL == pstFormatMember->stPkt.data )
    {
        printf( "you should call fmt_read first\n");
        FFDEMUX_UNLOCK(s_demuxMutex);
        return HI_ERR_DEMUXER_FREE_PACKET;
    }

    if (! (HI_FALSE == pstFormatMember->bHasAdts && pstFormatMember->stPkt.stream_index == pstFormatMember->s32AudioStreamIndex))
    {
        if (pstFormatMember->stPkt.data != pstFmtFrame->pu8Data )
        {
            printf(  "last read packet is not equal to the packet which is going to be freed \n");
        }
    }

    av_packet_unref(&pstFormatMember->stPkt);
    FFDEMUX_UNLOCK(s_demuxMutex);
    return HI_SUCCESS;

}

HI_S32 HI_FFMPEG_Seek(HI_HANDLE fmtHandle, HI_S32 s32StreamIndex, HI_S64 s64MSec)
{
    CHECK_VALID_HANDLE(fmtHandle);
    HI_FORMAT_MEMBER_S* pstFormatMember = (HI_FORMAT_MEMBER_S*)fmtHandle;

    if (HI_DEMUXER_NO_MEDIA_STREAM == s32StreamIndex)
    {
        printf( "s32StreamIndex  is  %d \n", s32StreamIndex);
        return  HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S64 s64TimeUs = ((HI_S64)s64MSec * 1000);
    /*s64TimeUs may oversize s64 to be negative value*/
    if ( s64MSec < 0 || s64TimeUs < 0 )
    {
        printf( "seek input time is negative or oversize: %lld ms\n", s64MSec);
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    if (s64TimeUs > pstFormatMember->pstFormatContext->duration)
    {
        printf( "seek input time beyond total time seektime: %lld, total time:%lld\n", s64TimeUs, pstFormatMember->pstFormatContext->duration);
        s64TimeUs = pstFormatMember->pstFormatContext->duration;
    }

    FFDEMUX_LOCK(s_demuxMutex);
    HI_S32 s32SeekTotalSec, s32SeekHour, s32SeekMin, s32SeekSec;
    HI_S32 s32FileTotalSec, s32FileHour, s32FileMin, s32FileSec;

    /* translate microsecond to hour:min:sec format */
    s32FileTotalSec  = pstFormatMember->pstFormatContext->duration / 1000000LL;
    s32FileHour  = s32FileTotalSec / 3600;
    s32FileMin  = (s32FileTotalSec % 3600) / 60;
    s32FileSec  = (s32FileTotalSec % 60);

    s32SeekTotalSec = s64TimeUs / 1000000LL;
    s32SeekHour = s32SeekTotalSec / 3600;
    s32SeekMin = (s32SeekTotalSec % 3600) / 60;
    s32SeekSec = (s32SeekTotalSec % 60);

    if (pstFormatMember->pstFormatContext->start_time != AV_NOPTS_VALUE)
    {
        s64TimeUs += pstFormatMember->pstFormatContext->start_time;
    }

    //we use one second to ensure thar the avformat_seek_file interface can find the key frame beyond s64SeekMin and s64SeekMax
    HI_S64 s64SeekTarget = s64TimeUs;
    printf( "Seek to %lld (%2d:%02d:%02d) of total duration %lld (%2d:%02d:%02d)    starttime: %lld\n",
                  s64TimeUs, s32SeekHour, s32SeekMin, s32SeekSec, pstFormatMember->pstFormatContext->duration, s32FileHour, s32FileMin, s32FileSec,
                  pstFormatMember->pstFormatContext->start_time);

    s32Ret = avformat_seek_file(pstFormatMember->pstFormatContext, -1,  INT64_MIN, s64SeekTarget, s64SeekTarget, 0);
    if (s32Ret < 0)
    {
        s32Ret = avformat_seek_file(pstFormatMember->pstFormatContext, -1,  s64SeekTarget, s64SeekTarget, INT64_MAX, 0);
        if (s32Ret < 0)
        {
            printf( "%s: error while seeking %lld duration: %lld,streamIndex:%d, s32Ret:%d\n",
                pstFormatMember->pstFormatContext->filename, s64MSec, pstFormatMember->pstFormatContext->duration, s32StreamIndex, s32Ret);
            FFDEMUX_UNLOCK(s_demuxMutex);
            return HI_ERR_DEMUXER_SEEK;
        }
    }

    //pkt.pts is time base on stream->timebase, need expressed in AV_TIME_BASE units
    pstFormatMember->iLastReadPts = s64SeekTarget;
    FFDEMUX_UNLOCK(s_demuxMutex);
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Probe(HI_HANDLE fmtHandle)
{
    CHECK_VALID_HANDLE(fmtHandle);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_FORMAT_MEMBER_S* pstFormatMember = (HI_FORMAT_MEMBER_S*)fmtHandle;
    FFDEMUX_LOCK(s_demuxMutex);

    /* retrieve stream information */
    if (avformat_find_stream_info(pstFormatMember->pstFormatContext, NULL) < 0)
    {
        printf( "Could not find stream information \n");
        FFDEMUX_UNLOCK(s_demuxMutex);
        return HI_ERR_DEMUXER_PROBE;
    }

    s32Ret = av_find_best_stream(pstFormatMember->pstFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    if (s32Ret < 0)
    {
        printf( "Could not find video stream in input file '%s', ret:%d \n", pstFormatMember->aszFilePath, s32Ret);
        pstFormatMember->s32VideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstFormatMember->pstVideoStream = NULL;
    }
    else
    {
        pstFormatMember->pstVideoStream = pstFormatMember->pstFormatContext->streams[s32Ret];
        pstFormatMember->s32VideoStreamIndex = s32Ret;
        pstFormatMember->enVidCodecId = pstFormatMember->pstVideoStream->codec->codec_id;
        printf( "video codec type  %s\n", avcodec_get_name(pstFormatMember->enVidCodecId));


        if ((AV_CODEC_ID_H264 == pstFormatMember->enVidCodecId) || (AV_CODEC_ID_HEVC == pstFormatMember->enVidCodecId))
        {
            if(AV_CODEC_ID_H264 == pstFormatMember->enVidCodecId) {
                pstFormatMember->pstH264StreamFilter = av_bitstream_filter_init("h264_mp4toannexb");
            }
            else if(AV_CODEC_ID_HEVC == pstFormatMember->enVidCodecId) {
                pstFormatMember->pstH264StreamFilter = av_bitstream_filter_init("hevc_mp4toannexb");
            }

            if (NULL == pstFormatMember->pstH264StreamFilter)
            {
                printf( "av_bitstream_filter_init h264_mp4toannexb failed\n");
                FFDEMUX_UNLOCK(s_demuxMutex);
                return HI_ERR_DEMUXER_PROBE;
            }

            uint8_t* pOutBuf = NULL;
            HI_S32 outBufSize = 0;
            //only parse extra data to get sps and pps
            av_bitstream_filter_filter(pstFormatMember->pstH264StreamFilter , pstFormatMember->pstVideoStream->codec, NULL, &pOutBuf, &outBufSize, NULL, 0, 0);
        }

        printf( "video timebase: %d %d \n", pstFormatMember->pstVideoStream->time_base.den,  pstFormatMember->pstVideoStream->time_base.num);
    }

    s32Ret = av_find_best_stream(pstFormatMember->pstFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    if (s32Ret < 0)
    {
        printf( "Could not find audio stream in input file '%s', ret:%d\n", pstFormatMember->aszFilePath, s32Ret);
        pstFormatMember->s32AudioStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstFormatMember->pstAudioStream = NULL;
    }
    else
    {
        pstFormatMember->pstAudioStream = pstFormatMember->pstFormatContext->streams[s32Ret];
        pstFormatMember->s32AudioStreamIndex = s32Ret;
        pstFormatMember->enAudCodecId = pstFormatMember->pstAudioStream->codec->codec_id;
        printf( "audio codec type  %s\n", avcodec_get_name(pstFormatMember->enAudCodecId));
        printf( "audio timebase: %d %d \n", pstFormatMember->pstAudioStream->time_base.den, pstFormatMember->pstAudioStream->time_base.num);
    }

    s32Ret = av_find_best_stream(pstFormatMember->pstFormatContext, AVMEDIA_TYPE_DATA, -1, -1, NULL, 0);

    if (s32Ret < 0)
    {
        printf( "Could not find data in input file '%s', ret:%d\n", pstFormatMember->aszFilePath, s32Ret);
        pstFormatMember->s32DataIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstFormatMember->pstData = NULL;
    }
    else
    {
        pstFormatMember->pstData = pstFormatMember->pstFormatContext->streams[s32Ret];
        pstFormatMember->s32DataIndex = s32Ret;
    }

    /* dump input information to stderr */
    av_dump_format(pstFormatMember->pstFormatContext, 0, pstFormatMember->aszFilePath, 0);

    HI_S64 s64FileSize = 0;

    if (FFMPEG_GET_FileSize(pstFormatMember, &s64FileSize) != HI_SUCCESS)
    {
        printf( "FFMPEG_GET_FileSize exec failed \n");
        FFDEMUX_UNLOCK(s_demuxMutex);
        return  HI_ERR_DEMUXER_OPEN_FILE;
    }

    pstFormatMember->s64FileSize = s64FileSize;
    printf( "get out fmt_probe.\n");
    FFDEMUX_UNLOCK(s_demuxMutex);
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Getinfo(HI_HANDLE fmtHandle, HI_FORMAT_FILE_INFO_S* pstFmtInfo)
{
    CHECK_VALID_HANDLE(fmtHandle);
    HI_FORMAT_MEMBER_S* pstFormatMember = (HI_FORMAT_MEMBER_S*)fmtHandle;
    HI_S32 s32Index = 0;

    if (NULL == pstFmtInfo)
    {
        printf( "pstFmtInfo is null \n");
        return HI_ERR_DEMUXER_NULL_PTR;
    }

    FFDEMUX_LOCK(s_demuxMutex);

    if (HI_DEMUXER_NO_MEDIA_STREAM ==  pstFormatMember->s32AudioStreamIndex && HI_DEMUXER_NO_MEDIA_STREAM == pstFormatMember->s32VideoStreamIndex)
    {
        printf( "should call fmt_probe first \n");
        FFDEMUX_UNLOCK(s_demuxMutex);
        return  HI_ERR_DEMUXER_ACTION;
    }

    if (pstFormatMember->s32VideoStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM
        && pstFormatMember->pstVideoStream)
    {
        pstFmtInfo->stSteamResolution[0].u32Width =  pstFormatMember->pstVideoStream->codec->width;
        pstFmtInfo->stSteamResolution[0].u32Height =  pstFormatMember->pstVideoStream->codec->height;
        pstFmtInfo->stSteamResolution[0].s32VideoStreamIndex = pstFormatMember->s32VideoStreamIndex;
        pstFmtInfo->s32UsedVideoStreamIndex = pstFormatMember->s32VideoStreamIndex;
        /*if video file duration short than 1s, den will be 0, use default fps 30*/
        AVRational stFrameRate = pstFormatMember->pstVideoStream->avg_frame_rate;

        pstFmtInfo->fFrameRate = (stFrameRate.den && stFrameRate.num) ?
            (HI_FLOAT)stFrameRate.num/(HI_FLOAT)stFrameRate.den : FFMPEG_DEFAULT_FPS;

        /*this demux lib will only deal with AV_CODEC_ID_H264/HI_FFMPEG_VIDEO_TYPE_H265 */
        if (AV_CODEC_ID_H264 == pstFormatMember->pstVideoStream->codec->codec_id)
        {
            pstFmtInfo->enVideoType = HI_FORMAT_VIDEO_TYPE_H264;
        }
        else if (AV_CODEC_ID_HEVC == pstFormatMember->pstVideoStream->codec->codec_id)
        {
            pstFmtInfo->enVideoType = HI_FORMAT_VIDEO_TYPE_H265;
        }
        else
        {
            pstFmtInfo->enVideoType = HI_FORMAT_VIDEO_TYPE_BUTT;
        }
    }
    else
    {
        pstFmtInfo->stSteamResolution[0].u32Width =  0;
        pstFmtInfo->stSteamResolution[0].u32Height =  0;
        pstFmtInfo->stSteamResolution[0].s32VideoStreamIndex = pstFormatMember->s32VideoStreamIndex;
        pstFmtInfo->fFrameRate = 0;
        pstFmtInfo->enVideoType = HI_FORMAT_VIDEO_TYPE_BUTT;
        pstFmtInfo->s32UsedVideoStreamIndex = pstFormatMember->s32VideoStreamIndex;
    }

    if(pstFmtInfo->s32UsedVideoStreamIndex >= HI_DEMUXER_RESOLUTION_CNT)
    {
        printf( "select videoidx: %d exceed max support %d \n",
            pstFmtInfo->s32UsedVideoStreamIndex, HI_DEMUXER_RESOLUTION_CNT-1);
        FFDEMUX_UNLOCK(s_demuxMutex);
        return  HI_ERR_DEMUXER_ACTION;
    }


    for (s32Index = 1; s32Index < HI_DEMUXER_RESOLUTION_CNT; s32Index++)
    {
        pstFmtInfo->stSteamResolution[s32Index].u32Width = 0;
        pstFmtInfo->stSteamResolution[s32Index].u32Height = 0;
        pstFmtInfo->stSteamResolution[s32Index].s32VideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    }

    // we just consider only have one  audio stream scene
    pstFmtInfo->s32UsedAudioStreamIndex = pstFormatMember->s32AudioStreamIndex;
    pstFmtInfo->u32SampleRate = 0;
    pstFmtInfo->u32AudioChannelCnt = 0;

    if (pstFormatMember->s32AudioStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM
        && pstFormatMember->pstAudioStream)
    {
        pstFmtInfo->u32SampleRate = pstFormatMember->pstAudioStream->codec->sample_rate;
        pstFmtInfo->u32AudioChannelCnt = pstFormatMember->pstAudioStream->codec->channels;
    }

    // MP4 data info
    pstFmtInfo->s32UsedDataIndex = pstFormatMember->s32DataIndex;

    pstFmtInfo->s64Duration  = pstFormatMember->pstFormatContext->duration / FFMPEG_TIME_UNIT;
    pstFmtInfo->u32Bitrate   = pstFormatMember->pstFormatContext->bit_rate;
    pstFmtInfo->s64StartTime =  pstFormatMember->pstFormatContext->start_time;
    pstFmtInfo->s64FileSize = pstFormatMember->s64FileSize;
    FFDEMUX_UNLOCK(s_demuxMutex);
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Setattr(HI_HANDLE fmtHandle, HI_FORMAT_MEDIA_ATTR_S* pstFmtInfo)
{
    printf( "ffmpeg format not support setAttr \n");
    return HI_ERR_DEMUXER_NOT_SUPPORT;
}

/** ffmpeg demuxer symbol */
HI_DEMUX_S   g_stFormat_entry =
{
    .aszDemuxerName = "ffmpeg_demuxer",
    .aszSupportFormat = "mp4,lrv,mov",
    .u32Priority = 2,
    .stFmtFun.fmt_open       = HI_FFMPEG_Open,
    .stFmtFun.fmt_getinfo    = HI_FFMPEG_Getinfo,
    .stFmtFun.fmt_read       = HI_FFMPEG_Read,
    .stFmtFun.fmt_free       = HI_FFMPEG_Free,
    .stFmtFun.fmt_close      = HI_FFMPEG_Close,
    .stFmtFun.fmt_probe   = HI_FFMPEG_Probe,
    .stFmtFun.fmt_seek   = HI_FFMPEG_Seek,
    .stFmtFun.fmt_setattr   = HI_FFMPEG_Setattr,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
