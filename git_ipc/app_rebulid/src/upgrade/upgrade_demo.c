#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include <linux/unistd.h>     /* ∞¸∫¨µ˜”√ _syscallX ∫Íµ»œ‡πÿ–≈œ¢*/
#include <linux/kernel.h>     /* ∞¸∫¨sysinfoΩ·ππÃÂ–≈œ¢*/
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <sys/reboot.h>
//#fixme
#ifdef u8
#undef u8
#endif

#ifdef s32
#undef s32
#endif

#ifdef u32
#undef u32
#endif

typedef signed char s8;
typedef unsigned char u8;
typedef const char c8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

//Êú∫Âô®Â≠óËäÇÈïøÂ∫¶
typedef signed long slong;
typedef unsigned long ulong;

typedef signed long long s64;
typedef unsigned  long long  u64;

#include "upgrade.h"

//‘§¡Ù∂‡œﬂ≥Ã÷¥––À¯
//Ω®“È1page=4096Bytes ∂‘∆Î≤Ÿ◊˜œµÕ≥ «∞¥“≥∑÷≈‰
void *NK_UpgradeMemmap(unsigned int phy_addr, unsigned int phy_size);
int NK_UpgradeMunmap(void *pVirAddr, unsigned int size);
s32 NK_DaemoFwUnEncodeHeader(u8 *pdata, u32 len);
int WATCHDOG_init(int timeout_s);
int WATCHDOG_enable();
int WATCHDOG_feed();

static unsigned short DoCrc16( unsigned short reg_init, char *message, unsigned int len)
{
	 unsigned short crc_reg = reg_init;

	const  unsigned short crc16_ccitt_table[] =
	{
		0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
		0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
		0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
		0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
		0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
		0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
		0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
		0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
		0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
		0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
		0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
		0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
		0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
		0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
		0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
		0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
		0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
		0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
		0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
		0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
		0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
		0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
		0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
		0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
		0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
		0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
		0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
		0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
		0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
		0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
		0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
		0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
	};
	
	while (len--)
	{
		crc_reg = (crc_reg >> 8) ^ crc16_ccitt_table[(crc_reg ^ *message++) & 0xff];
	}
	
	return crc_reg;
}

unsigned short NK_LIB_DoCrc16(void *pdata, unsigned int len)
{
	return DoCrc16(0, pdata, len);
}


int FifoRead(int fd, FIFO_CMD_S *pCmd)
{
	int ret;
	FIFO_CMD_S cmd;

	if (pCmd == NULL || fd < 0)
	{
		return FIFO_ARGS_ERROR;
	}
	
	//read cmd
	memset(&cmd, 0, sizeof(cmd));
	ret = read(fd, &cmd, sizeof(cmd));
	if (ret <= 0) //no data read
	{
		return FIFO_READ_ERROR; 
	}
	
	if (ret != sizeof(cmd))
	{
		printf("read ret =%d != sizeof(cmd) = %d\n", ret, sizeof(cmd));
		return FIFO_DATA_ERROR;
	}
	
	if (cmd.u32Crc != (unsigned int)NK_LIB_DoCrc16((void *)&cmd.u8Cmd[0], sizeof(cmd.u8Cmd)))
	{
		printf("read = ru32Crc =%x != 0x%x \n", cmd.u32Crc, NK_LIB_DoCrc16((void *)&cmd.u8Cmd[0], sizeof(cmd.u8Cmd)));
		return FIFO_CRC_ERROR;
	}
	
	if (cmd.u32MagicNum != FIFO_CMD_MAGIC_NUM)
	{
		printf("u32MagicNum  =%x != 0x%x \n", cmd.u32MagicNum, FIFO_CMD_MAGIC_NUM);
		return FIFO_MAGIC_ERROR;
	}

	memcpy(pCmd, &cmd, sizeof(FIFO_CMD_S));

	return ret;
}

int FifoWrite(int fd, FIFO_CMD_S *pCmd)
{
	int ret;
	
	if (pCmd == NULL)
	{
		return FIFO_ARGS_ERROR;
	}
	
	//write ack
	pCmd->u32MagicNum = FIFO_CMD_MAGIC_NUM;
	pCmd->u32Crc = (unsigned int)NK_LIB_DoCrc16((void *)&pCmd->u8Cmd[0], sizeof(pCmd->u8Cmd));
	ret = write(fd, pCmd, sizeof(FIFO_CMD_S));
	if (ret != sizeof(FIFO_CMD_S))
	{
		printf("write ret =%d != sizeof(cmd) = %d\n", ret, sizeof(FIFO_CMD_S));
		return FIFO_WRITE_ERROR;
	}

	return ret;
}

int GetFifoWriteFd(void)
{
	int ret;
	static int write_fd = -1;

	if (write_fd > 0)
	{
		return write_fd;
	}
	
	if (access(FIFO_CLIENT_NAME, F_OK) == -1)	
	{  
		ret = mkfifo(FIFO_CLIENT_NAME,	0777);	
		if (ret != 0)  
		{  
			printf("Could not create fifo %s \n", FIFO_CLIENT_NAME);  
			return -1;
		}  
	}  
		
	if (write_fd < 0)
	{
		write_fd = open(FIFO_CLIENT_NAME, O_RDWR, 0); //◊Ë»˚
		if (write_fd < 0)
		{
			printf("Could not open fifo %s \n", FIFO_CLIENT_NAME);	
			return -1;
		}
	}
		
	return write_fd;
}

int GetFifoReadFd(void)
{
	int ret;
	static int read_fd = -1;

	if (read_fd > 0)
	{
		return read_fd;
	}
	if (access(FIFO_SRV_NAME, F_OK) == -1)	
	{  
		ret = mkfifo(FIFO_SRV_NAME,	0777);	
		if (ret != 0)  
		{  
			printf("Could not create fifo %s \n", FIFO_SRV_NAME);  
			return -1;
		}  
	}  
		
	if (read_fd < 0)
	{
		read_fd = open(FIFO_SRV_NAME, O_RDWR, 0); //◊Ë»˚
		if (read_fd < 0)
		{
			printf("Could not open fifo %s \n", FIFO_SRV_NAME);	
			return -1;
		}
	}
		
	return read_fd;
}

int FifoAppWrite(FIFO_CMD_S *pstCmd)
{
	static unsigned int nIdx = 0;
	int ret = -1;
	FIFO_CMD_S stAck;
	FIFO_CMD_S stCmd;
	int read_fd;
	int write_fd;

	if (pstCmd == NULL)
	{
		return -1;
	}
	
	write_fd = GetFifoWriteFd();
	if (write_fd < 0)
	{
		return -1;
	}
	
	read_fd = GetFifoReadFd();
	if (read_fd < 0)
	{
		return -1;
	}
	
	nIdx++;
	memcpy(&stCmd, pstCmd, sizeof(FIFO_CMD_S));
	stCmd.u32Idx = nIdx;
	ret = FifoWrite(write_fd, &stCmd);
	if (ret > 0)
	{
		ret = FifoRead(read_fd, &stAck);
		if (ret > 0)
		{
			if (stAck.u32CmdType == FIFO_CMD_ACK)
			{
				if (stCmd.u32Idx  == stAck.u32Idx)
				{
					return 0;//OK 
				}
			}
		}
	}

	ret = -1;
	
	return ret;
}

int FifoAppRead(FIFO_CMD_S *pstCmd)
{	
	int ret;
	int read_fd;
	
	if (pstCmd == NULL)
	{
		return -1;
	}
	
	read_fd = GetFifoReadFd();
	if (read_fd < 0)
	{
		return -1;
	}

	ret = FifoRead(read_fd,pstCmd);
	
	return ret;
}

