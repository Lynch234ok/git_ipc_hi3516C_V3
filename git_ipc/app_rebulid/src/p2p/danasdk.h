#ifndef _P2PSDK_H
#define _P2PSDK_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef KP2P_API_EXPORTS
#define KP2P_API __declspec(dllexport)
#else
#define KP2P_API __declspec(dllimport)
#endif
#define ALIGNED(n) __declspec(align(n))
#else
#define KP2P_API
#define ALIGNED(n) __attribute__((aligned(n)))
#endif

#ifdef __MACH__
#include <sys/types.h>
#endif
#ifdef _WIN32
typedef int ssize_t;
#endif
#if defined(__MACH__) || defined(__unix__)
#include <sys/socket.h>
#elif defined(_WIN32)
#include <Windows.h>
#endif
#include <stdint.h>
#include <stdlib.h>


//========================================================
//  macro definitions
//========================================================
/* P2PSDK错误码 */
#define P2PSDK_ERR_INVALID_PARAM    -1
#define P2PSDK_ERR_ALREADY_INIT     -2

/* P2PSDK支持最大通道数 */
#define P2PSDK_MAX_CHANNEL          64
/* P2PSDK支持的最大码流数 */
#define P2PSDK_MAX_STREAM           4
/* P2PSDK支持的摄像头描述长度 */
#define P2PSDK_CAM_DES_STRLEN       32

/* P2PSDK日志级别宏 */
#define P2PSDK_EMERG     4
#define P2PSDK_CRIT      4
#define P2PSDK_ALERT     4
#define P2PSDK_ERR       3
#define P2PSDK_WARN      2
#define P2PSDK_NOTICE    1
#define P2PSDK_INFO      1
#define P2PSDK_TRACE     0


/* 帧头幻数"FRAM" */
#define P2PSDK_FRAMEHEAD_MAGIC      0x4652414d
#define P2PSDK_FRAMEHEAD_MAGIC2     0x4652414E
#define P2PSDK_FRAMEHEAD_VERSION    0x01000000


/* 透明通道服务sock类型 */
#define P2PSDK_VCON_TCP         0
#define P2PSDK_VCON_UDP         1
#define P2PSDK_VCON_UNIX_STREAM 2
#define P2PSDK_VCON_UNIX_DGRAM  3


//========================================================
//  enum declarations
//========================================================
/* 错误码 */
typedef enum kp2p_error_code_e {
    /* 成功 */
    KP2P_ERR_SUCCESS = 0,

    /* 超时错误码 */
    KP2P_ERR_TIMEOUT = -2,

    /* 连接断开错误码 */
    KP2P_ERR_CLOSE_BY_SELF = -10,
    KP2P_ERR_CLOSE_BY_PEER = -11,

    /* 用户校验错误码 */
    KP2P_ERR_AUTH_FAILED = -20,
    KP2P_ERR_GET_NONCE_FAILED = -21,
    KP2P_ERR_AUTH2_FAILED = -22,

    /* PTZ错误码 */
    KP2P_ERR_PTZ_CTRL_FAILED = -30,

    /* 预览错误码 */
    KP2P_ERR_OPEN_STREAM_FAILED = -40,

    /* 回放错误码 */
    KP2P_ERR_REC_SEARCH_FAILED = -50,
    KP2P_ERR_REC_PLAY_FAILED = -51,

    /* 语音错误码 */
    KP2P_ERR_VOP2P_CALL_FAILED = -60,

    /* 远程设置错误码 */
    KP2P_ERR_REMOTE_SETUP_FAILED = -70,

} kp2p_error_code_t;


/* 设备类型　*/
typedef enum kp2p_device_type_e {
    KP2P_DEVICE_IPC = 1,
    KP2P_DEVICE_NVR = 2,
    KP2P_DEVICE_DVR = 3
} kp2p_device_type_t;


