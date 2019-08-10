#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define I2C_SLAVE       0x0703  /* Use this slave address */
#define I2C_SLAVE_FORCE 0x0706  /* Use this slave address, even if it
                                   is already in use by a driver! */
#define I2C_16BIT_REG   0x0709  /* 16BIT REG WIDTH */
#define I2C_16BIT_DATA  0x070a  /* 16BIT DATA WIDTH */

#define HI_I2C_DEVICE "/dev/i2c-2"
static int  hi_i2c_fd = -1;

int  gsensor_hi_i2c_init(void)
{
	unsigned char i2c_addr = 0xA6;//0xA0;  //EEPROM Dev ID

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

	return 0;
}

int  gsensor_hi_i2c_exit(void)
{
	if(hi_i2c_fd >= 0) {
		close(hi_i2c_fd);
		hi_i2c_fd = -1;
	}

	return 0;
}

int gsensor_hi_i2c_read(uint8_t sub_addr, uint8_t *val)
{
    uint8_t buf[4];
    size_t idx = 0;
    ssize_t ret = 0;

    buf[idx++] = sub_addr & ((uint8_t)0xff);

    ret = read(hi_i2c_fd, buf, idx);
    if (ret < 0) {
		printf("\n%s error!\n", __FUNCTION__);
		return -1;
    }

	if(val) {
		*val = buf[0] & ((uint8_t)0xff);
	}

    return 0;
}

int gsensor_hi_i2c_write(uint8_t sub_addr, uint8_t val)
{
    uint8_t buf[8];
    size_t idx = 0;
    ssize_t ret = 0;

    buf[idx++] = sub_addr & ((uint8_t)0xff);
    buf[idx++] = val & ((uint8_t)0xff);

    ret = write(hi_i2c_fd, buf, idx);
    if(ret < 0) {
    	printf("\n%s error!\n", __FUNCTION__);
    	return -1;
    }

	return 0;	
}
