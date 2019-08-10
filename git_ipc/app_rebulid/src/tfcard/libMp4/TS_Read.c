
#include "mpeg_ps.h"
#include "mpeg_ts.h"
#include "mpeg_ts_proto.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "TS_Read.h"


typedef struct tagTSREAD_
{
    struct ts_demuxer_t* hTsDemuxer;
    FILE*   pFile;
    //FILE*   pWrite;
    int     nState;////0:??,1:??

    void*   pDataBuf;
    int     nEncType;
    int     bFrameKey;
    uint32_t nBufSize;
    uint64_t ptsUs;
    uint32_t nDataBufSize;  // 对应pDataBuf开辟的内存大小
}stTSREAD, *pstTSREAD;

#define TSREAD_AV_MAX_LEN   (256*1024)

inline const char* ftimestamp(int64_t t, char* buf)
{
    if (PTS_NO_VALUE == t)
    {
        sprintf(buf, "(null)");
    }
    else
    {
        t /= 90;
        sprintf(buf, "%2d:%02d:%02d.%03d", (int)(t / 36000000), (int)((t / 60000) % 60), (int)((t / 1000) % 60), (int)(t % 1000));
    }
    return buf;
}

static int64_t g_s_pts = 0;
static void on_ts_packet(void* param, int stream, int avtype, int flags, int64_t pts, int64_t dts, const void* data, size_t bytes)
{
    static int64_t backup_dts = 0;
    pstTSREAD pstRead = (pstTSREAD)param;
    if(NULL == pstRead || NULL == data)
    {
        printf("[%s:%d]ts packet NULL\n",__func__,__LINE__);
        return ;
    }
    
//  printf("pts=%lld,dts=%lld,g_s_pts=%lld\n",pts,dts,g_s_pts);
    if (0 == g_s_pts)
    {
        g_s_pts = pts;
    }

    dts = (dts > 0 && dts >= g_s_pts) ? (dts-g_s_pts):backup_dts;
    pts = (pts > 0 && pts >= g_s_pts) ? (pts-g_s_pts):backup_dts;

    if(pstRead->nDataBufSize < bytes)
    {
        void *bufTmp = NULL;
        bufTmp = calloc(1, sizeof(char) * bytes);
        if(NULL != bufTmp)
        {
            if(NULL != pstRead->pDataBuf)
            {
                free(pstRead->pDataBuf);
                pstRead->pDataBuf = NULL;
            }
            pstRead->pDataBuf = bufTmp;
            pstRead->nDataBufSize = bytes;
            printf("%s:%d realloc size %d\n", __FUNCTION__, __LINE__, bytes);
        }
        else
        {
            bytes = pstRead->nDataBufSize;
            printf("%s:%d realloc failed!!! frame size limit size %d\n", __FUNCTION__, __LINE__, bytes);
        }
    }

    if (PSI_STREAM_AAC == avtype)
    {
        memcpy(pstRead->pDataBuf, data, bytes);
        pstRead->nBufSize = bytes;
        pstRead->ptsUs = pts / 90;
        pstRead->bFrameKey = flags;
        pstRead->nEncType = TS_STREAMTYPE_AAC_AUDIO;
        pstRead->nState = 1;
        backup_dts = dts;
    }
    else if (PSI_STREAM_H264 == avtype)
    {
        memcpy(pstRead->pDataBuf, data, bytes);
        pstRead->nBufSize = bytes;
        pstRead->ptsUs = pts / 90;
        pstRead->bFrameKey = flags;
        pstRead->nEncType = TS_STREAMTYPE_H264_VIDEO;
        pstRead->nState = 1;
        backup_dts = dts;

    }
    else if (PSI_STREAM_H265 == avtype)
    {
        memcpy(pstRead->pDataBuf, data, bytes);
        pstRead->nBufSize = bytes;
        pstRead->ptsUs = pts / 90;
        pstRead->bFrameKey = flags;
        pstRead->nEncType = TS_STREAMTYPE_H265_VIDEO;
        pstRead->nState = 1;
        backup_dts = dts;

    }
    else
    {
        printf("[%s:%d]ts packet data error avtype=%d,flags:%d\n",__func__,__LINE__,avtype,flags);
        //assert(0);
    }
}



