#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packfile.h"
#include "mp4mux/hi_mp4_muxer.h"

#define PACK_DEBUG printf

typedef struct {
	HI_HANDLE MuxHandle;

	HI_HANDLE VdoHandle;
	HI_HANDLE AdiHandle;
	HI_HANDLE DatHandle;

	unsigned long long Duration;

	int  VideoType;
	int  AudioType;
    int  DataType;

	int  PicWSize;
	int  PicHSize;

	int  FrmRate;
	int  BitRate;
} MP4Cxt_t;

typedef struct {
	MP4Cxt_t      MP4Cxt;
	PackInfoCxt_t Info;
} PackMP4Cxt_t;

static int FSyncInvoke(void)
{
	//system("sync"); //FIXME:
	sync();

	return 0;
}

static HI_S32 PackMP4MuxTrackV(MP4Cxt_t * Cxt)
{
	HI_MP4_TRACK_INFO_S stVideo;

	memset(&stVideo, 0, sizeof(HI_MP4_TRACK_INFO_S));
	stVideo.enTrackType              = HI_MP4_STREAM_VIDEO;
	stVideo.stVideoInfo.enCodecID    = (HI_MP4_CODECID_E)Cxt->VideoType;
	stVideo.fSpeed                   = 1.0f;
	stVideo.stVideoInfo.u32BitRate   = Cxt->BitRate;
	stVideo.stVideoInfo.u32FrameRate = Cxt->FrmRate;
	stVideo.stVideoInfo.u32Height    = Cxt->PicWSize;
	stVideo.stVideoInfo.u32Width     = Cxt->PicHSize;
	stVideo.u32TimeScale             = 120000;
	snprintf(stVideo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "MP4Video");

	return HI_MP4_MUXER_CreateTrack(Cxt->MuxHandle, &(Cxt->VdoHandle), &stVideo);
}

static HI_S32 PackMP4MuxTrackA(MP4Cxt_t * Cxt)
{
	HI_MP4_TRACK_INFO_S stAudio;

	memset(&stAudio, 0, sizeof(stAudio));
	stAudio.fSpeed                    = 1.0f;
	stAudio.enTrackType               = HI_MP4_STREAM_AUDIO;
	stAudio.stAudioInfo.enCodecID     = (HI_MP4_CODECID_E)Cxt->AudioType;
	stAudio.stAudioInfo.u16SampleSize = 16;
	stAudio.stAudioInfo.u32Channels   = 1;
	stAudio.stAudioInfo.u32SamplePerFrame = 1024; // aac 1024, g711 320
	stAudio.stAudioInfo.u32SampleRate = 8000;
	stAudio.u32TimeScale              = 8000;
	snprintf(stAudio.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "MP4_AUDIO");

	return HI_MP4_MUXER_CreateTrack(Cxt->MuxHandle, &(Cxt->AdiHandle), &stAudio);
}

static HI_S32 PackMP4MuxTrackD(MP4Cxt_t * Cxt)
{
	HI_MP4_TRACK_INFO_S stData;

	memset(&stData, 0, sizeof(stData));
	stData.fSpeed        = 1.0f;
	stData.enTrackType   = HI_MP4_STREAM_DATA;
	stData.u32TimeScale  = 120000;
	snprintf(stData.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "MP4_META");
    stData.stDataInfo.enCodecID = (HI_MP4_CODECID_E)Cxt->DataType;
    stData.stDataInfo.u32Height = Cxt->PicHSize;
    stData.stDataInfo.u32Width = Cxt->PicWSize;
	return HI_MP4_MUXER_CreateTrack(Cxt->MuxHandle, &(Cxt->DatHandle), &stData);
}

