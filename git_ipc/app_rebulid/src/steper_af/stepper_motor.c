
#include <stepper_motor.h> //<! ģ�� StepperMotor �����ļ���
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
 * StepperMotor ģ��˽�о�����������ģ���ڲ���˽�г�Ա��\n
 * �ڴ��� StepperMotor ģ�鴴��ʱͳһ���䡣\n
 * λ���ھ�����ݽṹ @ref TJA_StepperMotor ��λ��\n
 * ������Ч���� @ref TJA_StepperMotor �ڴ�ռ䱻�����ͷš�\n
 * ����ͼ��\n
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

	TJA_MemAllocator	*MemAllocator; ///< ģ�����ڴ��������

	/// ��������һ�����ƶ��������Ժ�ֹͣǰ���ܴ�������һ��������
	TJA_Mutex			*Critical;

	/// �ο����޽Ƕȡ�
	struct {
		JA_Int lower, upper, home;
	} RefStep;

	/// �������ȫ���̡�
	/// ͨ�� calibration �����ֵ��
	/// ��ֵ��ģ���ʼ�������òο��Ƕ�֮�����㡣
	/// ������ܶԲ���������в���������Ҫ���� calibrate ���½�����ֵ��
	JA_Int full_distance;

	/// ��������̶�ȫ���̡�
	/// ���˫��λ���ز��������ֵΪ 0������Ե���λ���������ֵ�� full_distance һ�£�
	/// �ڵ������У׼�Ժ�@ref full_distance ����ڴ�ֵ��
	/// ���ڵ���������ڲ��ܱ����
	JA_Int fixed_distance;

	/// �������ڡ�
	/// ���� onStep �¼�ʱ�����û���
	JA_Int tick_cycle;
	/// ��ǰ�Ľ�����
	/// �������������òο��Ƕ�֮�����㣬
	/// ÿ�ο��Ƶ��������Ĵ�ֵ����ֵȡֵ��ΧΪ 0 - tick_cycle��
	/// ���� onStep �¼�ʱ�����û���
	JA_Int current_tick;

	/// ���������ǰ��λ��
	/// ��������ڲ������Բ�λ���㡣
	JA_Int current_step;
	JA_Int target_step; ///< ��¼ת��Ŀ�경λ��ʱ������

	/// ��������Ƕȡ�
	/// ��������ⲿ�����ԽǶ����㡣
	JA_Float current_dregree;
	JA_Float target_degree; ///< ��¼ת��Ŀ��Ƕ���ʱ������

	/// ���ֻص�������
	TJA_StepperMotorOnStep onStep;
	TJA_StepperMotorOnLimit onLimitLower;
	TJA_StepperMotorOnLimit onLimitUpper;
	TJA_StepperMotorOnCalibrated onCalibrated;
	TJA_StepperMotorStop StopMotor;
	TJA_StepperMoveCallBack MoveCallBack;

	/// ��̨�߳̿��ƾ����
	TJA_Thread *Calibrator;
	TJA_Thread *Mover;

	/// �û��Զ��������ġ�
	JA_PVoid user_ctx;

	JA_Char magic[4]; ///< ģ�����
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
			((long)(b)-(long)(a)<0)) // �� a �� b �ĺ���(����)���˺�Ϊ�� 

#define time_after_eq(a,b)\
	(typecheck(unsigned long, a) &&\
		typecheck(unsigned long, b) &&\
			((long)(b)-(long)(a)<=0)) // �� a �� b �ĺ���(����)���˺�Ϊ�� 

#define time_before(a,b)\
	(typecheck(unsigned long, a) &&\
		typecheck(unsigned long, b) &&\
			((long)(a)-(long)(b)<0)) // �� a �� b ��ǰ��(����)���˺�Ϊ�� 

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

	if(fd_mt != -1)//�Ѿ��򿪣��򷵻ؾ��
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
 * ͨ��ģ�鹫�о����ȡ˽�о����
 */
static inline TJA_PrivatedStepperMotor *
PRIVATED(TJA_StepperMotor *Public)
{
	/// ƫ�Ƶ�˽�о����
	return (TJA_PrivatedStepperMotor *)Public - 1;
}

/**
 * THIS ָ�붨�壬\n
 * ��������Ϊģ�� API �ӿ�ʵ�֡�
 */
#define THIS TJA_StepperMotor * const Public


static JA_Boolean
StepperMotor_selfcheck(THIS)
{
	/// TODO ͨ��ģ���ڲ����������Բ⣬�������Բ�����
	//return JAE_STRCMP(PRIVATED(Public)->magic, "OBJ");
	return JA_True;
}


/**
 * ��ȡģ�����ơ�
 */
static const JA_PChar
StepperMotor_name(THIS)
{
	return "StepperMotor";
}

