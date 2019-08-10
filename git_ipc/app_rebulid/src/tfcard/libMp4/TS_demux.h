#ifndef TSDEMUX_H_
#define TSDEMUX_H_

#define NK_MP4_EOF   (-2)
#define NK_MP4_MALLOC_FAIL  (-3)

#ifdef __cplusplus
extern "C" {
#endif

#include "mpeg_ts.h"

#define DEFAULT_VIDEO_BUF_SIZE (256*1024)
#define DEFAULT_AUDIO_BUF_SIZE (1024)
typedef enum  {
	FRAME_SLICE_P,
	FRAME_SLICE_I,
	FRAME_SLICE_VI
 }FRAME_SLICE_E;


typedef  void* lpNK_MP4;


lpNK_MP4 NK_MP4_DemuxerOpen(char *path);
int NK_MP4_DemuxNext(lpNK_MP4  demuxer, void **buf, uint32_t * bufSize, int *frameType, uint64_t *ptsUs,FRAME_SLICE_E * slicetype);
int NK_MP4_Demux_BUF_REPLACE(lpNK_MP4  demuxer,void *tmpvideobuf,void *tmpaudiobuf);
int  NK_MP4_DemuxerClose(lpNK_MP4 demux);
int NK_MP4_seek(lpNK_MP4 demuxer,FRAME_SLICE_E target_slicetype,uint64_t tstime_us);
int NK_MP4_getinfo(lpNK_MP4 demuxer,lpFILEDESCIP_S filedes);
int NK_MP4_GetJpeg_Thumbnail(lpNK_MP4 demuxer,void *thumbnailbuf);


#ifdef __cplusplus
}
#endif
#endif /* TSDEMUX_H_ */

