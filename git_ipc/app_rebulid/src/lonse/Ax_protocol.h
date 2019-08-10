#ifndef Ax_protocol_H
#define Ax_protocol_H 

#include "cm_types.h"

#define Port_Ax_CmdData        3001                        //TCP ȱʡ�������ݶ˿� �Խ��˿�
#define Port_Ax_http             80
#define Port_Ax_RTSP            554

#define Port_Ax_Multicast      1900
#define Port_Ax_Search_Local   1899
#define IP_Ax_Multicast  "239.255.255.250"//uPnP IP��ѯ �ಥ�Խ�IP
#define REC_FILE_EXT           "ra2"
#define REC_FILE_EXT_DOT       ".ra2"


#pragma option push -b
typedef enum TDevType {// sizeof 4
  dt_None=0,
  dt_MgrSvr=1,
  dt_VideoSvr=2,
  dt_S3=3,
  dt_S4=4,
  dt_S5=5,
  dt_S6=6,
  dt_S7=7,
  dt_S8=8,
  dt_S9=9,
  dt_Clt=10,
  dt_DevA1=11,
  dt_DevC2=12,
  dt_DevA3=13,
  dt_DevA4=14,			//ʹ�ô���
  dt_DevC5=15,
  dt_D6=16,
  dt_D7=17,
  dt_D8=18,
  dt_D9=19,
  dt_D10=20,
  dt_DevD1=21,
  dt_DevD4=22,
  dt_DevOther
}TDevType;

//-----------------------------------------------------------------------------
typedef struct TSMTPCfgPkt {//sizeof 500 Byte
  int Active;
  char100 Subject;
  char100 Reply;
  char100 Sender;
  char100 SMTPServer;
  int SMTPPort;
  char40 Account;
  char40 Password;
  int SSL;
  int Flag;
  int Flag1;
}TSMTPCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TFTPCfgPkt {//sizeof 320 Byte
  int Active;
  char100 FTPServer;
  int FTPPort;
  char40 Account;
  char40 Password;
  char40 UploadPath;
  int PASVMode;
  int ProxyType;
  char40 ProxyUserName;
  char40 ProxyPassword;
  int Flag;
}TFTPCfgPkt;

//-----------------------------------------------------------------------------
typedef enum TGroupType{//sizeof 4 Byte
  pt_Cmd,
  pt_PlayLive,
  pt_PlayHistory,
  pt_PlayMedia
}TGroupType;
//-----------------------------------------------------------------------------
typedef enum TFontColor { //δʹ��			//OSD ������ɫ sizeof 4 Byte
  cl_Black       =0x00,
  cl_Maroon      =0x01,
  cl_Green       =0x02,
  cl_Olive       =0x03,
  cl_Navy        =0x04,
  cl_Purple      =0x05,
  cl_Teal        =0x06,
  cl_Red         =0x07,
  cl_Lime        =0x08,
  cl_Yellow      =0x09,
  cl_Blue        =0x0a,
  cl_Fuchsia     =0x0b,
  cl_Aqua        =0X0c,
  cl_Gray        =0x0d,
  cl_Silver      =0x0e,
  cl_White       =0x0f,
  cl_Transparent =0xff
}TFontColor;

