/**
* \defgroup 接口
* 接口
* @author 北京银瀑技术
* @version 2.0
* @data 2009-2010
*/
/*@{*/

/**
* \defgroup 结构类型定义
* 算法数据类型定义
* @author 北京银瀑技术
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


/** 算法模块 */
typedef enum impALGO_MODULE_E
{
	IMP_NONE_AGLO_MODULE= 0x00000000,		/**< 算法模块 */
	IMP_PEA_AGLO_MODULE = 0x00000001,		/**< PEA算法模块 */
	IMP_AVD_AGLO_MODULE = 0x00000002,		/**< AVD算法模块 */
	IMP_VFD_AGLO_MODULE = 0x00000004,		/**< VFD算法模块 */
	IMP_AAI_AGLO_MODULE = 0x00000008		/**< AAI算法模块 */
}ALGO_MODULE_E;

/** 算法模块 */
typedef enum impALGO_STATUS_E
{
	IMP_AGLO_STATUS_ARM 	= 0,	/**< 算法布防状态 */
	IMP_AGLO_STATUS_DISARM 	= 1		/**< 算法撤防状态 */
}ALGO_STATUS_E;
/** 库相关信息 */
typedef struct impLIB_INFO_S
{
	IMP_U32 pu32Version;         /**< 版本号 */
	IMP_U32 pu32FuncAuthorized;  /**< 授权功能 */
} LIB_INFO_S;

/** 点结构定义 */
typedef struct impIMP_POINT_S
{
	IMP_S16 s16X;/**< x */
	IMP_S16 s16Y;/**< y */
}IMP_POINT_S;

/** 点结构定义 */
typedef struct impIMP_POINT32F_S
{
	IMP_FLOAT f32X;/**< x */
	IMP_FLOAT f32Y;/**< y */
}IMP_POINT32F_S;

/** 3维点结构定义 */
typedef struct impPOINT3D_S
{
	IMP_S32 s32X;/**< x */
	IMP_S32 s32Y;/**< y */
	IMP_S32 s32Z;/**< z */
} POINT3D_S;

/** 线段结构定义 */
typedef struct impLINE_S
{
	IMP_POINT_S stPs; /**< 起始点 */
	IMP_POINT_S stPe; /**< 结束点 */
} LINE_S;

/** 矩形结构定义 */
typedef struct impIMP_RECT_S
{
	IMP_S16 s16X1;  /**< 左上角x坐标 */
	IMP_S16 s16Y1;  /**< 左上角y坐标 */
	IMP_S16 s16X2;  /**< 右下角x坐标 */
	IMP_S16 s16Y2;  /**< 右下角y坐标 */
} IMP_RECT_S;

/** 状态定义 */
typedef enum impSTATUS_E
{
	IMP_STATUS_CHECK_LICENSE_TIMEOUT =-3, /**< 验证license失败 */
	IMP_STATUS_CHECK_LICENSE_FAILED =-2,  /**< 验证license失败 */
	IMP_STATUS_READ_MAC_FAILED = -1,      /**< 读取MAC失败 */
	IMP_STATUS_OK = 1,                    /**< 成功 */
	IMP_STATUS_SKIP,                      /**< 跳过 */
	IMP_STATUS_FALSE,                     /**< 不支持 */
	IMP_STATUS_UNSUPPORT
} STATUS_E;


/** 句柄定义 */
typedef void *IMP_HANDLE;

/** 模块数据长度定义 */
#define IMP_MODULE_DATLEN	256

/** IMP目标定义 */
typedef struct impOBJ_S
{
	IMP_U8 au8Buffer[IMP_MODULE_DATLEN];  /**< BUFFER定义 */
} OBJ_S;

/** 内存管理定义*/
#define IMP_MEM_ITEM_CNT	4

/** 内存类型定义*/
typedef enum impMEM_TYPE_E
{
	IMP_MEM_TYPE_IRAM = 0, /**< 内部Ram */
	IMP_MEM_TYPE_ERAM	   /**< 外部Ram */
} MEM_TYPE_E;

