
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
	unsigned int magic;             //MAGIC�����ڿ����ж����ݰ�NK_HICHIP_ALARM_MAGIC
	unsigned int length;            //����length���ڵ���������ݳ���
	unsigned int ver;               //�汾��,NK_HICHIP_ALARM_VER_10
	unsigned char checksum[16];     //У���,��ֵ���Ϊ"_+N1ALARM+CSUM+_"MD5����󽫽��������
	unsigned int type;              //��������
	unsigned int res[2];            //����λ
}stNK_HICHIP_ALARM_HEADER, *lpNK_HICHIP_ALARM_HEADER;

typedef struct
{
	unsigned int type;              //��������
	unsigned int res;				//����λ
}stNK_HICHIP_ALARM_BOUNDARY, *lpNK_HICHIP_ALARM_BOUNDARY;

typedef struct
{
	unsigned int chn;               //ͨ��
//	unsigned int rowGranularity;    //������������
//	unsigned int columnGranularity; //������������
//	unsigned int granularity[64];   //�������󣬰�λ��Ŵ�����
	unsigned int res[2];            //����λ
}stNK_HICHIP_MD_DATA, *lpNK_HICHIP_MD_DATA;

#define NK_HICHIP_IO_PIR (1<<0)
#define NK_HICHIP_IO_TF (1<<1)
typedef struct
{
	unsigned int chn;               //ͨ��
	unsigned int type;              //IO��������
	unsigned int res[2];            //����λ
}stNK_HICHIP_IO_DATA, *lpNK_HICHIP_IO_DATA;

#define NK_HICHIP_HEARTBEAT_LIVE    (1<<1)  //��������
typedef struct
{
	unsigned int status;            //״̬λ
	unsigned int res[2];            //����λ
}stNK_HICHIP_HEARTBEAT_DATA, *lpNK_HICHIP_HEARTBEAT_DATA;

#define NK_HICHIP_OPERATION_NONE    (0)     //�޲���
#define NK_HICHIP_OPERATION_START   (1<<1)  //��������
#define NK_HICHIP_OPERATION_PAUSE   (1<<2)  //��ͣ����
#define NK_HICHIP_OPERATION_STOP    (1<<3)  //ֹͣ����
#define NK_HICHIP_OPERATION_REQ_IDR (1<<4)  //����I֡
typedef struct
{
	unsigned int status;            //״̬λ
	unsigned int res[2];            //����λ
}stNK_HICHIP_OPERATION_DATA, *lpNK_HICHIP_OPERATION_DATA;
#pragma pack()

typedef enum
{
	DETECTION_MSG_MD,
	DETECTION_MSG_BI, //"bi":�������
	DETECTION_MSG_LV, //"lv":��Ƶ��ʧ
	DETECTION_MSG_VO, //"vo":��Ƶ�ڵ�
	DETECTION_MSG_VM, //"vm":��Ƶ�ڸ�
	DETECTION_MSG_ND, //"nd":�Ҳ���Ӳ��
	DETECTION_MSG_DE, //"de":Ӳ�̴���
	DETECTION_MSG_DB, //"db":Ӳ�̿ռ䲻��
	DETECTION_MSG_RE, //"re":¼�����
	DETECTION_MSG_DS, //"ds":�Ŵ�
	DETECTION_MSG_SM, //"sm":����
	DETECTION_MSG_RC, //"rc":ң����
	DETECTION_MSG_DK, //"dk":���尴��
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