static int FontColors[16+1] = //δʹ��
{
  0x000000,//cl_Black=0,//��
  0x000080,//cl_Maroon=1,//����
  0x008000,//cl_Green=2,//����
  0x008080,//cl_Olive=3,//����
  0x800000,//cl_Navy=4,//����
  0x800080,//cl_Purple=5,//��
  0x808000,//cl_Teal=6,//����
  0x0000FF,//cl_Red=7,//��
  0x00FF00,//cl_Lime=8,//��
  0x00FFFF,//cl_Yellow=9,//��
  0xFF0000,//cl_Blue=10,//��
  0xFF00FF,//cl_Fuchsia=11,//���
  0xFFFF00,//cl_Aqua=12,//��
  0x808080,//cl_Gray=13,//���
  0xC0C0C0,//cl_Silver=14,//��
  0xFFFFFF,//cl_White=15,//��
  0xFFFFFF//cl_Transparent=255//͸��
};
//-----------------------------------------------------------------------------
typedef enum TResolution {
  D1    = 0,
  HFD1  = 1,
  CIF   = 2,
  QCIF  = 3
}TResolution;
//-----------------------------------------------------------------------------
typedef enum TStandardEx {
  StandardExMin,
  P720x576,//1
  P720x288,
  P704x576,
  P704x288,
  P352x288,
  P176x144,
  N720x480,
  N720x240,
  N704x480,
  N704x240,
  N352x240,
  N176x120,
  V160x120,//  QQVGA
  V320x240,//   QVGA
  V640x480,//    VGA
  V800x600,//   SVGA
  V1024x768,//   XGA  
  V1280x720,//  WXGA yst
  V1280x800,//  WXGA
  //V1280x854,//  WXGA+
  V1280x960,//  _VGA
  V1280x1024,// SXGA
  V1360x768,// WXSGA+
  V1400x1050,// SXGA+
  V1600x1200,// UXGA
  V1680x1050,//WSXGA+
  V1920x1200,//WUXGA
  V2048x1536,// QXGA   
  V2560x1600,//QSXGAW
  V2560x2048,//QSXGA
  V3400x2400,//QUXGAW
  StandardExMax
}TStandardEx;
//-----------------------------------------------------------------------------
typedef enum TVideoType {                        //��Ƶ��ʽsizeof 4 Byte
  MPEG4          =0,
  MJPEG          =1,
  H264           =2,
}TVideoType;
//-----------------------------------------------------------------------------
typedef enum TImgFormat {
  if_RGB           =0,
  if_YUV420        =1,
  if_YUV422        =2,
}TImgFormat;
//-----------------------------------------------------------------------------
typedef enum TStandard {
  NTSC           =0,
  PAL            =1
}TStandard;
//-----------------------------------------------------------------------------
typedef struct TBatchCfgPkt { //�����޸����� sizeof 16
  int BitRate;                                   
  Byte Standard;                               
  Byte Resolution;                             
  Byte FrameRate;                              
  Byte IPInterval;                             
  int AudioActive;
  int DevTime;
}TBatchCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TVideoFormatEx { //δʹ��                   //��Ƶ��ʽ sizeof 64 ��δ�õ�
  Byte StandardEx;//TStandardEx
  Byte VideoType;//TVideoType                     //MPEG4=0x00, MJPEG=0x01  H264=0x02
  Byte FrameRate;                                //֡�� 1-30 MAX:PAL 25 NTSC 30
  Byte IPInterval;                               //IP֡��� 1-120 default 30

  Byte Brightness;                               //����   0-255
  Byte Contrast;                                 //�Աȶ� 0-255
  Byte Hue;                                      //ɫ��   0-255
  Byte Saturation;                               //���Ͷ� 0-255

  char40 Title;                                  //OSD���� 20������

  int  BitRate;                                  //���� 64K 128K 256K 512K 1024K 1536K 2048K 2560K 3072K
  Byte BitRateType;                              //0������ 1������
  bool FlipHorizontal;                           //ˮƽ��ת false or true
  bool FlipVertical;                             //��ֱ��ת false or true
  bool ShowOsdInDev;
  
  Byte OsdColor;
  bool IsShowTitle;                            //��ʾʱ����� false or true
  bool IsShowTime;                               //��ʾˮӡ false or true
  bool IsShowBitRate;

  int Flag;
}TVideoFormatEx;
//-----------------------------------------------------------------------------
typedef struct TVideoFormat {                    //��Ƶ��ʽ sizeof 128
  int  Standard;                                 //��ʽ PAL=1, NTSC=0
  int  Width;                                    //��
  int  Height;                                   //�� 
  TVideoType VideoType;                          //MPEG4=0x00, MJPEG=0x01  H264=0x02
  Byte  Brightness;                               //����   0-255
  Byte  Contrast;                                 //�Աȶ� 0-255
  Byte  Hue;                                      //ɫ��   0-255
  Byte  Saturation;                               //���Ͷ� 0-255
  int  FrameRate;                                //֡�� 1-30 
  int  IPInterval;                               //IP֡��� 10-120 
  int  BitRateType;                              //0������ 1������
  int  BitRate;                                  //���� 
  int  FlipHorizontal;                           //ˮƽ��ת false or true
  int  FlipVertical;                             //��ֱ��ת false or true
  char40 Title;                                  //OSD���� 20������

  Byte TitleFontSize;
  bool ShowTitleInDev;
  bool IsShowTitle;                            //��ʾʱ����� false or true
  Byte TitleColor;
  short TitleX;
  short TitleY;

  Byte TimeFontSize;
  bool ShowTimeInDev;
  bool IsShowTime;                               //��ʾˮӡ false or true
  Byte TimeColor;
  short TimeX;
  short TimeY;
/*
  Byte WaterMarkFontSize;//Reserved
  bool ShowWaterMarkInDev;//Reserved
  bool IsShowWaterMark;//Reserved
  Byte WaterMarkColor;//Reserved
  short WaterMarkX;//Reserved
  short WaterMarkY;//Reserved
*/
  Byte DeInterlaceType;
  bool IsDeInterlace;
  Byte Reserved[6];

  Byte FrameRateFontSize;
  bool ShowFrameRateInDev;
  bool IsShowFrameRate;
  Byte FrameRateColor;
  short FrameRateX;
  short FrameRateY;

  struct { 
    Byte VideoType;//MPEG4=0x0000, MJPEG=0x0001  H264=0x0002
    Byte StandardEx;//TStandardEx
    Byte FrameRate;//֡�� 1-30
    Byte BitRateType;//0������ 1������
    int BitRate;//����
  }Sub;//������

  int Flag;
}TVideoFormat;
//-----------------------------------------------------------------------------
typedef struct TVideoCfgPkt {                    //��Ƶ���ð� sizeof 148 Byte
  int  Chl;                                      //ͨ�� 0 
  int  Active;                                   //�Ƿ�����
  Byte InputType;                                //��������
  Byte CMOSFreq;                                 
  Byte Reserved[2];
  struct TVideoFormat VideoFormat;               //��Ƶ��ʽ
  int Flag;                                     
  int Flag1;
}TVideoCfgPkt;
//-----------------------------------------------------------------------------
typedef enum TAudioType {                        //��Ƶ��ʽsizeof 4 Byte
  AudioTypeNULL         =0x0000,
  PCM                   =0x0001,
  ADPCM                 =0x0002,
  IEEE_FLOAT            =0x0003,
  IBM_CVSD              =0x0005,  
  ALAW                  =0x0006,
  MULAW                 =0x0007,
  OKI_ADPCM             =0x0010,
  DVI_ADPCM             =0x0011,
  MEDIASPACE_ADPCM      =0x0012,  
  SIERRA_ADPCM          =0x0013,  
  G723_ADPCM            =0x0014,
  DIGISTD               =0x0015,  
  DIGIFIX               =0x0016,  
  DIALOGIC_OKI_ADPCM    =0x0017,
  MEDIAVISION_ADPCM     =0x0018,  
  YAMAHA_ADPCM          =0x0020,  
  SONARC                =0x0021,
  DSPGROUP_TRUESPEECH   =0x0022,  
  ECHOSC1               =0x0023,  
  AUDIOFILE_AF36        =0x0024,
  APTX                  =0x0025,  
  AUDIOFILE_AF10        =0x0026,  
  DOLBY_AC2             =0x0030,
  GSM610                =0x0031,
  MSNAUDIO              =0x0032,
  ANTEX_ADPCME          =0x0033,  
  CONTROL_RES_VQLPC     =0x0034,
  DIGIREAL              =0x0035,  
  DIGIADPCM             =0x0036,  
  CONTROL_RES_CR10      =0x0037,
  NMS_VBXADPCM          =0x0038,  
  CS_IMAADPCM           =0x0039,  
  ECHOSC3               =0x003A,
  ROCKWELL_ADPCM        =0x003B,  
  ROCKWELL_DIGITALK     =0x003C,  
  XEBEC                 =0x003D,
  G721_ADPCM            =0x0040,  
  G728_CELP             =0x0041,
  MPEGLAYER2            =0x0050,
  MPEGLAYER3            =0x0055,
  CIRRUS                =0x0060,  
  ESPCM                 =0x0061,  
  VOXWARE_1             =0x0062,
  CANOPUS_ATRAC         =0x0063,  
  G726_ADPCM            =0x0064,  
  G722_ADPCM            =0x0065,
  DSAT                  =0x0066,  
  DSAT_DISPLAY          =0x0067,  
  VOXWARE_2             =0x0075,
  SOFTSOUND             =0x0080,  
  RHETOREX_ADPCM        =0x0100,  
  CREATIVE_ADPCM        =0x0200,
  CREATIVE_FASTSPEECH8  =0x0202,  
  CREATIVE_FASTSPEECH10 =0x0203,  
  QUARTERDECK           =0x0220,
  FM_TOWNS_SND          =0x0300,  
  BTV_DIGITAL           =0x0400,  
  OLIGSM                =0x1000,
  OLIADPCM              =0x1001,  
  OLICELP               =0x1002,  
  OLISBC                =0x1003,
  OLIOPR                =0x1004,  
  LH_CODEC              =0x1100,  
  NORRIS                =0x1400,
}TAudioType;
//-----------------------------------------------------------------------------
typedef struct TAudioFormatEx { //δʹ��                   //��Ƶ��ʽ = sizeof 8 ��δ�õ�
  Byte AudioType;                              //PCM=0X0001, ADPCM=0x0011, MP2=0x0050, MP3=0X0055, GSM610=0x0031
  Byte nChannels;                               //������=0 ������=1
  WORD wBitsPerSample;                          //number of bits per sample of mono data 
  DWORD nSamplesPerSec;                          //������ 
}TAudioFormatEx;
//-----------------------------------------------------------------------------
typedef struct TAudioFormat {                    //��Ƶ��ʽ = TWaveFormatEx = sizeof 32
  DWORD wFormatTag;                              //PCM=0X0001, ADPCM=0x0011, MP2=0x0050, MP3=0X0055, GSM610=0x0031
  DWORD nChannels;                               //������=0 ������=1
  DWORD nSamplesPerSec;                          //������ 
  DWORD nAvgbytesPerSec;                         //for buffer estimation 
  DWORD nBlockAlign;                             //block size of data 
  DWORD wBitsPerSample;                          //number of bits per sample of mono data 
  DWORD cbSize;                                  //������С
  int Flag;                                      
}TAudioFormat;
//-----------------------------------------------------------------------------
typedef struct TAudioCfgPkt {                    //��Ƶ���ð� sizeof 48 Byte
  int  Chl;                                      //ͨ��0
  int  Active;                                   //�Ƿ���������
  struct TAudioFormat AudioFormat;               //��Ƶ��ʽ
#define MIC_IN  0
#define LINE_IN 1
  int InputType;                                 //0 MIC����, 1 LINE����
  int Flag;                                     
}TAudioCfgPkt;
//-----------------------------------------------------------------------------
typedef enum TPlayCtrl {                         //���ſ���sizeof 4 Byte
  PS_None               =0,                 //��
  PS_Play               =1,                 //����
  PS_Pause              =2,                 //��ͣ
  PS_Stop               =3,                 //ֹͣ
  PS_FastBackward       =4,                 //����
  PS_FastForward        =5,                 //���
  PS_StepBackward       =6,                 //����
  PS_StepForward        =7,                 //����
  PS_DragPos            =8,                 //�϶�
}TPlayCtrl; 
//-----------------------------------------------------------------------------
typedef struct TPlayCtrlPkt {
  TPlayCtrl PlayCtrl;
  DWORD Speed;
  DWORD Pos;
}TPlayCtrlPkt;
//-----------------------------------------------------------------------------
typedef struct TRecFilePkt {                     //������ʷ�� SizeOf 100
  char20 DevIP;
  byte Chl;
  byte RecType;                               //0:��ͨ¼Ӱ 1:����¼Ӱ 2ý���ļ�
  byte Reserved[2];
  int StartTime;                              //��ʼʱ��
  int EndTime;                                //����ʱ��
  char64 FileName;                               //�ļ���
  int Flag;                                      //����
}TRecFilePkt;
//-----------------------------------------------------------------------------
//  /sd/20091120/20091120_092749_0.ra2
typedef struct TRecFileIdx {                     //¼Ӱ�ļ������� sizeof 80
  char64 FileName;
  Byte Chl;
  Byte RecType;
  Byte Reserved;
  Byte Flag;
  time_t StartTime;
  time_t EndTime;
  DWORD FileSize;
}TRecFileIdx;

