#ifndef __HI_ISP_I2C_H__
#define __HI_ISP_I2C_H__

extern int _i2c_read(int addr, unsigned short* ret_data, char sensor_i2c_addr, unsigned int sensor_addr_byte,unsigned sensor_data_byte);
extern int _i2c_write(int addr,int data,char sensor_i2c_addr, unsigned int sensor_addr_byte,unsigned sensor_data_byte);
extern int _spi_read(unsigned int addr, unsigned short* ret_data);
extern int _spi_write(unsigned int addr, unsigned char data);


#endif //__HI_ISP_I2C_H__


