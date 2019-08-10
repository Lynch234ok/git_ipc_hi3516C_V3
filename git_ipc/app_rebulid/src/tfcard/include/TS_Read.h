
#ifndef TS_READER_H_
#define TS_READER_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef  void* HTSREAD;

typedef struct tagFILEDESCIP_
{
	uint64_t  s64FileSize;
    uint64_t  s64StartTime;             // p_data×Ö½ÚÊý
    uint64_t  s64Duration;             /**< Total duration of a file, in the unit of ms. */
    uint32_t  u32Height;
	uint32_t  u32Width;
    uint32_t  fFrameRate;                   /**< the frame rate of the stream*/
    uint32_t  u32Bitrate;                     /**< File bit rate, in the unit of bit/s. */
    uint32_t  u32AudioChannelCnt;
    uint32_t  u32SampleRate; /**< the sample rate of the audio stream */
    uint32_t  s32UsedAudioStreamIndex; /**< the index of the audio stream. one file may have many audio streams*/
    uint32_t  enVideoType;
} stFILEDESCIP,*pstFILEDESCIP;

typedef enum enTSREADERR_
{
	EN_TSREAD_ERR_OK = 0,////
	EN_TSREAD_ERR_FAILT = -1,
	EN_TSREAD_ERR_NULL = -2,
	EN_TSREAD_ERR_MALLOC = -3,
	EN_TSREAD_ERR_EOF = -4,
}enTSREADEER;


#define TS_STREAMTYPE_11172_AUDIO   (0x03)
#define TS_STREAMTYPE_13818_AUDIO   (0x04)
#define TS_STREAMTYPE_AAC_AUDIO     (0x0F)
#define TS_STREAMTYPE_g711_AUDIO    (0x90)
#define TS_STREAMTYPE_H264_VIDEO    (0x1B)
#define TS_STREAMTYPE_H265_VIDEO    (0x24)

HTSREAD TSREAD_Open(char* pstrFile);
int TSREAD_Close(HTSREAD hRead);
int TSREAD_Read(HTSREAD hRead, void** buf, uint32_t* bufSize, int* frameType, uint64_t* ptsUs, int* iskey);
int TSREAD_GetDescrip(HTSREAD hRead,pstFILEDESCIP pFileDescip);


#ifdef __cplusplus
}
#endif

#endif

