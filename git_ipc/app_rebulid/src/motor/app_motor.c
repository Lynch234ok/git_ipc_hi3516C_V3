#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>

#include "stepper.h"
#include "af_info.h"
#include "inifile.h"
#include "custom.h"

#define CON_BIG_THAN(A, B, C)  (((A) > (B)) && (((A) - (B)) >= (C)))
#define SMALL_STEPS   (8)
#define LARGE_STEPS    (24)
#define TRENDS_LIMIT (10) //数值持续下降计数
#define TRENDS_COUNT (50)

typedef enum{
    STEPPER_TYPE_FOCTEK_D14 = 0,
    STEPPER_TYPE_CNT,
}enSTEPPER_TYPE;

int Stepper_Enable = 0;
int Stepper_Type = 0;
int Stepper_FocusPoint_X = -1; //zoom pos
int Stepper_FocusPoint_Y = -1; //focus pos
int Stepper_FocusMax = -1;
int Stepper_ZoomMax  = -1;

//=========================
// FOCTEK D14 CURVE
//=========================
#define ZOOM_STEP_N  (43) //变倍电机位置区间等分值
#define FOCTEK_D14_STEPPER_FOCUSMAX (1980)
#define FOCTEK_D14_STEPPER_ZOOMMAX (1978)

#define CURVE_MAX_NUMB  (ZOOM_STEP_N)
#define CURVE_MAX_ZOOM  (FOCTEK_D14_STEPPER_ZOOMMAX)
#define CURVE_MAX_FOCUS (FOCTEK_D14_STEPPER_FOCUSMAX)

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

int Ftk_D14_RefCurve[2][CURVE_MAX_NUMB+1] = {
	{216, 216, 216, 224, 232, 240, 240, 256, 264, 272, 280, 296, 304, 320, 336,
	352, 376, 392, 416, 440, 456, 480, 512, 536, 568, 600, 640, 672, 720, 768,
	816, 864, 920, 976,1040,1112,1168,1240,1320,1392,1488,1592,1696,1784,},

	{112, 120, 128, 136, 136, 144, 152, 160, 168, 184, 192, 208, 216, 232, 240,
	256, 280, 296, 312, 336, 368, 384, 408, 432, 464, 496, 528, 560, 608, 656,
	696, 744, 792, 856, 912, 968,1040,1104,1192,1280,1376,1472,1584,1712,},
};

static int Ftk_D14_RefXValue[CURVE_MAX_NUMB+1];
static int Ftk_D14_RefYValue[CURVE_MAX_NUMB+1];

int Ftk_D14_RefCurveInit(int MaxX, int MaxY)
{
	int ii;

	for (ii = 0; ii < CURVE_MAX_NUMB+1; ii += 1) {
		Ftk_D14_RefXValue[ii] = (MaxX/CURVE_MAX_NUMB)*ii;
		Ftk_D14_RefYValue[ii] = Ftk_D14_RefCurve[0][ii];
	}

	return 0;
}

int CalcYValueByRefCurve(int RefYCurve[], int RefSize, int * YValue, int XValue)
{
	int ii;

	if (RefSize > ARRAY_SIZE(Ftk_D14_RefXValue)) {
		RefSize = ARRAY_SIZE(Ftk_D14_RefXValue);
	}

	for (ii = 1; ii < RefSize; ii += 1) {
		if ((XValue <= Ftk_D14_RefXValue[ii]) && (XValue >= Ftk_D14_RefXValue[ii-1])) {
			double Y1 = RefYCurve[ii-1];
			double X1 = Ftk_D14_RefXValue[ii-1];

			double Y2 = RefYCurve[ii];
			double X2 = Ftk_D14_RefXValue[ii];

			*YValue = Y1 + (XValue - X1) * (Y2 - Y1) / (X2 - X1);

			return 0;
		}
	}

	return -1;
}

int GetOptiCurveByPosXY(int OptiYCurve[], int OptiSize, int X, int Y)
{
	int Y_TOP, Y_BOT;
	if ((0 != CalcYValueByRefCurve(Ftk_D14_RefCurve[0], ARRAY_SIZE(Ftk_D14_RefCurve[0]), &Y_TOP, X))
	||  (0 != CalcYValueByRefCurve(Ftk_D14_RefCurve[1], ARRAY_SIZE(Ftk_D14_RefCurve[1]), &Y_BOT, X))) {
		return -1;
	}

	if (Y >= Y_TOP) {
		memcpy(OptiYCurve, Ftk_D14_RefCurve[0], sizeof(Ftk_D14_RefCurve[0])); //FIXME:
		return 0;
	}

	if (Y <= Y_BOT) {
		memcpy(OptiYCurve, Ftk_D14_RefCurve[1], sizeof(Ftk_D14_RefCurve[1])); //FIXME:
		return 0;
	}

	{
		int ii;
		double DY = ((double)(Y - Y_BOT)) / (Y_TOP - Y_BOT);

		for (ii = 0; ii < ARRAY_SIZE(Ftk_D14_RefXValue); ii += 1) {
			OptiYCurve[ii] = Ftk_D14_RefCurve[1][ii] + DY * (Ftk_D14_RefCurve[0][ii] - Ftk_D14_RefCurve[1][ii]);
		}
	}

	return 0;
}
//=================================================