/** 内存单元结构定义 */
typedef struct impMEM_ITEM_S
{
	IMP_U32 u32Size;		/**< 大小 */
	IMP_U32 u32Type;		/**< 类型 */
	IMP_VOID *pMem;			/**< 数据指针 */
} MEM_ITEM_S;

/** 内存结构定义*/
typedef struct impMEM_SET_S
{
	IMP_U32 u32ImgW;                      /**< 宽度 */
	IMP_U32 u32ImgH;                      /**< 高度 */
	IMP_U32 u32MemNum;                    /**< 数量 */
	MEM_ITEM_S stMems[IMP_MEM_ITEM_CNT];  /**< 数组 */
} MEM_SET_S;

/** 数据缓存结构定义 */
typedef struct impDATA_BUF_S
{
	IMP_S32 s32BufLen;       /**< 缓存长度 */
	IMP_S32 s32DatLen;       /**< 数据长度 */
	IMP_U8 *pu8Buf;          /**< 指针 */
} DATA_BUF_S;

/** 全局参数 */
typedef struct impGLOBAL_PARA_S
{
	IMP_S32 s32TimeUnit;	/**<  图像时间单位 */
	IMP_S32 s32FuncCode;	/**<  功能码 */
} GLOBAL_PARA_S;

/** 规则参数 */
#define IMP_PARA_RULE_BUFLEN		(1024 * 64)

/** 规则参数数据 */
typedef struct impRULE_PARA_DATA_S
{
	DATA_BUF_S stVanaData;    /**<  数据 */
} RULE_PARA_DATA_S;

/** 高级参数count */
#define IMP_PARA_ADVBUF_BUFCNT	16
/** 高级参数length */
#define IMP_PARA_ADVBUF_BUFLEN	(1024 * 8)

/** 高级参数数据 */
typedef struct impADVANCE_PARA_S
{
	DATA_BUF_S astAdvDats[IMP_PARA_ADVBUF_BUFCNT];  /** 高级参数数据数组 */
} ADVANCE_PARA_S;

/** 外部参数结构定义 */
typedef struct impEXTERNAL_PARA_S
{
	IMP_U32         u32Type;            /**<  类型 */
	GLOBAL_PARA_S	stGlobal;			/**<  全局参数 */
	RULE_PARA_DATA_S    stRule;		    /**<  场景数据 */
	ADVANCE_PARA_S	stAdvance;			/**< 高级参数（仅开发人员可设置） */
} EXTERNAL_PARA_S;

/** 内部参数结构定义 */
typedef struct impINNER_PARA_S
{
	IMP_VOID		*pRule;				/**<  场景参数 */
	GLOBAL_PARA_S	stGlobal;			/**<  全局参数 */
	ADVANCE_PARA_S	stAdvance;			/**<  高级参数（仅开发人员可设置）*/
} INNER_PARA_S;

/** 参数字符串元素最大个数 */
#define IMP_PARA_STRARRAY_ELEMCNT	16

/** 参数字符串数据 */
typedef struct impSTR_ARRAY_PARA_S
{
	IMP_S8 *pStrings[IMP_PARA_STRARRAY_ELEMCNT];  /**<  参数字符串数组 */
}STR_ARRAY_PARA_S;

/** 规则类型定义 */
typedef enum impRULE_TYPE_E
{
	IMP_RULE_SET_TYPE = 0,  /**<  规则集类型定义 */
} RULE_TYPE_E;

/** 规则检查定义 */
typedef struct impRULE_CHECK_S
{
	IMP_U32     u32Type;     /**<  类型 */
	DATA_BUF_S	stScnDat;    /**<  场景数据 */
	IMP_U32	    u32EnaCode;  /**<  使能码 */
} RULE_CHECK_S;

/** RGB像素结构定义 */
typedef struct impPIXEL_S
{
	IMP_U8 u8B;    /**< blue通道 */
	IMP_U8 u8G;    /**< green通道 */
	IMP_U8 u8R;    /**< red通道 */
} PIXEL_S;

/** HSV像素结构定义*/
typedef struct impHSV_PIXEL_S
{
	IMP_FLOAT f32H;    /**< blue通道 */
	IMP_FLOAT f32S;    /**< green通道 */
	IMP_FLOAT f32V;    /**< red通道 */
} HSV_PIXEL_S;

