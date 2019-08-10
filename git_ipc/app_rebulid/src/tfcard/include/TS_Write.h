
#ifndef _TS_WRITE_H_
#define _TS_WRITE_H_


#ifdef __cplusplus
extern "C"{
#endif

typedef  void* HTSWRITE;

typedef enum tagTSENCTYPE_
{
    EN_MPEG_TS_INVAILD = -1,
    EN_MPEG_TS_AUDIO   = 0,
    EN_MPEG_TS_VIDEO   = 1,
    EN_MPEG_TS_H265    = 2,
    EN_MPEG_TS_H264    = 3,
    EN_MPEG_TS_AAC     = 4,
    EN_MPEG_TS_WAV     = 5,
	EN_MPEG_TS_G711A   = 6,
    EN_MPEG_TS_G711U   = 7,
}enTSENCTYPE;

typedef enum tagTSSLICETYPE_
{
	EN_SLICE_TYPE_P = 0,
	EN_SLICE_TYPE_I,
	EN_SLICE_TYPE_VI
 }enTSSLICETYPE;


 typedef enum enTSWRITEERR_
{
	EN_TSWRITE_ERR_SUCCESS = (0),
	EN_TSWRITE_ERR_FAILT = (-1),
	EN_TSWRITE_ERR_FIRSIFRAME = (-2), ////第一帧不是I帧
	EN_TSWRITE_ERR_FILE = (-3), ////写文件错误
	EN_TSWRITE_ERR_DESCRIP = (-4), ////写Descrip错误
	EN_TSWRITE_ERR_TYPE = (-4), ////编码标识超出范围
}enTSWRITEERR;

HTSWRITE TSWRITE_Open(char* pstrFile, enTSENCTYPE nVencType);
int TSWRITE_Close(HTSWRITE hRecord);
int TSWRITE_Write(HTSWRITE hRecord, enTSENCTYPE nFrameType, unsigned char* pFrameBuf,	int nFrameSize,	uint64_t TimeStamp, enTSSLICETYPE nSliceType);
int TSWRITE_SetDescrip(HTSWRITE hRecord,int nPicWSize, int nPicHSize, int nBitRate, int nFrameRate);
int TSWRITE_Sync(HTSWRITE hRecord);


#ifdef __cplusplus
}
#endif

#endif


