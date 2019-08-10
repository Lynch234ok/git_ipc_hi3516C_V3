#ifndef __PACKFILE_H__
#define __PACKFILE_H__

enum {
	PACK_FRAME_TYPE_NONE         = 0,
	PACK_FRAME_TYPE_VIDEO_IFRAME = 1,
	PACK_FRAME_TYPE_VIDEO_PFRAME = 2,
	PACK_FRAME_TYPE_AUDIO        = 3,
	PACK_FRAME_TYPE_DATA         = 4,
};

enum
{
    PACK_CODEC_TYPE_H264 = 0,
    PACK_CODEC_TYPE_H265,
    PACK_CODEC_TYPE_MJPEG,
    PACK_CODEC_TYPE_AACLC,
    PACK_CODEC_TYPE_MP3,
    PACK_CODEC_TYPE_G726,
    PACK_CODEC_TYPE_G711_A,
    PACK_CODEC_TYPE_G711_M,
    PACK_CODEC_TYPE_BUTT
};

#if defined(HI3516E_V1)
#define PACK_SYNC_SIZE (512*1024)
#else
#define PACK_SYNC_SIZE (1*1024*1024)
#endif

typedef struct {
	unsigned int VFrameCount; //Video Frame Number Counting;
	unsigned int VBytesCount; //Video Stream Bytes Counting;
	unsigned int AFrameCount; //Audio Frame Number Counting;
	unsigned int ABytesCount; //Audio Stream Bytes Counting;

	unsigned int BytesCount;    //PackFile Media Frame Packed in Bytes;
	int          Duration;      //PackFile Duration Increase Counting;

	unsigned int MaxSyncSize;   //When to FSync or Sync;
	unsigned int SegByteSize;   //Bytes Counting in One Sync Loop;
	unsigned int SegSizeCount;  //FSync or Sync Times Counting;
} PackInfoCxt_t;

typedef struct PackMP4Cxt_t PackCxt_t;

PackCxt_t * PackFileNew(char * FileName, int PicWSize, int PicHSize, int BitRate, int FrameRate, int CodecType);
int PackFileWriteFrame(PackCxt_t * Pack,
		unsigned char * AVFrameBuff,
		int FrameSize,
		unsigned long long TimeStamp,
		int AVFrameType);
int PackFileEnd(PackCxt_t * Pack);
int PackFileFree(PackCxt_t * Pack);

int PackFileSetParam(PackCxt_t * Pack);
int PackFileGetInfo(PackCxt_t * Pack, PackInfoCxt_t * Info);

#endif //__PACKFILE_H__
