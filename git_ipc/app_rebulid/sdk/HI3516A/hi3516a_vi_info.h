#ifndef _HI_3516A_VI_INFO_H
#define _HI_3516A_VI_INFO_H

#define SET_VI_DEV_ATTR_IMX178(info)\
{\
			info.enIntfMode = VI_MODE_LVDS;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_LOW;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 2592;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1944;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
			info.stDevRect.s32X = 0;\
            info.stDevRect.s32Y = 20;\
            info.stDevRect.u32Width  = 2592;\
            info.stDevRect.u32Height = 1944;\
}
#define SET_VI_DEV_ATTR_IMX185(info)\
{\
			info.enIntfMode = VI_MODE_MIPI;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_LOW;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
			info.stDevRect.s32X = 6;\
            info.stDevRect.s32Y = 0;\
            info.stDevRect.u32Width  = 1920;\
            info.stDevRect.u32Height = 1080;\
}
#define SET_VI_DEV_ATTR_AR0330(info)\
{\
			info.enIntfMode = VI_MODE_MIPI;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_LOW;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 2048;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1520;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
			info.stDevRect.s32X = 0;\
            info.stDevRect.s32Y = 0;\
            info.stDevRect.u32Width  = 2048;\
            info.stDevRect.u32Height = 1520;\
}
//TODO
#define SET_VI_DEV_ATTR_OV4689(info)\
{\
			info.enIntfMode = VI_MODE_LVDS;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFc00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 2592;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1520;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
			info.stDevRect.s32X = 0;\
            info.stDevRect.s32Y = 0;\
            info.stDevRect.u32Width  = 2592;\
            info.stDevRect.u32Height = 1520;\
}

#define SET_VI_DEV_ATTR_MN34220(info)\
{\
				info.enIntfMode = VI_MODE_LVDS;\
				info.enWorkMode = VI_WORK_MODE_1Multiplex;\
				info.au32CompMask[0] = 0xFFF00000;\
				info.au32CompMask[1] = 0x00000000;\
				info.enScanMode = VI_SCAN_PROGRESSIVE;\
				info.s32AdChnId[0] = -1;\
				info.s32AdChnId[1] = -1;\
				info.s32AdChnId[2] = -1;\
				info.s32AdChnId[3] = -1;\
				info.enDataSeq = VI_INPUT_DATA_YUYV;\
				info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
				info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
				info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
				info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
				info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
				info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
				info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
				info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
				info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
				info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
				info.enDataPath = VI_PATH_ISP;\
				info.enInputDataType = VI_DATA_TYPE_RGB;\
				info.bDataRev = HI_FALSE;\
				info.stDevRect.s32X = 0;\
				info.stDevRect.s32Y = 16;\
				info.stDevRect.u32Width  = 1920;\
				info.stDevRect.u32Height = 1080;\
}


#define SET_VI_DEV_ATTR_OV5658(info)\
{\
				info.enIntfMode = VI_MODE_MIPI;\
				info.enWorkMode = VI_WORK_MODE_1Multiplex;\
				info.au32CompMask[0] = 0xFFF00000;\
				info.au32CompMask[1] = 0x00000000;\
				info.enScanMode = VI_SCAN_PROGRESSIVE;\
				info.s32AdChnId[0] = -1;\
				info.s32AdChnId[1] = -1;\
				info.s32AdChnId[2] = -1;\
				info.s32AdChnId[3] = -1;\
				info.enDataSeq = VI_INPUT_DATA_YUYV;\
				info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
				info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
				info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
				info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
				info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
				info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
				info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
				info.stSynCfg.stTimingBlank.u32HsyncAct = 2592;\
				info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVact = 1944;\
				info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
				info.enDataPath = VI_PATH_ISP;\
				info.enInputDataType = VI_DATA_TYPE_RGB;\
				info.bDataRev = HI_FALSE;\
				info.stDevRect.s32X = 0;\
				info.stDevRect.s32Y = 0;\
				info.stDevRect.u32Width  = 2592;\
				info.stDevRect.u32Height = 1944;\
}
 
#define SET_VI_DEV_ATTR_IMX326(info)\
{\
				info.enIntfMode = VI_MODE_MIPI;\
				info.enWorkMode = VI_WORK_MODE_1Multiplex;\
				info.au32CompMask[0] = 0xFFF00000;\
				info.au32CompMask[1] = 0x00000000;\
				info.enScanMode = VI_SCAN_PROGRESSIVE;\ 			
				info.s32AdChnId[0] = -1;\
				info.s32AdChnId[1] = -1;\
				info.s32AdChnId[2] = -1;\
				info.s32AdChnId[3] = -1;\
				info.enDataSeq = VI_INPUT_DATA_YUYV;\
				info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
				info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
				info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
				info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
				info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
				info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
				info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
				info.stSynCfg.stTimingBlank.u32HsyncAct = 2160;\
				info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVact = 2160;\
				info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			 	info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
				info.enDataPath = VI_PATH_ISP;\
				info.enInputDataType = VI_DATA_TYPE_RGB;\
				info.bDataRev = HI_FALSE;\
				info.stDevRect.s32X = 0;\
				info.stDevRect.s32Y = 0;\
				info.stDevRect.u32Width  = 2160;\
				info.stDevRect.u32Height = 2160;\
}
#define SET_VI_DEV_ATTR_OS05A(info)\
{\
                info.enIntfMode = VI_MODE_MIPI;\
				info.enWorkMode = VI_WORK_MODE_1Multiplex;\
				info.au32CompMask[0] = 0xFFC00000;\
				info.au32CompMask[1] = 0x00000000;\
				info.enScanMode = VI_SCAN_PROGRESSIVE;\
				info.s32AdChnId[0] = -1;\
				info.s32AdChnId[1] = -1;\
				info.s32AdChnId[2] = -1;\
				info.s32AdChnId[3] = -1;\
				info.enDataSeq = VI_INPUT_DATA_YUYV;\
				info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
				info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
				info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
				info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
				info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
				info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
				info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
				info.stSynCfg.stTimingBlank.u32HsyncAct = 2592;\
				info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVact = 1944;\
				info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
				info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
				info.enDataPath = VI_PATH_ISP;\
				info.enInputDataType = VI_DATA_TYPE_RGB;\
				info.bDataRev = HI_FALSE;\
				info.stDevRect.s32X = 0;\
				info.stDevRect.s32Y = 0;\
				info.stDevRect.u32Width  = 2592;\
				info.stDevRect.u32Height = 1944;\
}
#endif //_HI_3516A_VI_INFO_H
