
#include "sdk_def.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>

#ifndef SDK_ENC_H_
#define SDK_ENC_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	EPTZ_CMD_NULL = -1,
	EPTZ_CMD_UP,
	EPTZ_CMD_DOWN,
	EPTZ_CMD_LEFT,
	EPTZ_CMD_RIGHT,
	EPTZ_CMD_LEFT_UP,
	EPTZ_CMD_RIGHT_UP,
	EPTZ_CMD_LEFT_DOWN,
	EPTZ_CMD_RIGHT_DOWN,
	EPTZ_CMD_AUTOPAN,
	EPTZ_CMD_IRIS_OPEN,
	EPTZ_CMD_IRIS_CLOSE,
	EPTZ_CMD_ZOOM_IN,
	EPTZ_CMD_ZOOM_OUT,
	EPTZ_CMD_FOCUS_FAR,
	EPTZ_CMD_FOCUS_NEAR,
	EPTZ_CMD_STOP,
	EPTZ_CMD_WIPPER_ON,
	EPTZ_CMD_WIPPER_OFF,
	EPTZ_CMD_LIGHT_ON,
	EPTZ_CMD_LIGHT_OFF,
	EPTZ_CMD_POWER_ON,
	EPTZ_CMD_POWER_OFF,
	EPTZ_CMD_GOTO_PRESET,
	EPTZ_CMD_SET_PRESET,
	EPTZ_CMD_CLEAR_PRESET,
	EPTZ_CMD_TOUR,
	EPTZ_COMMAND_CNT,
}EPTZ_COMMAND;

typedef enum{
	eSDK_ENC_FIX_MODE_WALL=0,
	eSDK_ENC_FIX_MODE_CELL,
	eSDK_ENC_FIX_MODE_TABLE,
}emSDK_ENC_FIX_MODE;

typedef enum{
	eSDK_ENC_SHOW_MODE_WALL_ORIGIN = 0x0,
	eSDK_ENC_SHOW_MODE_WALL_180,
	eSDK_ENC_SHOW_MODE_WALL_SPLIT,
	eSDK_ENC_SHOW_MODE_WALL_WALL_SPLIT,
	eSDK_ENC_SHOW_MODE_WALL_4R,
	eSDK_ENC_SHOW_MODE_WALL_KITR,
	eSDK_ENC_SHOW_MODE_WALL_KITO,

	eSDK_ENC_SHOW_MODE_CELL_ORIGIN = 0x10,
	eSDK_ENC_SHOW_MODE_CELL_360,
	eSDK_ENC_SHOW_MODE_CELL_SPLIT,
	eSDK_ENC_SHOW_MODE_CELL_4R,
	eSDK_ENC_SHOW_MODE_CELL_WALL_SPLIT,
	eSDK_ENC_SHOW_MODE_CELL_180,
	eSDK_ENC_SHOW_MODE_CELL_KITR,
	eSDK_ENC_SHOW_MODE_CELL_KITO,

	eSDK_ENC_SHOW_MODE_TABLE_ORIGIN= 0x20,
	eSDK_ENC_SHOW_MODE_TABLE_360,
	eSDK_ENC_SHOW_MODE_TABLE_SPLIT,
	eSDK_ENC_SHOW_MODE_TABLE_4R,
	eSDK_ENC_SHOW_MODE_TABLE_VR,
	eSDK_ENC_SHOW_MODE_TABLE_KITR,
	eSDK_ENC_SHOW_MODE_TABLE_KITO,
}emSDK_ENC_SHOW_MODE;

typedef enum SDK_ENC_BUF_FRAME_TYPE
{
    kSDK_ENC_BUF_FRAME_UNUSE = 0,
    kSDK_ENC_BUF_FRAME_BASE_IDRSLICE = 1,                              //the Idr frame at Base layer
    kSDK_ENC_BUF_FRAME_BASE_PSLICE_REFTOIDR,                           //the P frame at Base layer, referenced by other frames at Base layer and reference to Idr frame
    kSDK_ENC_BUF_FRAME_BASE_PSLICE_REFBYBASE,                          //the P frame at Base layer, referenced by other frames at Base layer
    kSDK_ENC_BUF_FRAME_BASE_PSLICE_REFBYENHANCE,                       //the P frame at Base layer, referenced by other frames at Enhance layer
    kSDK_ENC_BUF_FRAME_ENHANCE_PSLICE_REFBYENHANCE,                    //the P frame at Enhance layer, referenced by other frames at Enhance layer
    kSDK_ENC_BUF_FRAME_ENHANCE_PSLICE_NOTFORREF,                       //the P frame at Enhance layer ,not referenced
    kSDK_ENC_BUF_FRAME_ENHANCE_PSLICE_BUTT
}enSDK_ENC_BUF_FRAME_TYPE;

