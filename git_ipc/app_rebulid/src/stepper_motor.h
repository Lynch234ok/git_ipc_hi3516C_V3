/*
 * JUAN BSP ģ�鼯��\n
 * �����������ģ�顣\n
 * ͨ���Բ������������ſ���ʵ�ֿ��Ʋ������ת����Ŀ�ġ�\n
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
 *  �������������Step�������\n
 *    ���������ʵ������ʱ���ԡ�����Step)����Ϊ��λ��\n
 *    ÿ�������ǰ��һ����λ������һ�����ߵĲ�����Ϊ���̣�Step Distance����\n
 *    ��������λ�ߵ����λ������λ���ؾ�����ȫ�������ߵĲ���Ϊȫ���̣�Full Step Distance����\n
 *    ������Ҫ���û���ʼ�������ʱ��ָ���������û�����У׼������Calibrate��ʱ���������\n
 *
 *  ����������Ƕȣ�Degree�������\n
 *    ���������ʵ��ʹ�õ�ʱ����ܲ��������ߵĲ�����\n
 *    �����Ĳ�������ߵ��Ǹ��Ƕȣ�������롰�Ƕȣ�Degree�������\n
 *    ����ĽǶ��Ǹ��ݵ���ڲ�����Ĳ��̣������û��趨�Ĳο��Ƕȣ�@ref TJA_StepperMotor::setRefDegree���ڲ�������ɡ�\n
 *    ���û��趨�ο��Ƕ��� -180 -- 180���� 0 ����Ӧ�ĽǶ�Ϊ -180 �ȡ�\n
 *    �ο��Ƕȿ��Ը���ʵ�ʵ��Ӧ�ó���ȥ��������Ϊʲôֵ��\n
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
 * StepperMotor ��Ϣ���ݽṹ��
 */
typedef struct jaStepperMotorInfo {

	/**
	 * ����������ݽṹ��
	 */
	struct {

		JA_Int lower, upper, home; ///< ����趨�ĵ�λ����λ��ԭ�㲽�̡�
		JA_Int current; ///< ��ǰ������ڲ��̡�

	} Distance;

	/**
	 * �Ƕ�������ݽṹ��
	 */
	struct {

		JA_Float lower, upper, home; ///< ����趨�ĵ�λ����λ��ԭ��Ƕȡ�
		JA_Float current; ///< ��ǰ������ڽǶȡ�

	} Degree;

} TJA_StepperMotorInfo;

typedef struct jaStepperMotor TJA_StepperMotor;

/**
 * StepperMotor ģ������
 */
struct jaStepperMotor {
#define THIS struct jaStepperMotor *const

	/**
	 * ģ������ӿڡ�
	 */
	//TJA_Object Object;	

	/**
	 * ���òο�������\n
	 * �����ο�������Ҫ���û�����ʵ������������ã�\n
	 * һ����ڹ̶�����ֻ��Ҫ����һ�Ρ�\n
	 * ���ô˽ӿ�ʱ���ڲ������ @ref TJA_StepperMotor::calibrate ����У׼��\n
	 * ����û������ô˽ӿ�ȥ���òο��Ƕ�ģ���ڲ���Ĭ���趨 lower:0 upper:+3000 home:1500 ����\n
	 * ���û��ڵ��ʹ��ǰ���� @ref TJA_StepperMotor::calibrate ȥУ׼����������޷�ʹ�á�\n
	 * @ref TJA_StepperMotor::moveTo �ӿڻ�����趨�Ĳ����жϴ��벽���Ƿ�Ϸ���\n
	 *
	 *
	 * @param[in]		lower		��λ���Ʋο��Ƕȣ�������Ĭ��Ϊ 0��
	 * @param[in]		upper		��λ���Ʋο��Ƕȣ�������Ĭ��Ϊ 3000��
	 * @param[in]		home		��λ���ڽǶȣ�������Ĭ��Ϊ 1500��
	 * @param[in]		wait		�ڲ����� TJA_StepperMotor::calibrate �Ƿ�������ʶ���������Ƿ�ӵ������������ڵ��� TJA_StepperMotor::calibrate ��Ϻ����ʹ���������ܡ�
	 *
	 *
	 * @return	���óɹ����� 0��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*setRefStep)(THIS, JA_Int lower, JA_Int upper, JA_Int home, JA_Boolean wait);

	/**
	 * �������У׼��\n
	 * ÿ�ε����� @ref TJA_StepperMotor::setRefDegree ����Ҫ���ô˽ӿڽ��нǶ�У׼��\n
	 *
	 */
	JA_Void
	(*calibrate)(THIS, JA_Boolean wait, JA_Int speed);

