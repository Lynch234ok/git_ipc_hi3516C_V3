
#include <pan_tilt.h> //<! 单体模块 PanTilt 定义文件。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stepper_motor.h>
#include "bsp_io.h"
#include <stepTable.h>

#define TMP_DIR "/tmp/PanTilt"

/**
 * PanTilt 模块私有句柄，句柄访问模块内部的私有成员。\n
 * 内存在 PanTilt 模块创建时统一分配。\n
 * 位置在句柄数据结构 @ref TJA_PanTilt 上位，\n
 * 这样有效避免 @ref TJA_PanTilt 内存空间被错误释放。\n
 * 如下图：\n
 *
 *  | TJA_PrivatedPanTilt
 * \|/
 *  ++++++++++++++++++++++++++
 *  |          |             |
 *  |          |             |
 *  ++++++++++++++++++++++++++
 *            /|\
 *             | TJA_PanTilt
 *
 */
typedef struct jaPrivatedPanTilt {

	/// 水平控制电机。
	TJA_StepperMotor *PanMotor;

	/// 垂直控制电机句柄。
	TJA_StepperMotor *TiltMotor;

	/// Auto Pan 后台线程。
	TJA_Thread *AutoPan;

} TJA_PrivatedPanTilt;

TJA_PanTilt *_PstepMotor = NULL;

/**
 * 定义内部单体模块句柄。
 */
static struct {

	/// 私有句柄。
	TJA_PrivatedPanTilt Privated;

	/// 公有句柄。
	TJA_PanTilt Public;

} _Singleton, *_PSingleton = JA_Nil;

/**
 *  单体模块私有句柄。
 */
static TJA_PrivatedPanTilt *const
_Privated = &_Singleton.Privated;

/**
 * 单体模块公有句柄。
 */
static TJA_PanTilt *const
_Public = &_Singleton.Public;

int ZOOM_TBL[SEG_NUM] = {20, 50, 90, 180, 373, 565, 758, 950, 1143, 1335, 1528, 1720, 1913, 2106}; 
int FOCUS_TBL[SEG_NUM] = {0, 205, 208, 219, 244, 291, 350, 438, 574, 703, 902, 1158, 1505, 1983};

inline JA_Int
getFocusStep(JA_Int CurZoomStep)
{
	JA_Int index, reVal;

	for(index = 0; index < SEG_NUM; index++)
	{
		if(CurZoomStep < ZOOM_TBL[index]) break;
	}
	
	if(index == 0)
	{
		reVal = FOCUS_TBL[index];
	}
	else if(index > 0 && index < SEG_NUM)
	{
		reVal = FOCUS_TBL[index - 1] 
				+ (CurZoomStep - ZOOM_TBL[index - 1]) 
					* (FOCUS_TBL[index] - FOCUS_TBL[index - 1]) 
					/ (ZOOM_TBL[index] - ZOOM_TBL[index - 1]);

	}
	else
	{
		reVal = FOCUS_TBL[index - 1]
				+ (CurZoomStep - ZOOM_TBL[index - 1]) 
					* (FOCUS_TBL[index - 1] - FOCUS_TBL[index - 2]) 
					/ (ZOOM_TBL[index - 1] - ZOOM_TBL[index - 2]);
	}

	return reVal;
}

/**
 * 水平转动。\n
 *
 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
 * @param[in]		up_down		左右转动标识，向上转动时为 True 向下转动时为 False。
 * @param[in]		sync		同步操作标识，为 True 时会等待此动作执行完成或者调用 stop 中断后才返回。
 *
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int PanTilt_pan(JA_Boolean force, JA_Boolean up_down, JA_Boolean sync, JA_Int speed)
{
	//强制结束
	if(force)
	{
		_Public->stop();
	}

	_Public->EnableFocus = JA_False;

	// 左方向转动
	return _Privated->PanMotor->move(_Privated->PanMotor, up_down
			, sync, speed, _Privated->TiltMotor, OBJ_PAN);
}

/**
 * 水平转动至指定角度。\n
 *
 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
 * @param[in]		degree		水平转动到的目标角度，角度必须为合理范围内。
 * @param[in]		sync		同步操作标识，为 True 时会等待此动作执行完成或者调用 stop 中断后才返回。
 * @param[in]		speed
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int PanTilt_panTo(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed)
{
	//强制结束
	if(force)
	{
		_Public->stop();
	}
	
	//转动到固定角度degree
	return _Privated->PanMotor->moveTo(_Privated->PanMotor, step
			, sync, speed, _Privated->TiltMotor, OBJ_PAN);
}

/**
 * 水平自动后台线程。
 *
 */
