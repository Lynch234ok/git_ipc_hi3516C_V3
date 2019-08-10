#ifndef GIT_IPC_GSENSOR_HI_I2C_H
#define GIT_IPC_GSENSOR_HI_I2C_H


#include <stdint.h>

extern int gsensor_hi_i2c_init(void);
extern int gsensor_hi_i2c_exit(void);
extern int gsensor_hi_i2c_read(uint8_t sub_addr, uint8_t *val);
extern int gsensor_hi_i2c_write(uint8_t sub_addr, uint8_t val);


#endif //GIT_IPC_GSENSOR_HI_I2C_H