#define RECFILELSTCOUNT 16
typedef struct TRecFileLst { //sizeof 1288
  int Total;
  int SubTotal;
  TRecFileIdx Lst[RECFILELSTCOUNT];
}TRecFileLst;
//-----------------------------------------------------------------------------
typedef struct TRecFileHead {                    //¼Ӱ�ļ�ͷ��ʽ sizeof 256 Byte
  DWORD DevType;                                 //�豸���� = DEV_Ax
  DWORD FileSize;                                //�ļ���С
  int StartTime;                              //��ʼʱ��
  int EndTime;                                //ֹͣʱ��
  char20 DevName;                                //�豸ID
  char20 DevIP;                                  //�豸IP
  DWORD VideoChannel;                            //��Ƶͨ����ͳ��
  DWORD AudioChannel;                            //��Ƶͨ����ͳ��
  struct TVideoFormat VideoFormat;               //��Ƶ��ʽ
  struct TAudioFormat AudioFormat;               //��Ƶ��ʽ
  int Flag;                                      //����
  int Flag1;                                      //����
  int Flag2;                                      //����
  int Flag3;                                      //����
  int Flag4;                                      //����
  int Flag5;                                      //����
  int Flag6;                                      //����
  int Flag7;                                      //����
}TRecFileHead;
//-----------------------------------------------------------------------------
typedef struct TFilePkt {                        //�ϴ��ļ��� sizeof 272
  int FileType;                                    //1 PTZЭ��  2�������� 
  DWORD FileSize;
  char256 FileName;
  int Handle;
  int Flag;
  char256 DstFile;
}TFilePkt;
//-----------------------------------------------------------------------------
typedef enum TAlmType {
  Alm_None             =0,//��
  Alm_MotionDetection  =1,//λ�Ʊ���Motion Detection
  Alm_DigitalInput     =2,//DI����
  Alm_VideoLost        =3,//��Ƶ��ʧ
  Net_Disconn          =4,//�������
  Net_ReConn           =5,//��������
  Alm_HddFill          =6,//����
  Alm_VideoBlind       =7,//��Ƶ�ڵ�
  Alm_Other2           =8,//��������2
  Alm_Other3           =9,//��������3
  Alm_Other4           =10,//��������4
  Alm_Other5           =11,//��������5
  Alm_OtherMax         =12    
}TAlmType;              

typedef struct TAlmSendPkt {                     //�����ϴ���sizeof 36
  TAlmType AlmType;                              //��������
  int AlmTime;                                //����ʱ��
  int AlmPort;                                   //�����˿�
  char20 DevIP;
  int Flag;                                      //MD ��������
}TAlmSendPkt;
//-----------------------------------------------------------------------------
typedef struct TDiCfgPkt {                     //DI״̬�� sizeof 264
  int UsedCount;
  struct {
    bool Active;                                 //DI�Ƿ�� ����=0������=1
    Byte InputType;                              // 0 �͵�ƽ��1 �ߵ�ƽ 
    Byte Tag;
    Byte Tag1;
  }Lst[64];
  int Flag;
}TDiCfgPkt;

typedef struct TDoCfgPkt {                        //do���ð���sizeof 264
  int UsedCount;
  struct {
    bool Active;                                 //DO�Ƿ��
    Byte OutType;                                // 0 �͵�ƽ��1 �ߵ�ƽ
    Byte Tag;
    Byte Tag1;
  }Lst[64];
  int Flag;
}TDoCfgPkt;

typedef struct TDiDoCfgPkt {                    //sizeof 528
  struct TDiCfgPkt DiCfgPkt;
  struct TDoCfgPkt DoCfgPkt;
}TDiDoCfgPkt;

typedef struct TDoControlPkt {                 //do���ư���sizeof 16
  int Channel;
  int Value;                                   // 0 �ء�1 ��
  int Flag;
  int Flag1;
}TDoControlPkt;
//-----------------------------------------------------------------------------
typedef enum TTaskDayType{w_close,w_1,w_2,w_3,w_4,w_5,w_6,w_7,w_1_5,w_1_6,w_6_7,w_1_7,w_Interval} TTaskDayType;
//typedef enum TTaskDayType{w_close,w_1,w_2,w_3,w_4,w_5,w_6,w_7,w_1_5,w_1_6,w_6_7,w_1_7} TTaskDayType;
typedef struct TTaskhm {
  //TTaskDayType w;//w_close,w_1,w_2,w_3,w_4,w_5,w_6,w_7,w_1_5,w_1_6,w_6_7,w_1_7
  Byte w;//w_close,w_1,w_2,w_3,w_4,w_5,w_6,w_7,w_1_5,w_1_6,w_6_7,w_1_7,w_Interval
  Byte Days;
  Byte Reserved[2];
  Byte start_h;//ʱ 0-23
  Byte start_m;//�� 0-59
  Byte stop_h;//ʱ 0-23
  Byte stop_m;//�� 0-59
}TTaskhm;