#define kSDK_ENC_BUF_DATA_MAGIC (0xff00a0a0UL)
typedef enum SDK_ENC_BUF_DATA_TYPE {
	// video
	kSDK_ENC_BUF_DATA_H264 = (0x00000000UL),
	kSDK_ENC_BUF_DATA_H265,
	kSDK_ENC_BUF_DATA_JPEG,
	// audio
	kSDK_ENC_BUF_DATA_PCM = (0x80000000UL),
	kSDK_ENC_BUF_DATA_G711A,
	kSDK_ENC_BUF_DATA_G711U,
	kSDK_ENC_BUF_DATA_AAC,
}enSDK_ENC_BUF_DATA_TYPE, *lpSDK_ENC_BUF_DATA_TYPE;

/* √ø¥Œ∏¸–¬P2P ±∂º–Ë“™ºÏ≤È’‚“ª≤ø∑÷ƒ⁄»› 
    “ÚŒ™’‚≤ø∑÷ƒ⁄»› «¥”p2psdk.høΩ±¥π˝¿¥
*/
#define ALIGNED(n) __attribute__((aligned(n)))
/* ÷±≤•÷°Õ∑ */
typedef struct _kp2p_live_head_s {
    /* ÷°¿‡–Õ£¨0£∫“Ù∆µ÷°£¨1£∫ ”∆µI÷°£¨2£∫ ”∆µP÷° */
    uint32_t frametype;
    /* Õ®µ¿ */
    uint32_t channel;
    /*  ”∆µ“Ù∆µ */
    union {
        struct {
            /*  ”∆µ±‡¬Î "H264"/"H265" */
            char enc[8];
            /*  ”∆µ÷°¬  */
            uint32_t fps;
            /*  ”∆µøÌ */
            uint32_t width;
            /*  ”∆µ∏ﬂ */
            uint32_t height;
        } v;
        struct {
            /* “Ù∆µ±‡¬Î "G711A" */
            char enc[8];
            /* ≤…—˘¬  */
            uint32_t samplerate;
            /* ≤…—˘ŒªøÌ */
            uint32_t samplewidth;
            /* channels ƒ¨»œ 1, µ•…˘µ¿ */
            uint32_t channels;
            /* compresss —πÀı¬ , G711 == 2.0 */
            float compress;
        } a;
    };
} ALIGNED(4) _kp2p_live_head_t;

/* ªÿ∑≈÷°Õ∑ */
typedef struct _kp2p_replay_head_s {
    /* ÷°¿‡–Õ£¨0£∫“Ù∆µ÷°£¨1£∫ ”∆µI÷°£¨2£∫ ”∆µP÷° */
    uint32_t frametype;
    /* ¬ºœÒ¬Î¡˜∂‘”¶µƒÕ®µ¿ */
    uint32_t channel;
    /* ¬ºœÒ¬Î¡˜µƒ¬ºœÒ¿‡–Õ */
    uint32_t type;
    /* ¬ºœÒ¬Î¡˜÷ ¡ø */
    uint32_t quality;
    union {
        struct {
            /*  ”∆µ±‡¬Î "H264"/"H265" */
            char enc[8];
            /*  ”∆µ÷°¬  */
            uint32_t fps;
            /*  ”∆µøÌ */
            uint32_t width;
            /*  ”∆µ∏ﬂ */
            uint32_t height;
        } v;
        struct {
            /* “Ù∆µ±‡¬Î "G711A"*/
            char enc[8];
            /* ≤…—˘¬  */
            uint32_t samplerate;
            /* ≤…—˘ŒªøÌ */
            uint32_t samplewidth;
            /* channels ƒ¨»œ 1, µ•…˘µ¿ */
            uint32_t channels;
            /* compresss —πÀı¬ , G711 == 2.0 */
            float compress;
        } a;
    };
} ALIGNED(4) _kp2p_replay_head_t;

