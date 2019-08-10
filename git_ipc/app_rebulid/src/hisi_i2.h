#ifndef _HISI_I2_H
#define _HISI_I2_H

extern int  hi_i2c_init(void);
extern int  hi_i2c_exit(void);
extern int hi_i2c_read(int addr, int reg, int * val);
extern int  hi_i2c_write(int addr, int reg, int val);
#endif
