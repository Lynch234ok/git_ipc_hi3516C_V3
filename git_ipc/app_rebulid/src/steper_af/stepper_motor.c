
#include <stepper_motor.h> //<! 模块 StepperMotor 定义文件。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include "object.h"
#include <jaThread.h>
#include <stepTable.h>

/**
 * StepperMotor 模块私有句柄，句柄访问模块内部的私有成员。\n
 * 内存在 StepperMotor 模块创建时统一分配。\n
 * 位置在句柄数据结构 @ref TJA_StepperMotor 上位，\n
 * 这样有效避免 @ref TJA_StepperMotor 内存空间被错误释放。\n
 * 如下图：\n
 *
 *  | TJA_PrivatedStepperMotor
 * \|/
 *  +------------------------+
 *  |          |             |
 *  |          |             |
 *  +------------------------+
 *            /|\
 *             | TJA_StepperMotor
 *
 */
typedef struct jaPrivatedStepperMotor {

	TJA_MemAllocator	*MemAllocator; ///< 模块内内存分配器。

	/// 控制锁，一个控制动作触发以后，停止前不能触发另外一个动作。
	TJA_Mutex			*Critical;

	/// 参考极限角度。
	struct {
		JA_Int lower, upper, home;
	} RefStep;

	/// 步进电机全步程。
	/// 通过 calibration 计算此值。
	/// 此值在模块初始化和设置参考角度之后清零。
	/// 清零后不能对步进电机进行操作，必须要调用 calibrate 重新矫正此值。
	JA_Int full_distance;

	/// 步进电机固定全步程。
	/// 针对双限位开关步进电机此值为 0，而针对但限位步进电机此值与 full_distance 一致，
	/// 在电机启动校准以后，@ref full_distance 会等于此值。
	/// 并在电机生存器内不能变更。
	JA_Int fixed_distance;

	/// 节拍周期。
	/// 触发 onStep 事件时传给用户。
	JA_Int tick_cycle;
	/// 当前的节拍数
	/// 节拍数会在设置参考角度之后清零，
	/// 每次控制电机都会更改次值，此值取值范围为 0 - tick_cycle，
	/// 触发 onStep 事件时传给用户。
	JA_Int current_tick;

	/// 步进电机当前步位。
	/// 步进电机内部操作以步位运算。
	JA_Int current_step;
	JA_Int target_step; ///< 记录转动目标步位临时变量。

	/// 步进电机角度。
	/// 步进电机外部操作以角度运算。
	JA_Float current_dregree;
	JA_Float target_degree; ///< 记录转动目标角度临时变量。

	/// 各种回调方法。
	TJA_StepperMotorOnStep onStep;
	TJA_StepperMotorOnLimit onLimitLower;
	TJA_StepperMotorOnLimit onLimitUpper;
	TJA_StepperMotorOnCalibrated onCalibrated;
	TJA_StepperMotorStop StopMotor;
	TJA_StepperMoveCallBack MoveCallBack;

	/// 后台线程控制句柄。
	TJA_Thread *Calibrator;
	TJA_Thread *Mover;

	/// 用户自定义上下文。
	JA_PVoid user_ctx;

	JA_Char magic[4]; ///< 模块幻数
} TJA_PrivatedStepperMotor;


#define typecheck(type,x) \
	({ type __dummy; \
		typeof(x) __dummy2; \
		(void)(&__dummy == &__dummy2); \
		1; \
	})

#define time_after(a,b)\
	(typecheck(unsigned long, a) &&\
		typecheck(unsigned long, b) &&\
			((long)(b)-(long)(a)<0)) // 当 a 在 b 的后面(大于)，此宏为真 

#define time_after_eq(a,b)\
	(typecheck(unsigned long, a) &&\
		typecheck(unsigned long, b) &&\
			((long)(b)-(long)(a)<=0)) // 当 a 在 b 的后面(大于)，此宏为真 

#define time_before(a,b)\
	(typecheck(unsigned long, a) &&\
		typecheck(unsigned long, b) &&\
			((long)(a)-(long)(b)<0)) // 当 a 在 b 的前面(大于)，此宏为真 

