#ifndef axdll_H
#define axdll_H

#include "Ax_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "App.txt"

#define PROC_TOTAL_DVS_EXE       10 
#define __DEVICETYPE              dt_DevA4
#define FLASH_ADDR_SN 0x00db0000
#define VIDEOCHLCOUNT             1
#define AUDIOCHLCOUNT             1
#define DICHLCOUNT                1
#define DOCHLCOUNT                1
#define RS485DEVCOUNT             1

typedef struct TDevCfg {
    struct TNetCfgPkt NetCfgPkt;                            //�豸�������ð�
    struct TWiFiCfgPkt WiFiCfgPkt;
    struct TDevInfoPkt DevInfoPkt;                          //�豸��Ϣ��
    struct TUserCfgPkt UserCfgPkt;								//�û����ð�
    struct TAlmCfgPkt AlmCfgPkt;                            //�������ð�
    struct TRS485CfgPkt RS485CfgPkt;                        //485ͨ�Ű�
    struct TDiDoCfgPkt DiDoCfgPkt;                          //DIDO���ð�
    struct TVideoCfgPkt VideoCfgPkt[VIDEOCHLCOUNT];         //��Ƶ���ð�--��ͨ��
    struct TAudioCfgPkt AudioCfgPkt[AUDIOCHLCOUNT];         //��Ƶ���ð�--��ͨ��
    struct TMDCfgPkt MDCfgPkt[VIDEOCHLCOUNT];               //�ƶ�����--��ͨ��
    struct THideAreaCfgPkt HideAreaCfgPkt[VIDEOCHLCOUNT];   //����¼Ӱ�����--��ͨ��
    struct TDiskCfgPkt DiskCfgPkt;													//�������ð�
    struct TRecCfgPkt RecCfgPkt[VIDEOCHLCOUNT];             //¼Ӱ���ð�
    struct TFTPCfgPkt FTPCfgPkt;
    struct TSMTPCfgPkt SMTPCfgPkt;
    int Flag;
    int Flag1;
  }TDevCfg;
//-----------------------------------------------------------------------------
  typedef struct TAlmState {//δʹ��
    int NightDetection;
    int VideoBlind;
    int MotionDetection;
    int VideoLost;
  }TAlmState;

  //-----------------------------------------------------------------------------
  typedef struct TPIDShmPkt { //δʹ��  	 //������PID����
    char AppName[64];
    int PID;
    dword time;
    int Flag;
  }TPIDShmPkt;
//-----------------------------------------------------------------------------
  typedef enum TCollectGrab{ //sizeof 4 BYTE
    CollectGrabNULL     = 0,
    OV7640              = 1,
    OV7660              = 2,
    MT9V111             = 3,
    OV9655              = 4,
    SONY_ICX408AK_CCD   = 5,
    SONY_ICX409AK_CCD   = 6,
    S7113_Composite     = 7,
    S7113_S_Video       = 8,
    IPTV_7113_S_Video   = 9,
    IPTV_7113_Tuner     =10,
    IPTV_7113_Composite =11,
    OV7720_OV7725       =12,
    S7137_SVideo        =13,
    S7137_Tuner         =14,
    S7137_Composite     =15,
    MT9M112_x           =16,
    OV7710_x            =17,
    Sony_IT1            =18,
    MT9M131             =19,								//ʹ�ô���
    TW9910_SVideo_1     =20,
    TW9910_Composite_1  =21,
    TW2835_module_1     =22,
    TW2835_module_2     =23,
    TW9910_SVideo_2     =24,
    TW9910_Composite_2  =25,
    MT9P031             =26,
    TW2815_CH0          =27,
    TW2815_CH1          =28,
    TW2815_CH2          =29,
    TW2815_CH3          =30,
    OV3642              =31,
    OV9710              =32,
    MT9V131             =33,
    MT9V136             =34,
    MT9D112             =35,
    MT9M131_RAW         =36,
    SONY3172            =37,
    OV2650              =38,
    CollectGrabMAX      =39
  }TCollectGrab;

//-----------------------------------------------------------------------------

  typedef struct TRes {//δʹ��		  //��־����
    struct {
      char* Login;//�û���¼
      char* PlayLive;//��ʼ�����ֳ�
      char* DOControl;//DO����
      char* Alarm;//����
      char* ClearAlarm;//�رվ���
      char* SetTime;//����ʱ��
      char* SetDevInfo;//�����豸��Ϣ
      char* SetNetCfg;//������������
      char* SetAlmCfg;//����Alarm����
      char* SetRecCfg;//����¼Ӱ����
      char* SetVideoCfg;//������Ƶ����
      char* SetAudioCfg;//������Ƶ����
      char* SetMDCfg;//�ƶ��������
      char* SetHideArea;//��¼
      char* SetRS485Cfg;
      char* SetDiDoCfg;
      char* SetDevReboot;//�����豸
      char* SetDevDefault;//ϵͳ�ص�ȱʡ���� Pkt.Value ���ָ�IP; Pkt.Value �ָ�IP
      char* SetUserCfg;//�����û��б�
      char* UpgradeImage;//����Ӱ��
      char* UploadFile;//�ϴ��ļ�
      char* DeleteFile;//ɾ���ļ�
    }Log;
    struct {
      char* None;
      char* MotionDetection;//λ�Ʊ���Motion Detection
      char* DigitalInput;//DI����
      char* VideoLost;//��Ƶ��ʧ
    }Alm;
  }TRes;

  //-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif
