//
// Created by WangJ on 2018/9/29.
// Copyright (c) 2018 JUAN. All rights reserved.
//

#ifndef OSSCLOUD_OSS_MSGPUSH_H
#define OSSCLOUD_OSS_MSGPUSH_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif



/**
* 上传模块初始化参数
*/

int KP2P_CloudMessageInit(const char *server, const char *eseeid);

int KP2P_CloudMessageDeinit();



/**
 * 媒体数据类型
 */
typedef enum {

    KP2P_CLOUD_MESSAGE_PIC_JPG,   ///< jpg 格式
    KP2P_CLOUD_MESSAGE_PIC_PNG,   ///< png 格式
    KP2P_CLOUD_MESSAGE_PIC_GIF,   ///< gif 格式
    KP2P_CLOUD_MESSAGE_PIC_CNT,
} enKP2P_CLOUD_MESSAGE_PIC_TYPE;

/**
 * @brief
 */

// msg_type 消息类型
//"bi":人体红外
//"lv":视频丢失
//"vo":视频遮挡
//"vm":视频遮盖
//"nd":找不到硬盘
//"de":硬盘错误
//"db":硬盘空间不足
//"re":录像错误
//"ds":门磁
//"sm":烟雾
//"rc":遥控器
//"bc_lowpower"
//"bc_tamper"
//"dk":门铃按键
//"other"

/**
 * @brief  推送一段图片缓存（一次搞定）到服务器，并推送报警
 * @param msg
 * @param msg_type
 * @param gmt_time
 * @param channel
 * @param buf
 * @param len
 * @param pic_type
 * @return
 */

int
KP2P_CloudMessagePush(const char *msg, const char *msg_type, time_t gmt_time, int channel,
                      const char *buf, int len, int pic_type);

/**
 * @brief 推送一张图片到服务器，并推送报警
 * @param msg
 * @param msg_type
 * @param gmt_time
 * @param channel
 * @param file
 * @param pic_type
 * @return
 */
int
KP2P_CloudMessagePushFile(const char *msg, const char *msg_type, time_t gmt_time, int channel,
                          const char *file, int pic_type);




#ifdef __cplusplus
}
#endif



#endif //OSSCLOUD_OSS_MSGPUSH_H