static JA_Void auto_pan(TJA_Thread * const Thread, JA_Integer argc, JA_PVoid argv[], JA_Int speed)
{
	TJA_StepperMotor *const PanMotor = _Privated->PanMotor;

	while (!Thread->terminated(Thread)) {

		/// 水平转动到低位尽头。
		PanMotor->move(PanMotor, JA_False, JA_True, speed, JA_Nil, OBJ_PAN);
		Thread->suspend(Thread, 0, 100, 0);

		/// 水平转动到高位尽头。
		PanMotor->move(PanMotor, JA_True, JA_True, speed, JA_Nil, OBJ_PAN);
		Thread->suspend(Thread, 0, 100, 0);
	}
}

/**
 * 水平遍历转动。\n
 * 调用此接口之后云台会一直保持水平遍历转动，直至被其他动作打断或者 stop 中断。
 *
 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
 *
 */
static JA_Int PanTilt_autoPan(JA_Boolean force)
{
	/// 强制停止。
	if(force) {
		_Public->stop();
	}
	return 0;
}


/**
 * 垂直转动。\n
 *
 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
 * @param[in]		left_right	左右转动标识，向作转动时为 True 向右摆动时为 False。
 * @param[in]		sync		同步操作标识，为 True 时会等待此动作执行完成或者调用 stop 中断后才返回。
 *
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int PanTilt_tilt(JA_Boolean force, JA_Boolean up_down, JA_Boolean sync, JA_Int speed)
{
	//强制结束
	if(force)
	{
		_Public->stop();
	}

	// 左方向转动
	return _Privated->TiltMotor->move(_Privated->TiltMotor, !up_down
				, sync, speed, JA_Nil, OBJ_TILT);
}

/**
 * 垂直转动至指定角度。\n
 *
 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
 * @param[in]		degree		水平转动到的目标角度，角度必须为合理范围内。
 * @param[in]		sync			同步操作标识，为 True 时会等待此动作执行完成或者调用 stop 中断后才返回。
 * @param[in]		speed
 *
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int PanTilt_tiltTo(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed)
{
	//强制结束
	if(force)
	{
		_Public->stop();
	}

	//转动到固定角度degree
	return _Privated->TiltMotor->moveTo(_Privated->TiltMotor, step
			, sync, speed, JA_Nil, OBJ_TILT);
}

/**
 * 云台转动到原始的位置。
 *
 * @param[in]		force
 * @param[in]		sync
 *
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int
PanTilt_home(JA_Boolean force, JA_Boolean sync, JA_Int speed, enObjType objType)
{
	//强制结束
	if(force)
	{
		_Public->stop();
	}

	//转动到home
	return (_Privated->PanMotor->home(_Privated->PanMotor, sync, speed, objType) 
			|| _Privated->TiltMotor->home(_Privated->TiltMotor, sync, speed, objType));
}

/**
 * 云台转动到指定的位置。
 *
 * @param[in]		force
 * @param[in]		pan_degree
 * @param[in]		titl_degree
 * @param[in]		sync
 *
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int
PanTilt_goTo(JA_Boolean force, JA_Int pan_step, JA_Int titl_step, JA_Boolean sync, JA_Int speed)
{
	//强制结束
	if(force)
	{
		_Public->stop();
	}

	return (_Public->panTo(force, pan_step, sync, speed) 
		|| _Public->tiltTo(force, titl_step, sync, speed));
}

/**
 * 获取当前云台所在角度。
 *
 * @param[out]		pan_degree_r
 * @param[out]		titl_degree_r
 *
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int
PanTilt_getStep(JA_Int *pan_degree_r, JA_Int *titl_degree_r)
{
	return (_Privated->PanMotor->getStep(_Privated->PanMotor, pan_degree_r)
			|| _Privated->TiltMotor->getStep(_Privated->TiltMotor, titl_degree_r));
}

static JA_Int
PanTilt_setStep(JA_Int *pan_degree_r, JA_Int *titl_degree_r)
{
	return (_Privated->PanMotor->setCurStep(_Privated->PanMotor, pan_degree_r)
			|| _Privated->TiltMotor->setCurStep(_Privated->TiltMotor, titl_degree_r));
}

static JA_Int
PanTilt_setTargetStep(JA_Int *pan_degree_r, JA_Int *titl_degree_r)
{
	_Public->PanStepBackup = *pan_degree_r;
	_Public->TiltStepBackup = *titl_degree_r;
	
	return 0;
}

static JA_Int
Pan_stop(void)
{	
	int reVal;
	reVal = _Privated->PanMotor->stop(_Privated->PanMotor);
	usleep(50);
	return reVal;
}

static JA_Int
Tilt_stop(void)
{
	int reVal;
	reVal = _Privated->TiltMotor->stop(_Privated->TiltMotor);
	usleep(50);
	return reVal;
}

/**
 * 停止上一次执行动作。\n
 *
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int
PanTilt_stop(void)
{
	int reVal;
	reVal = (_Privated->PanMotor->stop(_Privated->PanMotor)
			|| _Privated->TiltMotor->stop(_Privated->TiltMotor));
	usleep(50000);
	return reVal;

}

/**
 * 云台转动到0的位置。
 *
 * @param[in]		force
 * @param[in]		sync
 * @param[in]		speed
 * @return	执行成功返回 0，执行失败返回 -1。
 */
