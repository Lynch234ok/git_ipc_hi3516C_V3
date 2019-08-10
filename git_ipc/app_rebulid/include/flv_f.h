
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <string.h>

#ifndef FLV_F_H_
#define FLV_F_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef ssize_t (*fFLV_STREAM_DO_WRITE)(int fID, const void *buf, size_t count);

typedef struct FLV_STREAM
{
	void (*writeHook)(struct FLV_STREAM *_this, fFLV_STREAM_DO_WRITE do_write);

	// you must set video and audio before write them
	void (*setVideoAttr)(struct FLV_STREAM *_this, size_t resolutionWidth, size_t resolutionHeight, int frameRateFPS, int bitRateBPS);
	void (*setAudioAttr)(struct FLV_STREAM *_this);
	
	ssize_t (*writeH264)(struct FLV_STREAM *_this, const void *data, size_t dataLength, bool keyFrame, int timestampMS, void *buf, int bufMax);
	
}ST_FLV_STREAM, *LP_FLV_STREAM;

extern LP_FLV_STREAM flv_open(const char *fileDIR, int duration, int size);
extern LP_FLV_STREAM flv_dup(int fID, int duration, int size);
extern int flv_close(LP_FLV_STREAM hFLV);

#ifdef __cplusplus
};
#endif
#endif //FLV_F_H_