//app send mmz phy addr cmd to daemon_upgrade 
int FifoSendPhyAddrMapCmd(unsigned int u32PhyAddr, unsigned int u32Len)
{
	int ret = -1;
	FIFO_CMD_S stCmd;
	PHY_ADDR_INFO *pstAddrInfo;
	
	memset(&stCmd, 0, sizeof(FIFO_CMD_S));
	stCmd.u32CmdType = FIFO_CMD_PHY_ADDR_MMAP;
	pstAddrInfo = (PHY_ADDR_INFO *)stCmd.u8Cmd;
	pstAddrInfo->u32PhyAddr = u32PhyAddr;
	pstAddrInfo->u32PhyLen  = u32Len;
	ret = FifoAppWrite(&stCmd);
	if (ret != 0)
	{
		printf("%s ret = %d error\n", __FUNCTION__, ret);
	}

	return ret;
}

int FifoSendPhyAddrMunmapCmd(void)
{
	int ret = -1;
	FIFO_CMD_S stCmd;
	memset(&stCmd, 0, sizeof(FIFO_CMD_S));
	stCmd.u32CmdType = FIFO_CMD_PHY_ADDR_MUNMAP;
	ret = FifoAppWrite(&stCmd);
	if (ret != 0)
	{
		printf("%s ret = %d error\n", __FUNCTION__, ret);
	}

	return ret;
}

//app send mmz phy addr cmd to daemon_upgrade 
int FifoSendStartUpgradeCmd(UPGRADE_VER_SIZE_INFO *pUpgradeInfo)
{
	int ret = -1;
	FIFO_CMD_S stCmd;
	UPGRADE_VER_SIZE_INFO *pstInfo;

	if (pUpgradeInfo == NULL)
	{
		return -1;
	}

	if (sizeof(UPGRADE_VER_SIZE_INFO) > sizeof(stCmd.u8Cmd))
	{
		return -1;
	}

	memset(&stCmd, 0, sizeof(FIFO_CMD_S));
	stCmd.u32CmdType = FIFO_CMD_UPGRADE_START;
	pstInfo = (UPGRADE_VER_SIZE_INFO*)stCmd.u8Cmd;
	memcpy(pstInfo, pUpgradeInfo, sizeof(UPGRADE_VER_SIZE_INFO));

	ret = FifoAppWrite(&stCmd);
	if (ret != 0)
	{
		printf("%s ret = %d error\n", __FUNCTION__, ret);
	}

	return ret;
}

int FifoTimeOut(int read_fd, int sec, int usec)
{
	int ret;
	fd_set fd_rs;
	int maxfd;
	struct timeval timeout={0,1}; //selectÁ≠âÂæÖ3ÂæÆÁßíÔºå3ÂæÆÁßíËΩÆËØ¢ÔºåË¶ÅÈùûÈòªÂ°ûÂ∞±ÁΩÆ0
	
	timeout.tv_sec = sec;
	timeout.tv_usec = usec; //us
	FD_ZERO(&fd_rs); //ÊØèÊ¨°Âæ™ÁéØÈÉΩË¶ÅÊ∏ÖÁ©∫ÈõÜÂêàÔºåÂê¶Âàô‰∏çËÉΩÊ£ÄÊµãÊèèËø∞Á¨¶ÂèòÂåñ
	FD_SET(read_fd, &fd_rs); //Ê∑ªÂä†ÊèèËø∞Á¨¶
	maxfd = read_fd;
	ret = select(maxfd + 1, &fd_rs, NULL, NULL, &timeout);
	switch(ret) //select‰ΩøÁî® read
	{
	case -1://Âá∫Èîô
	case 0://Ë∂ÖÊó∂
		//printf("select:line:%d %s\n", __LINE__, ret==0 ? "timeout": "error");
		return -1;
	default://ok ÊúâÈúÄË¶ÅËØªÂèñÊï∞ÊçÆ		
		if(FD_ISSET(read_fd, &fd_rs))
		{
			return 0;
		}
	}

	return 1;
}

int AppReadUpgradeStatus(int flags, int sec, int us, FIFO_CMD_S *pstCmd)
{
	int ret = -1;
	int read_fd;
	FIFO_CMD_S stCmd;

	read_fd = GetFifoReadFd();
	if (read_fd < 0)
	{
		return -1;
	}

	#if 0
	//ÊèêÂâçÁÇπÂéªÈô§Ê®°ÂùóÂíåÊãâËøõÂ∫¶Êù°ÔºåÂÖàÂç°‰ΩèÁî®Êà∑Êìç‰Ωú
	if (0 == upgrade_flags)//ÂºÄÂßãÂçáÁ∫ßÔºåÂéªÈô§ÂÖ∂‰ªñÊ®°Âùó
	{
		if (GetFwUpgradeStatus())
		{
			NK_JSONAPI_ASyncCall("/gui/*", "UpgradeProgress", 0, NULL);//ÊãâÂéªËøõÂ∫¶Êù°
			NK_JSONAPI_ASyncCall("/gui/*", "ManualRecordStop", 0, NULL);//ÂÅúÊ≠¢ÂΩïÂÉèÊ®°Âùó
			printf("%s() UpgradeProgress ... ddd\n", __FUNCTION__);
			upgrade_flags = 1;
		}
	}
	#endif
	
	if (0 == FifoTimeOut(read_fd, sec, us))
	{
		memset(&stCmd, 0, sizeof(FIFO_CMD_S));
		ret = FifoRead(read_fd,&stCmd);
		if (ret < 0)
		{
			printf("%s FifoRead ret = %d error\n", __FUNCTION__, ret);
			return -1;
		}
		
		switch (stCmd.u32CmdType)
		{
		case FIFO_CMD_APP_TO_UPGRADE_PROGRESS:
		case FIFO_CMD_UPGRADE_TO_APP_PROGRESS:
			break;
		default:
			break;
		}
		
		memcpy(pstCmd, &stCmd, sizeof(FIFO_CMD_S));
		
		return 0;
	}

	return -1;
}

//‰∏ªÂä®ÂçáÁ∫ßÊó∂ËøõË°åËøõÂ∫¶Êù°Âà∑Êñ∞
int AppToUpgradeUpdateProgress(int bBlock, unsigned int nTotalUpgradeSize, unsigned int *pUpgradeSize)
{	
	int ret;
	FIFO_CMD_S stCmd;
	unsigned int *pProgress;
	memset(&stCmd, 0, sizeof(FIFO_CMD_S));
	int sec = 0;
	int us = 1000;
	unsigned int nUpgradeSize = 0;
	unsigned int nDetalSize = 0;

	if (pUpgradeSize)
	{
		nUpgradeSize = *pUpgradeSize;
	}
	
	do 
	{
		if (0 == AppReadUpgradeStatus(1,  sec, us, &stCmd))
		{
			switch (stCmd.u32CmdType)
			{
			case FIFO_CMD_APP_TO_UPGRADE_PROGRESS:
			case FIFO_CMD_UPGRADE_TO_APP_PROGRESS:
				pProgress = (unsigned int *)stCmd.u8Cmd;
			
				if (*pProgress%10==0)
				{
					printf("app %d\n", *pProgress);
				}
				
				if (*pProgress > 0 && *pProgress <= 99)
				{
						
					if (*pProgress > 90)
					{
						sec = 0;
						us = 1;
					}
					
					nDetalSize = *pProgress * nTotalUpgradeSize / 100;
					if (pUpgradeSize)
					{
						*pUpgradeSize += nDetalSize - (*pUpgradeSize - nUpgradeSize);
					}
				}
				else if (*pProgress ==	100)
				{
					nDetalSize = *pProgress * nTotalUpgradeSize / 100;
					if (pUpgradeSize)
					{
						*pUpgradeSize += nDetalSize - (*pUpgradeSize - nUpgradeSize);
					}
					return 0;
				}
				else
				{
					return -1;
				}
				break;
			case FIFO_CMD_UPGRADE_SUCCESS:
				nDetalSize = nTotalUpgradeSize;
				if (pUpgradeSize)
				{
					*pUpgradeSize += nDetalSize - (*pUpgradeSize - nUpgradeSize);
				}
				return 0;
				break;
			case FIFO_CMD_VER_BIN_ERR:	
			case FIFO_CMD_VER_UPGRADE_FAIL: 
				return -2;
				break;
			default:
				ret = -3;
				break;
			}
		}
		
	}while(bBlock);
	
	return ret;
}

