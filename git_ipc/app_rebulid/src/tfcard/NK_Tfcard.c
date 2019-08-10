#include <sys/reboot.h>
#include <sys/statvfs.h>
#include <sys/mount.h>
#include <unistd.h>
#include <NkUtils/types.h>
#include <signal.h>
#include "LibTfcard.h"
#include "TS_Read.h"
#include "TS_Write.h"
#include "NK_Tfcard.h"
#include "generic.h"
#include "netsdk_def.h"
#include "base/ja_process.h"
#include "media_buf.h"
#include <sdk_enc.h>
#include "global_runtime.h"


NK_Int NK_TFCARD_get_timezone_sec()
{
#if 0
    system_user_info *get_timezone = anyka_get_system_user_info();
    return (get_timezone->tzone * 36);
#endif

}

typedef void (*sighandler_t)(int);
int pox_system(const char *cmd_line)
{
   int ret = 0;
   sighandler_t old_handler;

   old_handler = signal(SIGCHLD, SIG_DFL);
   ret = system(cmd_line);
   signal(SIGCHLD, old_handler);

   return ret;
}

static int tfcard_dev_fdisk()
{
    const char *scriptPath = "/tmp/fdisk.script";
    char sysCommand[64];
    FILE* fID = NULL;
    int ret = 0;
    const char scriptContent[] =
        "d\n"
        "1\n"
        "d\n"
        "2\n"
        "d\n"
        "3\n"
        "d\n"
        "4\n"
        "n\n"
        "p\n" // primary partition
        "1\n" // partition number
        "\n" // first cylinder
        "\n" // last cylinder
        "p\n"//printf msg
        "w\n"; // write

    fID = fopen(scriptPath, "w+b");
    if(NULL != fID){
        fwrite(scriptContent, 1, strlen(scriptContent), fID);
        fclose(fID);
        // fdisk
        snprintf(sysCommand, sizeof(sysCommand), "fdisk %s < %s", TFCARD_MMCBLK0_PATH,scriptPath);
        NK_SYSTEM(sysCommand);
        REMOVE_FILE(scriptPath);
        return 0;
    }

    return -1;
}

static int tfcard_dev_mkfs()
{
    NK_Char cmdLine[64] = {0};
    NK_Int ret = 0;
    snprintf(cmdLine, sizeof(cmdLine), "mkdosfs -F 32 %s", TFCARD_MMCBLK0P1_PATH);
    //snprintf(cmdLine, sizeof(cmdLine), "mkfs.vfat %s", TFCARD_DEV_PATH);
    //if(0 != system(cmdLine)){
    if(0 != pox_system(cmdLine)){
        printf("[%s:%d]tfcard format error!!!\n",__func__,__LINE__);
        return -1;
    }
    printf("[%s:%d]tfcard format success!!!\n",__func__,__LINE__);
    return 0;
}


NK_Int TF_System(NK_PChar szCmd, NK_Boolean bBlocked)
{
    //NK_INFO("cmd(%s)\r\n", szCmd);
    return system(szCmd);
}

NK_Boolean TF_HasFile(NK_PChar path)
{
    NK_Boolean hasFile = NK_False;
    if(NK_Nil == path || strlen(path) < 0)
    {
        return NK_FALSE;
    }
    hasFile = (-1 != access(path, F_OK));
    return (hasFile ? NK_TRUE : NK_FALSE);
}