/** HSL像素结构定义 */
typedef struct impHSL_PIXEL_S
{
	IMP_FLOAT f32H;    /**< hue通道 */
	IMP_FLOAT f32S;    /**< saturation通道 */
	IMP_FLOAT f32L;    /**< luminance通道 */
} HSL_PIXEL_S;

/** RGB图像结构定义 */
typedef struct impRGB_IMAGE_S
{
	IMP_S32 s32W;      /**< 图像宽度 */
	IMP_S32 s32H;      /**< 图像高度 */
	IMP_U8 *pu8Data;    /**< 图像数据指针 */
	IMP_U32 u32Time;   /**< 时间 */
} RGB_IMAGE_S;


/** HSV图像结构定义 */
typedef struct impHSV_IMAGE_S
{
	IMP_S32   s32W;           /**< 图像宽度 */
	IMP_S32   s32H;           /**< 图像高度 */
	IMP_FLOAT *pf32Data;      /**< 图像数据指针 */
	IMP_U32   u32Time;        /**< 时间 */
} HSV_IMAGE_S;

/** 灰度图像结构定义 */
typedef struct impGRAY_IMAGE_S
{
	IMP_S32 s32W;           /**< 图像宽度 */
	IMP_S32 s32H;           /**< 图像高度 */
	IMP_U8 *pu8Data;        /**< 图像数据指针 */
} GRAY_IMAGE_S;

/** 16位灰度图像结构定义 */
typedef struct impGRAY_IMAGE16_S
{
	IMP_S32 s32W;           /**< 图像宽度 */
	IMP_S32 s32H;           /**< 图像高度 */
	IMP_S16 *ps16Data;      /**< 图像数据指针 */
} GRAY_IMAGE16_S;

/** 32位灰度图像结构定义 */
typedef struct impGRAY_IMAGE32_S
{
	IMP_S32 s32W;           /**< 图像宽度 */
	IMP_S32 s32H;           /**< 图像高度 */
	IMP_S32 *ps32Data;      /**< 图像数据指针 */
} GRAY_IMAGE32_S;

/** TMPL图像结构定义 */
typedef struct impTMPL_IMAGE_S
{
	IMP_S32 s32W;           /**< 图像宽度 */
	IMP_S32 s32H;           /**< 图像高度 */
	IMP_S32 s32BufLen;      /**< 图像内存长度 */
	IMP_U8 *pu8Data;        /**< 图像宽度图像数据指针 */
} TMPL_IMAGE_S;

/** YUV422图像数据类型 */
typedef struct impYUV_IMAGE422_S
{
	IMP_S32 s32W;           /**< 图像宽度*/
	IMP_S32 s32H;           /**< 图像高度*/
	IMP_U8 *pu8Y;           /**< 图像Y分量数据指针*/
	IMP_U8 *pu8U;           /**< 图像U分量数据指针*/
	IMP_U8 *pu8V;           /**< 图像V分量数据指针*/
	IMP_U32 u32Time;        /**< 时间*/
	IMP_U32 u32Flag;        /**< 标志*/
} YUV_IMAGE422_S;

/** YUV420图像数据类型 */
typedef struct impYUV_IMAGE420_S
{
	IMP_S32 s32W;           /**< 图像宽度*/
	IMP_S32 s32H;           /**< 图像高度*/
	IMP_U8 *pu8Y;           /**< 图像Y分量数据指针*/
	IMP_U8 *pu8U;           /**< 图像U分量数据指针*/
	IMP_U8 *pu8V;           /**< 图像V分量数据指针*/
	IMP_U32 u32Time;        /**< 时间*/
	IMP_U32 u32Flag;        /**< 标志*/
} YUV_IMAGE420_S;


typedef enum impIMAGE_FORMAT_E

{
         IMAGE_FORMAT_IMP_YUV422 = 0,
         IMAGE_FORMAT_IMP_YUV420
 } IMAGE_FORMAT_E;



typedef struct impIMAGE3_S

