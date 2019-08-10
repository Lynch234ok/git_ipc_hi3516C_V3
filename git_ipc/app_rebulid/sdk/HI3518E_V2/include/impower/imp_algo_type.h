/**
* \defgroup �ӿ�
* �ӿ�
* @author �������ټ���
* @version 2.0
* @data 2009-2010
*/
/*@{*/

/**
* \defgroup �ṹ���Ͷ���
* �㷨�������Ͷ���
* @author �������ټ���
* @version 2.0
* @data 2009-2010
*/
/*@{*/
#ifndef _IMP_ALGO_TYPE_H_
#define _IMP_ALGO_TYPE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char IMP_U8;
typedef unsigned char IMP_UCHAR;
typedef unsigned short IMP_U16;
typedef unsigned int IMP_U32;
typedef char IMP_S8;
typedef short IMP_S16;
typedef int IMP_S32;
#ifndef _M_IX86
	typedef unsigned long long IMP_U64;
	typedef long long IMP_S64;
#else
	typedef __int64 IMP_U64;
	typedef __int64 IMP_S64;
#endif
typedef char IMP_CHAR;
typedef char* IMP_PCHAR;
typedef float IMP_FLOAT;
typedef double IMP_DOUBLE;
typedef void IMP_VOID;
typedef unsigned long IMP_SIZE_T;
typedef unsigned long IMP_LENGTH_T;

typedef void *IMP_MODULE_HANDLE;

typedef enum
{
	IMP_FALSE = 0,
	IMP_TRUE = 1,
} IMP_BOOL;

#ifndef NULL
#define NULL 0L
#endif
#define IMP_NULL 0L
#define IMP_NULL_PTR 0L
#define IMP_SUCCESS 0
#define IMP_FAILURE (-1)

/** IMP_EXPORTS */
#if defined(IMP_API_EXPORTS)
#define IMP_EXPORTS __declspec(dllexport)
#elif defined(IMP_API_IMPORTS)
#define IMP_EXPORTS __declspec(dllimport)
#else
#define IMP_EXPORTS extern
#endif


#define IMP_ASSERT(x) ( assert(x) )
#define RESTRICT

/** IMP_INLINE */
#ifndef IMP_INLINE
#if defined __cplusplus
#define IMP_INLINE inline
#elif (defined WIN32 || defined WIN64) && !defined __GNUC__
#define IMP_INLINE __inline
#else
#define IMP_INLINE static
#endif
#endif


/** �㷨ģ�� */
typedef enum impALGO_MODULE_E
{
	IMP_NONE_AGLO_MODULE= 0x00000000,		/**< �㷨ģ�� */
	IMP_PEA_AGLO_MODULE = 0x00000001,		/**< PEA�㷨ģ�� */
	IMP_AVD_AGLO_MODULE = 0x00000002,		/**< AVD�㷨ģ�� */
	IMP_VFD_AGLO_MODULE = 0x00000004,		/**< VFD�㷨ģ�� */
	IMP_AAI_AGLO_MODULE = 0x00000008		/**< AAI�㷨ģ�� */
}ALGO_MODULE_E;

/** �㷨ģ�� */
typedef enum impALGO_STATUS_E
{
	IMP_AGLO_STATUS_ARM 	= 0,	/**< �㷨����״̬ */
	IMP_AGLO_STATUS_DISARM 	= 1		/**< �㷨����״̬ */
}ALGO_STATUS_E;
/** �������Ϣ */
typedef struct impLIB_INFO_S
{
	IMP_U32 pu32Version;         /**< �汾�� */
	IMP_U32 pu32FuncAuthorized;  /**< ��Ȩ���� */
} LIB_INFO_S;

/** ��ṹ���� */
typedef struct impIMP_POINT_S
{
	IMP_S16 s16X;/**< x */
	IMP_S16 s16Y;/**< y */
}IMP_POINT_S;

/** ��ṹ���� */
typedef struct impIMP_POINT32F_S
{
	IMP_FLOAT f32X;/**< x */
	IMP_FLOAT f32Y;/**< y */
}IMP_POINT32F_S;

