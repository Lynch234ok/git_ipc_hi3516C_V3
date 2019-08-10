#ifndef _LOW_H_
#define _LOW_H_

#ifdef __cplusplus
extern "C"{
#endif

#define ACK  0x00
#define NACK 0x01

typedef enum {
    GPIO_I2C_INIT  = 0,
	GPIO_I2C_START,
	GPIO_I2C_STOP,
	GPIO_I2C_SEND_BYTE,
	GPIO_I2C_RECV_BYTE,
	GPIO_I2C_SEND_ACK,
	GPIO_I2C_WAIT_ACK
} GPIO_I2C_IOCTL;

typedef unsigned char Uint8;

int  dev_Open(const char * dev);
void dev_Close(void);

void Init_Clock();

void I2C_Start(void);
void I2C_Stop(void);

void  I2C_SendByte(Uint8 c);
Uint8 I2C_ReceiveByte(void);

void  I2C_SendACK(Uint8 a);
Uint8 I2C_WaitACK(void);
void  I2C_Polling(Uint8 n);

void system_read(Uint8 addr, Uint8 n, Uint8 *data);
void system_write(Uint8 addr, Uint8 n, Uint8 *data);

void send_checksum(Uint8 *ChkSum);
void read_checksum(Uint8 *ChkSum);

void verify_read_password(Uint8 n, Uint8 * pw);
void verify_write_password(Uint8 n, Uint8 * pw);

void verify_authentication(Uint8 n, Uint8 * au);
void verify_crypto(Uint8 n, Uint8 * cr);
void Reset_Crypto(void);

void fuse_read(Uint8 *data);

#ifdef __cplusplus
}
#endif

#endif