/* P2P÷°Õ∑ */
typedef struct _kp2p_frame_head_s {
    /* Â∏¶Â∏ßÂ∫èÂè∑ÁöÑÁâàÊú¨ */
    uint32_t magic2;
    /* Â∏ßÂ∫èÂè∑ */
    uint32_t frame_seq;
    /* ‰øùÁïôÂ≠óÊÆµ */
    uint8_t reserve[32];

    /* magic number πÃ∂®Œ™ P2PSDK_FRAMEHEAD_MAGIC */
    uint32_t magic;
    /* ∞Ê±æ–≈œ¢£¨µ±«∞∞Ê±æŒ™2.0.0.0£¨πÃ∂®Œ™0x02000000 */
    uint32_t version;
    /* Õ∑¿‡–Õ£¨ 0£∫÷±≤•, 1:¬ºœÒ */
    uint32_t headtype;
    /* ÷°µƒ¬„ ˝æ›≥§∂»(Byte) */
    uint32_t framesize;
    /*  ±º‰¥¡ */
    uint64_t ts_ms;
    union {
        /* ÷±≤•÷° */
        _kp2p_live_head_t live;
        /* ªÿ∑≈÷° */
        _kp2p_replay_head_t replay;
        /* À´œÚ”Ô“Ù÷° */
        _kp2p_live_head_t vop2p;
    };
} ALIGNED(4) _kp2p_frame_head_t;

#pragma pack(4)
typedef struct SDK_ENC_BUF_ATTR {
	// public attr
	uint32_t magic; // must be "kSdkEncBufDataMagic"

	enSDK_ENC_BUF_DATA_TYPE type;
	uint64_t timestamp_us; // the timestamp of soc engine, unit: us
	uint64_t time_us; // the timestamp of system clock, unit: us
	size_t data_sz;
	union {
		// kSdkEncBufDataH264
		struct {
			bool keyframe; // TRUE / FALSE
			uint32_t ref_counter; // ref frame counter
			uint32_t fps;
			uint32_t width;
			uint32_t height;
			uint32_t frametype;
		}h264;
		struct {
			bool keyframe; // TRUE / FALSE
			uint32_t ref_counter; // ref frame counter
			uint32_t fps;
			uint32_t width;
			uint32_t height;
			uint32_t frametype;
		}h265;
	
		// kSdkEncBufDataPcm, kSdkEncBufDataG711a, kSdkEncBufDataG711u
		struct {
			uint32_t sample_rate;
			uint32_t sample_width;
			uint32_t packet;
			float compression_ratio; // if g711a.u == 2.0
		}pcm, g711a, g711u;
	};
	_kp2p_frame_head_t p2pFrameHead;
}stSDK_ENC_BUF_ATTR, *lpSDK_ENC_BUF_ATTR;
#pragma pack()

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define kSDK_ENC_QUALITY_AUTO (0)
#define kSDK_ENC_QUALITY_FLUENCY (1)
#define kSDK_ENC_QUALITY_BD (2)
#define kSDK_ENC_QUALITY_HD (3)

typedef struct SDK_ENC_STREAM_H264_ATTR {
	char name[32];
	int  width, height;
	int fps, gop;

#define kSDK_ENC_H264_PROFILE_AUTO (0)
#define kSDK_ENC_H264_PROFILE_BASELINE (1)
#define kSDK_ENC_H264_PROFILE_MAIN (2)
#define kSDK_ENC_H264_PROFILE_HIGH (3)
	int profile;
#define kSDK_ENC_H264_RC_MODE_AUTO (0)
#define kSDK_ENC_H264_RC_MODE_VBR (1)
#define kSDK_ENC_H264_RC_MODE_CBR (2)
#define kSDK_ENC_H264_RC_MODE_ABR (3)
#define kSDK_ENC_H264_RC_MODE_FIXQP (4)
	int rc_mode;
	int bps;
	int quality;
	int buf_id;
    int refEnable;//Ë∑≥Â∏ßÂèÇËÄÉ 1ÂºÄÂêØ0ÂÖ≥Èó≠
	bool _enable_smartP;
}ST_SDK_ENC_STREAM_H264_ATTR, *LP_SDK_ENC_STREAM_H264_ATTR;

typedef struct SDK_ENC_STREAM_H265_ATTR {
	char name[32];
	int  width, height;
	int fps, gop;
	
#define kSDK_ENC_H265_PROFILE_MAIN (0)

	int profile;
#define kSDK_ENC_H265_RC_MODE_AUTO (0)
#define kSDK_ENC_H265_RC_MODE_VBR (1)
#define kSDK_ENC_H265_RC_MODE_CBR (2)
#define kSDK_ENC_H265_RC_MODE_FIXQP (3)
	int rc_mode;
	int bps;
	int quality;
	int buf_id;
    int refEnable;//Ë∑≥Â∏ßÂèÇËÄÉ 1ÂºÄÂêØ0ÂÖ≥Èó≠
	bool _enable_smartP;
}ST_SDK_ENC_STREAM_H265_ATTR, *LP_SDK_ENC_STREAM_H265_ATTR;