	/**
	 * ��ȡ���������Ϣ��
	 *
	 * @param[out]		Info_r		�����Ϣ���ݽṹ���á�
	 *
	 * @return	��ȡ�ɹ����� 0���ṹ�� @ref Info_r ���ݽṹ�����֣�ʧ�ܷ��� -1��
	 *
	 */
	JA_Int
	(*getInfo)(THIS, TJA_StepperMotorInfo *Info_r);

	/**
	 * �����������ת����\n
	 * ���ô˽ӿ�֮��\n
	 * �����������в����� TJA_StepperMotor::stop ������\n
	 * �����һֱת���ø÷����ϵļ���λ�ò�ֹͣ��\n
	 *
	 * @param[in]		forward		ת������Ture Ϊ����ת����False Ϊ����ת����
	 * @param[in]		wait		����������ʶ��Ϊ Ture ʱ�˽ӿڻ�ȵ�ת��������Ϻ�ŷ��ء�
	 * @param[in]		speed		����ٶ�
	 * @param[in]		objType		��������
	 *
	 * @return	�����ɹ����� 0��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*move)(THIS, JA_Boolean forward, JA_Boolean wait, JA_Int speed, THIS syn_Motor, enObjType objType);

	/**
	 * �����������ת�����̶��Ƕȡ�\n
	 * ���ô˽ӿ�֮��\n
	 * �����������в����� TJA_StepperMotor::stop ������\n
	 * �����һֱת���øýǶ��ϲ�ֹͣ��\n
	 *
	 * @param[in]		degree		Ŀ��Ƕȡ�
	 * @param[in]		wait		����������ʶ��Ϊ Ture ʱ�˽ӿڻ�ȵ�ת��������Ϻ�ŷ��ء�
	 * @param[in]		speed		����ٶ�
	 * @param[in]		objType		��������
	 *
	 * @return	�����ɹ����� 0��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*moveTo)(THIS, JA_Int step, JA_Boolean wait, JA_Int speed, THIS syn_Motor, enObjType objType);

	/**
	 * �����������ת��ԭ��λ�á�\n
	 *
	 * @param[in]		wait		����������ʶ��Ϊ Ture ʱ�˽ӿڻ�ȵ�ת��������Ϻ�ŷ��ء�
	 *
	 * @return	�����ɹ����� 0��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*home)(THIS, JA_Boolean wait, JA_Int speed, enObjType objType);


	/**
	 * ��ֹ�������ת����\n
	 * �ڵ��� TJA_StepperMotor::move �� JA_StepperMotor::moveTo ������\n
	 * ���ҵ���Ѿ���ʼת���Ĺ����У����ô˽ӿڿ�����ֹ���ת����\n
	 *
	 * @return	�����ɹ����� 0��ʧ�ܷ��� -1��
	 */
	JA_Int
	(*stop)(THIS);