typedef enum kp2p_ptz_control_action_e {
    /* 停止云台操作 */
    KP2P_PTZ_CONTROL_ACTION_STOP = 0,

    /* 自动水平旋转 */
    KP2P_PTZ_CONTROL_ACTION_AUTO,

    /* 云台方向操作 */
    KP2P_PTZ_CONTROL_ACTION_UP,
    KP2P_PTZ_CONTROL_ACTION_DOWN,
    KP2P_PTZ_CONTROL_ACTION_LEFT,
    KP2P_PTZ_CONTROL_ACTION_RIGHT,

    /* 光圈操作 */
    KP2P_PTZ_CONTROL_ACTION_IRIS_OPEN,
    KP2P_PTZ_CONTROL_ACTION_IRIS_CLOSE,

    /* 镜头调焦操作 */
    KP2P_PTZ_CONTROL_ACTION_ZOOM_IN,
    KP2P_PTZ_CONTROL_ACTION_ZOOM_OUT,

    /* 镜头变倍操作 */
    KP2P_PTZ_CONTROL_ACTION_FOCUS_F,
    KP2P_PTZ_CONTROL_ACTION_FOCUS_N,

    /* 辅助开关 */
    KP2P_PTZ_CONTROL_ACTION_AUX,

    /* 预置点操作　*/
    KP2P_PTZ_CONTROL_ACTION_PRESET_SET,
    KP2P_PTZ_CONTROL_ACTION_PRESET_GO,
    KP2P_PTZ_CONTROL_ACTION_PRESET_CLEAR

} kp2p_ptz_control_action_t;


/* P2P帧头类型 */
enum P2P_FRAMEHEAD_HEADTYPE {
    /* 直播类型帧头 */
    P2P_FRAMEHEAD_HEADTYPE_LIVE,
    /* 回放类型帧头 */
    P2P_FRAMEHEAD_HEADTYPE_REPLAY,
    /* 语音对讲类型帧头 */
    P2P_RRAMEHEAD_HEADTYPE_VOP2P,
    /* 报警信息帧头 */
    P2P_FRAMEHEAD_HEADTYPE_ALARMMSG,
};


/* P2P帧类型 */
enum P2P_FRAMEHEAD_FREAMETYPE {
    /* 音频帧 */
    P2P_FRAMEHEAD_FREAMETYPE_AUDIO,
    /* 音频帧 duplicate*/
    P2P_FRAMEHEAD_FRAMETYPE_AUDIO = 0,
    /* 视频I帧 */
    P2P_FRAMEHEAD_FRAMETYPE_IFRAME,
    /* 视频P帧 */
    P2P_FRAMEHEAD_FRAMETYPE_PFRAME,
    /* 带外数据帧 */
    P2P_FRAMEHEAD_FRAMETYPE_OOB = 15,
};


/* 录像类型 */
enum P2P_REC_TYPE {
    /* 定时录像 */
    P2P_REC_TYPE_TIME = (1 << 0),
    /* 移动侦测录像 */
    P2P_REC_TYPE_MOTION = (1 << 1),
    /* 报警录像 */
    P2P_REC_TYPE_ALARM = (1 << 2),
    /* 手动录像 */
    P2P_REC_TYPE_MANU = (1 << 3),
    /* 所有类型 */
    P2P_REC_TYPE_ALL = (P2P_REC_TYPE_TIME | P2P_REC_TYPE_MOTION | P2P_REC_TYPE_ALARM | P2P_REC_TYPE_MANU),
};


/* 报警消息类型 */
enum P2P_ALARMMSG_TYPE {
    /* 文本信息 */
    P2P_ALARMMSG_TYPE_TEXT = 0,
    /* 音频信息 */
    P2P_ALARMMSG_TYPE_AUDIO = 1,
    /* 小图片信息 */
    P2P_ALARMMSG_TYPE_SMALL_IMG = 2,
    /* 常规大小图片 */
    P2P_ALARMMSG_TYPE_IMG = 3,
    /* 较大图片信息 */
    P2P_ALARMMSG_TYPE_LARGE_IMG = 4,
    /* 视频信息 */
    P2P_ALARMMSG_TYPE_VIDEO = 5,
};


//========================================================
//  structure declarations
//========================================================
/* P2P设备信息结构 */
struct DeviceInfo {
    /* 设备序列号 */
    char sn[32];
    /* 设备类型 */
    kp2p_device_type_t type;
    /* 设备通道数 */
    uint32_t max_ch;
    /* 产品型号、设备类型 */
    char model[32];
    /* 设备版本号 */
    char version[32];
    /* 设备硬件平台代码 */
    char hwcode[32];
    /* 设备厂商 */
    char vendor[16];
    /* 安装类型: 0-无 1-吊顶 2-壁挂 */
    uint32_t install_type;
    /* 云录像支持: 0-不支持 1-支持 */
    uint32_t cloud_record;
    /* 区域码 */
    char area[4];
    /* 通道摄像头描述 */
    char cam_des[P2PSDK_MAX_CHANNEL][P2PSDK_CAM_DES_STRLEN];
    /* 当前时区与GMT时间秒的偏移 */
    long tm_gmtoff;
};