//检测 TF 物理上是否存在事件
NK_Int TFCARD_OnExistTF()
{
    return TF_HasFile(TFCARD_MMCBLK0_PATH);
}
//检测 TF 是否存在事件
NK_Int TFCARD_OnDetectTF()
{
    return TF_HasFile(TFCARD_MMCBLK0P1_PATH);
}
//装载 TF 卡文件系统目录
NK_Int TFCARD_OnMountTF(NK_PChar mount_path)
{
    NK_Char cmdLine[64] = {0};
    NK_Int ret=0;
    if(NK_Nil == mount_path)
    {
        return NK_FALSE;
    }

    if(!TF_HasFile(mount_path))
    {
        //只限于可读写的文件系统的文件名
        printf("mkdir dir before mount,path = %s\n",mount_path);
        snprintf(cmdLine, sizeof(cmdLine), "mkdir -p %s", mount_path);
        TF_System(cmdLine, NK_TRUE);
    }

    snprintf(cmdLine, sizeof(cmdLine), "mount %s %s", TFCARD_MMCBLK0P1_PATH, mount_path);
    NK_SYSTEM(cmdLine);
    //TF_System(cmdLine, NK_TRUE);
    /*if(0 != (ret = mount(TFCARD_MMCBLK0P1_PATH, mount_path, "vfat", 0, NULL)))
    {
        //NK_ERROR("mount TFcard error, ret : %d, error : %d, path : %s",ret, errno, mount_path);
        return NK_FALSE;
    }*/
    //判断TF卡是否真的可读写，防止坏卡，或者可能小概率出现文件系统成为只读
    //NK_INFO("TFcard mount[%s] ok!", mount_path);
    return NK_TRUE;
}
//卸载 TF 卡文件系统目录事件
NK_Int TFCARD_OnUmountTF(NK_PChar mount_path)
{
    system("sync");
    usleep(500000);
    if(NK_Nil == mount_path)
    {
        return NK_FALSE;
    }
    if(0 != umount2(mount_path, MNT_DETACH))
    {
        printf("umount TFcard error,path : %s\n",mount_path);
        return NK_FALSE;
    }
    usleep(500000);
    printf("TFcard umount[%s] ok!\n", mount_path);
    NK_SYSTEM("mount");
    return NK_TRUE;
}
//清除 TF 卡挂载目录事件
NK_Int TFCARD_OnCleanTF(NK_PChar mount_path)
{
    NK_Char cmdLine[32] = {0};
    if(NK_Nil == mount_path)
    {
        return NK_FALSE;
    }
    snprintf(cmdLine, sizeof(cmdLine), "rm -rf %s", mount_path);
    TF_System(cmdLine, NK_TRUE);
    return NK_TRUE;
}
//获取 TF 卡总容量
NK_Size TFCARD_OnGetCapacity(NK_PChar mount_path)
{
    struct statvfs statFs;
    NK_Int64 capacity = 0;
    if(statvfs(mount_path, &statFs) < 0)
    {
//      NK_ERROR("get Gapacity error");
        return NK_FALSE;
    }
    capacity = statFs.f_blocks;
    capacity *= statFs.f_bsize;
    //转换成 MB 单位
    capacity /= 1024;
    capacity /= 1024;
    //NK_INFO ("get Gapacity : %d MB",(NK_Size)capacity);
    return (NK_Size)capacity;
 }
//获取 TF 卡可用量
NK_Size TFCARD_OnGetFreeSpace(NK_PChar mount_path)
{
    struct statvfs statFs;
    NK_Int64 freeSpace = 0;
    if(NK_Nil == mount_path)
    {
        return NK_FALSE;
    }
    if(statvfs(mount_path, &statFs) < 0)
    {
//      NK_ERROR("get freeSpace error");
        return NK_FALSE;
    }
    freeSpace = statFs.f_bavail;
    freeSpace *= statFs.f_bsize;

    //转换成 MB 单位
    freeSpace /= 1024;
    freeSpace /= 1024;

    return (NK_Size)freeSpace;
}
//TF 卡格式化事件
NK_Int TFCARD_OnFormat()
{
    if(0 != tfcard_dev_fdisk()){
        printf("[%s:%d]tfcard fdisk error: dev = %s\n",__func__,__LINE__,TFCARD_MMCBLK0_PATH);
        return -1;
    }
    if( 0 != tfcard_dev_mkfs()){
        printf("[%s:%d]tfcard mkfs error: dev = %s!!!!!\n",__func__,__LINE__,TFCARD_MMCBLK0P1_PATH);
        return -1;
    }

    return 0;

}

NK_Int TFCARD_OnGetOverWrite()
{
    return NK_TRUE;
}

static NK_Int TFCARD_onAfterFormat(NK_Boolean status)
{
    // 不管格式化是否成功,都把格式化前停止的录像线程重新启动
    NK_TFRECORD_Start(0,EN_RECORD_TYPE_TIMER);

}

static NK_Int TFCARD_OnRemountRo()
{
	system("mount -o remount,rw /dev/mmcblk0p1");

}

