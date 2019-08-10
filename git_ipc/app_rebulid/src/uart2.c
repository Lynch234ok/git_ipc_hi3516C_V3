#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<errno.h>
#include    "uart2.h"

#define DEBUG_UART
#ifdef DEBUG_UART
#define UART_TRACE(fmt...) \
	do{printf("\033[1;31mUART2->[%s]:%d ", __FUNCTION__, __LINE__);printf(fmt);printf("\033[m\r\n");}while(0)
#else
#define UART_TRACE(fmt...)
#endif

static int uart_GetFileDesc(struct Uart2* const thiz)
{
	return thiz->stAttr.fdDev > 0 ? thiz->stAttr.fdDev : -1;
}

static void uart_PrintStatus(struct Uart2* const thiz)
{
	char parity;
	printf("UART(%s) status:\r\n", thiz->stAttr.szDev);
	printf("Baud: %d\r\n", thiz->GetBaud(thiz));
	printf("Databit: %d\r\n", thiz->GetDatabit(thiz));
	printf("Stopbit: %d\r\n", thiz->GetStopbit(thiz));
	printf("Parity: %s\r\n", ('N' == (parity = thiz->GetParity(thiz))) ? "None" :\
		(('O' == parity) ? "Odd" : "Even"));
}

static int uart_SetBaud(struct Uart2* const thiz, int baud)
{
	speed_t nBaud = 0;
	struct termios options = {0};
	if(0 != tcgetattr(thiz->stAttr.fdDev, &options))
	{
		perror("tcgetattr");
		return -1;
	}
	if(baud <= 300){
		baud = 300;
		nBaud = B300;
	}else if(baud <= 1200){
		baud = 1200;
		nBaud = B1200;
	}else if(baud <= 2400){
		baud = 2400;
		nBaud = B2400;
	}else if(baud <= 4800){
		baud = 4800;
		nBaud = B4800;
	}else if(baud <= 9600){
		baud = 9600;
		nBaud = B9600;
	}else if(baud <= 19200){
		baud = 19200;
		nBaud = B19200;
	}else if(baud <= 38400){
		baud = 38400;
		nBaud = B38400;
	}else if(baud <= 57600){
		baud = 57600;
		nBaud = B57600;
	}else if(baud <= 115200){
		baud = 115200;
		nBaud = B115200;
	}else{
		baud = 4800;
		nBaud = B4800;
	}

	cfsetispeed(&options, nBaud);
	cfsetospeed(&options, nBaud);
	tcflush(thiz->stAttr.fdDev, TCIOFLUSH);
	if(0 !=  tcsetattr(thiz->stAttr.fdDev, TCSANOW, &options)) {
		perror("tcsetattr");
		return -1;
	}
	tcflush(thiz->stAttr.fdDev, TCIOFLUSH);
	return baud;
}

static int uart_GetBaud(struct Uart2* const thiz)
{
	struct termios options = {0};
	if(0 != tcgetattr(thiz->stAttr.fdDev, &options)) {
		perror("tcgetattr");
		return -1;
	}
	switch(cfgetispeed(&options)) {
	case B300:
		return 300;
	case B1200:
		return 1200;
	case B2400:
		return 2400;
	case B4800:
		return 4800;
	case B9600:
		return 9600;
	case B19200:
		return 19200;
	case B38400:
		return 38400;
	case B57600:
		return 57600;
	case B115200:
		return 115200;
	default:
		return -1;
	}
}

static int uart_SetDatabit(struct Uart2* const thiz, int databit)
{
	struct termios options = {0};
	if(databit != 5 && databit != 6 && databit != 7 && databit != 8) {
		errno = EINVAL;
		return -1;
	}

	if(0 != tcgetattr(thiz->stAttr.fdDev, &options)) {
		perror("tcgetattr");
		return -1;
	}

	options.c_cflag |= CLOCAL | CREAD;
	options.c_cflag &= ~CSIZE;
	switch(databit) {
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
	default:
		options.c_cflag |= CS8;
		break; 
	}
	tcflush(thiz->stAttr.fdDev, TCIOFLUSH);
	if(0 !=  tcsetattr(thiz->stAttr.fdDev, TCSANOW, &options)) {
		perror("tcsetattr");
		return -1;
	}
	return 0;
}

