#ifndef DANA_VIDEO_H
#define DANA_VIDEO_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _dana_video_cmd_e {
    DANAVIDEOCMD_DEVDEF,
    DANAVIDEOCMD_DEVREBOOT,
    DANAVIDEOCMD_GETSCREEN,
    DANAVIDEOCMD_GETALARM,
    DANAVIDEOCMD_GETBASEINFO,
    DANAVIDEOCMD_GETCOLOR,
    DANAVIDEOCMD_GETFLIP,
    DANAVIDEOCMD_GETFUNLIST,
    DANAVIDEOCMD_GETNETINFO,
    DANAVIDEOCMD_GETPOWERFREQ,
    DANAVIDEOCMD_GETTIME,
    DANAVIDEOCMD_GETWIFIAP,
    DANAVIDEOCMD_GETWIFI,
    DANAVIDEOCMD_PTZCTRL,
    DANAVIDEOCMD_SDCFORMAT,
    DANAVIDEOCMD_SETALARM,
    DANAVIDEOCMD_SETCHAN,
    DANAVIDEOCMD_SETCOLOR,
    DANAVIDEOCMD_SETFLIP,
    DANAVIDEOCMD_SETNETINFO,
    DANAVIDEOCMD_SETPOWERFREQ,
    DANAVIDEOCMD_SETTIME,
    DANAVIDEOCMD_SETVIDEO,
    DANAVIDEOCMD_SETWIFIAP,
    DANAVIDEOCMD_SETWIFI,
    DANAVIDEOCMD_STARTAUDIO,
    DANAVIDEOCMD_STARTTALKBACK,
    DANAVIDEOCMD_STARTVIDEO,
    DANAVIDEOCMD_STOPAUDIO, 
    DANAVIDEOCMD_STOPTALKBACK,
    DANAVIDEOCMD_STOPVIDEO,
    DANAVIDEOCMD_RECLIST,
    DANAVIDEOCMD_RECPLAY,
    DANAVIDEOCMD_RECSTOP,
    DANAVIDEOCMD_RECACTION,
    DANAVIDEOCMD_RECSETRATE,
    DANAVIDEOCMD_RECPLANGET,
    DANAVIDEOCMD_RECPLANSET,
    DANAVIDEOCMD_EXTENDMETHOD,

    DANAVIDEOCMD_RESOLVECMDFAILED,
} dana_video_cmd_t;

typedef void* pdana_video_conn_t;

typedef struct _dana_video_callback_funs_s {
    uint32_t (*danavideoconn_created)(pdana_video_conn_t danavideoconn);
    void (*danavideoconn_aborted)(pdana_video_conn_t danavideoconn);
    void (*danavideoconn_command_handler)(pdana_video_conn_t danavideoconn, dana_video_cmd_t cmd, uint32_t trans_id, void* cmd_arg, uint32_t cmd_arg_len); 

} dana_video_callback_funs_t;

// called when ipc recved p2pserver's heartbeat response
typedef void (*danavideo_hb_is_ok_callback_t) (void);

// called when ipc check that the conn with p2p2server is not avliable
typedef void (*danavideo_hb_is_not_ok_callback_t) (void);

// called when need upgrade rom
typedef void (*danavideo_upgrade_rom_callback_t) (const char* rom_path,  const char *rom_md5, const uint32_t rom_size); // 单位 Byte

// called when need autoset
typedef void (*danavideo_autoset_callback_t) (const uint32_t power_freq, const int64_t now_time, const char *time_zone, const char *ntp_server1, const char *ntp_server2);
// 电源频率 0 50Hz,  1 60Hz
// now_time 单位s
// time_zone 时区
// NTP 服务器1 2


// called when need setwifi
// setwifi有三种情况,一种是通过command_handler获取该命令
//                   一种是通过回调函数(本地模式)
//                   还有智能声控模式
//              ip_type  0: fixed 1:dhcp
//              enc_type 见danavideo_cmd.h
typedef void (*danavideo_local_setwifiap_callback_t) (const uint32_t ch_no, const uint32_t ip_type, const char *ipaddr, const char *netmask, const char *gateway, const char *dns_name1, const char *dns_name2, const char *essid, const char *auth_key, const uint32_t enc_type);

// called when need local auth
// 0 succeeded
// 1 failed
typedef uint32_t (*danavideo_local_auth_callback_t) (const char *user_name, const char *user_pass);

// called when create a new conf or update a exited conf
typedef void (*danavideo_conf_created_or_updated_t) (const char *conf_absolute_pathname);

