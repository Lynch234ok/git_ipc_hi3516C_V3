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

//1代表250us，值越大速度越慢
#define FOCUS_SPEED_HIGH		(16)
#define FOCUS_SPEED_MID			(24)
#define FOCUS_SPEED_LOW			(60)
#define FOCUS_SPEED_MANUAL		(100)
#define ZOOM_SPEED				(24)
#define ZOOM_SPEED_FAST			(8)

/**
 * PanTilt 单体模块句柄。
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
	 * 释放单体模块。
	 */
	JA_Void
	(*free)();

	/**
	 * 水平转动。\n
	 *
	 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
	 * @param[in]		left_right	左右转动标识，向作转动时为 True 向右摆动时为 False。
	 * @param[in]		sync		同步操作标识，为 True 时会等待此动作执行完成或者调用 stop 中断后才返回。
	 * @param[in]		speed		电机速度
	 *
	 * @return	执行成功返回 0，执行失败返回 -1。
	 */
	JA_Int
	(*pan)(JA_Boolean force, JA_Boolean left_right, JA_Boolean sync, JA_Int speed);

	/**
	 * 水平转动至指定角度。\n
	 *
	 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
	 * @param[in]		degree		水平转动到的目标角度，角度必须为合理范围内。
	 * @param[in]		sync		同步操作标识，为 True 时会等待此动作执行完成或者调用 stop 中断后才返回。
	 *
	 * @return	执行成功返回 0，执行失败返回 -1。
	 */
	JA_Int
	(*panTo)(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed);

	/**
	 * 水平遍历转动。\n
	 * 调用此接口之后云台会一直保持水平遍历转动，直至被其他动作打断或者 stop 中断。
	 *
	 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
	 *
	 */
	JA_Int
	(*autoPan)(JA_Boolean force);

	/**
	 * 停止Pan执行动作。\n
	 *
	 * @return	执行成功返回 0，执行失败返回 -1。
	 */
	JA_Int
	(*stopPan)(void);

	/**
	 * 垂直转动。\n
	 *
	 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
	 * @param[in]		left_right	左右转动标识，向作转动时为 True 向右摆动时为 False。
	 * @param[in]		sync		同步操作标识，为 True 时会等待此动作执行完成或者调用 stop 中断后才返回。
	 *
	 * @param[in]		speed		电机速度
	 * @return	执行成功返回 0，执行失败返回 -1。
	 */
	JA_Int
	(*tilt)(JA_Boolean force, JA_Boolean left_right, JA_Boolean sync, JA_Int speed);

	/**
	 * 垂直转动至指定角度。\n
	 *
	 * @param[in]		force		强制执行，当调用此接口的时候会强行终止上一个动作执行此次动作。
	 * @param[in]		degree		水平转动到的目标角度，角度必须为合理范围内。
	 * @param[in]		sync		同步操作标识，为 True 时会等待此动作执行完成或者调用 stop 中断后才返回。
	 *
	 * @return	执行成功返回 0，执行失败返回 -1。
	 */
	JA_Int
	(*tiltTo)(JA_Boolean force, JA_Int step, JA_Boolean sync, JA_Int speed);

	/**
	 * 停止Tilt执行动作。\n
	 *
	 * @return	执行成功返回 0，执行失败返回 -1。
	 */
	JA_Int
	(*stopTilt)(void);

	/**
	 * 云台转动到原始的位置。
	 *
	 * @param[in]		force
	 * @param[in]		sync
	 *
	 * @return	执行成功返回 0，执行失败返回 -1。
	 */
	JA_Int
	(*home)(JA_Boolean force, JA_Boolean sync, JA_Int speed, enObjType objType);

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
	JA_Int
	(*goTo)(JA_Boolean force, JA_Int pan_step, JA_Int titl_step, JA_Boolean sync, JA_Int speed);

	/**
	 * 获取当前云台所在角度。
	 *
	 * @param[out]		pan_degree_r
	 * @param[out]		titl_degree_r
	 *
	 * @return	执行成功返回 0，执行失败返回 -1。
	 */
	JA_Int
	(*getStep)(JA_Int *pan_step_r, JA_Int *titl_step_r);

	/**
	 * 停止上一次执行动作。\n
	 *
	 * @return	执行成功返回 0，执行失败返回 -1。
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
 * 返回单体模块句柄。\n
 * 当第一次调用此接口时，接口内部会初始化创建单体模块，\n
 * 在调用 TJA_PanTilt::free() 接口后单体模块句柄内部会释放资源，\n
 * 并在下一次调用此接口时重新创建。
 */
extern TJA_PanTilt *
PanTilt(void);

extern TJA_PanTilt *_PstepMotor;

#ifdef __cplusplus 
	}
#endif

#endif /* PAN_TILT_H_ */
