
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "uart_protocol.h"
#include "uart.h"
#include "pan_tilt.h"
#include <sys/prctl.h>
#include "base/ja_process.h"

#if defined(UART_PROTOCOL)
static int fd  = -1;
static bool flag = false;
static fUART_CALLBACK g_function_callback; 				//�ص�����
#define IPCAM_VIN_CH 1
#define MAX_CMD_SIZE (16)


#define PTZ_CUSTON_HUIXUN    	 (0)
#define PTZ_CUSTON_BEISIDE    	 (1)
#define PTZ_CUSTON_RONGTIANSHI   (2)
#define PTZ_CUSTON_BAITEJIA      (3)

static  int _ptzCustomTpye = 0;




#define UART_PTZ_PRINT_CMD(cmd,size) \
	do{\
		int i = 0;\
		printf("\033[47;30mPTZ Command:");\
		for(i = 0; i < size; ++i){printf(" %02X", cmd[i]);}\
		printf("\033[0m\r\n");\
	}while(0);

static unsigned char check_sum(unsigned char *buf, int n);
static int write_choice(char *buf, int ret);
static int read_choice(char *buf);
static int set_com_config(int fd,int baud_rate, int data_bits, char parity, int stop_bits);
static int send_data_to_mcu(int type, DATA *data);



unsigned int _af_32Fv1 = 0;

typedef struct UART_protocol_ptz
{
	unsigned char (*IsAutoPan)(int fd, int nChn);
	int (*Config)(int fd, int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr);
	int (*Send)(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param);
}UART_PROTOCOL_PTZ_t; 

int ptz_speed_level_switch(int speed_level)
{
	int n_speed = 0;
	
	switch(speed_level){
		default:
		case 1:
			n_speed = 1.0/8*63;
		break;
		case 2:
			n_speed = 2.0/8*63;
		break;
		case 3:
			n_speed = 3.0/8*63;
		break;
		case 4:
			n_speed = 4.0/8*63;
		break;
		case 5:
			n_speed = 5.0/8*63;
		break;
		case 6:
			n_speed = 6.0/8*63;
		break;
		case 7:
			n_speed = 7.0/8*63;
		break;
		case 8:
			n_speed = 8.0/8*63;
		break;
	}

	return n_speed;
}
static int get_PelcoD_Command(unsigned char* buf, PTZ_COMMAND cmd, unsigned char addr, unsigned char arg)
{
	if(!buf){
		return -1;
	}
	buf[0] = 0xff;
	buf[1] = addr;
	switch (cmd)
	{
	case PTZ_CMD_UP:
		buf[2] =0x00;
		buf[3] = 0x08;
		buf[4] = 0x00;
		buf[5] = arg;
		break;
	case PTZ_CMD_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x10;
		buf[4] = 0x00;
		buf[5] = arg;
		break;
	case PTZ_CMD_LEFT:
		buf[2] = 0x00;
		buf[3] = 0x04;
		buf[4] = arg;
		buf[5] = 0x00;
		break;
	case PTZ_CMD_RIGHT:
		buf[2] = 0x00;
		buf[3] = 0x02;
		buf[4] = arg;
		buf[5] = 0x00;
		break;
	case PTZ_CMD_LEFT_UP:
		buf[2] = 0x00;
		buf[3] = 0x0C;
		buf[4] = arg;
		buf[5] = arg;
		break;
	case PTZ_CMD_RIGHT_UP:
		buf[2] = 0x00;
		buf[3] = 0x0A;
		buf[4] = arg;
		buf[5] = arg;
		break;
	case PTZ_CMD_LEFT_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x14;
		buf[4] = arg;
		buf[5] = arg;
		break;
	case PTZ_CMD_RIGHT_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x12;
		buf[4] = arg;
		buf[5] = arg;
		break;

	case PTZ_CMD_AUTOPAN:
		buf[2] = 0x90;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;

	case PTZ_CMD_IRIS_OPEN:
		buf[2] = 0x02;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;
	case PTZ_CMD_IRIS_CLOSE:
		buf[2] = 0x04;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;

	case PTZ_CMD_ZOOM_IN:
		buf[2] = 0x00;
		buf[3] = 0x20;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;
	case PTZ_CMD_ZOOM_OUT:
		buf[2] = 0x00;
		buf[3] = 0x40;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;

	case PTZ_CMD_FOCUS_FAR:
		buf[2] = 0x00;
		buf[3] = 0x80;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;
	case PTZ_CMD_FOCUS_NEAR:
		buf[2] = 0x01;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;

	case PTZ_CMD_STOP:
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;

	case PTZ_CMD_LIGHT_ON:
		buf[2] = 0x00;
		buf[3] = 0x09;
		buf[4] = 0x00;
		buf[5] = 0x01;
		break;
	case PTZ_CMD_LIGHT_OFF:
		buf[2] = 0x00;
		buf[3] = 0x0B;
		buf[4] = 0x00;
		buf[5] = 0x01;
		break;

	case PTZ_CMD_WIPPER_ON:
		buf[2] = 0x00;
		buf[3] = 0x09;
		buf[4] = 0x00;
		buf[5] = 0x02;
		break;
	case PTZ_CMD_WIPPER_OFF:
		buf[2] = 0x00;
		buf[3] = 0x0B;
		buf[4] = 0x00;
		buf[5] = 0x02;
		break;

	case PTZ_CMD_POWER_ON:
		buf[2] = 0x00;
		buf[3] = 0x09;
		buf[4] = 0x00;
		buf[5] = 0x04;
		break;

	case PTZ_CMD_POWER_OFF:
		buf[2] = 0x00;
		buf[3] = 0x0B;
		buf[4] = 0x00;
		buf[5] = 0x04;
		break;

	case PTZ_CMD_GOTO_PRESET:
		buf[2] = 0x00;
		buf[3] = 0x07;
		buf[4] = 0x00;
		buf[5] = arg;
		break;
	case PTZ_CMD_SET_PRESET:
		buf[2] = 0x00;
		buf[3] = 0x03;
		buf[4] = 0x00;
		buf[5] = arg;
		break;

	case PTZ_CMD_CLEAR_PRESET:
		buf[2] = 0x00;
		buf[3] = 0x05;
		buf[4] = 0x00;
		buf[5] = arg;
		break;

	default: return 0;
	}

	unsigned char t = 0;
	int i = 0;
	for(i=1; i<6; i++)
		t += buf[i];
	buf[6] = t; // check bit
	return 7;
}

