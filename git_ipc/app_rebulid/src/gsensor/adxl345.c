#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "gsensor_hi_i2c.h"
#include "adxl345.h"





/*
 * Addresses
 */

#define ADXL345_CHIPID  (0xE5)

#define ADXL345_DEVICE_ID       0x00
#define ADXL345_THRESH_TAP      0x1D
#define ADXL345_OFSX            0x1E
#define ADXL345_OFSY            0x1F
#define ADXL345_OFSZ            0x20
#define ADXL345_DUR             0x21
#define ADXL345_Latent          0x22
#define ADXL345_Window          0x23
#define ADXL345_THRESH_ACK      0x24
#define ADXL345_THRESH_INACT    0x25
#define ADXL345_TIME_INACT      0x26
#define ADXL345_ACT_INACT_CTL   0x27
#define ADXL345_THRESH_FF       0x28
#define ADXL345_TIME_FF         0x29
#define ADXL345_TAP_AXES        0x2A
#define ADXL345_ACT_TAP_STATUS  0x2B
#define ADXL345_BW_RATE         0x2C
#define ADXL345_POWER_CTL       0x2D

#define ADXL345_INT_ENABLE      0x2E
#define ADXL345_INT_MAP         0x2F
#define ADXL345_INT_SOURCE      0x30
#define ADXL345_DATA_FORMAT     0x31
#define ADXL345_DATA_X0         0x32
#define ADXL345_DATA_X1         0x33
#define ADXL345_DATA_Y0         0x34
#define ADXL345_DATA_Y1         0x35
#define ADXL345_DATA_Z0         0x36
#define ADXL345_DATA_Z1         0x37
#define ADXL345_FIFO_CTL        0x38
#define ADXL345_FIFO_STATUS     0x39



static void delay_ms(unsigned int ms) {
	usleep(1000 * ms);
}

static void ADXL345_WR_Reg(uint8_t addr, uint8_t val)
{
	if (0 != gsensor_hi_i2c_write(addr, val)) {
        printf("[%s:%d] hi_i2c_write failed!\n", __FUNCTION__, __LINE__);
	}
}

static uint8_t ADXL345_RD_Reg(uint8_t addr)
{
	uint8_t val = 0;
	if (0 != gsensor_hi_i2c_read(addr, &val)) {
        printf("[%s:%d] hi_i2c_read failed!\n", __FUNCTION__, __LINE__);
	}

	return val;
}

/*
 * 读取分量
 */
void ADXL345_Read_XYZ(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t buf[6];
    uint8_t i;
	for(i = 0; i < 6; i ++) {  //Read X/Y/Z Axis Data;
		buf[i] = ADXL345_RD_Reg((uint8_t)ADXL345_DATA_X0+i);
	}

	*x = (int16_t)(((uint16_t)buf[1]<<8) + buf[0]);
	*y = (int16_t)(((uint16_t)buf[3]<<8) + buf[2]);
	*z = (int16_t)(((uint16_t)buf[5]<<8) + buf[4]);
}

