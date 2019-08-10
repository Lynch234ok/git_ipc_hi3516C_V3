#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define AT2C_ADDR (0xA0)

#if   defined(SD_GPIO_I2C)

static int SD_I2C_Init(void)
{
	return i2c_port_init();
}

static int SD_I2C_Exit(void)
{
	return i2c_port_exit();
}

static unsigned char SD_I2C_Read(int Addr, int Reg, int *Val)
{
	//usleep(50);  //Need Time Delay to Access Low Speed Device
	return i2c_ReadByte(Addr, Reg, Val);
}

static int SD_I2C_Write(int Addr, int Reg, int Val)
{
	usleep(50);  //Need Time Delay to Access Low Speed Device
	return i2c_WriteByte(Addr, Reg, Val);
}

#elif defined(SD_HI_I2C)

static int SD_I2C_Init(void)
{
	return hi_i2c_init();
}

static int SD_I2C_Exit(void)
{
	return hi_i2c_exit();
}

static unsigned char SD_I2C_Read(int Addr, int Reg, int *Val)
{
	//usleep(50);  //Need Time Delay to Access Low Speed Device
	return hi_i2c_read(Addr, Reg, Val);
}

static int SD_I2C_Write(int Addr, int Reg, int Val)
{
	usleep(50);  //Need Time Delay to Access Low Speed Device
	return hi_i2c_write(Addr, Reg, Val);
}

#else
static int SD_I2C_Init(void)
{
	return 0;
}

static int SD_I2C_Exit(void)
{
	return 0;
}

static unsigned char SD_I2C_Read(int Addr, int Reg, int *Val)
{
	return 0;
}

static int SD_I2C_Write(int Addr, int Reg, int Val)
{
	return 0;
}

#endif

int SD_Block_Read(unsigned char *pBuf, int nLen)
{
	int  tmpSize = nLen;
	int  ii;

	if(0 != SD_I2C_Init()) {
		return -1;
	}

	for(ii = 0; ii < tmpSize; ii ++) {
		int tmpVal = 0;
		if(0 != SD_I2C_Read(AT2C_ADDR, ii, &tmpVal)) {
			return -1;
		}
		pBuf[ii] = tmpVal;
	}

	if(0 != SD_I2C_Exit()) {
		return -1;
	}

	return 0;
}

int SD_Block_Write(unsigned char *pBuf, int nLen)
{
	int  tmpSize = nLen;
	int  ii;

	if(0 != SD_I2C_Init()) {
		return -1;
	}

	for(ii = 0; ii < tmpSize; ii ++) {
		if(0 != SD_I2C_Write(AT2C_ADDR, ii, pBuf[ii])) {
			return -1;
		}
	}

	if(0 != SD_I2C_Exit()) {
		return -1;
	}

	return 0;
}
