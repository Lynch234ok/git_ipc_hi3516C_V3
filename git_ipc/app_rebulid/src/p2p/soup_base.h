#ifndef JAP2P_SOUP_BASE_H
#define JAP2P_SOUP_BASE_H

#include <pthread.h>
#include <sys/syscall.h>
#include "p2p.h"
#include "common.h"
#include "linkedlist.h"

#ifndef PLAT_X64
#include "media_buf.h"
#include "sdk/sdk_api.h"
#include "p2p_base/Crypto.h"  //crpto chip
#include "ptz.h"

#ifdef P2P_FOR_IPC
#include "usrm.h"
#define MAX_CAM_CH 1
#else
#include "user.h"
#include "conf.h" //include MAX_CAM_CH
#endif

#ifdef P2P_FOR_DVR
typedef SDK_ENC_BUF_ATTR_t stSDK_ENC_BUF_ATTR;
#endif  //end of P2P_DEVICE_DVR


#define SDK_ENC_BUF_DATA_H264 (0x00000000)
#define SDK_ENC_BUF_DATA_H265 (0x00000001)
#define SDK_ENC_BUF_DATA_JPEG (0x00000002)
		// audio
#define SDK_ENC_BUF_DATA_PCM (0x80000000)
#define SDK_ENC_BUF_DATA_G711A (0x80000001)
#define SDK_ENC_BUF_DATA_G711U (0x80000002)
#define SDK_ENC_BUF_DATA_AAC (0x80000003)

#endif

#endif  // end of soup_base.h