static inline void u_sleep(int us)
{
	struct timeval TimeVal;
	TimeVal.tv_sec = us / 1000000;
	TimeVal.tv_usec = us % 1000000;

	select(1, NULL, NULL, NULL, &TimeVal);
}

#define COL(x)  "\033[;" #x "m"  
#define RED	  COL(31)   
#define GRAY	  "\033[0m" 

static JA_Int ja_openDev(void)
{
	static int fd_mt = -1;

	if(fd_mt != -1)//已经打开，则返回句柄
	{
		return fd_mt;
	}
	
	fd_mt = open(TIMER_DEV_NAME, O_RDWR);
	if (-1 == fd_mt) {
		printf("open /dev/hi35xx_motor failed!\n");
		return -1;
	}

	return fd_mt;
}

static JA_Boolean ja_mSleep(const int ms)
{
	int ms_delay = ms;
	int fd;
	
	fd = ja_openDev();
	if(-1 == fd)
	{
		//u_sleep(ms_delay * 1000);
		printf(RED"use u_sleep !!!\n"GRAY);
		return JA_True;
	}

	unsigned long curMs, overMs;
	int ret = 0;
	ret = ioctl(fd, IOC_MAGIC, &curMs);
	if (ret)
	{
		printf("IOC_HIMT failed!\n");
	}

	overMs = curMs + ms_delay;

	while (time_after_eq(overMs, curMs))
	{
		ioctl(fd, IOC_MAGIC, &curMs);
	}

	return JA_True;
}

/**
 * 通过模块公有句柄获取私有句柄。
 */
static inline TJA_PrivatedStepperMotor *
PRIVATED(TJA_StepperMotor *Public)
{
	/// 偏移到私有句柄。
	return (TJA_PrivatedStepperMotor *)Public - 1;
}

/**
 * THIS 指针定义，\n
 * 定义以下为模块 API 接口实现。
 */
#define THIS TJA_StepperMotor * const Public


static JA_Boolean
StepperMotor_selfcheck(THIS)
{
	/// TODO 通过模块内部特征进行自测，并返回自测结果。
	//return JAE_STRCMP(PRIVATED(Public)->magic, "OBJ");
	return JA_True;
}


/**
 * 获取模块名称。
 */
static const JA_PChar
StepperMotor_name(THIS)
{
	return "StepperMotor";
}

/**
 * 调试打印模块。
 */
static JA_Void
StepperMotor_dump(THIS)
{
	/// TODO 模块调试终端打印输出。
}

/**
 * 模块接口线程安全标识。
 */
static JA_Boolean
StepperMotor_threadsafe(THIS)
{
	/// TODO 通过模块内部特性确认模块接口是否线程安全。
	return JA_False;
}

static inline JA_Int
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
 * 步进电机前进。
 * 根据 0 - TJA_PrivatedStepperMotor::tick_cycle 的取值范围，
 * 不断周期性递增。
 *
 */
static inline JA_Void
stepper_forward(THIS)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// 调整节拍向前递增。
	++Privated->current_tick;
	if (Privated->current_tick > Privated->tick_cycle - 1) {
		/// 返回周期开始。
		Privated->current_tick = 0;
	}

	/// 触发步进电机走一步
	Privated->onStep(Public, Privated->user_ctx, Privated->current_tick, Privated->tick_cycle);
}

/**
 * 步进电机后退。
 * 根据 0 - TJA_PrivatedStepperMotor::tick_cycle 的取值范围，
 * 不断周期性递减。
 *
 */
static inline JA_Void
stepper_backward(THIS)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// 调整节拍向前递减。
	--Privated->current_tick;
	if (Privated->current_tick < 0)
	{
		/// 返回周期末尾。
		Privated->current_tick = Privated->tick_cycle - 1;
	}

	/// 触发步进电机退一步
	Privated->onStep(Public, Privated->user_ctx, Privated->current_tick, Privated->tick_cycle);
}

/**
 * 设置参考角度。
 */