static NK_Int TFCARD_OnRepairRo()
{
#if 0   // 目前不支持
	NK_Char reFile[64] = { 0 };
	NK_TFRCORD_RecordFile_Get(reFile);
	if(0 != strlen(reFile)){
		ak_dvr_repair_ro_disk(TFCARD_MOUNT_PATH,reFile);
	}
#endif
}

static NK_Int TFCARD_OnFsckRo()
{
    char cmd[256] = {0};
	NK_SYSTEM("umount -l /media/tf");
	sleep(1);
    snprintf(cmd, sizeof(cmd), "%s/sdcard_tools/fsck.fat -ab /dev/mmcblk0p1", IPCAM_ENV_HOME_DIR);
    NK_SYSTEM(cmd);
	APP_WIFI_exit_wifi();
    GLOBAL_reboot_system();
}


////////////////////////////////////////////////////////////////////////////////////////
NK_Int RECORD_AddReader(NK_Int nChn, NK_Int enMediaType, NK_PVoid *hReader)
{
    lpMEDIABUF_USER mediaBufUser = NULL;
    NK_PChar stream_str = (0 == nChn) ? "ch0_0.264" : "ch0_1.264";
    NK_Int mediaBufId = MEDIABUF_lookup_byname(stream_str);
    if(-1 == mediaBufId)
    {
        printf("lookup mediaBufId error\n");
        return NK_FALSE;
    }
    mediaBufUser = MEDIABUF_attach(mediaBufId);
    if(NK_Nil == mediaBufUser)
    {
        printf("mediaBufUser attach error\n");
        return NK_FALSE;
    }
    *hReader = (NK_PVoid)mediaBufUser;
    printf("[NK_XAPI_AddReader] nChn=%d,stream_str=%s \n" ,nChn,stream_str);
    return NK_TRUE;

}

NK_Int RECORD_LockReader(NK_PVoid *hReader)
{
    lpMEDIABUF_USER mediaBufUser = (lpMEDIABUF_USER)*hReader;
    if(NK_Nil == mediaBufUser)
    {
        printf("[RECORD_LockReader]------error!\n");
        return NK_FALSE;
    }
    if(0 == MEDIABUF_out_try_lock(mediaBufUser))
    {
        return NK_TRUE;
    }
    
    return NK_FALSE;

}

NK_Int RECORD_UlockReader(NK_PVoid *hReader)
{
    lpMEDIABUF_USER mediaBufUser = (lpMEDIABUF_USER)*hReader;
    if(NK_Nil == mediaBufUser)
    {
        printf("[RECORD_UlockReader]------error!\n");
        return NK_FALSE;
    }
    MEDIABUF_out_unlock(mediaBufUser);
    return NK_TRUE;

}

static NK_Int syncReader(NK_PVoid *hReader, NK_Int sec)
{
    return NK_TRUE;
    lpMEDIABUF_USER mediaBufUser = (lpMEDIABUF_USER)*hReader;
    if(NK_Nil == mediaBufUser)
    {
        printf("[%s:%d]RECORD_SyncReader------error!\n",__func__,__LINE__);
        return NK_FALSE;
    }
    if(MEDIABUF_sync(mediaBufUser) < 0)
    {
        printf("[%s:%d]MediaBuffer sync failed!\n",__func__,__LINE__);  
        return NK_FALSE;

    }
    //MEDIABUF_sync2(mediaBufUser, sec);
    return NK_TRUE;
}

/**
 * 默认请求主码流I帧
 */
static NK_Int requestStreamKeyframe()
{
    SDK_ENC_request_stream_keyframe(0, 0);

}

static NK_Int RECORD_IsEmpty(NK_PVoid *hReader)
{
    return NK_TRUE;
    lpMEDIABUF_USER mediaBufUser = (lpMEDIABUF_USER)*hReader;
    if(NK_Nil == mediaBufUser)
    {
        printf("[RECORD_IsEmpty]------error!\n");
        return NK_FALSE;
    }

    return MEDIABUF_is_empty(mediaBufUser);
}