static int uart_GetDatabit(struct Uart2* const thiz)
{
	struct termios options = {0};
	if(0 != tcgetattr(thiz->stAttr.fdDev, &options)) {
		perror("tcgetattr");
		return -1;
	}
	switch(options.c_cflag & CSIZE) {
	case CS5:
		return 5;
	case CS6:
		return 6;
	case CS7:
		return 7;
	case CS8:
		return 8;
	default:
		break;
	}
	return 0;
}

static int uart_SetStopbit(struct Uart2* const thiz, int stopbit)
{
	struct termios options = {0};
	if(stopbit != 1 && stopbit != 2){
		errno = EINVAL;
		return -1;
	}
	// get uart attr
	if(0 != tcgetattr(thiz->stAttr.fdDev, &options)) {
		perror("tcgetattr");
		return -1;
	}
	switch(stopbit) {
	case 2:
		options.c_cflag |= CSTOPB;
		break;

	case 1:
	default:
		options.c_cflag &= ~CSTOPB;
		break;
	}
	tcflush(thiz->stAttr.fdDev, TCIOFLUSH);
	if(0 !=  tcsetattr(thiz->stAttr.fdDev, TCSANOW, &options)){
		perror("tcsetattr");
		return -1;
	}
	return 0;
}

static int uart_GetStopbit(struct Uart2* const thiz)
{
	struct termios options = {0};
	// get uart attr
	if(0 != tcgetattr(thiz->stAttr.fdDev, &options)){
		perror("tcgetattr");
		return -1;
	}
	if(options.c_cflag & CSTOPB){
		return 2;
	}
	else{
		return 1;
	}
}

static int uart_SetParity(struct Uart2* const thiz, char parity)
{
	struct termios options = {0};
	// get uart attr
	if(0 != tcgetattr(thiz->stAttr.fdDev, &options)){
		perror("tcgetattr");
		return -1;
	}
	switch(parity){
	// Odd
	case 1:
	case 'o':
	case 'O':
		options.c_cflag |= PARENB;
		options.c_cflag |= PARODD;
		options.c_iflag |= INPCK | ISTRIP;
		break;
	// even
	case 2:
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK | ISTRIP;
		break;
	// none
	case 0:
	case 'n':
	case 'N':
	default:
		options.c_cflag &= ~PARENB;
		break;
	}

	tcflush(thiz->stAttr.fdDev, TCIOFLUSH);
	if(0 !=  tcsetattr(thiz->stAttr.fdDev, TCSANOW, &options)){
		perror("tcsetattr");
		return -1;
	}
	//tcflush(thiz->stAttr.fdDev, TCIOFLUSH);
	return 0;
}

static char uart_GetParity(struct Uart2* const thiz)
{
	struct termios options = {0};
	if(0 != tcgetattr(thiz->stAttr.fdDev, &options)){
		perror("tcgetattr");
		return 'N';
	}
	if(options.c_cflag & PARENB){
		if(options.c_cflag & PARODD){
			return 'O';
		}else{
			return 'E';
		}
	}else{
		return 'N';
	}
}

static ssize_t uart_Send(struct Uart2* const thiz, uint8_t* bytes, size_t len)
{
	ssize_t nSize = 0;
	if((nSize = write(thiz->stAttr.fdDev, bytes, len)) < 0){
		perror("write");
		return -1;
	}
	return nSize;
}

static ssize_t uart_Recv(struct Uart2* const thiz, uint8_t* ret_array, size_t len)
{
	ssize_t nSize = 0;
	if((nSize = read(thiz->stAttr.fdDev, ret_array, len)) < 0) {
		perror("read");
		return -1;
	}
	return nSize;
}

