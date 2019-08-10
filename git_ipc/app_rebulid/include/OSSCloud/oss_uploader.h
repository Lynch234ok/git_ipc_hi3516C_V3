/**
 * OSS 码流上传模块对外接口
 *
 * @file oss_uploader.h
 * @author Tong Lei
 * @date 2016-12-31
 *
 * 注： 库的使用者不能修改本头文件，并要确保头文件和库文件是匹配的
 *
 * 修改历史
 * --
 * 日期 | 修改者 | 说明
 * -- | -- | --
 * 2016-12-31 | Tong Lei | 添加 Doxygen 注释
 * 2017-01-06 | Tong Lei | 初始化添加时区参数
 * 2017-01-06 | Tong Lei | 添加 extern "C" 标记
 * 2017-01-12 | Tong Lei | 开始/结束上传、获取状态接口改名
 * 2017-07-27 | Tong Lei | 添加获取录像类型接口
 * 2018-01-22 | Tong Lei | 取帧回调接口增加帧率参数; 增加版本号宏和变量
 * 2018-01-24 | Tong Lei | 修正线程参数传递方式不合理的问题; option 也初始化 hls_list_size 变量; 更新 c media sdk 库： 同步上游最新代码; m3u8 中 version 字段由 4 改回 3
 * 2018-01-29 | Tong Lei | 调整云存文件前缀
 * 2018-02-05 | Tong Lei | 调整取帧回调接口; 更新 oss sdk 库，并修正ts文件时间戳的计算错误
 * 2018-02-06 | Tong Lei | token 根据过期时间定时获取，而不是每次上传录像都获取; 增加设备错误通知类型
 * 2018-02-07 | Tong Lei | 2.1.1; 封面图片异步上传
 * 2018-02-07 | Tong Lei | 2.1.2; 调整部分 初始化/去初始化 代码
 * 2018-02-09 | Tong Lei | 2.2.0; 增加获取矫正参数文件回调接口
 * 2018-02-10 | Tong Lei | 2.2.1; 修正 json 解析异常导致程序崩溃的问题
 * 2018-02-25 | Tong Lei | 2.2.2; 写帧失败会继续尝试而不是去开始新的录像; 封面、矫正参数文件上传失败会重试; 视频帧写入失败会重试; 修复获取 token 时没有将相关错误通过回调通知出去的问题
 * 2018-02-27 | Tong Lei | 2.2.3; 关闭上传流失败会重试
 * 2018-02-27 | Tong Lei | 2.2.4; 为避免异常，不再支持写帧、关闭上传流失败时的重试
 * 2018-02-28 | Tong Lei | 2.2.5; 设置请求 token 时的超时; 以 https 方式向 sts 服务器请求
 * 2018-03-22 | Tong Lei | 3.0.0; 调整接口名字; 调整 log 函数; 基本类型改用 C 标准类型; 增加对码流大小、ping值的记录功能
 */

#ifndef NK_OSS_UPLOADER_H
#define NK_OSS_UPLOADER_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 版本号结构体
 */
typedef struct {

    int major;
    int minor;
    int patch;

} NK_OSS_UPLD_LIB_VERSION;

/**
 * 版本号
 */
// 头文件版本号
#define NK_OSS_UPLD_LIB_VER_MAJOR 3
#define NK_OSS_UPLD_LIB_VER_MINOR 0
#define NK_OSS_UPLD_LIB_VER_PATCH 0
// 库文件版本号
extern const NK_OSS_UPLD_LIB_VERSION g_NK_OSS_UPLD_LIB_VERSION;

/**
 * 错误码
 */
typedef enum {

    NK_OSS_UL_ERRCODE_OK = 0,               ///< 操作成功
    NK_OSS_UL_ERRCODE_ERR,                  ///< 出现错误
    NK_OSS_UL_ERRCODE_DEV_NOT_BOUND         ///< 设备未绑定

} NK_OSS_UL_ERRCODE;

/**
 * 上传任务的打开或关闭状态
 */
typedef enum {

    NK_OSS_UPLOAD_INVALID_QUERY = -1,         ///< 无效查询。 通道号或码流号错误
    NK_OSS_UPLOAD_OPENED = 0,                 ///< 某个码流的上传已开启
    NK_OSS_UPLOAD_CLOSED,                     ///< 某个码流的上传已关闭
    NK_OSS_UPLOAD_CLOSED_BUT_UNFINISH         ///< 某个码流的上传已关闭，但仍有操作未完成

} NK_OSS_UploadStatus;

/**
 * 上传模块主动通知消息级别
 */
typedef enum {

    NK_OSS_UL_NTLV_DEBUG = 0,   ///< 通知级别为 Debug
    NK_OSS_UL_NTLV_INFO,    ///< 通知级别为 Info
    NK_OSS_UL_NTLV_WARN,    ///< 通知级别为 Warning
    NK_OSS_UL_NTLV_ERROR,   ///< 通知级别为 Error
    NK_OSS_UL_NTLV_ALERT    ///< 通知级别为 Alert

} NK_OSS_UL_NotifyLevel;