int OnWayAFInfo(void)
{
	unsigned long Fv1 = 0;
	unsigned long Fv2 = 0;

	int ii, jj;

	Stepper_SpeedSet(STEPPER_SPD_800PPS);
	for (ii = 0; ii < 3000; ii += 1) {
		Stepper_FocusStep(1);
		Stepper_ZoomStep(1);
	}

	Stepper_FocusStop();
	Stepper_ZoomStop();
	usleep(500*1000);

	for (ii = 0; ii < Stepper_FocusMax; ii += 1) {
		Stepper_FocusStep(0);
	}

	for (ii = 0; ii < Stepper_ZoomMax; ii += 1) {
		Stepper_ZoomStep(0);
	}
}

void SmipleFocusTest(void)
{
	int ii, jj;

	int MaxFv2 = 0;
	int MaxStep;
	int tmpFlag = 1;

	unsigned long Fv2 = 0;

	int Direction = STEPPER_FOCUS_FORWARD;

	MaxFv2    = 0;
	MaxStep   = 0;

	// 无条件遍历一遍行程，获取Peak位置
	Stepper_SpeedSet(STEPPER_SPD_800PPS);
	for (ii = 0; ii < 2500; ii += 1) {
		int tmpFocusCur = 0;

		Stepper_FocusStep(Direction);
		Stepper_FocusGetInfo(&tmpFocusCur, NULL);

		while(0 != AFStatistics_GetFV(NULL, &Fv2, 5000)) {
			usleep(20);
		}

		if(MaxFv2 < Fv2) { //获取极值及位置
			MaxFv2  = Fv2;
			MaxStep = tmpFocusCur;
		}
	}

	//运动至Peak位置
	Stepper_FocusGotoPos(MaxStep, &tmpFlag);
	Stepper_FocusStop();
}

int ForNBackWardRuning(void)
{
	int ThisStep;
	int tmpFlag = 1;

	int ii, jj;

	Stepper_FocusGetInfo(&ThisStep, NULL);

	//在当前位置往复运动, 最后回到原位，查看电机反复运动后是否存在误差
	for(ii = 0; ii < 30; ii += 1) {
		Stepper_FocusGotoPos(ThisStep-200, &tmpFlag);
		Stepper_FocusGotoPos(ThisStep+200, &tmpFlag);
	}

	Stepper_FocusGotoPos(ThisStep, &tmpFlag);
	Stepper_FocusStop();

	return 0;
}

int GetZoomStepByCurZoomPos()
{
    int curZoomPos = 0, MaxZoomPos = 0;
    int stepCnt = 0, index = 0;
    Stepper_ZoomGetInfo(&curZoomPos, &MaxZoomPos);

#define ZOOM_STEPS 2

    index = curZoomPos*ZOOM_STEP_N/MaxZoomPos;

    if(index>=0 && index<(ZOOM_STEP_N+1)/5)
    {
        stepCnt = ZOOM_STEPS*10;
    }
    else if(index >= (ZOOM_STEP_N+1)/5 && index < (ZOOM_STEP_N+1)*2/5)
    {
        stepCnt = ZOOM_STEPS*8;
    }
    else if(index >= (ZOOM_STEP_N+1)*2/5 && index < (ZOOM_STEP_N+1)*3/5)
    {
        stepCnt = ZOOM_STEPS*6;
    }
    else if(index >= (ZOOM_STEP_N+1)*3/5 && index < (ZOOM_STEP_N+1)*4/5)
    {
        stepCnt = ZOOM_STEPS*2;
    }
    else if(index >= (ZOOM_STEP_N+1)*4/5 && index < (ZOOM_STEP_N+1))
    {
        stepCnt = ZOOM_STEPS*1;
    }
    else
    {
        stepCnt = ZOOM_STEPS*2;
    }

    return stepCnt;
}

