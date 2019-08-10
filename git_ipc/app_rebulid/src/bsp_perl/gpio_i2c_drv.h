
#ifndef __GPIO_I2C_H__
#define __GPIO_I2C_H__

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

#ifdef __KERNEL__

extern unsigned char gpio_i2c_check(unsigned char dev);
extern int  gpio_i2c_lines(void);

//Read From [dev] for [darr] buffer at [rarr] address
extern int gpio_i2c_rd_arr(int Grp, unsigned char dev,
    unsigned char rarr[], int rsize,
    unsigned char darr[], int dsize);
//Write To [dev] for [darr] buffer at [rarr] address
extern int gpio_i2c_wr_arr(int Grp, unsigned char dev,
    unsigned char rarr[], int rsize,
    unsigned char darr[], int dsize);

extern int gpio_i2c_read(unsigned char devaddress, unsigned char address);
extern int gpio_i2c_write(unsigned char devaddress, unsigned char address, unsigned char data);
extern int gpio_i2c_probe(unsigned char devaddress);

extern int gpio_i2c_readX (int Grp, unsigned char devaddress, unsigned char address);
extern int gpio_i2c_writeX(int Grp, unsigned char devaddress, unsigned char address, unsigned char data);
extern int gpio_i2c_probeX(int Grp, unsigned char devaddress);

int gpio_i2c_readX_KeyPad_Fix(
    int Grp,
    unsigned char Dev,
    void (*Func[2])(void));

int gpio_i2c_writeX_KeyPad_Fix(
    int Grp,
    unsigned char Dev,
    unsigned char rarr[], int rsize,
    unsigned char darr[], int dsize,
    void (*Func[2])(void));

#endif //__KERNEL__

#endif //__GPIO_I2C_H__

