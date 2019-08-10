
#include <pan_tilt.h> //<! ����ģ�� PanTilt �����ļ���
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stepper_motor.h>
#include "bsp_io.h"
#include <stepTable.h>

#define TMP_DIR "/tmp/PanTilt"

/**
 * PanTilt ģ��˽�о�����������ģ���ڲ���˽�г�Ա��\n
 * �ڴ��� PanTilt ģ�鴴��ʱͳһ���䡣\n
 * λ���ھ�����ݽṹ @ref TJA_PanTilt ��λ��\n
 * ������Ч���� @ref TJA_PanTilt �ڴ�ռ䱻�����ͷš�\n
 * ����ͼ��\n
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

	/// ˮƽ���Ƶ����
	TJA_StepperMotor *PanMotor;

	/// ��ֱ���Ƶ�������
	TJA_StepperMotor *TiltMotor;

	/// Auto Pan ��̨�̡߳�
	TJA_Thread *AutoPan;

} TJA_PrivatedPanTilt;

TJA_PanTilt *_PstepMotor = NULL;

/**
 * �����ڲ�����ģ������
 */
static struct {

	/// ˽�о����
	TJA_PrivatedPanTilt Privated;

	/// ���о����
	TJA_PanTilt Public;

} _Singleton, *_PSingleton = JA_Nil;

/**
 *  ����ģ��˽�о����
 */
static TJA_PrivatedPanTilt *const
_Privated = &_Singleton.Privated;

/**
 * ����ģ�鹫�о����
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
 * ˮƽת����\n
 *
 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
 * @param[in]		up_down		����ת����ʶ������ת��ʱΪ True ����ת��ʱΪ False��
 * @param[in]		sync		ͬ��������ʶ��Ϊ True ʱ��ȴ��˶���ִ����ɻ��ߵ��� stop �жϺ�ŷ��ء�
 *
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
 */
static JA_Int PanTilt_pan(JA_Boolean force, JA_Boolean up_down, JA_Boolean sync, JA_Int speed)
{
	//ǿ�ƽ���
	if(force)
	{
		_Public->stop();
	}

	_Public->EnableFocus = JA_False;

	// ����ת��
	return _Privated->PanMotor->move(_Privated->PanMotor, up_down
			, sync, speed, _Privated->TiltMotor, OBJ_PAN);
}

/**
 * ˮƽת����ָ���Ƕȡ�\n
 *
 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
 * @param[in]		degree		ˮƽת������Ŀ��Ƕȣ��Ƕȱ���Ϊ����Χ�ڡ�
 * @param[in]		sync		ͬ��������ʶ��Ϊ True ʱ��ȴ��˶���ִ����ɻ��ߵ��� stop �жϺ�ŷ��ء�
 * @param[in]		speed
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
 */
static JA_Int PanTilt_panTo(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed)
{
	//ǿ�ƽ���
	if(force)
	{
		_Public->stop();
	}
	
	//ת�����̶��Ƕ�degree
	return _Privated->PanMotor->moveTo(_Privated->PanMotor, step
			, sync, speed, _Privated->TiltMotor, OBJ_PAN);
}

/**
 * ˮƽ�Զ���̨�̡߳�
 *
 */
static JA_Void auto_pan(TJA_Thread * const Thread, JA_Integer argc, JA_PVoid argv[], JA_Int speed)
{
	TJA_StepperMotor *const PanMotor = _Privated->PanMotor;

	while (!Thread->terminated(Thread)) {

		/// ˮƽת������λ��ͷ��
		PanMotor->move(PanMotor, JA_False, JA_True, speed, JA_Nil, OBJ_PAN);
		Thread->suspend(Thread, 0, 100, 0);

		/// ˮƽת������λ��ͷ��
		PanMotor->move(PanMotor, JA_True, JA_True, speed, JA_Nil, OBJ_PAN);
		Thread->suspend(Thread, 0, 100, 0);
	}
}

/**
 * ˮƽ����ת����\n
 * ���ô˽ӿ�֮����̨��һֱ����ˮƽ����ת����ֱ��������������ϻ��� stop �жϡ�
 *
 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
 *
 */
static JA_Int PanTilt_autoPan(JA_Boolean force)
{
	/// ǿ��ֹͣ��
	if(force) {
		_Public->stop();
	}
	return 0;
}


