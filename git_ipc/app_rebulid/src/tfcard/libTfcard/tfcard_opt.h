
#ifndef _TFCARD_OPT_H_
#define _TFCARD_OPT_H_

#ifdef __cplusplus
extern "C"{
#endif

// tf CARD status
#define sTFCARD_STATUS_OK "ok"
#define sTFCARD_STATUS_FORMATED "already_format"
#define sTFCARD_STATUS_NOT_FORMAT "no_format"
#define sTFCARD_STATUS_EXCEPTION "exception"
#define sTFCARD_STATUS_NO_TFCARD "no_tfcard"
#define sTFCARD_STATUS_FORMATTING "formatting"
#define sTFCARD_STATUS_ABNORMAL "abnormal"


#define TFCARD_FORMAT_INVALID	(0x100) ////无效格式化
#define TFCARD_FORMAT_READY		(0x101) ////准备格式化
#define TFCARD_FORMAT_RUNING	(0x102) ////正在格式化
#define TFCARD_FORMAT_FINISH	(0x103) ////完成格式化

#define TFCARD_REPAIR_RUNING	(0x104) ////正在修复



typedef struct tagTFCARDATTR
{
	NK_Boolean bFormat;////检测TF是否存分区，是否需要格式化
	NK_Int emStatus;////显示TF状况
	struct {
		NK_Boolean formating;////是否需要格式化
		NK_Boolean repairing;////是否需要修复
		NK_Boolean mounted;////是否挂载TRUE:已经挂载，FALSE:没有挂载

		NK_Char dev_path[32];//// /dev/mmcblk0p1
		NK_Char fs_path[32];//// /mnt/rec
		NK_Char strType[8];////设置类型 type=vfat
		NK_Char strOpts[2];//// rw/ro/
	}TFSlot; ////文件系统挂载信息
	stTFSDKEVENT EventSet;
	NK_Size max_buffer_size_kb;
}stTFCARDATTR, *pstTFCARDATTR;

NK_Int TFCARDOPT_Init(pstTFPARAM ptfParam, pstTFSDKEVENT pEventSet);
NK_Int TFCARDOPT_Exit();
NK_Boolean TFCARDOPT_Exist();
NK_Boolean TFCARDOPT_Detect();
NK_Boolean TFCARDOPT_IsMounted();
NK_Boolean TFCARDOPT_Mounted();
NK_Boolean TFCARDOPT_Umounted();
NK_Boolean TFCARDOPT_CleanTF();
NK_Size TFCARDOPT_GetCapacity();
NK_Size TFCARDOPT_GetFreespace();
NK_Int TFCARDOPT_FormatTF();

NK_Int TFCARDOPT_GetFormatStatus();
NK_Boolean TFCARDOPT_GetOverWrite();

NK_PChar TFCARDOPT_GetMountPath();
NK_Int TFCARDOPT_SetFormat(NK_Int bFormat);
NK_Int TFCARDOPT_GetTfcardStatus(char *ret_status);


#ifdef __cplusplus
}
#endif

#endif