/**
 * 上传模块主动通知消息类型
 */
typedef enum {

    NK_OSS_UL_NT_ESEE_TOKEN_GOT = 0,    ///< 成功获取 token

    NK_OSS_UL_NT_ESEE_TOKEN_GET_ERROR,  ///< 获取 token 失败

    NK_OSS_UL_NT_ESEE_TOKEN_NOT_BOUND,  ///< 设备未绑定


    NK_OSS_UL_NT_OSS_OP_OK, ///< 对 OSS 的操作成功

    NK_OSS_UL_NT_OSS_OP_ERROR,  ///< 对 OSS 的操作出现了错误

    NK_OSS_UL_NT_OSS_AUTH_ERROR,    ///< 操作 OSS 时出现认证错误

    NK_OSS_UL_NT_OSS_NW_ERROR,  ///< 操作 OSS 是出现网络问题

    NK_OSS_UL_NT_OSS_UNSPPORT_DATATYPE,  ///< 不支持的上传数据类型

    NK_OSS_UL_NT_DEV_ERR  ///< 设备错误

} NK_OSS_UL_NotifyType;

/**
 * 媒体数据类型
 */
typedef enum {

    NK_OSS_UL_PL_UNSUPPORT = -1,    ///< 不支持的负载类型
    NK_OSS_UL_PL_JPG,   ///< jpg 格式
    NK_OSS_UL_PL_PNG,   ///< png 格式
    NK_OSS_UL_PL_H264,  ///< h264 格式
    NK_OSS_UL_PL_AAC    ///< acc 格式

} NK_OSS_UL_PL_TYPE;


/**
 * 当前支持的最大通道数.
 */
#define NK_OSS_UPLD_MAX_CH 16

/**
 * 每个通道当前支持的最大码流数
 */
#define NK_OSS_UPLD_MAX_S_PCH 2

/**
 * 当前设置的某个码流新的上传任务等待它前一个上传任务结束的超时时间。单位 毫秒
 * 每个码流同一时间只能有一个上传任务。
 */
#define NK_OSS_TIMEOUTMS_WAIT_PRE 2000

/**
 * OSS 上传模块主动通知回调函数。此接口阻塞时间需要尽量小
 *
 * @param ctx   [in] 保存回调函数需要的上下文。在调用 NK_OSS_StartUpload 的时候传入 OSS 上传模块。
 * @param notifyLevel   [in] 此条通知消息的级别
 * @param notifyType    [in] 此条通知消息的类型
 *
 * @return 回调函数应返回数据的长度。 无数据可读或获取数据失败应返回 0。
 */
typedef ssize_t (*NK_OSS_UploaderNotifyCB)(void * ctx,
                                           NK_OSS_UL_NotifyLevel notifyLevel,
                                           NK_OSS_UL_NotifyType notifyType);

/**
 * OSS 上传模块获取矫正参数 json 文本回调函数。需要保证数据缓冲区的有效期足够让模块上传完 json 文件
 *
 * @param ctx   [in] 保存回调函数需要的上下文。在调用 NK_OSS_StartUpload 的时候传入 OSS 上传模块。
 * @param pData [out] 矫正参数 json 文本数据（指针）。需要保证数据缓冲区的有效期足够让模块上传完 json 文件。
 *
 * @return 回调函数应返回数据的长度。 无数据可读或获取数据失败应返回 0。
 */
typedef ssize_t (*NK_OSS_UploaderGetCorJsonCB)(void * ctx,
                                               uint8_t **pData);

/**
 * OSS 上传模块获取图片回调函数。需要保证数据缓冲区的有效期足够让模块上传完图片
 *
 * @param ctx   [in] 保存回调函数需要的上下文。在调用 NK_OSS_StartUpload 的时候传入 OSS 上传模块。
 * @param pPayload_type [out] 图片类型（指针）
 * @param pData [out] 图片数据（指针）。需要保证数据缓冲区的有效期足够让模块上传完图片。
 *
 * @return 回调函数应返回数据的长度。 无数据可读或获取数据失败应返回 0。
 */
typedef ssize_t (*NK_OSS_UploaderGetCoverPicCB)(void * ctx,
                                                NK_OSS_UL_PL_TYPE *pPayloadType,
                                                uint8_t **pData);

/**
 * OSS 上传模块读帧回调函数。需要保证数据缓冲区的有效期足够让模块上传完帧
 *
 * @param ctx   [in] 保存回调函数需要的上下文。在调用 NK_OSS_StartUpload 的时候传入 OSS 上传模块。
 * @param pPayload_type [out] 数据帧类型（指针）
 * @param pData [out]帧数据（指针）。需要保证数据缓冲区的有效期足够让模块上传完帧。
 * @param pFrameTsMs [out]时间戳。
 * @param pFrameRate [out]视频帧率（指针），只有当负载类型为视频帧的时候才需要填上帧率。
 *
 * @return 回调函数应返回帧数据的长度。 无帧可读或获取帧失败应返回 0。
 */
