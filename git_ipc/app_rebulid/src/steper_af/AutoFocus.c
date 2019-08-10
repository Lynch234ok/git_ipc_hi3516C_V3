/******************************************************************************
  A simple program of Hisilicon Hi35xx video encode implementation.
  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include <AutoFocus.h>


#define COL(x)  "\033[;" #x "m"  
#define RED	  COL(31)  
#define GREEN   COL(32)  
#define YELLOW  COL(33)  
#define BLUE	  COL(34)  
#define MAGENTA COL(35)  
#define CYAN	  COL(36)  
#define WHITE   COL(0)  
#define GRAY	  "\033[0m" 


static stSTRUCT_TIMER Timer = {

	.ActTime = 0,
	.CurTime = 0,
	.setTimer = HI_FALSE,
};

static stSTRUCT_TIMER SaveDataTime = {

	.ActTime = 0,
	.CurTime = 5700,
	.setTimer = HI_FALSE,
};

static stSTRUCT_ALARM Alarm = {

	.ActTime = 0,
	.Arg = NULL,
	.CurTime = 0,
	.FromStat = FOCUS_STAUS_CNT,
	.setAlarm = HI_FALSE,
};

static stSTRUCT_ALARM Terminated_Alarm = {

	.ActTime = 0,
	.Arg = NULL,
	.CurTime = 0,
	.FromStat = FOCUS_STAUS_CNT,
	.setAlarm = HI_FALSE,
};

static stSTRUCT_WAIT Waiter = {
	
	.wait = HI_FALSE,
};
//图像权值
static int AFWeight[8][8] = {
	{1,1,1,1,1,1,1,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,1,1,1,1,1,1,1,},
};

static char * status_type[FOCUS_STAUS_CNT+1] =
{
	"S_FOCUS_INIT_A\0",
	"S_FOCUS_INIT_B\0",
	"S_FOCUS_PROBE\0",
	"S_FOCUS_PROBE_DATA_COLE\0",
	"S_FOCUS_MOVE\0",
	"S_FOCUS_MOVE_DATA_COLE\0",
	"S_FOCUS_SHARP_TUN\0",
	"S_FOCUS_SHARP_TUN_DATA_COLE\0",

	"S_FOCUS_FIN_DATA_COLE\0",
	"S_FOCUS_FIN_PROBE_OUT\0",
	"S_FOCUS_FIN_PROBE_IN\0",
	"S_FOCUS_FIN_PROBE_DATA_COLE\0",
	"S_FOCUS_MOVE_REVER\0",
	"S_FOCUS_MOVE_REVER_PROBE\0",
	"S_FOCUS_MOVE_REVER_PROBE_COL\0",
	"S_FOCUS_MOVE_DATA_COL_REVER\0",
	"S_FOCUS_SHARP_TUN_REVER\0",
	"S_FOCUS_HIGHT_DAT_COL\0",
	"FOCUS_STAUS_CNT\0",
};

static volatile int auto_status = FOCUS_STAUS_CNT, auto_stop;

#define TRANS(status, v)\
		do{\
			auto_status = status;\
			if(v)\
			{\
				_DEBUG("TRANS--Line: %d\n", __LINE__);\
				printf("\033[1;36m%s\033[m\n", status_type[status]);\
			}\
		}while(0)

//设置超时
#define SET_ALARM(seconds, fromStatus, arg)\
	do{\
		Alarm.ActTime = (seconds * 1000) >> FRAME_T;\
		Alarm.CurTime = 0;\
		Alarm.FromStat = fromStatus;\
		Alarm.Arg = arg;\
		Alarm.setAlarm = HI_TRUE;\
	}while(0)

//清除超时
#define CLR_ALARM()\
	do{\
		Alarm.FromStat = FOCUS_STAUS_CNT;\
		Alarm.Arg = NULL;\
		Alarm.setAlarm = HI_FALSE;\
	}while(0)
	
	
//检查是否有超时
#define CHECK_ALARM()\
	do{\
		Alarm.CurTime++;\
		if(Alarm.setAlarm == HI_TRUE\
			&& Alarm.CurTime > Alarm.ActTime)\
		{\
			alarm_callback();\
		}\
	}while(0)

#define SLEEP(seconds)\
	do{\
		Timer.ActTime = (100) >> FRAME_T;\
		Timer.CurTime = 0;\
		Timer.setTimer = HI_TRUE;\
	}while(0)

#define SET_WAIT()\
	do{\
		Waiter.wait = HI_TRUE;\
	}while(0)

#define SET_T_ALARM(fromStatus)\
	do{\
		Terminated_Alarm.ActTime = (5 * 1000) >> FRAME_T;\
		Terminated_Alarm.CurTime = 0;\
		Terminated_Alarm.FromStat = fromStatus;\
		Terminated_Alarm.setAlarm = HI_TRUE;\
	}while(0)

#define CLR_T_ALARM()\
		do{\
			Terminated_Alarm.FromStat = FOCUS_STAUS_CNT;\
			Terminated_Alarm.Arg = NULL;\
			Terminated_Alarm.setAlarm = HI_FALSE;\
		}while(0)

#define CHECK_SLEEP()\
	do{\
		Timer.CurTime++;\
		if(Timer.setTimer == HI_TRUE\
			&& Timer.CurTime < Timer.ActTime)\
		{\
			return HI_SUCCESS;\
		}\
		Timer.setTimer = HI_FALSE;\
	}while(0)

#define CHECK_WAIT()\
	do{\
		if(Waiter.wait == HI_TRUE)\
		{\
			if(pStepMotor->Terminated())\
			{\
				Waiter.wait = HI_FALSE;\
			}\
			else\
			{\
				return HI_SUCCESS;\
			}\
		}\
	}while(0)

#define CHECK_T_ALARM()\
	do{\
		Terminated_Alarm.CurTime++;\
		if(Terminated_Alarm.setAlarm == HI_TRUE\
			&& Terminated_Alarm.CurTime > Terminated_Alarm.ActTime)\
		{\
			if(pStepMotor->Terminated() == HI_TRUE)\
			{\
				t_alarm_callback();\
			}\
		}\
	}while(0)

#define CHECK_SAVE_DATA()\
	do{\
		SaveDataTime.CurTime++;\
		if (SaveDataTime.CurTime > SaveDataTime.ActTime) {\
			SaveDataTime.setTimer = HI_TRUE;\
			SaveDataTime.ActTime = 143;\
			SaveDataTime.CurTime = 0;\
		}\
	}while(0)

extern inline JA_Int
getFocusStep(JA_Int CurZoomStep);
			
static float ProbeAFVal[PROBE_CNT - DROP_CNT];

//AF趋势计算
//X为采集序号，Y为AF值
static bool CalculateLineK(float *k)
{
	int lCount = PROBE_CNT - DROP_CNT;
	float mX, mY, mXX, mXY;
	mX = mY = mXX = mXY = 0;

	int pos = 1;
	while(pos <= (PROBE_CNT - DROP_CNT))
	{
	  mX += pos;
	  mY += ProbeAFVal[pos-1];
	  mXX += pos * pos;
	  mXY += pos * ProbeAFVal[pos-1];
	  pos++;
	}

	if (mX*mX - mXX*lCount == 0) {

		return false;
	}
	
	*k = (mY*mX - mXY*lCount) / (mX*mX - mXX*lCount);

	return true;
}

//超时函数
static void alarm_callback(void)
{
	switch(Alarm.FromStat)
	{		
		case S_FOCUS_FIN_DATA_COLE:
			*((int*)Alarm.Arg) = 0;
			break;
			
		default:
			break;
	}

	Alarm.setAlarm = HI_FALSE;
}

static void t_alarm_callback(void)
{
	switch(Terminated_Alarm.FromStat)
	{
		case S_FOCUS_PROBE:
			auto_stop = HI_TRUE;
			break;
			
		case S_FOCUS_MOVE:
		case S_FOCUS_SHARP_TUN:
		case S_FOCUS_MOVE_REVER:
		case S_FOCUS_SHARP_TUN_REVER:
		case S_FOCUS_MOVE_REVER_PROBE:
			//TRANS(S_FOCUS_FIN_PROBE_IN);
			break;
			
		case S_FOCUS_FIN_PROBE_DATA_COLE:
		case S_FOCUS_HIGHT_DAT_COL:
			TRANS(S_FOCUS_MOVE_REVER, true);
			break;

		case S_FOCUS_MOVE_REVER_PROBE_COL:
			TRANS(S_FOCUS_MOVE, true);
			break;
			
		default:
			break;
	}

	Terminated_Alarm.setAlarm = HI_FALSE;
}

//自动聚焦初始化
HI_S32 auto_focus_init(void)
{
	ISP_DEV IspDev;
	ISP_STATISTICS_CFG_S stIspStaticsCfg;
	ISP_PUB_ATTR_S stPubattr;
	HI_S32 s32Ret = HI_FAILURE;

	ISP_FOCUS_STATISTICS_CFG_S AutoFocusCfg ={
		{1, 8, 8, 2592, 1520, 1, 0},
		{{1, 1, 1}, {188, 476, -235, 375, -184, 276, -206}, {7, 2, 2, 0}, 10},
		{{0, 1, 0}, {200, 0, 0, 0, -55, 0, 0}, {6, 0, 0, 0}, 20},
		{{-6, 12, 22, 12, -6}, 127},
		{{-16, 21, 0, -21, 16}, 10},
		{0, {0, 0,}, {3, 2}}
	};

	IspDev = 0;
	s32Ret = HI_MPI_ISP_GetStatisticsConfig(IspDev, &stIspStaticsCfg);
	s32Ret |= HI_MPI_ISP_GetPubAttr(IspDev, &stPubattr);
	AutoFocusCfg.stConfig.u16Vsize = stPubattr.stWndRect.u32Height;
	AutoFocusCfg.stConfig.u16Hsize = stPubattr.stWndRect.u32Width;
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_ISP_GetStatisticsConfig error!(s32Ret = 0x%x)\n", s32Ret);
		return HI_FAILURE;
	}

	memcpy(&stIspStaticsCfg.stFocusCfg, &AutoFocusCfg, sizeof(ISP_FOCUS_STATISTICS_CFG_S));

	s32Ret = HI_MPI_ISP_SetStatisticsConfig(IspDev, & stIspStaticsCfg);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_ISP_SetStatisticsConfig error!(s32Ret = 0x%x)\n", s32Ret);
		return HI_FAILURE;
	}
	return HI_SUCCESS;
}

HI_S32 auto_focus(TJA_PanTilt *pStepMotor
		, enAUTO_FOCUS_CMD *pReCMD
		, HI_U32 *pu32Fv
		, ISP_AF_INFO_S *pstAfInfo)//, int fd)
{
	static signed int count = 0, Dir = 0;
	static unsigned long highestFocusVal = 0;
	static HI_U32 u32Fv1Back = 0, u32Fv2Back = 0;
	static HI_BOOL powerOff = HI_FALSE, reverFlag = HI_FALSE;
	static ISP_FOCUS_STATISTICS_CFG_S stFocusCfg;
	static int fd, n;
	HI_BOOL checkDirPul = HI_FALSE;
	float k;
	bool reval_b;
	
	HI_S32 s32Ret = HI_FAILURE;
	int i, j, TargetFocusStep;
	*pReCMD = FOCUS_CMD_CNT;
	ISP_DEV IspDev;
	ISP_STATISTICS_CFG_S stIspStaticsCfg;
	HI_U32 u32Fv1_n, u32Fv2_n, u32Fv1, u32Fv2, u32FvTemp;
	static stSavData myData;

	if(HI_FALSE == g_stJaAutoFocus.AutoFocusIsInit)
	{
		s32Ret = g_stJaAutoFocus.init();
		if (HI_SUCCESS != s32Ret)
		{
			printf("auto_focus_init error!(s32Ret = 0x%x)\n", s32Ret);
			return -1;
		}
		
		g_stJaAutoFocus.AutoFocusIsInit = HI_TRUE;
	}

	if(powerOff == HI_FALSE)//第一次上电
	{
		if(!((fd = open(AUTO_FOCUS_DATA_PATH, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)) > 0)){
			printf("open file \"%s\" error: %s", AUTO_FOCUS_DATA_PATH, strerror(errno));
			return -1;
		}
		lseek(fd, 0, SEEK_SET);
		read(fd, &myData, sizeof(myData));
		
		IspDev = 0;
		s32Ret = HI_MPI_ISP_GetStatisticsConfig(IspDev, &stIspStaticsCfg);
		
		memcpy(&stFocusCfg, &stIspStaticsCfg.stFocusCfg, sizeof(ISP_FOCUS_STATISTICS_CFG_S));
		if(myData.g_savStatus != S_FOCUS_FIN_DATA_COLE)//无保存值，则重新扫描焦点
		{	
			*pReCMD = FOCUS_CMD_INIT;
		}
		else							//有保存值，则切换到稳定状态时读取的焦点值
		{
			pStepMotor->setCurStep(&myData.zoomStep, &myData.focusStep);			
			highestFocusVal = myData.g_savHighestFocusVal;
			//TRANS(S_FOCUS_FIN_PROBE_IN);
			//pStepMotor->EnableFocus = JA_True;
		}

		powerOff = HI_TRUE;
		return HI_SUCCESS;
	}

	if (pStepMotor->isReset)
	{
		//1、初始化到当前位置	(当前位置是否为不可调清的位置，暂时不用此方案，改用方案2)
		//pStepMotor->getStep(&myData.zoomStep, &myData.focusStep);

		//2、初始化到上次清晰的画面
		lseek(fd, 0, SEEK_SET);
		read(fd, &myData, sizeof(myData));
		//有保存值，则切换到稳定状态时读取的焦点值
		if(myData.g_savStatus == S_FOCUS_FIN_DATA_COLE)
		{
			pStepMotor->setTargetStep(&myData.zoomStep, &myData.focusStep);
			highestFocusVal = myData.g_savHighestFocusVal;
			TRANS(S_FOCUS_FIN_PROBE_IN, true);
		}
		
		*pReCMD = FOCUS_CMD_INIT;
		pStepMotor->isReset = JA_False;
		return HI_SUCCESS;
	}

	//大概5秒保存一次数据；进入清晰状态必定保存
	CHECK_SAVE_DATA();
	if(SaveDataTime.setTimer == HI_TRUE)
	{
		pStepMotor->getStep(&myData.zoomStep, &myData.focusStep);
		lseek(fd, 0, SEEK_SET);
		write(fd, &myData, sizeof(myData)); //写保存文件
		SaveDataTime.setTimer = HI_FALSE;
	}

	if(pStepMotor->manual == JA_True
		|| pStepMotor->goTarget == JA_True
		|| pStepMotor->FocusGoTo == JA_True)
	{
		return HI_SUCCESS;
	}

	//变倍的过程中，不能变焦
	if(pStepMotor->EnableFocus == JA_False)
	{
		*pReCMD = FOCUS_CMD_CNT;
		CLR_ALARM();
		CLR_T_ALARM();
		Waiter.wait = HI_FALSE;	//clear wait
		TRANS(S_FOCUS_FIN_PROBE_IN, JA_False);

		if(pStepMotor->StopFocus == JA_True)
		{
			pStepMotor->StopFocus = JA_False;
			*pReCMD = FOCUS_CMD_STOP;
			SLEEP(1);
		}

		return HI_SUCCESS;
	}

	HI_U32 u32SumFv1 = 0;
	HI_U32 u32SumFv2 = 0;
	HI_U32 u32WgtSum = 0;

	//get data 
	for(i=0; i<stFocusCfg.stConfig.u16Vwnd; i++)
	{
		for(j=0; j<stFocusCfg.stConfig.u16Hwnd; j++)
		{
			HI_U32 u32H1 = pstAfInfo->stAfStat->stZoneMetrics[i][j].u16h1;
			HI_U32 u32H2 = pstAfInfo->stAfStat->stZoneMetrics[i][j].u16h2;
			HI_U32 u32V1 = pstAfInfo->stAfStat->stZoneMetrics[i][j].u16v1;
			HI_U32 u32V2 = pstAfInfo->stAfStat->stZoneMetrics[i][j].u16v2;
			
			u32Fv1_n = (u32H1 * ALPHA + u32V1 * ((1<<BLEND_SHIFT) - ALPHA)) >> BLEND_SHIFT;
			u32Fv2_n = (u32H2 * BELTA + u32V2 * ((1<<BLEND_SHIFT) - BELTA)) >> BLEND_SHIFT;
			u32SumFv1 += AFWeight[i][j] * u32Fv1_n;
			u32SumFv2 += AFWeight[i][j] * u32Fv2_n;
			u32WgtSum += AFWeight[i][j];
		}
	}

	u32Fv1 = u32SumFv1 / u32WgtSum;
	u32Fv2 = u32SumFv2 / u32WgtSum;
	u32FvTemp = u32Fv1;/**曲线选择**/

	*pu32Fv = u32FvTemp;
	
	CHECK_WAIT();
	CHECK_ALARM();
	CHECK_SLEEP();
	CHECK_T_ALARM();
	
	switch(auto_status)
	{
		case S_FOCUS_INIT_A:
			*pReCMD = FOCUS_CMD_STOP;
			SLEEP(1);
			TRANS(S_FOCUS_INIT_B, JA_True);
			break;

		case S_FOCUS_INIT_B:
			*pReCMD = FOCUS_CMD_IN;
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_HIGH;
			SET_WAIT();
			auto_stop = HI_FALSE;
			highestFocusVal = 0;
			TRANS(S_FOCUS_PROBE, JA_True);
			break;

		case S_FOCUS_PROBE:
			*pReCMD = FOCUS_CMD_OUT;
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_HIGH;
			SET_T_ALARM(S_FOCUS_PROBE);
			TRANS(S_FOCUS_PROBE_DATA_COLE, JA_True);
			break;

		case S_FOCUS_PROBE_DATA_COLE:
			if(u32FvTemp > highestFocusVal)
			{
				highestFocusVal = u32FvTemp;
			}

			if(auto_stop == HI_TRUE)
			{
				*pReCMD = FOCUS_CMD_STOP;
				SLEEP(1);
				TRANS(S_FOCUS_MOVE, JA_True);
				auto_stop = HI_FALSE;
			}
			break;

		case S_FOCUS_MOVE:
			*pReCMD = FOCUS_CMD_IN;
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_MID;
			SET_T_ALARM(S_FOCUS_MOVE);
			TRANS(S_FOCUS_MOVE_DATA_COLE, JA_True);
			break;

		case S_FOCUS_MOVE_DATA_COLE:
			_DEBUG(RED"highest = %ld, u32FvTemp = %ld\n"GRAY, highestFocusVal, u32FvTemp);
			if( u32FvTemp > ( (highestFocusVal * STAB_FACTOR_H) >> BLEND_SHIFT) )
			{
				CLR_T_ALARM();
				*pReCMD = FOCUS_CMD_STOP;
				SLEEP(1);
				TRANS(S_FOCUS_SHARP_TUN, JA_True);
				
				if( u32FvTemp > ( (highestFocusVal * FOCUS_FACTOR) >> BLEND_SHIFT) )
				{
					TRANS(S_FOCUS_FIN_DATA_COLE, JA_True);
				}
			}
			pStepMotor->CheckTarget = JA_True;
			break;

		case S_FOCUS_SHARP_TUN:
			*pReCMD = FOCUS_CMD_IN;		/////可修改为微调
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_LOW;
			SET_T_ALARM(S_FOCUS_SHARP_TUN);
			TRANS(S_FOCUS_SHARP_TUN_DATA_COLE, JA_True);
			break;

		case S_FOCUS_SHARP_TUN_DATA_COLE:
			_DEBUG("highest = %ld, u32FvTemp = %ld\n", highestFocusVal, u32FvTemp);
			if( u32FvTemp > ( (highestFocusVal * FOCUS_FACTOR) >> BLEND_SHIFT) )
			{
				CLR_T_ALARM();
				*pReCMD = FOCUS_CMD_STOP;
				SLEEP(1);
				TRANS(S_FOCUS_FIN_DATA_COLE, JA_True);
			}
			else if (u32FvTemp < ( (highestFocusVal * STAB_FACTOR_L) >> BLEND_SHIFT))
			{
				CLR_T_ALARM();
				*pReCMD = FOCUS_CMD_STOP;
				SLEEP(1);
				TRANS(S_FOCUS_FIN_PROBE_IN, JA_True);
			}
			count = 0;
			break;
		//进入稳定状态，不断进行检测，如有变动则进入S_FOCUS_FIN_PROBE_IN进行变动检测
		case S_FOCUS_FIN_DATA_COLE: 
			myData.g_savStatus = auto_status;
			myData.g_savHighestFocusVal = highestFocusVal;
			SaveDataTime.setTimer = HI_TRUE;	//使能保存数据
			pStepMotor->EnableFocus = JA_False;
			CLR_ALARM();//离开前将超时清除
			//TRANS(S_FOCUS_FIN_PROBE_IN);
			break;

		case S_FOCUS_FIN_PROBE_OUT:
			*pReCMD = FOCUS_CMD_OUT;
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_HIGH;
			//SLEEP(2);
			TRANS(S_FOCUS_FIN_PROBE_IN, JA_True);
			break;
			
		case S_FOCUS_FIN_PROBE_IN://往IN方向走，探测focus数据的变化趋势
			*pReCMD = FOCUS_CMD_IN;
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_MID;
			highestFocusVal = u32FvTemp;
			checkDirPul = HI_TRUE;
			reverFlag = HI_FALSE;
			count = 0;
			Dir = 0;
			n = 0;
			TRANS(S_FOCUS_FIN_PROBE_DATA_COLE, JA_True);
			break;

		case S_FOCUS_FIN_PROBE_DATA_COLE://探测focus数据的变化趋势
			count++; 
			if(count <= DROP_CNT)
			{
				u32Fv1Back = u32FvTemp;
			}
			else
			{
				ProbeAFVal[n] = u32FvTemp;
				n++;
				
				if (u32FvTemp > highestFocusVal)
				{
					highestFocusVal = u32FvTemp;
				}
				
				if(u32FvTemp < u32Fv1Back)
				{
					Dir--;//往反方向走了
				}
				else
				{
					Dir++;//方向正好
				}

				_DEBUG(YELLOW"Dir = %d\n"GRAY, Dir);
				_DEBUG(BLUE"u32FvTemp = %d\n"GRAY, u32FvTemp);
				if(count >= PROBE_CNT)
				{
					//计算数据变化趋势
					reval_b = CalculateLineK(&k);
					//计算成功才使用
					if (reval_b == true)
					{
						if (k > 0)
						{
							Dir = 1;
						}
						else
						{
							Dir = -1;
						}
					}
					
					if(Dir > 0)
					{
						SET_T_ALARM(S_FOCUS_FIN_PROBE_DATA_COLE);
						_DEBUG("#####");
						g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_HIGH;
						TRANS(S_FOCUS_HIGHT_DAT_COL, JA_True);//以当前电机方向进行检测
					}
					else if(Dir < 0)
					{
						*pReCMD = FOCUS_CMD_STOP;
						highestFocusVal = u32FvTemp;
						SLEEP(1);
						TRANS(S_FOCUS_MOVE_REVER_PROBE, JA_True);//反转电机方向进行检测
					}
				}
				u32Fv1Back = u32FvTemp;
			}
			break;

		case S_FOCUS_MOVE_REVER_PROBE:
			*pReCMD = FOCUS_CMD_OUT;
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_HIGH;
			SET_T_ALARM(S_FOCUS_MOVE_REVER_PROBE);
			TRANS(S_FOCUS_MOVE_REVER_PROBE_COL, JA_True);
			break;

		case S_FOCUS_MOVE_REVER_PROBE_COL:
			_DEBUG(BLUE"highest = %ld, u32FvTemp = %ld\n"GRAY, highestFocusVal, u32FvTemp);
			if(u32FvTemp > highestFocusVal)//检测最高焦点值
			{
				CLR_T_ALARM();
				SET_T_ALARM(S_FOCUS_MOVE_REVER_PROBE_COL);
				highestFocusVal = u32FvTemp;
			}
			else if(u32FvTemp < ( (highestFocusVal * STAB_FACTOR_L) >> BLEND_SHIFT))
			{
				*pReCMD = FOCUS_CMD_STOP;
				CLR_T_ALARM();
				SLEEP(1);
				TRANS(S_FOCUS_MOVE, JA_True);
			}
			pStepMotor->CheckTarget = JA_True;
			break;

		case S_FOCUS_MOVE_REVER:
			*pReCMD = FOCUS_CMD_OUT;
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_MID;
			SET_T_ALARM(S_FOCUS_MOVE_REVER);	
			TRANS(S_FOCUS_MOVE_DATA_COL_REVER, JA_True);
			break;

		case S_FOCUS_MOVE_DATA_COL_REVER:
			if( u32FvTemp > ( (highestFocusVal * STAB_FACTOR_H) >> BLEND_SHIFT) )
			{
				CLR_T_ALARM();
				*pReCMD = FOCUS_CMD_STOP;
				SLEEP(1);
				TRANS(S_FOCUS_SHARP_TUN_REVER, JA_True);

				if( u32FvTemp > ( (highestFocusVal * FOCUS_FACTOR) >> BLEND_SHIFT) )
				{
					TRANS(S_FOCUS_FIN_DATA_COLE, JA_True);
				}
			}
			pStepMotor->CheckTarget = JA_True;
			break;

		case S_FOCUS_SHARP_TUN_REVER:
			*pReCMD = FOCUS_CMD_OUT;	/////可修改为微调
			g_stJaAutoFocus.MotorSpeed = FOCUS_SPEED_LOW;
			SET_T_ALARM(S_FOCUS_SHARP_TUN_REVER);
			TRANS(S_FOCUS_SHARP_TUN_DATA_COLE, JA_True);
			break;

		case S_FOCUS_HIGHT_DAT_COL:
			if(u32FvTemp > highestFocusVal)//检测最高焦点值
			{
				CLR_T_ALARM();
				SET_T_ALARM(S_FOCUS_HIGHT_DAT_COL);
				highestFocusVal = u32FvTemp;
			}
			else if(u32FvTemp < ( (highestFocusVal * STAB_FACTOR_L) >> BLEND_SHIFT))
			{
				*pReCMD = FOCUS_CMD_STOP;
				CLR_T_ALARM();
				SLEEP(1);
				TRANS(S_FOCUS_MOVE_REVER, JA_True);
			}
			pStepMotor->CheckTarget = JA_True;
			break;

		default:
			break;
	}

	//test rever dir
	pStepMotor->getStep(&myData.zoomStep, &myData.focusStep);
	if ( (checkDirPul == HI_TRUE && myData.focusStep < FOCUS_REVERSE_STEP)
		|| reverFlag == HI_TRUE )
	{
		if(*pReCMD == FOCUS_CMD_IN)
		{
			*pReCMD = FOCUS_CMD_OUT;
		}
		else if(*pReCMD == FOCUS_CMD_OUT)
		{
			*pReCMD = FOCUS_CMD_IN;
		}

		reverFlag = HI_TRUE;
		checkDirPul = HI_FALSE;
	}

	//检测当前位置偏离预设曲线的步数
	if(pStepMotor->CheckTarget == JA_True)
	{
		TargetFocusStep = getFocusStep(myData.zoomStep);
		if(abs(myData.focusStep - TargetFocusStep) > FOCUS_THRESHOLD)
		{
			pStepMotor->setTargetStep(&myData.zoomStep, &TargetFocusStep);
			*pReCMD = FOCUS_CMD_GOTO;
			//SLEEP(1);
			pStepMotor->FocusGoTo = JA_True;
			TRANS(S_FOCUS_FIN_PROBE_IN, JA_True);//下次有效时，直接探测趋势
		}
		pStepMotor->CheckTarget = JA_False;
	}

	return HI_SUCCESS;
}