static int get_PelcoP_Command(unsigned char* buf, PTZ_COMMAND cmd, unsigned char addr, unsigned char arg)
{
	char t = 0;
	int i = 0;
	if(!buf){
		return -1;
	}
	buf[0] = 0xa0;
	buf[1] = addr;

	switch (cmd)
	{
	case PTZ_CMD_UP:
		buf[2] = 0x00;
		buf[3] = 0x08;
		buf[4] = 0x00;
		buf[5] = arg;
		break;
	case PTZ_CMD_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x10;
		buf[4] = 0x00;
		buf[5] = arg;
		break;
	case PTZ_CMD_LEFT:
		buf[2] = 0x00;
		buf[3] = 0x04;
		buf[4] = arg;
		buf[5] = 0x00;
		break;
	case PTZ_CMD_RIGHT:
		buf[2] = 0x00;
		buf[3] = 0x02;
		buf[4] = arg;
		buf[5] = 0x00;
		break;
	case PTZ_CMD_LEFT_UP:
		buf[2] = 0x00;
		buf[3] = 0x0c;
		buf[4] = arg;
		buf[5] = arg;
		break;
	case PTZ_CMD_RIGHT_UP:
		buf[2] = 0x00;
		buf[3] = 0x0a;
		buf[4] = arg;
		buf[5] = arg;
		break;
	case PTZ_CMD_LEFT_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x14;
		buf[4] = arg;
		buf[5] = arg;
		break;
	case PTZ_CMD_RIGHT_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x12;
		buf[4] = arg;
		buf[5] = arg;
		break;
	case PTZ_CMD_AUTOPAN:
		buf[2] = 0x20;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;
	case PTZ_CMD_IRIS_OPEN:
		buf[2] = 0x04;
		buf[3] = 0x00;
		buf[4] = 0x30;
		buf[5] = 0x30;
		break;
	case PTZ_CMD_IRIS_CLOSE:
		buf[2] = 0x08;
		buf[3] = 0x00;
		buf[4] = 0x30;
		buf[5] = 0x30;
		break;
	case PTZ_CMD_FOCUS_FAR:
		buf[2] = 0x01;
		buf[3] = 0x00;
		buf[4] = 0x30;
		buf[5] = 0x30;
		break;
	case PTZ_CMD_FOCUS_NEAR:
		buf[2] = 0x02;
		buf[3] = 0x00;
		buf[4] = 0x30;
		buf[5] = 0x30;
		break;
	case PTZ_CMD_ZOOM_IN:
		buf[2] = 0x00;
		buf[3] = 0x20;
		buf[4] = 0x30;
		buf[5] = 0x30;
		break;
	case PTZ_CMD_ZOOM_OUT:
		buf[2] = 0x00;
		buf[3] = 0x40;
		buf[4] = 0x30;
		buf[5] = 0x30;
		break;
	case PTZ_CMD_STOP:
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		break;

	case PTZ_CMD_GOTO_PRESET:
		buf[2] = 0x00;
		buf[3] = 0x07;
		buf[4] = 0x00;
		buf[5] = arg;
		break;
	case PTZ_CMD_SET_PRESET:
		buf[2] = 0x00;
		buf[3] = 0x03;
		buf[4] = 0x00;
		buf[5] = arg;
		break;

	case PTZ_CMD_CLEAR_PRESET:
		buf[2] = 0x00;
		buf[3] = 0x05;
		buf[4] = 0x00;
		buf[5] = arg;
		break;

	case PTZ_CMD_LIGHT_ON:
		buf[2] = 0x00;
		buf[3] = 0x09;
		buf[4] = 0x00;
		buf[5] = 0x01;
		break;
	case PTZ_CMD_LIGHT_OFF:
		buf[2] = 0x00;
		buf[3] = 0x0B;
		buf[4] = 0x00;
		buf[5] = 0x01;
		break;

	case PTZ_CMD_WIPPER_ON:
		buf[2] = 0x00;
		buf[3] = 0x09;
		buf[4] = 0x00;
		buf[5] = 0x02;
		break;
	case PTZ_CMD_WIPPER_OFF:
		buf[2] = 0x00;
		buf[3] = 0x0B;
		buf[4] = 0x00;
		buf[5] = 0x02;
		break;

/*
	case PTZ_CMD_POWER_ON:
		buf[2] = 0x;
		buf[3] = 0x;
		buf[4] = 0x;
		buf[5] = 0x;
		break;
	case PTZ_CMD_POWER_OFF:
		buf[2] = 0x;
		buf[3] = 0x;
		buf[4] = 0x;
		buf[5] = 0x;
		break;
*/
	default: return 0;
	}

	buf[6] = 0xaf;

	for(i = 0; i < 7; i++){
		t ^= buf[i];
	}
	buf[7] = t; // check bit
	return 8;
}




static unsigned char ptz_IsAutoPan(int fd, int nChn)
{
	return 0;
}

static int ptz_Config(int fd, int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr)
{
	//FIXME 该接口暂不使用，参数个数需要调整
/*
	int data_bits = 8;
	char parity = 'N';
	int stop_bits = 1;
		
	if(fd != -1){
		set_com_config(fd, nBaud, data_bits, parity, stop_bits);
	}
*/
	return 0;
}

