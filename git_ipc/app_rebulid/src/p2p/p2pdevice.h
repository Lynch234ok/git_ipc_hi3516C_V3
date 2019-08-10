#ifndef __P2PDEVICE_H__
#define __P2PDEVICE_H__
//#include "mediabufDelegate.h"
#include "mutex_f.h"

#ifdef WIN32
#pragma comment(lib, "P2PSDKDevice.lib")
#endif
#define SIZE_BUFF 200*1024

extern int au_flag;
extern LP_MUTEX au_mutex;

#define MAGIC_NUMBER 0x76636f6e
#define MAGIC_NUMBER_RETRANSMIT 0x72656e64
#define DST_FILE_PATH "/media/conf"
#define DST_FILE_PATH_DEBUG      DST_FILE_PATH "/vcon_debug.txt"
#define DST_FILE_PATH_CST_SOUND  DST_FILE_PATH "/Custom_sound"

struct P2PDeviceDemo {
	char serialNo[32];
	char eseeId[32];
    char version[32];
	char hwcode[32];
    char vendor[16];
    char model[32];        // 产品型号、设备类型
    unsigned int install_type;
    unsigned int cloud_record;  // 云录像支持: 0-不支持 1-支持
    char area[4];          // 区域码
    int max_ch;
    char capabilities[64];  // 能力集
	void (*OnDevOnline)(const char *eseeid, void *ctx);
	//int (*OnDevAttachStream)(int chn, int streamNo, void *ctx);
	//int (*OnDevDetachStream)(int chn, int streamNo, void *ctx);
	void *ctx;
    unsigned char apMode;
};


/**
 * @brief 手机定制: 数据包标识
 */
enum VCON_PACKED_FLAG {
    /* first */
    VCON_PACKED_FLAG_FIRST = 0,
    /* continue */
    VCON_PACKED_FLAG_CONTINUS = 1,
    /* end */
    VCON_PACKED_FLAG_END = 2,
	/* retransmit */
	VCON_PACKED_FLAG_REND = 4,
};


/**
 * @brief 手机定制: 数据的文件类型
 */
enum VCON_FILE_TYPE {
    /* DEBUG */
    VCON_FILE_TYPE_DEBUG = 0,
    /* PCM */
    VCON_FILE_TYPE_PCM = 1,
    /* G711A */
    VCON_FILE_TYPE_G711A = 2,
    /* jpeg */
    VCON_FILE_TYPE_JPEG = 3,
    /* jnp */
    VCON_FILE_TYPE_JNP = 4,
};

/**
 * @brief 手机定制: 数据包的头信息
 */
typedef struct vconDataHead
{
	unsigned int magic;			// magic number 固定为 0x76636f6e
	unsigned int version;		// 版本信息
	unsigned int FileType;		// 文件主类型 1:PCM  2:G711A  3：jpeg   4：jnp
	unsigned int packedFlag;	// 包标识 0:first 1:continue  2:end
	unsigned int crc;			// 包数据crc校验码 0：则不校验
	unsigned int nFileLen;		// 文件总数据长度，数据总大小
	unsigned int npackedLen;	// 包数据长度，
	unsigned int npackedNo;		// 包序号
	unsigned int reverse;		// 保留字段
	char* data;
}vconDataHead_t;
/**
 * @brief 手机定制:设备重传的信息
 */
typedef struct vconRendDataHead
{
	unsigned int magic;			// magic number 固定为 0x72656e64
	unsigned int version;		// 版本信息
	unsigned int FileType;		// 文件主类型
	unsigned int npackedNo;		// 包序号
	unsigned int endflag;		// 结束标识 1:未结束 2：结束
	unsigned int reverse;		// 保留字段
}vconRendDataHead_t;

//int P2PDeviceStart(struct P2PDeviceDemo* pDemo);

int P2P_sdkdestroy();

#ifdef __cplusplus
extern "C"{
#endif

/*
* @brief  大拿报警推送
* @return 成功  发送出去的数据长度
*         失败    -1
* 备注:接口参数后面再根据需求做调整
*/
extern int P2P_danaAlarmMsgSend();

/*
* @brief  监测到移动侦测，通过接口设置标记大拿云存储录像类型为移动侦测录像类型
*/
extern void P2P_danaRecordMotion(bool motion);

#ifdef __cplusplus
}
#endif

#endif