/** 3ά��ṹ���� */
typedef struct impPOINT3D_S
{
	IMP_S32 s32X;/**< x */
	IMP_S32 s32Y;/**< y */
	IMP_S32 s32Z;/**< z */
} POINT3D_S;

/** �߶νṹ���� */
typedef struct impLINE_S
{
	IMP_POINT_S stPs; /**< ��ʼ�� */
	IMP_POINT_S stPe; /**< ������ */
} LINE_S;

/** ���νṹ���� */
typedef struct impIMP_RECT_S
{
	IMP_S16 s16X1;  /**< ���Ͻ�x���� */
	IMP_S16 s16Y1;  /**< ���Ͻ�y���� */
	IMP_S16 s16X2;  /**< ���½�x���� */
	IMP_S16 s16Y2;  /**< ���½�y���� */
} IMP_RECT_S;

/** ״̬���� */
typedef enum impSTATUS_E
{
	IMP_STATUS_CHECK_LICENSE_TIMEOUT =-3, /**< ��֤licenseʧ�� */
	IMP_STATUS_CHECK_LICENSE_FAILED =-2,  /**< ��֤licenseʧ�� */
	IMP_STATUS_READ_MAC_FAILED = -1,      /**< ��ȡMACʧ�� */
	IMP_STATUS_OK = 1,                    /**< �ɹ� */
	IMP_STATUS_SKIP,                      /**< ���� */
	IMP_STATUS_FALSE,                     /**< ��֧�� */
	IMP_STATUS_UNSUPPORT
} STATUS_E;


/** ������� */
typedef void *IMP_HANDLE;

/** ģ�����ݳ��ȶ��� */
#define IMP_MODULE_DATLEN	256

/** IMPĿ�궨�� */
typedef struct impOBJ_S
{
	IMP_U8 au8Buffer[IMP_MODULE_DATLEN];  /**< BUFFER���� */
} OBJ_S;

/** �ڴ������*/
#define IMP_MEM_ITEM_CNT	4

/** �ڴ����Ͷ���*/
typedef enum impMEM_TYPE_E
{
	IMP_MEM_TYPE_IRAM = 0, /**< �ڲ�Ram */
	IMP_MEM_TYPE_ERAM	   /**< �ⲿRam */
} MEM_TYPE_E;

/** �ڴ浥Ԫ�ṹ���� */
typedef struct impMEM_ITEM_S
{
	IMP_U32 u32Size;		/**< ��С */
	IMP_U32 u32Type;		/**< ���� */
	IMP_VOID *pMem;			/**< ����ָ�� */
} MEM_ITEM_S;

/** �ڴ�ṹ����*/
typedef struct impMEM_SET_S
{
	IMP_U32 u32ImgW;                      /**< ��� */
	IMP_U32 u32ImgH;                      /**< �߶� */
	IMP_U32 u32MemNum;                    /**< ���� */
	MEM_ITEM_S stMems[IMP_MEM_ITEM_CNT];  /**< ���� */
} MEM_SET_S;

/** ���ݻ���ṹ���� */
typedef struct impDATA_BUF_S
{
	IMP_S32 s32BufLen;       /**< ���泤�� */
	IMP_S32 s32DatLen;       /**< ���ݳ��� */
	IMP_U8 *pu8Buf;          /**< ָ�� */
} DATA_BUF_S;

/** ȫ�ֲ��� */
typedef struct impGLOBAL_PARA_S
{
	IMP_S32 s32TimeUnit;	/**<  ͼ��ʱ�䵥λ */
	IMP_S32 s32FuncCode;	/**<  ������ */
} GLOBAL_PARA_S;

/** ������� */
#define IMP_PARA_RULE_BUFLEN		(1024 * 64)

/** ����������� */
typedef struct impRULE_PARA_DATA_S
{
	DATA_BUF_S stVanaData;    /**<  ���� */
} RULE_PARA_DATA_S;

/** �߼�����count */
#define IMP_PARA_ADVBUF_BUFCNT	16
/** �߼�����length */
#define IMP_PARA_ADVBUF_BUFLEN	(1024 * 8)