stJaAutoFocus g_stJaAutoFocus = {
	
	.AutoFocusIsInit = HI_FALSE,
	.MotorSpeed	= FOCUS_SPEED_LOW,
	.init =auto_focus_init,
	.auto_focus =auto_focus,
};

#ifdef TEST_AUTO_FOCUS
void test_auto_focus(void)
{
	unsigned char buf[7];
	enAUTO_FOCUS_CMD ReCMD;
	HI_U32 u32Fv;
	HI_S32 s32Ret;

	ISP_FOCUS_STATISTICS_CFG_S FocusCfg ={
		{1, 8, 8, 2592, 1520, 1, 0},
		{{1, 1, 1}, {188, 476, -235, 375, -184, 276, -206}, {7, 2, 2, 0}, 10},
		{{0, 1, 0}, {200, 0, 0, 0, -55, 0, 0}, {6, 0, 0, 0}, 20},
		{{-6, 12, 22, 12, -6}, 127},
		{{-16, 21, 0, -21, 16}, 10},
		{0, {0, 0,}, {3, 2}}
	};

	Uart* uart = PUART_Struct("/dev/ttyAMA3");
	if(uart)
	{
		if(auto_status == FOCUS_STAUS_CNT)
		{
			printf("Set baudrate 9600\r\n");
			printf("Set databit 8\r\n");
			printf("Set stopbit 1\r\n");
			printf("Set parity NONE\r\n");
			uart->SetBaud(uart, 9600);
			uart->SetDatabit(uart, 8);
			uart->SetStopbit(uart, 1);
			uart->SetParity(uart, 0);			
		}

		s32Ret = auto_focus_init(FocusCfg);
		if (HI_SUCCESS != s32Ret)
		{
			printf("auto_focus_init error!(s32Ret = 0x%x)\n", s32Ret);
			return ;
		}

		while(1)
		{
			s32Ret = auto_focus(&ReCMD, &u32Fv);
			//printf("u32Fv = %u\n", u32Fv);
			if (HI_SUCCESS != s32Ret)
			{
				printf("auto_focus error!(s32Ret = 0x%x)\n", s32Ret);
				return ;
			}
			else if(ReCMD < FOCUS_CMD_CNT)
			{
				g_hPelcoD.GetCommand((unsigned char*)&buf, ReCMD + PTZ_CMD_ZOOM_IN, 0x0, 0);	//action
				uart->Send(uart, (unsigned char*)&buf, sizeof(buf));	
			}

			usleep(35*1000);
		}

		PUART_Destruct(&uart);
	}
}

void SAMPLE_Usage(void)
{
	printf("Please input your select:\n");
    printf("index:\n");
	printf("\t 0) func.\n");
	printf("\t q) To exit.\n");
}
/******************************************************************************
* function    : main()
* Description : video venc sample
******************************************************************************/
int main(int argc, char** argv)
{
	unsigned char input;

	while(1)
	{
		SAMPLE_Usage();
		scanf("%c", &input);
		getchar();
		switch(input)
		{
			case '0':
				test_auto_focus();
				break;
				
			case 'q':
				return 0;
				
			default:
				break;
		}
	}
	
	return 0;
}
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

