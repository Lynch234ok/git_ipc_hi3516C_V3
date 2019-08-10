#ifndef __UPGRADE_H__
#define __UPGRADE_H__
#include <unistd.h>
#include "pthread.h"

#define MEMGETINFO              _IOR('M', 1, struct mtd_info_user)

#define FLASH_SIZE_PAD8M	(8*1024*1024)
#define FLASH_SIZE_PAD16M	(16*1024*1024)
#define FLASH_SIZE_PAD32M	(32*1024*1024)
#define FLASH_SIZE_PAD64M	(64*1024*1024)
#define FLASH_SIZE_PAD128M	(128*1024*1024)
#define FLASH_SIZE_PAD256M	(256*1024*1024)
		

enum upgrade_ret_code
{
    UPGRADE_SUCCESS = 0,
    UPGRADE_UPGRADING,//升级中
    UPGRADE_START,
    UPGRADE_HAD_INIT,
    UPGRADE_NO_USB_DEV,
    UPGRADE_USB_MOUNT_ERR,
    UPGRADE_NO_FILE_ROM,//没有找到升级文件
    UPGRADE_FILE_ROM_SIZE_ERR,//
    UPGRADE_FILE_ROM_UNENCOD_ERR,//
    UPGRADE_FILE_ROM_OPT_ERR,//操作升级文件失败，
    UPGRADE_FILE_ROM_FAIL,//操作升级文件失败，
};


#define MAX_DEPTH_SCAN 	(5)
#define MAX_PATH_LEN	(128)

//for  upgrade_daemon 小进程
#if 1
//定义使用小进程进行升级，没有定义则使用主进程进行升级
#ifndef MK_UPGRADE_APP 
#define UPGRADE_BY_DAEMO //这个宏给App.out主进程使用
#endif

//小进程那边使用，小进程不会使用Cmake中Makefile导出的JSON_PATH宏
#ifndef JSON_PATH
#define JSON_PATH "/config/json"
#endif

#define USB_UPGRADE_FLAGS				"help_up.rom"
#define USB_FORCE_UPGRADE_FLAGS			"help_force.rom"

#define  MTD_UPRADE_MAX_CNT					10	 
#define  CMD_MAX_LEN						256	 
#define  FIFO_CLIENT_NAME 					"/tmp/app_to_up_fifo" //业务写的方向 (1) app write to upgrade_daemon and (2)upgrade_daemon read from app
#define  FIFO_SRV_NAME 						"/tmp/up_to_app_fifo" //业务写的方向 (1) upgrade_daemon write to app, and (2)app read from upgrade_daemon
#define  FIFO_CMD_MAGIC_NUM  				0x19823F08

//cmd 
#define  FIFO_CMD_ACK  						0x0 //正确接收到的回应
#define  FIFO_CMD_ERROR_ACK  				0x01 //接收到了，但是接收数据命令解析错误知会
#define  FIFO_CMD_PHY_ADDR_MMAP  			0x1001 //把下载好的升级包 通过物理地址传输过去； 
#define  FIFO_CMD_PHY_ADDR_MUNMAP 			0x1002 //把下载好的升级包 通过物理地址传输过去； 
#define  FIFO_CMD_UPGRADE_START 			0x1003 //开始升级，带版本在内存中的偏移，以及版本大小
#define  FIFO_CMD_APP_TO_UPGRADE_PROGRESS 	0x1004 //进度条，这个升级时app发起要upgrade升级的
#define  FIFO_CMD_UPGRADE_TO_APP_PROGRESS 	0x1005 //进度条，这个升级时upgrade发起升级，需要app拉起进度条，如果app挂掉则没有进度条也会升级的
#define  FIFO_CMD_VER_BIN_ERR 				0x1006 //接收到的升级数据包，解析不对
#define  FIFO_CMD_VER_UPGRADE_FAIL 			0x1007 //升级失败
#define  FIFO_CMD_UPGRADE_REBOOT 			0x1008 //重启了，这时app接收到这个命令应该保存该保存的东西，否则马上会重启的
#define  FIFO_CMD_UPGRADE_SUCCESS 			0x1009 //升级成功，这时app接收到这个命令应该保存该保存的东西，否则马上会重启的

//error num
#define FIFO_READ_ERROR		(-1)
#define FIFO_WRITE_ERROR	(-2)
#define FIFO_ARGS_ERROR		(-3)
#define FIFO_DATA_ERROR		(-100)
#define FIFO_CRC_ERROR		(-101)
#define FIFO_MAGIC_ERROR	(-102)

//####
#define  UPGRADE_PROGRESS_EROR 	(0xFFFF)


#define MEM_PAGE_SIZE  0x1000
#define PAGE_SIZE_MASK 0xfffff000

#define MEM_DEV		"/dev/mem"

#define MAX_FLASH_SIZE 		(17*1024*1024) //fixme
#define MAX_OFFSET_SIZE 	(16*1024*1024)