static JA_Int
PanTilt_Init(JA_Boolean force, JA_Boolean sync, JA_Int speed)
{
	//强制结束
	if(force)
	{
		_Public->stop();
	}

	_Public->EnableFocus = false;
	_Public->isInit	= true;
	//转动到home
	return ( _Privated->PanMotor->Init(_Privated->PanMotor, sync, speed
			, ZoomStep+200, _Privated->TiltMotor, FocusStep + 200, OBJ_PAN) );
}

static JA_Int
PanTilt_InitTarget(JA_Boolean force, JA_Boolean sync, JA_Int speed)
{
	_Public->EnableFocus = false;
	//转动到home
	return ( _Privated->PanMotor->InitTarget(_Privated->PanMotor, sync, speed
			, _Public->PanStepBackup, _Privated->TiltMotor, _Public->TiltStepBackup, OBJ_PAN) );
}

static JA_Boolean
PanTilt_Terminated(void)
{
	return _Privated->TiltMotor->Terminated(_Privated->TiltMotor);
}

/**
 * 初始化单体句柄。
 */
static inline JA_Void
SINGLETON_CREATE()
{
	/// 复位单体数据内容。
	memset(&_Singleton, 0, sizeof(_Singleton));
	/// 设置句柄
	_PSingleton = &_Singleton;
}

/**
 * 释放单体句柄。
 */
static inline JA_Void
SINGLETON_FREE()
{
	/// 清空句柄
	_PSingleton = JA_Nil;
	/// 复位单体数据内容。
	memset(&_Singleton, 0, sizeof(_Singleton));
}

/**
 * 释放单体模块。
 */
JA_Void
PanTilt_free()
{
	/// TODO 销毁模块内资源
	if(JA_Nil != _Privated->PanMotor)
	{
		JaStepperMotor_Free(&(_Privated->PanMotor));
	}

	if(JA_Nil != _Privated->TiltMotor)
	{
		JaStepperMotor_Free(&(_Privated->TiltMotor));
	}

	/// 释放单体句柄。
	//SINGLETON_FREE();
}

/**
 * foucus电机步进驱动。
 *
 */
