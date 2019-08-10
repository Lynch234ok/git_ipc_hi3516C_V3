#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include "hisi_i2.h"

#define I2C_SLAVE       0x0703  /* Use this slave address */
#define I2C_SLAVE_FORCE 0x0706  /* Use this slave address, even if it
                                   is already in use by a driver! */
#define I2C_16BIT_REG   0x0709  /* 16BIT REG WIDTH */
#define I2C_16BIT_DATA  0x070a  /* 16BIT DATA WIDTH */

#if  defined(HI3516C_V3) || defined(HI3516E_V1)
#define HI_I2C_DEVICE "/dev/i2c-1"
static int  hi_i2c_fd = -1;
#else
#define HI_I2C_DEVICE "/dev/i2c-2"
static int  hi_i2c_fd = -1;
#endif


int  hi_i2c_init(void)
{
	unsigned char i2c_addr = 0xA0;  //EEPROM Dev ID

#if  defined(HI3516C_V3) || defined(HI3516E_V1)

    hi_i2c_fd = open(HI_I2C_DEVICE, O_RDWR);
    if(hi_i2c_fd < 0) {
        printf("Open "HI_I2C_DEVICE" Failed!\n");

        return -1;
    }

	if (0 > ioctl(hi_i2c_fd, I2C_SLAVE_FORCE, (i2c_addr>>1))) {

        close(hi_i2c_fd);
		hi_i2c_fd = -1;

        return -1;
    }

#else

    hi_i2c_fd = open(HI_I2C_DEVICE, O_RDWR);
    if(hi_i2c_fd < 0) {
        printf("Open "HI_I2C_DEVICE" Failed!\n");

        return -1;
    }

    if((0 > ioctl(hi_i2c_fd, I2C_SLAVE_FORCE, i2c_addr))
	|| (0 > ioctl(hi_i2c_fd, I2C_16BIT_REG, 0))
	|| (0 > ioctl(hi_i2c_fd, I2C_16BIT_DATA, 0))) {
        printf("\nhi_i2c_fd Init Failed!\n");

        close(hi_i2c_fd);
		hi_i2c_fd = -1;

        return -1;
    }

#endif

	return 0;
}

int  hi_i2c_exit(void)
{
	if(hi_i2c_fd >= 0) {
		close(hi_i2c_fd);
		hi_i2c_fd = -1;
	}

	return 0;
}

int hi_i2c_read(int addr, int reg, int * val)
{


#if  defined(HI3516C_V3) || defined(HI3516E_V1)
	
	unsigned char buf[4];
	int retval = -1;
	int data = 0;

	static struct i2c_rdwr_ioctl_data rdwr;
	static struct i2c_msg msg[2];

	memset(buf, 0, sizeof(buf));
    msg[0].addr = addr >> 1;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = buf;

	msg[1].addr = addr >> 1;
	msg[1].flags = 0;
	msg[1].flags |= I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = buf;

    rdwr.msgs = &msg[0];
    rdwr.nmsgs = (unsigned int)2;
	buf[0] = reg & 0xff;

    retval = ioctl(hi_i2c_fd, I2C_RDWR, &rdwr);
    if (retval != 2) {
		printf("\n%s error!\n", __FUNCTION__);
		return -1;
    }

	if(val) {
		*val = buf[0] & 0xff;
	}


	
#else
	
	unsigned char buf[4];
	int  idx = 0;
	int  ret = 0;

    buf[idx++] = reg & 0xff;

    ret = read(hi_i2c_fd, buf, idx);
    if (ret < 0) {
		printf("\n%s error!\n", __FUNCTION__);
		return -1;
    }

	if(val) {
		*val = buf[0] & 0xff;
	}

#endif

    return 0;
}

int  hi_i2c_write(int addr, int reg, int val)
{
    unsigned char buf[8];
    int  idx = 0;
    int  ret = 0;

    buf[idx++] = reg & 0xff;
    buf[idx++] = val & 0xff;

    ret = write(hi_i2c_fd, buf, idx);
    if(ret < 0) {
    	printf("\n%s error!\n", __FUNCTION__);
    	return -1;
    }

	return 0;	
}