static int ptz_Send(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param)
{
	DATA data;
	int nCmdSize = 0,n_speed = 1;
	unsigned char au8Cmd[MAX_CMD_SIZE] = {0};
	unsigned char au8Addr[IPCAM_VIN_CH];

	ST_NSDK_PTZ_CFG stPtzConfig;
	memset(&stPtzConfig, 0, sizeof(stPtzConfig));
	NETSDK_conf_ptz_ch_get(&stPtzConfig);
	n_speed = ptz_speed_level_switch(stPtzConfig.stPtzExternalConfig.nSpeed);
	
	if(0 == strcmp("pelco-d",stPtzConfig.stPtzExternalConfig.strProtocolName))
	{
		if((PTZ_CMD_AUTOPAN == enCmd)  && ( 0 == strcmp(stPtzConfig.stPtzExternalConfig.strptzCustomTpye,"BEISIDE"))){
			//贝斯得自动巡航命令特殊处理，调用99号预置位
			nCmdSize = get_PelcoD_Command(au8Cmd, PTZ_CMD_GOTO_PRESET, (unsigned char)stPtzConfig.stPtzExternalConfig.nAddress,99);			
		}else if((PTZ_CMD_AUTOPAN == enCmd)  && ( 0 == strcmp(stPtzConfig.stPtzExternalConfig.strptzCustomTpye,"RONGTIANSHI"))){
			//荣天视自动巡航命令特殊处理，调用39号预置位
			nCmdSize = get_PelcoD_Command(au8Cmd, PTZ_CMD_GOTO_PRESET, (unsigned char)stPtzConfig.stPtzExternalConfig.nAddress,39);
		}else if((PTZ_CMD_GOTO_PRESET == enCmd) || (PTZ_CMD_SET_PRESET == enCmd) || (PTZ_CMD_CLEAR_PRESET == enCmd)){
			//预置位命令设置带预置点num参数
			nCmdSize = get_PelcoD_Command(au8Cmd, enCmd, (unsigned char)stPtzConfig.stPtzExternalConfig.nAddress,u8Param);
		}else{
			//普通云台设置命令带speed参数，由网页配置参数控制
			nCmdSize = get_PelcoD_Command(au8Cmd, enCmd, (unsigned char)stPtzConfig.stPtzExternalConfig.nAddress, (unsigned char)n_speed);
		}

	}else if(0 == strcmp("pelco-p",stPtzConfig.stPtzExternalConfig.strProtocolName))
	{
		if((PTZ_CMD_GOTO_PRESET == enCmd) || (PTZ_CMD_SET_PRESET == enCmd) || (PTZ_CMD_CLEAR_PRESET == enCmd)){
 			nCmdSize = get_PelcoP_Command(au8Cmd, enCmd, (unsigned char)stPtzConfig.stPtzExternalConfig.nAddress, u8Param);
		}else{
			nCmdSize = get_PelcoP_Command(au8Cmd, enCmd, (unsigned char)stPtzConfig.stPtzExternalConfig.nAddress, (unsigned char)n_speed);
		}
	}else
	{
		printf("\033[34m PTZ Protocol is Invalid \033[0m \n");
	}

	UART_PTZ_PRINT_CMD(au8Cmd, nCmdSize);
	memcpy(data.sendData.pelcod_data, au8Cmd, nCmdSize);


	if(PTZ_CUSTON_BEISIDE == _ptzCustomTpye){//贝斯得命令格式接口
		UART_auto_send(UART_CMD_SEND_PELCOD_DATA, &data);
	}else if((PTZ_CUSTON_HUIXUN == _ptzCustomTpye) || (PTZ_CUSTON_RONGTIANSHI==_ptzCustomTpye) || (PTZ_CUSTON_BAITEJIA ==_ptzCustomTpye)){
		UART_auto_send(UART_CMD_SEND_PELCOD_DATA_HXST,&data);//汇讯视通接口
	}

	
	return 0;
}


/*
unsigned char ascChar[][5] = {
		{"0"},{"1"},{"2"},{"3"},{"4"},{"5"},{"6"},{"7"},{"8"},{"9"},
		{"A"},{"B"},{"C"},{"D"},{"E"},{"F"},{"G"},{"H"},{"I"},{"J"},
		{"K"},{"L"},{"M"},{"N"},{"O"},{"P"},{"Q"},{"R"},{"S"},{"T"},
		{"U"},{"V"},{"W"},{"X"},{"Y"},{"Z"},{"!"},{"?"},{"��"},{"&"},
		{"("},{")"},{","},{"."},{":"},{";"},{"��"},{"*"},{"%"},{"+"},
		{"-"},{"X"},{"/"},{"="},{"\\"},{"\""},{"`"},{"��"},{"��"},{" "},
};
*/

//贝斯得云台控制需求字符集1
unsigned char ascChar[][5] = {
		{"0"},{"1"},{"2"},{"3"},{"4"},{"5"},{"6"},{"7"},{"8"},{"9"},
		{"A"},{"B"},{"C"},{"D"},{"E"},{"F"},{"G"},{"H"},{"I"},{"J"},
		{"K"},{"L"},{"M"},{"N"},{"O"},{"P"},{"Q"},{"R"},{"S"},{"T"},
		{"U"},{"V"},{"W"},{"X"},{"Y"},{"Z"},{"!"},{"?"},{"#"},{"&"},
		{"("},{")"},{","},{"."},{":"},{";"},{"~"},{"*"},{"%"},{"+"},
		{"-"},{"X"},{"/"},{"="},{"\\"},{"\""},{"`"},{"_"},{"@"},{" "},
};


