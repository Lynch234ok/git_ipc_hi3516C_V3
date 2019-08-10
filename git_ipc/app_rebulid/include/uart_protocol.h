#ifndef _UART_PROTOCOL_H_
#define _UART_PROTOCOL_H_

#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <termios.h> 


#include "ptz.h"
#include "netsdk.h"


#define BUFFSIZE 1024
#define UART_NAME "/dev/ttyAMA1"
#define TRUE 1
#define FALSE 0
#define BAUDRATE 57600

typedef union _Data
{
	unsigned int dataBase;

	struct{
		unsigned char line;
		unsigned char cow;
		unsigned char string[BUFFSIZE];
		unsigned int count;
	}osd;

	struct{
		unsigned char pelcod_data[BUFFSIZE];
		unsigned char afData[4];
		unsigned char agcData;
		unsigned char color:4;
		unsigned char baud_or_multiple:4;
	}sendData;
	
	unsigned int sensor_gain;
}DATA;


enum{
 	UART_CMD_READ_BRIGHT = 0,		//亮度
	UART_CMD_READ_SHARPENESS,			//锐度
	UART_CMD_READ_SHUTTER,			//电子快门
	UART_CMD_READ_MIRROR,			//镜像
	
	UART_CMD_SET_AF,				//配置AF
	UART_CMD_SET_SATURATION,		//配置色彩数据
	UART_CMD_SET_BRIGHT,			//配置亮度
	UART_CMD_SET_SHARPENESS,		//配置锐度
	UART_CMD_SET_SHUTTER,			//配置电子快门
	UART_CMD_SET_MIRROR,			//配置镜像
	UART_CMD_SET_STANDARD_OSD,		//标准字符集
	UART_CMD_SET_OSD1,				//显示OSD数据（字符集1）
	UART_CMD_SET_OSD2,				//显示OSD数据（字符集2）
	UART_CMD_CLEAR_OSD,				//清除OSD显示
	
	UART_CMD_SEND_DATA,				//主动发送数据给MCU
	UART_CMD_SEND_DATA_BSD,		    //贝斯得主动发送数据给MCU
	
	UART_CMD_SEND_PELCOD_DATA,
	UART_CMD_SEND_DATA_HXST,          //汇讯视通发送数据协议给MCU
	UART_CMD_SEND_PELCOD_DATA_HXST,   //汇讯视通发送PELCOD数据协议给MCU
	UART_CMD_SEND_GAIN_HXST,          //汇讯视通发送增益数据协议给MCU

	UART_CMD_SEND_AF_DATA_RONGTIANSHI,        //荣天视发送AF数据协议给MCU

};

extern unsigned int _af_32Fv1;

typedef void (*fUART_CALLBACK)(int cmd, DATA* data);

extern int UART_auto_send(int cmd, DATA *data); 	//主动发数据给MCU
extern int UART_callback_init(fUART_CALLBACK function_callback);
extern int UART_callback_destory();

extern int UART_protocol_ptz_Init();
extern void UART_protocol_ptz_Destroy();
extern int UART_protocol_ptz_Config(int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr);
extern int UART_protocol_ptz_Send(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param);


extern int UART_protocol_reconfig(int nBaudRate, int nDateBit, char strParity, int nStopBit);

int ptz_speed_level_switch(int speed_level);

#endif //_UART_PROTOCOL_H_