{

         IMP_S32 s32W;           /**< 图像宽度*/
         IMP_S32 s32H;           /**< 图像高度*/
         IMP_U8 *pu8D1;           /**< 图像分量1 数据指针*/
         IMP_U8 *pu8D2;           /**< 图像分量2 数据指针*/
         IMP_U8 *pu8D3;           /**< 图像分量3 数据指针*/
         IMP_U32 u32Time;        /**< 时间*/
         IMP_U32 u32Flag;        /**< 标志*/
         IMAGE_FORMAT_E enFormat;        /**< 格式*/
} IMAGE3_S;


/** 事件类型 */
typedef enum impEVT_TYPE_TOP_E
{
	IMP_EVT_TYPE_UNKNOWN        = 0x00000000,		/**< 未知类型 */
	IMP_EVT_TYPE_Armed          = 0x00002000,		/**< 警戒启动 */
	IMP_EVT_TYPE_Disarmed       = 0x00002001,		/**< 警戒解除 */
	IMP_EVT_TYPE_AlarmCleared   = 0x00002002,		/**< 警报清除 */
	IMP_EVT_TYPE_SignalLoss     = 0x00003000		/**< 视频信号丢失 */
}EVT_TYPE_TOP_E;

/** 事件状态 */
typedef enum impEVT_STATUS_E
{
	IMP_EVT_STATUS_NOSTA         = 0,				/**< 无状态 */
	IMP_EVT_STATUS_START         = 1,				/**< 开始 */
	IMP_EVT_STATUS_END           = 2,				/**< 结束 */
	IMP_EVT_STATUS_PROCEDURE     = 3				/**< 过程中 */
} EVT_STATUS_E;

/** 视频分析功能码 */
typedef enum impFUNC_ANALYSIS_E
{
	IMP_FUNC_ATM_ABANDUM    = 0x00000001,	/**< ATM异物检测 */
	IMP_FUNC_ABANDUM        = 0x00000002,	/**< 遗弃物检测 */
	IMP_FUNC_OBJSTOLEN      = 0x00000004,	/**< 被盗物检测 */
	IMP_FUNC_NOPARKING      = 0x00000008,	/**< 非法停车检测 */
	IMP_FUNC_PERIMETER      = 0x00000010,	/**< 周界保护 */
	IMP_FUNC_TRIPWIRE       = 0x00000020,	/**< 绊线 */
	IMP_FUNC_MTRIPWIRE      = 0x00000040,	/**< 多绊线 */
	IMP_FUNC_ABNMLVEL       = 0x00000080,	/**< 非正常速度 */
	IMP_FUNC_LOITER         = 0x00000100,	/**< 徘徊 */
}FUNC_ANALYSIS_E;

/** 目标ID最小值 */
#define IMP_TGT_ID_MIN		0x00000000
/** 目标ID最大值 */
#define IMP_TGT_ID_MAX		0x1FFFFFFF

/** 目标类型 */
typedef enum impTGT_TYPE_E
{
	IMP_TGT_TYPE_HUMAN          = 0x00000001,	/**< 人 */
	IMP_TGT_TYPE_UNKNOWN        = 0x00000002,	/**< 未知 */
	IMP_TGT_TYPE_VEHICLE        = 0x00000003,	/**< 车 */
	IMP_TGT_TYPE_ANIMAL         = 0x00000004	/**< 动物 */
}TGT_TYPE_E;

/** 目标事件 */
typedef enum impTGT_EVENT_E
{
	IMP_TGT_EVENT_UNKNOWN       = 0x00000000,	/**< 未知事件*/
	IMP_TGT_EVENT_ATM_ABANDUM   = 0x00000001,	/**< ATM异物检测 */
	IMP_TGT_EVENT_ABANDUM       = 0x00000002,	/**< 遗弃物检测 */
	IMP_TGT_EVENT_OBJSTOLEN     = 0x00000004,	/**< 被盗物检测 */
	IMP_TGT_EVENT_NOPARKING     = 0x00000008,	/**< 非法停车检测 */
	IMP_TGT_EVENT_PERIMETER     = 0x00000010,	/**< 周界保护 */
	IMP_TGT_EVENT_TRIPWIRE      = 0x00000020,	/**< 绊线 */
	IMP_TGT_EVENT_MTRIPWIRE     = 0x00000040,	/**< 多绊线 */
	IMP_TGT_EVENT_ABNORMALVEL   = 0x00000800,	/**< 非正常速度 */
	IMP_TGT_EVENT_LOITER        = 0x00000100,	/**< 徘徊 */
}TGT_EVENT_E;