int GetGivenFocusPosByCurZoomPos(int updateCurveFlag)
{
    int finalFocus = 0;
    int curZoom = 0;
    int ret = 0;

    switch(Stepper_Type)
    {
        case STEPPER_TYPE_FOCTEK_D14:
        default:
            if(updateCurveFlag && Stepper_FocusPoint_X != -1 && Stepper_FocusPoint_Y != -1)
            {
                GetOptiCurveByPosXY(Ftk_D14_RefYValue, ARRAY_SIZE(Ftk_D14_RefYValue), Stepper_FocusPoint_X, Stepper_FocusPoint_Y);
            }
            Stepper_ZoomGetInfo(&curZoom, NULL);
            ret = CalcYValueByRefCurve(Ftk_D14_RefYValue, ARRAY_SIZE(Ftk_D14_RefYValue), &finalFocus, curZoom);
            if(ret == 0)
            {
                return finalFocus;
            }
            else
            {
                return -1;
            }
    }
}

int GotoInitFocusPos(int* pRunningFlag)
{
    int finalFocus = 0, maxFocus = 0;
    int curZoom = 0;
    Stepper_ZoomGetInfo(&curZoom, NULL);
    Stepper_FocusGetInfo(NULL, &maxFocus);
    finalFocus = GetGivenFocusPosByCurZoomPos(1);
    if(finalFocus>=0)
    {
        finalFocus = (finalFocus+50)>=maxFocus?(maxFocus-1):(finalFocus+50); //预留余量
    }
    return Stepper_FocusGotoPos(finalFocus, pRunningFlag);
}

int  FindDirection(int * GoingUpDirection, int SkipStep, int StepCount)
{
	int ii, jj;

	int LstFv2 = 0;
	int MaxFv2 = 0;

	unsigned long Fv2 = 0;

	int Direction  = 0; //Assume One Direction
	int CntThrends = 0;  //CntThrends: ++, Going UP; --, Going DOWN; ==, No Change
	int ZeroCounts = 0;

    int curFocusPos = 0, maxFocusPos = 0;

	if (!GoingUpDirection) {
		return -1;
	}

    //STEP 0 确定电机的初始步进方向
    Stepper_FocusGetInfo(&curFocusPos, &maxFocusPos);
    if(curFocusPos >= maxFocusPos/3)
    {
        Direction = STEPPER_FOCUS_BACKWARD;
    }
    else
    {
        Direction = STEPPER_FOCUS_FORWARD;
    }

	//STEP.1 获取当前值作为初值
	while(0 != AFStatistics_GetFV(NULL, &Fv2, 5000)) {
		usleep(20);
	}

	LstFv2    = Fv2;
	MaxFv2    = 0;

	//STEP.2 无条件向前运行一段距离，获取当前后续运动反向
	for (ii = 0; ii < StepCount; ii += 1) {
		for (jj = 0; jj < SkipStep; jj += 1) {
			Stepper_FocusStep(Direction);
		}

		while(0 != AFStatistics_GetFV(NULL, &Fv2, 20000)) {
			usleep(20);
		}

		if(MaxFv2 < Fv2) { //获取极值及位置
			MaxFv2  = Fv2;
		}
	}

	//STEP.3 判定当前运动结果
	if(CON_BIG_THAN(MaxFv2, LstFv2, 10)) { //前方趋势为上升
		if(CON_BIG_THAN(MaxFv2, Fv2, 4)) { //已经过极值峰，反向运动
			Direction = !Direction;
			printf("\r\n111 Go Reversed MaxFv2 %d, LstFv2 %d, Fv2 %d \r\n", MaxFv2, LstFv2, Fv2);
		}
	}
	else {  //趋势不变或下降，则反向运动
		Direction = !Direction;
		printf("\r\n222 Go Reversed MaxFv2 %d, LstFv2 %d, Fv2 %d\r\n", MaxFv2, LstFv2, Fv2);
	}

	*GoingUpDirection = Direction;

	return 0;
}

