
#ifndef __HICHIP_ALARM_H__
#define __HICHIP_ALARM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NK_HICHIP_ALARM_MAGIC   (0x4E314152)//N1AR
#define NK_HICHIP_ALARM_VER_10  (0x01000000)//ver 1.0.0.0

#define NK_HICHIP_ALARM_MD              (1<<0)
#define NK_HICHIP_ALARM_IO              (1<<1)
#define NK_HICHIP_ALARM_HEARDBEAT       (1<<29)
#define NK_HICHIP_ALARM_OPERATION       (1<<30)
#define NK_HICHIP_ALARM_END				(0xFFFFFFFF)

#pragma pack(4)
typedef struct
{
	unsigned int magic;             //MAGIC，用于快速判断数据包NK_HICHIP_ALARM_MAGIC
	unsigned int length;            //包括length在内的往后的数据长度
	unsigned int ver;               //版本号,NK_HICHIP_ALARM_VER_10
	unsigned char checksum[16];     //校验和,将值填充为"_+N1ALARM+CSUM+_"MD5运算后将结果填充回来
	unsigned int type;              //报警类型
	unsigned int res[2];            //保留位
}stNK_HICHIP_ALARM_HEADER, *lpNK_HICHIP_ALARM_HEADER;

typedef struct
{
	unsigned int type;              //报警类型
	unsigned int res;				//保留位
}stNK_HICHIP_ALARM_BOUNDARY, *lpNK_HICHIP_ALARM_BOUNDARY;

typedef struct
{
	unsigned int chn;               //通道
//	unsigned int rowGranularity;    //触发点阵总行
//	unsigned int columnGranularity; //触发点阵总列
//	unsigned int granularity[64];   //触发点阵，按位存放触发点
	unsigned int res[2];            //保留位
}stNK_HICHIP_MD_DATA, *lpNK_HICHIP_MD_DATA;

#define NK_HICHIP_IO_PIR (1<<0)
#define NK_HICHIP_IO_TF (1<<1)
typedef struct
{
	unsigned int chn;               //通道
	unsigned int type;              //IO报警类型
	unsigned int res[2];            //保留位
}stNK_HICHIP_IO_DATA, *lpNK_HICHIP_IO_DATA;

#define NK_HICHIP_HEARTBEAT_LIVE    (1<<1)  //心跳保持
typedef struct
{
	unsigned int status;            //状态位
	unsigned int res[2];            //保留位
}stNK_HICHIP_HEARTBEAT_DATA, *lpNK_HICHIP_HEARTBEAT_DATA;

#define NK_HICHIP_OPERATION_NONE    (0)     //无操作
#define NK_HICHIP_OPERATION_START   (1<<1)  //发送码流
#define NK_HICHIP_OPERATION_PAUSE   (1<<2)  //暂停码流
#define NK_HICHIP_OPERATION_STOP    (1<<3)  //停止码流
#define NK_HICHIP_OPERATION_REQ_IDR (1<<4)  //请求I帧
typedef struct
{
	unsigned int status;            //状态位
	unsigned int res[2];            //保留位
}stNK_HICHIP_OPERATION_DATA, *lpNK_HICHIP_OPERATION_DATA;
#pragma pack()

typedef enum
{
	DETECTION_MSG_MD,
	DETECTION_MSG_BI, //"bi":人体红外
	DETECTION_MSG_LV, //"lv":视频丢失
	DETECTION_MSG_VO, //"vo":视频遮挡
	DETECTION_MSG_VM, //"vm":视频遮盖
	DETECTION_MSG_ND, //"nd":找不到硬盘
	DETECTION_MSG_DE, //"de":硬盘错误
	DETECTION_MSG_DB, //"db":硬盘空间不足
	DETECTION_MSG_RE, //"re":录像错误
	DETECTION_MSG_DS, //"ds":门磁
	DETECTION_MSG_SM, //"sm":烟雾
	DETECTION_MSG_RC, //"rc":遥控器
	DETECTION_MSG_DK, //"dk":门铃按键
	DETECTION_MSG_EXT, //"other"
	DETECTION_MSG_CNT,
}enDETECTION_MSG;
extern char *nk_msg_str[DETECTION_MSG_CNT];

typedef struct
{
	unsigned int chn;
	enDETECTION_MSG type;
}stNK_HICHIP_ALARM_TYPE, *lpNK_HICHIP_ALARM_TYPE;

typedef struct
{
	unsigned int alarmNum;
	lpNK_HICHIP_ALARM_TYPE alarmType;
	unsigned int ioDataNum;
	lpNK_HICHIP_IO_DATA ioData;  //if IO type not in enDETECTION_MSG, put in this ioData.
	unsigned int heartBeatDataNum;
	lpNK_HICHIP_HEARTBEAT_DATA heartBeatData;
	unsigned int operationDataNum;
	lpNK_HICHIP_OPERATION_DATA operationData;
}stNK_HICHIP_ALARM_ARG, *lpNK_HICHIP_ALARM_ARG;

typedef struct
{
	unsigned int type;
	union{
		stNK_HICHIP_MD_DATA md;
		stNK_HICHIP_IO_DATA io;
		stNK_HICHIP_HEARTBEAT_DATA heartBeat;
		stNK_HICHIP_OPERATION_DATA operation;		
	};
}stNK_HICHIP_ALARM_DATA, *lpNK_HICHIP_ALARM_DATA;

extern int HICHIP_alarm_parse2_init(lpNK_HICHIP_ALARM_ARG alarmArg);
extern int HICHIP_alarm_parse2_deinit(lpNK_HICHIP_ALARM_ARG alarmArg);
extern int HICHIP_alarm_parse2(char *pbuf, int len, lpNK_HICHIP_ALARM_ARG alarmArg);  //success return 0
extern int HICHIP_alarm_parse(char *pbuf, int len, lpNK_HICHIP_ALARM_DATA retData, int maxNum);  //return data num
extern int HICHIP_alarm_construct(char *retBuf, int bufSize, lpNK_HICHIP_ALARM_DATA alarmData, int alarmNum);  //success return data size

#ifdef __cplusplus
}
#endif

#endif /* __HICHIP_ALARM_H__ */