/**
 * ���Դ�ӡģ�顣
 */
static JA_Void
StepperMotor_dump(THIS)
{
	/// TODO ģ������ն˴�ӡ�����
}

/**
 * ģ��ӿ��̰߳�ȫ��ʶ��
 */
static JA_Boolean
StepperMotor_threadsafe(THIS)
{
	/// TODO ͨ��ģ���ڲ�����ȷ��ģ��ӿ��Ƿ��̰߳�ȫ��
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
 * �������ǰ����
 * ���� 0 - TJA_PrivatedStepperMotor::tick_cycle ��ȡֵ��Χ��
 * ���������Ե�����
 *
 */
static inline JA_Void
stepper_forward(THIS)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// ����������ǰ������
	++Privated->current_tick;
	if (Privated->current_tick > Privated->tick_cycle - 1) {
		/// �������ڿ�ʼ��
		Privated->current_tick = 0;
	}

	/// �������������һ��
	Privated->onStep(Public, Privated->user_ctx, Privated->current_tick, Privated->tick_cycle);
}

/**
 * ����������ˡ�
 * ���� 0 - TJA_PrivatedStepperMotor::tick_cycle ��ȡֵ��Χ��
 * ���������Եݼ���
 *
 */
static inline JA_Void
stepper_backward(THIS)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// ����������ǰ�ݼ���
	--Privated->current_tick;
	if (Privated->current_tick < 0)
	{
		/// ��������ĩβ��
		Privated->current_tick = Privated->tick_cycle - 1;
	}

	/// �������������һ��
	Privated->onStep(Public, Privated->user_ctx, Privated->current_tick, Privated->tick_cycle);
}

/**
 * ���òο��Ƕȡ�
 */
static JA_Int
StepperMotor_setRefStep(THIS, JA_Int lower, JA_Int upper, JA_Int home, JA_Boolean wait)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// �жϲ����Ϸ��ԡ�
	if (!(lower < upper)) {
		JaLog("StepperMoto: Lower/Upper Degree Invalid.");
		return -1;
	}

	if (!(home >= lower && home <= upper)) {
		JaLog("StepperMoto: Home Degree Invalid.");
		return -1;
	}

	/// ���ò�����
	Privated->RefStep.lower = lower;
	Privated->RefStep.upper = upper;
	Privated->RefStep.home = home;

	/// ���㲽�̺ͽ�������
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
 * �Լ������ƺ�̨�̡߳�
 *
 */
static JA_Void
calibrator(TJA_Thread * const Thread, JA_Integer argc, JA_PVoid argv[])
{
	TJA_StepperMotor *const Public = argv[0];
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	JA_Int speed = Thread->speed;

	/// ���� onCalibrated �¼���
	if (JA_Nil != Privated->onCalibrated) {
		Privated->onCalibrated(Public, Privated->user_ctx, speed);
	}
}

/**
 * �������У׼��
 */
static JA_Void
StepperMotor_calibrate(THIS, JA_Boolean wait, JA_Int speed, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	JA_Integer argc = 0;
	JA_PVoid argv[32];

	/// ֹͣת����
	//Public->stop(Public);

	/// ������̨�߳̿�ʼУ׼��
	argv[argc++] = (JA_PVoid)(Public);
	Privated->Calibrator = pJaThreadKit->create(calibrator, argc, argv, speed, JA_Nil, objType);

	/// ������־��λʱӦ���ȴ����߳̽�����
	if (wait) {
		pJaThreadKit->wait(&Privated->Calibrator);
		pJaThreadKit->terminate(&Privated->Calibrator, JA_True);
	}
}


/**
 * ������������̡߳�
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
			/// һֱ����֪�� current_step �� target_step һ��ʱͣ�¡�
			if(Privated->current_step  < Privated->target_step) {
				/// �����Ҫǰ������Ŀ�ꡣ
				stepper_forward(Public);
				++Privated->current_step; ///< ���� current_step ��ֵҪʱ������״̬����ͬ����
			} else {
				/// �����Ҫ���˵���Ŀ�ꡣ
				stepper_backward(Public);
				--Privated->current_step; ///< ���� current_step ��ֵҪʱ������״̬����ͬ����
			}
			
			tilt_TargetStep = getFocusStep(Privated->current_step);
			while(tilt_TargetStep != Privated_syn->current_step
				&& tilt_TargetStep <= HIGH_FOCUS_STEP)//ͬ��ʱ��ͷҲ����ͻ�������ⶥס��ǲ���
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
			/// һֱ����֪�� current_step �� target_step һ��ʱͣ�¡�
			if(Privated->current_step  < Privated->target_step) {
				/// �����Ҫǰ������Ŀ�ꡣ
				stepper_forward(Public);
				++Privated->current_step; ///< ���� current_step ��ֵҪʱ������״̬����ͬ����
			} else {
				/// �����Ҫ���˵���Ŀ�ꡣ
				stepper_backward(Public);
				--Privated->current_step; ///< ���� current_step ��ֵҪʱ������״̬����ͬ����
			}

			ja_mSleep(speed);
		}
	}

	//���͵����ѹ
	Privated->StopMotor(Public);

	Thread->terminate(&tmpThread, JA_True);
	/// �˳������ٽ�����
	//Privated->Critical->unlock(Privated->Critical);
	JaLog("StepperMotor: Distance=%d Step=%d Degree=%.3f.",
			Privated->full_distance,
			Privated->current_step,
			Privated->current_dregree);

	pthread_exit(0);
}

/**
 * ��ȡ���������Ϣ��
 *
 * @param[out]		Info_r		�����Ϣ���ݽṹ���á�
 *
 * @return	��ȡ�ɹ����� 0���ṹ�� @ref Info_r ���ݽṹ�����֣�ʧ�ܷ��� -1��
 *
 */
