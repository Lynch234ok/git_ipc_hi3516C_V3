#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "hi_spi.h"
#include "hi3516c_isp_i2c.h"
#include "hi3516c.h"

#ifdef HI_GPIO_I2C
#include "gpioi2c_ex.h"
#else
#include "hi_i2c.h"
#endif

static int g_fd = -1;

int _i2c_read(int addr,char sensor_i2c_addr, unsigned int sensor_addr_byte,unsigned sensor_data_byte)
{
    unsigned char buf[4];
    int retval = -1;
    int data = 0;
    int ret;

    static struct i2c_rdwr_ioctl_data rdwr;
    static struct i2c_msg msg[2];
 
    g_fd = open("/dev/i2c-0", O_RDWR);
    if(g_fd < 0)
    {
        printf("Open /dev/i2c-0 error!\n");
        return -1;
    }
 
    ret = ioctl(g_fd, I2C_SLAVE_FORCE, (sensor_i2c_addr>>1));
    if (ret < 0)
    {
        printf("CMD_SET_DEV error!\n");
        close(g_fd);
        return ret;
    }
    memset(buf, 0, sizeof(buf));
 
    msg[0].addr = sensor_i2c_addr >> 1;
    msg[0].flags = 0;
    msg[0].len = sensor_addr_byte;
    msg[0].buf = buf;
 
    msg[1].addr = sensor_i2c_addr >> 1;
    msg[1].flags = 0;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = sensor_data_byte;
    msg[1].buf = buf;
 
    rdwr.msgs = &msg[0];
    rdwr.nmsgs = (unsigned int)2;
 
    if(sensor_addr_byte == 2)
    {
 	   buf[0] = (addr >> 8) & 0xff;
 	   buf[1] = addr & 0xff;
    }
    else
    {
 	   buf[0] = addr & 0xff;
    }
 
    retval = ioctl(g_fd, I2C_RDWR, &rdwr);
    if (retval != 2) {
 	   printf("CMD_I2C_READ error!\n");
 	   close(g_fd);
 	   return -1;
    }
 
    if (sensor_data_byte == 2)
    {
 	   data = buf[1] | (buf[0] << 8);
    }
    else
    {
 	   data = buf[0];
    }
 
    printf("0x%x 0x%x\n",addr, data); 
    close(g_fd);
    return data;


}

 
 int _i2c_write(int addr,int data,char sensor_i2c_addr, unsigned int sensor_addr_byte,unsigned sensor_data_byte)
 {	 
 	int idx = 0;
	int ret;
	char buf[8];

    g_fd = open("/dev/i2c-0", O_RDWR);
    if(g_fd < 0)
    {
        printf("Open /dev/i2c-0 error!\n");
        return -1;
    }

    ret = ioctl(g_fd, I2C_SLAVE_FORCE, (sensor_i2c_addr>>1));
    if (ret < 0)
    {
        printf("CMD_SET_DEV error!\n");
        close(g_fd);
        return ret;
    }

	if (sensor_addr_byte == 2) 
	{
		buf[idx] = (addr >> 8) & 0xff;
		idx++;
		buf[idx] = addr & 0xff;
		idx++;
	} 
	else
	{
		buf[idx] = addr & 0xff;
		idx++;
	}

	if (sensor_data_byte == 2)
	{
		buf[idx] = (data >> 8) & 0xff;
		idx++;
		buf[idx] = data & 0xff;
		idx++;
	} 
	else 
	{
		buf[idx] = data & 0xff;
		idx++;
	}

	ret = write(g_fd, buf, (sensor_addr_byte + sensor_data_byte));
	if(ret < 0)
	{
		printf("I2C_WRITE error!\n");
		close(g_fd);
		return -1;
	}
 	close(g_fd);
	return 0;	
 }


int sensor_spi_init(void)
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
		 printf("ioctl SPI_IOC_WR_MODE err, value = %u ret = %d\n", value, ret);
		 return ret;
	 }
 
	 value = 8;
	 ret = ioctl(g_fd, SPI_IOC_WR_BITS_PER_WORD, &value);
	 if (ret < 0)
	 {
		 printf("ioctl SPI_IOC_WR_BITS_PER_WORD err, value = %u ret = %d\n",value, ret);
		 return ret;
	 }
 
	 value = 2000000;
	 ret = ioctl(g_fd, SPI_IOC_WR_MAX_SPEED_HZ, &value);
	 if (ret < 0)
	 {
		 printf("ioctl SPI_IOC_WR_MAX_SPEED_HZ err, value = %u ret = %d\n",value, ret);
		 return ret;
	 }
 
	 return 0;
 }