static JA_Int
StepperMotor_setRefStep(THIS, JA_Int lower, JA_Int upper, JA_Int home, JA_Boolean wait)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// 判断参数合法性。
	if (!(lower < upper)) {
		JaLog("StepperMoto: Lower/Upper Degree Invalid.");
		return -1;
	}

	if (!(home >= lower && home <= upper)) {
		JaLog("StepperMoto: Home Degree Invalid.");
		return -1;
	}

	/// 设置参数。
	Privated->RefStep.lower = lower;
	Privated->RefStep.upper = upper;
	Privated->RefStep.home = home;

	/// 清零步程和节拍数。
	Privated->current_tick = 0;
	Privated->full_distance = 0;

	return 0;
}

static JA_Int
SetMotorCurStep(THIS, JA_Int *currentStep)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	Privated->current_step = *currentStep;
	return 0;
}

/**
 * 自检电机控制后台线程。
 *
 */
static JA_Void
calibrator(TJA_Thread * const Thread, JA_Integer argc, JA_PVoid argv[])
{
	TJA_StepperMotor *const Public = argv[0];
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	JA_Int speed = Thread->speed;

	/// 触发 onCalibrated 事件。
	if (JA_Nil != Privated->onCalibrated) {
		Privated->onCalibrated(Public, Privated->user_ctx, speed);
	}
}

/**
 * 步进电机校准。
 */
static JA_Void
StepperMotor_calibrate(THIS, JA_Boolean wait, JA_Int speed, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	JA_Integer argc = 0;
	JA_PVoid argv[32];

	/// 停止转动。
	//Public->stop(Public);

	/// 启动后台线程开始校准。
	argv[argc++] = (JA_PVoid)(Public);
	Privated->Calibrator = pJaThreadKit->create(calibrator, argc, argv, speed, JA_Nil, objType);

	/// 阻塞标志置位时应当等待此线程结束。
	if (wait) {
		pJaThreadKit->wait(&Privated->Calibrator);
		pJaThreadKit->terminate(&Privated->Calibrator, JA_True);
	}
}


/**
 * 电机步进控制线程。
 * @param Thread
 */
static JA_Void
mover(TJA_Thread * const Thread, JA_Integer argc, TJA_StepperMotor * argv)
{
	JA_Boolean delayFlag = JA_False;
	TJA_StepperMotor *const Public = argv;
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	TJA_Thread * tmpThread = Thread;
	
	JA_Int speed = tmpThread->speed;
	if(1 > speed)
	{
		printf("The speed of the motor is too fast !!!: %d\n", speed);
		return;
	}
	
	TJA_StepperMotor *const Public_syn = tmpThread->syn_Motor;
	TJA_PrivatedStepperMotor *Privated_syn;
	if(JA_Nil != Public_syn)
	{
		Privated_syn = PRIVATED(Public_syn);

		JA_Int tilt_TargetStep;
		while (!Thread->terminated(Thread)
				&& Privated->current_step != Privated->target_step)
		{
			/// 一直驱动知道 current_step 与 target_step 一致时停下。
			if(Privated->current_step  < Privated->target_step) {
				/// 电机需要前进到达目标。
				stepper_forward(Public);
				++Privated->current_step; ///< 更新 current_step 此值要时刻与电机状态保持同步。
			} else {
				/// 电机需要后退到达目标。
				stepper_backward(Public);
				--Privated->current_step; ///< 更新 current_step 此值要时刻与电机状态保持同步。
			}
			
			tilt_TargetStep = getFocusStep(Privated->current_step);
			while(tilt_TargetStep != Privated_syn->current_step
				&& tilt_TargetStep <= HIGH_FOCUS_STEP)//同步时镜头也不能突出，以免顶住外壳玻璃
			{
				if(Privated_syn->current_step < tilt_TargetStep){
					stepper_forward(Public_syn);
					++Privated_syn->current_step;
				}else{
					stepper_backward(Public_syn);
					--Privated_syn->current_step;
				}
				ja_mSleep(speed);
				delayFlag = JA_True;
			}

			if(delayFlag == JA_False)ja_mSleep(speed);
			else delayFlag = JA_False;
		}
	}
	else
	{
		while (!Thread->terminated(Thread)
				&& Privated->current_step != Privated->target_step)
		{
			/// 一直驱动知道 current_step 与 target_step 一致时停下。
			if(Privated->current_step  < Privated->target_step) {
				/// 电机需要前进到达目标。
				stepper_forward(Public);
				++Privated->current_step; ///< 更新 current_step 此值要时刻与电机状态保持同步。
			} else {
				/// 电机需要后退到达目标。
				stepper_backward(Public);
				--Privated->current_step; ///< 更新 current_step 此值要时刻与电机状态保持同步。
			}

			ja_mSleep(speed);
		}
	}

	//拉低电机电压
	Privated->StopMotor(Public);

	Thread->terminate(&tmpThread, JA_True);
	/// 退出操作临界区。
	//Privated->Critical->unlock(Privated->Critical);
	JaLog("StepperMotor: Distance=%d Step=%d Degree=%.3f.",
			Privated->full_distance,
			Privated->current_step,
			Privated->current_dregree);

	pthread_exit(0);
}

