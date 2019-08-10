
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "sdcard_def.h"

#ifndef SDCARD_FRAMER_H_
#define SDCARD_FRAMER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kSDF_V_CODEC_H264 (0)

#define kSDF_V_PROFILE_BASELINE (0)
#define kSDF_V_PROFILE_MAIN (1)
#define kSDF_V_PROFILE_HIGH (2)

typedef struct SDCARD_FRAMER_VIDEO {
	int sessionID;
	int channelID, channelSubID;
	int codec; // no use
	int profile; // no use
	int resolutionWidth, resolutionHeight;
	int frameRate;
	bool keyFrame;
}ST_SDCARD_FRAMER_VIDEO, *LP_SDCARD_FRAMER_VIDEO;

#define kSDF_A_CODEC_G711A (0)

typedef struct SDCARD_FRAMER_AUDIO {
	int sessionID;
	int channelID;
	int codec; // no use
	int sampleRate, sampleBitWidth;
}ST_SDCARD_FRAMER_AUDIO, *LP_SDCARD_FRAMER_AUDIO;

#define kSDF_P_CODEC_JPEG (0)

typedef struct SDCARD_FRAMER_PIC {
	int sessionID;
	int channelID;
	int codec; // jpeg only
	int resolutionWidth, resolutionHeight;
}ST_SDCARD_FRAMER_PIC, *LP_SDCARD_FRAMER_PIC;

#define kSDF_TYPE_VIDEO (0)
#define kSDF_TYPE_AUDIO (1)
#define kSDF_TYPE_PIC (2)
#define kSDF_TYPE_EOF (-1)

typedef struct SDCARD_FRAMER {

	int type;
	int dataLength;
	int timestampMS;
	int timeS;

	union {
		ST_SDCARD_FRAMER_VIDEO video;
		ST_SDCARD_FRAMER_AUDIO audio;
		ST_SDCARD_FRAMER_PIC picture;
	};
}ST_SDCARD_FRAMER, *LP_SDCARD_FRAMER;

extern int SDCARD_framer_make(LP_SDCARD_FRAMER framer, char *result, int resultMax);
extern int SDCARD_framer_parse(int fID, LP_SDCARD_FRAMER result, char *frame, int frameMax);
	
#ifdef __cplusplus
};
#endif
#endif //SDCARD_FS_H_

