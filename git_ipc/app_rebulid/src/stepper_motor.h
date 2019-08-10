/*
 * JUAN BSP 模块集。\n
 * 步进电机控制模块。\n
 * 通过对步进电机相关引脚控制实现控制步进点击转动的目的。\n
 *
 */


//#include <JaEmbedded/mem_allocator.h>


#ifndef BSP_STEPPER_MOTOR_H_
#define BSP_STEPPER_MOTOR_H_

//#include <jaThread.h>
#include <object.h>


#ifdef __cplusplus 
	extern "C" { 
#endif

#define TIMER_DEV_NAME 		"/dev/hi35xx_motor"
#define IOC_MAGIC			5

/**
 *  步进电机“步（Step）“概念：\n
 *    步进电机在实际运行时候以“步（Step)”作为单位，\n
 *    每当电机向前走一个单位，即走一步，走的步数称为步程（Step Distance），\n
 *    电机从最低位走到最高位（由限位开关决定）全过程所走的步数为全步程（Full Step Distance）。\n
 *    步程需要由用户初始化电机的时候指定，或者用户调用校准方法（Calibrate）时计算出来。\n
 *
 *  步进电机”角度（Degree）”概念：\n
 *    步进电机在实际使用的时候可能并不关心走的步数，\n
 *    而关心步进电机走到那个角度，因此引入“角度（Degree）”概念。\n
 *    电机的角度是根据电机内部运算的步程，加上用户设定的参考角度（@ref TJA_StepperMotor::setRefDegree）内部换算而成。\n
 *    如用户设定参考角度是 -180 -- 180，则步 0 所对应的角度为 -180 度。\n
 *    参考角度可以根据实际电机应用场景去决定设置为什么值。\n
 *
 *
 *
 *  Step Distance Begin                          Step Distance End
 *  (must be 0)                                  (Indicated or Calibrate)
 *   |                                            |
 *  \|/                                          \|/
 *   |                                            |
 *   |--------------------------------------------|
 *   |                                            |
 *  /|\                                          /|\
 *   |                                            |
 *  RefDegree.lower                             RefDegree.upper
 *
 */

/**
 * StepperMotor 信息数据结构。
 */
typedef struct jaStepperMotorInfo {

	/**
	 * 步程相关数据结构。
	 */
	struct {

		JA_Int lower, upper, home; ///< 电机设定的低位，高位，原点步程。
		JA_Int current; ///< 当前电机所在步程。

	} Distance;

	/**
	 * 角度相关数据结构。
	 */
	struct {

		JA_Float lower, upper, home; ///< 电机设定的低位，高位，原点角度。
		JA_Float current; ///< 当前电机所在角度。

	} Degree;

} TJA_StepperMotorInfo;

typedef struct jaStepperMotor TJA_StepperMotor;

/**
 * StepperMotor 模块句柄。
 */
struct jaStepperMotor {
#define THIS struct jaStepperMotor *const

	/**
	 * 模块基础接口。
	 */
	//TJA_Object Object;	

	/**
	 * 设置参考步数。\n
	 * 步进参考步数需要由用户根据实际情况进行设置，\n
	 * 一般对于固定方案只需要设置一次。\n
	 * 调用此接口时会内部会调用 @ref TJA_StepperMotor::calibrate 进行校准。\n
	 * 如果用户不调用此接口去设置参考角度模块内部则默认设定 lower:0 upper:+3000 home:1500 步。\n
	 * 但用户在电机使用前调用 @ref TJA_StepperMotor::calibrate 去校准电机，否则无法使用。\n
	 * @ref TJA_StepperMotor::moveTo 接口会根据设定的步数判断传入步数是否合法。\n
	 *
	 *
	 * @param[in]		lower		低位限制参考角度，不设置默认为 0。
	 * @param[in]		upper		高位限制参考角度，不设置默认为 3000。
	 * @param[in]		home		正位所在角度，不设置默认为 1500。
	 * @param[in]		wait		内部调用 TJA_StepperMotor::calibrate 是否阻塞标识，但不管是否拥塞，电机必须在调用 TJA_StepperMotor::calibrate 完毕后才能使用其他功能。
	 *
	 *
	 * @return	设置成功返回 0，失败返回 -1。
	 */
	JA_Int
	(*setRefStep)(THIS, JA_Int lower, JA_Int upper, JA_Int home, JA_Boolean wait);

	/**
	 * 步进电机校准。\n
	 * 每次调用完 @ref TJA_StepperMotor::setRefDegree 均需要调用此接口进行角度校准。\n
	 *
	 */
	JA_Void
	(*calibrate)(THIS, JA_Boolean wait, JA_Int speed);

	/**
	 * 获取步进电机信息。
	 *
	 * @param[out]		Info_r		电机信息数据结构引用。
	 *
	 * @return	获取成功返回 0，结构在 @ref Info_r 数据结构中体现，失败返回 -1。
	 *
	 */
	JA_Int
	(*getInfo)(THIS, TJA_StepperMotorInfo *Info_r);

	/**
	 * 驱动步进电机转动。\n
	 * 调用此接口之后，\n
	 * 若操作过程中不调用 TJA_StepperMotor::stop 方法，\n
	 * 电机会一直转动置该方向上的极限位置才停止。\n
	 *
	 * @param[in]		forward		转动方向，Ture 为正向转动，False 为逆向转动。
	 * @param[in]		wait		阻塞操作标识，为 Ture 时此接口会等到转动操作完毕后才返回。
	 * @param[in]		speed		电机速度
	 * @param[in]		objType		对象类型
	 *
	 * @return	操作成功返回 0，失败返回 -1。
	 */
	JA_Int
	(*move)(THIS, JA_Boolean forward, JA_Boolean wait, JA_Int speed, THIS syn_Motor, enObjType objType);

	/**
	 * 驱动步进电机转动到固定角度。\n
	 * 调用此接口之后，\n
	 * 若操作过程中不调用 TJA_StepperMotor::stop 方法，\n
	 * 电机会一直转动置该角度上才停止。\n
	 *
	 * @param[in]		degree		目标角度。
	 * @param[in]		wait		阻塞操作标识，为 Ture 时此接口会等到转动操作完毕后才返回。
	 * @param[in]		speed		电机速度
	 * @param[in]		objType		对象类型
	 *
	 * @return	操作成功返回 0，失败返回 -1。
	 */
	JA_Int
	(*moveTo)(THIS, JA_Int step, JA_Boolean wait, JA_Int speed, THIS syn_Motor, enObjType objType);

	/**
	 * 驱动步进电机转到原点位置。\n
	 *
	 * @param[in]		wait		阻塞操作标识，为 Ture 时此接口会等到转动操作完毕后才返回。
	 *
	 * @return	操作成功返回 0，失败返回 -1。
	 */
	JA_Int
	(*home)(THIS, JA_Boolean wait, JA_Int speed, enObjType objType);


	/**
	 * 终止步进电机转动。\n
	 * 在调用 TJA_StepperMotor::move 和 JA_StepperMotor::moveTo 方法，\n
	 * 并且电机已经开始转动的过程中，调用此接口可以终止电机转动。\n
	 *
	 * @return	操作成功返回 0，失败返回 -1。
	 */
	JA_Int
	(*stop)(THIS);


	/**
	 * 获取当前步进电机所在步数。
	 *
	 * @param[out]		val		所在步数的值。
	 *
	 * @return	获取成功返回 0，结果在 @ref val 中取得，获取失败返回 -1。
	 */
	JA_Int
	(*getStep)(THIS, JA_Int *val);

	JA_Int
	(*Init)(THIS, JA_Boolean wait, JA_Int speed, JA_Int max_step
		, TJA_StepperMotor *syn_Motor, JA_Int max_SynStep, enObjType objType);

	JA_Int
	(*InitTarget)(THIS, JA_Boolean wait, JA_Int speed, JA_Int target_step
		, TJA_StepperMotor *syn_Motor, JA_Int target_SynStep, enObjType objType);

	JA_Boolean
	(*Terminated)(THIS);
	
	JA_Int
	(*setCurStep)(THIS, JA_Int *currentStep);

#undef THIS
};


/**
 * 步进电机转动时触发回调。\n
 * 步进电机在运转时候会调用此接口。\n
 * 参数 @ref tick 会根据点击定义的节拍周期以此传入，\n
 * 如步进电机节拍周期为 4，则在正向转动时会依次传入 0,1,2,3,0,1,2,3...\n
 * 反向转动时候则传入 0,3,2,1,0,3,2,1...\n
 * 用户根据电机的特点以及步进节拍做出响应的控制。
 *
 * @param[in]		user_ctx	用户自定义上下文。
 * @param[in]		tick		当前步进节拍。
 * @param[in]		tick_cycle	用户设定的节拍周期数。
 *
 * @return	电机控制运转正常返回 0，点击控制错误需要返回 -1。\n
 * 			模块内部检测到电机控制错误则会终止当前步进动作。
 */
typedef JA_Int		(*TJA_StepperMotorOnStep)(TJA_StepperMotor *const Motor, JA_PVoid user_ctx, JA_Int tick, JA_Int tick_cycle);

/**
 * 步进电机限位探测回调。\n
 * 步进电机每走一步会调用此接口进行探测是否到极限位置。\n
 *
 * @param[in]		user_ctx	用户自定义上下文。
 *
 * @return	步进电机到了极限位置返回 True，否则返回 False。
 */
typedef JA_Boolean	(*TJA_StepperMotorOnLimit)(TJA_StepperMotor *const Motor, JA_PVoid user_ctx);


/**
 * 步进电机校验完毕回调。\n
 * 当用户设定了此回调以后在模块内部校验完毕以后会触发此回调。\n
 * 一般来说在此回调里做的动作就是驱动电机到一个默认位置。
 *
 * @param[in]		user_ctx	用户自定义上下文。
 *
 */
typedef JA_Void		(*TJA_StepperMotorOnCalibrated)(TJA_StepperMotor *const Motor
													, JA_PVoid user_ctx
													, JA_Int speed);

typedef JA_Int		(*TJA_StepperMotorStop)(TJA_StepperMotor *const Motor);

typedef JA_Void		(*TJA_StepperMoveCallBack)(void);
/**
 * 创建 StepperMotor 模块句柄。\n
 * 针对双限位开关的步进电机。\n
 *
 * @param[in]		MemAllocator		模块内使用的内存分配器。
 * @param[in]		tick_cycle			步进电机的节拍周期，电机工作时会根据节拍周期自增/自减节拍并在触发 @ref onStep 传给用户。
 * @param[in]		onStep				步进触发动作回调。
 * @param[in]		onLimitLower		下位限位探测回调。
 * @param[in]		onLimitUpper		上位限位探测回调。
 * @param[in]		onCalibrated		校验完毕回调。
 * @param[in]		user_ctx			用户自定义上下文，会在各回调接口中回传给用户。
 *
 * @return	创建成功返回模块句柄，失败返回 Nil。
 */
extern TJA_StepperMotor *
JaStepperMotor_Create(TJA_MemAllocator *MemAllocator,//可以不用
		JA_Int tick_cycle,//决定节拍
		TJA_StepperMotorOnStep onStep,
		TJA_StepperMotorOnLimit onLimitLower,
		TJA_StepperMotorOnLimit onLimitUpper,
		TJA_StepperMotorOnCalibrated onCalibrated,//自检事件，可以不用
		JA_PVoid user_ctx,
		TJA_StepperMotorStop StopMotor,
		TJA_StepperMoveCallBack MoveCallBack);//不用


/**
 * 创建 StepperMotor 模块句柄。\n
 * 针对单限位开关的步进电机。\n
 *
 * @param[in]		MemAllocator		模块内使用的内存分配器。
 * @param[in]		tick_cycle			步进电机的节拍周期，电机工作时会根据节拍周期自增/自减节拍并在回调 @ref onStep 传给用户。
 * @param[in]		onStep				步进触发动作回调。
 * @param[in]		onLimitLower		下位限位探测回调。
 * @param[in]		full_distance		电机运行的全步程，在单限位开关的电机，通过实验计算步程并在电机初始化时预设进电机。
 * @param[in]		onCalibrated		校验完毕回调。
 * @param[in]		user_ctx			用户自定义上下文，会在各回调接口中回传给用户。
 *
 * @return	创建成功返回模块句柄，失败返回 Nil。
 */
extern TJA_StepperMotor *
JaStepperMotor_Create2(TJA_MemAllocator *MemAllocator,
		JA_Int tick_cycle,
		TJA_StepperMotorOnStep onStep,
		TJA_StepperMotorOnLimit onLimitLower,
		JA_Int full_distance,
		TJA_StepperMotorOnCalibrated onCalibrated,
		JA_PVoid user_ctx,
		TJA_StepperMotorStop StopMotor,
		TJA_StepperMoveCallBack MoveCallBack);


/**
 * 销毁 StepperMotor 模块句柄。
 */
extern JA_Void
JaStepperMotor_Free(TJA_StepperMotor **StepperMotor_r);


#ifdef __cplusplus 
	}
#endif

#endif /* BSP_STEPPER_MOTOR_H_ */