int  TryRoughFocus(int Direction, int StepSkip, int StepCount, int TrendMax, int* pRunningFlag)
{
	int ii, jj;

	int LstFv2 = 0;
	int MaxFv2 = 0;
	int MaxStep = 0;

	unsigned long Fv2 = 0;

	int CntThrends = 0;  //CntThrends: ++, Going UP; --, Going DOWN; ==, No Change
	int ZeroCounts = 0;

	//STEP.1 获取当前值作为初值
	while(0 != AFStatistics_GetFV(NULL, &Fv2, 5000) && (*pRunningFlag)) {
		usleep(20);
	}

	LstFv2    = Fv2;
	MaxFv2    = 0;
	MaxStep   = 0;

	for (ii = 0; ii < StepCount; ii += 1) {
		if(*pRunningFlag == 0)
		{
			break;
		}

		int tmpFocusCur = 0, maxFocusCur = 0;

		for (jj = 0; jj < StepSkip; jj += 1) {
			Stepper_FocusStep(Direction);
		}
		Stepper_FocusGetInfo(&tmpFocusCur, &maxFocusCur);

		while(0 != AFStatistics_GetFV(NULL, &Fv2, 20000) && (*pRunningFlag)) {
			usleep(20);
		}

		if (MaxFv2 < Fv2) { //获取极值及位置
			MaxFv2  = Fv2;
			MaxStep = tmpFocusCur;
		}

		if (CON_BIG_THAN(LstFv2, Fv2, 1)) { //数值持续下降
			CntThrends ++;
			ZeroCounts = 0;
		}
		else { //数值变化小于阈值或者持续爬升
			ZeroCounts ++;

			if(ZeroCounts > 5) {
				CntThrends = 0;
				ZeroCounts = 0;
			}
		}

		LstFv2 = Fv2;

		//printf("INFO[%d, %d, %d] cntThrends %d\r\n", ii, tmpFocusCur, Fv2, CntThrends);

        if((tmpFocusCur == 0 && Direction == STEPPER_FOCUS_BACKWARD)
            || (tmpFocusCur == maxFocusCur -1 && Direction == STEPPER_FOCUS_FORWARD)
            || CntThrends >= TrendMax)
        {
            printf("@@@ Get peek! CurPos %d, MaxPos %d, MaxStep %d\n", tmpFocusCur, maxFocusCur, MaxStep);
            //在该粗调方向上的极值点
            Stepper_FocusStop();
            usleep(10*1000);
            return Stepper_FocusGotoPos(MaxStep, pRunningFlag);
        }

	}

	return -1;
}

int  TryFineFocus(int StepBgn, int StepEnd, int uDelay, int* pRunningFlag)
{
	int MaxFv2  = 0;
	int MaxStep = 0;

	unsigned long Fv2 = 0;

	int tmpStep = 0;
	int maxFocus = 0;
	int ret = 0;

    Stepper_FocusGetInfo(NULL, &maxFocus);
    if(StepBgn < 0)
    {
        StepBgn = 0;
    }
    else if(StepBgn >= maxFocus)
    {
        StepBgn = maxFocus - 1;
    }
    
    if(StepEnd < 0)
    {
        StepEnd = 0;
    }
    else if(StepEnd >= maxFocus)
    {
        StepEnd = maxFocus - 1;
    }

	tmpStep = (StepEnd > StepBgn) ? (1) : (-1);

	while (StepBgn != StepEnd) {

		if(*pRunningFlag == 0)
		{
			return -1;
		}

		int tmpFocusCur = 0;

		Stepper_FocusGotoPos(StepBgn, pRunningFlag);
		Stepper_FocusGetInfo(&tmpFocusCur, NULL);

		usleep(uDelay); //Delay to Get Stable ISP Info.

		while(0 != AFStatistics_GetFV(NULL, &Fv2, 5000) && (*pRunningFlag)) {
			usleep(20);
		}

		if(MaxFv2 < Fv2) { //获取极值及位置
			MaxFv2  = Fv2;
			MaxStep = tmpFocusCur;
		}

		StepBgn += tmpStep;
	}

	//运动至Peak位置
	ret = Stepper_FocusGotoPos(MaxStep, pRunningFlag);
	Stepper_FocusStop();

	return ret;
}

int TryAccurateFocus(int uDelay, int* pRunningFlag)
{
    int curFocusPos = 0;
    int interval = 30; //变焦电机精调范围

    Stepper_SpeedSet(STEPPER_SPD_400PPS);  //低速高精调焦
    Stepper_FocusGetInfo(&curFocusPos, NULL);
    return TryFineFocus(curFocusPos-interval, curFocusPos+interval, uDelay, pRunningFlag);
}

//=========================
// APP MOTOR MODULE
//=========================

#define PRESET_DATA_FILE "/media/conf/netsdk/preset_data"
#define PRESERVE_DATA_FILE "/media/conf/netsdk/preserve_data"

#define MAX_PRESET_CNT 256

typedef struct{
    int bEnable;
    int ZoomPos;
    int FocusPos;
}MOTOR_PRESET_S;

typedef struct{
    int bInit;
    int bRunningFlag;//标记整个模块的工作状态，可用于强制退出模块

    pthread_t motorPid, followPid;
    int bProcessRunning, bFollowRunning;

    int bFocusRunning;

    int isZoomForward;
    int bZoomRunning;

    MOTOR_PRESET_S stMotorPreset[MAX_PRESET_CNT];

    int bPreserve;
    MOTOR_PRESET_S stMotorPreserve;
}MOTOR_CTX_S;