#define MDLSTCOUNT                3
typedef struct TMDCfgPkt {                       //�ƶ����� sizeof 96
  int Chl;                                      
  int Active;                                    //false or true
  struct {
    bool Active;
    Byte Reserved;
    Byte Sensitive;                              //��������� 0-255
    Byte Tag;
    RECT1 Rect1;                        //������� sizeof 8
  }Lst[MDLSTCOUNT];
  struct TTaskhm hm[MDLSTCOUNT];
  int Flag;
}TMDCfgPkt;
//-----------------------------------------------------------------------------
#define ALMCFGLST       16
typedef struct TAlmCfgPkt {                   //�������ð� sizeof 268
  int AlmOutTimeLen;                    //�������ʱ��(��) 0 ..600 s
  int AutoClearAlarm;
  int Flag;
  struct {
    Byte AlmType;//Byte(TAlmType)
    Byte Channel;
    bool ActiveBuzzer;
    bool IsAlmRec;
    bool NetSend;
    bool ActiveDO;//DI����DOͨ�� false close
    Byte DOChannel;// 1-255 do channel
    Byte GotoPTZPoint;// >0ΪԤ��λ
  }Lst[ALMCFGLST];
  struct TTaskhm hm[ALMCFGLST];
}TAlmCfgPkt;
//-----------------------------------------------------------------------------
#define USER_GUEST     1 
#define USER_OPERATOR  2
#define USER_ADMIN     3
#define GROUP_GUEST    1
#define GROUP_OPERATOR 2
#define GROUP_ADMIN    3

#define MAXUSERCOUNT             20              //����û�����
typedef struct TUserCfgPkt {                     //sizeof 1048
  int Count;
  struct {
    int UserGroup;                                 //Guest=1 Operator=2 Administrator=3
    int Authority;                                 //3Ϊadmin ,
    char20 UserName;                               //�û��� admin���ܸ���
    char20 Password;                               //����
    int Flag;
  }Lst[MAXUSERCOUNT];
  int Flag;
}TUserCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TLogPkt {                         //sizeof 112
  int LogType;                                   //��־���� ����=0 ����=1 ��ͨ=2 ����=3
  int LogID;                                     //��־���
  int LogTime;                                   //����ʱ��
  char20 OperatUser;                             //�����û�
  char80 Content;                                //����
}TLogPkt;
//-----------------------------------------------------------------------------
typedef struct TLogSearchPkt {                   //��־��ѯ�� sizeof 16
  int LogType;                                   //��־���� ����=0 ����=1 ��ͨ=2 ����=3
  int StartTime;                       
  int EndTime;
  int Flag;
}TLogSearchPkt;
//-----------------------------------------------------------------------------
typedef struct TLogLstPkt {                  //��־���б� sizeof 1352
  int Total;
  int SubTotal;
  TLogPkt Lst[12];
}TLogLstPkt;
//-----------------------------------------------------------------------------
typedef enum TPTZCmd {                           //sizeof 4 Byte
  PTZ_None,
  PTZ_Up,//��
  PTZ_Up_Stop,//��ֹͣ
  PTZ_Down,//��
  PTZ_Down_Stop,//��ֹͣ
  PTZ_Left,//��
  PTZ_Left_Stop,//��ֹͣ
  PTZ_Right,//��
  PTZ_Right_Stop,//��ֹͣ

  PTZ_LeftUp,//����
  PTZ_LeftUp_Stop,//����ֹͣ
  PTZ_RightUp,//����
  PTZ_RightUp_Stop,//����ֹͣ
  PTZ_LeftDown,//����
  PTZ_LeftDown_Stop,//����ֹͣ
  PTZ_RightDown,//����
  PTZ_RightDown_Stop,//����ֹͣ

  PTZ_IrisIn,//��ȦС
  PTZ_IrisInStop,//��Ȧֹͣ
  PTZ_IrisOut,//��Ȧ��
  PTZ_IrisOutStop,//��Ȧֹͣ

  PTZ_ZoomIn,//����С
  PTZ_ZoomInStop,//����ֹͣ
  PTZ_ZoomOut,//���ʴ�
  PTZ_ZoomOutStop,//����ֹͣ

  PTZ_FocusIn,//����С
  PTZ_FocusInStop,//����ֹͣ
  PTZ_FocusOut,//�����
  PTZ_FocusOutStop,//����ֹͣ

  PTZ_LightOn,//�ƹ�С
  PTZ_LightOff,//�ƹ��
  PTZ_RainBrushOn,//��ˢ��
  PTZ_RainBrushOff,//��ˢ��
  PTZ_AutoOn,//�Զ���ʼ  //Rotation
  PTZ_AutoOff,//�Զ�ֹͣ

  PTZ_TrackOn,
  PTZ_TrackOff,
  PTZ_IOOn,
  PTZ_IOOff,

  PTZ_ClearPoint,//��̨��λ
  PTZ_SetPoint,//�趨��̨��λ
  PTZ_GotoPoint,//��̨��λ
  PTZ_SetPointRotation,
  PTZ_SetPoint_Left,
  PTZ_GotoPoint_Left,
  PTZ_SetPoint_Right,
  PTZ_GotoPoint_Right,
  PTZ_DayNightMode,//���졢ҹ��ģʽ 0���� 1ҹ��
  PTZ_Max
}TPTZCmd;
//-----------------------------------------------------------------------------
typedef enum TPTZProtocol {                      //��̨Э�� sizeof 4
  Pelco_P               =0,
  Pelco_D               =1,
  Protocol_Custom       =2,
}TPTZProtocol;