typedef enum _danavideo_msg_type {
   audio_stream = 0x20000000,
   video_stream = 0x40000000,
   extend_data   = 0x60000000,
   pic_stream   = 0x80000000,
} danavideo_msg_type_t;

typedef enum _danavideo_codec_type {
    H264    = 1,
    MPEG    = 2,
    MJPEG   = 3,
    H265    = 4,
    G711A   = 101,
    ULAW    = 102,
    G711U   = 103,
    PCM     = 104,
    ADPCM   = 105,
    G721    = 106,
    G723    = 107,
    G726_16 = 108,
    G726_24 = 109,
    G726_32 = 110,
    G726_40 = 111,
    AAC     = 112,
    JPG     = 200,
} danavidecodec_t;

typedef struct _dana_packet_s {
    char    *data;
    int32_t   len;
} dana_packet_t;

typedef struct _dana_audio_packet_s {
    char  *data;
    int32_t len;
    danavidecodec_t  codec; // 音频编码
} dana_audio_packet_t;


// 用于设置lib_danavideo_start超时时间,
void lib_danavideo_set_startup_timeout(const uint32_t timeout_sec);

void lib_danavideo_set_hb_is_ok(danavideo_hb_is_ok_callback_t fun);
void lib_danavideo_set_hb_is_not_ok(danavideo_hb_is_not_ok_callback_t fun);

void lib_danavideo_set_upgrade_rom(danavideo_upgrade_rom_callback_t fun);

void lib_danavideo_set_autoset(danavideo_autoset_callback_t fun);

void lib_danavideo_set_local_setwifiap(danavideo_local_setwifiap_callback_t fun);

void lib_danavideo_set_local_auth(danavideo_local_auth_callback_t fun);

void lib_danavideo_set_conf_created_or_updated(danavideo_conf_created_or_updated_t fun);

uint32_t lib_danavideo_linked_version(void);
char * lib_danavideo_linked_version_str(uint32_t version);

char * lib_danavideo_deviceid_from_conf(const char *danale_path);

char * lib_danavideo_deviceid();

// ***本地监听端口***
// 在lib_danavideo_start之前 调用 则会采用设置的端口(固定模式下 或者非固定模式下自由选择)
// 在lib_danavideo_start之后 调用 则默认会启动34102 然后判断设置端口是否和34102相同 如果不相同则会关闭34102 启动用户设置的端口,否则直接返回

void lib_danavideo_set_listen_port(const bool listen_port_fixed, const uint16_t listen_port);
uint16_t lib_danavideo_get_listen_port(); 


// 创建配置文件
// 必须在lib_danavideo_init调用之后才可以调用
// 如果指定的目录下配置文件存在并检测有效，则不会创建
// 否则会重新创建一个配置文件
// 非阻塞
bool lib_danavideo_create_on_check_conf();

// 更新发送数据接口 
// 增加lib_danavideoconn_send 
// 取消danavideo_msg_max_size
// 取消lib_danavideo_packet_create
//     lib_danavideo_packet_destroy
//     lib_danavideoconn_sendvideomsg
//     lib_danavideoconn_sendaudiomsg
//     lib_danavideoconn_sendpicmsg
// 更新danavideo_msg_type_t 将rec_stream变更为extend_data 以支持extend_data的发送
// 支持发送video audio pic extend_data
bool lib_danavideoconn_send(pdana_video_conn_t danavideoconn, const danavideo_msg_type_t msg_type, const danavidecodec_t codec, const uint8_t ch_no, const uint8_t is_keyframe, const uint32_t timestamp, const char* data, const uint32_t data_len, const uint32_t timeout_usec);

dana_audio_packet_t* lib_danavideoconn_readaudio(pdana_video_conn_t danavideoconn, uint32_t timeout_usec); // call lib_danavideo_audio_packet_destroy to free
void lib_danavideo_audio_packet_destroy(dana_audio_packet_t *danaaudiomsg);

// lib_danavideo_init不再自动创建配置文件 当配置文件不存在(或无效)时直接返回false
bool lib_danavideo_init(const char *danale_path, const char *agent_user, const char *agent_pass, const char *chip_code, const char *scheme_code, const char *product_code, dana_video_callback_funs_t *danavideocallbackfuns);

uint32_t lib_danavideo_set_userdata(pdana_video_conn_t danavideoconn, void *userdata);

uint32_t lib_danavideo_get_userdata(pdana_video_conn_t danavideoconn, void **userdata);

bool lib_danavideo_start();

bool lib_danavideo_cmd_exec_response(pdana_video_conn_t danavideoconn, const dana_video_cmd_t cmd, char* error_method, const uint32_t trans_id, const uint32_t code, const char* code_msg); 