/* 直播帧头 */
typedef struct kp2p_live_head_s {
    /* 帧类型，0：音频帧，1：视频I帧，2：视频P帧 */
    uint32_t frametype;
    /* 通道 */
    uint32_t channel;
    /* 视频音频 */
    union {
        struct {
            /* 视频编码 "H264"/"H265" */
            char enc[8];
            /* 视频帧率 */
            uint32_t fps;
            /* 视频宽 */
            uint32_t width;
            /* 视频高 */
            uint32_t height;
        } v;
        struct {
            /* 音频编码 "G711A" */
            char enc[8];
            /* 采样率 */
            uint32_t samplerate;
            /* 采样位宽 */
            uint32_t samplewidth;
            /* channels 默认 1, 单声道 */
            uint32_t channels;
            /* compresss 压缩率, G711 == 2.0 */
            float compress;
        } a;
    };
} ALIGNED(4) kp2p_live_head_t;


/* 回放帧头 */
typedef struct kp2p_replay_head_s {
    /* 帧类型，0：音频帧，1：视频I帧，2：视频P帧 */
    uint32_t frametype;
    /* 录像码流对应的通道 */
    uint32_t channel;
    /* 录像码流的录像类型 */
    uint32_t type;
    /* 录像码流质量 */
    uint32_t quality;
    union {
        struct {
            /* 视频编码 "H264"/"H265" */
            char enc[8];
            /* 视频帧率 */
            uint32_t fps;
            /* 视频宽 */
            uint32_t width;
            /* 视频高 */
            uint32_t height;
        } v;
        struct {
            /* 音频编码 "G711A"*/
            char enc[8];
            /* 采样率 */
            uint32_t samplerate;
            /* 采样位宽 */
            uint32_t samplewidth;
            /* channels 默认 1, 单声道 */
            uint32_t channels;
            /* compresss 压缩率, G711 == 2.0 */
            float compress;
        } a;
    };
} ALIGNED(4) kp2p_replay_head_t;


/* 告警帧头 */
typedef struct kp2p_alarm_head_s {
    /* 信息类型 */
    uint32_t type;
    /* 通道 */
    uint32_t channel;
    /* 视频音频 */
    union {
        struct {
            /* 视频编码 "H264"/"H265" */
            char enc[8];
            /* 视频帧率 */
            uint32_t fps;
            /* 视频宽 */
            uint32_t width;
            /* 视频高 */
            uint32_t height;
        } v;
        struct {
            /* 音频编码 "G711A" */
            char enc[8];
            /* 采样率 */
            uint32_t samplerate;
            /* 采样位宽 */
            uint32_t samplewidth;
            /* channels 默认 1, 单声道 */
            uint32_t channels;
            /* compresss 压缩率, G711 == 2.0 */
            float compress;
        } a;
    };
} ALIGNED(4) kp2p_alarm_head_t;


/* P2P帧头 */
typedef struct kp2p_frame_head_s {
    /* 带帧序号的版本 */
    uint32_t magic2;
    /* 帧序号 */
    uint32_t frame_seq;
    /* 保留字段 */
    uint8_t reserve[32];
    /* magic number 固定为 P2PSDK_FRAMEHEAD_MAGIC */
    uint32_t magic;
    /* 版本信息固定为 P2PSDK_FRAMEHEAD_VERSION */
    uint32_t version;
    /* 头类型， 0：直播, 1:录像 */
    uint32_t headtype;
    /* 帧的裸数据长度(Byte) */
    uint32_t framesize;
    /* 时间戳 */
    uint64_t ts_ms;
    union {
        /* 直播帧 */
        kp2p_live_head_t live;
        /* 回放帧 */
        kp2p_replay_head_t replay;
        /* 双向语音帧 */
        kp2p_live_head_t vop2p;
        /* 报警帧 */
        kp2p_alarm_head_t alarm;
    };
} ALIGNED(4) kp2p_frame_head_t;