/**
 * 获取步进电机信息。
 *
 * @param[out]		Info_r		电机信息数据结构引用。
 *
 * @return	获取成功返回 0，结构在 @ref Info_r 数据结构中体现，失败返回 -1。
 *
 */
static JA_Int
StepperMotor_getInfo(THIS, TJA_StepperMotorInfo *Info_r)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	JA_Float percentage = 0.0;

	/// 在全步程没有确认之前获取信息失败。
	if (!Privated->full_distance) {
		JaLog("StepperMotor: Motor Never Calibrated.");
		return -1;
	}

	Info_r->Distance.lower = 0;
	Info_r->Distance.current = Privated->current_step;
	Info_r->Distance.upper = Privated->full_distance;
	percentage = (Info_r->Degree.home - Privated->RefStep.lower) / (Privated->RefStep.upper - Privated->RefStep.lower);
	Info_r->Distance.home = (JA_Int)(((JA_Float)Privated->full_distance) * percentage);

	Info_r->Degree.lower = Privated->RefStep.lower;
	Info_r->Degree.upper = Privated->RefStep.upper;
	Info_r->Degree.home  = Privated->RefStep.home;
	Info_r->Degree.current = Privated->current_dregree;

	return 0;
}

/**
 * 驱动步进电机转动。
 */
static JA_Int
StepperMotor_move(THIS, JA_Boolean forward, JA_Boolean wait,
	JA_Int speed, TJA_StepperMotor *syn_Motor, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	if (forward) {
		/// 正方向转动。
		return Public->moveTo(Public, Privated->RefStep.upper, wait
				, speed, syn_Motor, objType);
	}

	return Public->moveTo(Public, Privated->RefStep.lower, wait
				, speed, syn_Motor, objType);
}

/**
 * 驱动步进电机转动到固定角度。
 */
static JA_Int
StepperMotor_moveTo(THIS, JA_Int step, JA_Boolean wait, 
	JA_Int speed, TJA_StepperMotor *syn_Motor, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	JA_Integer argc = 0;

	Privated->target_step = step;
	JaLog("StepperMoter: Moto Move to Degree:%.3f Step:%d.",
			Privated->target_degree, Privated->target_step);

	/// 创建后台线程控制电机步进。
	Privated->Mover = pJaThreadKit->create(mover, argc, Public, speed, syn_Motor, objType);
	if (wait) {
		/// 等待操作结束。
		pJaThreadKit->wait(&Privated->Mover);
		pJaThreadKit->Thread_free(&Privated->Mover);
	}
	return 0;
}

/**
 * 驱动步进电机转到原点位置。\n
 *
 * @param[in]		wait		阻塞操作标识，为 Ture 时此接口会等到转动操作完毕后才返回。
 * @param[in]		speed		电机速度
 * @return	操作成功返回 0，失败返回 -1。
 */
static JA_Int
StepperMotor_home(THIS, JA_Boolean wait, JA_Int speed, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	return Public->moveTo(Public, Privated->RefStep.home, wait, speed, JA_Nil, objType);
}

/**
 * 终止步进电机转动。
 */
static JA_Int
StepperMotor_stop(THIS)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// 销毁仍在运行的线程。
	if(JA_Nil != Privated->Mover) 
	{
		pJaThreadKit->terminate(&Privated->Mover, JA_True);
	}

	return 0;
}