#define DAEMO_THREAD_DEF_BACKLOG    (64)               ///< 默认允许线程连接数。
#define DAEMO_THREAD_MAX_BACKLOG    (128)              ///< 最大允许线程连接数。
#define DAEMO_THREAD_STACK_MIN      (1024 * 4)         ///< 线程栈区最小值（单位：字节）。
#define DAEMO_THREAD_STACK_MAX      (1024 * 1024 * 8)  ///< 线程栈区最大值（单位：字节）。
#define DAEMO_THREAD_STACK_DEF      (1024 * 1024 * 2)  ///< 线程栈区默认值（单位：字节）。

typedef struct verison_size_info
{
	unsigned int bIsKernel; //是否为kernel分区
	unsigned int u32Offset; 
	unsigned int u32VerSize; 
	unsigned int u32crc; 
	unsigned int u32Idx; 
}VER_SIZE_INFO;

typedef struct upgrade_ver_info
{
	unsigned int  u32KernelMtdIdx; 
	unsigned int  u32UpgradeMtdCnt; 
	unsigned int  u32TotalVerSize; 
	VER_SIZE_INFO stBlockInfo[MTD_UPRADE_MAX_CNT];
}UPGRADE_VER_SIZE_INFO;

typedef struct daemon_upgrade_info
{
	unsigned int nStartTimePts;//小进程启动的时的时间戳，单位s
	int bArgActive;//带参数检测升级的
	char *pArgRomFile;//升级文件
	int bWdtActive;
	int bWdtActiveAck;
	int bUpgrade;
	int bUpgradeType;//0 app to upgrade;1: upgrade 发起的给 app的升级 
	int bUpgradeProgress;//进度条
	char *pVirAddr;//
	unsigned int nVirSize;
	UPGRADE_VER_SIZE_INFO stUpgradeInfo;
}DAEMON_UPGRADE_INFO;

typedef struct phy_addr_info
{
	unsigned int u32PhyAddr; 
	unsigned int u32PhyLen; 
}PHY_ADDR_INFO;

typedef struct fifo_cmd
{
	unsigned int u32MagicNum;
	unsigned int u32CmdType; 
	unsigned int u32Idx; 
	char 		 u8Cmd[CMD_MAX_LEN];
	unsigned int u32Crc; 
}FIFO_CMD_S; 

unsigned short NK_LIB_DoCrc16(void *pdata, unsigned int len);

typedef int (*NK_FUNC_T)(int, int);
int FifoRead(int fd, FIFO_CMD_S *pCmd);
int FifoWrite(int fd, FIFO_CMD_S *pCmd);
int FifoSendPhyAddrMapCmd(unsigned int u32PhyAddr, unsigned int u32Len);
int FifoSendPhyAddrMunmapCmd(void);
int FifoSendStartUpgradeCmd(UPGRADE_VER_SIZE_INFO *pUpgradeInfo);
int FifoReadUpgradeProgress(NK_FUNC_T pFunc);
int GetFifoReadFd(void);
int FifoTimeOut(int read_fd, int sec, int usec);
int AppToUpgradeUpdateProgress(int bBlock, unsigned int nTotalUpgradeSize, unsigned int *pUpgradeSize);

int UpgradeToAppUpdateProgress(void);//被动升级时，进度条检测线程

int NK_UCODE_Init();
int NK_UCODE_ReadItem(char* name, void* data, int size);
int NK_ODM_Init();
int NK_ODM_ReadItem(char* name, void* data, int size);

int DaemoGetHwId(char *pHwId, int *pLen, int nMaxLen);
int DaemoGetOdmId(char *pOdmId, int *pLen, int nMaxLen);
unsigned int DaemoGetProductId(void);
int DaemoGetFwVersion(char *pVer, int *pLen, int nMaxLen);
int DaemoGetVersion(unsigned int *pMainVer, unsigned int *pSubVer, unsigned int *pCustomVer);
int FindUpgradeRomByPath(char* mnt_dir, int nDepth, char* pfile);
int JDK_UsbUpgradeTrigger(NK_FUNC_T pFunc, char* file_name);
int NK_USBSTORAGE_CheckFile(const char *filename, int bDelFlags);

int WTD_Enable();
int WTD_Disable();
int WTD_Feed();

#endif

void *JDK_CallocMmzBuf(unsigned int *pu32RetPhyAddr, unsigned int u32Len);
int JDK_FreeMmzBuf(unsigned int u32PhyAddr, void *pVirtAddr);

void *CallocFwRomBuf(unsigned int *pRetPhyAddr, unsigned int size);
void FreeFwRomBuf(void* p, unsigned int u32PhyAddr);
int GetFwUpgradeStatus();
int SetFwUpgradeStatus(int bIsUpgrading);
///////////////////////////////////////
int NK_UpgradeSetStat(int nRetCode, int nProcess);
int FindUpgradeRomByPath(char* mnt_dir, int nDepth, char* pfile);
int JDK_UsbUpgradeTrigger(NK_FUNC_T pFunc, char* file_name);
int GetHwId(char *pHwId, int *pLen, int nMaxLen);
int GetOdmId(char *pOdmId, int *pLen, int nMaxLen);
int UsbUpgrade(NK_FUNC_T pFunc, const char* file_name);
void *CallocFwRomBuf(unsigned int *pu32RetPhyAddr, unsigned int size);
void FreeFwRomBuf(void* p, unsigned int u32RetPhyAddr);

#endif