/** �߼��������� */
typedef struct impADVANCE_PARA_S
{
	DATA_BUF_S astAdvDats[IMP_PARA_ADVBUF_BUFCNT];  /** �߼������������� */
} ADVANCE_PARA_S;

/** �ⲿ�����ṹ���� */
typedef struct impEXTERNAL_PARA_S
{
	IMP_U32         u32Type;            /**<  ���� */
	GLOBAL_PARA_S	stGlobal;			/**<  ȫ�ֲ��� */
	RULE_PARA_DATA_S    stRule;		    /**<  �������� */
	ADVANCE_PARA_S	stAdvance;			/**< �߼���������������Ա�����ã� */
} EXTERNAL_PARA_S;

/** �ڲ������ṹ���� */
typedef struct impINNER_PARA_S
{
	IMP_VOID		*pRule;				/**<  �������� */
	GLOBAL_PARA_S	stGlobal;			/**<  ȫ�ֲ��� */
	ADVANCE_PARA_S	stAdvance;			/**<  �߼���������������Ա�����ã�*/
} INNER_PARA_S;

/** �����ַ���Ԫ�������� */
#define IMP_PARA_STRARRAY_ELEMCNT	16

/** �����ַ������� */
typedef struct impSTR_ARRAY_PARA_S
{
	IMP_S8 *pStrings[IMP_PARA_STRARRAY_ELEMCNT];  /**<  �����ַ������� */
}STR_ARRAY_PARA_S;

/** �������Ͷ��� */
typedef enum impRULE_TYPE_E
{
	IMP_RULE_SET_TYPE = 0,  /**<  �������Ͷ��� */
} RULE_TYPE_E;

/** �����鶨�� */
typedef struct impRULE_CHECK_S
{
	IMP_U32     u32Type;     /**<  ���� */
	DATA_BUF_S	stScnDat;    /**<  �������� */
	IMP_U32	    u32EnaCode;  /**<  ʹ���� */
} RULE_CHECK_S;

/** RGB���ؽṹ���� */
typedef struct impPIXEL_S
{
	IMP_U8 u8B;    /**< blueͨ�� */
	IMP_U8 u8G;    /**< greenͨ�� */
	IMP_U8 u8R;    /**< redͨ�� */
} PIXEL_S;

/** HSV���ؽṹ����*/
typedef struct impHSV_PIXEL_S
{
	IMP_FLOAT f32H;    /**< blueͨ�� */
	IMP_FLOAT f32S;    /**< greenͨ�� */
	IMP_FLOAT f32V;    /**< redͨ�� */
} HSV_PIXEL_S;

/** HSL���ؽṹ���� */
typedef struct impHSL_PIXEL_S
{
	IMP_FLOAT f32H;    /**< hueͨ�� */
	IMP_FLOAT f32S;    /**< saturationͨ�� */
	IMP_FLOAT f32L;    /**< luminanceͨ�� */
} HSL_PIXEL_S;

/** RGBͼ��ṹ���� */
typedef struct impRGB_IMAGE_S
{
	IMP_S32 s32W;      /**< ͼ���� */
	IMP_S32 s32H;      /**< ͼ��߶� */
	IMP_U8 *pu8Data;    /**< ͼ������ָ�� */
	IMP_U32 u32Time;   /**< ʱ�� */
} RGB_IMAGE_S;


/** HSVͼ��ṹ���� */
typedef struct impHSV_IMAGE_S
{
	IMP_S32   s32W;           /**< ͼ���� */
	IMP_S32   s32H;           /**< ͼ��߶� */
	IMP_FLOAT *pf32Data;      /**< ͼ������ָ�� */
	IMP_U32   u32Time;        /**< ʱ�� */
} HSV_IMAGE_S;

/** �Ҷ�ͼ��ṹ���� */
typedef struct impGRAY_IMAGE_S
{
	IMP_S32 s32W;           /**< ͼ���� */
	IMP_S32 s32H;           /**< ͼ��߶� */
	IMP_U8 *pu8Data;        /**< ͼ������ָ�� */
} GRAY_IMAGE_S;