NK_Int RECORD_GetMedia(NK_PVoid* hReader, pstTFMEDIAHEAD pMediaBuf, NK_PVoid* pFrameData)
{

    NK_UInt64 outSize = 0;
    lpMEDIABUF_USER mediaBufUser = (lpMEDIABUF_USER)*hReader;
    lpSDK_ENC_BUF_ATTR pstMediaAttr = NULL;
    if(NK_Nil == mediaBufUser)
    {
        //NK_ERROR("lookup mediaBufUser NK_Nil");
        return NK_FALSE;
    }

    if( 0 != MEDIABUF_out(mediaBufUser, (void *)&(pstMediaAttr), NULL, (size_t*)&outSize))
    {
        //NK_ERROR("lookup MEDIABUF_out NK_Nil");
        return NK_FALSE;
    }
    *pFrameData = (NK_PVoid)(pstMediaAttr + 1); //// 祼码流的偏移
    pMediaBuf->coderStamp_us = pstMediaAttr->timestamp_us / 1000;
    pMediaBuf->sysTime_us = pstMediaAttr->time_us / 1000; // 该值在写文件时未使用
    pMediaBuf->dataSize = pstMediaAttr->data_sz;    //// 祼码流的大小

    switch(pstMediaAttr->type)
    {
        case kSDK_ENC_BUF_DATA_H264:
            {
                pMediaBuf->codec = TFCARD_VCODEC_H264;
                pMediaBuf->isKeyFrame = pstMediaAttr->h264.keyframe;
                pMediaBuf->fps = pstMediaAttr->h264.fps;
                pMediaBuf->width = pstMediaAttr->h264.width;
                pMediaBuf->height = pstMediaAttr->h264.height;
            }
            break;
        case kSDK_ENC_BUF_DATA_H265:
        {
            pMediaBuf->codec = TFCARD_VCODEC_H265;
            pMediaBuf->isKeyFrame = pstMediaAttr->h265.keyframe;
            pMediaBuf->fps = pstMediaAttr->h265.fps;
            pMediaBuf->width = pstMediaAttr->h265.width;
            pMediaBuf->height = pstMediaAttr->h265.height;
        }
        break;
        case kSDK_ENC_BUF_DATA_AAC:
            {
                pMediaBuf->codec = TFCARD_ACODEC_AAC;
                pMediaBuf->sampleRate = pstMediaAttr->g711a.sample_rate;
                pMediaBuf->sampleWidth = pstMediaAttr->g711a.sample_width;
                pMediaBuf->compressionRatio = pstMediaAttr->g711a.compression_ratio;
                pMediaBuf->samplePacket = 1;
                pMediaBuf->isKeyFrame = 0;
            }
            break;
        default:
            return NK_FALSE;
    }
    return NK_TRUE;


}

NK_Int RECORD_DelReader(NK_PVoid *hReader)
{    
     lpMEDIABUF_USER mediaBufUser = (lpMEDIABUF_USER)*hReader;
     if(mediaBufUser == NULL){
        printf("lookup RECORD_DelReader NK_Nil\n");
        return NK_FALSE;
     }
     MEDIABUF_detach(mediaBufUser);
     mediaBufUser = NK_Nil;
    return NK_TRUE;
}

static unsigned int mdTime = 0;

static void ipcam_timer_Md()
{
    if(mdTime > 0)
    {
        mdTime--;
    }
}

NK_Boolean RECORD_GetMdStatus(NK_Int nRecChn)
{
    return mdTime > 0 ? true : false;
}


NK_Boolean NK_TFRCORD_MdSetting()
{
    mdTime = 30;
}

NK_Int RECORD_SetRecStatus(NK_Int nRecChn, NK_Int nRecType)
{
    return NK_FALSE;
}


NK_Int TEST_PLAY_Start();


