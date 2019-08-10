

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
#include "uart_ptz.h"
#include "pan_tilt.h"

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#define DEBUG_UART_PTZ
#ifdef DEBUG_UART_PTZ
#define UART_PTZ_TRACE(fmt...) \
	do{printf("\033[1;31mPTZ->[%s]:%d ", __FUNCTION__, __LINE__);printf(fmt);printf("\033[m\r\n");}while(0)
#else
#define UART_PTZ_TRACE(fmt...)
#endif


#define PTZ_UART_DEV "/dev/ttyAMA1"
#define MAX_CMD_SIZE (16)

#define IPCAM_VIN_CH 1

typedef enum
{
	PTP_PELCOD = 0,
	PTP_PELCOP,
	//PTP_ALEC,
	PTZ_PROCOTOL_CNT,
}PTZ_PROCOTOL;

typedef struct UART_ptz
{
	// private
	UART_t* devUart;
	unsigned char bBusy;
	PTZ_PROCOTOL aenProcotol[IPCAM_VIN_CH];
	int anBaud[IPCAM_VIN_CH];
	unsigned char au8Addr[IPCAM_VIN_CH];
	unsigned char abAutoPan[IPCAM_VIN_CH];

	// public
	unsigned char (*IsAutoPan)(struct UART_ptz* const thiz, int nChn);
	int (*Config)(struct UART_ptz* const thiz, int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr);
	int (*Send)(struct UART_ptz* const thiz, int nChn, PTZ_COMMAND enCmd, unsigned char u8Param);
	//
}UART_PTZ_t;

inline static UART_PTZHandle HANDLE(PTZ_PROCOTOL enProtocol)
{
	printf("protocol:%d\n\n", enProtocol);
	UART_PTZHandle ahProtocol[PTZ_PROCOTOL_CNT];
	//assert(enProtocol < PTZ_PROCOTOL_CNT);
	ahProtocol[PTP_PELCOD] = g_hPelcoD;
	ahProtocol[PTP_PELCOP] = g_hPelcoP;
	//ahProtocol[PTP_ALEC] = g_hAlec;
	return ahProtocol[enProtocol];
};

static unsigned char ptz_IsAutoPan(struct UART_ptz* const thiz, int nChn)
{
	//assert(nChn < IPCAM_VIN_CH);
	return thiz->abAutoPan[nChn] ? TRUE : FALSE;
}

static int ptz_Config(struct UART_ptz* const thiz, int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr)
{
	//assert(nChn < IPCAM_VIN_CH);
	if(!strncasecmp(szProtocol, "PELCOD", strlen("PELCOD")) || !strncasecmp(szProtocol, "PELCO-D", strlen("PELCO-D"))){
		thiz->aenProcotol[nChn] = PTP_PELCOD;
		//UART_PTZ_TRACE("config ptz %d to Pelco-D", nChn + 1);
	}else if(!strncasecmp(szProtocol, "PELCOP", strlen("PELCOP")) || !strncasecmp(szProtocol, "PELCO-P", strlen("PELCO-P"))){
		thiz->aenProcotol[nChn] = PTP_PELCOP;
		//UART_PTZ_TRACE("config ptz %d to Pelco-P", nChn + 1);
	}/*else if(!strncasecmp(szProtocol, "ALEC", strlen("ALEC"))){
		thiz->aenProcotol[nChn] = PTP_ALEC;
		//UART_PTZ_TRACE("config ptz %d to Alec", nChn + 1);
	}*/else{
		thiz->aenProcotol[nChn] = PTP_PELCOD; // default protocol
		//UART_PTZ_TRACE("config ptz %d to default Pelco-D", nChn + 1);
	}
	//thiz->aenProcotol[nChn] = PTP_PELCOP;
	thiz->anBaud[nChn] = nBaud;
	thiz->au8Addr[nChn] = u8Addr;
	return 0;
}

