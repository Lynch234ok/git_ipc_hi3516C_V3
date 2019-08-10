#ifndef GIT_IPC_ADXL345_H
#define GIT_IPC_ADXL345_H

#include <stdint.h>


/*
 * 初始化
 */
int ADXL345_Init(void);
/*
 * 去初始化
 */
void ADXL345_Deinit(void);

/*
 * 读取分量
 */
void ADXL345_Read_XYZ(int16_t *x, int16_t *y, int16_t *z);

/*
 * 得到三个角度. 单位0.1°
 */
void ADXL345_Get_Angles(double x, double y, double z,
                        double *p_angle_x, double *p_angle_y, double *p_angle_z);

/*
 * 自动校准
 */
void ADXL345_AUTO_Adjust(int8_t *p_cor_x, int8_t *p_cor_y, int8_t *p_cor_z);

/*
 * 计算偏移值(用于校准)
 */
void ADXL345_Cal_CorrectXYZ(int16_t x, int16_t y, int16_t z,
                            int8_t *p_cor_x, int8_t *p_cor_y, int8_t *p_cor_z);
/*
 * 读取偏移值(用于校准)
 */
void ADXL345_Read_CorrectXYZ(int8_t *p_cor_x, int8_t *p_cor_y, int8_t *p_cor_z);

/*
 * 写入偏移值(用于校准)
 */
void ADXL345_Write_CorrectXYZ(int8_t cor_x, int8_t cor_y, int8_t cor_z);


#endif // GIT_IPC_ADXL345_H