	/**
	 * ��ȡ��ǰ����������ڲ�����
	 *
	 * @param[out]		val		���ڲ�����ֵ��
	 *
	 * @return	��ȡ�ɹ����� 0������� @ref val ��ȡ�ã���ȡʧ�ܷ��� -1��
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
 * �������ת��ʱ�����ص���\n
 * �����������תʱ�����ô˽ӿڡ�\n
 * ���� @ref tick ����ݵ������Ľ��������Դ˴��룬\n
 * �粽�������������Ϊ 4����������ת��ʱ�����δ��� 0,1,2,3,0,1,2,3...\n
 * ����ת��ʱ������ 0,3,2,1,0,3,2,1...\n
 * �û����ݵ�����ص��Լ���������������Ӧ�Ŀ��ơ�
 *
 * @param[in]		user_ctx	�û��Զ��������ġ�
 * @param[in]		tick		��ǰ�������ġ�
 * @param[in]		tick_cycle	�û��趨�Ľ�����������
 *
 * @return	���������ת�������� 0��������ƴ�����Ҫ���� -1��\n
 * 			ģ���ڲ���⵽������ƴ��������ֹ��ǰ����������
 */
typedef JA_Int		(*TJA_StepperMotorOnStep)(TJA_StepperMotor *const Motor, JA_PVoid user_ctx, JA_Int tick, JA_Int tick_cycle);

/**
 * ���������λ̽��ص���\n
 * �������ÿ��һ������ô˽ӿڽ���̽���Ƿ񵽼���λ�á�\n
 *
 * @param[in]		user_ctx	�û��Զ��������ġ�
 *
 * @return	����������˼���λ�÷��� True�����򷵻� False��
 */
typedef JA_Boolean	(*TJA_StepperMotorOnLimit)(TJA_StepperMotor *const Motor, JA_PVoid user_ctx);


/**
 * �������У����ϻص���\n
 * ���û��趨�˴˻ص��Ժ���ģ���ڲ�У������Ժ�ᴥ���˻ص���\n
 * һ����˵�ڴ˻ص������Ķ����������������һ��Ĭ��λ�á�
 *
 * @param[in]		user_ctx	�û��Զ��������ġ�
 *
 */
typedef JA_Void		(*TJA_StepperMotorOnCalibrated)(TJA_StepperMotor *const Motor
													, JA_PVoid user_ctx
													, JA_Int speed);

typedef JA_Int		(*TJA_StepperMotorStop)(TJA_StepperMotor *const Motor);

typedef JA_Void		(*TJA_StepperMoveCallBack)(void);
/**
 * ���� StepperMotor ģ������\n
 * ���˫��λ���صĲ��������\n
 *
 * @param[in]		MemAllocator		ģ����ʹ�õ��ڴ��������
 * @param[in]		tick_cycle			��������Ľ������ڣ��������ʱ����ݽ�����������/�Լ����Ĳ��ڴ��� @ref onStep �����û���
 * @param[in]		onStep				�������������ص���
 * @param[in]		onLimitLower		��λ��λ̽��ص���
 * @param[in]		onLimitUpper		��λ��λ̽��ص���
 * @param[in]		onCalibrated		У����ϻص���
 * @param[in]		user_ctx			�û��Զ��������ģ����ڸ��ص��ӿ��лش����û���
 *
 * @return	�����ɹ�����ģ������ʧ�ܷ��� Nil��
 */
extern TJA_StepperMotor *
JaStepperMotor_Create(TJA_MemAllocator *MemAllocator,//���Բ���
		JA_Int tick_cycle,//��������
		TJA_StepperMotorOnStep onStep,
		TJA_StepperMotorOnLimit onLimitLower,
		TJA_StepperMotorOnLimit onLimitUpper,
		TJA_StepperMotorOnCalibrated onCalibrated,//�Լ��¼������Բ���
		JA_PVoid user_ctx,
		TJA_StepperMotorStop StopMotor,
		TJA_StepperMoveCallBack MoveCallBack);//����


/**
 * ���� StepperMotor ģ������\n
 * ��Ե���λ���صĲ��������\n
 *
 * @param[in]		MemAllocator		ģ����ʹ�õ��ڴ��������
 * @param[in]		tick_cycle			��������Ľ������ڣ��������ʱ����ݽ�����������/�Լ����Ĳ��ڻص� @ref onStep �����û���
 * @param[in]		onStep				�������������ص���
 * @param[in]		onLimitLower		��λ��λ̽��ص���
 * @param[in]		full_distance		������е�ȫ���̣��ڵ���λ���صĵ����ͨ��ʵ����㲽�̲��ڵ����ʼ��ʱԤ��������
 * @param[in]		onCalibrated		У����ϻص���
 * @param[in]		user_ctx			�û��Զ��������ģ����ڸ��ص��ӿ��лش����û���
 *
 * @return	�����ɹ�����ģ������ʧ�ܷ��� Nil��
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
 * ���� StepperMotor ģ������
 */
extern JA_Void
JaStepperMotor_Free(TJA_StepperMotor **StepperMotor_r);


#ifdef __cplusplus 
	}
#endif

#endif /* BSP_STEPPER_MOTOR_H_ */