unsigned char hzText[] = 
{
	0xB0,0xB4,0xB2,0xBF,0xB2,0xCB,0xB3,0xF6,0xB5,0xA5,0xB5,0xC8,
	0xB5,0xF7,0xB6,0xA8,0xB6,0xAF,0xB6,0xC8,

	0xB7,0xB5,0xB8,0xB4,0xB9,0xD8,0xB9,0xE2,0xBB,0xD8,0xBB,0xFA,
	0xBF,0xAA,0xBF,0xD8,0xC1,0xC1,0xC4,0xA3,

	0xC6,0xBD,0xC6,0xF4,0xC7,0xE5,0xC7,0xF8,0xC9,0xCF,0xC9,0xE3,
	0xC9,0xE8,0xCA,0xBD,0xCB,0xD9,0xCB,0xF8,

	0xCD,0xCB,0xCE,0xBB,0xCE,0xC4,0xCF,0xDF,0xCF,0xF1,0xD1,0xA1,
	0xD1,0xD4,0xD3,0xA6,0xD3,0xC3,0xD3,0xD2,

	0xD3,0xEF,0xD3,0xF2,0xD4,0xF1,0xD6,0xC3,0xD6,0xC6,0xD6,0xD0,
	0xD7,0xAA,0xD7,0xF3,0xD7,0xF7,0xC7,0xF2,

	0xC9,0xA8,0xC3,0xE8,0xD1,0xB2,0xBA,0xBD,0xD3,0xB2,0xC8,0xED,
	0xCB,0xAE,0xB4,0xB9,0xB1,0xE4,0xB1,0xB6,

	0xB4,0xFD,0xBB,0xD6,0xD6,0xD8,0xBB,0xA8,0xC7,0xD0,0xBB,0xBB,
	0xBE,0xDB,0xBD,0xB9,0xB2,0xA8,0xCC,0xD8,

	0xC2,0xCA,0xC8,0xAB,0xB2,0xBB,0xCA,0xBC,0xD0,0xAD,0xD2,0xE9,
	0xB5,0xD8,0xD6,0xB7,0xCF,0xD0,0xB3,0xFD,

	0xC2,0xB7,0xD0,0xAA,0xB0,0xE6,0xB3,0xA7,0xCA,0xB1,0xBC,0xE4,
	0xB1,0xED,0xB1,0xBE,0xD6,0xF7,0xD4,0xA4,

	0xBA,0xEC,0xCD,0xE2,0xCF,0xD4,0xCA,0xBE,0xB7,0xBD,0xB1,0xD5,
	0xCF,0xD6,0xBA,0xDA,0xD2,0xB9,0xC7,0xBF,

	0xD7,0xD4,0xD6,0xB1,0xB0,0xB5,0xD0,0xC5,0xCF,0xA2,0xB6,0xD4,
	0xD3,0xEB,0xB5,0xC0,0xCA,0xE4,0xC8,0xEB,

	0xBF,0xED,0xCC,0xAC,0xC6,0xE4,0xBB,0xAD,0xC3,0xC6,0xC7,0xD0,
	0xC8,0xD5,0xCA,0xD6,0xC8,0xF1,0xBE,0xB5,

	0xB1,0xB1,0xB5,0xE3,0xCA,0xFD,0xD7,0xD6,0xBD,0xB5,0xD4,0xEB,
	0xD0,0xD0,0xD5,0xFD,0xD2,0xFE,0xCB,0xBD,

	0xD5,0xDA,0xB1,0xCE,0xD0,0xCD,0xCE,0xC2,0xB5,0xE7,0xD7,0xD3,
	0xBF,0xEC,0xC3,0xC5,0xB2,0xCA,0xC9,0xAB,

	0xB0,0xD7,0xC8,0xA6,0xB1,0xE0,0xBC,0xAD,0xBA,0xC5,0xC0,0xC0,
	0xBC,0xEC,0xD0,0xA3,0xD7,0xBC,0xD0,0xBE,

	0xCA,0xB6,0xB1,0xF0,0xCF,0xB5,0xCD,0xB3,0xC5,0xE4,0xCD,0xA8,
	0xD1,0xB6,0xBC,0xFE,0xCA,0xB9,0xC4,0xDC,

	
	0xC4,0xDA,0xC3,0xEB,0xB1,0xA8,0xBE,0xAF,0xD4,0xCB,0xB9,0xEC,
	0xBC,0xA3,0xCF,0xDE,0xD6,0xB5,0xB5,0xB1,

	0xC7,0xB0,0xD7,0xB4,0xC6,0xC1,0xC4,0xBB,0xCC,0xE1,0xB7,0xAD,
	0xB3,0xA4,0xCA,0xD8,0xCD,0xFB,0xD6,0xB4,

	0xCD,0xA3,0xD6,0xB9,0xCF,0xC2,0xD5,0xFB,0xCD,0xB7,0xC1,0xAA,
	0xB3,0xD6,0xD0,0xF8,0xC8,0xB7,0xC8,0xCF,

	0xB5,0xC6,0xC0,0xE0,0xA1,0xF1,0xA1,0xF0,0xA1,0xFA
};


/*串口的基本配置命令*/
static int set_com_config(int fd,int baud_rate, int data_bits, char parity, int stop_bits)
{
	struct termios new_cfg,old_cfg;
	int speed;

	/*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
	if  (tcgetattr(fd, &old_cfg)  !=  0) {
		perror("tcgetattr");
		return -1;
	}
	
	/*设置字符大小*/
	new_cfg = old_cfg;
	cfmakeraw(&new_cfg);
	new_cfg.c_cflag &= ~CSIZE;
	
	/*CLOCAL和CREAD分别用于本地连接和接受使能，因此，首先要通过位掩码的方式激活这两个选项*/    
	new_cfg.c_cflag |= CLOCAL | CREAD;
  	/*设置波特率*/
  	switch (baud_rate)
  	{ 		
  		case 110:
  		{
			speed = B110;
		}
		break;

		case 300:
		{
			speed = B300;
		}
		break;

		case 600:
		{
			speed = B600;
		}
		break;

		case 1200:
		{
			speed = B1200;
		}
  		case 2400:
		{
			speed = B2400;
		}
		break;

  		case 4800:
		{
			speed = B4800;
		}
		break;

  		case 9600:
		{
			speed = B9600;
		}
		break;
  		
		case 19200:
		{
			speed = B19200;
		}
		break;

  		case 57600:
		{
			speed = B57600;
		}
		break;
		
		default:
		case 115200:
		{
			speed = B115200;
		}
		break;
  	}
	cfsetispeed(&new_cfg, speed);
	cfsetospeed(&new_cfg, speed);

	/*设置停止位*/
	switch (data_bits)
	{
		case 7:
		{
			new_cfg.c_cflag |= CS7;
		}
		break;
		
		default:
		case 8:
		{
			new_cfg.c_cflag |= CS8;
		}
		break;
  	}
  	
  	/*设置奇偶校验位*/
  	switch (parity)
  	{
		default:
		case 'n':
		case 'N':
		{
			new_cfg.c_cflag &= ~PARENB;   
			new_cfg.c_iflag &= ~INPCK;    
		}
		break;

        case 'o':
		case 'O':
        {
            new_cfg.c_cflag |= (PARODD | PARENB);  
            new_cfg.c_iflag |= INPCK;             
        }
        break;

		case 'e':
        case 'E':
		{
			new_cfg.c_cflag |= PARENB;     
			new_cfg.c_cflag &= ~PARODD;   
			new_cfg.c_iflag |= INPCK;     
        }
		break;

        case 's':  /*as no parity*/
		case 'S':
        {
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_cflag &= ~CSTOPB;
		}
		break;
	}
		
	/*设置停止位*/
	switch (stop_bits)
	{
		default:
		case 1:
		{
			new_cfg.c_cflag &=  ~CSTOPB;
		}
		break;

		case 2:
		{
			new_cfg.c_cflag |= CSTOPB;
		}
	}
	
	/*设置等待时间和最小接收字符�*/
	new_cfg.c_cc[VTIME]  = 10;
	new_cfg.c_cc[VMIN] = 0;
	
	/*处理未接收字符*/
	tcflush(fd, TCIFLUSH);
	
	/*激活新配置*/
	if((tcsetattr(fd, TCSANOW, &new_cfg)) != 0){
		perror("tcsetattr");
		return -1;
	}
	
	return 0;
}	

/*校验和*/
static unsigned char check_sum(unsigned char *buf, int n)
{
	int i;
	unsigned char result = 0;
	
	for (i=0; i<n; i++) {
		result += buf[i];
	}
	return (result&0xff);
}