/** 目标状态 */
typedef enum impTGT_STATUS_E
{
	IMP_TGT_STATUS_UNKNOWN		= 0x00000000,	/**< 未知状态 */
	IMP_TGT_STATUS_MEASURED		= 0x00000010,	/**< 目标位置由测量得到 */
	IMP_TGT_STATUS_PREDICTED	= 0x00000020,	/**< 目标位置由预测得到 */
	IMP_TGT_STATUS_MOTION		= 0x00000100,	/**< 目标具有运动属性 */
	IMP_TGT_STATUS_STATIC		= 0x00000200	/**< 目标当前是静止的 */
}TGT_STATUS_E;

/** 目标轨迹长度 */
#define IMP_MAX_TRAJECT_LEN	40

/** 目标轨迹结构定义 */
typedef struct impTGT_TRAJECT_S
{
	IMP_S32 s32Length;			          	    /**< 轨迹长度 */
	IMP_POINT_S	stPoints[IMP_MAX_TRAJECT_LEN];	/**< 轨迹点数组 */
} TGT_TRAJECT_S;

/** 目标数据 */
typedef struct impTGT_MOTION_ITEM_S
{
	IMP_S32	  	  s32Velocity;				/**< 速率（pix/s）*/
	IMP_S32		  s32Direction;				/**< 方向（0~359度）*/
	TGT_TRAJECT_S stTraject;				/**< 轨迹*/
} TGT_MOTION_ITEM_S;

/** 目标数据缓冲长度 */
#define IMP_BUFLEN_TGT_ITEM	256

/** 目标数据 */
#define IMP_BUFLEN_TGT_DATA	(IMP_BUFLEN_TGT_ITEM - (sizeof(IMP_U32) * 4 + sizeof(IMP_POINT_S) + sizeof(IMP_RECT_S)))

/** 目标数据结构 */
typedef struct impTGT_ITEM_S
{
	IMP_U32	u32Id;							/**< ID */
	IMP_U32	u32Status;						/**< 状态 */
	IMP_U32	u32Type;						/**< 类型 */
	IMP_U32	u32Event;						/**< 事件 */
	IMP_POINT_S	stPoint;					/**< 位置 */
	IMP_RECT_S	stRect;						/**< 区域 */
	IMP_U8	au8Data[IMP_BUFLEN_TGT_DATA];   /**< 数据 */
} TGT_ITEM_S;

/** 目标的最大个数 */
#define IMP_MAX_TGT_CNT			64

/** 目标集合 */
typedef struct impTGT_SET_S
{
	IMP_S32	    s32TargetNum;					 /**< 目标数目 */
	TGT_ITEM_S	astTargets[IMP_MAX_TGT_CNT];     /**< 目标数据 */
} TGT_SET_S;



/** 事件最大个数 */
#define IMP_MAX_EVT_CNT	128

/** 事件项个数 */
#define IMP_BUFLEN_EVT_ITEM	64

/** 事件长度 */
#define IMP_BUFLEN_EVT_DATA	(IMP_BUFLEN_EVT_ITEM - sizeof(IMP_U32) * 5)

