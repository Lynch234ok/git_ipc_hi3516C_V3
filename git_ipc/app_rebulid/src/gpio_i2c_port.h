
#ifndef __GPIO_I2C_PORT_H__
#define __GPIO_I2C_PORT_H__

typedef enum {
    GPIO_I2C_ARR_RD = 0,
    GPIO_I2C_ARR_WR,
} GPIO_I2C_IOCTRL_ENUM;

typedef struct {
    int  Grp;
    int  Dev;
    int  RSize;
    int  DSize;
    char Buf[256];
} GI2C_ARR_STRUCT;

#endif //__GPIO_I2C_PORT_H__