int ADXL345_Init(void)
{
	if (0 != gsensor_hi_i2c_init()) {
		printf("[%s:%d] i2c init failed!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(ADXL345_RD_Reg(ADXL345_DEVICE_ID) == ADXL345_CHIPID) { 	// 读取器件ID
		ADXL345_WR_Reg(ADXL345_DATA_FORMAT,  0X2B);				// 低电平中断输出,13位全分辨率,输出数据右对齐,16g量程
		ADXL345_WR_Reg(ADXL345_BW_RATE,      0x0A);				// 数据输出速度为100Hz
		ADXL345_WR_Reg(ADXL345_POWER_CTL,    0x28);  			// 链接使能,测量模式
		ADXL345_WR_Reg(ADXL345_INT_ENABLE,   0x00);				// 不使用中断

		return 0;
	}

	return -1;
}

void ADXL345_Deinit(void)
{
    if (0 != gsensor_hi_i2c_exit()) {
        printf("[%s:%d] i2c exit failed!\n", __FUNCTION__, __LINE__);
    }
}

//读取ADXL的平均值
//x,y,z:读取10次后取平均值
static void ADXL345_RD_Avval(int16_t *x, int16_t *y, int16_t *z)
{
	int16_t tx = 0, ty = 0, tz = 0;
    int i;

	for(i=0;i<10;i++) {
        ADXL345_Read_XYZ(x, y, z);
		delay_ms(10);

		tx+=*x;
		ty+=*y;
		tz+=*z;
	}

	*x = tx/((int16_t)10);
	*y = ty/((int16_t)10);
	*z = tz/((int16_t)10);
}

void ADXL345_AUTO_Adjust(int8_t *p_cor_x, int8_t *p_cor_y, int8_t *p_cor_z)
{
	int16_t tx, ty, tz;
	int i;
	int16_t offx = 0, offy = 0, offz = 0;

	ADXL345_WR_Reg(ADXL345_POWER_CTL,0x00);   //先进入休眠模式.
	delay_ms(100);

	ADXL345_WR_Reg(ADXL345_DATA_FORMAT,0X2B); //低电平中断输出,13位全分辨率,输出数据右对齐,16g量程
	ADXL345_WR_Reg(ADXL345_BW_RATE,0x0A);     //数据输出速度为100Hz
	ADXL345_WR_Reg(ADXL345_POWER_CTL,0x28);   //链接使能,测量模式
	ADXL345_WR_Reg(ADXL345_INT_ENABLE,0x00);  //不使用中断

	ADXL345_WR_Reg(ADXL345_OFSX,0x00);
	ADXL345_WR_Reg(ADXL345_OFSY,0x00);
	ADXL345_WR_Reg(ADXL345_OFSZ,0x00);
	delay_ms(12);

	for(i = 0; i < 10; i ++) {
		ADXL345_RD_Avval(&tx, &ty, &tz);
		offx += tx;
		offy += ty;
		offz += tz;
	}

	offx/=10;
	offy/=10;
	offz/=10;

	*p_cor_x = (int8_t)(-offx/(int16_t)4);
	*p_cor_y = (int8_t)(-offy/(int16_t)4);
	*p_cor_z = (int8_t)(-(offz-(int16_t)256)/(int16_t)4);

	ADXL345_WR_Reg(ADXL345_OFSX, (uint8_t)(*p_cor_x));
	ADXL345_WR_Reg(ADXL345_OFSY, (uint8_t)(*p_cor_y));
	ADXL345_WR_Reg(ADXL345_OFSZ, (uint8_t)(*p_cor_z));
}

//读取ADXL345的数据times次,再取平均
//x,y,z:读到的数据
//times:读取多少次
void ADXL345_Read_Average(int16_t *x, int16_t *y, int16_t *z, int times)
{
	int i;
    int16_t tx,ty,tz;

	*x=0;
	*y=0;
	*z=0;

	if(times) {//读取次数不为0
		for(i = 0;i < times;i ++) {//连续读取times次
            ADXL345_Read_XYZ(&tx,&ty,&tz);

			*x+=tx;
			*y+=ty;
			*z+=tz;
			delay_ms(5);
		}

		*x /= times;
		*y /= times;
		*z /= times;
	}
}

//得到角度
//x,y,z:x,y,z方向的重力加速度分量(不需要单位,直接数值即可)
//dir: 要获得相对于哪个轴的角度. 0, Z轴; 1, X轴; 2, Y轴.
//返回值:角度值.单位0.1°.
int16_t ADXL345_Get_Angle(double x, double y, double z, int dir)
{

	double temp;
	double res = 0;

	switch(dir) {
		case 0://与自然Z轴的角度
			temp = sqrt((x*x+y*y))/z;
			res  = atan(temp);
			break;

		case 1://与自然X轴的角度
			temp = x/sqrt((y*y+z*z));
			res  = atan(temp);
			break;

		case 2://与自然Y轴的角度
			temp = y/sqrt((x*x+z*z));
			res  = atan(temp);
			break;

		default://与自然Z轴的角度
			temp = sqrt((x*x+y*y))/z;
			res  = atan(temp);
	}

	return (int16_t)(res*1800/3.14);
}

// 得到三个角度. 单位0.1°
// x,y,z: x,y,z方向的重力加速度分量(不需要单位,直接数值即可)
void ADXL345_Get_Angles(double x, double y, double z,
                        double *p_angle_x, double *p_angle_y, double *p_angle_z)
{
	double temp;
	double res = 0;

	// angle x
	temp = x/sqrt((y*y+z*z));
	res  = atan(temp);
	*p_angle_x = res*1800/3.14;


	// angle y
	temp = y/sqrt((x*x+z*z));
	res  = atan(temp);
	*p_angle_y = res*1800/3.14;


	// angle z
	temp = sqrt((x*x+y*y))/z;
	res  = atan(temp);
	*p_angle_z = res*1800/3.14;
}

/*
 * 计算偏移值(用于校准)
 */
void ADXL345_Cal_CorrectXYZ(int16_t x, int16_t y, int16_t z,
                            int8_t *p_cor_x, int8_t *p_cor_y, int8_t *p_cor_z)
{
    *p_cor_x = (int8_t)(-x/(int16_t)4);
    *p_cor_y = (int8_t)(-y/(int16_t)4);
    *p_cor_z = (int8_t)(-(z-(int16_t)256)/(int16_t)4);
}

/*
 * 读取偏移值(用于校准)
 */
void ADXL345_Write_CorrectXYZ(int8_t cor_x, int8_t cor_y, int8_t cor_z)
{
    ADXL345_WR_Reg(ADXL345_OFSX, (uint8_t)cor_x);
    ADXL345_WR_Reg(ADXL345_OFSY, (uint8_t)cor_y);
    ADXL345_WR_Reg(ADXL345_OFSZ, (uint8_t)cor_z);
}

/*
 * 写入偏移值(用于校准)
 */
void ADXL345_Read_CorrectXYZ(int8_t *p_cor_x, int8_t *p_cor_y, int8_t *p_cor_z)
{
    *p_cor_x = (int8_t)ADXL345_RD_Reg(ADXL345_OFSX);
    *p_cor_y = (int8_t)ADXL345_RD_Reg(ADXL345_OFSY);
    *p_cor_z = (int8_t)ADXL345_RD_Reg(ADXL345_OFSZ);
}