/** 事件类型定义 */
typedef enum impEVT_TYPE_E
{
	IMP_EVT_TYPE_SignalBad          = 0x00000001,		/**< 视频信号异常检测 */
	IMP_EVT_TYPE_AlarmAtmAbabdum    = 0x00000101,		/**< ATM异物检测 */
	IMP_EVT_TYPE_AlarmAbabdum       = 0x00000102,		/**< 遗弃物检测 */
	IMP_EVT_TYPE_AlarmObjStolen     = 0x00000103,		/**< 被盗物检测 */
	IMP_EVT_TYPE_AlarmNoParking     = 0x00000104,		/**< 非法停车检测 */
	IMP_EVT_TYPE_AlarmPerimeter     = 0x00000105,		/**< 警戒区域检测 */
	IMP_EVT_TYPE_AlarmTripwire      = 0x00000106,		/**< 单警戒线检测 */
	IMP_EVT_TYPE_AlarmMTripwire     = 0x00000107,		/**< 双警戒线检测 */
	IMP_EVT_TYPE_AlarmAbnmlvel      = 0x00000108,		/**< 非正常速度检测	*/
	IMP_EVT_TYPE_AlarmLoiter        = 0x00000109,		/**< 徘徊检测 */

	IMP_EVT_TYPE_AVD_AlarmBase		= 0x00000200,		/**< AVD算法告警 */
	IMP_EVT_TYPE_AlarmSceneChg    	= 0x00000201,		/**< 场景变换 */
	IMP_EVT_TYPE_AlarmNoSignal      = 0x00000202,		/**< 信号缺失 */
	IMP_EVT_TYPE_AlarmBrightAbnml   = 0x00000203,		/**< 亮度异常 */
	IMP_EVT_TYPE_AlarmClarityAbnml  = 0x00000204,		/**< 视频模糊 */
	IMP_EVT_TYPE_AlarmColorAbnml    = 0x00000205,		/**< 视频偏色 */
	IMP_EVT_TYPE_AlarmNoise      	= 0x00000206,		/**< 噪声干扰 */
	IMP_EVT_TYPE_AlarmPtzLoseCrl    = 0x00000207,		/**< PTZ失控  */
	IMP_EVT_TYPE_AlarmFreeze      	= 0x00000208,		/**< 画面冻结 */
	IMP_EVT_TYPE_AlarmInterfere		= 0x00000209,		/**< 人为干扰 */
	IMP_EVT_TYPE_AlarmBrightHigh    = 0x0000020a,		/**< 亮度高 */
	IMP_EVT_TYPE_AlarmBrightLow     = 0x0000020b,		/**< 亮度低 */

	IMP_EVT_TYPE_VFD_AlarmBase		= 0x00000300,		/**< VFD算法告警 */
	IMP_EVT_TYPE_AlarmFaceCapture   = 0x00000301,		/**< 人脸抓拍 */
	IMP_EVT_TYPE_AlarmFaceCamouflage= 0x00000302,		/**< 人脸伪装 */
	IMP_EVT_TYPE_AlarmPasswordPeep  = 0x00000303,		/**< 密码偷窥 */
}EVT_TYPE_E;

/** 视频异常类型 */
enum
{
	VETYPE_CameraMoved = 0,					/**< 相机移动或遮挡 */
	VETYPE_SignalBad						/**< 视频质量差 */
};

/** 异常类型结构 */
typedef struct impEVT_DATA_EXCEPTION_S
{
	IMP_U32	u32Type;						/**< 异常类型 */
} EVT_DATA_EXCEPTION_S;

/** ATM事件数据结构 */
typedef struct impEVT_DATA_ATM_ABANDUM_S
{
	IMP_U32	u32TId;						/**< 目标标识 */
	IMP_RECT_S	stRect;					/**< 事件位置 */
} EVT_DATA_ATM_ABANDUM_S;

/** 遗弃物事件数据结构 */
typedef struct impEVT_DATA_ABANDUM_S
{
	IMP_U32	u32TId;						/**< 目标标识 */
	IMP_RECT_S	stRect;					/**< 事件位置 */
} EVT_DATA_ABANDUM_S;

/** 被盗物检测事件数据结构 */
typedef struct impEVT_DATA_OBJSTOLEN_S
{
	IMP_U32	u32TId;						/**< 目标标识 */
	IMP_RECT_S	stRect;					/**< 事件位置 */
} EVT_DATA_OBJSTOLEN_S;

/** 目标检测事件数据结构 */
typedef struct impEVT_DATA_TARGET_S
{
	IMP_U32	u32TId;						/**< 目标标识 */
	IMP_RECT_S	stRect;					/**< 事件位置 */
} EVT_DATA_TARGET_S;