static JA_Int
tilt_onStep(TJA_StepperMotor *const Motor, JA_PVoid user_ctx, JA_Int tick, JA_Int tick_cycle)
{
	//2489
	if(4 == tick_cycle) {
		switch (tick)
		{
			/**
			 * Step[0] HHLL
			 * Step[1] LHHL
			 * Step[2] LLHH
			 * Step[3] HLLH
			 *
			 */
			case 0:
			case 1:
			case 2:
				BSP_IO_TILT_CTRL(3 << tick);
				break;
			case 3:
				BSP_IO_TILT_CTRL(0x09);
				break;

			default:
				break;
		}
	} else if(8 == tick_cycle) {
		//*八拍
		switch (tick)
		{
			/**
			 * Step[0] HLLL
			 * Step[1] HHLL
			 * Step[2] LHLL
			 * Step[3] LHHL
			 * Step[4] LLHL
			 * Step[5] LLHH
			 * Step[6] LLLH
			 * Step[7] HLLH
			 *
			 */
			case 0:
			case 2:
			case 4:
			case 6:
				BSP_IO_TILT_CTRL(1 << (tick / 2));
				break;

			case 1:
			case 3:
			case 5:
				BSP_IO_TILT_CTRL(3 << (tick / 2));
				break;

			case 7:
				BSP_IO_TILT_CTRL(0x09);
				break;

			default:
				break;
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

static JA_Boolean
tilt_onLimit(TJA_StepperMotor *const Motor, JA_PVoid user_ctx)
{
	/// 根据限位开关的状态反馈。
	//return BSP_IO_TILT_LIMIT();
}

static JA_Void
tilt_onCalibrated(TJA_StepperMotor *const Motor, JA_PVoid user_ctx, JA_Int speed)
{
	/// TODO
}

/**
 * zoom电机步进驱动。
 *
 */
static JA_Int
pan_onStep(TJA_StepperMotor *const Motor, JA_PVoid user_ctx, JA_Int tick, JA_Int tick_cycle)
{
	//2489
	if(4 == tick_cycle) {
		switch (tick)
		{
			/**
			 * Step[0] HHLL
			 * Step[1] LHHL
			 * Step[2] LLHH
			 * Step[3] HLLH
			 *
			 */
			case 0:
			case 1:
			case 2:
				BSP_IO_PAN_CTRL(3 << tick);
				break;
			case 3:
				BSP_IO_PAN_CTRL(0x09);
				break;

			default:
				break;
		}
	} else if(8 == tick_cycle) {
		//*八拍
		switch (tick)
		{
			/**
			 * Step[0] HLLL
			 * Step[1] HHLL
			 * Step[2] LHLL
			 * Step[3] LHHL
			 * Step[4] LLHL
			 * Step[5] LLHH
			 * Step[6] LLLH
			 * Step[7] HLLH
			 *
			 */
			case 0:
			case 2:
			case 4:
			case 6:
				BSP_IO_PAN_CTRL(1 << (tick / 2));
				break;

			case 1:
			case 3:
			case 5:
				BSP_IO_PAN_CTRL(3 << (tick / 2));
				break;

			case 7:
				BSP_IO_PAN_CTRL(0x09);
				break;

			default:
				break;
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

static JA_Int
tilt_Stop(TJA_StepperMotor *const Motor)
{
	BSP_IO_TILT_CTRL(0);

	return 0;
}

static JA_Int
pan_Stop(TJA_StepperMotor *const Motor)
{
	BSP_IO_PAN_CTRL(0);

	if(_Public->goTarget == JA_True)
	{
		_Public->goTarget = JA_False;
	}

	if(_Public->TestCurve == JA_False
		&& _Public->goTarget == JA_False)
	{
		_Public->EnableFocus = JA_True;
		_Public->StopFocus = JA_True;
	}

	if(_Public->isInit == JA_True)
	{
		_Public->isInit	= JA_False;
		//_Public->goTarget = JA_True;
		_Public->goTargetPulse = JA_True;
	}
	
	return 0;
}

static JA_Boolean
pan_onLimit(TJA_StepperMotor *const Motor, JA_PVoid user_ctx)
{
	/// 根据限位开关的状态反馈。
	//return BSP_IO_PAN_LIMIT();
}

static JA_Void
pan_onCalibrated (TJA_StepperMotor *const Motor, JA_PVoid user_ctx, JA_Int speed)
{
	/// TODO
	/*
	JA_Int pan_CurStep, tilt_CurStep, tilt_TargetStep;
	
	_Public->getStep(&pan_CurStep, &tilt_CurStep);
	tilt_TargetStep = getFocusStep(pan_CurStep);
	
	if(tilt_TargetStep > CALCULATE_INTER)
	{
		_Privated->TiltMotor->stop(_Privated->TiltMotor);
		_Privated->TiltMotor->moveTo(_Privated->TiltMotor, tilt_TargetStep
									, JA_True, speed, JA_Nil);
	}
	else
	{
		printf("tilt_TargetStep < CALCULATE_INTER");
	}
	*/
}

static 	JA_Int
testCamera(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed)
{
	_Public->TestCurve = JA_True;
	//强制结束
	if(force)
	{
		_Public->stop();
	}

	if(_Public->CurSeg >= SEG_NUM)
	{
		_Public->CurSeg = 0;
	}
	
	if(_Public->CurSeg <= (SEG_NUM - 1)){

		printf("#####CurSeg zoom step = %d\n", ZOOM_TBL[_Public->CurSeg]);
		
		//转动到固定步数step
		_Privated->PanMotor->moveTo(_Privated->PanMotor, ZOOM_TBL[_Public->CurSeg++]
				, sync, speed, JA_Nil, OBJ_PAN);

		_Public->TestCurve = JA_False;
		
		return 0;
	}
	else{
		_Public->TestCurve = JA_False;
		return -1;
	}
}


static JA_Void
pan_move_call_back(void)
{/*
	JA_Int pan_CurStep, tilt_CurStep, tilt_TargetStep;
	
	_Public->getStep(&pan_CurStep, &tilt_CurStep);
	tilt_TargetStep = ALPHA_STEP*pan_CurStep + BELTA_STEP;

	printf("pan_move_call_back*** pan_CurStep = %d\n", pan_CurStep);
	printf("pan_move_call_back*** tilt_TargetStep = %d\n", tilt_TargetStep);
	if(tilt_TargetStep > CALCULATE_INTER)
	{
		printf("pan_move_call_back*** pan_onCalibrated reset \n");
		_Privated->TiltMotor->stop(_Privated->TiltMotor);
		_Privated->TiltMotor->moveTo(_Privated->TiltMotor, tilt_TargetStep
									, false, ZOOM_SPEED, JA_Nil);
	}
	else
	{
		printf("tilt_TargetStep < CALCULATE_INTER");
	}
*/
}

/**
 * 单体句柄。
 *
 */
TJA_PanTilt *
PanTilt(void)
{
	TJA_PrivatedPanTilt *const Privated = &_Singleton.Privated;
	TJA_PanTilt *const Public = &_Singleton.Public;

	if (!_PSingleton) {
		/// 初始化单体句柄。
		SINGLETON_CREATE();

		/// 初始化私有成员。
		/// 初始化水平电机。
		Privated->PanMotor = JaStepperMotor_Create(JA_Nil,
					4, ///< 4 拍。
					pan_onStep, pan_onLimit, pan_onLimit
					, JA_Nil, JA_Nil, pan_Stop, JA_Nil);

		/// 初始化垂直电机。
		Privated->TiltMotor = JaStepperMotor_Create(JA_Nil,
					4, ///< 4 拍。
					tilt_onStep, tilt_onLimit, tilt_onLimit, tilt_onCalibrated
					, JA_Nil, tilt_Stop, JA_Nil);

		/// 初始化公有成员。
		Public->manual				= JA_False;
		Public->isInit				= JA_False;
		Public->goTarget			= JA_False;
		Public->goTargetPulse		= JA_False;
		Public->TestCurve			= JA_False;
		Public->EnableFocus			= JA_False;
		Public->StopFocus			= JA_False;
		Public->isReset				= JA_False;
		Public->CheckTarget			= JA_False;
		Public->FocusGoTo			= JA_False;
		Public->stopPlace			= 0;
		Public->CurSeg				= 0;
		Public->PanStepBackup		= ZOOM_TBL[3];
		Public->TiltStepBackup		= FOCUS_TBL[3];
		Public->free				= PanTilt_free;
		Public->pan 				= PanTilt_pan;		//zoom电机上下转动
		Public->panTo				= PanTilt_panTo;	//zoom电机转动到指定步数
		Public->autoPan				= NULL;
		Public->stopPan				= Pan_stop;
		Public->tilt				= PanTilt_tilt;		//foucus电机上下转动
		Public->tiltTo				= PanTilt_tiltTo;	//foucus电机转动到指定步数
		Public->stopTilt			= Tilt_stop;
		
		Public->home				= PanTilt_home;		//两电机均复位
		Public->goTo				= PanTilt_goTo;		//两电机分别转动到指定步数
		Public->getStep				= PanTilt_getStep;	//获取两电机的步数
		Public->stop				= PanTilt_stop;		//停止两电机
		Public->Init				= PanTilt_Init; 	//驱动电机到初始lower位置
		Public->InitTarget			= PanTilt_InitTarget;	//驱动电机到初始上次记录的位置
		Public->Terminated			= PanTilt_Terminated;	//等待focus电机停止
		Public->setCurStep 			= PanTilt_setStep;
		Public->setTargetStep		= PanTilt_setTargetStep;
		Public->testCamera			= testCamera;
		/// 其他初始化操作。
		Privated->PanMotor->setRefStep(Privated->PanMotor, LOW_ZOOM_STEP, HIGH_ZOOM_STEP, LOW_ZOOM_STEP, JA_False); ///< 异步校准。
		Privated->TiltMotor->setRefStep(Privated->TiltMotor, LOW_FOCUS_STEP, HIGH_FOCUS_STEP, LOW_FOCUS_STEP, JA_False); ///< 异步校准。

	}

	/// 返回公有句柄。
	return Public;
}