/**
 * @brief 录像记录
 */
struct rec_record {
    /* 录像通道号，从0算起 */
    uint32_t chn;
    /* 录像类型 */
    uint32_t type;
    /* 录像开始时间，GMT */
    uint32_t startTime;
    /* 录像结束时间, GMT */
    uint32_t endTime;
    /* 录像质量 */
    uint32_t quality;
} ALIGNED(4);


/**
 * @brief 录像记录列表
 */
typedef struct rec_list {
    /* 此次请求的记录索引 */
    uint32_t recordIdx;
    /* 最大可填充录像记录数 */
    uint32_t maxRecord;
    /* 实际匹配的记录数 */
    uint32_t recordCnt;
    /* 满足要求的总记录数 */
    uint32_t recordTotal;
    /* 记录填充缓存 */
    struct rec_record *pRecord;
} RecList;

/**
 * @brief 录像搜索条件
 */
typedef struct kp2p_find_file_cond_s {
    /* 查找通道 */
    uint32_t channel;
    /* 类型 */
    uint32_t type;
    /* 开始时间 */
    time_t start_time;
    /* 结束时间 */
    time_t stop_time;
} kp2p_find_file_cond_t;

/**
 * @brief 录像搜索结果
 */
typedef struct kp2p_find_file_data_s {
    /* 录像通道 */
    uint32_t channel;
    /* 开始时间 */
    time_t start_time;
    /* 结束时间 */
    time_t stop_time;
    /* 录像文件大小 */
    uint32_t file_size;
    /* 录像文件类型 */
    uint32_t file_type;
} kp2p_find_file_data_t;


/**
 * @brief P2P设备抽象
 */
struct P2PSDKDevice {
    /* 设备信息 */
    struct DeviceInfo info;

    /* 用户上下文 */
    void *ctx;

    /**
     * @brief 设备上线回调
     * @param ctx               用户上下文
     * @param eid               易视网ID
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnDevOnline)(void *ctx, const char *eid);

    /**
     * @brief 设备下线回调
     * @param ctx               用户上下文
     * @param eid               易视网ID
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnDevOffline)(void *ctx, const char *eid);

    /**
     * @brief 用户校验回调
     * @param ctx               用户上下文
     * @param name              用户名
     * @param password          密码
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnAuth)(void *ctx, const char *name, const char *password);

    /**
     * @brief 独占模式获取nonce回调
     * @param ctx               用户上下文
     * @param nonce_buff        存储nonce的缓冲
     * @param buff_size         缓冲长度
     * @return                  成功  0
     *                          失败  -1
     */
    ssize_t (*OnGetNonce)(void *ctx, char *nonce_buff, size_t buff_size);

    /**
     * @brief 独占模式用户校验回调
     * @param ctx               用户上下文
     * @param auth_info         校验信息
     * @param signature         签名
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnAuth2)(void *ctx, const char *auth_info, const char *signature);

    /**
     * @brief 云台控制回调
     * @param ctx               用户上下文
     * @param channel           通道
     * @param action            具体动作(e.g. KP2P_PTZ_CONTROL_ACTION_UP)
     * @param param1            参数1
     * @param param2            参数2
     * @return                  成功  0
     *                          失败  -1
     * 控制动作                              描述    			参数1                   参数2
     * KP2P_PTZ_CONTROL_ACTION_STOP         停止云台运动      无                      无
     * KP2P_PTZ_CONTROL_ACTION_AUTO         自动水平旋转		开启或停止自动，取值1或者0  无
     * KP2P_PTZ_CONTROL_ACTION_UP			控制云台向上运动	运动速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_DOWN			控制云台向下运动	运动速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_LEFT			控制云台向左运动	运动速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_RIGHT        控制云台向右运动	运动速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_IRIS_OPEN    控制光圈打开		镜头速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_IRIS_CLOSE   控制光圈关闭		镜头速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_ZOOM_IN		调整远焦			镜头速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_ZOOM_OUT     调整近焦			镜头速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_FOCUS_F		调整变倍大		镜头速度，取值0~7			无
     * KP2P_PTZ_CONTROL_ACTION_FOCUS_N		调整变倍小		镜头速度，取值0~7         无
     * KP2P_PTZ_CONTROL_ACTION_AUX			辅助开关1		    开关号0/1                打开或关闭，取值1或0
     * KP2P_PTZ_CONTROL_ACTION_PRESET_SET   设置预置点位置    预置点                   无
     * KP2P_PTZ_CONTROL_ACTION_PRESET_GO    移动到预置点      预置点                  　无
     * KP2P_PTZ_CONTROL_ACTION_PRESET_CLEAR 清除预置点位置    预置点                   无
     */
    int (*OnPtzCtrl)(void *ctx, uint32_t channel, uint32_t action, uint32_t param1, uint32_t param2);

