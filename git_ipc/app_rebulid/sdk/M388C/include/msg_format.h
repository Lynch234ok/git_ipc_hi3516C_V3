
/*
 *******************************************************************************
 *  Copyright (c) 2010-2015 VATICS Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VATICS INC.                                                     |
 *  +-----------------------------------------------------------------+
 *
 *******************************************************************************
 */ 
#ifndef MSG_FORMAT_H
#define MSG_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#define FECVENC_OSD_SIZE 64
#define FECVENC_PATH_SIZE 128
#define PARAM_UNSED	-65536

/*========== System =========*/
/**
 * @defgroup System 
 * @brief Data structure used for system operation.
 */
/*@{*/  /* Begin of System */

/**
 * @struct      msg_channel_t
 * @brief       The structure for the channel information.
 */
typedef struct
{
	int channel;	/*!< Output channel ID */
	uint32_t w;		/*!< Width */
	uint32_t h;		/*!< Height */
	uint32_t fps;	/*!< maximum capture fps */
	uint32_t gop;	/*!< Encoding gop */
} msg_channel_t;
/*@}*/  /* End of System */

/*========== Video Encoder =========*/
/**
 * @defgroup Video Encoder 
 * @brief Data structure used for video encoder operation.
 */
/*@{*/  /* Begin of Video Encoder */

/**
 * @struct      msg_video_t
 * @brief       The structure for the video encoder information.
 */
typedef struct
{
	/* Common */
	uint32_t type;				/*!< 1: H264, 2: MJPEG, 3: SVC-T */
	uint32_t fps;				/*!< Encoding fps */
	uint32_t qp;				/*!< qp */
	int32_t  mbqp;				/*!< mbqp */		

	/* H.264 only */
	uint32_t gop;				/*!< gop */
	int      bitrate;			/*!< bitrate control.  0:no bitrate constraint */	
	uint32_t advanced_mode;		/*!< 0:fps priority; 1:quality priority; 2:customized */ 
	
	/* For customized mode only */
	uint32_t min_qp;			/*!< The minimum value of qp */
	uint32_t max_qp;			/*!< The maximum value of qp */
	uint32_t min_fps;			/*!< The minimum value of fps */
} msg_video_t;

/**
 * @struct      msg_osd_t
 * @brief       The structure for saving the OSD string.
 */
typedef struct
{
	char str[FECVENC_OSD_SIZE];	/*!< The OSD string buffer */
} msg_osd_t;

/**
 * @struct      msg_roi_t
 * @brief       The structure for ROI window mode operation.
 */
typedef struct
{
	/* ROI window mode */
	uint32_t enable;		/*!< Enable ROI or not */
	uint32_t win_idx;		/*!< Window index: 0~7 */
	
	/* one macroblock is 16x16 pixels */
	uint32_t mb_start_x;	/*!< The x start position of the macroblock (unit in 16x16-pixels macroblock) */
	uint32_t mb_start_y;	/*!< The y start position of the macroblock (unit in 16x16-pixels macroblock) */
	uint32_t mb_end_x;		/*!< The x end position of the macroblock (unit in 16x16-pixels macroblock) */
	uint32_t mb_end_y;		/*!< The y end position of the macroblock (unit in 16x16-pixels macroblock) */
	
	uint32_t qp;			/*!< Delta QP */
	uint32_t enc_interval;	/*!< Encoding Interval */
} msg_roi_t;

/**
 * @struct      msg_df_t
 * @brief       The structure for dropframe info operation.
 */
typedef struct
{
	uint32_t skip_mode; /*!< skip mode (0: disable, 1: enable right away if in_cond matched, 2: enable at second gop if in_cond matched) */
	uint32_t in_cond_frames; /*!< enter df cond ( by frames) */
	uint32_t in_cond_bits;  /*!< enter df cond ( by bits) */
	uint32_t out_cond;	 	/*!< exit df cond (0: by bitrates, 1: by qp) */
}msg_df_t;

/**
 * @struct      msg_pds_t
 * @brief       The structure for prediction search operation.
 */
typedef struct
{
	uint32_t enable; /*!< prediction search mode (0: disable, 1: enable) */
}msg_pds_t;

/*@}*/  /* End of Video Encoder */


/*========== ISPE =========*/
/**
 * @defgroup ISPE 
 * @brief Data structure used for ISPE operation.
 */
/*@{*/  /* Begin of ISPE */

/**
 * @struct      msg_rt_t
 * @brief       The structure for rotation mode.
 */
typedef struct
{
	uint32_t mode;		/*!< Rotation mode: 0: Normal, 1: 90, 2: 270 */
} msg_rt_t;

/**
 * @struct      msg_md_t
 * @brief       The structure for the window-mode motion detection.
 */
typedef struct
{
	uint32_t enable;		/*!< Enable motion detection or not */
	uint32_t idx;			/*!< Window index: 0~15 */
	uint32_t ops;			/*!< Operation: 0: ADD_WINDOW, 1: DELETE_WINDOW, 2: SET_WINDOW */
	uint32_t start_x;		/*!< The x coordinate of the start position */
	uint32_t start_y;		/*!< The y coordinate of the start position */
	uint32_t w;				/*!< The width of the motion window */
	uint32_t h;				/*!< The height of the motion window */
	uint32_t obj_size;		/*!< The objective size */// objective size
	uint32_t sensitivity;	/*!< The sensitivity */
} msg_md_t;

/**
 * @struct      msg_pm_t
 * @brief       The structure for the primacy mask.
 */