typedef struct TPTZPkt {                         //PTZ ��̨����  sizeof 108
  TPTZCmd PTZCmd;                                    //=PTZ_None Ϊ͸������
  union {
    struct {
      TPTZProtocol Protocol;                         //��̨Э��
      int Address;                                   //��̨��ַ
      int PanSpeed;                                  //��̨�ٶ�
      int Value;                                     //������Ԥ��λ
    };
    struct {
      char100 TransBuf;
      int TransBufLen;
    };
  };
}TPTZPkt;
//-----------------------------------------------------------------------------
typedef struct TPlayLivePkt {                    //�����ֳ���//sizeof 20
  DWORD VideoChlMask;//ͨ������ 
  DWORD AudioChlMask;
  int Value;                                     //Value=0��������֡��Value=1ֻ������ƵI֡
  DWORD SubVideoChlMask;
//11  int IsRecvAlarm;                               //0�����豸���� 1�������豸����
  int Flag;                                      //���� 
}TPlayLivePkt;
//-----------------------------------------------------------------------------
typedef struct TPlayBackPkt {                    //sizeof 20
  int Chl;
  int FileType;                                  //0:��ͨ¼Ӱ 1:����¼Ӱ 2ý���ļ�
  int StartTime;                                 //��ʼʱ��
  int EndTime;                                   //����ʱ��
  int Flag;
}TPlayBackPkt;
//-----------------------------------------------------------------------------
typedef enum TMsgID {
  Msg_None                  = 0,
  Msg_Login                 = 1001,//�û���¼
  Msg_PlayLive              = 1002,//��ʼ�����ֳ�
  Msg_StartPlayRecFile      = 1003,//����¼Ӱ�ļ�
  Msg_StopPlayRecFile       = 1004,//ֹͣ����¼Ӱ�ļ�
  Msg_StartRec              = 1007,//��ʼ�豸¼Ӱ
  Msg_StopRec               = 1008,//ֹͣ�豸¼Ӱ
  Msg_GetRecFileLst         = 1009,//ȡ��¼Ӱ�ļ��б�
  Msg_StartDownloadFile     = 1011,//��ʼ�����ļ�
  Msg_StopDownloadFile      = 1012,//ֹͣ�����ļ�
  Msg_StartUploadFile       = 1013,//��ʼ�ϴ��ļ�
  Msg_AbortUploadFile       = 1014,//ȡ���ϴ��ļ�
  Msg_StartTalk             = 1015,//��ʼ�Խ�
  Msg_StopTalk              = 1016,//ֹͣ�Խ�
  Msg_PlayControl           = 1017,//���ſ���
  Msg_PTZControl            = 1018,//��̨����
  Msg_Alarm                 = 1020,//����
  Msg_ClearAlarm            = 1021,//�رվ���
  Msg_SetBrightness         = 1022,//��������
  Msg_SetContrast           = 1023,//���öԱȶ�
  Msg_SetHue                = 1024,//����ɫ��
  Msg_SetSaturation         = 1025,//���ñ��Ͷ�
  Msg_GetTime               = 1026,//ȡ��ʱ��
  Msg_SetTime               = 1027,//����ʱ��
  Msg_GetDevInfo            = 1028,//ȡ���豸��Ϣ
  Msg_SetDevInfo            = 1029,//�����豸��Ϣ
  Msg_GetNetCfg             = 1030,//ȡ����������
  Msg_SetNetCfg             = 1031,//������������
  Msg_GetAlmCfg             = 1032,//ȡ��Alarm����
  Msg_SetAlmCfg             = 1033,//����Alarm����
  Msg_GetRecCfg             = 1034,//ȡ��¼Ӱ����
  Msg_SetRecCfg             = 1035,//����¼Ӱ����
  Msg_GetVideoCfg           = 1036,//ȡ����Ƶ����
  Msg_SetVideoCfg           = 1037,//������Ƶ����
  Msg_GetAudioCfg           = 1038,//ȡ����Ƶ����
  Msg_SetAudioCfg           = 1039,//������Ƶ����
  Msg_GetMDCfg              = 1040,//�ƶ��������
  Msg_SetMDCfg              = 1041,//�ƶ��������
  Msg_GetHideArea           = 1042,//��¼
  Msg_SetHideArea           = 1043,//��¼
  Msg_DeleteFile            = 1044,//ɾ���ļ�
  Msg_GetLogLst             = 1045,//ȡ����־�б�
  Msg_DelLog                = 1046,//ɾ����־
  Msg_SetDevReboot          = 1047,//�����豸
  Msg_SetDevLoadDefault     = 1048,//ϵͳ�ص�ȱʡ���� Pkt.Value= 0 ���ָ�IP, Pkt.Value= 1 �ָ�IP
  Msg_SendSense             = 1051,//1.���أ�����appsense���ͻظ�appsense, 
  Msg_GetDevRecFileHead     = 1052,//ȡ���豸�ļ��ļ�ͷ��Ϣ
  Msg_GetDevState           = 1053,//ȡ��ϵͳ״̬
  Msg_GetUserLst            = 1054,//ȡ���û��б�
  Msg_SetUserLst            = 1055,//�����û��б�
  Msg_GetRS485Cfg           = 1065,
  Msg_SetRS485Cfg           = 1066,
  Msg_GetDiDoCfg            = 1067,
  Msg_SetDiDoCfg            = 1068,
  Msg_GetFTPCfg             = 1069,
  Msg_SetFTPCfg             = 1070,
  Msg_KillUserConn          = 1071,//�Ͽ��û�����
  Msg_GetSMTPCfg            = 1072,
  Msg_SetSMTPCfg            = 1073,
  Msg_GetAllCfg             = 1081,//ȡ����������
  Msg_GetWiFiCfg            = 1082,//ȡ��WiFi����
  Msg_SetWiFiCfg            = 1083,//����WiFi����
  Msg_GetDiskCfg            = 1084,//����Disk����
  Msg_SetDiskCfg            = 1085,//����Disk����
  Msg_DevSnapShot           = 1100,//�豸����
  Msg_GetColors             = 1101,//ȡ�����ȡ��Աȶȡ�ɫ�ȡ����Ͷ�
  Msg_SetColors             = 1102,//�������ȡ��Աȶȡ�ɫ�ȡ����Ͷ�
  //Msg_SetColorDefault       = 1103,
  Msg_StartUploadFileEx     = 1113,//��ʼ�ϴ��ļ�tftp
  Msg_GetMulticastInfo      = 2001,
  Msg_SetMulticastInfoOld   = 2002,
  Msg_SetMulticastInfo      = 2003,
  Msg_SetBatchCfg           = 2004,//�����޸�����
  Msg_Sense                 = 2010,
  Msg_Debug                 = 3001,//����
  Msg_______
}TMsgID;
//-----------------------------------------------------------------------------
#define RECPLANLST 4
typedef struct TPlanRecPkt {                        //�ų�¼Ӱ�ṹ sizeof 224
  struct {
    bool Active;
    Byte start_h;    //ʱ 0-23
    Byte start_m;    //�� 0-59
    Byte stop_h;     //ʱ 0-23
    Byte stop_m;     //�� 0-59
    bool IsRun;      //��ǰ�ƻ��Ƿ�����
    Byte Flag1;
    Byte Flag2;
  }Week[7][RECPLANLST];                                 //��һ���������� ÿ�����4������
}TPlanRecPkt;
//-----------------------------------------------------------------------------
typedef enum TRecStyle {
  rs_RecManual,
  rs_RecAuto,
  rs_RecPlan,
  rs_RecAlarm
}TRecStyle;