typedef struct SDK_ENC_STREAM_ATTR{
	enSDK_ENC_BUF_DATA_TYPE enType;
	union{
		ST_SDK_ENC_STREAM_H264_ATTR  H264_attr;
		ST_SDK_ENC_STREAM_H265_ATTR  H265_attr;
	};
}ST_SDK_ENC_STREAM_ATTR,*LP_SDK_ENC_STREAM_ATTR;



typedef struct SDK_ENC_STREAM_G711A_ATTR {
	int ain; // ignore when init
	int vin_ref; // ignore when init
	int sample_rate, sample_width;
	size_t packet_size;
}ST_SDK_ENC_STREAM_G711A_ATTR, *LP_SDK_ENC_STREAM_G711A_ATTR;

typedef enum SDK_ENC_SNAPSHOT_QUALITY {
	kSDK_ENC_SNAPSHOT_QUALITY_HIGHEST,
	kSDK_ENC_SNAPSHOT_QUALITY_HIGH,
	kSDK_ENC_SNAPSHOT_QUALITY_MEDIUM,
	kSDK_ENC_SNAPSHOT_QUALITY_LOW,
	kSDK_ENC_SNAPSHOT_QUALITY_LOWEST,
}enSDK_ENC_SNAPSHOT_QUALITY, *lpSDK_ENC_SNAPSHOT_QUALITY;

#define kSDK_ENC_SNAPSHOT_SIZE_AUTO (0)
#define kSDK_ENC_SNAPSHOT_SIZE_MAX (-1)
#define kSDK_ENC_SNAPSHOT_SIZE_MIN (-2)

typedef union SDK_ENC_VIDEO_OVERLAY_PIXEL {
	uint32_t argb8888;
	struct {
		uint8_t blue, green, red, alpha;
	};
}stSDK_ENC_VIDEO_OVERLAY_PIXEL, *lpSDK_ENC_VIDEO_OVERLAY_PIXEL;

#define SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(_pixel, _a, _r, _g, _b) \
	do{\
		_pixel.alpha = (_a), _pixel.red = (_r), _pixel.green = (_g), _pixel.blue = (_b);\
	}while(0)

typedef struct SDK_ENC_VIDEO_OVERLAY_PIXEL_FORMAT {
	uint32_t rmask, gmask, bmask, amask;
}stSDK_ENC_VIDEO_OVERLAY_PIXEL_FORMAT, *lpSDK_ENC_VIDEO_OVERLAY_PIXEL_FORMAT;

typedef struct SDK_ENC_VIDEO_OVERLAY_CANVAS {

	size_t width, height;
	stSDK_ENC_VIDEO_OVERLAY_PIXEL_FORMAT pixel_format;
	void* pixels;
	int phy_addr;

	int (*put_pixel)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y,
		stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel);
	int (*get_pixel)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y,
		lpSDK_ENC_VIDEO_OVERLAY_PIXEL ret_pixel);
	bool (*match_pixel)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel1, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel2);
	int (*put_rect)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y,
		size_t width, size_t height,stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel);
	int (*fill_rect)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y,
		size_t width, size_t height, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel);
	int (*erase_rect)(struct SDK_ENC_VIDEO_OVERLAY_CANVAS* canvas, int x, int y, size_t width, size_t height);
	
}ST_SDK_ENC_VIDEO_OVERLAY_CANVAS, *LP_SDK_ENC_VIDEO_OVERLAY_CANVAS;

typedef int (*fSDK_ENC_DO_BUFFER_REQUEST)(int buf_id, size_t data_size, bool key_flag, unsigned long long Pts);
typedef int (*fSDK_ENC_DO_BUFFER_APPEND)(int buf_id, const void *data_piece, ssize_t data_piece_size);
typedef int (*fSDK_ENC_DO_BUFFER_COMMIT)(int buf_id);

