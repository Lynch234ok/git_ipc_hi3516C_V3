/*
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install,
 *  Copy or use the software.
 *
 *  Copyright (C) 2012, JUAN, Co, All Rights Reserved.
 *
 *  Created on: Sep 1, 2015
 *	Author: Frank Law
 *
 */




#ifndef PAN_TILT_H_
#define PAN_TILT_H_

#include <object.h>
#include <jaThread.h>

#ifdef __cplusplus 
	extern "C" { 
#endif

#define YU_TONG_28_12

#ifdef YU_TONG_28_8
#define FocusStep		(3132)
#define ZoomStep		(2088)
#else  //YU_TONG_28_12
#define FocusStep		(2342)
#define ZoomStep		(2285)
#endif

//1����250us��ֵԽ���ٶ�Խ��
#define FOCUS_SPEED_HIGH		(16)
#define FOCUS_SPEED_MID			(24)
#define FOCUS_SPEED_LOW			(60)
#define FOCUS_SPEED_MANUAL		(100)
#define ZOOM_SPEED				(24)
#define ZOOM_SPEED_FAST			(8)

/**
 * PanTilt ����ģ������
 */
typedef struct jaPanTilt {

	JA_Boolean	manual;
	JA_Boolean  isInit;
	JA_Boolean	goTarget;
	JA_Boolean	goTargetPulse;
	JA_Boolean  TestCurve;
	JA_Boolean	EnableFocus;
	JA_Boolean	StopFocus;
	JA_Boolean  isReset;
	JA_Boolean  CheckTarget;
	JA_Boolean  FocusGoTo;
	JA_Int	CurSeg;
	JA_Int  PanStepBackup;
	JA_Int  TiltStepBackup;
	JA_Int  stopPlace;
	/**
	 * �ͷŵ���ģ�顣
	 */
	JA_Void
	(*free)();

	/**
	 * ˮƽת����\n
	 *
	 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
	 * @param[in]		left_right	����ת����ʶ������ת��ʱΪ True ���Ұڶ�ʱΪ False��
	 * @param[in]		sync		ͬ��������ʶ��Ϊ True ʱ��ȴ��˶���ִ����ɻ��ߵ��� stop �жϺ�ŷ��ء�
	 * @param[in]		speed		����ٶ�
	 *
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*pan)(JA_Boolean force, JA_Boolean left_right, JA_Boolean sync, JA_Int speed);

	/**
	 * ˮƽת����ָ���Ƕȡ�\n
	 *
	 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
	 * @param[in]		degree		ˮƽת������Ŀ��Ƕȣ��Ƕȱ���Ϊ����Χ�ڡ�
	 * @param[in]		sync		ͬ��������ʶ��Ϊ True ʱ��ȴ��˶���ִ����ɻ��ߵ��� stop �жϺ�ŷ��ء�
	 *
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*panTo)(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed);

	/**
	 * ˮƽ����ת����\n
	 * ���ô˽ӿ�֮����̨��һֱ����ˮƽ����ת����ֱ��������������ϻ��� stop �жϡ�
	 *
	 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
	 *
	 */
	JA_Int
	(*autoPan)(JA_Boolean force);

	/**
	 * ֹͣPanִ�ж�����\n
	 *
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*stopPan)(void);

	/**
	 * ��ֱת����\n
	 *
	 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
	 * @param[in]		left_right	����ת����ʶ������ת��ʱΪ True ���Ұڶ�ʱΪ False��
	 * @param[in]		sync		ͬ��������ʶ��Ϊ True ʱ��ȴ��˶���ִ����ɻ��ߵ��� stop �жϺ�ŷ��ء�
	 *
	 * @param[in]		speed		����ٶ�
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*tilt)(JA_Boolean force, JA_Boolean left_right, JA_Boolean sync, JA_Int speed);

	/**
	 * ��ֱת����ָ���Ƕȡ�\n
	 *
	 * @param[in]		force		ǿ��ִ�У������ô˽ӿڵ�ʱ���ǿ����ֹ��һ������ִ�д˴ζ�����
	 * @param[in]		degree		ˮƽת������Ŀ��Ƕȣ��Ƕȱ���Ϊ����Χ�ڡ�
	 * @param[in]		sync		ͬ��������ʶ��Ϊ True ʱ��ȴ��˶���ִ����ɻ��ߵ��� stop �жϺ�ŷ��ء�
	 *
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*tiltTo)(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed);

	/**
	 * ֹͣTiltִ�ж�����\n
	 *
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*stopTilt)(void);

	/**
	 * ��̨ת����ԭʼ��λ�á�
	 *
	 * @param[in]		force
	 * @param[in]		sync
	 *
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*home)(JA_Boolean force, JA_Boolean sync, JA_Int speed, enObjType objType);

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
	JA_Int
	(*goTo)(JA_Boolean force, JA_Int pan_step, JA_Int titl_step, JA_Boolean sync, JA_Int speed);

	/**
	 * ��ȡ��ǰ��̨���ڽǶȡ�
	 *
	 * @param[out]		pan_degree_r
	 * @param[out]		titl_degree_r
	 *
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*getStep)(JA_Int *pan_step_r, JA_Int *titl_step_r);

	/**
	 * ֹͣ��һ��ִ�ж�����\n
	 *
	 * @return	ִ�гɹ����� 0��ִ��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*stop)(void);

	JA_Int
	(*Init)(JA_Boolean force, JA_Boolean sync, JA_Int speed);

	JA_Int
	(*InitTarget)(JA_Boolean force, JA_Boolean sync, JA_Int speed);

	JA_Boolean
	(*Terminated)(void);

	JA_Int
	(*setCurStep)(JA_Int *pan_degree_r, JA_Int *titl_degree_r);

	JA_Int
	(*setTargetStep)(JA_Int *pan_degree_r, JA_Int *titl_degree_r);

	JA_Int
	(*testCamera)(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed);
} TJA_PanTilt;

/**
 * ���ص���ģ������\n
 * ����һ�ε��ô˽ӿ�ʱ���ӿ��ڲ����ʼ����������ģ�飬\n
 * �ڵ��� TJA_PanTilt::free() �ӿں���ģ�����ڲ����ͷ���Դ��\n
 * ������һ�ε��ô˽ӿ�ʱ���´�����
 */
extern TJA_PanTilt *
PanTilt(void);

extern TJA_PanTilt *_PstepMotor;

#ifdef __cplusplus 
	}
#endif

#endif /* PAN_TILT_H_ */