PackCxt_t * PackFileNew(char * FileName, int PicWSize, int PicHSize, int BitRate, int FrameRate, int CodecType)
{
	int ret = -1;
	PackMP4Cxt_t * tmpCxt = NULL;

	tmpCxt = (PackMP4Cxt_t *)calloc(1, sizeof(PackMP4Cxt_t));
	if(tmpCxt == NULL) return NULL;

	tmpCxt->Info.MaxSyncSize = PACK_SYNC_SIZE; //Force Sync Every Time Packing Bytes Reach MaxSyncSize

	{
		HI_HANDLE hMP4Muxer;
		HI_MP4_MUXER_CONFIG_S stMP4MuxerCfg;

		// config for mp4 muxer
		memset(&stMP4MuxerCfg, 0, sizeof(stMP4MuxerCfg));
		snprintf(stMP4MuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, FileName);
		stMP4MuxerCfg.hRepairHandle   = -1;
#if defined(HI3516E_V1)
        stMP4MuxerCfg.u32PreAllocUnit = 512 * 1024;
        stMP4MuxerCfg.u32VBufSize     = 512 * 1024;
#else
		stMP4MuxerCfg.u32PreAllocUnit = 1024 * 1024;
		stMP4MuxerCfg.u32VBufSize     = 1024 * 1024;
#endif
		stMP4MuxerCfg.bConstantFps    = HI_FALSE;
		stMP4MuxerCfg.bCo64Flag       = HI_FALSE;

		ret = HI_MP4_MUXER_Create(&hMP4Muxer, &stMP4MuxerCfg);
		if (HI_SUCCESS != ret) {
			free(tmpCxt);
			return NULL;
		}

		tmpCxt->MP4Cxt.MuxHandle = hMP4Muxer;

		tmpCxt->MP4Cxt.VdoHandle = 1;
		tmpCxt->MP4Cxt.AdiHandle = 2;
		tmpCxt->MP4Cxt.DatHandle = 3;

		/* default use HI_MP4_CODEC_ID_H264 */
        tmpCxt->MP4Cxt.VideoType = (CodecType == PACK_CODEC_TYPE_H265) ? HI_MP4_CODEC_ID_H265 : HI_MP4_CODEC_ID_H264;
		tmpCxt->MP4Cxt.AudioType = HI_MP4_CODEC_ID_AACLC;
        tmpCxt->MP4Cxt.DataType = HI_MP4_CODEC_ID_DATA;

		tmpCxt->MP4Cxt.PicWSize  = PicWSize;
		tmpCxt->MP4Cxt.PicHSize  = PicHSize;
		tmpCxt->MP4Cxt.FrmRate   = FrameRate;
		tmpCxt->MP4Cxt.BitRate   = BitRate;
		if((HI_SUCCESS != PackMP4MuxTrackV(&(tmpCxt->MP4Cxt)))
		|| (HI_SUCCESS != PackMP4MuxTrackA(&(tmpCxt->MP4Cxt)))
		|| (HI_SUCCESS != PackMP4MuxTrackD(&(tmpCxt->MP4Cxt)))) {
			HI_U64 MP4_Duration = 0;
			HI_MP4_MUXER_DestroyAllTracks(tmpCxt->MP4Cxt.MuxHandle, NULL);
			HI_MP4_MUXER_Destroy(tmpCxt->MP4Cxt.MuxHandle, &MP4_Duration);

			free(tmpCxt);
			return NULL;
		}
	}

	FSyncInvoke();

	return (PackCxt_t *)tmpCxt;
}