/*校验和，头字节不参与校验*/
static unsigned char check_sum_2(unsigned char *buf, int n)
{
	int i;
	unsigned char result = 0;
	
	for (i=1; i<n; i++) {
		result += buf[i];
	}
	return (result&0xff);
}



/* 线程用来监听串口发送过来的数据进行判断*/
void *listen_thread(void *arg)
{
	int ret;
	unsigned char RBuf[BUFFSIZE];
	fd_set fs_read;
	struct timeval time;

	
	prctl(PR_SET_NAME, "UART_listen_thread");
	
	//�̷߳���
	pthread_detach(pthread_self());
	sleep(3);
	
	while (flag)
	{
		FD_ZERO(&fs_read);
		FD_SET(fd, &fs_read);
		memset(RBuf, 0, sizeof(RBuf));
		
		time.tv_sec = 5;
		time.tv_usec = 0;
		
		ret = select(fd+1, &fs_read, NULL, NULL, &time);

		switch(ret){
		
		case -1:
			perror("read error\n");
			break;
				
		case 0:
		//	printf("read DATA timed out.\n");
			break;	
				
		default:
			ret = read(fd, RBuf, BUFFSIZE);
			if (ret < 0){
				perror("read light failed");
				break;
			}
			
			if(PTZ_CUSTON_BEISIDE == _ptzCustomTpye){//贝斯得命令格式接口
				int j = 0;
				int cmd_len = 0;			
				
				while(j < ret){
					//判断起始3个字节是否为起始字符
					if (RBuf[j]==0x51 && RBuf[j+1]==0x01 && RBuf[j+2]==0x04) {
						
						cmd_len = 6 + RBuf[j+ 5] + 1;  //数据长度 RBuf[j+ 5]	
						//printf("sum = 0x%x\n",check_sum((RBuf + j), cmd_len-1));
						if (RBuf[j + cmd_len-1] != check_sum((RBuf + j), cmd_len-1))
						{
							j++;
							continue;
						}
						
						if (RBuf[j + 3] == 0x79){								//接收到读命令
							read_choice(RBuf + j);
						}else if(RBuf[j + 3] == 0x78){							//接收到写命令					
							write_choice(RBuf + j, cmd_len);
						}else{
							j ++;			
							continue;
						}
						j = j + cmd_len ;
						
					}else{
						j++;
					}
				}

			}else if((PTZ_CUSTON_HUIXUN == _ptzCustomTpye) || (PTZ_CUSTON_RONGTIANSHI == _ptzCustomTpye) || (PTZ_CUSTON_BAITEJIA == _ptzCustomTpye)){//汇讯视通、荣天视、百特嘉接口
				//汇讯视通方案协议判断命令请求
				if (RBuf[0]==0xff && RBuf[1]==0x01 && RBuf[2]==0x60 
					&& RBuf[3] == 0x01 && RBuf[4] == 0X55 && RBuf[5] == 0xaa && RBuf[6] == 0x61) {
	//					printf("\033[34m _af_32Fv1 = 0x%x  \033[0m	\n",_af_32Fv1);
	
						DATA data;
						data.sendData.afData[0] = _af_32Fv1 & 0xff;
						data.sendData.afData[1] = (_af_32Fv1 >> 8) & 0xff;
						data.sendData.agcData = 128;
						UART_auto_send(UART_CMD_SEND_DATA_HXST,&data);
				}else if(RBuf[0]==0xff && RBuf[1]==0x01 && RBuf[2]==0x70 
					&& RBuf[3] == 0x01 && RBuf[4] == 0X01 && RBuf[5] == 0x06 && RBuf[6] == 0x79){
						DATA data;
						data.sensor_gain = SENSOR_get_gain();
	//					printf("\033[34m data.sensor_gain = 0x%x	\033[0m  \n ",data.sensor_gain);
						UART_auto_send(UART_CMD_SEND_GAIN_HXST,&data);
				}

			}else{

			}

			break;
		
		}
	}
	
}

/*初始化数据获取接口回调*/
int UART_callback_init(fUART_CALLBACK function_callback)
{
	g_function_callback = function_callback;
	return 0;
}

/*释放数据获取接口回调*/
int UART_callback_destory()
{
	g_function_callback = NULL;
	return 0;
}

/*读命令时选择读取相应的数据*/
static int read_choice(char *buf)
{
	DATA data;
	int cmd;

	memset(&data, 0 , sizeof(DATA));
	
	//接收的数据长度必须为 1 个字节
	if (buf[5] != 0x01){
		printf("error size \n");
		return -1;
	}
//	printf("%#x \n", check_sum(buf, 7));
	//判断校验和是否与用户输入的检验和相等，若相等，程序继续往下进行，若不相等，则退出此函数 
	if (buf[7] != check_sum(buf, 7)){
		
		printf("check_sum fail \n");
		return -1;
	}
	
	switch(buf[4]) {
	
	case 0x12:							//读取亮度请求
		cmd = UART_CMD_READ_BRIGHT;
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
			
		send_data_to_mcu(buf[4], &data);
		break;
			
	case 0x13:							//读取锐度请求
		cmd = UART_CMD_READ_SHARPENESS;
		if(g_function_callback) {
			g_function_callback(cmd, &data);
			}
			
		send_data_to_mcu(buf[4], &data);
		break;
			
	case 0x14:							//快门数据
		cmd = UART_CMD_READ_SHUTTER;
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
			
		send_data_to_mcu(buf[4], &data);
		break;
			
	case 0x15:							//镜像数据
		cmd = UART_CMD_READ_MIRROR;
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
			
		send_data_to_mcu(buf[4], &data);
		break;
			
	default:
		perror("Invalid data type");
		break;
	}
	return 0;
}