typedef struct
{
	uint32_t enable;	/*!< Enable primacy mask or not */
	uint32_t start_x;	/*!< The x coordinate of the start position */
	uint32_t start_y;	/*!< The y coordinate of the start position */
	uint32_t w;			/*!< The width of the primacy mask */
	uint32_t h;			/*!< The height of the primacy mask */
} msg_pm_t;

/**
 * @struct      msg_fec_t
 * @brief       The structure for the fisheye correction.
 */
typedef struct
{
	/* fisheye correction */
	int channel;		/*!< The output channel ID */
	int mode;			/*!< 0:ori, 1:p360, 2:p180_a, 3:p180_s, 4:p180_t, 5:6r, 6:4r, 7:1p2r, 8:1o3r, 9: sphere, 10:1r, 11:p360_s */
	float pan;			/*!< Pan (reserved): value range depends on de-warping mode, please refer to coeff_gen.h */
	float tilt;			/*!< Tilt (reserved): value range depends on de-warping mode, please refer to coeff_gen.h */
	float zoom;			/*!< Zoom (reserved) value range depends on de-warping mode, please refer to coeff_gen.h */
	float focal;		/*!< Focal: value range depends on de-warping mode, please refer to coeff_gen.h */
	
	int   cell_idx;		/*!< Index of a certain partition of the layout, range: depends on layout */
	int   step;     	/*!< The step count. -1 means infinite or positive integer */
	float pan_step;		/*!< The step unit of pan */
	float tilt_step;	/*!< The step unit of tilt */
	float zoom_step;	/*!< The step unit of zoom */
	float curvature;	/*!< The curvature */
	float slope;		/*!< The slope */
	
	int   app_type;		/*!< The applied type: 0: ceiling, 1: table, 2: wall */

	int lens;			/*!< 0: LENS_STEREOGRAPHIC, 1: LENS_EQUISOLIDANGLE, 2: LENS_EQUIDISTANT, 3: LENS_ORTHOGRAPHIC, 5: LENS_NODISTORT */
	float dst_offset_x;	/*!< The destination offset in the x direction. */
	float dst_offset_y; /*!< The destination offset in the y direction. */	
	float dst_xy_ratio; /*!< The destination x/y ratio. Range: 0.1 ~ 2 */

	int enable_object_tracking;
} msg_fec_t;

/**
 * @struct      msg_ctrl_output_t
 * @brief       The structure for controlling the channel output.
 */
typedef struct
{
	int channel;		/*!< Output channel ID want to be controlled */
	char ctrl_value;	/*!<  0: pause, 1: resume */
} msg_ctrl_output_t;

/**
 * @struct      msg_snapshot_t
 * @brief       The structure for controlling the channel to get snapshot.
 */
typedef struct
{
	int channel;		/*!< Output channel ID want to be controlled */
	char path[FECVENC_PATH_SIZE];	/*!<  path to save *.jpg file */
	int jpeg_qp;
} msg_snapshot_t;
/*@}*/  /* End of ISPE */

/*========== Autoscene =========*/
/**
 * @defgroup Autoscene 
 * @brief Data structure used for autoscene operation.
 */
/*@{*/  /* Begin of Autoscene */

/**
 * @struct      msg_ae_gain_t
 * @brief       The structure for the AE gain control.
 */
typedef struct
{
	unsigned int min_gain;	/*!< The min gain for auto exposure. (1000~512000) default: 1000 */
	unsigned int max_gain;	/*!< The max gain for auto exposure. (1000~512000) default: 32000 */
} msg_ae_gain_t;

/**
 * @struct      msg_ae_shutter_speed_t
 * @brief       The structure for the AE shutter speed.
 */
typedef struct
{
	unsigned int min_speed;	/*!< The min shutter speed for auto exposure. (1000000~30) default: 1000000 */
	unsigned int max_speed;	/*!< The max shutter speed for auto exposure. (1000000~30) default: 30 */
} msg_ae_shutter_speed_t;

/**
 * @struct      msg_nr_t
 * @brief       The structure for the noise reduction.
 */
typedef struct
{
	unsigned int mode;				/*!< Enable 3D NR or not. Default is enabled (1).  */
	unsigned int strength_level_2d;	/*!< 2D NR level: 0~200. Default: 100  */
	unsigned int strength_level_3d;	/*!< 3D NR level: 0~200. Default: 100  */
} msg_nr_t;

/**
 * @struct      msg_wb_t
 * @brief       The structure for the white balance.
 */
typedef struct
{
	unsigned int mode;		/*!< The mode of WB: auto:0, full:1, user:2. Default: 0  */
	unsigned int gain_r;	/*!< The Gain of R: 1~8191. Default: 1024  */
	unsigned int gain_b;	/*!< The Gain of B: 1~8191. Default: 1024  */
} msg_wb_t;
/*@}*/  /* End of Autoscene */

/*========== Audio =========*/
/**
 * @defgroup Audio 
 * @brief Data structure used for audio operation.
 */
/*@{*/  /* Begin of Audio */

/**
 * @struct      msg_audio_t
 * @brief       The structure for the audio control.
 */
typedef struct
{
	uint32_t type;		/*!< The type of audio codec: 0: AAC; 1: G.711 u-law; 2: G.711 a-law; 3: G.726 */
	uint32_t bitrate;	/*!< The bitrate: 0: 8 Kbps; 1: 16 Kbps; 2: 24 Kbps; 3: 32 Kbps; 4: 40 Kbps */
} msg_audio_t;
/*@}*/  /* End of Audio */

#ifdef __cplusplus
}
#endif

#endif