/** 16λ�Ҷ�ͼ��ṹ���� */
typedef struct impGRAY_IMAGE16_S
{
	IMP_S32 s32W;           /**< ͼ���� */
	IMP_S32 s32H;           /**< ͼ��߶� */
	IMP_S16 *ps16Data;      /**< ͼ������ָ�� */
} GRAY_IMAGE16_S;

/** 32λ�Ҷ�ͼ��ṹ���� */
typedef struct impGRAY_IMAGE32_S
{
	IMP_S32 s32W;           /**< ͼ���� */
	IMP_S32 s32H;           /**< ͼ��߶� */
	IMP_S32 *ps32Data;      /**< ͼ������ָ�� */
} GRAY_IMAGE32_S;

/** TMPLͼ��ṹ���� */
typedef struct impTMPL_IMAGE_S
{
	IMP_S32 s32W;           /**< ͼ���� */
	IMP_S32 s32H;           /**< ͼ��߶� */
	IMP_S32 s32BufLen;      /**< ͼ���ڴ泤�� */
	IMP_U8 *pu8Data;        /**< ͼ����ͼ������ָ�� */
} TMPL_IMAGE_S;

/** YUV422ͼ���������� */
typedef struct impYUV_IMAGE422_S
{
	IMP_S32 s32W;           /**< ͼ����*/
	IMP_S32 s32H;           /**< ͼ��߶�*/
	IMP_U8 *pu8Y;           /**< ͼ��Y��������ָ��*/
	IMP_U8 *pu8U;           /**< ͼ��U��������ָ��*/
	IMP_U8 *pu8V;           /**< ͼ��V��������ָ��*/
	IMP_U32 u32Time;        /**< ʱ��*/
	IMP_U32 u32Flag;        /**< ��־*/
} YUV_IMAGE422_S;

/** YUV420ͼ���������� */
typedef struct impYUV_IMAGE420_S
{
	IMP_S32 s32W;           /**< ͼ����*/
	IMP_S32 s32H;           /**< ͼ��߶�*/
	IMP_U8 *pu8Y;           /**< ͼ��Y��������ָ��*/
	IMP_U8 *pu8U;           /**< ͼ��U��������ָ��*/
	IMP_U8 *pu8V;           /**< ͼ��V��������ָ��*/
	IMP_U32 u32Time;        /**< ʱ��*/
	IMP_U32 u32Flag;        /**< ��־*/
} YUV_IMAGE420_S;


typedef enum impIMAGE_FORMAT_E

{
         IMAGE_FORMAT_IMP_YUV422 = 0,
         IMAGE_FORMAT_IMP_YUV420
 } IMAGE_FORMAT_E;



typedef struct impIMAGE3_S

{

         IMP_S32 s32W;           /**< ͼ����*/
         IMP_S32 s32H;           /**< ͼ��߶�*/
         IMP_U8 *pu8D1;           /**< ͼ�����1 ����ָ��*/
         IMP_U8 *pu8D2;           /**< ͼ�����2 ����ָ��*/
         IMP_U8 *pu8D3;           /**< ͼ�����3 ����ָ��*/
         IMP_U32 u32Time;        /**< ʱ��*/
         IMP_U32 u32Flag;        /**< ��־*/
         IMAGE_FORMAT_E enFormat;        /**< ��ʽ*/
} IMAGE3_S;


/** �¼����� */
typedef enum impEVT_TYPE_TOP_E
{
	IMP_EVT_TYPE_UNKNOWN        = 0x00000000,		/**< δ֪���� */
	IMP_EVT_TYPE_Armed          = 0x00002000,		/**< �������� */
	IMP_EVT_TYPE_Disarmed       = 0x00002001,		/**< ������ */
	IMP_EVT_TYPE_AlarmCleared   = 0x00002002,		/**< ������� */
	IMP_EVT_TYPE_SignalLoss     = 0x00003000		/**< ��Ƶ�źŶ�ʧ */
}EVT_TYPE_TOP_E;