/*写命令时选择，配置IPC参数*/
static int write_choice(char *buf, int ret)
{
	DATA data;
	int cmd;
	int len;
	int cnt;
	int i, j;
	len = ret;
	
	memset(&data, 0 , sizeof(DATA));
	
//	printf("%#x \n", check_sum(buf, len-1));
	//判断校验和是否与用户输入的检验和相等，若相等，程序继续往下进行，若不相等，则退出此函数
	if (buf[len-1] != check_sum(buf, len-1))
	{
		printf("check_sum fail \n");
		return -1;
	}
	switch(buf[4]) {
	
	case 0x01:
		cmd = UART_CMD_SET_OSD1;			//1IPC的显示OSD数据（字符集1）
		data.osd.line = buf[6];				//行
		data.osd.cow  = buf[7];				//列
		len = len - 9;	    				//计算字符串所占的字节
		
		if ((buf[5]-2) == len)				//判断用户输入的字节是否与字符串的字节数相等
		{
			memset(data.osd.string, 0, sizeof(data.osd.string));
			//memcpy(data.osd.string, &buf[8], len);
			
			for(cnt=0, i=8, j=0; cnt<len; cnt++){
				
				if(buf[i] > 59){
					buf[i] = 59;
				}
				
				if (buf[i] <= 59){
					memcpy(&data.osd.string[j],ascChar[buf[i]] , strlen(ascChar[buf[i]])+1);
					j += strlen(ascChar[buf[i]]);
					i++;
				}
				else{
	
					memset(data.osd.string, 0, sizeof(data.osd.string));
				}
			}
			if(g_function_callback) {
					g_function_callback(cmd, &data);
			}
			break;
		}	
		printf("error data size\n");
		break;
		
	case 0x02:								//2IPC的清除OSD显示
		cmd = UART_CMD_CLEAR_OSD;
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
		break;
		
	case 0x03:								//3配置IPC的AF参考值		
		cmd = UART_CMD_SET_AF;
		data.dataBase = buf[6];
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
		break;
			
	case 0x11:								//4配置IPC的色彩数据
		cmd = UART_CMD_SET_SATURATION;
		data.dataBase = buf[6];
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
		break;
			
	case 0x12:								//5 设置IPC的亮度数据
		cmd = UART_CMD_SET_BRIGHT;
		data.dataBase = buf[6];
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
		break;
		
	case 0x13:								//6设置IPC的锐度数据
		cmd = UART_CMD_SET_SHARPENESS;
		data.dataBase = buf[6];
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
		break;
			
	case 0x14:								//7 设置IPC的电子快门数据
		cmd = UART_CMD_SET_SHUTTER;
		data.dataBase = buf[6];
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
		break;
		
	case 0x15:								//8 设置IPC的镜像数据
		cmd = UART_CMD_SET_MIRROR;
		data.dataBase = buf[6];
		if(g_function_callback) {
			g_function_callback(cmd, &data);
		}
		break;
		
	case 0x21:								//9 IPC的显示OSD数据（字符集2）
		cmd = UART_CMD_SET_OSD2;
		data.osd.line = buf[6];				//行
		data.osd.cow  = buf[7];				//列
		len = len - 9;
		if ((buf[5]-2) == len)
		{
			memset(data.osd.string, 0, sizeof(data.osd.string));
			//memcpy(data.osd.string, &buf[8], len);
			for(cnt=0, i=8, j=0; cnt<len; cnt++){
				
				memcpy(&data.osd.string[j], &hzText[buf[i]*2], 2);
				j += 2;
				i++;
			}
			if(g_function_callback) {
				g_function_callback(cmd, &data);
			}
			break;
		}	
		printf("error data size\n");
		break;
		
	case 0x31:
		cmd = UART_CMD_SET_STANDARD_OSD;	//10 标准字符集
		data.osd.line = buf[6];				//行
		data.osd.cow  = buf[7];				//列
		len = len - 9;
		/*data.osd.line = buf[6]<<8 | buf[7];
		data.osd.cow  = buf[8]<<8 | buf[9];
		len = len - 11;*/

		if ((buf[5]-2) == len)
		{
			memset(data.osd.string, 0, sizeof(data.osd.string));
			memcpy(data.osd.string, &buf[8], len);
			data.osd.count = len;

			if(g_function_callback) {
				g_function_callback(cmd, &data);
				}
			break;
		}	
		printf("error data size\n");
		break;
		
	default:
		perror("Invalid data type");
		break;
	}

	return 0;
}

//将读取的数据发送给MCU
static int send_data_to_mcu(int type, DATA *data)
{
	unsigned char WBuf[9];
	memset(WBuf, 0, sizeof(WBuf));
	
	WBuf[0]  = 0x51;
	WBuf[1]  = 0x01;
	WBuf[2]  = 0x04; 	//起始字符为3个字节
	WBuf[3]  = 0x79; 	//读命令
	WBuf[4]  = type; 	//数据类型
	WBuf[5]  = 0x01; 	//数据长度
	WBuf[6]  = data->dataBase;
	WBuf[7]  = check_sum(WBuf, 7);

	if(fd>0){
		write(fd, WBuf, 8);
	}
	
	return 0;
}