#ifdef DEBUG_UART_PTZ
#define UART_PTZ_PRINT_CMD(cmd,size) \
	do{\
		int i = 0;\
		printf("\033[47;30mPTZ Command:");\
		for(i = 0; i < size; ++i){printf(" %02X", cmd[i]);}\
		printf("\033[0m\r\n");\
	}while(0);
#else
#define UART_PTZ_PRINT_CMD(cmd,size)
#endif

static int ptz_Send(struct UART_ptz* const thiz, int nChn, PTZ_COMMAND enCmd, unsigned char u8Param)
{
	int nDatabit = 0;
	int nStopbit = 0;
	char chParity = 'N';
	unsigned char au8Cmd[MAX_CMD_SIZE];
	int nCmdSize = 0;
	UART_PTZHandle hProtocal;
	//assert(nChn < IPCAM_VIN_CH);
	if(thiz->bBusy){
		UART_PTZ_TRACE("PTZ bus is busy!");
		return -1;
	}
	thiz->bBusy = TRUE;
	// config first
	//hProtocal = HANDLE(thiz->aenProcotol[nChn]);
	hProtocal = HANDLE(PTP_PELCOD);
	hProtocal.GetConfig(&nDatabit, &nStopbit, &chParity);
	UART_PTZ_TRACE("chn=%d set baud=%d", nChn, thiz->anBaud[nChn]);
//	thiz->anBaud[nChn] = thiz->devUart->set_baud(thiz->devUart, 57600);
//	thiz->devUart->set_databit(thiz->devUart, nDatabit);
//	thiz->devUart->set_stopbit(thiz->devUart, nStopbit);
//	thiz->devUart->set_parity(thiz->devUart, chParity);
	// get command
	if(PTZ_CMD_STOP == enCmd){
		thiz->abAutoPan[nChn] = FALSE;
	}else if(PTZ_CMD_AUTOPAN == enCmd){
		if(thiz->IsAutoPan(thiz, nChn)){
			enCmd = PTZ_CMD_STOP;
			thiz->abAutoPan[nChn] = FALSE;
		}else{
			thiz->abAutoPan[nChn] = TRUE;
		}
	}
	nCmdSize = hProtocal.GetCommand(au8Cmd, enCmd, thiz->au8Addr[nChn], u8Param);
	UART_PTZ_PRINT_CMD(au8Cmd, nCmdSize);

#if defined(UART_PROTOCOL)
#include "uart_protocol.h"
	DATA data;

	memcpy(data.sendData.pelcod_data, au8Cmd, nCmdSize);
	UART_auto_send(UART_CMD_SEND_PELCOD_DATA, &data);

#endif
	// send command
	thiz->devUart->write(thiz->devUart, au8Cmd, nCmdSize);
	thiz->bBusy = FALSE;
	

	return 0;
}


static UART_PTZ_t s_stUART_PTZ =
{
	.devUart = NULL,
	.bBusy = FALSE,
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

int uart_ptz_Init()
{
	if(s_stUART_PTZ.devUart){
		UART_PTZ_TRACE("ptz has inited");
		return -1;
	}	
	s_stUART_PTZ.devUart = UART_open(PTZ_UART_DEV);
	//assert(s_stUART_PTZ.devUart);
	return 0;
}

void uart_ptz_Destroy()
{
	if(s_stUART_PTZ.devUart){
		// diable rs485
		UART_close(s_stUART_PTZ.devUart);
		s_stUART_PTZ.devUart = NULL;
	}
}

int uart_ptz_Config(int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr)
{	
	int channel;
	channel = nChn < IPCAM_VIN_CH ? nChn : IPCAM_VIN_CH;
	return s_stUART_PTZ.Config(&s_stUART_PTZ, channel, szProtocol, nBaud, u8Addr);
}

int uart_ptz_Send(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param)
{
	int channel;
	channel = nChn < IPCAM_VIN_CH ? nChn : IPCAM_VIN_CH;
	s_stUART_PTZ.Send(&s_stUART_PTZ, channel, enCmd, u8Param);
	focus_contrl(enCmd, u8Param);
	return 0;
}