/** �¼�״̬ */
typedef enum impEVT_STATUS_E
{
	IMP_EVT_STATUS_NOSTA         = 0,				/**< ��״̬ */
	IMP_EVT_STATUS_START         = 1,				/**< ��ʼ */
	IMP_EVT_STATUS_END           = 2,				/**< ���� */
	IMP_EVT_STATUS_PROCEDURE     = 3				/**< ������ */
} EVT_STATUS_E;

/** ��Ƶ���������� */
typedef enum impFUNC_ANALYSIS_E
{
	IMP_FUNC_ATM_ABANDUM    = 0x00000001,	/**< ATM������ */
	IMP_FUNC_ABANDUM        = 0x00000002,	/**< �������� */
	IMP_FUNC_OBJSTOLEN      = 0x00000004,	/**< �������� */
	IMP_FUNC_NOPARKING      = 0x00000008,	/**< �Ƿ�ͣ����� */
	IMP_FUNC_PERIMETER      = 0x00000010,	/**< �ܽ籣�� */
	IMP_FUNC_TRIPWIRE       = 0x00000020,	/**< ���� */
	IMP_FUNC_MTRIPWIRE      = 0x00000040,	/**< ����� */
	IMP_FUNC_ABNMLVEL       = 0x00000080,	/**< �������ٶ� */
	IMP_FUNC_LOITER         = 0x00000100,	/**< �ǻ� */
}FUNC_ANALYSIS_E;

/** Ŀ��ID��Сֵ */
#define IMP_TGT_ID_MIN		0x00000000
/** Ŀ��ID���ֵ */
#define IMP_TGT_ID_MAX		0x1FFFFFFF

/** Ŀ������ */
typedef enum impTGT_TYPE_E
{
	IMP_TGT_TYPE_HUMAN          = 0x00000001,	/**< �� */
	IMP_TGT_TYPE_UNKNOWN        = 0x00000002,	/**< δ֪ */
	IMP_TGT_TYPE_VEHICLE        = 0x00000003,	/**< �� */
	IMP_TGT_TYPE_ANIMAL         = 0x00000004	/**< ���� */
}TGT_TYPE_E;

/** Ŀ���¼� */
typedef enum impTGT_EVENT_E
{
	IMP_TGT_EVENT_UNKNOWN       = 0x00000000,	/**< δ֪�¼�*/
	IMP_TGT_EVENT_ATM_ABANDUM   = 0x00000001,	/**< ATM������ */
	IMP_TGT_EVENT_ABANDUM       = 0x00000002,	/**< �������� */
	IMP_TGT_EVENT_OBJSTOLEN     = 0x00000004,	/**< �������� */
	IMP_TGT_EVENT_NOPARKING     = 0x00000008,	/**< �Ƿ�ͣ����� */
	IMP_TGT_EVENT_PERIMETER     = 0x00000010,	/**< �ܽ籣�� */
	IMP_TGT_EVENT_TRIPWIRE      = 0x00000020,	/**< ���� */
	IMP_TGT_EVENT_MTRIPWIRE     = 0x00000040,	/**< ����� */
	IMP_TGT_EVENT_ABNORMALVEL   = 0x00000800,	/**< �������ٶ� */
	IMP_TGT_EVENT_LOITER        = 0x00000100,	/**< �ǻ� */
}TGT_EVENT_E;

/** Ŀ��״̬ */
typedef enum impTGT_STATUS_E
{
	IMP_TGT_STATUS_UNKNOWN		= 0x00000000,	/**< δ֪״̬ */
	IMP_TGT_STATUS_MEASURED		= 0x00000010,	/**< Ŀ��λ���ɲ����õ� */
	IMP_TGT_STATUS_PREDICTED	= 0x00000020,	/**< Ŀ��λ����Ԥ��õ� */
	IMP_TGT_STATUS_MOTION		= 0x00000100,	/**< Ŀ������˶����� */
	IMP_TGT_STATUS_STATIC		= 0x00000200	/**< Ŀ�굱ǰ�Ǿ�ֹ�� */
}TGT_STATUS_E;

