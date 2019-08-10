

#ifndef _TFCARD_EXPORT_H_
#define _TFCARD_EXPORT_H_

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/vfs.h>

#include <sys/mount.h>
#include <sys/statvfs.h>

//////
#include "LibTfcard.h"
#include "TS_Write.h"
#include "TS_Read.h"

#if 0
#include "mp4mux.h"
#include "mpeg_ts.h"
#include "mpegts.h"
#include "TS_demux.h"
#endif

#include "tfcard_opt.h"
#include "tfcard_thread.h"
#include "tfcard_play.h"
#include "tfcard_rec.h"
#include "tfcard_file.h"

#define VIDEO_PATH "video"
#define TFCARD_MIN_FREE_SPACE (256)

#if 0
#ifndef LOG_TRACE
#define LOG_TRACE(msg, ...) printf("\033[1;30mTRACE [%s:%s:%d] " msg "\033[0;0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG(msg, ...) printf("\033[1;30mTRACE [%s:%s:%d] " msg "\033[0;0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef LOG_INFO
#define LOG_INFO(msg, ...) printf("\033[1;30mTRACE [%s:%s:%d] " msg "\033[0;0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef LOG_WARN
#define LOG_WARN(msg, ...) printf("\033[1;30mTRACE [%s:%s:%d] " msg "\033[0;0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(msg, ...) printf("\033[1;30mTRACE [%s:%s:%d] " msg "\033[0;0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif


#ifndef NK_TRACE
#define NK_TRACE(format, ...) LOG_TRACE(format, ##__VA_ARGS__)
#endif

#ifndef NK_DEBUG
#define NK_DEBUG(format, ...) LOG_DEBUG(format, ##__VA_ARGS__)
#endif

#ifndef NK_INFO
#define NK_INFO(format, ...) LOG_INFO(format, ##__VA_ARGS__)
#endif

#ifndef NK_WARN
#define NK_WARN(format, ...) LOG_WARN(format, ##__VA_ARGS__)
#endif

#ifndef NK_ERROR
#define NK_ERROR(format, ...) LOG_ERROR(format, ##__VA_ARGS__)
#endif
#endif

#endif