/**
 * 判断步进电机是否转动。
 */
static JA_Boolean
StepperMotor_terminated(THIS)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	//线程仍在运行
	if(JA_Nil != Privated->Mover) 
	{
		return Privated->Mover->terminated(Privated->Mover);
	}
	
	return JA_True;
}

/**
 * 驱动步进电机转到0位置。\n
 *
 * @param[in]		wait		阻塞操作标识，为 Ture 时此接口会等到转动操作完毕后才返回。
 * @param[in]		speed		电机速度
 *
 * @return	操作成功返回 0，失败返回 -1。
 */
static JA_Int
StepperMotor_init(THIS, JA_Boolean wait, JA_Int speed
	, JA_Int max_step, TJA_StepperMotor *syn_Motor, JA_Int max_SynStep, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	Privated->current_step = max_step;	//强制设置当前位置为最大
	
	TJA_PrivatedStepperMotor *const syn_Privated = PRIVATED(syn_Motor);
	syn_Privated->current_step = max_SynStep;
	syn_Privated->target_step = 0;
	
	return Public->moveTo(Public, 0, wait, speed, syn_Motor, objType);	//然后往最小位置0跑
}

/**
 * 驱动步进电机转到0位置。\n
 *
 * @param[in]		wait		阻塞操作标识，为 Ture 时此接口会等到转动操作完毕后才返回。
 * @param[in]		speed		电机速度
 *
 * @return	操作成功返回 0，失败返回 -1。
 */
static JA_Int
StepperMotor_initTarget(THIS, JA_Boolean wait, JA_Int speed
	, JA_Int target_step, TJA_StepperMotor *syn_Motor, JA_Int target_SynStep, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);	
	Privated->target_step = target_step;	
	
	TJA_PrivatedStepperMotor *const syn_Privated = PRIVATED(syn_Motor);
	syn_Privated->target_step = target_SynStep;

	return Public->moveTo(Public, target_step, wait, speed, syn_Motor, objType);	//然后往最小位置0跑
}

/**
 * 获取当前电机所在步数。
 */
static JA_Int
StepperMotor_getStep(THIS, JA_Int *val)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	*val = Privated->current_step;

	return 0;
}

/**
 * 单限位开关的回调。
 */
static JA_Boolean
StepperMotor_onUpperLimit(THIS, JA_PVoid user_ctx)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// 单限位开关的步进电机通过当前步程限制电机高位。
	return Privated->current_step >= Privated->full_distance ? JA_True : JA_False;
}


/**
 * THIS 指针定义解除，\n
 * 定义以上为模块 API 接口实现。
 */
#undef THIS

/**
 * 创建 StepperMotor 模块句柄。\n
 * 针对双限位开关的步进电机。\n
 *
 */