typedef struct TRecCfgPkt {                      //¼Ӱ���ð� sizeof 260
  int ID;
  int DevID;//PC�˹������ֻ���ڴ洢���ݿ����豸��š��豸�˱���
  int Chl;
  bool IsLoseFrameRec;//�Ƿ�֡¼Ӱ
  byte RecStreamType;//0 ������ 1 ������
  byte Reserved;
  bool IsRecAudio;//¼����Ƶ ��û���õ�
  DWORD Rec_AlmPrevTimeLen;//��ǰ¼Ӱʱ��     5 s
  DWORD Rec_AlmTimeLen;//����¼Ӱʱ��        10 s
  DWORD Rec_NmlTimeLen;//һ��¼Ӱ�ָ�ʱ��   600 s
  TRecStyle RecStyle;//¼Ӱ����
  TPlanRecPkt Plan;
  int bFlag;
}TRecCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TDiskCfgPkt {   //sizeof 888
  int IsFillOverlay;      // �Ƿ񸲸������ļ�(false��true,falseΪ������,trueΪ����,ȱʡΪfalse)
  char20 CurrentDiskName; // ��ǰ����¼Ӱ�Ĵ������� 0..7, ReadOnly
  struct {
    char20 DiskName;      // ���� 
    int Active;           // �Ƿ���Ϊ¼Ӱ���� false or true
    DWORD DiskSize;       // M ReadOnly
    DWORD FreeSize;       // M
    DWORD MinFreeSize;    // M
  }Disk[24];
}TDiskCfgPkt;
//-----------------------------------------------------------------------------
#define HIDEAREALSTCOUNT          3
typedef struct THideAreaCfgPkt {                 //����¼Ӱ����� sizeof 72
  int Chl;                                       //ͨ�� 0..15 ��Ӧ 1..16ͨ��
  int Active;
  struct {
    int Active;                                    //false or true
    RECT1 Rect1;
  }Lst[HIDEAREALSTCOUNT];
  int Flag;
}THideAreaCfgPkt;
//-----------------------------------------------------------------------------
typedef enum TLanguage {
  cn = 0,
  tw = 1,
  en = 2 
}TLanguage;
static const  char* DevLanguage[3] = {"cn","tw","en"};
//-----------------------------------------------------------------------------
typedef struct TAxInfo {//sizeof 40
  union {
    char40 Reserved;
    struct {
      bool ExistWiFi;
      bool ExistSD;
      bool ExistUSB;
      bool ExistHD;       // 4
      DWORD VideoTypeMask;// 8
      uint64 StandardMask;//16
      DWORD AutioTypeMask;//20��δ�õ�
      bool NotExistAudio;
      bool NotExistIO;
      bool NotExistRS485;
      byte flag;
      uint64 SubStandardMask;//32
    };
  };
}TAxInfo;

#pragma pack(4)
typedef struct TDevInfoPkt {                     //�豸��Ϣ��sizeof 180
  char DevModal[12];                             //�豸�ͺ� 
  DWORD SN;
  int DevType;                                   //�豸����
  char20 SoftVersion;                            //����汾
  char20 FileVersion;                            //�ļ��汾
  char20 DevName;                                //�豸��ʶ
  char40 DevDesc;                                //�豸��ע
  struct TAxInfo Info;

  int VideoChlCount;
  Byte AudioChlCount;
  Byte DiChlCount;
  Byte DoChlCount;
  Byte RS485DevCount;
  Byte Language;//TLanguage
  Byte MaxUserConn;                               //����û������� default 10
  Byte OEMType;
  bool DoubleStream;                              //�Ƿ�˫���� 
  struct {
    Byte w;//TTaskDayType;
    Byte start_h;//ʱ 0-23
    Byte start_m;//�� 0-59
    Byte Days;
  }RebootHM;
  int Flag;
}TDevInfoPkt;
#pragma pack()
//-----------------------------------------------------------------------------
typedef struct TUserConnInfoPkt {//�豸���û�������Ϣ sizeof 12
  int LoginTime;
  DWORD ClientIP;
  WORD ClientPort;
  Byte GroupType;//(Byte)TGroupType
  Byte UserGroup;//Guest=1 Operator=2 Administrator=3
}TUserConnInfoPkt;
//-----------------------------------------------------------------------------
typedef struct TDevStatePkt {                    //�豸״̬��Ϣ�� 1020
  struct {  //Ӳ����Ϣ
    Byte DiskType;                                  //1=HD  2=SD 3=USB
    Byte Reserved;
    WORD Tag;
    DWORD TotalSpace;                              //Ӳ�̴�С(M)
    DWORD FreeSpace;                               //Ӳ��ʣ��ռ�( M )
  }HD[4];
  int ConnCount;//�����û�����
  TUserConnInfoPkt UserConnInfoPkt[80];         //�����û���Ϣ
  //int Flag;                                      //����
  //int Flag1;                                     //����
  int LastVideoDataTime;
  int LastAudioDataTime;
  int NetworkOK;
  //char20 wIP;
}TDevStatePkt;
//-----------------------------------------------------------------------------
typedef struct TWiFiCfgPkt {                     //�������ð� sizeof 200
  int Active;
  char20 DevIP;
  char20 SubMask;
  char20 Gateway;
  char32 SSID;
  int Channel;//Ƶ��1..14 default 1=Auto
#define Encrypt_None   0
#define Encrypt_WEP    1
#define Encrypt_WPA    2
  int EncryptType;//(Encrypt_None,Encrypt_WEP,Encrypt_WPA);
  char64 Password;
  union {
    struct {
      char32 ValueStr;
    };
    struct {
      int WEPKeyBit;//(kBit64,kBit128);
      int WEPIndex;//0..3;//=0
    };
    struct {
      int WPAKeyType;//(TKIP,AES);
    };
  };
}TWiFiCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TNetCfgPkt {                      //�豸�������ð�sizeof 372
  int CmdPort;                                   //�������ݶ˿�
  int rtspPort;                                  //rtsp�˿�
  int HttpPort;                                  //http��ҳ�˿�
  /*WORD CmdPort;                                   //�������ݶ˿�
  WORD wCmdPort;
  WORD rtspPort;                                  //rtsp�˿�
  WORD wrtspPort;
  WORD HttpPort;                                  //http��ҳ�˿�
  WORD wHttpPort;*/
  struct {
    int IPType;               // 0=��̬IP(StaticIP) 1=��̬IP(DHCP)
    char20 DevIP;
    char20 DevMAC;
    char20 SubMask;
    char20 Gateway;
    char20 DNS1;
    char20 DNS2;
    //char20 wIP;
    int Flag;
  }Lan;
  struct {
    int Active;
    int DDNSType;                               //0=3322.ORG 1=dynDNS.ORG
    char40 DDNSDomain;                           //��DDNS SERVER IP
    char40 HostAccount;                          //DDNS�ʺ�
    char40 HostPassword;                         //DDNS����
    int Flag;
  }DDNS;
  struct {
    int AutoStart;
    char40 Account;
    char40 Password;
    int Flag;
  }PPPOE;
  struct {
    int Active;
    int Flag;
  }uPnP;
  int Flag;
}TNetCfgPkt;
//-----------------------------------------------------------------------------
typedef enum TBaudRate{
  BaudRate_1200  =    1200,
  BaudRate_2400  =    2400,
  BaudRate_4800  =    4800,
  BaudRate_9600  =    9600,
  BaudRate_19200  =  19200,
  BaudRate_38400  =  38400,
  BaudRate_57600  =  57600,
  BaudRate_115200 = 115200
}TBaudRate;

typedef enum TDataBit{
  DataBit_5 = 5,
  DataBit_6 = 6,
  DataBit_7 = 7,
  DataBit_8 = 8
}TDataBit;

typedef enum TParityCheck{
  ParityCheck_None  = 0,
  ParityCheck_Odd   = 1,
  ParityCheck_Even  = 2,
  ParityCheck_Mask  = 3,
  ParityCheck_Space = 4
}TParityCheck;

typedef enum TStopBit{
  StopBit_1   = 0,
  StopBit_1_5 = 1,
  StopBit_2   = 2
}TStopBit;

typedef struct TRS485CfgPkt {                       //485ͨ�Ű� sizeof 280
  int Chl;
  TBaudRate BPS;//������
  TDataBit DataBit;//����λ
  TParityCheck ParityCheck;//��żУ��
  TStopBit StopBit;//ֹͣλ
  struct {
    Byte Address;
    Byte PTZProtocol;//��̨Э��
    Byte PTZSpeed;
    Byte Reserved;
  }Lst[32];//��Ӧ��Ӧ����Ƶͨ��
  
  //char PTZNameLst[128];//��ʱδ�õ� format "Pelco_P\nPelco_D\nProtocol_Custom"

  int PTZCount;
  char20 PTZNameLst[6];
  int Reserved;

  int Flag;
}TRS485CfgPkt;