static JA_Int
StepperMotor_getInfo(THIS, TJA_StepperMotorInfo *Info_r)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	JA_Float percentage = 0.0;

	/// ��ȫ����û��ȷ��֮ǰ��ȡ��Ϣʧ�ܡ�
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
 * �����������ת����
 */
static JA_Int
StepperMotor_move(THIS, JA_Boolean forward, JA_Boolean wait,
	JA_Int speed, TJA_StepperMotor *syn_Motor, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	if (forward) {
		/// ������ת����
		return Public->moveTo(Public, Privated->RefStep.upper, wait
				, speed, syn_Motor, objType);
	}

	return Public->moveTo(Public, Privated->RefStep.lower, wait
				, speed, syn_Motor, objType);
}

/**
 * �����������ת�����̶��Ƕȡ�
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

	/// ������̨�߳̿��Ƶ��������
	Privated->Mover = pJaThreadKit->create(mover, argc, Public, speed, syn_Motor, objType);
	if (wait) {
		/// �ȴ�����������
		pJaThreadKit->wait(&Privated->Mover);
		pJaThreadKit->Thread_free(&Privated->Mover);
	}
	return 0;
}

/**
 * �����������ת��ԭ��λ�á�\n
 *
 * @param[in]		wait		����������ʶ��Ϊ Ture ʱ�˽ӿڻ�ȵ�ת��������Ϻ�ŷ��ء�
 * @param[in]		speed		����ٶ�
 * @return	�����ɹ����� 0��ʧ�ܷ��� -1��
 */
static JA_Int
StepperMotor_home(THIS, JA_Boolean wait, JA_Int speed, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	return Public->moveTo(Public, Privated->RefStep.home, wait, speed, JA_Nil, objType);
}

/**
 * ��ֹ�������ת����
 */
static JA_Int
StepperMotor_stop(THIS)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// �����������е��̡߳�
	if(JA_Nil != Privated->Mover) 
	{
		pJaThreadKit->terminate(&Privated->Mover, JA_True);
	}

	return 0;
}

/**
 * �жϲ�������Ƿ�ת����
 */
static JA_Boolean
StepperMotor_terminated(THIS)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	//�߳���������
	if(JA_Nil != Privated->Mover) 
	{
		return Privated->Mover->terminated(Privated->Mover);
	}
	
	return JA_True;
}

/**
 * �����������ת��0λ�á�\n
 *
 * @param[in]		wait		����������ʶ��Ϊ Ture ʱ�˽ӿڻ�ȵ�ת��������Ϻ�ŷ��ء�
 * @param[in]		speed		����ٶ�
 *
 * @return	�����ɹ����� 0��ʧ�ܷ��� -1��
 */
static JA_Int
StepperMotor_init(THIS, JA_Boolean wait, JA_Int speed
	, JA_Int max_step, TJA_StepperMotor *syn_Motor, JA_Int max_SynStep, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);
	Privated->current_step = max_step;	//ǿ�����õ�ǰλ��Ϊ���
	
	TJA_PrivatedStepperMotor *const syn_Privated = PRIVATED(syn_Motor);
	syn_Privated->current_step = max_SynStep;
	syn_Privated->target_step = 0;
	
	return Public->moveTo(Public, 0, wait, speed, syn_Motor, objType);	//Ȼ������Сλ��0��
}

/**
 * �����������ת��0λ�á�\n
 *
 * @param[in]		wait		����������ʶ��Ϊ Ture ʱ�˽ӿڻ�ȵ�ת��������Ϻ�ŷ��ء�
 * @param[in]		speed		����ٶ�
 *
 * @return	�����ɹ����� 0��ʧ�ܷ��� -1��
 */