typedef struct SDK_ENC_API {
	
	// enc buffering callback
	fSDK_ENC_DO_BUFFER_REQUEST do_buffer_request;
	fSDK_ENC_DO_BUFFER_APPEND do_buffer_append;
	fSDK_ENC_DO_BUFFER_COMMIT do_buffer_commit;

	int (*create_stream_h264)(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr);
	int (*release_stream_h264)(int vin, int stream);
	int (*enable_stream_h264)(int vin, int stream, bool flag);
	int (*set_stream_h264)(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr);
	int (*get_stream_h264)(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr);
	int (*request_stream_h264_keyframe)(int vin, int stream);
	
	int (*create_stream_h265)(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr);
	int (*release_stream_h265)(int vin, int stream);
	int (*enable_stream_h265)(int vin, int stream, bool flag);
	int (*set_stream_h265)(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr);
	int (*get_stream_h265)(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr);
	int (*request_stream_h265_keyframe)(int vin, int stream);

	int (*create_stream_g711a)(int ain, int vin_ref);
	int (*create_audio_stream)(int ain, int vin_ref, int type);
	int (*release_stream_g711a)(int ain);
	
	int (*start)(void);
	int (*stop)(void);
	
	int (*creat_snapshot_chn)(int vin, int vencChannelJPEG, ssize_t width, ssize_t height);
	int (*release_snapshot_chn)(int vin, int vencChannelJPEG);
	int (*snapshot)(int vin, enSDK_ENC_SNAPSHOT_QUALITY quality, ssize_t width, ssize_t height, FILE* stream);

	// overlay
	LP_SDK_ENC_VIDEO_OVERLAY_CANVAS (*create_overlay_canvas)(size_t width, size_t height);
	LP_SDK_ENC_VIDEO_OVERLAY_CANVAS (*load_overlay_canvas)(const char *bmp24_path);
	void (*release_overlay_canvas)(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas);

	int (*create_overlay)(int vin, int stream, const char* overlay_name,
		float x, float y, LP_SDK_ENC_VIDEO_OVERLAY_CANVAS const canvas);
	int (*release_overlay)(int vin, int stream, const char* overlay_name);

	LP_SDK_ENC_VIDEO_OVERLAY_CANVAS (*get_overlay_canvas)(int vin, int stream, const char* overlay_name);

	int (*show_overlay)(int vin, int stream, const char* overlayName, bool showFlag);
	int (*update_overlay)(int vin, int stream, const char* overlay_name);
	int (*eptz_ctrl)(int vin, int stream, int cmd, int param);
	int (*enc_mode)(int vin, int stream, int fix_mode, int show_mode);
	void *(*upgrade_env_prepare)(void *param);
	int (*update_overlay_bytext)(int vin, int stream, const char* text);	
	int (*enc_resolution)(int width, int height);
    int (*snapshotToBuf)(int vin, enSDK_ENC_SNAPSHOT_QUALITY quality, ssize_t width, ssize_t height, char *buf, int *nLen);

}stSDK_ENC_API, *lpSDK_ENC_API;

// could be used after 'SDK_init_enc' success to call
extern lpSDK_ENC_API sdk_enc;

extern int SDK_ENC_init();
extern int SDK_ENC_destroy();
extern int SDK_ENC_wdr_destroy();
extern int SDK_ENC_create_stream(int vin, int stream, LP_SDK_ENC_STREAM_ATTR stream_attr);
extern int SDK_ENC_release_stream(int vin, int stream);
extern int SDK_ENC_set_stream(int vin, int stream,LP_SDK_ENC_STREAM_ATTR stream_attr);
extern int SDK_ENC_get_stream(int vin, int stream, LP_SDK_ENC_STREAM_ATTR stream_attr);
extern int SDK_ENC_enable_stream(int vin, int stream, bool flag);
extern int SDK_ENC_request_stream_keyframe(int vin, int stream);
extern int SDK_ENC_eptz_ctrl(int nChn, int enCmd, unsigned char u8Param);
extern int SDK_ENC_get_enc_pts(int vin, int stream, unsigned long long *encPts);

#if defined(HI3518E_V2)
/*
* @brief ‰∏ªË¶ÅÁî®Êù•ÁªôÂà∞sdkÂ±ÇÂΩìÂâçÁöÑËÆæÂ§áÂûãÂè∑ÔºåÂå∫ÂàÜPxÂíåCxÁöÑQPÂÄºÔºå
         Âú®SDK_ENC_initÂêéË∞ÉÁî®Ôºå‰∏çÂª∫ËÆÆÁî®‰∫éÂÖ∂ÂÆÉÂäüËÉΩ
* @param model emSDK_ENC_PRODUCT_MODEL_PX = 0,
               emSDK_ENC_PRODUCT_MODEL_CX,
*/


extern void SDK_ENC_set_model(int model);
#endif

#ifdef __cplusplus
};
#endif
#endif //SDK_ENC_H_

