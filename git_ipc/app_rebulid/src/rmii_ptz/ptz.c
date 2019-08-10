#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "steper_ctrl.h"
#include "ptz.h"

typedef struct PTZ
{
	int init;
	int speed;
	PTZ_COMMAND cmd;
	int (*Send)(struct PTZ* const thiz, int nChn, PTZ_COMMAND enCmd, unsigned char u8Param);
}PTZ;

static int ptz_Send(struct PTZ* const thiz, int nChn, PTZ_COMMAND enCmd, unsigned char u8Param)
{
	thiz->speed = u8Param;
	switch(enCmd){
		case PTZ_CMD_UP:
			STEPER_startV(1, thiz->speed);
			break;
		case PTZ_CMD_DOWN:
			STEPER_startV(-1, thiz->speed);
			break;
		case PTZ_CMD_LEFT:
			STEPER_startH(-1, thiz->speed);
			break;
		case PTZ_CMD_RIGHT:
			STEPER_startH(1, thiz->speed);
			break;
		case PTZ_CMD_STOP:{
				switch(thiz->cmd){
					case PTZ_CMD_UP:
					case PTZ_CMD_DOWN:
						STEPER_stopV();
						break;
					case PTZ_CMD_LEFT:
					case PTZ_CMD_RIGHT:
					case PTZ_CMD_AUTOPAN:
						STEPER_stopH();
						break;
					case PTZ_CMD_LEFT_UP:
					case PTZ_CMD_RIGHT_UP:
					case PTZ_CMD_LEFT_DOWN:
					case PTZ_CMD_RIGHT_DOWN:
						STEPER_stopH();
						STEPER_stopV();
						break;
				}
			}
			break;
		case PTZ_CMD_AUTOPAN:
			if(PTZ_CMD_AUTOPAN == thiz->cmd){
				STEPER_stopH();
			}else{
				STEPER_startH(0, thiz->speed);
			}
			break;
		case PTZ_CMD_LEFT_UP:
			break;
		case PTZ_CMD_RIGHT_UP:
			break;
		case PTZ_CMD_LEFT_DOWN:
			break;
		case PTZ_CMD_RIGHT_DOWN:
			break;
		default:
			break;
	}
	thiz->cmd = enCmd;
	return 0;
}


static PTZ s_stPTZ =
{
	.init = 0,
	.speed = 0,
	.Send = ptz_Send,
};

int PTZ_Init()
{
	s_stPTZ.init = 1;
	STEPER_init();
	return 0;
}

void PTZ_Destroy()
{
	STEPER_destroy();
	s_stPTZ.init = 0;
}


int PTZ_Cmd_str2int(const char *ptz_cmd)
{
	int i = 0;
	const char *ptz_cmd_str[PTZ_COMMAND_CNT] = {
		"down",
		"up",
		"left",
		"right",
		"left up",
		"right up",
		"left down",
		"right down",
		"auto",
		"apertureout",
		"aperturein",
		"zoomin",
		"zoomout",
		"focusout",
		"focusin",
		"stop",
		"brush",
		"brush",
		"light",
		"light",
		"POWER_ON",
		"POWER_OFF",
		"GOTO_PRESET",
		"SET_PRESET",
		"CLEAR_PRESET",
		"auto"
		};	

	if(!ptz_cmd)return -1;
		
	for(i = 0; i < PTZ_COMMAND_CNT; i++){
		if(!strcmp(ptz_cmd, ptz_cmd_str[i])){
			return i;
		}
	}
	return -1;
}

int PTZ_Send(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param)
{
	if(s_stPTZ.init){
		return s_stPTZ.Send(&s_stPTZ, nChn, enCmd, u8Param);
	}else{
		return -1;
	}
}