static int record_on_schedule(stRECSCHEDULE *stSchedule)
{
    ST_NSDK_SYSTEM_SETTING sinfo={0};
    NETSDK_conf_system_get_setting_info(&sinfo);
    int i = 0;

    if(sinfo.timeRecordEnabled == false){
        return -1;
    }

    for(i = 0;
            i < sizeof(stSchedule->stSectionSlot)
                / sizeof(stSchedule->stSectionSlot[0]);
            i++)
    {
        if(false == sinfo.TFcard_Record.Schedule[i].enabled)
        {
            continue;
        }

        stSchedule->stSectionSlot[i].enable = sinfo.TFcard_Record.Schedule[i].enabled;
        stSchedule->stSectionSlot[i].chnMask = 0;
        stSchedule->stSectionSlot[i].nRecType = EN_RECORD_TYPE_TIMER;
        stSchedule->stSectionSlot[i].nStartTime = sinfo.TFcard_Record.Schedule[i].BeginTime.hour * 3600
                    + sinfo.TFcard_Record.Schedule[i].BeginTime.min * 60
                    + sinfo.TFcard_Record.Schedule[i].BeginTime.sec ;
        stSchedule->stSectionSlot[i].nStopTime = sinfo.TFcard_Record.Schedule[i].EndTime.hour * 3600
                    + sinfo.TFcard_Record.Schedule[i].EndTime.min * 60
                    + sinfo.TFcard_Record.Schedule[i].EndTime.sec ;
        stSchedule->stSectionSlot[i].nScheduleType = sinfo.TFcard_Record.Schedule[i].weekday;
    }
    return 0;
}

NK_Int NK_TFCARD_Init(NK_PChar pstrDevpath, NK_PChar pstrMountPath)
{
    stTFPARAM tfParam;
    stTFSDKEVENT stEvent;
    if(NK_Nil == pstrDevpath || NK_Nil == pstrMountPath)
    {
        return NK_FALSE;
    }
    memset(&tfParam, 0, sizeof(stTFPARAM));
    memset(&stEvent, 0, sizeof(stTFSDKEVENT));
    sprintf(tfParam.strDevpath, "%s", pstrDevpath);
    sprintf(tfParam.strMountPath, "%s", pstrMountPath);
    tfParam.bRecordOverLoad = NK_TRUE;

    stEvent.OnExistTF = TFCARD_OnExistTF;
    stEvent.OnDetectTF = TFCARD_OnDetectTF;
    stEvent.OnMountTF = TFCARD_OnMountTF;
    stEvent.OnUmountTF = TFCARD_OnUmountTF;
    stEvent.OnCleanTF = TFCARD_OnCleanTF;
    stEvent.OnGetCapacity = TFCARD_OnGetCapacity;
    stEvent.OnGetFreeSpace = TFCARD_OnGetFreeSpace;
    stEvent.OnFormat = TFCARD_OnFormat;
    stEvent.OnGetOverWrite = TFCARD_OnGetOverWrite;
    stEvent.fTFcardOnAfterFormat = TFCARD_onAfterFormat;
	stEvent.fTFcardOnRemountRo = TFCARD_OnRemountRo;
	stEvent.fTFcardOnRepairRo = TFCARD_OnRepairRo;
	stEvent.fTFcardOnFsckRo = TFCARD_OnFsckRo;

    return TFSDK_CARD_Init(&tfParam, &stEvent);
}
NK_Int NK_TFCARD_Exit()
{
    return TFSDK_CARD_Exit();
}

NK_Size NK_TFCARD_GetCapacity()
{
    return TFSDK_CARD_GetCapacity();

}

NK_Size NK_TFCARD_GetFreeSpace()
{
    return TFSDK_CARD_GetFreeSpace();

}

NK_Int NK_TFCARD_Format()
{
    return TFSDK_CARD_Format();

}

NK_Int NK_TFCARD_GetStatus()
{
    return TFSDK_CARD_GetStatus();

}
NK_Int NK_TFCARD_GetTsRecPath_Of_Hex(NK_PChar pReFile)
{
	return TFSDK_CARD_GetTsRecPathOfHex(pReFile);
}