bool lib_danavideo_cmd_setvideo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t fps);
bool lib_danavideo_cmd_startvideo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t fps);
bool lib_danavideo_cmd_getalarm_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t motion_detection, const uint32_t opensound_detection, const uint32_t openi2o_detection, const uint32_t smoke_detection, const uint32_t shadow_detection, const uint32_t other_detection);
bool lib_danavideo_cmd_getbaseinfo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const char *dana_id, const char *api_ver, const char *sn, const char *device_name, const char *rom_ver, const uint32_t device_type, const uint32_t ch_num, const uint64_t sdc_size, const uint64_t sdc_free); // // dana_id < 49; api_ver < 129; sn < 129; device_name < 129; rom_ver < 129
bool lib_danavideo_cmd_getcolor_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t brightness, const uint32_t contrast, const uint32_t saturation, const uint32_t hue);
bool lib_danavideo_cmd_getflip_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t flip_type);
bool lib_danavideo_cmd_getfunlist_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t methodes_count, const char **methodes); // methodes_count <= 160
bool lib_danavideo_cmd_getnetinfo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t ip_type, const char *ipaddr, const char *netmask, const char *gateway, const uint32_t dns_type, const char *dns_name1, const char *dns_name2, const uint32_t http_port); // ipaddr < 16; netmask < 16; gataway < 16; dns_type < 10; dns_name1 < 16; dns_name2 < 16
bool lib_danavideo_cmd_getpowerfreq_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t freq);
bool lib_danavideo_cmd_gettime_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const int64_t now_time, const char *time_zone, const char *ntp_server_1, const char *ntp_server_2); // time_zone < 65

typedef struct _libdanavideo_wifiinfo_s {
    char essid[33];
    uint32_t enc_type;
    uint32_t quality;
} libdanavideo_wifiinfo_t;

bool lib_danavideo_cmd_getwifiap_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t wifi_device, const uint32_t wifi_list_count, const libdanavideo_wifiinfo_t *wifi_list); // wifi_list_count <= 20;  essid < 33
bool lib_danavideo_cmd_getwifi_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const char *essid, const char *auth_key, const uint32_t enc_type); // essid < 33; auth_key < 65
bool lib_danavideo_cmd_starttalkback_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t audio_codec);


typedef struct _libdanavideo_reclist_recordinfo_s {
    int64_t start_time;
    uint32_t length;
    uint32_t record_type;
} libdanavideo_reclist_recordinfo_t;

bool lib_danavideo_cmd_reclist_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t rec_lists_count, const libdanavideo_reclist_recordinfo_t *rec_lists); // rec_lists_count <= 35

typedef struct _libdanavideo_recplanget_recplan_s {
    uint32_t record_no;
    size_t week_count; // <= 7  表示week数组中有几天
    uint32_t week[7];
    char start_time[33]; // 时:分:秒 12:12:12
    char end_time[33];
    uint32_t status;
} libdanavideo_recplanget_recplan_t;

bool lib_danavideo_cmd_recplanget_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t rec_plans_count, const libdanavideo_recplanget_recplan_t *rec_plans); // rec_plans_count <= 3


bool lib_danavideo_stop();

bool lib_danavideo_deinit();


/////////////////////////////////////// 工具类 ////////////////////////////////////////

typedef enum _danavideo_device_type_e {
    DANAVIDEO_DEVICE_IPC = 1,
    DANAVIDEO_DEVICE_DVR,
    DANAVIDEO_DEVICE_NVR,
    DANAVIDEO_DEVICE_Unknown_type = -1,
} danavideo_device_type_t;

bool lib_danavideo_util_setdeviceinfo(const danavideo_device_type_t device_type, const uint32_t channel_num, const char *rom_ver, const char *api_ver, const char *rom_md5); // rom_ver < 129; api_ver < 129; rom_md5 < 65


// 消息类型: 1,移动侦测警告；2,声音侦测；3,红外;4,其它；99，系统消息(软件升级等).
bool lib_danavideo_util_pushmsg(const uint32_t ch_no, const uint32_t msg_type, const char *msg_title, const char *msg_body, const int64_t cur_time, const uint32_t att_flag, const char *att_path, const char *att_type, const uint32_t record_flag, const int64_t start_time, const uint32_t time_len, const uint32_t save_site, const char *save_path);

bool lib_danavideo_local_searchapp(const char *check_data, const uint32_t encrypt_flag);


// 智能声控
void lib_danavideo_smart_conf_init();
void lib_danavideo_smart_conf_parse(const int16_t *audio, int32_t size); // PCM 音频数据: 16位有符号数，采样频率必须为48000


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
