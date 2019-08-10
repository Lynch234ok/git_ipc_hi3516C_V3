

//#define NDEBUG
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

#include "uart.h"
#include "ptz.h"

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#define DEBUG_PTZ
#ifdef DEBUG_PTZ
#define PTZ_TRACE(fmt...) \
	do{printf("\033[1;31mPTZ->[%s]:%d ", __FUNCTION__, __LINE__);printf(fmt);printf("\033[m\r\n");}while(0)
#else
#define PTZ_TRACE(fmt...)
#endif


#define MAX_CMD_SIZE (16)

#define IPCAM_VIN_CH 1



static PTZ_t s_stPTZ =
{
	.Init = NULL, 
	.Destroy = NULL,
	.Config = NULL,
	.Send = NULL,
};

int PTZ_Init(PTZ_t *ptzAttr)
{
	if(ptzAttr){
		memcpy(&s_stPTZ, ptzAttr, sizeof(PTZ_t));	
		if(s_stPTZ.Init){
			s_stPTZ.Init();
			return 0;
		}
	}
	return -1;
}


void PTZ_Destroy()
{
	if(s_stPTZ.Destroy){
		s_stPTZ.Destroy();
	}else{
		PTZ_TRACE("PTZ not Init");
	}
}

int PTZ_Config(int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr)
{
	if(s_stPTZ.Config){
		return s_stPTZ.Config(nChn, szProtocol, nBaud, u8Addr);
	}else{
		PTZ_TRACE("PTZ not Init");
		return -1;
	}
}

int PTZ_Cmd_str2int(const char *ptz_cmd)
{
	int i = 0;
	const char *ptz_cmd_str[PTZ_COMMAND_CNT] = {
		
		"up",
		"down",
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


int PTZ_P2P_Cmd_str2int(const char *ptz_cmd)
{
	int i = 0;
	const char *ptz_cmd_str[PTZ_COMMAND_CNT] = {
		"up",
		"down",
		"left",
		"right",
		"left up",
		"right up",
		"left down",
		"right down",
		"auto",
		"apertureout",
		"aperturein",
		"zoom_i",
		"zoom_o",
		"focus_f",
		"focus_n",
		"stop",
		"brush",
		"brush",
		"light",
		"light",
		"POWER_ON",
		"POWER_OFF",
		"preset_g",
		"preset_s",
		"preset_c",
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
	if(s_stPTZ.Send){
		return s_stPTZ.Send(nChn, enCmd, u8Param);
	}else{
		PTZ_TRACE("PTZ not Init");
		return -1;
	}
}