#ifdef MK_UPGRADE_APP

//‘§¡Ù∂‡œﬂ≥Ã÷¥––À¯
static pthread_mutex_t   		g_lock = PTHREAD_MUTEX_INITIALIZER;
#define UpgradeLock()  	 		(void)pthread_mutex_lock(&g_lock); 
#define UpgradeUnlock()    		(void)pthread_mutex_unlock(&g_lock); 

static DAEMON_UPGRADE_INFO g_UpgradeInfo;

int NK_MAIN_SUB_System(char *pCmd, int bBlocked);//–°Ω¯≥Ã π”√

void DoEncrypt(u8 *buf, u32 len)
{

}

s32 FlashOpen(const u8 *mtdname)
{
	s32 fd;

	if (NULL == mtdname)
	{
		return -1;
	}

	if (-1 == (fd = open((const char *)mtdname, O_RDWR)))
	{
		return -2;
	}
	
	return fd;
}


void FlashClose(s32 fd)
{
	close(fd);
}

s32 FlashErase(s32 fd, s32 start, s32 size)
{
	struct erase_info_user erase_info;

	erase_info.start = start;
	erase_info.length = size;
	if (ioctl(fd, MEMERASE, &erase_info) < 0)
	{
		perror("MEMERASE:");
		return -1;
	}

	return 0;

}

s32 FlashWrite(s32 fd, s32 start, s32 size, const u8 *buffer)
{
	s32 ret;
	if (-1 == lseek(fd, start, SEEK_SET))
	{
		perror("lseek:");
		return  -1;
	}
	ret = write(fd, buffer, size);
	if (ret < size)
	{
		//perror("write:");
		printf("ret =%d: start=%u, size=%u\n", ret, start, size);
		return -2;
	}

	return 0;
}

s32 FlashRead(s32 fd, s32 start, s32 size, u8 *buffer)
{
	
	if (-1 == lseek(fd, start, SEEK_SET))
	{
		return  -1;
	}

	if (read(fd, buffer, size) < size)
	{
		return -2;
	}

	return 0;
}


s32 FlashGetinfo(s32 fd, struct mtd_info_user *mtd_info)
{
	if (ioctl(fd, MEMGETINFO, mtd_info))
	{
		return -1;
	}
	
	return 0;
}

s32 GetMtdBlockNum(u32 *pMtdBlockNum)
{	
	s32 ret = -1;
	s32 i;
	s32 fd;
	u32 nPartCnt = 0;
	u32 nPartNum = 12;
	u8 mtd_name[32];

	if (pMtdBlockNum == NULL)
	{
		return -1;
	}
	
	for (i = 0; i < nPartNum; i++)
	{
		sprintf((char *)mtd_name, "/dev/mtd%d", i);//◊¢“‚/dev/mtdblockx ÷±Ω”∂¡–¥æÕø…“‘≤ª–Ë“™erase
		if (access((const char *)mtd_name, 0) != 0)
		{
			break;
		}
		
		fd = FlashOpen(mtd_name);
		if (fd < 0)
		{
			break;
		}
		
		FlashClose(fd);
		nPartCnt++;
		ret = 0;
	}
	
	*pMtdBlockNum = nPartCnt;
	
	return ret;
}


s32 CheckFlashSize(UPGRADE_VER_SIZE_INFO *pstUpgradeInfo)//ºÚµ•ºÏ≤È∑÷«¯ «∑Ò∂‘£¨»Áπ˚∑÷«¯¥Û–°º”∆¿¥≤ª «≥£”√flash¥Û–°£¨ƒ«øœ∂® «flash¥Û–°À„¥Ì¡À
{
	int i;
	int ret;
	u32 size = 0;
	int fd;
	char mtd_name[64];
	int upgrade_mtd_idx = -1;
	struct mtd_info_user mtdinfo;
	unsigned int mtdsize = 0;
	int nmtd_max_cnt = 0;
	unsigned int upgrade_mtd_rom_size; 

	if (pstUpgradeInfo == NULL)
	{
		return -1;
	}
	
	ret = GetMtdBlockNum(&nmtd_max_cnt);
	if (ret != 0)
	{
		return ret;
	}

	for (i = 0; i < pstUpgradeInfo->u32UpgradeMtdCnt; i++)
	{
		upgrade_mtd_idx = pstUpgradeInfo->stBlockInfo[i].u32Idx;
		upgrade_mtd_rom_size = pstUpgradeInfo->stBlockInfo[i].u32VerSize;
		if (upgrade_mtd_idx > nmtd_max_cnt)
		{
			printf("upgrade_mtd_idx = %d > nmtd_max_cnt = %d\n", upgrade_mtd_idx,  nmtd_max_cnt);
			return -1;
		}
	
		sprintf((char *)mtd_name, "/dev/mtd%d", upgrade_mtd_idx);//◊¢“‚/dev/mtdblockx ÷±Ω”∂¡–¥æÕø…“‘≤ª–Ë“™erase
		fd = FlashOpen(mtd_name);
		if (fd < 0)
		{
			ret = -50;
			return ret;
		}
			
		ret = FlashGetinfo(fd, &mtdinfo);
		if (ret != 0 || mtdinfo.erasesize > 512*1024)//ƒø«∞øº¬«erasesize◊Ó¥ÛŒ™512K
		{
			FlashClose(fd);
			ret = -51;
			return ret;
		}
		
		mtdsize 	= mtdinfo.size;	/// µ•∏ˆmtdµƒ◊‹¥Û–°
		if (upgrade_mtd_rom_size > mtdsize)
		{
			printf("upgrade_mtd_idx=%d mtdsize = %d < upgrade_mtd_rom_size = %d too big\n", upgrade_mtd_idx, mtdsize, upgrade_mtd_rom_size);
			return -1;
		}
	}
	
	return 0;
}

//‘§¡Ù∂‡œﬂ≥Ã÷¥––À¯
static pthread_mutex_t   g_Memmaplock = PTHREAD_MUTEX_INITIALIZER;
#define UpgradeMemmapLock()  	 (void)pthread_mutex_lock(&g_Memmaplock);
#define UpgradeMemmapUnlock()   (void)pthread_mutex_unlock(&g_Memmaplock);  