//-----------------------------------------------------------------------------
typedef struct TColorsPkt {
  int Chl;
  Byte  Brightness;                               //����   0-255
  Byte  Contrast;                                 //�Աȶ� 0-255
  Byte  Hue;                                      //ɫ��   0-255
  Byte  Saturation;                               //���Ͷ� 0-255
}TColorsPkt;
//-----------------------------------------------------------------------------
typedef struct TMulticastInfoPkt {               //�ಥ������Ϣ��sizeof 556->588
  TDevInfoPkt DevInfo;
  TNetCfgPkt NetCfg;
  int Flag;
  struct {
    int  Standard;                                 //��ʽ PAL=1, NTSC=0
    int  Width;                                    //�� 720 360 180 704 352 176 640 320 160
    int  Height;                                   //�� 480 240 120 576 288 144 
    TVideoType VideoType;                          //MPEG4=0x00, MJPEG=0x01  H264=0x02
  }v;
  struct {
    DWORD wFormatTag;                              //PCM=0X0001, ADPCM=0x0011, MP2=0x0050, MP3=0X0055, GSM610=0x0031
    DWORD nChannels;                               //������=0 ������=1
    DWORD nSamplesPerSec;                          //������ 
    DWORD wBitsPerSample;                          //number of bits per sample of mono data 
  }a;
  TWiFiCfgPkt WiFiCfg;
}TMulticastInfoPkt;
//-----------------------------------------------------------------------------
#define Head_CmdPkt           0xAAAAAAAA         //�������ͷ
#define Head_VideoPkt         0xBBBBBBBB         //��Ƶ����ͷ
#define Head_AudioPkt         0xCCCCCCCC         //��Ƶ����ͷ
#define Head_TalkPkt          0xDDDDDDDD         //�Խ�����ͷ
#define Head_UploadPkt        0xEEEEEEEE         //�ϴ���
#define Head_DownloadPkt      0xFFFFFFFF         //���ذ�
#define Head_CfgPkt           0x99999999         //���ð�
#define Head_SensePkt         0x88888888         //����
//-----------------------------------------------------------------------------

#pragma pack(4)
typedef struct THeadPkt{                         //sizeof 8
  DWORD VerifyCode;                              //У���� = 0xAAAAAAAA 0XBBBBBBBB 0XCCCCCCCC 0XDDDDDDDD 0XEEEEEEEE
  DWORD PktSize;                                 //������С=1460-8(waiting)
}THeadPkt;

#pragma pack()
//-----------------------------------------------------------------------------
typedef struct TFrameInfo { //¼Ӱ�ļ�����֡ͷ  16 Byte
  Int64 FrameTime;                               //֡ʱ�䣬time_t*1000000 +us
  Byte Chl;                                      //ͨ�� 0
  bool IsIFrame;                                 //�Ƿ�I֡
  WORD FrameID;                                  //֡����,��0 ��ʼ,��65535���ܶ���ʼ
  union {
    DWORD PrevIFramePos;                         //ǰһ��I֡�ļ�ָ�룬�����ļ��д�������������
    int StreamType;                              //0Ϊ������ 1Ϊ������ 
    DWORD DevID;                                 //�����Ӷ��豸ʱ�õ����ݱ���
  };
}TFrameInfo;

typedef struct TDataFrameInfo { //¼Ӱ�ļ�����֡ͷ  24 Byte
  THeadPkt Head;
  TFrameInfo Frame;
}TDataFrameInfo;
//-----------------------------------------------------------------------------
//�������
#define ERR_FAIL           0
#define ERR_OK             1
#define ERR_MAXUSERCONN    10001//�����û�����������趨
//-----------------------------------------------------------------------------
typedef struct TLoginPkt {                       //�û���¼�� sizeof 252->892
  char20 UserName;                               //�û�����
  char20 Password;                               //�û�����
  char20 DevIP;                                  //Ҫ���ӵ��豸IP,�� host
  int UserGroup;                                 //Guest=1 Operator=2 Administrator=3  
  int SendSensePkt;                              //�Ƿ������� 0������ 1����
  TDevInfoPkt DevInfoPkt;
  TVideoFormat v[4];
  TAudioFormat a[4];
  int Flag;//�����Ƿ����ߡ�0�����ߡ�1����
}TLoginPkt;
//-----------------------------------------------------------------------------
typedef struct TCmdPkt {                         //sizeof 1460-8
  DWORD PktHead;                                 //��ͷУ���� =Head_CmdPkt 0xAAAAAAAA
  TMsgID MsgID;                                  //��Ϣ
  DWORD Session;                                 //�����û���ɣ������������¼��ʱ��ֵΪ0�����ڷ��ص�¼����Session  //����Ϊ�����ڲ�ͨѶ��ʱ����ֵ����
  DWORD Value;                                   //���Ի򷵻�ֵ 0 or 1 or ErrorCode
  union {
    char ValueStr[1460- 4*4 - 8];
    struct TLoginPkt LoginPkt;                   //��¼��
    struct TPlayLivePkt LivePkt;                 //�����ֳ���
    struct TRecFilePkt RecFilePkt;               //�ط�¼Ӱ��
    struct TPTZPkt PTZPkt;                       //��̨����̨
    struct TRecFileLst RecFileLst;
    struct TPlayCtrlPkt PlayCtrlPkt;             //�ط�¼Ӱ���ư�

    struct TAlmSendPkt AlmSendPkt;               //�����ϴ���
    struct TDevInfoPkt DevInfoPkt;               //�豸��Ϣ��
    struct TLogSearchPkt LogSearchPkt;           //��־��ѯ��
    struct TLogLstPkt LogLst;                    //��־���б�
    struct TNetCfgPkt NetCfgPkt;                 //�豸�������ð�
    struct TWiFiCfgPkt WiFiCfgPkt;               //�����������ð�
    struct TDiskCfgPkt DiskCfgPkt;               //�������ð�
    struct TUserCfgPkt UserCfgPkt;               //�û����ð�
    struct TRecCfgPkt RecCfgPkt;                 //¼Ӱ���ð�
    struct TMDCfgPkt MDCfgPkt;                   //�ƶ�����--��ͨ��
    struct TDiDoCfgPkt DiDoCfgPkt;               //DIDO���ð�
    struct TDoControlPkt DoControlPkt;           //DO���ư�    
    struct THideAreaCfgPkt HideAreaCfgPkt;       //����¼Ӱ�����--��ͨ��
    struct TAlmCfgPkt AlmCfgPkt;                 //�������ð�
    struct TVideoCfgPkt VideoCfgPkt;             //��Ƶ���ð�--��ͨ��
    struct TAudioCfgPkt AudioCfgPkt;             //��Ƶ���ð�--��ͨ��    
    struct TRecFileHead FileHead;                //ȡ���豸�ļ��ļ�ͷ��Ϣ
    struct TDevStatePkt DevStatePkt;             //�豸״̬��Ϣ��
    struct TFilePkt FilePkt;                     //�ϴ��ļ���
    struct TRS485CfgPkt RS485CfgPkt;             //485ͨ�Ű�--��ͨ��
    struct TUserConnInfoPkt UserConnInfoPkt;     //����Ҫ���ڶϿ�ĳЩ�û�����
    struct TColorsPkt Colors;                    //����ȡ�����ȡ��Աȶȡ�ɫ�ȡ����Ͷ�

    struct TMulticastInfoPkt MulticastInfo;      //�ಥ��Ϣ

    struct TFTPCfgPkt FTPCfgPkt;
    struct TSMTPCfgPkt SMTPCfgPkt;
    struct TBatchCfgPkt BatchCfgPkt;             //�����޸�����
  };
}TCmdPkt;
//-----------------------------------------------------------------------------
typedef struct TNetCmdPkt {                      //���緢�Ͱ� sizeof 1460
  struct THeadPkt HeadPkt;
  struct TCmdPkt CmdPkt;
}TNetCmdPkt;

