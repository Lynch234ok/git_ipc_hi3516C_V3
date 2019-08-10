#if defined(GSENSOR)

#ifndef GIT_IPC_DEVICE_BINDING_H
#define GIT_IPC_DEVICE_BINDING_H


#include <stdint.h>

/*
 * call order:
 * GSENSOR_init()
 * -> GSENSOR_set_correct_xyz()/GSENSOR_get_angles()/GSENSOR_auto_correct()
 * -> GSENSOR_deinit()
 */

/*
 * 初始化
 */
extern int GSENSOR_init( void );
/*
 * 去初始化
 */
extern void GSENSOR_deinit( void );

/*
 * 获取各个角度
 */
extern void GSENSOR_get_angles(double *p_angle_x, double *p_angle_y, double *p_angle_z);

/*
 * 自动校准
 * 会设置并输出三个校准值，以后 GSensor 初始化之后需要设置这三个校准值
 */
extern void GSENSOR_auto_correct(int8_t *p_cor_x,
                                 int8_t *p_cor_y,
                                 int8_t *p_cor_z);
/*
 * 设置校准值
 */
extern void GSENSOR_set_correct_xyz(int8_t cor_x,
                                    int8_t cor_y,
                                    int8_t cor_z);

/*
@return true 初始化成功
        false 初始化失败
*/
extern bool GSENSOR_is_init();

/*
 * Do not GSENSOR_init() or GSENSOR_deinit() when testing
 */
extern int GSENSOR_test_start( void );
extern void GSENSOR_test_stop( void );


#endif // GIT_IPC_DEVICE_BINDING_H

#endif // defined(GSENSOR)
