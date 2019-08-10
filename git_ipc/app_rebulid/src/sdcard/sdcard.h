
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "sdcard_def.h"
#include "sdcard_db.h"
#include "sdcard_framer.h"

#ifndef SDCARD_H_
#define SDCARD_H_
#ifdef __cplusplus
extern "C" {
#endif


typedef bool (*fSDCARD_RECORDER_ACCESS)(void);
typedef int (*fSDCARD_RECORDER_APPLICATION)(fSDCARD_RECORDER_ACCESS access);

extern int SDCARD_request_format();

#define kSD_STAT_NOT_INIT (-1)
#define kSD_STAT_ON_WORK (0)
#define kSD_STAT_EJECTED (1000)
#define kSD_STAT_FS_ERROR (2000)
#define kSD_STAT_FORMATTING (2001)
//#define k

extern int SDCARD_status();

//typedef void 

extern int SDCARD_init(const char *mountPoint, const char *mountDIR, const char *cachePath, fSDCARD_RECORDER_APPLICATION recorderApp);
extern void SDCARD_destroy();

// file record
extern void *SDCARD_media_open(int fileID);
extern void SDCARD_media_close(void *fID);

extern int SDCARD_media_write(void *fID, const LP_SDCARD_FRAMER framer, void *data, int dataLength);
extern int SDCARD_media_read(int fID, LP_SDCARD_FRAMER framer, void *data, int dataMax);
extern int SDCARD_media_tell(void *fID);

extern bool SDCARD_media_exist(int fileID);
extern int SDCARD_media_remove(int fileID);

// file playback
extern int SDCARD_media_open_range(int beginFileID, int endFileID, int *fIDs, int nFIDMax);
extern int SDCARD_media_close_range(int *fIDs, int nFIDMax);

#ifdef __cplusplus
};
#endif
#endif //SDCARD_H_