static HTFRECORD g_hRecord[4];
NK_Int NK_TFRECORD_Start(NK_Int nChnId, NK_Int nRecType)
{
    stTFRECPARAM stRecParam;
    stTFRECEVENT stRecEvent;
    memset(&stRecParam , 0 , sizeof(stTFRECPARAM));
    memset(&stRecEvent , 0 , sizeof(stTFRECEVENT));

    stRecParam.nChnId = nChnId;
    stRecParam.enRecType = nRecType;////nRecType;
    stRecEvent.AddReader = RECORD_AddReader;
    stRecEvent.DelReader = RECORD_DelReader;
    stRecEvent.LockReader = RECORD_LockReader;
    stRecEvent.UlockReader = RECORD_UlockReader;
    stRecEvent.ReadMediaFrame = RECORD_GetMedia;
    stRecEvent.SyncReader = syncReader;
    stRecEvent.requestStreamKeyframe = requestStreamKeyframe;
    stRecEvent.IsEmpty = RECORD_IsEmpty;
    stRecEvent.GetMdStatus = RECORD_GetMdStatus;
    stRecEvent.SetRecStatus = RECORD_SetRecStatus;
    if(NK_Nil != g_hRecord[nChnId])
    {
        TFSDK_RECORD_Stop(g_hRecord[nChnId]);
        g_hRecord[nChnId] = NK_Nil;
    }
    g_hRecord[nChnId] = TFSDK_RECORD_Start(&stRecParam, &stRecEvent);
    if(NK_Nil == g_hRecord[nChnId])
    {
        return NK_FALSE;
    }

    stRECSCHEDULE stSchedule;
    memset(&stSchedule, 0 , sizeof(stSchedule));
    record_on_schedule(&stSchedule);
    TFSDK_RECORD_SetParam(g_hRecord[nChnId], EN_TFRECORD_OPT_SCHEDULE, &stSchedule, NK_Nil);

    TICKER_add_task(ipcam_timer_Md, 1, false);

    return NK_TRUE;
}
NK_Int NK_TFRECORD_Stop(NK_Int nChnId)
{
    if(NK_Nil == g_hRecord[nChnId])
    {
        return NK_FALSE;
    }
    TFSDK_RECORD_Stop(g_hRecord[nChnId]);
    g_hRecord[nChnId] = NK_Nil;
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
    return 0;
}
NK_Int NK_TFRCORD_SetParam()
{
    stRECSCHEDULE stSchedule;
    memset(&stSchedule, 0 , sizeof(stSchedule));
    record_on_schedule(&stSchedule);
    TFSDK_RECORD_SetParam(g_hRecord[0], EN_TFRECORD_OPT_SCHEDULE, &stSchedule, NK_Nil);
    return NK_TRUE;
}

NK_Int NK_TFRCORD_RecordFile_Get(NK_PChar pReFile)
{
	TFSDK_RECORD_RecordFile_Get(g_hRecord[0],pReFile);
	return 0;
}

NK_PVoid NK_TFPLAY_Open(NK_PChar pstrFile)
{
    HTFPLAY hPlay = NK_Nil;
    if(NK_Nil == pstrFile)
    {
        return NK_Nil;
    }
    hPlay = TFSDK_PLAY_Open(NK_TRUE, pstrFile);
    if(NK_Nil == hPlay)
    {
        return NK_Nil;
    }
    return (NK_PVoid)hPlay;
}
NK_PVoid NK_TFPLAY_OpenEx(NK_Int nChn,NK_UInt32 nStartUtc,  NK_UInt32 nEndUtc,  NK_Int enMode, NK_PVoid pUserCtx)
{

    HTFPLAY hPlay = NK_Nil;
    stTFPLAYSEARCH stPlayParam;
    memset(&stPlayParam, 0 , sizeof(stTFPLAYSEARCH));
    stPlayParam.nChn = nChn;
    stPlayParam.nStartUtc = nStartUtc;
    stPlayParam.nEndUtc = nEndUtc;
    stPlayParam.nType = enMode;
    stPlayParam.pUserCtx = pUserCtx;
    hPlay = TFSDK_PLAY_Open(NK_FALSE, &stPlayParam);
    if(NK_Nil == hPlay)
    {   
        return NK_Nil;
    }
    return (NK_PVoid)hPlay;
}

