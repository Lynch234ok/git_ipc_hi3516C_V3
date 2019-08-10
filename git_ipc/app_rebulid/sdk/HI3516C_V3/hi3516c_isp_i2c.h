#ifndef __HI3516C_ISP_I2C_H__
#define __HI3516C_ISP_I2C_H__

extern int _i2c_read(int addr,char sensor_i2c_addr, unsigned int sensor_addr_byte,unsigned sensor_data_byte);
extern int _i2c_write(int addr,int data,char sensor_i2c_addr, unsigned int sensor_addr_byte,unsigned sensor_data_byte);
extern int sensor_spi_init(void);
extern int _spi_read(unsigned int addr, unsigned short* ret_data);
extern int _spi_write(unsigned int addr, unsigned char data);
extern int aptina_i2c_read(int addr, unsigned short* ret_data);
extern int aptina_i2c_write(int addr, int data);

extern int ar0237_i2c_read(int addr, unsigned short* ret_data);
extern int ar0237_i2c_write(int addr, int data);

extern int sc_i2c_read(int addr, unsigned short* ret_data);
extern int sc_i2c_write(int addr, int data);

extern int sc2235_i2c_read(int addr, unsigned short* ret_data);
extern int sc2235_i2c_write(int addr, int data);


#endif //__HI3516C_ISP_I2C_H__