/**
 * ��ֱת����\n
 *
 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
 * @param[in]		left_right	����ת����ʶ������ת��ʱΪ True ���Ұڶ�ʱΪ False��
 * @param[in]		sync		ͬ��������ʶ��Ϊ True ʱ��ȴ��˶���ִ����ɻ��ߵ��� stop �жϺ�ŷ��ء�
 *
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
 */
static JA_Int PanTilt_tilt(JA_Boolean force, JA_Boolean up_down, JA_Boolean sync, JA_Int speed)
{
	//ǿ�ƽ���
	if(force)
	{
		_Public->stop();
	}

	// ����ת��
	return _Privated->TiltMotor->move(_Privated->TiltMotor, !up_down
				, sync, speed, JA_Nil, OBJ_TILT);
}

/**
 * ��ֱת����ָ���Ƕȡ�\n
 *
 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
 * @param[in]		degree		ˮƽת������Ŀ��Ƕȣ��Ƕȱ���Ϊ����Χ�ڡ�
 * @param[in]		sync			ͬ��������ʶ��Ϊ True ʱ��ȴ��˶���ִ����ɻ��ߵ��� stop �жϺ�ŷ��ء�
 * @param[in]		speed
 *
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
 */
static JA_Int PanTilt_tiltTo(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed)
{
	//ǿ�ƽ���
	if(force)
	{
		_Public->stop();
	}

	//ת�����̶��Ƕ�degree
	return _Privated->TiltMotor->moveTo(_Privated->TiltMotor, step
			, sync, speed, JA_Nil, OBJ_TILT);
}

/**
 * ��̨ת����ԭʼ��λ�á�
 *
 * @param[in]		force
 * @param[in]		sync
 *
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
 */
static JA_Int
PanTilt_home(JA_Boolean force, JA_Boolean sync, JA_Int speed, enObjType objType)
{
	//ǿ�ƽ���
	if(force)
	{
		_Public->stop();
	}

	//ת����home
	return (_Privated->PanMotor->home(_Privated->PanMotor, sync, speed, objType) 
			|| _Privated->TiltMotor->home(_Privated->TiltMotor, sync, speed, objType));
}

/**
 * ��̨ת����ָ����λ�á�
 *
 * @param[in]		force
 * @param[in]		pan_degree
 * @param[in]		titl_degree
 * @param[in]		sync
 *
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
 */
static JA_Int
PanTilt_goTo(JA_Boolean force, JA_Int pan_step, JA_Int titl_step, JA_Boolean sync, JA_Int speed)
{
	//ǿ�ƽ���
	if(force)
	{
		_Public->stop();
	}

	return (_Public->panTo(force, pan_step, sync, speed) 
		|| _Public->tiltTo(force, titl_step, sync, speed));
}

/**
 * ��ȡ��ǰ��̨���ڽǶȡ�
 *
 * @param[out]		pan_degree_r
 * @param[out]		titl_degree_r
 *
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
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
 * ֹͣ��һ��ִ�ж�����\n
 *
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
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
 * ��̨ת����0��λ�á�
 *
 * @param[in]		force
 * @param[in]		sync
 * @param[in]		speed
 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
 */
static JA_Int
PanTilt_Init(JA_Boolean force, JA_Boolean sync, JA_Int speed)
{
	//ǿ�ƽ���
	if(force)
	{
		_Public->stop();
	}

	_Public->EnableFocus = false;
	_Public->isInit	= true;
	//ת����home
	return ( _Privated->PanMotor->Init(_Privated->PanMotor, sync, speed
			, ZoomStep+200, _Privated->TiltMotor, FocusStep + 200, OBJ_PAN) );
}

static JA_Int
PanTilt_InitTarget(JA_Boolean force, JA_Boolean sync, JA_Int speed)
{
	_Public->EnableFocus = false;
	//ת����home
	return ( _Privated->PanMotor->InitTarget(_Privated->PanMotor, sync, speed
			, _Public->PanStepBackup, _Privated->TiltMotor, _Public->TiltStepBackup, OBJ_PAN) );
}

static JA_Boolean
PanTilt_Terminated(void)
{
	return _Privated->TiltMotor->Terminated(_Privated->TiltMotor);
}

/**
 * ��ʼ����������
 */