NK_Int NK_TFPLAY_Close(NK_PVoid hPlay)
{
    return TFSDK_PLAY_Close(hPlay);
}
#if 1
NK_Int NK_TFPLAY_Read(NK_PVoid hPlay, NK_Int enMode, pstTFMEDIAHEAD pMediaAttr, NK_PVoid *pMediaAddr)
{
    NK_Int nRet = 0;
    stTFMEDIAHEAD stFrameHead;
    if(NK_Nil == hPlay || NK_Nil == pMediaAttr)
    {
        return NK_FALSE;
    }
    memset(&stFrameHead,0,sizeof(stTFMEDIAHEAD));

    nRet = TFSDK_PLAY_Read(hPlay, enMode, &stFrameHead, pMediaAddr);
    if(EN_TFSDK_PLAY_READ_ERR_MALLOC == nRet ||
        EN_TFSDK_PLAY_READ_ERR_FILE == nRet ||
        EN_TFSDK_PLAY_READ_ERR_FAIL == nRet )
    {   
        return NK_FALSE;
    }
    if(EN_TFSDK_PLAY_READ_ERR_NEXT == nRet)
    {
        return NK_TRUE;
    }


    pMediaAttr->dataSize = stFrameHead.dataSize;
    pMediaAttr->coderStamp_us = stFrameHead.coderStamp_us;
    pMediaAttr->sysTime_us = stFrameHead.sysTime_us;
    switch(stFrameHead.codec)
    {
        case TFCARD_VCODEC_H264:
            {
                
                pMediaAttr->codec = TFCARD_VCODEC_H264;
                pMediaAttr->isKeyFrame = stFrameHead.isKeyFrame;
                pMediaAttr->dataSize = stFrameHead.dataSize;
                pMediaAttr->fps = stFrameHead.fps;
                pMediaAttr->width = stFrameHead.width;
                pMediaAttr->height = stFrameHead.height;  
            }
            break;
        case TFCARD_VCODEC_H265:
            {
#if 1
                pMediaAttr->codec = TFCARD_VCODEC_H265;
                pMediaAttr->isKeyFrame = stFrameHead.isKeyFrame;
                pMediaAttr->dataSize = stFrameHead.dataSize;
                pMediaAttr->fps = stFrameHead.fps;
                pMediaAttr->width = stFrameHead.width;
                pMediaAttr->height = stFrameHead.height;  
#endif
            }
            break;
        case TFCARD_ACODEC_AAC:
            {
                pMediaAttr->codec = TFCARD_ACODEC_AAC;
                pMediaAttr->sampleRate = stFrameHead.sampleRate;
                pMediaAttr->sampleWidth = stFrameHead.sampleWidth;
                pMediaAttr->compressionRatio = stFrameHead.compressionRatio;

            }
            break;

        default:
            return NK_FALSE;
    }

    return NK_TRUE;
}

#define GOP_TIME_SPACE 10
NK_Boolean NK_TFPLAY_ReadSeek(NK_PVoid hPlayBack, pstTFMEDIAHEAD pMediaAttr,NK_PVoid *pFrameData,NK_UInt32 nSeekUtc)
{
    if(NK_Nil == hPlayBack || NK_Nil == pMediaAttr)
    {
        return NK_FALSE;
    }
    stTFMEDIAHEAD stFrameHead;
    NK_Int nRet = 0;
    NK_UInt16 time_interval = 0;
    NK_UInt32 play_curTime = 0;

    while(NK_TRUE)
    {
        memset(&stFrameHead, 0 , sizeof(stTFMEDIAHEAD));
        nRet = TFPLAY_ReadFrame(hPlayBack,&stFrameHead, pFrameData);
        
        if(EN_TFSDK_PLAY_READ_ERR_SUCCESS != nRet &&
            EN_TFSDK_PLAY_READ_ERR_NEXT != nRet)
        {
            return NK_FALSE;
        }

        play_curTime = (NK_UInt32)(stFrameHead.coderStamp_us/1000);
        time_interval = abs(nSeekUtc - play_curTime);
        if((TFCARD_VCODEC_H264 == stFrameHead.codec ||
            TFCARD_VCODEC_H265 == stFrameHead.codec) &&
            time_interval < GOP_TIME_SPACE &&
            1 == stFrameHead.isKeyFrame)
        {
            break;
        }else if(time_interval > 250){
            printf("[%s:%d]Read Seek failed\n",__func__,__LINE__);
            break;
        }
    }

    pMediaAttr->codec = stFrameHead.codec;
    pMediaAttr->dataSize = stFrameHead.dataSize;
    pMediaAttr->coderStamp_us = stFrameHead.coderStamp_us;
    pMediaAttr->sysTime_us = stFrameHead.sysTime_us;
    pMediaAttr->isKeyFrame = stFrameHead.isKeyFrame;
    pMediaAttr->fps = stFrameHead.fps;
    pMediaAttr->width = stFrameHead.width;
    pMediaAttr->height = stFrameHead.height;
    
    return NK_TRUE;
}
#endif