HTSREAD TSREAD_Open(char* pstrFile)
{
    FILE* pFile = NULL;
    pstTSREAD pstRead = NULL;
    if(NULL == pstrFile)
    {
        return NULL;
    }

    pFile = fopen(pstrFile, "rb");
    if(NULL == pFile)
    {
        return NULL;
    }
    pstRead = (pstTSREAD)malloc(sizeof(stTSREAD));
    if(NULL == pstRead)
    {
        return NULL;
    }
    memset(pstRead, 0 , sizeof(stTSREAD));
    pstRead->pFile = pFile;

    printf("tsread open %s ok!\n", pstrFile);
    pstRead->hTsDemuxer = ts_demuxer_create(on_ts_packet, pstRead);
    if(NULL == pstRead->hTsDemuxer)
    {
        goto ERROR;
    }
    pstRead->pDataBuf = calloc(1, sizeof(char)*TSREAD_AV_MAX_LEN);
    pstRead->nDataBufSize = TSREAD_AV_MAX_LEN;

    if(NULL == pstRead->pDataBuf)
    {
        goto ERROR;
    }
    g_s_pts = 0;
    return (HTSREAD)pstRead;

ERROR:

    if(NULL != pstRead->hTsDemuxer)
    {
        //ts_demuxer_flush(pstRead->hTsDemuxer);
        ts_demuxer_destroy(pstRead->hTsDemuxer);
    }
    if(NULL != pstRead->pFile)
    {
        fclose(pstRead->pFile);
    }
    if(NULL != pstRead->pDataBuf)
    {
        free(pstRead->pDataBuf);
    }
    if(NULL != pstRead)
    {
        free(pstRead);
    }
    return NULL;
}

int TSREAD_Close(HTSREAD hRead)
{
    pstTSREAD pstRead = (pstTSREAD)hRead;
    if(NULL == pstRead)
    {
        return -1;
    }

    if(NULL != pstRead->hTsDemuxer)
    {
        
        //ts_demuxer_flush(pstRead->hTsDemuxer);
        ts_demuxer_destroy(pstRead->hTsDemuxer);
    }
    if(NULL != pstRead->pFile)
    {
    
        fclose(pstRead->pFile);
    }
    if(NULL != pstRead->pDataBuf)
    {
        free(pstRead->pDataBuf);
        pstRead->nDataBufSize = 0;
        pstRead->pDataBuf = NULL;
    }
    if(NULL != pstRead)
    {
        free(pstRead);
        pstRead = NULL;
    }
    return 0;
}
int TSREAD_GetDescrip(HTSREAD hRead,pstFILEDESCIP pFileDescip)
{
    return 0;
}

int TSREAD_Read(HTSREAD hRead, void** buf, uint32_t* bufSize, int* frameType, uint64_t* ptsUs, int* iskey)
{
    unsigned char ptr[188] = {0};
    int nRet = 0;
    int nReadRet = 0;
    size_t nLenInput = 0;
    pstTSREAD pstRead = (pstTSREAD)hRead;
    if( NULL == pstRead ||
        NULL == pstRead->hTsDemuxer ||
        NULL == pstRead->pFile)
    {
        return EN_TSREAD_ERR_FAILT;
    }
    pstRead->nState = 0;
    
    while (1)
    {
        
        nReadRet = fread(ptr, sizeof(ptr), 1, pstRead->pFile);
        
        if(1 != nReadRet)
        {       
            printf("[%s:%d]Read file failure:%d\n",__func__,__LINE__,nReadRet);
            nRet = EN_TSREAD_ERR_EOF;
            break;
        }

        nLenInput = ts_demuxer_input(pstRead->hTsDemuxer, ptr, sizeof(ptr));
        if(1 == nLenInput)
        {           
            printf("[%s:%d]Frame failure to point to the next file\n",__func__,__LINE__);
            nRet = EN_TSREAD_ERR_EOF;
            break;
        }
        if(0 > nLenInput)
        {
            printf("[%s:%d]Frame failure to point to the next frame\n",__func__,__LINE__);
            continue;
        }

        if(1 == pstRead->nState && 0 == nLenInput)
        {
            if((TS_STREAMTYPE_H264_VIDEO == pstRead->nEncType)
                || (TS_STREAMTYPE_H265_VIDEO == pstRead->nEncType))
            {
                *buf = pstRead->pDataBuf;
                *bufSize = pstRead->nBufSize;
                *frameType = pstRead->nEncType;
                *ptsUs = pstRead->ptsUs;
                *iskey = pstRead->bFrameKey;
            }
#if 1
            if(TS_STREAMTYPE_AAC_AUDIO == pstRead->nEncType)
            {
                *buf = pstRead->pDataBuf;
                *bufSize = pstRead->nBufSize;
                *frameType = pstRead->nEncType;
                *ptsUs = pstRead->ptsUs;
                *iskey = pstRead->bFrameKey;
            }
#endif
            nRet = EN_TSREAD_ERR_OK;
            break;
        }
    }
    return nRet;
}


