
#ifndef SDCARD_DEF_H_
#define SDCARD_DEF_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kSD_MOUNT_POINT "/dev/sdb1"
#define kSD_MOUNT_FOLDER "/media/sdcard"

#define kSD_MEDIA_FOLDER "/media/sdcard/media"
#define kSD_IMAGE_FOLDER "/media/sdcard/picture"

#define kSD_MEDIA_FILE_MAX_MB (64)
#define kSD_FREE_RESERVED_MB (128 + 64)

#ifdef __cplusplus
};
#endif
#endif //SDCARD_DEF_H_