static fTFPLAYSEARCHCB g_PlaySearchCB;
NK_Int PLAY_SearchCB(NK_PVoid pUserCtx, NK_Int bFileType, NK_Int nNumber, NK_PVoid lParam)
{
    stNKTFPLAYTIME stPlayTime;

    if(NK_Nil == g_PlaySearchCB)
    {
        return NK_FALSE;
    }
    if(NK_FALSE == bFileType)
    {
        memset(&stPlayTime, 0 , sizeof(stNKTFPLAYTIME));
        memcpy(&stPlayTime,lParam,sizeof(stNKTFPLAYTIME));
        //g_PlaySearchCB(pUserCtx,bFileType,nNumber,(NK_PVoid)&stPlayTime);
    }

    return NK_TRUE;

}

NK_Int NK_TFPLAY_SearchFile(NK_Int nChn,
                                 NK_UInt32 nStartUtc,
                                 NK_UInt32 nEndUtc,
                                 NK_Int enMode,
                                 NK_PVoid pUserCtx,
                                 fTFPLAYSEARCHCB pfuch)
{

#define REC_PLAY_MAX_SEARCH_RECS 10000

    stTFPLAYSEARCH stSearch;
    if(NK_Nil == pfuch)
    {
        return NK_FALSE;
    }
    memset(&stSearch,0,sizeof(stTFPLAYSEARCH));
    stSearch.nChn = nChn;
    stSearch.nType = enMode;
    stSearch.nStartUtc = nStartUtc;
    stSearch.nEndUtc = nEndUtc;
    stSearch.pUserCtx = pUserCtx;
    //g_PlaySearchCB = pfuch;
    return TFSDK_PLAY_Search(&stSearch, pfuch,REC_PLAY_MAX_SEARCH_RECS);
}






NK_Int NK_TFPLAY_SearchTime(NK_Int nChn,
                              NK_UInt32 nStartUtc,
                              NK_UInt32 nEndUtc,
                              NK_Int enMode,
                              NK_PVoid pUserCtx,
                              fTFPLAYSEARCHCB pfuch)
{
    stTFPLAYSEARCH stSearch;
    memset(&stSearch,0,sizeof(stTFPLAYSEARCH));
    stSearch.nChn = nChn;
    stSearch.nType = enMode;
    stSearch.nStartUtc = nStartUtc;
    stSearch.nEndUtc = nEndUtc;
    stSearch.pUserCtx = pUserCtx;
    //g_PlaySearchCB = pfuch;
    return TFSDK_PLAY_SearchEx(&stSearch,pfuch);
}

NK_PVoid NK_TFPLAY_CreateHistory(NK_Int nChn,
                              NK_UInt32 nStartUtc,
                              NK_UInt32 nEndUtc,
                              NK_Int enMode,
                              NK_PVoid pUserCtx)
{
    stTFPLAYSEARCH stSearch;
    memset(&stSearch,0,sizeof(stTFPLAYSEARCH));
    stSearch.nChn = nChn;
    stSearch.nType = enMode;
    stSearch.nStartUtc = nStartUtc;
    stSearch.nEndUtc = nEndUtc;
    stSearch.pUserCtx = pUserCtx;
    return TFSDK_PLAY_CreateHistory(&stSearch);
}
                              
NK_Int NK_TFPLAY_FreeHistory(NK_PVoid hHistory)
{
    return TFSDK_PLAY_FreeHistory(hHistory);
}

NK_Int NK_TFPLAY_GetHistoryLen(NK_PVoid hHistory)
{
    return TFSDK_PLAY_GetHistoryLen(hHistory);
}

NK_Int NK_TFPLAY_GetHistoryFile(NK_PVoid hHistory, NK_Int nIndex, NK_PVoid *pstFile)
{
    return TFSDK_PLAY_GetHistoryFile(hHistory, nIndex, (pstTFPLAYFILE)pstFile);
}