    /**
     * @brief 带宽改变回调
     * @param bandwidth         建议带宽值
     */
    void (*OnBandwidthChanged)(float bandwidth);

    /**
     * @brief 码流建立请求回调
     * @param ctx               用户上下文
     * @param channel           通道
     * @param stream            码流
     * @return                  成功  有效的直播上下文
     *                          失败  NULL
     */
    void *(*OnAttachStream)(void *ctx, uint32_t channel, uint32_t stream);

    /**
     * @brief 强制I帧回调
     * @param ctx               用户上下文
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnForceIFrame)(void *live_ctx);

    /**
     * @brief 读取帧回调
     * @param live_ctx          直播上下文
     * @param frame             用于存储具体裸帧的数据地址指针
     * @return                  成功  有效的帧数据长度(包括0)
     *                          失败  -1
     */
    ssize_t (*OnReadFrame)(void *live_ctx, void **frame);

    /**
     * @brief 读取帧后处理回调
     * @param live_ctx          直播上下文
     * @param frame             存储具体裸帧的数据地址
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnAfterReadFrame)(void *live_ctx, void *frame);

    /**
     * @brief 码流释放回调
     * @param live_ctx          直播上下文
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnDetachStream)(void *live_ctx);

    /**
     * @brief 录像搜索回调
     * @param ctx               用户上下文
     * @param chn_cnt           通道数
     * @param chns              通道列表(e.g. {0, 1, 2, 3})
     * @param type              录像类型
     * @param begin_time        开始时间
     * @param end_time          结束时间
     * @param quality           录像质量
     * @param lists             存放录像结果列表,用于回调返回
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnFetchRecList)(void *ctx, uint32_t chn_cnt, const uint8_t *chns, uint32_t type, time_t begin_time, time_t end_time, uint32_t quality, RecList *lists);

    /**
     * @brief 查找录像文件开始回调
     * @param ctx               用户上下文
     * @param cond              查找条件
     * @return                  成功  有效的查找上下文
     *                          失败  NULL
     */
    void *(*OnFindFileStart)(void *ctx, kp2p_find_file_cond_t *cond);

    /**
     * @brief 查找下一条录像记录回调
     * @param find_ctx          查找上下文
     * @param file_data         录像数据存储区
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnFindNextFile)(void *find_ctx, kp2p_find_file_data_t *file_data);

    /**
     * @brief 录像查找关闭回调
     * @param find_ctx          查找上下文
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnFindFileStop)(void *find_ctx);

    /**
     * @brief 打开录像回调
     * @param ctx               用户上下文
     * @param chn_cnt           通道数
     * @param chns              通道列表(e.g. {0, 1, 2, 3})
     * @param type              录像类型
     * @param begin_time        开始时间
     * @param end_time          结束时间
     * @param quality           录像质量
     * @return                  成功  有效的录像播放上下文
     *                          失败  NULL
     */
    void *(*OnOpenRecFile)(void *ctx, uint32_t chn_cnt, const uint8_t *chns, uint32_t type, time_t begin_time, time_t end_time, uint32_t quality);

    /**
     * @brief 打开录像回调
     * @param ctx               用户上下文
     * @param chn_cnt           通道数
     * @param chns              通道列表(e.g. {0, 1, 2, 3})
     * @param type              录像类型
     * @param begin_time        开始时间
     * @param end_time          结束时间
     * @param quality           录像质量
     * @param open_type         打开类型: 0 录像回放 1 录像下载
     * @return                  成功  有效的录像播放上下文
     *                          失败  NULL
     */
    void *(*OnOpenRecFile2)(void *ctx, uint32_t chn_cnt, const uint8_t *chns, uint32_t type, time_t begin_time, time_t end_time, uint32_t quality, uint32_t open_type);