/** Ŀ��켣���� */
#define IMP_MAX_TRAJECT_LEN	40

/** Ŀ��켣�ṹ���� */
typedef struct impTGT_TRAJECT_S
{
	IMP_S32 s32Length;			          	    /**< �켣���� */
	IMP_POINT_S	stPoints[IMP_MAX_TRAJECT_LEN];	/**< �켣������ */
} TGT_TRAJECT_S;

/** Ŀ������ */
typedef struct impTGT_MOTION_ITEM_S
{
	IMP_S32	  	  s32Velocity;				/**< ���ʣ�pix/s��*/
	IMP_S32		  s32Direction;				/**< ����0~359�ȣ�*/
	TGT_TRAJECT_S stTraject;				/**< �켣*/
} TGT_MOTION_ITEM_S;

/** Ŀ�����ݻ��峤�� */
#define IMP_BUFLEN_TGT_ITEM	256

/** Ŀ������ */
#define IMP_BUFLEN_TGT_DATA	(IMP_BUFLEN_TGT_ITEM - (sizeof(IMP_U32) * 4 + sizeof(IMP_POINT_S) + sizeof(IMP_RECT_S)))

/** Ŀ�����ݽṹ */
typedef struct impTGT_ITEM_S
{
	IMP_U32	u32Id;							/**< ID */
	IMP_U32	u32Status;						/**< ״̬ */
	IMP_U32	u32Type;						/**< ���� */
	IMP_U32	u32Event;						/**< �¼� */
	IMP_POINT_S	stPoint;					/**< λ�� */
	IMP_RECT_S	stRect;						/**< ���� */
	IMP_U8	au8Data[IMP_BUFLEN_TGT_DATA];   /**< ���� */
} TGT_ITEM_S;

/** Ŀ��������� */
#define IMP_MAX_TGT_CNT			64

/** Ŀ�꼯�� */
typedef struct impTGT_SET_S
{
	IMP_S32	    s32TargetNum;					 /**< Ŀ����Ŀ */
	TGT_ITEM_S	astTargets[IMP_MAX_TGT_CNT];     /**< Ŀ������ */
} TGT_SET_S;



/** �¼������� */
#define IMP_MAX_EVT_CNT	128

/** �¼������ */
#define IMP_BUFLEN_EVT_ITEM	64

/** �¼����� */
#define IMP_BUFLEN_EVT_DATA	(IMP_BUFLEN_EVT_ITEM - sizeof(IMP_U32) * 5)

