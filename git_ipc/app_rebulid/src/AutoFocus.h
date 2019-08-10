
#ifndef __AUTO_FOCUS_H__
#define __AUTO_FOCUS_H__

#include <hi_type.h>
#include "hi_comm_isp.h"
#include "pan_tilt.h"
#include "hi_comm_3a.h"


//#define __DEBUG__
#ifdef __DEBUG__ 
#define _DEBUG(format,...) printf(format, ##__VA_ARGS__) 
#else 
#define _DEBUG(format,...) 
#endif 

/*
ϵ����ȡֵ��ΧΪ0~64����Ӧ��ֵΪ0.00~1.00;

ϵ���ļ���:�е�CPU����Ը������֧�ֲ�̫�ã�����ڳ����н��и��������ܻ�ʹ�ü���ʱ��ϳ���
Ӱ������ִ�У������ڴ�����λ�ķ�ʽ���渡�����;���������Ҫʵ����ֵA����0.8������64*0.8=51.2��
�����ڳ����п���д��"(A*51)>>6"��������6λ���ǳ���64;
*/

/*
 *��ֵϵ����STAB_FACTOR_LΪ���ޣ�STAB_FACTOR_HΪ���ޣ����������STAB_FACTOR_H��������Ϊ
 *��ʼ��������״̬����ʱ������ٶȽ��ͣ��������½���STAB_FACTOR_L��������Ϊ�����Ѿ�����
 *����״̬����ʱ������ת�ҽ���������̽�⽹��ı仯���ƣ�
 *
 *����ʹ�� STAB_FACTOR_L < STAB_FACTOR_H
 */
#define STAB_FACTOR_L		48
#define STAB_FACTOR_H		52

/*
 *��ϵ��Ŀǰ����
 */
#define ZOOM_FACTOR			52

/*
 *����ϵ����������ֵ���ڴ�ϵ������Ϊ��ͷ�Ѿ��ﵽ������;
 */
#define FOCUS_FACTOR		60

/*
 *����ƫ�����ֵ;��ǰ������ȥԤ�������϶�Ӧ�Ĳ���������������FOCUS_THRESHOLD����Ϊ�Խ�
 *ʧ�ܣ���ʱ�����¶�λ��Ԥ��������������̽�⽹��
 */
#define FOCUS_THRESHOLD		500

/*
 *focus reverse Step �������Ϊfocus����������һ�룻
 *��̽�⽹���ʱ�򣬻�̶���һ�������ߣ���������ȷ����ǰ��������������ߴ��߻������ʹ��ߣ�
 *����̶���IN�����ߣ�������ʱ����һ�˻�����߲�����̽�⵽�Ľ���ֵ�̶�������Ϊ�˱�����
 *�������Ӱ������FOCUS_REVERSE_STEP�������ڴ�ֵ��IN�����ߣ�С�ڴ�ֵ�������(��IN��OUT��
 *OUT��IN);
 */
#define FOCUS_REVERSE_STEP	1000

#define FRAME_T				5		//���������õ�ʱ���������ڶ�ʱ: 5->30ms, 4->20ms
#define DROP_CNT			8		//������Ƶ�ʱ�����Ĵ���(Ϊż��)
#define PROBE_CNT			(15+DROP_CNT)		//��⽹��仯���ƵĴ���������Ϊ����
#define BLEND_SHIFT 		6
#define ALPHA 				29 // 0.45
#define BELTA 				54 // 0.85

#define AUTO_FOCUS_DATA_PATH				"/media/conf/AutoFocusData.csv"	//�������ݱ���·��

/*
�۽�������������״̬
*/
typedef enum
{
	S_FOCUS_INIT_A = 0,
	S_FOCUS_INIT_B,
	S_FOCUS_PROBE,
	S_FOCUS_PROBE_DATA_COLE,
	S_FOCUS_MOVE,
	S_FOCUS_MOVE_DATA_COLE,
	S_FOCUS_SHARP_TUN,
	S_FOCUS_SHARP_TUN_DATA_COLE,
	S_FOCUS_FIN_DATA_COLE,
	S_FOCUS_FIN_PROBE_OUT,
	S_FOCUS_FIN_PROBE_IN,
	S_FOCUS_FIN_PROBE_DATA_COLE,
	S_FOCUS_MOVE_REVER,
	S_FOCUS_MOVE_REVER_PROBE,
	S_FOCUS_MOVE_REVER_PROBE_COL,
	S_FOCUS_MOVE_DATA_COL_REVER,
	S_FOCUS_SHARP_TUN_REVER,
	S_FOCUS_HIGHT_DAT_COL,
	FOCUS_STAUS_CNT,
}enAUTO_FOCUS_STAUS;

/*
���صĵ����������
*/
typedef enum
{
	FOCUS_CMD_IN = 0,
	FOCUS_CMD_OUT,
	FOCUS_CMD_FAR,
	FOCUS_CMD_NEAR,
	FOCUS_CMD_STOP,
	FOCUS_CMD_INIT,
	FOCUS_CMD_GOTO,
	FOCUS_CMD_CNT,
}enAUTO_FOCUS_CMD;

/**
@brief �������ݽṹ��

���ڱ������ݣ��������ٴ��ϵ�ʱ��������ɨ�轹�㣻
*/
typedef struct savData{
	int g_savStatus;	///<�ϵ�ʱ������״̬
	unsigned long g_savHighestFocusVal;	///<�ϵ�ʱ�Ľ�������
	int focusStep;
	int zoomStep;
}stSavData;

/**
@brief ��ʱ�ṹ��

���ڼ�¼��ʱ��Ϣ������ʱ����ʱִ�г�ʱ������
*/
typedef struct tagSTRUCT_ALARM
{
	HI_U32 ActTime;		///<���õĳ�ʱʱ��,��λs
	void * Arg;			///<��ʱ����ʱ���ݸ���ʱ�����Ĳ���
	HI_U32 CurTime;		///<��ǰʱ��,��λs
	HI_S32 FromStat;	///<���ó�ʱ��ʱ��������״̬
	HI_BOOL setAlarm;	///<��ʱ��־,�����ж��Ƿ��г�ʱ������
}stSTRUCT_ALARM;

/**
@brief ��ʱ�ṹ��

���ڼ�¼״̬����ʱ��Ϣ,��Ϊ�ھ۽������в���������˶����޷����Ʋ�����,ֻ��ͨ����ʱ�ķ�ʽ
�ȴ�������������;
*/
typedef struct tagSTRUCT_SLEEP_TIMER
{
	HI_U32 ActTime;		///<���õ���ʱʱ��,��λs
	HI_U32 CurTime;		///<��ǰʱ��,��λs
	HI_BOOL setTimer;	///<��ʱ��־,�����ж��Ƿ�����ʱ������
}stSTRUCT_TIMER;

typedef struct tagSTRUCT_WAIT
{
	HI_BOOL wait;
}stSTRUCT_WAIT;

typedef struct tagJaAutoFocus
{
	HI_BOOL AutoFocusIsInit;
	HI_S32	MotorSpeed;
	HI_S32 (*init)(void);
	HI_S32 (*auto_focus)(TJA_PanTilt *pStepMotor
		, enAUTO_FOCUS_CMD *pReCMD
		, HI_U32 *pu32Fv
		, ISP_AF_INFO_S *pstAfInfo);

}stJaAutoFocus;

extern stJaAutoFocus g_stJaAutoFocus;


#endif