int PackFileWriteFrame(PackCxt_t * Pack,
		unsigned char * AVFrameBuff,
		int FrameSize,
		unsigned long long TimeStamp,
		int AVFrameType)
{
	HI_S32 ret = -1;
	PackMP4Cxt_t * tmpCxt = (PackMP4Cxt_t *)Pack;
	if(Pack == NULL) return -1;

	if((AVFrameType == PACK_FRAME_TYPE_VIDEO_IFRAME)
	|| (AVFrameType == PACK_FRAME_TYPE_VIDEO_PFRAME)) {
		HI_MP4_FRAME_DATA_S FrmDat;

		FrmDat.pu8DataBuffer = AVFrameBuff;
		FrmDat.u32DataLength = FrameSize;
		FrmDat.bKeyFrameFlag = (AVFrameType == PACK_FRAME_TYPE_VIDEO_IFRAME) ? HI_TRUE : HI_FALSE;
		FrmDat.u64TimeStamp  = TimeStamp * 1000;

		ret = HI_MP4_MUXER_WriteFrame(tmpCxt->MP4Cxt.MuxHandle, tmpCxt->MP4Cxt.VdoHandle, &FrmDat);
		if (HI_SUCCESS != ret) {
			PACK_DEBUG("HI_MP4_MUXER_WriteFrame for Video fail %d", ret);
			return -1;
		}

		tmpCxt->Info.VFrameCount += 1;
		tmpCxt->Info.VBytesCount += FrameSize;
	}
	else if (AVFrameType == PACK_FRAME_TYPE_AUDIO) {
		HI_MP4_FRAME_DATA_S FrmDat;

		FrmDat.pu8DataBuffer = AVFrameBuff;
		FrmDat.u32DataLength = FrameSize;
		FrmDat.bKeyFrameFlag = HI_FALSE;
		FrmDat.u64TimeStamp  = TimeStamp * 1000;

		ret = HI_MP4_MUXER_WriteFrame(tmpCxt->MP4Cxt.MuxHandle, tmpCxt->MP4Cxt.AdiHandle, &FrmDat);
		if (HI_SUCCESS != ret) {
			PACK_DEBUG("HI_MP4_MUXER_WriteFrame for Video fail %d", ret);
			return -1;
		}

		tmpCxt->Info.AFrameCount += 1;
		tmpCxt->Info.ABytesCount += FrameSize;
	}
	else if(AVFrameType == PACK_FRAME_TYPE_DATA) {
		HI_MP4_FRAME_DATA_S FrmDat;

		FrmDat.pu8DataBuffer = AVFrameBuff;
		FrmDat.u32DataLength = FrameSize;
		FrmDat.bKeyFrameFlag = PACK_FRAME_TYPE_DATA;
		FrmDat.u64TimeStamp  = TimeStamp * 1000;

		ret = HI_MP4_MUXER_WriteFrame(tmpCxt->MP4Cxt.MuxHandle, tmpCxt->MP4Cxt.DatHandle, &FrmDat);
		if (HI_SUCCESS != ret) {
			PACK_DEBUG("HI_MP4_MUXER_WriteFrame for data fail %d", ret);
			return -1;
		}
	}
	else {
		PACK_DEBUG("Unsupported Frame Type! type: %X", AVFrameType);

		return -1;
	}

	//DurationCount += ; //Just Roughly Counting //FIXME:

	tmpCxt->Info.BytesCount  += FrameSize;
	tmpCxt->Info.SegByteSize += FrameSize;

	if(tmpCxt->Info.SegByteSize >= tmpCxt->Info.MaxSyncSize) { //Reach MaxSyncSize to Fsync
		tmpCxt->Info.SegByteSize   = 0;
		tmpCxt->Info.SegSizeCount += 1;

		FSyncInvoke();
	}

	return 0;
}

int PackFileEnd(PackCxt_t * Pack)
{
	int ret = -1;
	PackMP4Cxt_t * tmpCxt = (PackMP4Cxt_t *)Pack;
	if(Pack == NULL) return -1;

	ret = HI_MP4_MUXER_DestroyAllTracks(tmpCxt->MP4Cxt.MuxHandle, NULL);
	if (HI_SUCCESS != ret) {
		printf("HI_MP4_MUXER_DestroyAllTracks fail %d", ret);
	}

	ret = HI_MP4_MUXER_Destroy(tmpCxt->MP4Cxt.MuxHandle, &(tmpCxt->MP4Cxt.Duration));
	if (HI_SUCCESS != ret) {
		printf("HI_MP4_MUXER_Destroy fail %d", ret);
	}

	FSyncInvoke();

	return 0;
}

int PackFileFree(PackCxt_t * Pack)
{
	PackMP4Cxt_t * tmpCxt = (PackMP4Cxt_t *)Pack;
	if(Pack == NULL) return -1;

	free(tmpCxt);
	return 0;
}

int PackFileSetParam(PackCxt_t * Pack)
{
	PackMP4Cxt_t * tmpCxt = (PackMP4Cxt_t *)Pack;
	if(Pack == NULL) return -1;

	return -1;
}

int PackFileGetInfo(PackCxt_t * Pack, PackInfoCxt_t * Info)
{
	PackMP4Cxt_t * tmpCxt = (PackMP4Cxt_t *)Pack;
	if(Pack == NULL || Info == NULL) return -1;

	tmpCxt->Info.Duration = tmpCxt->MP4Cxt.Duration; //FIXME:
	*Info = tmpCxt->Info;

	return 0;
}