/** �¼����Ͷ��� */
typedef enum impEVT_TYPE_E
{
	IMP_EVT_TYPE_SignalBad          = 0x00000001,		/**< ��Ƶ�ź��쳣��� */
	IMP_EVT_TYPE_AlarmAtmAbabdum    = 0x00000101,		/**< ATM������ */
	IMP_EVT_TYPE_AlarmAbabdum       = 0x00000102,		/**< �������� */
	IMP_EVT_TYPE_AlarmObjStolen     = 0x00000103,		/**< �������� */
	IMP_EVT_TYPE_AlarmNoParking     = 0x00000104,		/**< �Ƿ�ͣ����� */
	IMP_EVT_TYPE_AlarmPerimeter     = 0x00000105,		/**< ���������� */
	IMP_EVT_TYPE_AlarmTripwire      = 0x00000106,		/**< �������߼�� */
	IMP_EVT_TYPE_AlarmMTripwire     = 0x00000107,		/**< ˫�����߼�� */
	IMP_EVT_TYPE_AlarmAbnmlvel      = 0x00000108,		/**< �������ٶȼ��	*/
	IMP_EVT_TYPE_AlarmLoiter        = 0x00000109,		/**< �ǻ���� */

	IMP_EVT_TYPE_AVD_AlarmBase		= 0x00000200,		/**< AVD�㷨�澯 */
	IMP_EVT_TYPE_AlarmSceneChg    	= 0x00000201,		/**< �����任 */
	IMP_EVT_TYPE_AlarmNoSignal      = 0x00000202,		/**< �ź�ȱʧ */
	IMP_EVT_TYPE_AlarmBrightAbnml   = 0x00000203,		/**< �����쳣 */
	IMP_EVT_TYPE_AlarmClarityAbnml  = 0x00000204,		/**< ��Ƶģ�� */
	IMP_EVT_TYPE_AlarmColorAbnml    = 0x00000205,		/**< ��Ƶƫɫ */
	IMP_EVT_TYPE_AlarmNoise      	= 0x00000206,		/**< �������� */
	IMP_EVT_TYPE_AlarmPtzLoseCrl    = 0x00000207,		/**< PTZʧ��  */
	IMP_EVT_TYPE_AlarmFreeze      	= 0x00000208,		/**< ���涳�� */
	IMP_EVT_TYPE_AlarmInterfere		= 0x00000209,		/**< ��Ϊ���� */
	IMP_EVT_TYPE_AlarmBrightHigh    = 0x0000020a,		/**< ���ȸ� */
	IMP_EVT_TYPE_AlarmBrightLow     = 0x0000020b,		/**< ���ȵ� */

	IMP_EVT_TYPE_VFD_AlarmBase		= 0x00000300,		/**< VFD�㷨�澯 */
	IMP_EVT_TYPE_AlarmFaceCapture   = 0x00000301,		/**< ����ץ�� */
	IMP_EVT_TYPE_AlarmFaceCamouflage= 0x00000302,		/**< ����αװ */
	IMP_EVT_TYPE_AlarmPasswordPeep  = 0x00000303,		/**< ����͵�� */
}EVT_TYPE_E;

/** ��Ƶ�쳣���� */
enum
{
	VETYPE_CameraMoved = 0,					/**< ����ƶ����ڵ� */
	VETYPE_SignalBad						/**< ��Ƶ������ */
};

/** �쳣���ͽṹ */
typedef struct impEVT_DATA_EXCEPTION_S
{
	IMP_U32	u32Type;						/**< �쳣���� */
} EVT_DATA_EXCEPTION_S;

/** ATM�¼����ݽṹ */
typedef struct impEVT_DATA_ATM_ABANDUM_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ */
	IMP_RECT_S	stRect;					/**< �¼�λ�� */
} EVT_DATA_ATM_ABANDUM_S;

/** �������¼����ݽṹ */
typedef struct impEVT_DATA_ABANDUM_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ */
	IMP_RECT_S	stRect;					/**< �¼�λ�� */
} EVT_DATA_ABANDUM_S;

/** ���������¼����ݽṹ */
typedef struct impEVT_DATA_OBJSTOLEN_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ */
	IMP_RECT_S	stRect;					/**< �¼�λ�� */
} EVT_DATA_OBJSTOLEN_S;

/** Ŀ�����¼����ݽṹ */
typedef struct impEVT_DATA_TARGET_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ */
	IMP_RECT_S	stRect;					/**< �¼�λ�� */
} EVT_DATA_TARGET_S;

/** �ܽ����¼����ݽṹ */
typedef struct impEVT_DATA_PERIMETER_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ */
	IMP_RECT_S  stRect;					/**< �¼�λ�� */
	struct
	{									/**< ���� */
		IMP_U32	u32Mode;				/**< ������ģʽ */
	} stRule;
} EVT_DATA_PERIMETER_S;

/** ���߼���¼����ݽṹ */
typedef struct impEVT_DATA_TRIPWIRE_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ */
	IMP_RECT_S	stRect;					/**< �¼�λ�� */
	IMP_U8  u8LineIndex;                /**< ���ߺ� */
	struct
	{
		IMP_S32		s32Bidirection;		/**< ��ʾ�ð����Ƿ�Ϊ˫�����(0: ��, 1: ��) */
		IMP_S32		s32AbnmlAngle;		/**< ���߽�ֹ����Ƕ�(unit: degree) */
		LINE_S		stLine;				/**< ����λ��(unit: pixel) */
	} stRule;							/**< ���� */
} EVT_DATA_TRIPWIRE_S;