/*主动发IPC数据的接口*/
int UART_auto_send(int cmd, DATA *data)
{
	if(fd <= 0){
		return -1;
	}
	
	unsigned char WBuf[20];
	memset(WBuf, 0, sizeof(WBuf));
	int ret;

	if (cmd == UART_CMD_SEND_DATA) {		//1 获取IPC的AF数据，AGC的数据，色彩，波特率
		WBuf[0]  = 0x51;
		WBuf[1]  = 0x01;
		WBuf[2]  = 0x04; 	//起始字符为3个字节
		WBuf[3]  = 0x79; 	//读命令
		WBuf[4]  = 0x19; 	//数据类型
		WBuf[5]  = 0x06; 	//数据长度
		WBuf[6]  = data->sendData.afData[0];
		WBuf[7]  = data->sendData.afData[1];
		WBuf[8]  = data->sendData.afData[2];
		WBuf[9]  = data->sendData.afData[3];
		WBuf[10] = data->sendData.agcData;
		WBuf[11] = (data->sendData.color)<<4 | (data->sendData.baud_or_multiple);
		WBuf[12] = check_sum(WBuf, 12);	
		ret = write(fd, WBuf, 13);
		
		if(ret < 0){
			printf("\033[33m   write error  \033[0m  \n");
		}
	}else if(cmd == UART_CMD_SEND_DATA_BSD){
		WBuf[0]  = 0x51;
		WBuf[1]  = 0x01;
		WBuf[2]  = 0x04;	//起始字符为3个字节
		WBuf[3]  = 0x79;	//读命令
		WBuf[4]  = 0x19;	//数据类型
		WBuf[5]  = 0x06;	//数据长度
		WBuf[6]  = data->sendData.afData[0];
		WBuf[7]  = data->sendData.afData[1];
		WBuf[8]  = data->sendData.afData[2];
		WBuf[9]  = data->sendData.afData[3];
		WBuf[10] = data->sendData.agcData;
		WBuf[11] = data->sendData.color;
		WBuf[12] = data->sendData.baud_or_multiple;
		
		WBuf[13] = check_sum(WBuf, 13); 
		ret = write(fd, WBuf, 14);
		
		if(ret < 0){
			printf("\033[33m   write error	\033[0m  \n");
		}
		
	}else if (cmd == UART_CMD_SEND_PELCOD_DATA){
		WBuf[0]  = 0x51;
		WBuf[1]  = 0x01;
		WBuf[2]  = 0x04;
		WBuf[3]  = 0x79; 
		WBuf[4]  = 0x18;
		WBuf[5]  = 0x07;
		WBuf[6]  = data->sendData.pelcod_data[0];
		WBuf[7]  = data->sendData.pelcod_data[1];
		WBuf[8]  = data->sendData.pelcod_data[2];
		WBuf[9]  = data->sendData.pelcod_data[3];
		WBuf[10] = data->sendData.pelcod_data[4];
		WBuf[11] = data->sendData.pelcod_data[5];
		WBuf[12] = data->sendData.pelcod_data[6];
		WBuf[13] = check_sum(WBuf, 13);
		ret = write(fd, WBuf, 14);

		if(ret < 0){
			printf("\033[33m   write error  \033[0m  \n");
		}
		
	}else if(cmd == UART_CMD_SEND_DATA_HXST){//汇讯视通发送云台控制数据

		WBuf[0]  = 0xFF;
		WBuf[1]  = 0x01;
		WBuf[2]  = 0x60;
		WBuf[3]  = 0x01;
		WBuf[4]  = data->sendData.afData[0];
		WBuf[5]  = data->sendData.afData[1];
		WBuf[6]  = check_sum_2(WBuf, 6);
		
		ret = write(fd, WBuf, 7);
		
		if(ret < 0){
			printf("\033[33m   write error	\033[0m  \n");
		}
	}else if(cmd == UART_CMD_SEND_PELCOD_DATA_HXST){//汇讯视通发送云台控制数据

		WBuf[0]  = data->sendData.pelcod_data[0];
		WBuf[1]  = data->sendData.pelcod_data[1];
		WBuf[2]  = data->sendData.pelcod_data[2];
		WBuf[3]  = data->sendData.pelcod_data[3];
		WBuf[4]  = data->sendData.pelcod_data[4];
		WBuf[5]  = data->sendData.pelcod_data[5];
		WBuf[6]  = data->sendData.pelcod_data[6];
		
		ret = write(fd, WBuf, 7);
		
		if(ret < 0){
			printf("\033[33m   write error	\033[0m  \n");
		}
	}else if(cmd == UART_CMD_SEND_GAIN_HXST){//汇讯视通发送增益数据
		WBuf[0]  = 0xff;
		WBuf[1]  = 0x01;
		WBuf[2]  = 0x70;
		WBuf[3]  = 0x01;
		WBuf[4]  = data->sensor_gain & 0xff;
		WBuf[5]  = (data->sensor_gain >> 8)& 0xff;
		WBuf[6]  = check_sum_2(WBuf, 6);
		
		ret = write(fd, WBuf, 7);
		
		if(ret < 0){
			printf("\033[33m   write error	\033[0m  \n");
		}

	}else if(cmd == UART_CMD_SEND_AF_DATA_RONGTIANSHI){ 
		//荣天视云台AF变焦数据发送命令格式
		WBuf[0]  = 0xff;
		WBuf[1]  = 0xAF;
		WBuf[2]  = data->sendData.afData[0];
		WBuf[3]  = data->sendData.afData[1];
		WBuf[4]  = data->sendData.color;
		WBuf[5]  = 0;
		WBuf[6]  = check_sum_2(WBuf, 6);

		ret = write(fd, WBuf, 7);

		if(ret < 0){
			printf("\033[33m   write error  \033[0m  \n");
		}
	}
	
	return 0;
}


static UART_PROTOCOL_PTZ_t s_stUART_PROTOCOL_PTZ =
{
	.IsAutoPan = ptz_IsAutoPan,
	.Config = ptz_Config,
	.Send = ptz_Send,
};

static int ptz_preset_contrl(unsigned char u8Param)
{
	int reVal = 1;
#if defined(STEPER_AF)
	switch(u8Param)
	{
		case 84:
			_PstepMotor->isReset = true;
			break;
			
		default:
			reVal = -1;
			break;
	}
#endif	
	return reVal;
}

static int motor_control(PTZ_COMMAND enCmd, unsigned char u8Param)
{
    if(enCmd == PTZ_CMD_FOCUS_FAR || enCmd == PTZ_CMD_FOCUS_NEAR)
    {
        APP_MOTOR_StartAutoFocus();
    }
    else if(enCmd == PTZ_CMD_ZOOM_IN)
    {
        APP_MOTOR_StartZoom(0);
    }
    else if(enCmd == PTZ_CMD_ZOOM_OUT)
    {
        APP_MOTOR_StartZoom(1);
    }
    else if(enCmd == PTZ_CMD_STOP)
    {
        APP_MOTOR_StopZoom();
    }
    else if(enCmd == PTZ_CMD_SET_PRESET)
    {
        if(u8Param == 0)
        {
            //设置预置位0不做任何操作
            return 0;
        }
        APP_MOTOR_SetPreset(u8Param);
    }
    else if(enCmd == PTZ_CMD_GOTO_PRESET)
    {
        if(u8Param == 0)
        {
            //调用预置位0清除所以预置位
            APP_MOTOR_ClearAllPreset();
            return 0;
        }
        APP_MOTOR_GotoPreset(u8Param);
    }
    else if (enCmd == PTZ_CMD_CLEAR_PRESET)
    {
        if(u8Param == 0)
        {
            //清除预置位0不做任何操作
            return 0;
        }
        APP_MOTOR_ClearPreset(u8Param);
    }

    return 0;
}