static JA_Int
StepperMotor_initTarget(THIS, JA_Boolean wait, JA_Int speed
	, JA_Int target_step, TJA_StepperMotor *syn_Motor, JA_Int target_SynStep, enObjType objType)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);	
	Privated->target_step = target_step;	
	
	TJA_PrivatedStepperMotor *const syn_Privated = PRIVATED(syn_Motor);
	syn_Privated->target_step = target_SynStep;

	return Public->moveTo(Public, target_step, wait, speed, syn_Motor, objType);	//Ȼ������Сλ��0��
}

/**
 * ��ȡ��ǰ������ڲ�����
 */
static JA_Int
StepperMotor_getStep(THIS, JA_Int *val)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	*val = Privated->current_step;

	return 0;
}

/**
 * ����λ���صĻص���
 */
static JA_Boolean
StepperMotor_onUpperLimit(THIS, JA_PVoid user_ctx)
{
	TJA_PrivatedStepperMotor *const Privated = PRIVATED(Public);

	/// ����λ���صĲ������ͨ����ǰ�������Ƶ����λ��
	return Privated->current_step >= Privated->full_distance ? JA_True : JA_False;
}


/**
 * THIS ָ�붨������\n
 * ��������Ϊģ�� API �ӿ�ʵ�֡�
 */
#undef THIS

/**
 * ���� StepperMotor ģ������\n
 * ���˫��λ���صĲ��������\n
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

	/// ��ʼ�������
	/// ���о�����ڴ�ռ����˽�о���ĸ�λ��
	/// ��Ч��ֹģ�鹫�о�����ⲿ�������ͷš�
	Privated	= calloc(1, sizeof(TJA_PrivatedStepperMotor) + sizeof(TJA_StepperMotor));
	Public		= (TJA_StepperMotor *)(Privated + 1);

	/// ��ʼ��˽�г�Ա��
	Privated->MemAllocator	= MemAllocator;
	snprintf(Privated->magic, sizeof(Privated->magic), "%s", "OBJ");
	/// ��ʼ������˽�г�Ա��
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

	/// ��ʼ�����г�Ա��
	/// ��ʼ���������г�Ա��
	Public->setRefStep			= StepperMotor_setRefStep;//���ò���
	Public->calibrate			= StepperMotor_calibrate;
	Public->getInfo				= NULL;//StepperMotor_getInfo;
	Public->getStep				= StepperMotor_getStep;
	
	Public->move				= StepperMotor_move;//�ڲ�Ҳ�ǵ���moveTo
	Public->moveTo				= StepperMotor_moveTo;
	Public->home				= StepperMotor_home;
	Public->stop				= StepperMotor_stop;
	Public->Init				= StepperMotor_init;
	Public->InitTarget			= StepperMotor_initTarget;
	Public->Terminated			= StepperMotor_terminated;
	Public->setCurStep			= SetMotorCurStep;
	/// ������ʼ��������

	/// ���ع��о����
	return Public;
}


/**
 * ���� StepperMotor ģ������\n
 * ��Ե���λ���صĲ��������\n
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

		/// ��ʼ�������
		Public = JaStepperMotor_Create(MemAllocator, tick_cycle,
				onStep, onLimitLower, StepperMotor_onUpperLimit, 
				onCalibrated, user_ctx, StopMotor, MoveCallBack);
		if (JA_Nil != Public) {
			/// �����ڲ�������
			Privated = PRIVATED(Public);

			/// ����ȫ���̡�
			Privated->fixed_distance = full_distance;
		}
	} else {
		JaLog("StepperMotor: Setup Distance(%d) Error.", full_distance);
	}

	return Public;
}

/**
 * ���� StepperMotor ģ������
 */
JA_Void
JaStepperMotor_Free(TJA_StepperMotor **StepperMotor_r)
{
	TJA_StepperMotor *Public = JA_Nil;
	TJA_PrivatedStepperMotor *Privated = JA_Nil;
	TJA_MemAllocator *MemAllocator = JA_Nil;

	/// ��ȡ˽�о����
	Public = StepperMotor_r[0];
	Privated = PRIVATED(Public);
	StepperMotor_r[0] = JA_Nil; ///< ����ǰ����վ����ֹ�ϲ��û����ͷŹ�����������á�
	MemAllocator = Privated->MemAllocator;

	/// ֹͣ���������
	Public->stop(Public);

	/// �����������е��̡߳�
	if (JA_Nil != Privated->Mover) {
		pJaThreadKit->terminate(&Privated->Mover, JA_True);
	}
	if (JA_Nil != Privated->Calibrator) {
		pJaThreadKit->terminate(&Privated->Calibrator, JA_True);
	}

	/// �ͷ�ģ������
	MemAllocator->free(MemAllocator, Privated);
	Privated = JA_Nil;
}