typedef ssize_t (*NK_OSS_UploaderReadFrameCB)(void * ctx,
                                              NK_OSS_UL_PL_TYPE *pPayloadType,
                                              uint8_t **pData,
                                              uint64_t *pFrameTsMs,
                                              int32_t *pVideoFrameRate);

typedef ssize_t (*NK_OSS_UploaderAfterReadFrameCB)(void * ctx, uint8_t *pData);


/**
 * 上传模块初始化参数
 */
typedef struct {

    char * device_esee_id;    ///< 设备易视网 ID

    char * esee_server;   ///< 易视服务器域名或 IP

    int tz;     ///< 设备所在时区偏移秒数

    NK_OSS_UploaderGetCorJsonCB get_corjson_cb;   ///< 获取矫正参数 json 文本回调函数。设为 NULL 的话，则不会上传封面

    NK_OSS_UploaderGetCoverPicCB get_coverpic_cb;   ///< 获取封面图片回调函数。设为 NULL 的话，则不会上传封面

    NK_OSS_UploaderReadFrameCB read_frame_cb;   ///< 读帧回调函数。不可未空

    NK_OSS_UploaderNotifyCB uploader_notify_cb; ///< Uploader 主动通知回调函数。设为 NULL 的话，则不会有主动通知消息

    NK_OSS_UploaderAfterReadFrameCB  after_read_frame_cb;

} NK_OSS_UploaderInitParam;

/**
 * OSS 上传模块初始化(默认只开启1个通道)
 *
 * @param param 上传模块初始化参数
 * @return 0 成功, 非 0 失败
 */
extern int NK_OSS_UploaderInit(NK_OSS_UploaderInitParam *param);

/**
 * @brief OSS 上传模块初始化
 * @param param 上传模块初始化参数
 * @param max_ch 设备最大通道数
 * @return
 */
extern int NK_OSS_UploaderInit_V2(NK_OSS_UploaderInitParam *param, int max_ch);

/**
 * OSS 上传模块去初始化。
 *
 * 释放模块动态分配的资源。如果有正在运行的任务，则任务会退出。
 * 此接口等待任务退出时会因网络问题而阻塞。
 */
extern void NK_OSS_UploaderDeinit(void);



enum OSS_REC_TYPE {
    /* 定时录像 */
            OSS_REC_TYPE_TIME = (1 << 0),
    /* 移动侦测录像 */
            OSS_REC_TYPE_MOTION = (1 << 1),
    /* 报警录像 */
            OSS_REC_TYPE_ALARM = (1 << 2),
    /* 手动录像 */
            OSS_REC_TYPE_MANU = (1 << 3),
    /* 所有类型 */
            OSS_REC_TYPE_ALL = (OSS_REC_TYPE_TIME | OSS_REC_TYPE_MOTION | OSS_REC_TYPE_ALARM | OSS_REC_TYPE_MANU),
};

/**
 * 开始上传视频。
 *
 * 当指定码流有未完成任务，此接口等待任务退出时会阻塞，阻塞时间最长为 NK_OSS_TIMEOUTMS_WAIT_PRE + 因网络问题导致的接口阻塞时间。
 *
 * @param ctx   [in] 传入的上下文，用于回调
 * @param channel_id    [in] 通道 ID，从 0 开始
 * @param stream_id [in] 码流 ID，从 0 开始
 * @param rec_type  [in] 录像类型。 0，普通类型; 大于 0，未定义的其它类型
 * @return 0 成功, 非 0 失败
 */
extern int NK_OSS_StartUpload(void *ctx, uint32_t channel_id, uint32_t stream_id, int rec_type);

/**
* 结束上传视频
*
* @param channel_id [in] 通道 ID，从 0 开始
* @param stream_id  [in] 码流 ID，从 0 开始
* @return 0 成功, 非 0 失败
*/
extern int NK_OSS_StopUpload(uint32_t channel_id, uint32_t stream_id);

/**
* 获取上传状态
*
* @param channel_id [in] 通道 ID，从 0 开始
* @param stream_id  [in] 码流 ID，从 0 开始
* @return OSS 上传状态
*/
extern NK_OSS_UploadStatus NK_OSS_GetUploadStatus(uint32_t channel_id, uint32_t stream_id);

/**
 * 获取某个通道的录像类型
 *
 * @param channel_id [in] 通道 ID，从 0 开始
 * @patam pType [out] (指针) 通道的录像类型。 0: 移动侦测录像 1: 全天候录像。通道的录像类型与客户的云存套餐类型有关
 * @return 函数执行结果
 */
extern NK_OSS_UL_ERRCODE NK_OSS_GetRecType(int channel_id, int *pType);


#ifdef __cplusplus
}
#endif

#endif //NK_OSS_UPLOADER_H
