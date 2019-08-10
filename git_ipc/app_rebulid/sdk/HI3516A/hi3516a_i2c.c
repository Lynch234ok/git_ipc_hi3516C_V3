#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "hi_spi.h"
#include "hi_i2c.h"
#include "hi_isp_i2c.h"


static int g_fd = -1;

int _i2c_read(int addr, unsigned short* ret_data, char sensor_i2c_addr, unsigned int sensor_addr_byte,unsigned sensor_data_byte)
{
	int fd = -1;
	int ret;
	char buf[4];
	int data;

	fd = open("/dev/i2c-0", O_RDWR);
	
	if(fd < 0)
	{
		printf("Open /dev/i2c-0 error!\n");
		return -1;
	}

	ret = ioctl(fd, I2C_SLAVE_FORCE, sensor_i2c_addr);
	if (ret < 0)
	{
		printf("CMD_SET_DEV error!\n");
		close(fd);
		return -1;
	}

	if (sensor_addr_byte == 2)
	{
		ret = ioctl(fd, I2C_16BIT_REG, 1);
	}
	else
	{
		ret = ioctl(fd, I2C_16BIT_REG, 0);
	}
	if (ret < 0)
	{
		printf("CMD_SET_REG_WIDTH error!\n");
		close(fd);
		return -1;
	}	
	
	if (sensor_data_byte == 2)
	{
		ret = ioctl(fd, I2C_16BIT_DATA, 1);
	}
	else
	{
		ret = ioctl(fd, I2C_16BIT_DATA, 0);
	}
	
	if (ret < 0)
	{
			printf("CMD_SET_DATA_WIDTH error!\n");
		close(fd);
			return -1;
	}
	if (sensor_addr_byte == 2) {
		buf[0] = addr & 0xff;
		buf[1] = (addr >> 8) & 0xff;
	} else{
		buf[0] = addr & 0xff;
	}

	ret = read(fd, buf, sensor_addr_byte);
	if (ret < 0)
	{		
		printf("hi_i2c read faild!\n");
		close(fd);
		return -1;
	}

	if(sensor_data_byte == 2)
	{
		data = buf[0] | (buf[1] << 8);
	}
	else 
	{
		data = buf[0];
	}
	
	*ret_data = data;
	
	close(fd);
	return 0;
}


int _i2c_write(int addr,int data,char sensor_i2c_addr, unsigned int sensor_addr_byte,unsigned sensor_data_byte)
{	 
	int fd = -1;
	int idx = 0;
	int ret;
	char buf[8];

	fd = open("/dev/i2c-0", O_RDWR);
	
	if(fd < 0)
	{
		printf("Open /dev/i2c-0 error!\n");
		return -1;
	}

	ret = ioctl(fd, I2C_SLAVE_FORCE, sensor_i2c_addr);
	if (ret < 0)
	{
		printf("CMD_SET_DEV error!\n");
		close(fd);
		return -1;
	}

	buf[idx++] = addr & 0xFF;
	if (sensor_addr_byte == 2)
	{
		ret = ioctl(fd, I2C_16BIT_REG, 1);
		buf[idx++] = addr >> 8;
	}
	else
	{
		ret = ioctl(fd, I2C_16BIT_REG, 0);
	}

	if (ret < 0)
	{
		printf("CMD_SET_REG_WIDTH error!\n");
		close(fd);
		return -1;
	}

	buf[idx++] = data;
	if (sensor_data_byte == 2)
	{
		ret = ioctl(fd, I2C_16BIT_DATA, 1);
		buf[idx++] = data >> 8;
	}
	
	else
	{
		ret = ioctl(fd, I2C_16BIT_DATA, 0);
	}

	if (ret)
	{
		printf("hi_i2c write faild!\n");
		close(fd);
		return -1;
	}

	write(fd, buf, idx);
	if(ret < 0)
	{
		printf("I2C_WRITE error!\n");
		return -1;
	}

	close(fd);
	return 0;	
 }

static int sensor_spi_init(void)
{
    if(g_fd >= 0)
    {
        return 0;
    }    
    unsigned int value;
    int ret = 0;
    char file_name[] = "/dev/spidev0.0";
    
    g_fd = open(file_name, 0);
    if (g_fd < 0)
    {
        printf("Open %s error!\n",file_name);
        return -1;
    }

    value = SPI_MODE_3 | SPI_LSB_FIRST;// | SPI_LOOP;
    ret = ioctl(g_fd, SPI_IOC_WR_MODE, &value);
    if (ret < 0)
    {
        printf("ioctl SPI_IOC_WR_MODE err, value = %d ret = %d\n", value, ret);
        return ret;
    }

    value = 8;
    ret = ioctl(g_fd, SPI_IOC_WR_BITS_PER_WORD, &value);
    if (ret < 0)
    {
        printf("ioctl SPI_IOC_WR_BITS_PER_WORD err, value = %d ret = %d\n",value, ret);
        return ret;
    }

    value = 2000000;
    ret = ioctl(g_fd, SPI_IOC_WR_MAX_SPEED_HZ, &value);
    if (ret < 0)
    {
        printf("ioctl SPI_IOC_WR_MAX_SPEED_HZ err, value = %d ret = %d\n",value, ret);
        return ret;
    }

    return 0;
}


int _spi_read(unsigned int addr, unsigned short* ret_data)
{	 
	sensor_spi_init();
	int ret = 0;
	struct spi_ioc_transfer mesg[1];
	unsigned char  tx_buf[8] = {0};
	unsigned char  rx_buf[8] = {0};
	
	tx_buf[0] = (addr & 0xff00) >> 8;
	tx_buf[0] |= 0x80;
	tx_buf[1] = addr & 0xff;
	tx_buf[2] = 0;

	memset(mesg, 0, sizeof(mesg));
	mesg[0].tx_buf = (__u32)tx_buf;
	mesg[0].len    = 3;
	mesg[0].rx_buf = (__u32)rx_buf;
	mesg[0].cs_change = 1;

	ret = ioctl(g_fd, SPI_IOC_MESSAGE(1), mesg);
	if (ret  < 0) {  
		printf("SPI_IOC_MESSAGE error \n");  
		return -1;	
	} 
	//printf("0x%x 0x%x\n", addr, rx_buf[2]); 
    *ret_data = rx_buf[2];
	//printf("0x%x ret_data = 0x%x\n", addr, *ret_data); 
	
	return 0;
}

  
int _spi_write(unsigned int addr, unsigned char data)
{	 
	sensor_spi_init();
	struct spi_ioc_transfer mesg[1];
	unsigned char  tx_buf[8] = {0};
	unsigned char  rx_buf[8] = {0};
	int ret;
	tx_buf[0] = (addr & 0xff00) >> 8;
	tx_buf[0] &= (~0x80);
	tx_buf[1] = addr & 0xff;
	tx_buf[2] = data;

	memset(mesg, 0, sizeof(mesg));	
	mesg[0].tx_buf = (__u32)tx_buf;  
	mesg[0].len    = 3;  
	mesg[0].rx_buf = (__u32)rx_buf; 
	mesg[0].cs_change = 1;

	ret = ioctl(g_fd, SPI_IOC_MESSAGE(1), mesg);
	if (ret < 0) {	
		printf("SPI_IOC_MESSAGE error \n");  
		return -1;	
	}

	return 0;
}