//Ω®“È1page=4096Bytes ∂‘∆Î≤Ÿ◊˜œµÕ≥ «∞¥“≥∑÷≈‰
void *NK_UpgradeMemmap(unsigned int phy_addr, unsigned int phy_size)
{
	int mem_fd;
	void *pVirAddr = NULL;
	unsigned int phy_addr_in_page;
	unsigned int page_diff;
	unsigned int size_in_page;

	UpgradeMemmapLock();

	mem_fd = open (MEM_DEV, O_RDWR | O_NONBLOCK | O_SYNC); /*without cache*/
	if (mem_fd < 0)
	{
		printf("memmap():open %s error!\n", MEM_DEV);
		UpgradeMemmapUnlock();
		return NULL;
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
	page_diff = phy_addr - phy_addr_in_page;

	/* size in page_size */
	size_in_page =((phy_size + page_diff - 1) & PAGE_SIZE_MASK) + MEM_PAGE_SIZE;
	printf("phy_addr_in_page=0x%x  phy_addr=0x%x  delta=%d phy_size = %d, page_diff=%d size_in_page=%d\n", phy_addr_in_page, phy_addr, phy_addr_in_page - phy_addr,  phy_size, page_diff, size_in_page);

	pVirAddr = mmap ((void *)NULL, size_in_page, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (long)phy_addr_in_page);
	if (pVirAddr == MAP_FAILED)
	{
		UpgradeMemmapUnlock();
		printf("mmap @ 0x%x error!\n", phy_addr_in_page);
		return NULL;
	}

	if (mem_fd > 0)
	{ 
		close(mem_fd);
		mem_fd = -1;
	}
	
	UpgradeMemmapUnlock();
	
    return (void *)((unsigned int)pVirAddr+page_diff);
}

int NK_UpgradeMunmap(void *pVirAddr, unsigned int size)
{
	int ret;
	void *addr;

	addr = (void *)((unsigned int)pVirAddr & PAGE_SIZE_MASK);
	ret =  munmap(addr, size);
	if (ret != 0)
	{
		printf("munmap ret = %d\n", ret);
	}
	printf("%s() munmap  ret = %d\n", __FUNCTION__, ret);
	return ret;
}


//∞Ê±æºÏ≤‚:∞¸¿®∞Ê±æ∫≈£¨ ˝æ›£¨–£—È∫Õ£¨“‘º∞ ˝æ›Ω‚√‹µ»
s32 FwCheckAndUnEncData(u8 *pdata, u32 len)
{
	return 0;
}

#define MTDBLK_PREFIX "/dev/mtdblock"
#define FLASH_MTD_BLOCK	(64*1024)
char g_u8KernelBak[FLASH_MTD_BLOCK];
static int BackupKernel(unsigned int nMtdIdx)
{
	int nRet = 0;
	int nKernel = -1;
	int const nKernelBkSize = sizeof(g_u8KernelBak);
	unsigned char* pu8Demage = NULL;
	char MtdPath[128];

	if (nMtdIdx <= 2) //ÈÅøÂÖçuboot Áõ∏ÂÖ≥ÂåÖÂê´ÂàÜÂå∫
	{
		printf("%s idx = %d error\n", __FUNCTION__, nMtdIdx);
		return -1;
	}
	snprintf(MtdPath, sizeof(MtdPath), "%s%d", MTDBLK_PREFIX, nMtdIdx);

	if(access(MtdPath, 0) != 0)
	{
		printf("update %s never beginned fail\n", MtdPath);
		return -1;
	}
	
	nKernel = open(MtdPath, O_RDWR);
	if(nKernel < 0)
	{
		printf("open %s error\n", __FUNCTION__);
		return -2;
	}
	
	nRet = read(nKernel, g_u8KernelBak, nKernelBkSize);
	if(nKernelBkSize != nRet)
	{
		printf("read %s error\n", __FUNCTION__);
		close(nKernel);
		return -3;
	}
	
	// demage kernel block
	pu8Demage = (unsigned char*)(alloca(nKernelBkSize));
	if (pu8Demage == NULL)
	{
			return -4;
	}
	memset(pu8Demage, 0, nKernelBkSize);
	// resart from beginning
	lseek(nKernel, 0, SEEK_SET);
	nRet = write(nKernel, pu8Demage, nKernelBkSize);
	fsync(nKernel);
	close(nKernel);
	nKernel = -1;
	
	printf("backup kernel size = %d\n", nKernelBkSize);
	return 0;
}

static int RecoverKernel(unsigned int nMtdIdx)
{
	int nRet = 0;
	int fdKernel = -1;
	int const nKernelBkSize = sizeof(g_u8KernelBak);
	char MtdPath[128];
	
	if (nMtdIdx <= 2) //ÈÅøÂÖçuboot Áõ∏ÂÖ≥ÂåÖÂê´ÂàÜÂå∫
	{
		printf("%s idx = %d error\n", __FUNCTION__, nMtdIdx);
		return -1;
	}
	
	snprintf(MtdPath, sizeof(MtdPath), "%s%d", MTDBLK_PREFIX, nMtdIdx);
	
	if(access(MtdPath, 0) != 0)
	{
		printf("RecoverKernel update %s never beginned fail\n", MtdPath);
		return -1;
	}
	
	fdKernel = open(MtdPath, O_WRONLY);
	if(fdKernel < 0)
	{
		printf("open %s error\n", __FUNCTION__);
		return -2;
	}
	
	nRet = write(fdKernel, g_u8KernelBak, nKernelBkSize);
	assert(nKernelBkSize == nRet);
	fsync(fdKernel);
	close(fdKernel);
	
	printf("recover kernel size = %d OK!\n", nKernelBkSize);
	
	return 0;
}

#define RES_MTDBLOCK_IDX    6

s32 NK_FwGetUpgradeData(u8 *pdata, u32 len, UPGRADE_VER_SIZE_INFO *pstUpgradeInfo, NK_FUNC_T pFunc)
{
	s32 ret = -1;
	s32 fd;
	s32 ret_size;
	u32 size;
	u32 i, j, k;
	u32 bBlockPad = 0;//Œƒº˛ £”‡–Ë“™–¥»Îµƒ «∑Ò «blockµƒ’˚ ˝±∂
	u32 nWriteSize = 0;//Œƒº˛¿€º∆–¥»Î¥Û–°
	u32 mtdsize;//∑÷«¯¥Û–°
	u32 erasesize;//≤¡≥˝block¥Û–°
	struct mtd_info_user mtdinfo;
	u8 mtd_name[32];
	FILE *pf = NULL;
	u8 filename[128];
	u8 *ptmp;
	u8 *pblock = NULL;
	u32 loop_cnt;
	u32 last_mtd_id = 0;//◊Ó∫Û“ª∏ˆ∑÷«¯id ∑÷«¯±£¡Ù
    u32 persent_size = 0;
    u32 persent = 0;
    u32 bStartUpgrade = 0;
	u32 u32UpgradeBlockSize = 0;
	u16 u16UpgradeCrc = 0;
	u16 u16UpgradeTmpCrc = 0;
	u8 *pResMtdblockTmp = NULL;

	ret = CheckFlashSize(pstUpgradeInfo);//…˝º∂◊Ó∫Û“ª∏ˆ∑÷«¯
	if(ret != 0)
	{
		return ret;
	}
	
#ifdef KERNEL_RECOVER
	ret = BackupKernel(pstUpgradeInfo->u32KernelMtdIdx);
	if (ret != 0)
	{
		return ret;
	}
#endif
	for (i = 0; i < pstUpgradeInfo->u32UpgradeMtdCnt; i++)//◊¢“‚±£¡Ù∑÷«¯≤ªƒ‹≤¡≥˝
	{
		last_mtd_id = pstUpgradeInfo->stBlockInfo[i].u32Idx;
		ptmp = pdata + pstUpgradeInfo->stBlockInfo[i].u32Offset;
		memset(mtd_name, 0, sizeof(mtd_name));
		u32UpgradeBlockSize = pstUpgradeInfo->stBlockInfo[i].u32VerSize;

		sprintf((char *)mtd_name, "/dev/mtd%d", last_mtd_id);//◊¢“‚/dev/mtdblockx ÷±Ω”∂¡–¥æÕø…“‘≤ª–Ë“™erase
		fd = FlashOpen(mtd_name);
		if (fd < 0)
		{
			ret = -50;
			goto end;
		}
		
		ret = FlashGetinfo(fd, &mtdinfo);
		if (ret != 0 || mtdinfo.erasesize > 512*1024)//ƒø«∞øº¬«erasesize◊Ó¥ÛŒ™512K
		{
			FlashClose(fd);
			ret = -51;
			goto end;
		}
		mtdsize 	= mtdinfo.size;	/// µ•∏ˆmtdµƒ◊‹¥Û–°
		erasesize	= mtdinfo.erasesize;///≤¡≥˝block¥Û–°
		if (u32UpgradeBlockSize > mtdsize || mtdinfo.erasesize > 512*1024)//ƒø«∞øº¬«erasesize◊Ó¥ÛŒ™512K
		{
			FlashClose(fd);
			ret = -52;
			printf("mtd_name = %s len = %d , mtdsize = %d, mtdinfo.erasesize = %d\n",mtd_name, u32UpgradeBlockSize , mtdsize, mtdinfo.erasesize);
			goto end;
		}
		
		if (NULL == pblock)
		{
			pblock = malloc(erasesize);
			if (NULL == pblock)
			{
				FlashClose(fd);
				ret = -53;
				goto end;
			}
			memset(pblock, 0xFF, erasesize);
		}

        if (last_mtd_id == RES_MTDBLOCK_IDX)
        {
            u16UpgradeCrc = NK_LIB_DoCrc16(ptmp, u32UpgradeBlockSize);
            system("echo 3 > /proc/sys/vm/drop_caches");
            pResMtdblockTmp = malloc(u32UpgradeBlockSize);
            if (pResMtdblockTmp == NULL)
            {
                pResMtdblockTmp = NULL;
                printf("pResMtdblockTmp malloc u32UpgradeBlockSize = %d fail\n", u32UpgradeBlockSize);
            }
            else
            {
               memcpy(pResMtdblockTmp, ptmp, u32UpgradeBlockSize);
            }
        }

		loop_cnt = ((u32UpgradeBlockSize + (erasesize - 1)) & ~(erasesize - 1))/erasesize;
		
		memset(pblock, 0xFF, erasesize);
		nWriteSize = 0;
		for(j = 0; j < loop_cnt; j++)
		{
			bStartUpgrade = 1;
			if (nWriteSize + erasesize <= u32UpgradeBlockSize)
			{
				bBlockPad = 1;
				nWriteSize += erasesize;
			}
			else 
			{
				bBlockPad = 0;
			}

			if (bBlockPad)
			{
				FlashRead(fd, j*erasesize, erasesize, pblock);         
				if(0 != memcmp(pblock, ptmp, erasesize))
				{
					//printf("Writing the %dth Block of %s in flash.\n", j, mtd_name);
					//printf(".");
					if (FlashErase(fd, j*erasesize, erasesize) < 0)

					{
						printf("Erase flash failed.\n");
						FlashClose(fd);
						ret = -55;
						goto end;
					}

					if(FlashWrite(fd, j*erasesize, erasesize, (const u8*)ptmp) < 0)

					{
						//printf("write flash failed j=%d, 0x%x, 0x%x loop_cnt=%d pheader->mtdinfo[%d].u32size=%u\n", j, j*erasesize, erasesize, loop_cnt, i, pheader->mtdinfo[i].u32size);
						FlashClose(fd);
						ret = -56;
						goto end;
					}
				}
			//	else
				{    
					//printf("The %dth  Block of %s in flash is the same to file.\n", j, mtd_name);
				} 
				
				ptmp += erasesize;			
				persent_size += erasesize; 
			}
			else //◊Ó∫Û“ª∏ˆblock£¨ £”‡µƒπª“ª∏ˆblock≥§∂»£¨÷ª»°–Ë“™µƒ≤ø∑÷
			{
				memset(pblock, 0xFF, erasesize);
				memcpy(pblock, ptmp, u32UpgradeBlockSize - nWriteSize);
				if (FlashErase(fd, j*erasesize, erasesize) < 0)
				{
					printf("Erase flash failed.\n");
					FlashClose(fd);
					ret = -65;
					goto end;
				}
		
				if(FlashWrite(fd, j*erasesize, erasesize, (const u8*)pblock) < 0)//’‚¿Ô≤ªƒ‹ «ptmp£¨ª·‘ΩΩÁµƒ
				{
					//printf("write flash failed j=%d, 0x%x, 0x%x loop_cnt=%d pheader->mtdinfo[%d].u32size=%u\n", j, j*erasesize, erasesize, loop_cnt, i, pheader->mtdinfo[i].u32size);
					FlashClose(fd);
					ret = -66;
					goto end;
				}
				
				persent_size += (u32UpgradeBlockSize - nWriteSize); 
			}
            //printf("len = %u, ppersent_size = %u", len, *ppersent_size);
			
            if (pFunc)
            {
            	persent = (persent_size*100)/len;
                if(persent > 0 && persent < 100)
				{
					pFunc(0, persent);
				}
            }
			if (persent%10==0)
			{
				printf("upgrade %d%%\n", persent);
				UPGRADE_CGI_updateRate(persent);
			}

		}

		FlashClose(fd);

		#ifdef KERNEL_RECOVER
		if (pstUpgradeInfo->u32KernelMtdIdx == last_mtd_id)
		{
			ret = BackupKernel(pstUpgradeInfo->u32KernelMtdIdx);
			if (ret != 0)
			{
				ret = -40;
				goto end;
			}
		}
		#endif

	}
	
#ifdef KERNEL_RECOVER
	RecoverKernel(pstUpgradeInfo->u32KernelMtdIdx);
#endif

    for (i = 0; i < pstUpgradeInfo->u32UpgradeMtdCnt; i++)//◊¢“‚±£¡Ù∑÷«¯≤ªƒ‹≤¡≥˝
    {
        last_mtd_id = pstUpgradeInfo->stBlockInfo[i].u32Idx;
        if (last_mtd_id == RES_MTDBLOCK_IDX)
        {
            ptmp = pdata + pstUpgradeInfo->stBlockInfo[i].u32Offset;
            u32UpgradeBlockSize = pstUpgradeInfo->stBlockInfo[i].u32VerSize;
            u16UpgradeTmpCrc = NK_LIB_DoCrc16(ptmp, u32UpgradeBlockSize);
            if (u16UpgradeCrc != u16UpgradeTmpCrc && pResMtdblockTmp != NULL)
            {
                ptmp = pResMtdblockTmp;
                printf("u16UpgradeCrc  0x%x != u16UpgradeTmpCrc = 0x%x\n", u16UpgradeCrc, u16UpgradeTmpCrc);
                sprintf((char *)mtd_name, "/dev/mtd%d", last_mtd_id);//◊¢“‚/dev/mtdblockx ÷±Ω”∂¡–¥æÕø…“‘≤ª–Ë“™erase
                fd = FlashOpen(mtd_name);
                if (fd < 0)
                {
                    ret = -50;
                    goto end;
                }

                ret = FlashGetinfo(fd, &mtdinfo);
                if (ret != 0 || mtdinfo.erasesize > 512*1024)//ƒø«∞øº¬«erasesize◊Ó¥ÛŒ™512K
                {
                    FlashClose(fd);
                    ret = -51;
                    goto end;
                }
                mtdsize     = mtdinfo.size; /// µ•∏ˆmtdµƒ◊‹¥Û–°
                erasesize   = mtdinfo.erasesize;///≤¡≥˝block¥Û–°
                if (u32UpgradeBlockSize > mtdsize || mtdinfo.erasesize > 512*1024)//ƒø«∞øº¬«erasesize◊Ó¥ÛŒ™512K
                {
                    FlashClose(fd);
                    ret = -52;
                    printf("mtd_name = %s len = %d , mtdsize = %d, mtdinfo.erasesize = %d\n",mtd_name, u32UpgradeBlockSize , mtdsize, mtdinfo.erasesize);
                    goto end;
                }

                if (NULL == pblock)
                {
                    pblock = malloc(erasesize);
                    if (NULL == pblock)
                    {
                        FlashClose(fd);
                        ret = -53;
                        goto end;
                    }
                    memset(pblock, 0xFF, erasesize);
                }

                loop_cnt = ((u32UpgradeBlockSize + (erasesize - 1)) & ~(erasesize - 1))/erasesize;

                memset(pblock, 0xFF, erasesize);
                nWriteSize = 0;
                for(j = 0; j < loop_cnt; j++)
                {
                    bStartUpgrade = 1;
                    if (nWriteSize + erasesize <= u32UpgradeBlockSize)
                    {
                        bBlockPad = 1;
                        nWriteSize += erasesize;
                    }
                    else
                    {
                        bBlockPad = 0;
                    }

                    if (bBlockPad)
                    {
                        FlashRead(fd, j*erasesize, erasesize, pblock);
                        if(0 != memcmp(pblock, ptmp, erasesize))
                        {
                            //printf("Writing the %dth Block of %s in flash.\n", j, mtd_name);
                            printf(".");
                            if (FlashErase(fd, j*erasesize, erasesize) < 0)
                            {
                                printf("Erase flash failed.\n");
                                FlashClose(fd);
                                ret = -55;
                                goto end;
                            }

                            if(FlashWrite(fd, j*erasesize, erasesize, (const u8*)ptmp) < 0)

                            {
                                //printf("write flash failed j=%d, 0x%x, 0x%x loop_cnt=%d pheader->mtdinfo[%d].u32size=%u\n", j, j*erasesize, erasesize, loop_cnt, i, pheader->mtdinfo[i].u32size);
                                FlashClose(fd);
                                ret = -56;
                                goto end;
                            }
                        }
                    //  else
                        {
                            //printf("The %dth  Block of %s in flash is the same to file.\n", j, mtd_name);
                        }

                        ptmp += erasesize;
                        persent_size += erasesize;
                    }
                    else //◊Ó∫Û“ª∏ˆblock£¨ £”‡µƒπª“ª∏ˆblock≥§∂»£¨÷ª»°–Ë“™µƒ≤ø∑÷
                    {
                        memset(pblock, 0xFF, erasesize);
                        memcpy(pblock, ptmp, u32UpgradeBlockSize - nWriteSize);
                        if (FlashErase(fd, j*erasesize, erasesize) < 0)
                        {
                            printf("Erase flash failed.\n");
                            FlashClose(fd);
                            ret = -65;
                            goto end;
                        }

                        if(FlashWrite(fd, j*erasesize, erasesize, (const u8*)pblock) < 0)//’‚¿Ô≤ªƒ‹ «ptmp£¨ª·‘ΩΩÁµƒ
                        {
                            //printf("write flash failed j=%d, 0x%x, 0x%x loop_cnt=%d pheader->mtdinfo[%d].u32size=%u\n", j, j*erasesize, erasesize, loop_cnt, i, pheader->mtdinfo[i].u32size);
                            FlashClose(fd);
                            ret = -66;
                            goto end;
                        }

                        persent_size += (u32UpgradeBlockSize - nWriteSize);
                    }
                    //printf("len = %u, ppersent_size = %u", len, *ppersent_size);
                }

            }
            else
            {
                printf("OK\n");
            }

            break;
        }//mtdblock res
    }



    if (pFunc)
    {   
       pFunc(0, 100);
    }

	return 0;

end:
	if (pblock != NULL)
	{
		free(pblock);
	}
	
    if (pFunc)
    {   
        pFunc(UPGRADE_FILE_ROM_FAIL, 0);
    }

	if (0 == bStartUpgrade)//ËøòÊ≤°ÊúâÂºÄÂßãÂçáÁ∫ßÔºåÂè™ÊòØÊ£ÄÊü•‰∏çËøáÔºåÂàôÊÅ¢Â§ç
	{
		RecoverKernel(pstUpgradeInfo->u32KernelMtdIdx);
	}
	
	return  ret;
}


static void _sig_proc_kill(int sig)
{
}

int NK_MAIN_SUB_System(char *pCmd, int bBlocked)//–°Ω¯≥Ã π”√
{
	int ret;
	int pid; 
	int status_tmp;
	
	if (pCmd == NULL)
	{
		return -1;
	}
	
	pid = vfork();
	if (pid == 0) //◊”Ω¯≥Ã
	{	
		signal(SIGINT, _sig_proc_kill);
		ret = execl("/bin/sh", "sh", "-c", pCmd, (char *)0);
		return ret;
	}
	
	waitpid(pid, &status_tmp, 0);

	return 0;
}


int NK_FreeCacheMem(void)
{
	int ret;
	int fd;
	char buf[32] = "1"; 
	char *drop_file = "/proc/sys/vm/drop_caches";
	
	fd = open(drop_file, O_RDWR, 0666);
	if (fd < 0)
	{
		perror("open");
		printf("%s() line:%d error drop_file=%s\n", __FUNCTION__, __LINE__, drop_file);
		return -1;
	}
	
	#if 0
	ret = read(fd, buf, 2);
	if (ret < 0)
	{
		perror("read");
	}
	printf("ret = %d fd=%d %c\n", ret, fd, buf[0]);
	#endif
	
	buf[0] = '1';
	buf[1] = '\0';
	ret = write(fd, buf, 2);
	if (ret < 0)
	{
		perror("write");
		NK_MAIN_SUB_System("echo 3 > /proc/sys/vm/drop_caches", 1);
	}

	close(fd);
	
	return 0;
}

int NK_GetFreeCacheMemSize(unsigned int *pFreeSize)
{
	int ret;
	struct sysinfo s_info;

	if (NULL == pFreeSize)
	{
		return -2;
	}
	
	ret = sysinfo(&s_info);
	if (ret != 0)
	{
		perror("sysinfo");
		return -1;
	}
	
	*pFreeSize = (unsigned int)s_info.freeram;

	return ret;
}


int UpgradeProgress(int nRetcode, int nProgress)
{
	DAEMON_UPGRADE_INFO *pUpgradeInfo;
	
	pUpgradeInfo = &g_UpgradeInfo;

	pUpgradeInfo->bUpgradeProgress = nProgress;
	
	return 0;
}

int FwUpgradeReadRomFile(char *pFileName, char *pbuf, unsigned int nBufSize)
{
	s32 size;
	s32 buf_size;
	s32 ret_size;
	s32 fd = -1;
	s32 ret = -1;
	//u8 *pbuf;
	u8 *ptmp;
	FILE *pfile = NULL;


	if (access(pFileName, 0) != 0)
	{
		printf("%s() line:%d error! %s not exist\n", __FUNCTION__, __LINE__,pFileName);
		ret = -2;
		goto exit;
	}
	
	fd = open(pFileName, O_RDONLY);
	if (fd < 0)
	{
		printf("%s() line:%d error! %s open error\n", __FUNCTION__, __LINE__,pFileName);
		ret = -3;
		goto exit;
	}
	
	size = lseek(fd, 0,SEEK_END);
	if (size <= 0 || size > nBufSize)
	{
		printf("%s() line:%d error! too big %u\n", __FUNCTION__, __LINE__, size);
		ret = -4;
		goto exit;
	}
	close(fd);	
	fd = -1;

	memset(pbuf, 0,  size);

	pfile = fopen(pFileName, "rb");
	if (pfile == NULL)
	{
		printf("%s() line:%d error! %s fopen error\n", __FUNCTION__, __LINE__, pFileName);
		ret = -5;
		goto exit;
	}
	
	ptmp  = pbuf;
	buf_size = size;
	do
	{
		ret_size =  fread(ptmp, 1, buf_size, pfile);
		if (ret_size < 0)
		{
			ret = -6;
			goto exit;
		}
		ptmp += ret_size;

		buf_size -= ret_size;
		
	} while(buf_size > 0);

	fclose(pfile);
	pfile = NULL;
	return 0;	
	
exit:

	if (fd > 0)
	{
		close(fd);
	}
	
	if (pfile)
	{
		fclose(pfile);
	}

	if (pbuf)
	{
		free(pbuf);
	}

	return ret;
}

unsigned int GetAppStartTime(void)//ø™ª˙ ±º‰
{
	struct timespec sys_time = {0, 0};
	unsigned int nSrvTimeBaseSec = 0;//øÕªß∂ÀªÚ∑˛ŒÒ∆˜ø™ª˙ª˘¥° ±º‰
	
	clock_gettime(CLOCK_MONOTONIC, &sys_time);
	
	nSrvTimeBaseSec = (unsigned int)sys_time.tv_sec;
	if (nSrvTimeBaseSec <= 0)
	{
		nSrvTimeBaseSec = (unsigned int)&sys_time;//ªÒ»°“ª∏ˆ’ªµƒµÿ÷∑ œ‡µ±”⁄ «“ª∏ˆ∫‹¥Ûµƒ ±º‰
	}
	
	//printf("nSrvTimeBaseSec = %d\n", nSrvTimeBaseSec);
	
	return nSrvTimeBaseSec;
}

void *UpgradeCommunicateProc(void *args)
{
	int i;
	int ret;
	int nProgress = -1;
	int *pProgress = NULL;
	DAEMON_UPGRADE_INFO *pUpgradeInfo = (DAEMON_UPGRADE_INFO *)args;
	int src_fd = -1; //"/tmp/up_to_app_fifo"
	int client_fd = -1;//"/tmp/app_to_up_fifo"
	FIFO_CMD_S cmd;
	FIFO_CMD_S ack;
	PHY_ADDR_INFO *pstAddrInfo;
	VER_SIZE_INFO *pstSizeInfo;
	UPGRADE_VER_SIZE_INFO *pstCmdUpgradeInfo;
	fd_set fd_rs;
	int maxfd;
	struct timeval timeout={1,3}; //selectµ»¥˝3Œ¢√Î£¨3Œ¢√Î¬÷—Ø£¨“™∑«◊Ë»˚æÕ÷√0
	char *pChVirAddr = NULL;
	unsigned int ncrc = 0;
	static char *pVirAddr = NULL;
	static unsigned int nPhySize = 0;
	
	if (access(FIFO_SRV_NAME, F_OK) == -1)	
	{  
		ret = mkfifo(FIFO_SRV_NAME,  0777);	
		if (ret != 0)  
		{  
			printf("Could not create fifo %s \n", FIFO_SRV_NAME);  
			goto EXIT;
		}  
	}  
	
	if (access(FIFO_CLIENT_NAME, F_OK) == -1)	
	{  
		ret = mkfifo(FIFO_CLIENT_NAME,  0777);	
		if (ret != 0)  
		{  
			printf("Could not create fifo %s \n", FIFO_CLIENT_NAME);  
			goto EXIT;
		}  
	}  
	
	src_fd = open(FIFO_SRV_NAME, O_RDWR, 0); //◊Ë»˚
	if (src_fd < 0)
	{
		printf("Could not open fifo %s \n", FIFO_CLIENT_NAME);  
		goto EXIT;
	}


	client_fd = open(FIFO_CLIENT_NAME, O_RDWR, 0);//◊Ë»˚
	if (client_fd < 0)
	{
		printf("Could not open fifo %s \n", FIFO_CLIENT_NAME);  
		goto EXIT;
	}

	while (1)
	{
		if (0 == pUpgradeInfo->bUpgrade)
		{
			timeout.tv_sec = 0;
			timeout.tv_usec = 2000; //2ms
			FD_ZERO(&fd_rs); //√ø¥Œ—≠ª∑∂º“™«Âø’ºØ∫œ£¨∑Ò‘Ú≤ªƒ‹ºÏ≤‚√Ë ˆ∑˚±‰ªØ
			
			FD_SET(client_fd, &fd_rs); //ÃÌº”√Ë ˆ∑˚
			FD_SET(src_fd, &fd_rs); //ÃÌº”√Ë ˆ∑˚
			maxfd = client_fd;
			if (src_fd > maxfd)
			{
				maxfd = src_fd;
			}
			ret = select(maxfd + 1, &fd_rs, NULL, NULL, &timeout);
			switch(ret) //select π”√ read
			{
			case -1://≥ˆ¥Ì
			case 0://≥¨ ±		
				usleep(100000);
				//printf("select:line:%d %s\n", __LINE__, ret==0 ? "timeout": "error");
				continue;
			default://ok ”––Ë“™∂¡»° ˝æ›		
				break;
			}

			if(FD_ISSET(client_fd, &fd_rs))
			{
				printf("%s() FifoRead IN line:%d\n", __FUNCTION__, __LINE__);
				if (0 == pUpgradeInfo->bUpgrade)
				{
					UpgradeLock();
					memset(&cmd, 0, sizeof(cmd));
					ret = FifoRead(client_fd, &cmd);
					if (ret <= 0)
					{
						usleep(100000);
						UpgradeUnlock();
						continue;
					}
	
					memcpy(&ack, &cmd, sizeof(ack));
					ack.u32CmdType = FIFO_CMD_ACK;
					ret =FifoWrite(src_fd, &ack);
					if (ret <= 0)
					{
						usleep(100000);
						UpgradeUnlock();
						continue;
					}
					
					if (cmd.u32CmdType == FIFO_CMD_PHY_ADDR_MMAP)
					{
						pstAddrInfo = (PHY_ADDR_INFO *)cmd.u8Cmd;
						//printf("pstAddrInfo->u32PhyAddr = 0x%x, %u\n", pstAddrInfo->u32PhyAddr, pstAddrInfo->u32PhyLen);
						//pVirAddr = (char *)mmap((void *)pstAddrInfo->u32PhyAddr, pstAddrInfo->u32PhyLen, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE | MAP_SHARED, -1, 0);
						pVirAddr = (char *)NK_UpgradeMemmap(pstAddrInfo->u32PhyAddr, pstAddrInfo->u32PhyLen);
						if (pVirAddr == NULL || pVirAddr == (char *)(-1))
						{
							printf("pVirAddr == %p error\n", pVirAddr);
							nPhySize = 0;
							pVirAddr = NULL;
						}
						else
						{
							nPhySize = pstAddrInfo->u32PhyLen;
							printf("pVirAddr == %p OK nPhySize = %d\n", pVirAddr, nPhySize);
						}
					}
					else if (cmd.u32CmdType == FIFO_CMD_PHY_ADDR_MUNMAP)
					{
						if (pVirAddr && nPhySize > 0)
						{
							NK_UpgradeMunmap(pVirAddr, nPhySize);
							pVirAddr = NULL;
							nPhySize = 0;
						}
					}
					else if (cmd.u32CmdType == FIFO_CMD_UPGRADE_START)
					{
						pstCmdUpgradeInfo = (UPGRADE_VER_SIZE_INFO *)cmd.u8Cmd;
						printf("FIFO_CMD_UPGRADE_START pVirAddr == %p nPhySize=%d \n", pVirAddr, nPhySize);
						if (pVirAddr != NULL && pstCmdUpgradeInfo->u32TotalVerSize < MAX_FLASH_SIZE && nPhySize < MAX_FLASH_SIZE && pstCmdUpgradeInfo->u32UpgradeMtdCnt <= MTD_UPRADE_MAX_CNT)
						{
							pChVirAddr = NULL;
							for (i = 0; i < pstCmdUpgradeInfo->u32UpgradeMtdCnt; i++)
							{
								if (pstCmdUpgradeInfo->stBlockInfo[i].u32Offset >= 0 && pstCmdUpgradeInfo->stBlockInfo[i].u32Offset < nPhySize)
								{
									pChVirAddr = pVirAddr + pstCmdUpgradeInfo->stBlockInfo[i].u32Offset;
									ncrc = NK_LIB_DoCrc16(pChVirAddr, pstCmdUpgradeInfo->stBlockInfo[i].u32VerSize);
									if (ncrc != pstCmdUpgradeInfo->stBlockInfo[i].u32crc)
									{
										pChVirAddr = NULL;
										printf("*pOffset = %d error\n", pstCmdUpgradeInfo->stBlockInfo[i].u32Offset);
										pChVirAddr = NULL;
										break;
									}
								}
							}

					
							if (pChVirAddr != NULL && pstCmdUpgradeInfo->u32TotalVerSize > 0 && pstCmdUpgradeInfo->u32TotalVerSize <= nPhySize)
							{
								pUpgradeInfo->bUpgrade = 1;
								pUpgradeInfo->bUpgradeType = 0;
								pUpgradeInfo->pVirAddr = pVirAddr;//ËøôÈáåÊòØÂü∫Âú∞ÂùÄ
								pUpgradeInfo->nVirSize = pstCmdUpgradeInfo->u32TotalVerSize;
								pUpgradeInfo->bUpgradeProgress = 0;
								memcpy(&pUpgradeInfo->stUpgradeInfo, pstCmdUpgradeInfo, sizeof(UPGRADE_VER_SIZE_INFO));
								printf("u32TotalVerSize = %d\n", pstCmdUpgradeInfo->u32TotalVerSize);
							}
							else
							{
								memset(&cmd, 0, sizeof(cmd));
								cmd.u32CmdType = FIFO_CMD_VER_BIN_ERR;
								FifoWrite(src_fd, &cmd);
							}
						}
						else
						{
							printf("%s() line:%d error \n", __FUNCTION__, __LINE__);
							memset(&cmd, 0, sizeof(cmd));
							cmd.u32CmdType = FIFO_CMD_VER_BIN_ERR;
							FifoWrite(src_fd, &cmd);
						}
					}
					UpgradeUnlock();
				}	
			}
		}
		else //’˝‘⁄…˝º∂
		{
			if (nProgress != pUpgradeInfo->bUpgradeProgress && pUpgradeInfo->bUpgradeProgress > 0)
			{
				memset(&cmd, 0, sizeof(cmd));
				if (pUpgradeInfo->bUpgradeProgress == UPGRADE_PROGRESS_EROR)
				{
					nProgress = 0;//
				}
				else
				{
					nProgress = pUpgradeInfo->bUpgradeProgress;
				}
								
				pProgress = (int *)cmd.u8Cmd;
				*pProgress = nProgress;
				if (pUpgradeInfo->bUpgradeType)//upgrade∑¢∆µƒ…˝º∂
				{
					cmd.u32CmdType = FIFO_CMD_UPGRADE_TO_APP_PROGRESS;
				}
				else
				{
					cmd.u32CmdType = FIFO_CMD_APP_TO_UPGRADE_PROGRESS;
				}
				ret =FifoWrite(src_fd, &cmd);
				if (ret <= 0)
				{
					printf("%s() line:%d error\n", __FUNCTION__, __LINE__);
					usleep(100000);
				}
			}
			else
			{
				if (nProgress < 95)
				{
					usleep(100000);
				}
			}
		}
	}

EXIT:

	if (src_fd > 0)
	{
		close(src_fd);
	}

	
	if (src_fd > 0)
	{
		close(client_fd);
	}
	
	pthread_exit(NULL);	return NULL;
}

void *Upgrade_WdtActiveProc(void *args)
{	
	int i;
	int nWdtActiveTime = 30;
	int nSleepTime = nWdtActiveTime/3;
	int nEnable = 1;
	DAEMON_UPGRADE_INFO *pUpgradeInfo = (DAEMON_UPGRADE_INFO *)args;

	if (pUpgradeInfo == NULL)
	{
		return NULL;
	}
	
	while (1)
	{
		if (!pUpgradeInfo->bWdtActive)//’˝≥£«Èøˆœ¬’‚¿Ô «≤ªƒ‹Œππ∑µƒ£¨÷ª”–…˝º∂µƒ ±∫Ú≤≈ƒ‹Œππ∑
		{
			sleep(1);
			continue;
		}
		pUpgradeInfo->bWdtActiveAck = 1;
		
		if (nEnable)
		{
			WATCHDOG_init(200);
			WATCHDOG_enable();
			nEnable = 0;
		}
		
		WATCHDOG_feed();
		sleep(nSleepTime);
		sleep(1);

		//printf("%s \n", __FUNCTION__);
	}
	
	printf("%s() exit!\n", __FUNCTION__);
	pthread_exit(NULL);
	
	return NULL;
}

int DaemoThreadCreate(pthread_t *thread, void *(*start_routine) (void *), void *arg, int stack_size)
{
	int err = 0;
	pthread_attr_t Attr;

	/// ≥ı ºªØ Ù–‘°£
	pthread_attr_init(&Attr);

	#if 0
	int bDetached = 1;

	if (bDetached) 
	{
		/// ¥¥Ω®≥…∑÷¿Î Ωœﬂ≥Ã°£
		pthread_attr_setdetachstate(&Attr, PTHREAD_CREATE_DETACHED);
	} 
	else 
	{
		pthread_attr_setdetachstate(&Attr, PTHREAD_CREATE_JOINABLE);
	}
	#endif
	

	/// …Ë÷√œﬂ≥Ã’ª«¯ƒ⁄¥Ê¥Û–°°£
	if (stack_size < DAEMO_THREAD_STACK_MIN) 
	{
		printf("Thread: Set Minimal Stack( Size = %u ).", stack_size);
		stack_size = DAEMO_THREAD_STACK_MIN;
	}
	else if(stack_size > DAEMO_THREAD_STACK_MAX)
	{
		printf("[%s:%d]Thread: Set Maximal Stack( Size = %u ).", __func__, __LINE__, stack_size);
		stack_size = DAEMO_THREAD_STACK_MAX;
	}

	if (0 != pthread_attr_setstacksize(&Attr, stack_size)) 
	{
		printf("[%s:%d]Thread: Set Stack( Size = %u ) Error, Set Default Stack( Size = %u ).", __func__, __LINE__, stack_size, DAEMO_THREAD_STACK_DEF);
		if (0 != pthread_attr_setstacksize(&Attr, DAEMO_THREAD_STACK_DEF)) 
		{
			printf("[%s:%d]Thread: Set Default Stack( Size = %u ) Error.", __func__, __LINE__, DAEMO_THREAD_STACK_DEF);
		}
	}

	/// Õ®π˝ pthread ∑Ω∑®¥¥Ω®œﬂ≥Ã°£
	err = pthread_create(thread, &Attr, start_routine, arg);
	if (0 != err) 
	{
		printf("[%s:%d]pthread_create error:%d!\n", __func__, __LINE__, err);
		return -1;
	} 

	/// œ˙ªŸ¡Ÿ ± Ù–‘°£
	//pthread_attr_destroy(&Attr);

	/// ¥¥Ω®≥…π¶°£
	return err;
}


s32 main(s32 argc, const char *argv[])
{
	int ret;
	int fd;
	pthread_t pid;
	pthread_t pid_wdt;
	unsigned int nRomMaxSize = 0;
	unsigned int nFreeSize = 0;
	DAEMON_UPGRADE_INFO *pUpgradeInfo;
	unsigned int bForceEnable = 0;
	printf("upgrade version: %s %s\n", __DATE__, __TIME__);

	pUpgradeInfo = &g_UpgradeInfo;
	memset(pUpgradeInfo, 0, sizeof(DAEMON_UPGRADE_INFO));
	pUpgradeInfo->nStartTimePts = GetAppStartTime();
	
	ret = DaemoThreadCreate(&pid, Upgrade_WdtActiveProc, (void*)pUpgradeInfo, DAEMO_THREAD_STACK_DEF);
	if (ret != 0)
	{
		perror("pthread_create: Upgrade_WdtActiveProc \n");
		printf("ret = %d\n", ret);
		return -1;
	}

	ret = DaemoThreadCreate(&pid, UpgradeCommunicateProc, (void*)pUpgradeInfo, DAEMO_THREAD_STACK_DEF);
	if (ret != 0)
	{
		perror("pthread_create: UpgradeCommunicateProc \n");
		printf("ret = %d\n", ret);
		return -1;
	}
	
	if (argc > 1)
	{
		pUpgradeInfo->bArgActive = 1;
	}
		
	while (1) 
	{
		usleep(100000);
		if (pUpgradeInfo->bUpgrade)
		{
			UpgradeLock();
			pUpgradeInfo->bWdtActive = 1;

			if (pUpgradeInfo->pVirAddr != NULL && pUpgradeInfo->nVirSize > 0)
			{
			 	UpgradeProgress(0, 1);//œ»∞—Ω¯∂»Ãı¿≠∆¿¥
				usleep(1);

				UPGRADE_CGI_GetUpgradeRate();
				ret = NK_FwGetUpgradeData(pUpgradeInfo->pVirAddr,  pUpgradeInfo->nVirSize, &pUpgradeInfo->stUpgradeInfo, UpgradeProgress);
				if (ret == 0)
				{
					pUpgradeInfo->bUpgradeProgress = 100;
					UpgradeProgress(0, 100);//…˝º∂Ω· ¯
					usleep(2000);
				}
				else
				{
					pUpgradeInfo->bUpgradeProgress = UPGRADE_PROGRESS_EROR;// ß∞‹±Í÷æ
				}
				
				printf("NK_FwGetUpgradeData  ret = %d\n", ret);
			}
			
			sync(); 
			sleep(1);
			pUpgradeInfo->bUpgrade = 0;
			pUpgradeInfo->bWdtActive = 0;
			UpgradeUnlock();
			sleep(5);
			if (ret == 0)
			{
				NK_MAIN_SUB_System("reboot -f", 1);
			}
		}
		usleep(100000);
	}
}

#endif