//#pragma pack(pop)
//-----------------------------------------------------------------------------
typedef struct TTalkHeadPkt {                    //�Խ�����ͷ  sizeof 32
  DWORD VerifyCode;                              //У���� = 0XDDDDDDDD
  DWORD PktSize;                                 
  char20 TalkIP;
  DWORD TalkPort;
}TTalkHeadPkt;

//*****************************************************************************
//*****************************************************************************
//**DECODER PROTOCOL***DECODER PROTOCOL***DECODER PROTOCOL***DECODER PROTOCOL**
//*****************************************************************************
//*****************************************************************************

typedef enum TdecMsgID{//�������� sizeof 4 Byte
  decmsg_None,               //��
  decmsg_Login,              //��¼
  decmsg_Sense,              //����
  decmsg_Reboot,             //�����豸
  decmsg_Default,            //ϵͳ�ص�ȱʡ����
  decmsg_SearchencDev,       //��ѯA1, A3, A4�����豸
  decmsg_GetTime,            //ȡ��ʱ��
  decmsg_SetTime,            //����ʱ��
  decmsg_GetDevCfg,          //ȡ���豸����
  decmsg_SetDevCfg,          //�����豸����
  decmsg_GetPlayCfg,         //ȡ�ò�������
  decmsg_SetPlayCfg,         //���ò�������
  decmsg_PTZControl,         //��̨����
  decmsg_TransSend,          //͸����������(����)
  decmsg_Ping,               //Զ��ping
  decmsg_SetSplit,           //�ָ���Ļ
  decmsg_SelectSplit,        //ѡ��ָ��
  decmsg_StartTalk,          //��ʼ�Խ�
  decmsg_Talking,            //�Խ���
  decmsg_StopTalk,           //ֹͣ�Խ�
  decmsg_DownloadFile,       //��ʼ�����ļ�
  decmsg_AbortDownloadFile,  //ȡ�������ļ�
  decmsg_UploadFile,         //��ʼ�ϴ��ļ�
  decmsg_AbortUploadFile,    //ȡ���ϴ��ļ�
  //decmsg_Play,               //��ʼ����
  //decmsg_Pause,              //��ͣ����
  //decmsg_Stop,               //ֹͣ����
  //decmsg_PlayPrev,           //����ǰһ����
  //decmsg_PlayNext,           //���ź�һ����
  //decmsg_PlayIndex,          //����ָ������
  //decmsg_StartInsPlay,       //��ʼ���벥��
  //decmsg_StopInsPlay,        //ֹͣ���벥��
  decmsg_None_max
}TdecMsgID;

typedef struct TdecHeadPkt{//���������ͷ sizeof 16
  DWORD VerifyCode;          //У���� = 0xAAAAAAAA
  TdecMsgID MsgID;           //��Ϣ
  DWORD Value;               //����һЩ����
  int NextSize;              //��������С
}TdecHeadPkt;

typedef struct TencDevInfo{//A1 A3 A4�豸��Ϣ sizeof 156
  DWORD encDevID;            // = SN ?
  char40 DevHost;            //�������豸IP������
  char40 SvrHost;            //ת��������IP������
  char20 DevName;            //�豸����
  char20 UserName;           //�������豸�ʺ�
  char20 Password;           //�������豸����
  WORD DataPort;             //�������豸�˿�
  Byte DevType;              //�������豸TDevType
  Byte Channel;              //����ͨ��
  TStandardEx Standard;
  int Reserved;
}TencDevInfo;

typedef struct TPlayItem { //sizeof 36
  DWORD PlayTime;//��
  struct {
    DWORD encDevID;
    bool IsFullScreen;
    bool IsPlayAudio;
    WORD Reserved;
  }Play[4];
}TPlayItem;

typedef struct TdecPlayCfg {//sizeof 16
  DWORD DevCount;
  struct TencDevInfo* DevLst;
  DWORD ItemsCount;
  struct TPlayItem* Items;
}TdecPlayCfg;

typedef struct TdecDevCfg{ //�豸��Ϣ��sizeof 1048
  struct {
    DWORD SN;                  //���к�
    Byte DevType;              //TDevType
    Byte ChlCount;             //ͨ������
    Byte OEMType;              //OEM�汾����
    Byte Language;             //TLanguage;
    TVersion SoftVersion;      //����汾
    TVersion FileVersion;      //�ļ��汾
    char20 DevName;            //�豸����
    Byte Reserved[8];
  }Info;
  struct {
    WORD CmdPort;              //�������ݶ˿�
    WORD HttpPort;             //http��ҳ�˿�
    char20 DevIP;              //�豸IP
    char20 DevMAC;             //�豸MAC��ַ
    char20 SubMask;            //�豸��������
    char20 Gateway;            //�豸����
    char20 DNS1;               //�豸DNS
  }Net;
  struct {
    Byte w;                    //TTaskDayType;
    Byte start_h;              //��w!=w_IntervalʱֵΪ0-23, ��w==w_IntervalʱΪ���Сʱ
    Byte start_m;              //��0-59
    Byte tag;                  //����
  }RebootHM;                 //��ʱ����
  struct {
    int Count;                 //�û����� = Lst������
    struct {
      char20 UserName;           //�û��� admin�û����ܸ���
      char20 Password;           //����
      DWORD Authority;           //Guest=1 Operator=2 Administrator=3
    }Lst[MAXUSERCOUNT];
  }User;
  int Flag;                  //����
}TdecDevCfg;

typedef struct TdecLoginPkt {//�û���¼�� sizeof 48
  char20 UserName;           //�û�����
  char20 Password;           //�û�����
  int Flag;
  int Reserved;
}TdecLoginPkt;

typedef struct TdecPingPkt { //Զ��PING��  sizeof 48
  char40 RemoteIP;
  DWORD RemotePort;          //Ϊ0ʱΪICMP PING, ����0ʱΪTCP PING
  DWORD TimeOut;             //��ʱ,��λms
}TdecPingPkt;

typedef struct TdecSearchPkt {//��ѯ�� sizeof 1064
  TdecHeadPkt Head;
  TdecDevCfg Cfg;
}TdecSearchPkt;

#pragma option pop //end C++Builder enum 4 Byte

#endif //end Ax_protocol_H

