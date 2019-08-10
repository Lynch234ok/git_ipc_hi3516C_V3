
#include <string.h>
#include "ptz.h"
#include "uart_ptz.h"


// set ptz speed, according cmd speed level
static unsigned char cmd_speed(unsigned int level)
{
	switch(level)
	{
	case 1: return 0x10; //lower speed
	case 2: return 0x1a; //low speed
	default:
	case 3: return 0x24; //normal speed
	case 4: return 0x30; //high speed
	case 5: return 0x3c; //higher speed
	}
	return 0x24;
}

static void get_name(char* ret_name)
{
	if(ret_name){
		strcpy(ret_name,"PELCO-D");
	}
}

static void get_config(int* ret_databit, int* ret_stopbit, char* ret_parity)
{
	if(ret_databit){
		*ret_databit = 8;
	}
	if(ret_stopbit){
		*ret_stopbit = 1;
	}
	if(ret_parity){
		*ret_parity = 'N';
	}
}

static int get_command(unsigned char* buf, PTZ_COMMAND cmd, unsigned char addr, unsigned char arg)
{
	if(!buf){
		return -1;
	}
	buf[0] = 0xff;
	buf[1] = 0x1;
	switch (cmd)
	{
	case PTZ_CMD_UP:
		buf[2] =0x00;
		buf[3] = 0x08;
		buf[4] = 0x00;
		buf[5] = cmd_speed(arg);
		break;
	case PTZ_CMD_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x10;
		buf[4] = 0x00;
		buf[5] = cmd_speed(arg);
		break;
	case PTZ_CMD_LEFT:
		buf[2] = 0x00;
		buf[3] = 0x04;
		buf[4] = cmd_speed(arg);
		buf[5] = 0x00;
		break;
	case PTZ_CMD_RIGHT:
		buf[2] = 0x00;
		buf[3] = 0x02;
		buf[4] = cmd_speed(arg);
		buf[5] = 0x00;
		break;
	case PTZ_CMD_LEFT_UP:
		buf[2] = 0x00;
		buf[3] = 0x0C;
		buf[4] = cmd_speed(arg);
		buf[5] = cmd_speed(arg);
		break;
	case PTZ_CMD_RIGHT_UP:
		buf[2] = 0x00;
		buf[3] = 0x0A;
		buf[4] = cmd_speed(arg);
		buf[5] = cmd_speed(arg);
		break;
	case PTZ_CMD_LEFT_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x14;
		buf[4] = cmd_speed(arg);
		buf[5] = cmd_speed(arg);
		break;
	case PTZ_CMD_RIGHT_DOWN:
		buf[2] = 0x00;
		buf[3] = 0x12;
		buf[4] = cmd_speed(arg);
		buf[5] = cmd_speed(arg);
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

const UART_PTZHandle g_hPelcoD =
{
	.GetName = get_name,
	.GetConfig = get_config,
	.GetCommand = get_command,
};