static inline JA_Void
SINGLETON_CREATE()
{
	/// ��λ�����������ݡ�
	memset(&_Singleton, 0, sizeof(_Singleton));
	/// ���þ��
	_PSingleton = &_Singleton;
}

/**
 * �ͷŵ�������
 */
static inline JA_Void
SINGLETON_FREE()
{
	/// ��վ��
	_PSingleton = JA_Nil;
	/// ��λ�����������ݡ�
	memset(&_Singleton, 0, sizeof(_Singleton));
}

/**
 * �ͷŵ���ģ�顣
 */
JA_Void
PanTilt_free()
{
	/// TODO ����ģ������Դ
	if(JA_Nil != _Privated->PanMotor)
	{
		JaStepperMotor_Free(&(_Privated->PanMotor));
	}

	if(JA_Nil != _Privated->TiltMotor)
	{
		JaStepperMotor_Free(&(_Privated->TiltMotor));
	}

	/// �ͷŵ�������
	//SINGLETON_FREE();
}

/**
 * foucus�������������
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
		//*����
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
	/// ������λ���ص�״̬������
	//return BSP_IO_TILT_LIMIT();
}

static JA_Void
tilt_onCalibrated(TJA_StepperMotor *const Motor, JA_PVoid user_ctx, JA_Int speed)
{
	/// TODO
}

/**
 * zoom�������������
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
		//*����
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
	/// ������λ���ص�״̬������
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
	//ǿ�ƽ���
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
		
		//ת�����̶�����step
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
 * ��������
 *
 */
TJA_PanTilt *
PanTilt(void)
{
	TJA_PrivatedPanTilt *const Privated = &_Singleton.Privated;
	TJA_PanTilt *const Public = &_Singleton.Public;

	if (!_PSingleton) {
		/// ��ʼ����������
		SINGLETON_CREATE();

		/// ��ʼ��˽�г�Ա��
		/// ��ʼ��ˮƽ�����
		Privated->PanMotor = JaStepperMotor_Create(JA_Nil,
					4, ///< 4 �ġ�
					pan_onStep, pan_onLimit, pan_onLimit
					, JA_Nil, JA_Nil, pan_Stop, JA_Nil);

		/// ��ʼ����ֱ�����
		Privated->TiltMotor = JaStepperMotor_Create(JA_Nil,
					4, ///< 4 �ġ�
					tilt_onStep, tilt_onLimit, tilt_onLimit, tilt_onCalibrated
					, JA_Nil, tilt_Stop, JA_Nil);

		/// ��ʼ�����г�Ա��
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
		Public->pan 				= PanTilt_pan;		//zoom�������ת��
		Public->panTo				= PanTilt_panTo;	//zoom���ת����ָ������
		Public->autoPan				= NULL;
		Public->stopPan				= Pan_stop;
		Public->tilt				= PanTilt_tilt;		//foucus�������ת��
		Public->tiltTo				= PanTilt_tiltTo;	//foucus���ת����ָ������
		Public->stopTilt			= Tilt_stop;
		
		Public->home				= PanTilt_home;		//���������λ
		Public->goTo				= PanTilt_goTo;		//������ֱ�ת����ָ������
		Public->getStep				= PanTilt_getStep;	//��ȡ������Ĳ���
		Public->stop				= PanTilt_stop;		//ֹͣ�����
		Public->Init				= PanTilt_Init; 	//�����������ʼlowerλ��
		Public->InitTarget			= PanTilt_InitTarget;	//�����������ʼ�ϴμ�¼��λ��
		Public->Terminated			= PanTilt_Terminated;	//�ȴ�focus���ֹͣ
		Public->setCurStep 			= PanTilt_setStep;
		Public->setTargetStep		= PanTilt_setTargetStep;
		Public->testCamera			= testCamera;
		/// ������ʼ��������
		Privated->PanMotor->setRefStep(Privated->PanMotor, LOW_ZOOM_STEP, HIGH_ZOOM_STEP, LOW_ZOOM_STEP, JA_False); ///< �첽У׼��
		Privated->TiltMotor->setRefStep(Privated->TiltMotor, LOW_FOCUS_STEP, HIGH_FOCUS_STEP, LOW_FOCUS_STEP, JA_False); ///< �첽У׼��

	}

	/// ���ع��о����
	return Public;
}

