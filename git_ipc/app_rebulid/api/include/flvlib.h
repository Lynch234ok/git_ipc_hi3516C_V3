
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <string.h>

#ifndef FLV_LIB_H_
#define FLV_LIB_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef ssize_t (*FLV_WRITE_FUNC)(int fd, const void* buf, size_t count);
typedef struct FLV
{
	void (*set_write)(struct FLV* thiz, FLV_WRITE_FUNC do_write);
	
	void (*set_video)(struct FLV *thiz, size_t width, size_t height, int framerate_fps, int bitrate_kbps);
	void (*set_audio)(struct FLV *thiz);	
	ssize_t (*write_h264)(struct FLV *thiz, const char* data, size_t data_size, bool key_frame, uint32_t timestamp_ms);
	ssize_t (*write_audio)(struct FLV *thiz, const char* data, size_t data_size, uint32_t timestamp_ms, unsigned char audio_conf);
	
}FLV_t;

extern FLV_t* FLV_open_file(const char* file_name);
extern FLV_t* FLV_dup_file(int fd);
extern int FLV_close(FLV_t* flv);

#ifdef __cplusplus
};
#endif
#endif // FLV_LIB_H_