//用于保存到配置区的数据
typedef struct{
    int crc;
    MOTOR_PRESET_S stMotorPreset[MAX_PRESET_CNT];
}MOTOR_PRESET_DATA_S;

typedef struct{
    int crc;
    MOTOR_PRESET_S stMotorPreserve;
}MOTOR_PRESERVE_DATA_S;

static MOTOR_CTX_S s_stMotorCtx;

static int SavePresetData()
{
    FILE* fd = NULL;
    int ret = 0;
    int crc = 0;
    MOTOR_PRESET_DATA_S stPresetData;
    memset(&stPresetData, 0, sizeof(stPresetData));

    memcpy((void*)&stPresetData.stMotorPreset, (void*)&s_stMotorCtx.stMotorPreset, sizeof(s_stMotorCtx.stMotorPreset));
    crc = CRC_getByteCRC(&stPresetData, sizeof(stPresetData));
    stPresetData.crc = crc;
    printf("@@@@ SavePresetData crc %d\n", crc);
    fd = fopen(PRESET_DATA_FILE, "w+");
    if(fd == NULL)
    {
        return -1;
    }

    ret = fwrite(&stPresetData, sizeof(stPresetData), 1, fd);
    if(ret <= 0)
    {
        fclose(fd);
        return -1;
    }

    fclose(fd);

    return 0;
}

static int LoadPresetData()
{
    FILE* fd = NULL;
    int ret = 0;
    int crc = 0;
    MOTOR_PRESET_DATA_S stPresetData;
    memset(&stPresetData, 0, sizeof(stPresetData));

    if(access(PRESET_DATA_FILE, F_OK) == 0)
    {
        fd = fopen(PRESET_DATA_FILE, "r+");
        if(fd == NULL)
        {
            return -1;
        }

        ret = fread(&stPresetData, sizeof(stPresetData), 1, fd);
        if(ret <= 0)
        {
            fclose(fd);
            return -1;
        }

        fclose(fd);

        crc = stPresetData.crc;
        stPresetData.crc = 0;
        
        printf("@@@@ LoadPresetData getByteCrc %d, crc %d\n", CRC_getByteCRC(&stPresetData, sizeof(stPresetData)), crc);
        if(CRC_getByteCRC(&stPresetData, sizeof(stPresetData)) == crc)
        {
            memcpy((void*)&s_stMotorCtx.stMotorPreset, (void*)&stPresetData.stMotorPreset, sizeof(s_stMotorCtx.stMotorPreset));
        }
        else
        {
            return -1;
        }
    }
    return 0;
}

static int SavePreserveData()
{
    FILE* fd = NULL;
    int ret = 0;
    int crc = 0;
    MOTOR_PRESERVE_DATA_S stPreserveData;
    memset(&stPreserveData, 0, sizeof(stPreserveData));

    memcpy((void*)&stPreserveData.stMotorPreserve, (void*)&s_stMotorCtx.stMotorPreserve, sizeof(s_stMotorCtx.stMotorPreserve));
    crc = CRC_getByteCRC(&stPreserveData, sizeof(stPreserveData));
    stPreserveData.crc = crc;
    
    fd = fopen(PRESERVE_DATA_FILE, "w+");
    if(fd == NULL)
    {
        return -1;
    }

    ret = fwrite(&stPreserveData, sizeof(stPreserveData), 1, fd);
    if(ret <= 0)
    {
        fclose(fd);
        return -1;
    }

    fclose(fd);

    return 0;
}

static int LoadPreserveData()
{
    FILE* fd = NULL;
    int ret = 0;
    int crc = 0;
    MOTOR_PRESERVE_DATA_S stPreserveData;
    memset(&stPreserveData, 0, sizeof(stPreserveData));

    if(access(PRESERVE_DATA_FILE, F_OK) == 0)
    {
        fd = fopen(PRESERVE_DATA_FILE, "r+");
        if(fd == NULL)
        {
            return -1;
        }

        ret = fread(&stPreserveData, sizeof(stPreserveData), 1, fd);
        if(ret <= 0)
        {
            fclose(fd);
            return -1;
        }

        fclose(fd);

        //解析数据并保存
        crc = stPreserveData.crc;
        stPreserveData.crc = 0;
        if(CRC_getByteCRC(&stPreserveData, sizeof(stPreserveData)) == crc)
        {
            memcpy((void*)&s_stMotorCtx.stMotorPreserve, (void*)&stPreserveData.stMotorPreserve, sizeof(s_stMotorCtx.stMotorPreserve));
        }
        else
        {
            return -1;
        }
    }
    return 0;

}