int _spi_read(unsigned int addr, unsigned short* ret_data)
 {	 
	 sensor_spi_init();
	 int ret = 0;
	 struct spi_ioc_transfer mesg[1];
	 unsigned char	tx_buf[8] = {0};
	 unsigned char	rx_buf[8] = {0};
	 
	 tx_buf[0] = (addr & 0xff00) >> 8;
	 tx_buf[0] |= 0x80;
	 tx_buf[1] = addr & 0xff;
	 tx_buf[2] = 0;
 
	 memset(mesg, 0, sizeof(mesg));
	 mesg[0].tx_buf = (__u32)tx_buf;
	 mesg[0].len	= 3;
	 mesg[0].rx_buf = (__u32)rx_buf;
	 mesg[0].cs_change = 1;
 
	 ret = ioctl(g_fd, SPI_IOC_MESSAGE(1), mesg);
	 if (ret  < 0) {  
		 printf("SPI_IOC_MESSAGE error \n");  
		 return -1;  
	 }
  //   printf("func:%s ret = %d, rx_buf = %#x, %#x, %#x\n", __func__, ret , rx_buf[0], rx_buf[1], rx_buf[2]);
  
	 printf("0x%x 0x%x\n", addr, rx_buf[2]); 
	 *ret_data = rx_buf[2];
	 printf("0x%x ret_data = 0x%x\n", addr, *ret_data); 
	 
	 return 0;
 }

  
int _spi_write(unsigned int addr, unsigned char data)
 {	 
	 sensor_spi_init();
	 struct spi_ioc_transfer mesg[1];
	 unsigned char	tx_buf[8] = {0};
	 unsigned char	rx_buf[8] = {0};
	 int ret;
	 tx_buf[0] = (addr & 0xff00) >> 8;
	 tx_buf[0] &= (~0x80);
	 tx_buf[1] = addr & 0xff;
	 tx_buf[2] = data;
 
	 memset(mesg, 0, sizeof(mesg));  
	 mesg[0].tx_buf = (__u32)tx_buf;  
	 mesg[0].len	= 3;  
	 mesg[0].rx_buf = (__u32)rx_buf; 
	 mesg[0].cs_change = 1;
 
	 ret = ioctl(g_fd, SPI_IOC_MESSAGE(1), mesg);
	 if (ret < 0) {  
		 printf("SPI_IOC_MESSAGE error \n");  
		 return -1;  
	 }
	 return 0;
 }
 

 int aptina_i2c_read(int addr, unsigned short* ret_data)
 {
	 const unsigned char sensor_i2c_addr	 =	 0x20;		 
	 const unsigned int  sensor_addr_byte	 =	 2;
	 const unsigned int  sensor_data_byte	 =	 2;

	 *ret_data = _i2c_read(addr,sensor_i2c_addr, sensor_addr_byte,sensor_data_byte);
	 return 0;

 }

 int aptina_i2c_write(int addr, int data)
 {
	 const unsigned char sensor_i2c_addr	 =	 0x20;		 
	 const unsigned int  sensor_addr_byte	 =	 2;
	 const unsigned int  sensor_data_byte	 =	 2;

	 _i2c_write(addr,data,sensor_i2c_addr,sensor_addr_byte,sensor_data_byte);

	 return 0;

 }

int ar0237_i2c_read(int addr, unsigned short* ret_data)
{
	aptina_i2c_read(addr,ret_data);
	return 0;
}

int ar0237_i2c_write(int addr, int data)
{
	aptina_i2c_write(addr,data);
	return 0;
}

 int sc_i2c_read(int addr, unsigned short* ret_data)
 {
	 const unsigned char sensor_i2c_addr	 =	 0x60;		 
	 const unsigned int  sensor_addr_byte	 =	 2;
	 const unsigned int  sensor_data_byte	 =	 1;

	 *ret_data = _i2c_read(addr,sensor_i2c_addr, sensor_addr_byte,sensor_data_byte);
	 return 0;

 }

 int sc_i2c_write(int addr, int data)
 {
	 const unsigned char sensor_i2c_addr	 =	 0x60;		 
	 const unsigned int  sensor_addr_byte	 =	 2;
	 const unsigned int  sensor_data_byte	 =	 1;

	 _i2c_write(addr,data,sensor_i2c_addr,sensor_addr_byte,sensor_data_byte);

	 return 0;

 }

int sc2235_i2c_read(int addr, unsigned short* ret_data)
{
	sc_i2c_read(addr,ret_data);
	return 0;
}

int sc2235_i2c_write(int addr, int data)

{
	sc_i2c_write(addr,data);
	return 0;
}


