#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include "gpio_i2c_port.h"

#define I2C_GRP_NUM (0)

#define XDBG_DEV "/dev/gpio_i2c"

static int i2c_port_dev = -1;

#define i2c_port_devcheck(x) ((x) >= 0)

static int  LowReadReg(GI2C_ARR_STRUCT * pGI2C)
{
    if (!i2c_port_devcheck(i2c_port_dev) || ioctl(i2c_port_dev, GPIO_I2C_ARR_RD, pGI2C) < 0) {
        printf("ioctl Read Debug Err !!! \n");
        return -1;
    }

	return 0;
}

static int  LowWriteReg(GI2C_ARR_STRUCT * pGI2C)
{
    if (!i2c_port_devcheck(i2c_port_dev) || ioctl(i2c_port_dev, GPIO_I2C_ARR_WR, pGI2C) < 0) {
        printf("ioctl Write Debug Err !!! \n");
        return -1;
    }

	return 0;
}

int  i2c_port_init(void)
{
    int  tmpDev = open(XDBG_DEV, O_RDWR);

    if(i2c_port_devcheck(tmpDev)) {
	    i2c_port_dev = tmpDev;
	    return 0;
	}

    printf("%s Open Failed\n", XDBG_DEV);
    return -1;
}

void  i2c_port_exit(void)
{
    if(i2c_port_devcheck(i2c_port_dev)) {
        close(i2c_port_dev);
		i2c_port_dev = -1;
	}
}

static int CheckI2CDevRemap(int iDev, int *oGrp, int *oDev)
{
	if(oGrp) *oGrp = I2C_GRP_NUM;
	if(oDev) *oDev = iDev;

	return 0;
}

int  i2c_WriteByte(int ucChipAddr, int ucRegAddr, int ucRegValue)
{
    GI2C_ARR_STRUCT tmpStruct;
	int  oGrp = 0, oDev = 0;

	CheckI2CDevRemap(ucChipAddr, &oGrp, &oDev);

    tmpStruct.Grp   = oGrp;
    tmpStruct.Dev   = oDev;
    tmpStruct.RSize = 1;
    tmpStruct.DSize = 1;
	tmpStruct.Buf[0] = ucRegAddr;
	tmpStruct.Buf[1] = ucRegValue;

	return LowWriteReg(&tmpStruct);
}

int  i2c_ReadByte(int ucChipAddr, int ucRegAddr, int * ucValue)
{
    GI2C_ARR_STRUCT tmpStruct;
	int  oGrp = 0, oDev = 0;

	CheckI2CDevRemap(ucChipAddr, &oGrp, &oDev);

    tmpStruct.Grp   = oGrp;
    tmpStruct.Dev   = oDev;
    tmpStruct.RSize = 1;
    tmpStruct.DSize = 1;
	tmpStruct.Buf[0] = ucRegAddr;

	if(0 == LowReadReg(&tmpStruct)) {
		* ucValue = tmpStruct.Buf[1];
		return 0;
	}

	return -1;
}