/** 周界检测事件数据结构 */
typedef struct impEVT_DATA_PERIMETER_S
{
	IMP_U32	u32TId;						/**< 目标标识 */
	IMP_RECT_S  stRect;					/**< 事件位置 */
	struct
	{									/**< 规则 */
		IMP_U32	u32Mode;				/**< 警戒区模式 */
	} stRule;
} EVT_DATA_PERIMETER_S;

/** 绊线检测事件数据结构 */
typedef struct impEVT_DATA_TRIPWIRE_S
{
	IMP_U32	u32TId;						/**< 目标标识 */
	IMP_RECT_S	stRect;					/**< 事件位置 */
	IMP_U8  u8LineIndex;                /**< 绊线号 */
	struct
	{
		IMP_S32		s32Bidirection;		/**< 表示该绊线是否为双向绊线(0: 否, 1: 是) */
		IMP_S32		s32AbnmlAngle;		/**< 绊线禁止方向角度(unit: degree) */
		LINE_S		stLine;				/**< 绊线位置(unit: pixel) */
	} stRule;							/**< 规则 */
} EVT_DATA_TRIPWIRE_S;

/** 多绊线检测事件数据结构 */
typedef struct impEVT_DATA_MTRIPWIRE_S
{
	IMP_U32	u32TId;						/**< 目标标识 */
	IMP_RECT_S	stRect;					/**< 事件位置 */
	IMP_U8  u8LineIndex;                /**< 绊线号 */
	struct
	{
		IMP_S32		as32AbnmlAngle[2];	/**< 绊线禁止方向角度(unit: degree) */
		LINE_S		astLine[2];			/**< 绊线位置(unit: pixel) */
	} stRule;							/**< 规则 */
} EVT_DATA_MTRIPWIRE_S;

/** 异常速度检测事件数据结构 */
typedef struct impEVT_DATA_ABNMLVEL_S
{
	IMP_U32	u32TId;						/**< 目标标识*/
	IMP_RECT_S	stRect;					/**< 事件位置*/
} EVT_DATA_ABNMLVEL_S;

/** 徘徊速度检测事件数据结构 */
typedef struct impEVT_DATA_LOITER_S
{
	IMP_U32	u32TId;						/**< 目标标识*/
	IMP_RECT_S	stRect;					/**< 事件位置*/
} EVT_DATA_LOITER_S;

/** 事件项 */
typedef struct impEVT_ITEM_S
{
	IMP_U32	u32Type;						/**< 事件类型 */
	IMP_U32	u32Id;							/**< 事件标识 */
	IMP_U32	u32Level;						/**< 事件警报级别 */
	IMP_U32	u32Status;						/**< 事件状态 */
	IMP_U32	u32Zone;						/**< 事件发生区域 */
	IMP_U8	au8Data[IMP_BUFLEN_EVT_DATA];   /**< 事件数据 */
} EVT_ITEM_S;

/** 事件集 */
typedef struct impEVT_SET_S
{
	IMP_S32	    s32EventNum;					/**< 事件数目 */
	EVT_ITEM_S	astEvents[IMP_MAX_EVT_CNT];		/**< 事件数据 */
} EVT_SET_S;

/** PEA 处理结果 */
typedef struct impRESULT_S
{
	TGT_SET_S	stTargetSet;		/**< 目标 */
	EVT_SET_S	stEventSet;		    /**< 事件 */
} RESULT_S;




/** 最多人脸数 */
#define IMP_MAX_FACE_CNT 50
/** 人脸数据 */
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

/** 人脸数据集合 */
typedef struct impVFD_FACE_SET_S
{
    IMP_S32	    s32FaceNum;
	FACE_ITEM_S	astFaces[IMP_MAX_FACE_CNT];
}VFD_FACE_SET_S;

/** VFD 处理结果 */
typedef struct impVFD_RESULT_S
{
    VFD_FACE_SET_S stFaceSet;
    EVT_SET_S	stEventSet;
}VFD_RESULT_S;

/** 光流像素定义 */
typedef struct impOPTICAL_FLOW_PIXEL_S
{
	IMP_FLOAT fVx;   /** 光流像素X */
	IMP_FLOAT fVy;   /** 光流像素Y */
}OPTICAL_FLOW_PIXEL_S;

/** 光流 */
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