static int DeletePreserveData()
{
    if(access(PRESERVE_DATA_FILE, F_OK) == 0)
    {
        unlink(PRESERVE_DATA_FILE);
    }

    return 0;
}

static int LoadMotorPara()
{
    ST_CUSTOM_SETTING custom;
    memset(&custom, 0, sizeof(custom));

    CUSTOM_get(&custom);

    printf("@@@ \nmotorEnabled %d, motorType %d\n", 
        custom.motor.motorEnabled, custom.motor.motorType);
    if(custom.motor.motorEnabled >= 0)
    {
        Stepper_Enable = custom.motor.motorEnabled;
    }
    else
    {
        Stepper_Enable = 0;
    }

    if(custom.motor.motorType >= 0 && custom.motor.motorType < STEPPER_TYPE_CNT)
    {
        Stepper_Type = custom.motor.motorType;
    }
    else
    {
        Stepper_Type = STEPPER_TYPE_FOCTEK_D14;
    }

    switch(Stepper_Type)
    {
        case STEPPER_TYPE_FOCTEK_D14:
        default:
            Stepper_FocusMax = FOCTEK_D14_STEPPER_FOCUSMAX;
            Stepper_ZoomMax  = FOCTEK_D14_STEPPER_ZOOMMAX;
            break;
    }
    
    return 0;
}

void* _follow_process(void* argv)
{
    int finalFocusPos = 0;
    int i = 0;
    int updateCurveFlag = 0;
    char thName[128] = {0};

    sprintf(thName, "follow_process");
    prctl(PR_SET_NAME,thName);

    s_stMotorCtx.bFollowRunning = 1;
    while(s_stMotorCtx.bFollowRunning)
    {
        updateCurveFlag = 1;
        while(s_stMotorCtx.bZoomRunning)
        {
            finalFocusPos = GetGivenFocusPosByCurZoomPos(updateCurveFlag);
            updateCurveFlag = 0;

            Stepper_SpeedSet(STEPPER_SPD_800PPS);
            Stepper_FocusGotoPos(finalFocusPos, &s_stMotorCtx.bZoomRunning);

            usleep(10);
        }

        usleep(100000);
    }

    pthread_exit(NULL);
    return NULL;
}

