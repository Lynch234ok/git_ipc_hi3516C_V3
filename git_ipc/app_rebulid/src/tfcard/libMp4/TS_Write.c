
#include "mpeg_ps.h"
#include "mpeg_ts.h"
#include "mpeg_ts_proto.h"

#include "TS_Write.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>


static void* ts_alloc(void* param, size_t bytes)
{
	static char s_buffer[188];
	assert(bytes <= sizeof(s_buffer));
	return s_buffer;
}

static void ts_free(void* param, void* packet)
{
	return;
}

static void ts_write(void* param, const void* packet, size_t bytes)
{
	fwrite(packet, bytes, 1, (FILE*)param);
}


inline const char* ts_type(int type)
{
	switch (type)
	{
		case PSI_STREAM_MP3: return "MP3";
		case PSI_STREAM_AAC: return "AAC";
		case PSI_STREAM_H264: return "H264";
		case PSI_STREAM_H265: return "H265";
		default: return "*";
	}
}

typedef struct tagTSWRITE{
	FILE* pFile;
	void* hMpegts;
	int	  nAudioId;
	int	  nVideoId;
}stTSWRITE, *pstTSWRITE;


HTSWRITE TSWRITE_Open(char* pstrFileName, enTSENCTYPE nVencType)
{
	if(NULL == pstrFileName)
	{
		return NULL;
	}
	pstTSWRITE pTsWrite = (pstTSWRITE)malloc(sizeof(stTSWRITE));
	if(NULL == pTsWrite)
	{
		return NULL;
	}
	memset(pTsWrite, 0 , sizeof(stTSWRITE));

	struct mpeg_ts_func_t tshandler;
	tshandler.alloc = ts_alloc;
	tshandler.write = ts_write;
	tshandler.free = ts_free;

	pTsWrite->pFile = fopen(pstrFileName, "wb");
	if(NULL == pTsWrite->pFile)
	{
		goto ERROR;
	}

	pTsWrite->hMpegts = mpeg_ts_create(&tshandler, pTsWrite->pFile);
	if(NULL == pTsWrite->hMpegts)
	{
		goto ERROR;
	}
	pTsWrite->nAudioId = mpeg_ts_add_stream(pTsWrite->hMpegts, STREAM_AUDIO_AAC, NULL, 0);
	if(EN_MPEG_TS_H265 == nVencType)
	{
        pTsWrite->nVideoId = mpeg_ts_add_stream(pTsWrite->hMpegts, STREAM_VIDEO_H265, NULL, 0);
	}
	else if(EN_MPEG_TS_H264 == nVencType)
	{
        pTsWrite->nVideoId = mpeg_ts_add_stream(pTsWrite->hMpegts, STREAM_VIDEO_H264, NULL, 0);
	}

	return (HTSWRITE)pTsWrite;

ERROR:

	if(NULL != pTsWrite->hMpegts)
	{
		mpeg_ts_destroy(pTsWrite->hMpegts);
	}
	if(NULL != pTsWrite->pFile)
	{
		fclose(pTsWrite->pFile);
	}
	if(NULL != pTsWrite)
	{
		free(pTsWrite);
	}
	return NULL;
}

int TSWRITE_SetDescrip(HTSWRITE hRecord,int nPicWSize, int nPicHSize, int nBitRate, int nFrameRate)
{
	return 0;
}

int TSWRITE_Close(HTSWRITE hRecord)
{
	pstTSWRITE pTsWrite = (pstTSWRITE)hRecord;
	if(NULL == pTsWrite)
	{
		return -1;
	}
	if(NULL != pTsWrite->hMpegts)
	{
		mpeg_ts_destroy(pTsWrite->hMpegts);
	}
	if(NULL != pTsWrite->pFile)
	{
		fflush(pTsWrite->pFile);
		fsync(fileno(pTsWrite->pFile));
		fclose(pTsWrite->pFile);
	}
	free(pTsWrite);
	pTsWrite = NULL;
	return 0;
}

int TSWRITE_Write(HTSWRITE hRecord, enTSENCTYPE nFrameType, unsigned char* pFrameBuf,	int nFrameSize,	uint64_t TimeStamp, enTSSLICETYPE nSliceType)
{
	int nRet = 0;
	int stream_pid = 0;
	pstTSWRITE pstWrite = (pstTSWRITE)hRecord;
	if(NULL == pstWrite)
	{
		return EN_TSWRITE_ERR_FAILT;
	}

	int flags = (EN_SLICE_TYPE_I == nSliceType) ? 1 : 0;
	int64_t pts = TimeStamp *90;
	int64_t dts = TimeStamp *90;

	switch(nFrameType)
	{
		case EN_MPEG_TS_H264:
		case EN_MPEG_TS_H265:
			{
			    if( 0 > mpeg_ts_write(pstWrite->hMpegts, pstWrite->nVideoId,
						flags, pts, dts, pFrameBuf, (size_t)nFrameSize))
			    {
			    	return EN_TSWRITE_ERR_FAILT;
			    }
			}
			break;
		case EN_MPEG_TS_AAC:
			{
				if( 0 > mpeg_ts_write(pstWrite->hMpegts, pstWrite->nAudioId,
						flags, pts, dts, pFrameBuf, (size_t)nFrameSize))
				{
					return EN_TSWRITE_ERR_FAILT;
				}
			}
			break;
		default:
			return EN_TSWRITE_ERR_TYPE;
	}

	return EN_TSWRITE_ERR_SUCCESS;
}

int TSWRITE_Sync(HTSWRITE hRecord)
{
    pstTSWRITE pTsWrite = (pstTSWRITE)hRecord;
    if(NULL == pTsWrite)
    {
        return -1;
    }

    if(NULL != pTsWrite->pFile)
    {
        fsync(fileno(pTsWrite->pFile));
    }

	return 0;

}

