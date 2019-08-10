
#ifndef __UART_PTZ_H__
#define __UART_PTZ_H__


typedef struct UART_PTZHandle
{
	void (*GetName)(char* ret_szName);
	void (*GetConfig)(int* ret_nDatabit, int* ret_nStopbit, char* ret_chParity);
	int (*GetCommand)(unsigned char* ret_u8Buf, PTZ_COMMAND emCmd, unsigned char u8Addr, unsigned char u8Param);
	
}UART_PTZHandle;

// all the ptz protocol
extern const UART_PTZHandle g_hPelcoD;
extern const UART_PTZHandle g_hPelcoP;
extern const UART_PTZHandle g_hAlec;

#ifdef __cplusplus
extern "C" {
#endif
extern int uart_ptz_Init();
extern void uart_ptz_Destroy();
extern int uart_ptz_Config(int nChn, const char* szProtocol, int nBaud, unsigned char u8Addr);
extern int uart_ptz_Send(int nChn, PTZ_COMMAND enCmd, unsigned char u8Param);
#ifdef __cplusplus
}
#endif

#endif //__UART_PTZ_H__