void* _motor_process(void* argv)
{
    int ZoomCur = 0, ZoomMax = 0;
    int FocusCur = 0, FocusMax = 0;
    int ret = 0, i = 0;
    int tmpStepCur = 0;
    char thName[128] = {0};
    int zoomIndex = 0;
    int stepCnt = 0;

    sprintf(thName, "motor_process");
    prctl(PR_SET_NAME,thName);

    s_stMotorCtx.bProcessRunning = 1;

    if(!s_stMotorCtx.bInit)
    {
        s_stMotorCtx.bInit = 1;

        LoadPresetData();
        LoadPreserveData();

        AFStatistics_Init();

        Stepper_SpeedSet(STEPPER_SPD_800PPS);
        Stepper_Init(Stepper_FocusMax, Stepper_ZoomMax, -1, -1); //初始化并强制电机位置归零校正

        if(s_stMotorCtx.stMotorPreserve.bEnable)
        {
            printf("@@@ GOTO PRESERVER POS: ZOOM %d, FOCUS %d\n", s_stMotorCtx.stMotorPreserve.ZoomPos, s_stMotorCtx.stMotorPreserve.FocusPos);
            Stepper_ZoomGotoPos(s_stMotorCtx.stMotorPreserve.ZoomPos, &s_stMotorCtx.bRunningFlag);
            Stepper_ZoomStop();
            sleep(10);  // 为了等待云台初始化完成，再进行聚焦操作

            s_stMotorCtx.bFocusRunning = 1;
            printf("@@@ finish preserver pos!\n");
        }
        else
        {
            //第一次启动时自动对焦
            APP_MOTOR_StartAutoFocus();
        }
    }

    while(s_stMotorCtx.bProcessRunning)
    {
        while(s_stMotorCtx.bZoomRunning)
        {
            Stepper_SpeedSet(STEPPER_SPD_800PPS);
            stepCnt = GetZoomStepByCurZoomPos();

            for(i=0;i<stepCnt;i++)
            {
                Stepper_ZoomStep(s_stMotorCtx.isZoomForward);
            }

            usleep(10000);

            Stepper_ZoomGetInfo(&ZoomCur, &ZoomMax);
            if((s_stMotorCtx.isZoomForward == 1 && ZoomCur == (ZoomMax - 1))
                ||(s_stMotorCtx.isZoomForward == 0 && ZoomCur == 0))
            {
                APP_MOTOR_StopZoom();
                usleep(500000);
            }
        }

        if(s_stMotorCtx.bFocusRunning) 
        {
#if 0
            //自动调焦实现
            int Direction = STEPPER_FOCUS_FORWARD;
            printf("@@@ start FindDirection!\n");
            Stepper_SpeedSet(STEPPER_SPD_800PPS); //粗略校正Peak运动方向
            if(0 == FindDirection(&Direction, LARGE_STEPS, TRENDS_COUNT) && s_stMotorCtx.bFocusRunning) 
            {
                printf("@@@ start TryRoughFocus!\n");
                Stepper_SpeedSet(STEPPER_SPD_800PPS);  //高速粗调焦
                if(0 == TryRoughFocus(Direction, SMALL_STEPS, 3200/SMALL_STEPS, TRENDS_LIMIT) && s_stMotorCtx.bFocusRunning) 
                {
                    printf("@@@ start TryFineFocus!\n");
                    int tmpStepCur = 0;
                    Stepper_FocusGetInfo(&tmpStepCur, NULL);
                    Stepper_SpeedSet(STEPPER_SPD_400PPS);  //低速高精调焦
                    TryFineFocus(tmpStepCur-30, tmpStepCur+30, 20*1000);

                    printf("AUTO-FOCUS DONE\r\n");
                    s_stMotorCtx.bPreserve = 1;
                }
                else 
                {
                    printf("AUTO-FOCUS FAILED\r\n");
                }
            }
            else
            {
                printf("FIND DIRECTION FAILED\r\n");
            }
#else
            Stepper_SpeedSet(STEPPER_SPD_800PPS);  //高速粗调焦
            if(0 == GotoInitFocusPos(&s_stMotorCtx.bFocusRunning))
            {
                printf("@@@ start TryRoughFocus!\n");
                if(0 == TryRoughFocus(STEPPER_FOCUS_BACKWARD, SMALL_STEPS, 3200/SMALL_STEPS, TRENDS_LIMIT, &s_stMotorCtx.bFocusRunning)) 
                {
                    printf("@@@ start TryAccurateFocus!\n");
                    if(0 == TryAccurateFocus(0, &s_stMotorCtx.bFocusRunning))
                    {
                        printf("AUTO-FOCUS DONE\r\n");
                        Stepper_FocusGetInfo(&Stepper_FocusPoint_Y, NULL);
                        Stepper_ZoomGetInfo(&Stepper_FocusPoint_X,NULL);
                        s_stMotorCtx.bPreserve = 1;
                    }
                }
                else 
                {
                    printf("TryRoughFocus FAILED\r\n");
                }
            }
            else
            {
                printf("GotoInitFocusPos FAILED\r\n");
            }
#endif

            s_stMotorCtx.bFocusRunning = 0;
        }

        if(s_stMotorCtx.bPreserve)
        {
            Stepper_FocusGetInfo(&FocusCur, &FocusMax);
            Stepper_ZoomGetInfo(&ZoomCur, &ZoomMax);

            s_stMotorCtx.stMotorPreserve.bEnable = 1;
            s_stMotorCtx.stMotorPreserve.FocusPos = FocusCur;
            s_stMotorCtx.stMotorPreserve.ZoomPos = ZoomCur;
            s_stMotorCtx.bPreserve = 0;

            printf("@@@ Save Preserve Data ZoomPos %d, FocusPos %d\n", ZoomCur, FocusCur);
            ret = SavePreserveData();
            if(ret != 0)
            {
                printf("@@@ Save Preserve Data failed!\n");
            }
        }

        usleep(100000);
    }
    
    pthread_exit(NULL);
    return NULL;
}

int APP_MOTOR_Init(void)
{
    int ret = 0;
    pthread_t pid = 0;
    memset(&s_stMotorCtx, 0, sizeof(s_stMotorCtx));

    LoadMotorPara();

    if(!Stepper_Enable)
    {
        printf("@@@ Stepper is not enable!\n");
        return -1;
    }

    switch(Stepper_Type)
    {
        case STEPPER_TYPE_FOCTEK_D14:
        default:
            Ftk_D14_RefCurveInit(CURVE_MAX_ZOOM, CURVE_MAX_FOCUS);
            break;
    }

    s_stMotorCtx.bRunningFlag = 1;

    ret = pthread_create(&pid, NULL, _motor_process, NULL);
    if(ret != 0)
    {
        printf("@@@ pthread_create _motor_process failed!\n");
    }
    s_stMotorCtx.motorPid = pid;

    pid = 0;
    ret = pthread_create(&pid, NULL, _follow_process, NULL);
    if(ret != 0)
    {
        printf("@@@ pthread_create _follow_process failed!\n");
    }
    s_stMotorCtx.followPid = pid;

    return 0;
}