/** ����߼���¼����ݽṹ */
typedef struct impEVT_DATA_MTRIPWIRE_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ */
	IMP_RECT_S	stRect;					/**< �¼�λ�� */
	IMP_U8  u8LineIndex;                /**< ���ߺ� */
	struct
	{
		IMP_S32		as32AbnmlAngle[2];	/**< ���߽�ֹ����Ƕ�(unit: degree) */
		LINE_S		astLine[2];			/**< ����λ��(unit: pixel) */
	} stRule;							/**< ���� */
} EVT_DATA_MTRIPWIRE_S;

/** �쳣�ٶȼ���¼����ݽṹ */
typedef struct impEVT_DATA_ABNMLVEL_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ*/
	IMP_RECT_S	stRect;					/**< �¼�λ��*/
} EVT_DATA_ABNMLVEL_S;

/** �ǻ��ٶȼ���¼����ݽṹ */
typedef struct impEVT_DATA_LOITER_S
{
	IMP_U32	u32TId;						/**< Ŀ���ʶ*/
	IMP_RECT_S	stRect;					/**< �¼�λ��*/
} EVT_DATA_LOITER_S;

/** �¼��� */
typedef struct impEVT_ITEM_S
{
	IMP_U32	u32Type;						/**< �¼����� */
	IMP_U32	u32Id;							/**< �¼���ʶ */
	IMP_U32	u32Level;						/**< �¼��������� */
	IMP_U32	u32Status;						/**< �¼�״̬ */
	IMP_U32	u32Zone;						/**< �¼��������� */
	IMP_U8	au8Data[IMP_BUFLEN_EVT_DATA];   /**< �¼����� */
} EVT_ITEM_S;

/** �¼��� */
typedef struct impEVT_SET_S
{
	IMP_S32	    s32EventNum;					/**< �¼���Ŀ */
	EVT_ITEM_S	astEvents[IMP_MAX_EVT_CNT];		/**< �¼����� */
} EVT_SET_S;

/** PEA ������ */
typedef struct impRESULT_S
{
	TGT_SET_S	stTargetSet;		/**< Ŀ�� */
	EVT_SET_S	stEventSet;		    /**< �¼� */
} RESULT_S;




/** ��������� */
#define IMP_MAX_FACE_CNT 50
/** �������� */
typedef struct impVFD_FACE_ITEM_S
{
	IMP_S32 rightEyeX;

	IMP_S32 rightEyeY;

	IMP_S32 leftEyeX;

	IMP_S32 leftEyeY;

	IMP_S32 centerX;

	IMP_S32 centerY;

    IMP_S32 faceSize;

	IMP_S32 angleInPlane;

	IMP_S32 angleOutPlane;

	IMP_S32 faceScore;

	IMP_S32 serialNumber;

	IMP_S32 shapeScore;

//	IMP_RECT_S rect;

} FACE_ITEM_S;

/** �������ݼ��� */
typedef struct impVFD_FACE_SET_S
{
    IMP_S32	    s32FaceNum;
	FACE_ITEM_S	astFaces[IMP_MAX_FACE_CNT];
}VFD_FACE_SET_S;

/** VFD ������ */
typedef struct impVFD_RESULT_S
{
    VFD_FACE_SET_S stFaceSet;
    EVT_SET_S	stEventSet;
}VFD_RESULT_S;

/** �������ض��� */
typedef struct impOPTICAL_FLOW_PIXEL_S
{
	IMP_FLOAT fVx;   /** ��������X */
	IMP_FLOAT fVy;   /** ��������Y */
}OPTICAL_FLOW_PIXEL_S;

/** ���� */
typedef struct impOPTICAL_FLOW_S
{
	IMP_S32 s32X1;
	IMP_S32 s32Y1;
	IMP_S32 s32Value1;
	IMP_S32 s32X2;
	IMP_S32 s32Y2;
	IMP_S32 s32Value2;

}OPTICAL_FLOW_S;

#ifdef __cplusplus
}
#endif

#endif /*_IMP_ALGO_TYPE_H_*/

/*@}*/
/*@}*/