    /**
     * @brief 录像帧读取回调
     * @param replay_ctx        录像播放上下文
     * @param frame             用于存储具体裸帧的数据地址指针
     * @return                  成功  有效的帧数据长度(包括0)
     *                          失败  -1
     */
    ssize_t (*OnReadRecFrame)(void *replay_ctx, void **frame);

    /**
     * @brief 录像帧读取后处理回调
     * @param replay_ctx        录像播放上下文
     * @param frame             存储具体裸帧的数据地址
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnAfterReadRecFrame)(void *replay_ctx, void *frame);

    /**
     * @brief 关闭录像回调
     * @param replay_ctx        录像播放上下文
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnCloseRecFile)(void *replay_ctx);

    /**
     * @brief 语音拨号回调
     * @param ctx               用户上下文
     * @param session           连接会话
     * @param channel           通道
     * @param errno             错误码(e.g. 打开语音失败被设置成-1)
     * @return                  成功  有效的语音通话上下文
     *                          失败  NULL
     */
    void *(*OnVoP2PCall)(void *ctx, void *session, uint32_t channel, int *errno);

    /**
     * @brief 语音数据接收回调
     * @param talk_ctx          语音通话上下文
     * @param frame_head        语音数据帧头
     * @param voice_data        语音数据
     * @param voice_data_size   语音数据长度
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnVoP2PTalkRecv)(void *talk_ctx, kp2p_frame_head_t *frame_head, const void *voice_data, size_t voice_data_size);

    /**
     * @brief 语音挂起回调
     * @param talk_ctx          语音通话上下文
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnVoP2PHangup)(void *talk_ctx);

    /**
     * @brief 报警拉取回调
     * @param ctx               用户上下文
     * @param session           连接会话
     * @param chn               通道
     * @param ticket            时间节点
     * @param type              报警类型
     * @return                  成功  0
     *                          失败  -1
     */
    int (*OnPullAlarmMsg)(void *ctx, void *session, uint32_t chn, time_t ticket, uint32_t type);

    /**
     * @brief 远程设置回调
     * @param ctx               用户上下文
     * @param req_data          远程设置请求的数据
     * @param req_data_size     远程设置请求的数据长度
     * @param rsp_buf           存放响应的数据缓冲
     * @param rsp_buf_size      存放响应的数据缓冲长度
     * @return                  成功  有效的响应数据长度
     *                          失败  -1
     */
    ssize_t (*OnRemoteSetup)(void *ctx, const char *req_data, size_t req_data_size, void *rsp_buf, size_t rsp_buf_size);

    /**
     * @param ctx               用户上下文
     * @param ssid              WiFI名字
     * @param password          WiFi密码
     *                          成功  0
     *                          失败  -1
     */
    int (*OnSmartlinkSetWifi)(void *ctx, const char *ssid, const char *password);

    /**
     * @param ctx               用户上下文
     * @param rom_size          固件文件大小
     * @param rom_ver           固件版本
     * @param save_pathname     固件存放的路径，输出参数
     *                          成功  返回0
     *                          失败  其他代码
     */
    uint32_t (*OnNewRomCome)(void *ctx, const uint64_t rom_size, const char *rom_ver, char save_pathname[512]);

    /**
     * @param ctx               用户上下文
     * @param code              状态码，0成功
     * @param rom_pathname      固件存放路径
     */
    void (*OnRomDownloadComplete)(void *ctx, const uint32_t code, const char *rom_pathname);

    /**
     * @param ctx               用户上下文
     * @param rom_pathname      固件存放路径
     * @param upgrade_timeout   固件更新超时，单位秒
     */
    void (*OnUpgradeConfirm)(void *ctx, const char *rom_pathname, uint32_t *upgrade_timeout);

    /**
     * @param ctx               用户上下文
     * @param channel           通道
     */
    void (*OnCloudRecordStart)(void *ctx, uint32_t channel);
};


/**
 * @brief 虚拟通道地址生成器
 */
typedef int (*fAddrGenerator)(struct sockaddr *app_service_addr);



//========================================================
//  API declarations
//========================================================
/**
 * @brief 获取P2PSDK版本号
 * @return                  版本号字符串
 */
KP2P_API const char* P2PSDKGetVersion(void);


/**
 * @brief 设置日志输出路径
 * @param path              路径字符串
 * @param file_size         日志文件最大大小(byte)
 * @return                  成功  0
 *                          失败  -1
 */