int APP_MOTOR_Exit(void)
{
    if(s_stMotorCtx.bInit)
    {
        s_stMotorCtx.bRunningFlag = 0;

        APP_MOTOR_StopZoom();
        APP_MOTOR_StopAutoFocus();

        if(s_stMotorCtx.motorPid)
        {
            s_stMotorCtx.bProcessRunning = 0;
            pthread_join(s_stMotorCtx.motorPid, NULL);
            s_stMotorCtx.motorPid = 0;
        }

        if(s_stMotorCtx.followPid)
        {
            s_stMotorCtx.bFollowRunning = 0;
            pthread_join(s_stMotorCtx.followPid, NULL);
            s_stMotorCtx.followPid = 0;
        }
        
        Stepper_Exit();
        AFStatistics_Exit();
        memset(&s_stMotorCtx, 0, sizeof(s_stMotorCtx));
    }
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);

    return 0;
}


int APP_MOTOR_StartAutoFocus(void)
{
    if(!s_stMotorCtx.bInit)
    {
        return -1;
    }

    s_stMotorCtx.bFocusRunning = 1;
    return 0;
}

int APP_MOTOR_StopAutoFocus(void)
{
    if(!s_stMotorCtx.bInit)
    {
        return -1;
    }

    s_stMotorCtx.bFocusRunning = 0;
    return 0;
}


int APP_MOTOR_StartZoom(int isForward)
{
    if(!s_stMotorCtx.bInit)
    {
        return -1;
    }

    s_stMotorCtx.bFocusRunning = 0;
    s_stMotorCtx.bZoomRunning = 1;
    s_stMotorCtx.isZoomForward = isForward;
    return 0;
}

int APP_MOTOR_StopZoom()
{
    if(!s_stMotorCtx.bInit)
    {
        return -1;
    }

    if(s_stMotorCtx.bZoomRunning)
    {
        s_stMotorCtx.bZoomRunning = 0;

        //停止变倍后，进行自动调焦
        s_stMotorCtx.bFocusRunning = 1;
    }
    return 0;
}

int APP_MOTOR_SetPreset(int index)
{
    if(!s_stMotorCtx.bInit)
    {
        return -1;
    }

    if(index >= MAX_PRESET_CNT)
    {
        return -1;
    }

    int ZoomCur = 0, ZoomMax = 0;
    int FocusCur = 0, FocusMax = 0;
    Stepper_FocusGetInfo(&FocusCur, &FocusMax);
    Stepper_ZoomGetInfo(&ZoomCur, &ZoomMax);

    s_stMotorCtx.stMotorPreset[index].bEnable = 1;
    s_stMotorCtx.stMotorPreset[index].FocusPos = FocusCur;
    s_stMotorCtx.stMotorPreset[index].ZoomPos = ZoomCur;

    SavePresetData();

    return 0;
}

int APP_MOTOR_GotoPreset(int index)
{
    if(!s_stMotorCtx.bInit)
    {
        return -1;
    }

    int tmpStepCur = 0;

    if(index >= MAX_PRESET_CNT)
    {
        return -1;
    }

    if(s_stMotorCtx.stMotorPreset[index].bEnable)
    {
        Stepper_ZoomGotoPos(s_stMotorCtx.stMotorPreset[index].ZoomPos, &s_stMotorCtx.bRunningFlag);
        Stepper_ZoomStop();

        s_stMotorCtx.bFocusRunning = 1;
    }

    return 0;
}

int APP_MOTOR_ClearPreset(int index)
{
    if(!s_stMotorCtx.bInit)
    {
        return -1;
    }

    if(index >= MAX_PRESET_CNT)
    {
        return -1;
    }

    if(s_stMotorCtx.stMotorPreset[index].bEnable)
    {
        s_stMotorCtx.stMotorPreset[index].bEnable = 0;

        SavePresetData();
    }
    
    return 0;
}

int APP_MOTOR_ClearAllPreset()
{
    if(!s_stMotorCtx.bInit)
    {
        return -1;
    }

    memset(&s_stMotorCtx.stMotorPreset, 0, sizeof(&s_stMotorCtx.stMotorPreset));

    DeletePreserveData();
    return 0;
}