static ssize_t uart_Recv_Nb(struct Uart2* const thiz, uint8_t* ret_array, size_t len)
{
	ssize_t nSize = 0;
	int ret = 0;
	struct timeval select_timeo;
	int fd;


	fd = thiz->stAttr.fdDev;
	fd_set rfd_set;

	select_timeo.tv_sec = 0;
	select_timeo.tv_usec = 0;

	FD_ZERO(&rfd_set);
	FD_SET(fd, &rfd_set);

	ret = select(fd + 1, &rfd_set, NULL, NULL, &select_timeo);
    if (ret < 0) {
		UART_TRACE("select failed!");
        return -1;
	} else if (0 == ret) {
		// timeout!
        return 0;
	} else {
        if(FD_ISSET(fd, &rfd_set)) {
            if((nSize = read(thiz->stAttr.fdDev, ret_array, len)) < 0) {
				UART_TRACE("read failed!");
				return -1;
			}
            return nSize;
		} else {
			UART_TRACE("fd is not in rfd_set!!");
			return -1;
		}
	}
}

Uart2* PUART_Struct(const char* szDev)
{
	int fdDev = 0; // open device failed
	if((fdDev = open(szDev, O_RDWR)) <= 0){
		UART_TRACE("open device \"%s\" error: %s", szDev, strerror(errno));
		return NULL;
	}

	{
		struct termios options = {0};
		if(0 != tcgetattr(fdDev, &options)){
			perror("tcgetattr");
			close(fdDev);
			return NULL;
		}
		cfmakeraw(&options);
		if(0 !=  tcsetattr(fdDev, TCSANOW, &options)){
			perror("tcsetattr");
			close(fdDev);
			return NULL;
		}
	}

	Uart2* thiz = calloc(sizeof(Uart2), 1);
	if (NULL == thiz) {
		UART_TRACE("calloc failed!");
		return NULL;
	}
	thiz->stAttr.fdDev = fdDev;
	memset(thiz->stAttr.szDev, 0, sizeof(thiz->stAttr.szDev));
	strcpy(thiz->stAttr.szDev, szDev);

	thiz->GetFileDesc = uart_GetFileDesc;
	thiz->PrintStatus = uart_PrintStatus;
	thiz->SetBaud = uart_SetBaud;
	thiz->GetBaud = uart_GetBaud;
	thiz->SetDatabit = uart_SetDatabit;
	thiz->GetDatabit = uart_GetDatabit;
	thiz->SetStopbit = uart_SetStopbit;
	thiz->GetStopbit = uart_GetStopbit;
	thiz->SetParity = uart_SetParity;
	thiz->GetParity = uart_GetParity;
	thiz->Send = uart_Send;
	thiz->Recv = uart_Recv;
	thiz->Recv_Nb = uart_Recv_Nb;
	return thiz;
}

void PUART_Destruct(Uart2** pthiz)
{
	close((*pthiz)->stAttr.fdDev);
	free(*pthiz);
	*pthiz = NULL;
}

/*
int main(int argc, char** argv)
{
	Uart2* uart = PUART_Struct(argv[1]);
	if(uart){
		printf("UART start test, press any keys to continue.");
		getchar();

		printf("Set baudrate 115200\r\n");
		printf("Set databit 8\r\n");
		printf("Set stopbit 1\r\n");
		printf("Set parity NONE\r\n");
		uart->SetBaud(uart, 115200);
		uart->SetDatabit(uart, 8);
		uart->SetStopbit(uart, 1);
		uart->SetParity(uart, 0);
		printf("Press any keys to check UART status.");
		getchar();

		uart->PrintStatus(uart);
		printf("Press any keys to send \"Hello\".");
		getchar();

		uart->Send(uart, (unsigned char*)"Hello\r\n", strlen("Hello\r\n"));
		printf("Press any keys to send \"Merry Christmas\"");
		getchar();

		uart->Send(uart, (unsigned char*)"Merry Christmas\r\n.", strlen("Merry Christmas\r\n"));
		printf("Press any keys to send \"Goodbye\".");
		getchar();

		uart->Send(uart, (unsigned char*)"Goodbye\r\n", strlen("Goodbye\r\n"));
		printf("Press any keys to finish testing");
		getchar();

		PUART_Destruct(&uart);
	}
	return 0;
}
*/