TJA_StepperMotor *
JaStepperMotor_Create(TJA_MemAllocator *MemAllocator,
		JA_Int tick_cycle,
		TJA_StepperMotorOnStep onStep,
		TJA_StepperMotorOnLimit onLimitLower,
		TJA_StepperMotorOnLimit onLimitUpper,
		TJA_StepperMotorOnCalibrated onCalibrated,
		JA_PVoid user_ctx,
		TJA_StepperMotorStop StopMotor,
		TJA_StepperMoveCallBack MoveCallBack)
{
	TJA_PrivatedStepperMotor *Privated = JA_Nil;
	TJA_StepperMotor *Public = JA_Nil;

	if (!MemAllocator) {
		MemAllocator = NULL;
	}

	/// 初始化句柄。
	/// 公有句柄的内存空间仅靠私有句柄的高位。
	/// 有效防止模块公有句柄在外部被意外释放。
	Privated	= calloc(1, sizeof(TJA_PrivatedStepperMotor) + sizeof(TJA_StepperMotor));
	Public		= (TJA_StepperMotor *)(Privated + 1);

	/// 初始化私有成员。
	Privated->MemAllocator	= MemAllocator;
	snprintf(Privated->magic, sizeof(Privated->magic), "%s", "OBJ");
	/// 初始化其他私有成员。
	Privated->Critical = NULL;//JaMutex_Create(Privated->MemAllocator);
	Privated->RefStep.lower = 0;
	Privated->RefStep.upper = 3000;
	Privated->RefStep.home = 0;
	Privated->tick_cycle = tick_cycle;
	Privated->full_distance = 0;
	Privated->fixed_distance = 0;
	Privated->current_dregree = 0.0;
	Privated->current_step = 0;
	Privated->target_degree = 0.0;
	Privated->target_step = 0;
	Privated->onStep = onStep;
	Privated->onLimitLower = onLimitLower;
	Privated->onLimitUpper = onLimitUpper;
	Privated->onCalibrated = onCalibrated;
	Privated->Calibrator = JA_Nil;
	Privated->Mover = JA_Nil;
	Privated->user_ctx = user_ctx;
	Privated->StopMotor = StopMotor;
	Privated->MoveCallBack = MoveCallBack;

	/// 初始化公有成员。
	/// 初始化其他公有成员。
	Public->setRefStep			= StepperMotor_setRefStep;//设置步数
	Public->calibrate			= StepperMotor_calibrate;
	Public->getInfo				= NULL;//StepperMotor_getInfo;
	Public->getStep				= StepperMotor_getStep;
	
	Public->move				= StepperMotor_move;//内部也是调用moveTo
	Public->moveTo				= StepperMotor_moveTo;
	Public->home				= StepperMotor_home;
	Public->stop				= StepperMotor_stop;
	Public->Init				= StepperMotor_init;
	Public->InitTarget			= StepperMotor_initTarget;
	Public->Terminated			= StepperMotor_terminated;
	Public->setCurStep			= SetMotorCurStep;
	/// 其他初始化操作。

	/// 返回公有句柄。
	return Public;
}


/**
 * 创建 StepperMotor 模块句柄。\n
 * 针对单限位开关的步进电机。\n
 *
 */
TJA_StepperMotor *
JaStepperMotor_Create2(TJA_MemAllocator *MemAllocator,
		JA_Int tick_cycle,
		TJA_StepperMotorOnStep onStep,
		TJA_StepperMotorOnLimit onLimitLower,
		JA_Int full_distance,
		TJA_StepperMotorOnCalibrated onCalibrated,
		JA_PVoid user_ctx,
		TJA_StepperMotorStop StopMotor,
		TJA_StepperMoveCallBack MoveCallBack)
{
	TJA_StepperMotor *Public = JA_Nil;
	TJA_PrivatedStepperMotor *Privated = JA_Nil;

	if (full_distance > 0) {

		/// 初始化句柄。
		Public = JaStepperMotor_Create(MemAllocator, tick_cycle,
				onStep, onLimitLower, StepperMotor_onUpperLimit, 
				onCalibrated, user_ctx, StopMotor, MoveCallBack);
		if (JA_Nil != Public) {
			/// 更新内部参数。
			Privated = PRIVATED(Public);

			/// 更新全步程。
			Privated->fixed_distance = full_distance;
		}
	} else {
		JaLog("StepperMotor: Setup Distance(%d) Error.", full_distance);
	}

	return Public;
}

/**
 * 销毁 StepperMotor 模块句柄。
 */
JA_Void
JaStepperMotor_Free(TJA_StepperMotor **StepperMotor_r)
{
	TJA_StepperMotor *Public = JA_Nil;
	TJA_PrivatedStepperMotor *Privated = JA_Nil;
	TJA_MemAllocator *MemAllocator = JA_Nil;

	/// 获取私有句柄。
	Public = StepperMotor_r[0];
	Privated = PRIVATED(Public);
	StepperMotor_r[0] = JA_Nil; ///< 操作前先清空句柄防止上层用户在释放过程中意外调用。
	MemAllocator = Privated->MemAllocator;

	/// 停止步进电机。
	Public->stop(Public);

	/// 销毁仍在运行的线程。
	if (JA_Nil != Privated->Mover) {
		pJaThreadKit->terminate(&Privated->Mover, JA_True);
	}
	if (JA_Nil != Privated->Calibrator) {
		pJaThreadKit->terminate(&Privated->Calibrator, JA_True);
	}

	/// 释放模块句柄。
	MemAllocator->free(MemAllocator, Privated);
	Privated = JA_Nil;
}





