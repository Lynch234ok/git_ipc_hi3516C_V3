#ifndef GIT_IPC_UART2_H
#define GIT_IPC_UART2_H

#include <stdint.h>

typedef struct UartAttr
{
	int fdDev;
	char szDev[16];
} Uart2Attr;

typedef struct Uart2
{
	Uart2Attr stAttr;
	int (*GetFileDesc)(struct Uart2* const thiz);
	void (*PrintStatus)(struct Uart2* const thiz);
	int (*SetBaud)(struct Uart2* const thiz, int nBaud);	// 2400 ...
	int (*GetBaud)(struct Uart2* const thiz);
	int (*SetDatabit)(struct Uart2* const thiz, int nDatabit);	// 5 6 7 8
	int (*GetDatabit)(struct Uart2* const thiz);
	int (*SetStopbit)(struct Uart2* const thiz, int nStopbit);	// 1 2
	int (*GetStopbit)(struct Uart2* const thiz);
	int (*SetParity)(struct Uart2* const thiz, char chParity);	// n o e
	char (*GetParity)(struct Uart2* const thiz);
    ssize_t (*Send)(struct Uart2* const thiz, uint8_t* u8Bytes, size_t nSize);
    ssize_t (*Recv)(struct Uart2* const thiz, uint8_t* ret_u8Bytes, size_t nSize);
    ssize_t (*Recv_Nb)(struct Uart2* const thiz, uint8_t* ret_u8Bytes, size_t nSize);
} Uart2;

extern Uart2* PUART_Struct(const char* szDev);
extern void PUART_Destruct(Uart2** pthiz);

#endif	//GIT_IPC_UART2_H