KP2P_API int P2PSDKSetLogPath(const char *path, int file_size);


/**
 * @brief 设置P2PSDK日志级别
 * @param level             日志级别
 * @return                  成功  0
 *                          失败  P2PSDK_ERR_INVALID_PARAM 参数错误
 */
KP2P_API int P2PSDKSetLogLevel(int level);


/**
 * @brief 初始化P2PSDK
 * @param p_device          设备结构指针
 * @return                  成功  0
 *                          失败  P2PSDK_ERR_INVALID_PARAM 参数错误
 */
KP2P_API int P2PSDKInit(struct P2PSDKDevice *p_device);


/**
 * @brief 初始化P2PSDK
 * @param device          设备结构指针
 * @param danale_path       大拿设备配置文件路径
 * @param agent_user        代理帐号
 * @param agent_pass        代理密码
 * @param chip_code         芯片代码
 * @param scheme_code       方案代码
 * @param product_code      产品代码
 * @return                  成功  0
 *                          失败  P2PSDK_ERR_INVALID_PARAM 参数错误
 */
KP2P_API int P2PSDKInit2(struct P2PSDKDevice *device,
                         const char *danale_path,
                         const char *agent_user, const char *agent_pass,
                         const char *chip_code, const char *scheme_code, const char *product_code);


/**
 * @brief 反初始化P2PSDK
 * @return                  成功  0
 *                          失败
 */
KP2P_API int P2PSDKDeinit(void);


/**
 * @brief 获取设备ID
 * @return                  设备ID
 */
KP2P_API const char *P2PSDKGetDeviceID(void);


/**
 * @brief 开始smartlink
 * @param if_name           WiFi网卡名字，e.g. wlan0
 * @return                  成功  0
 *                          失败  -1
 */
KP2P_API int P2PSDKSmartlinkStart(const char *if_name);


/**
 * @brief 结束smartlink
 * @return                  成功  0
 *                          失败  -1
 */
KP2P_API int P2PSDKSmartlinkStop(void);


/**
 * @brief 设置易视网服务器地址和端口
 * @param server            服务器IP或者域名字符串
 * @param port              服务器端口
 * @return                  成功  0
 *                          失败  -1
 */
KP2P_API int P2PSDKSetEseeServerAddr(const char *server, uint16_t port);


/**
 * @brief 设置通道摄像机描述
 * @param channel           通道号
 * @param descriptions      描述字符串
 * @return                  成功  0
 *                          失败  P2PSDK_ERR_INVALID_PARAM 参数错误
 */
KP2P_API int P2PSDKSetCamDes(uint32_t channel, const char *descriptions);


/**
 * @brief 增加一个虚拟通道
 * @param app               应用名称
 * @param type              类型
 * @return                  成功  0
 *                          失败
 */
KP2P_API int P2PSDKAddVconApp(const char *app, uint32_t type, fAddrGenerator addr_generator);


/**
 * @brief 语音数据发送
 * @param session           连接会话
 * @param header_voice      数据帧头
 * @param voice_data        数据缓冲
 * @param voice_data_size   缓冲长度
 * @return                  成功  发送出去的数据长度
 *                          失败
 */
KP2P_API ssize_t P2PSDKVoP2PTalkSend(void *session, kp2p_frame_head_t *voice_header, void *voice_data, size_t voice_data_size);


/**
 * @brief 报警信息发送
 * @param session           连接会话
 * @param header_msg        报警信息帧头
 * @param buf_msg           报警信息缓冲
 * @param buf_len           信息长度
 * @return                  成功  发送出去的数据长度
 *                          失败
 */
KP2P_API ssize_t P2PSDKSendAlarmMsg(void *session, kp2p_frame_head_t *header_msg, void *alarm_msg,  size_t msg_len);


/**
 * @brief 云录像上传
 * @param frame_head        录像信息头
 * @param rec_type          录像类型
 * @param frame             裸帧数据
 * @param frame_size        裸帧长度
 * @param timeout           最大超时时间，单位us
 * @return                  成功  0
 *                          失败  -1
 */
KP2P_API int P2PSDKCloudRecordUpload(kp2p_frame_head_t *frame_head, uint32_t rec_type, void *frame, uint32_t frame_size, uint32_t timeout);


#ifdef __cplusplus
}
#endif
#endif
