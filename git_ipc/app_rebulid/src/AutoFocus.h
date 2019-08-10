
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
系数的取值范围为0~64，对应的值为0.00~1.00;

系数的计算:有的CPU本身对浮点计算支持不太好，如果在程序中进行浮点计算可能会使得计算时间较长，
影响程序的执行，所以在此用移位的方式代替浮点计算;比如程序中要实现数值A乘以0.8，由于64*0.8=51.2，
所以在程序中可以写成"(A*51)>>6"，向右移6位就是除以64;
*/

/*
 *阀值系数，STAB_FACTOR_L为低限，STAB_FACTOR_H为高限，当焦点进入STAB_FACTOR_H以上则认为
 *开始进入清晰状态，这时候调节速度降低；当焦点下降到STAB_FACTOR_L以下则认为焦点已经不在
 *清晰状态，这时候电机反转找焦或者重新探测焦点的变化趋势；
 *
 *必须使得 STAB_FACTOR_L < STAB_FACTOR_H
 */
#define STAB_FACTOR_L		48
#define STAB_FACTOR_H		52

/*
 *此系数目前弃用
 */
#define ZOOM_FACTOR			52

/*
 *清晰系数，当焦点值高于此系数则认为镜头已经达到最清晰;
 */
#define FOCUS_FACTOR		60

/*
 *步数偏离最大值;当前步数减去预设曲线上对应的步数，如果结果大于FOCUS_THRESHOLD则认为对焦
 *失败，这时会重新定位到预设曲线上再重新探测焦点
 */
#define FOCUS_THRESHOLD		500

/*
 *focus reverse Step 最好设置为focus电机最大步数的一半；
 *在探测焦点的时候，会固定往一个方向走，这样方便确定当前的走向是往焦点高处走还是往低处走，
 *比如固定往IN方向走，但是这时在另一端会出现走不动而探测到的焦点值固定的现象，为了避免这
 *种现象的影响设置FOCUS_REVERSE_STEP，当大于此值往IN方向走，小于此值电机反向(即IN变OUT，
 *OUT变IN);
 */
#define FOCUS_REVERSE_STEP	1000

#define FRAME_T				5		//函数被调用的时间间隔，用于定时: 5->30ms, 4->20ms
#define DROP_CNT			8		//检测趋势的时候丢弃的次数(为偶数)
#define PROBE_CNT			(15+DROP_CNT)		//检测焦点变化趋势的次数，必须为奇数
#define BLEND_SHIFT 		6
#define ALPHA 				29 // 0.45
#define BELTA 				54 // 0.85

#define AUTO_FOCUS_DATA_PATH				"/media/conf/AutoFocusData.csv"	//焦点数据保存路径

/*
聚焦过程中所处的状态
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
返回的电机控制命令
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
@brief 保存数据结构体

用于保存数据，这样在再次上电时不用重新扫描焦点；
*/
typedef struct savData{
	int g_savStatus;	///<断电时所处的状态
	unsigned long g_savHighestFocusVal;	///<断电时的焦点数据
	int focusStep;
	int zoomStep;
}stSavData;

/**
@brief 超时结构体

用于记录超时信息，当超时发生时执行超时函数；
*/
typedef struct tagSTRUCT_ALARM
{
	HI_U32 ActTime;		///<设置的超时时间,单位s
	void * Arg;			///<超时发生时传递给超时函数的参数
	HI_U32 CurTime;		///<当前时间,单位s
	HI_S32 FromStat;	///<设置超时的时候所处的状态
	HI_BOOL setAlarm;	///<超时标志,用于判断是否有超时被设置
}stSTRUCT_ALARM;

/**
@brief 延时结构体

用于记录状态的延时信息,因为在聚焦过程中步进电机的运动是无法控制步数的,只能通过延时的方式
等待电机动作的完成;
*/
typedef struct tagSTRUCT_SLEEP_TIMER
{
	HI_U32 ActTime;		///<设置的延时时间,单位s
	HI_U32 CurTime;		///<当前时间,单位s
	HI_BOOL setTimer;	///<延时标志,用于判断是否有延时被设置
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