static int focus_contrl(PTZ_COMMAND enCmd, unsigned char u8Param)
{
	static JA_Boolean focusFlag = false, zoomFlag = false;
#if defined(STEPER_AF)	
	if(!_PstepMotor)
	{
		printf("focus_contrl Error: _PstepMotor is NULL !! \n");
		return -1;
	}

	if(_PstepMotor->isInit == true
		|| _PstepMotor->goTarget == true
		|| _PstepMotor->FocusGoTo == true) 
	{
		return 0;
	}
	
	switch(enCmd)
	{
		case PTZ_CMD_ZOOM_IN:
			zoomFlag = true;
			_PstepMotor->pan(true, false, false, ZOOM_SPEED_FAST);
			break;

			
		case PTZ_CMD_ZOOM_OUT:
			zoomFlag = true;
			_PstepMotor->pan(true, true, false, ZOOM_SPEED_FAST);
			break;
			
		case PTZ_CMD_FOCUS_FAR:
			focusFlag = true;
			_PstepMotor->manual = true;
			_PstepMotor->tilt(JA_True, JA_True, false, FOCUS_SPEED_MANUAL);
			break;

		case PTZ_CMD_FOCUS_NEAR:
			focusFlag = true;
			_PstepMotor->manual = true;
			_PstepMotor->tilt(JA_True, JA_False, false, FOCUS_SPEED_MANUAL);
			break;
/*
		case PTZ_CMD_UP:
			_PstepMotor->getStep(&pan_step, &tilt_step);
			printf("##### zoomStep = %d\n", pan_step);
			printf("##### focusStep = %d\n", tilt_step);
			break;

		case PTZ_CMD_DOWN:
			ptz_preset_contrl(84);
			//zoomFlag = true;
			//_PstepMotor->testCamera(true, 0, true, ZOOM_SPEED);
			break;
*/
		case PTZ_CMD_STOP:
			if(focusFlag == true)
			{
				_PstepMotor->stopTilt();
				focusFlag = false;
			}
			if(zoomFlag == true)
			{
				_PstepMotor->stopPan();
				zoomFlag = false;
				_PstepMotor->manual = false;
			}
			break;
			
		case PTZ_CMD_GOTO_PRESET:
			ptz_preset_contrl(u8Param);
			break;

		case PTZ_CMD_AUTOPAN:
			ptz_preset_contrl(84);
			break;
			
		default:
			break;
	}
#endif
	return 0;
}


int UART_protocol_reconfig(int nBaudRate, int nDateBit, char strParity, int nStopBit)
{
	if(fd>0){
		if(nBaudRate == 0)
		{
			return -1;
		}

		ST_NSDK_PTZ_CFG stPtz;
		memset(&stPtz, 0, sizeof(ST_NSDK_PTZ_CFG));
		NETSDK_conf_ptz_ch_get(&stPtz);

		if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"BEISIDE")){//贝斯得发送AF变焦数据接口
			_ptzCustomTpye = PTZ_CUSTON_BEISIDE;

		}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"HUIXUN")){//汇讯视通接口
			_ptzCustomTpye = PTZ_CUSTON_HUIXUN;

		}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"RONGTIANSHI")){	
			_ptzCustomTpye = PTZ_CUSTON_RONGTIANSHI;

		}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"BAITEJIA")){
			_ptzCustomTpye = PTZ_CUSTON_BAITEJIA;

		}else{	
			_ptzCustomTpye = PTZ_CUSTON_HUIXUN;
		}
		
		/* 设置串口配置属性 */
		if(set_com_config(fd, nBaudRate,nDateBit,strParity,nStopBit)){
			perror("set_com_config fail");
			return -1;
		}
		
		return 0;

	}
	return -1;
}




int UART_protocol_ptz_Init()
{
	ST_NSDK_PTZ_CFG stPtz;
	memset(&stPtz, 0, sizeof(ST_NSDK_PTZ_CFG));
	NETSDK_conf_ptz_ch_get(&stPtz);

	if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"BEISIDE")){//贝斯得发送AF变焦数据接口
		_ptzCustomTpye = PTZ_CUSTON_BEISIDE;

	}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"HUIXUN")){//汇讯视通接口
		_ptzCustomTpye = PTZ_CUSTON_HUIXUN;

	}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"RONGTIANSHI")){	
		_ptzCustomTpye = PTZ_CUSTON_RONGTIANSHI;

	}else if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"BAITEJIA")){
		_ptzCustomTpye = PTZ_CUSTON_BAITEJIA;

	}else{	
		_ptzCustomTpye = PTZ_CUSTON_HUIXUN;
	}


	int err,nRet;
	pthread_t tid;
	pthread_attr_t pthread_attr;
	
#if defined(HI3518E_V2) //18Ev200
	//NK_SYSTEM("himm 0x200f00cc 0x3");
	//NK_SYSTEM("himm 0x200f00d0 0x3");
	
#elif defined(HI3516E_V1) //16Ev100
	NK_SYSTEM("himm 0x12040088 0x3");// himm：直接向寄存器中写入值
	NK_SYSTEM("himm 0x12040090 0x3");

#elif defined(HI3516D) //16DV100
    NK_SYSTEM("himm 0x200F007C 0x1");
    NK_SYSTEM("himm 0x200F0084 0x1");
#endif

 	sleep(1);

	/* 1 打开设备文件 */
	fd = open(UART_NAME, O_RDWR|O_NOCTTY|O_NDELAY); // 设备文件名："/dev/ttyAMA1"
	if (fd == -1) {
		perror("open serial fail ");
	}
	/* 设置配置属性 */
	if(set_com_config(fd, BAUDRATE, 8, 'N', 1) < 0) {
		perror("set_com_config fail" );
		return -1;
	}
	 	
	flag = true;
	/* 启动监听函数 */

	
	nRet = pthread_attr_init(&pthread_attr);

	if(0 == nRet)
    {
        pthread_attr_setstacksize(&pthread_attr, 131072);
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);

        err = pthread_create(&tid, &pthread_attr, listen_thread, NULL);

        if (err != 0) {
            printf("pthread_create error: %s \n", strerror(err));
        }

        pthread_attr_destroy(&pthread_attr);
    }

	return 0;
}

void UART_protocol_ptz_Destroy()
{
	flag = false;
	
	if (fd > 0){
		close(fd);
		fd = -1;
	}

	g_function_callback = NULL;
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
	
	return 0;
}
int UART_protocol_ptz_Config(int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr)
{	
	if(fd > 0){
		int channel;
		channel = nChn < IPCAM_VIN_CH ? nChn : IPCAM_VIN_CH;
		s_stUART_PROTOCOL_PTZ.Config(fd, channel, szProtocol, nBaud, u8Addr);
		
		return 0;
	}
	return -1;
}


int UART_protocol_ptz_Send(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param)
{
	int channel;
	channel = nChn < IPCAM_VIN_CH ? nChn : IPCAM_VIN_CH;
	s_stUART_PROTOCOL_PTZ.Send(channel, enCmd, u8Param);
	focus_contrl(enCmd, u8Param);
    motor_control(enCmd, u8Param);
	return 0;
}


#endif




