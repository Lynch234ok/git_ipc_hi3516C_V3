#include "../sdk_trace.h"

#include "hi_isp_cfg_parse.h"
#include "hi_isp_cfg.h"
#include "hi3518e.h"
#include "hi_isp_api.h"

#include <math.h>

extern const HI_U16 gs_Gamma[7][GAMMA_NODE_NUMBER];


static int
_isp_get_iso(void)
{
	uint32_t ret_iso = 0;
	ISP_EXP_INFO_S stExpInfo ;
	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(0, &stExpInfo));
	ret_iso = (uint32_t)((unsigned long long)stExpInfo.u32AGain * (unsigned long long)stExpInfo.u32DGain * (unsigned long long)stExpInfo.u32ISPDGain >> 20);
	return ret_iso / 1024;
}

static float
_isp_get_real_iso(void)
{
	uint32_t ret_iso = 0;
	ISP_EXP_INFO_S stExpInfo ;
	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(0, &stExpInfo));
	ret_iso = (uint32_t)((unsigned long long)stExpInfo.u32AGain * (unsigned long long)stExpInfo.u32DGain * (unsigned long long)stExpInfo.u32ISPDGain >> 20);
	return (float)ret_iso / 1024;
}


static int
count_from_iso(int n)
{	
	int ret = 0;
	int iso = _isp_get_iso();
	
	if(0 == n || 0 == iso){
		return 0;
	}
	if(n >= 10){
		if(iso < 2){
			ret = 0;
		}else if( iso < 4 ){
			ret = 1;
		}else if( iso < 8 ){
			ret = 2;	
		}else if( iso < 16 ){
			ret = 3;
		}else if( iso < 32 ){
			ret = 4;
		}else if( iso < 64 ){
			ret = 5;	
		}else if( iso < 128 ){
			ret = 6;	
		}else if( iso < 256 ){
			ret = 7;	
		}else if( iso < 512 ){
			ret = 8;
		}else if( iso < 1024 ){
			ret = 9;
		}else{
			ret = 9;
		}
	}
	else if(n >= 8){
		if(iso < 2){
			ret = 0;
		}else if( iso < 4 ){
			ret = 1;
		}else if( iso < 8 ){
			ret = 2;	
		}else if( iso < 16 ){
			ret = 3;
		}else if( iso < 32 ){
			ret = 4;
		}else if( iso < 64 ){
			ret = 5;	
		}else if( iso < 128 ){
			ret = 6;	
		}else if( iso < 256 ){
			ret = 7;	
		}else{
			ret = 7;
		}
	}else if(n >= 4){
		if(iso <= 16){
			ret = 0;
		}else if( iso < 32 ){
			ret = 1;
		}else if( iso < 64 ){
			ret = 2;	
		}else if( iso < 128 ){
			ret = 3;
		}else{
			ret = 3;
		}
	}else{
	}

	/*
	if( 0 == n || 0 == iso){
		return 0;
	}else if(iso >= 2 && iso <=  128){
		ret = (int)(log(iso) / log(2) + 0.43) / (8 / n) - 1;
			
	}else{
		ret =  (int)(log(iso) / log(2) + 0.41) / (8 / n);
	}

	if(ret < 0 ){
		ret = 0;
	}else if(ret >= n){
		ret = n - 1;
	}
*/
	return ret;
	
}

#ifdef INI_DEBUG_ECHO_OF_ALL
static void
debug_ae(LpIspCfgAttr ispCfgAttr)
{
	int i = 0, j = 0;
	printf("\033[31;1m[AE]\033[0m\n\n");
	ECHO_FOR_LOOP_0("AeCompensation", i, ispCfgAttr->aeCfgAttr.AeCompensation);
	ECHO_FOR_LOOP_0("MaxAgainTarget", i, ispCfgAttr->aeCfgAttr.MaxAgainTarget);
	ECHO_FOR_LOOP_0("MaxDgainTarget", i, ispCfgAttr->aeCfgAttr.MaxDgainTarget);
	ECHO_FOR_LOOP_0("MaxISPDgainTarget", i, ispCfgAttr->aeCfgAttr.MaxISPDgainTarget);
	ECHO_FOR_LOOP_0("MaxSystemGainTarget", i, ispCfgAttr->aeCfgAttr.MaxSystemGainTarget);
	ECHO_HEX_DEC("AeSpeed", ispCfgAttr->aeCfgAttr.AeSpeed);
	ECHO_HEX_DEC("AeTolerance", ispCfgAttr->aeCfgAttr.AeTolerance);
	ECHO_FOR_LOOP_2("AeHistslope", i, j, ispCfgAttr->aeCfgAttr.AeHistslope);
	ECHO_FOR_LOOP_0("AeHistOffset", i, ispCfgAttr->aeCfgAttr.AeHistOffset);	
	ECHO_FOR_LOOP_2("AeHistweight", i, j, ispCfgAttr->aeCfgAttr.AeHistweight);
}

static void
debug_awb(LpIspCfgAttr ispCfgAttr)
{
	int i = 0, j = 0;
	printf("\033[31;1m[AWB]\033[0m\n\n");

	ECHO_FOR_LOOP_0("HighColorTemp", i, ispCfgAttr->awbCfgAttr.HighColorTemp);
	ECHO_FOR_LOOP_0("MidColorTemp", i, ispCfgAttr->awbCfgAttr.MidColorTemp);
	ECHO_FOR_LOOP_0("LowColorTemp", i, ispCfgAttr->awbCfgAttr.LowColorTemp);

	ECHO_FOR_LOOP_2("HighCCM", i, j, ispCfgAttr->awbCfgAttr.HighCCM);
	ECHO_FOR_LOOP_2("MidCCM", i, j, ispCfgAttr->awbCfgAttr.MidCCM);
	ECHO_FOR_LOOP_2("LowCCM", i, j, ispCfgAttr->awbCfgAttr.LowCCM);
	ECHO_FOR_LOOP_0("WbRefTemp", i, ispCfgAttr->awbCfgAttr.WbRefTemp);
	ECHO_FOR_LOOP_2("StaticWB", i, j, ispCfgAttr->awbCfgAttr.StaticWB);
	ECHO_FOR_LOOP_2("AwbPara", i, j, ispCfgAttr->awbCfgAttr.AwbPara);
	ECHO_FOR_LOOP_2("Saturation", i, j, ispCfgAttr->awbCfgAttr.Saturation);
	ECHO_FOR_LOOP_2("BlackLevel", i, j, ispCfgAttr->awbCfgAttr.BlackLevel);
}

static void
debug_imp(LpIspCfgAttr ispCfgAttr)
{
	int i = 0, j = 0;
	printf("\033[31;1m[AWB]\033[0m\n\n");
	ECHO_FOR_LOOP_2("SharpenAltD", i, j, ispCfgAttr->impCfgAttr.SharpenAltD);
	ECHO_FOR_LOOP_2("SharpenAltUd", i, j, ispCfgAttr->impCfgAttr.SharpenAltUd);
	ECHO_FOR_LOOP_2("OverShoot", i, j, ispCfgAttr->impCfgAttr.OverShoot);
	ECHO_FOR_LOOP_2("UnderShoot", i, j, ispCfgAttr->impCfgAttr.UnderShoot);
	ECHO_FOR_LOOP_2("TextureNoiseThd", i, j, ispCfgAttr->impCfgAttr.TextureNoiseThd);
	ECHO_FOR_LOOP_2("EdgeNoiseThd", i, j, ispCfgAttr->impCfgAttr.EdgeNoiseThd);
	ECHO_FOR_LOOP_2("EnLowLumaShoot", i, j, ispCfgAttr->impCfgAttr.EnLowLumaShoot);

	ECHO_FOR_LOOP_2("SnrVarStrength", i, j, ispCfgAttr->impCfgAttr.SnrVarStrength);
	ECHO_FOR_LOOP_2("SnrThreshold", i, j, ispCfgAttr->impCfgAttr.SnrThreshold);
	
	ECHO_FOR_LOOP_2("DemosaicUuSlope", i, j, ispCfgAttr->impCfgAttr.DemosaicUuSlope);
	ECHO_FOR_LOOP_0("Gamma", i, ispCfgAttr->impCfgAttr.Gamma);
	ECHO_FOR_LOOP_0("DrcThreshold", i, ispCfgAttr->impCfgAttr.DrcThreshold);
	ECHO_FOR_LOOP_2("DrcAutoStrength", i, j,ispCfgAttr->impCfgAttr.DrcAutoStrength);
	
	ECHO_FOR_LOOP_1("DefectPixelSlope", i, ispCfgAttr->impCfgAttr.DefectPixelSlope);
	ECHO_FOR_LOOP_1("DefectPixelBlendRatio", i, ispCfgAttr->impCfgAttr.DefectPixelBlendRatio);
	
	ECHO_FOR_LOOP_0("SlowFrameRateHighHold", i, ispCfgAttr->impCfgAttr.SlowFrameRateHighHold);
	ECHO_FOR_LOOP_0("SlowFrameRateLowHold", i, ispCfgAttr->impCfgAttr.SlowFrameRateLowHold);
	ECHO_HEX_DEC("AntiFlikeThreshold", ispCfgAttr->impCfgAttr.AntiFlikeThreshold);	
	
	ECHO_HEX_DEC("DaylightToNight", ispCfgAttr->impCfgAttr.DaylightToNight);	
	ECHO_FOR_LOOP_1("NightToDaylight", i, ispCfgAttr->impCfgAttr.NightToDaylight);
	
	ECHO_FOR_LOOP_2("YPKStr", i, j, ispCfgAttr->impCfgAttr.YPKStr);
	ECHO_FOR_LOOP_2("YSFStr", i, j, ispCfgAttr->impCfgAttr.YSFStr);
	ECHO_FOR_LOOP_2("YTFStr", i, j, ispCfgAttr->impCfgAttr.YTFStr);
	ECHO_FOR_LOOP_2("TFStrMax", i, j, ispCfgAttr->impCfgAttr.TFStrMax);
	ECHO_FOR_LOOP_2("YPKTFStrMovStr", i, j, ispCfgAttr->impCfgAttr.TFStrMov);
	ECHO_FOR_LOOP_2("YSmthStr", i, j, ispCfgAttr->impCfgAttr.YSmthStr);
	ECHO_FOR_LOOP_2("YSmthRat", i, j, ispCfgAttr->impCfgAttr.YSmthRat);
	ECHO_FOR_LOOP_2("YSFStrDlt", i, j, ispCfgAttr->impCfgAttr.YSFStrDlt);
	ECHO_FOR_LOOP_2("YSFStrDl", i, j, ispCfgAttr->impCfgAttr.YSFStrDl);
	ECHO_FOR_LOOP_2("YTFStrDlt", i, j, ispCfgAttr->impCfgAttr.YTFStrDlt);
	ECHO_FOR_LOOP_2("YTFStrDl", i, j, ispCfgAttr->impCfgAttr.YTFStrDl);
	ECHO_FOR_LOOP_2("YSFBriRat", i, j, ispCfgAttr->impCfgAttr.YSFBriRat);
	ECHO_FOR_LOOP_2("CSFStr", i, j, ispCfgAttr->impCfgAttr.CSFStr);
	ECHO_FOR_LOOP_2("CTFstr", i, j, ispCfgAttr->impCfgAttr.CTFstr);
	ECHO_FOR_LOOP_2("YTFMdWin", i, j, ispCfgAttr->impCfgAttr.YTFMdWin);

}

static int
debug_register(int isDaylight, int scene)
{
	int i = 0,j = 0;

	ISP_DEV IspDev = 0;
	int iso = _isp_get_iso();

	ISP_EXPOSURE_ATTR_S AE_obj;

	ISP_COLORMATRIX_ATTR_S CCM_obj;
	ISP_WB_ATTR_S AWB_obj;
	ISP_SATURATION_ATTR_S SATURATION_obj;
	ISP_BLACK_LEVEL_S BLACK_LEVEL_obj;

	ISP_SHARPEN_ATTR_S SHARPEN_obj;
	ISP_NR_ATTR_S NR_obj;
	ISP_DEMOSAIC_ATTR_S DEMOSAIC_obj;
	ISP_GAMMA_ATTR_S GAMMA_obj;
//	VPSS_GRP_PARAM_S VPSS_obj;
	VPSS_NR_PARAM_U   VpssU_obj ;
	ISP_DRC_ATTR_S DRC_obj;
	ISP_DP_DYNAMIC_ATTR_S DP_obj;

	if(HI_SUCCESS == HI_MPI_ISP_GetExposureAttr(IspDev, &AE_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetCCMAttr(IspDev, &CCM_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetWBAttr(IspDev, &AWB_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetSaturationAttr(IspDev, &SATURATION_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetBlackLevelAttr(IspDev, &BLACK_LEVEL_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetSharpenAttr(IspDev, &SHARPEN_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetNRAttr(IspDev, &NR_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetGammaAttr(IspDev, &GAMMA_obj) \
			&& HI_SUCCESS == HI_MPI_VPSS_GetNRParam(IspDev, &VpssU_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetDPDynamicAttr(IspDev, &DP_obj))
	{
		printf("\n\033[31;1m##### [%s] ##############################################################################################\033[0m\n\n", __func__);
	// iso
		printf("\033[31;1m[ISO]\033[0m\n");
		ECHO_HEX_DEC("ISO", iso);
	// param
		printf("\033[31;1m[Param]\033[0m\n");
		ECHO_HEX_DEC("isDayligh", isDaylight);
		ECHO_HEX_DEC("Scene", scene);
	// AE_obj
		printf("\033[31;1m[AE]\033[0m\n");
		ECHO_HEX_DEC("u8Compensation", AE_obj.stAuto.u8Compensation);
		ECHO_HEX_DEC("u32AGainMax", AE_obj.stAuto.stAGainRange.u32Max);
		ECHO_HEX_DEC("u32DGainMax", AE_obj.stAuto.stDGainRange.u32Max);
		ECHO_HEX_DEC("u32ISPDGainMax", AE_obj.stAuto.stISPDGainRange.u32Max);
		ECHO_HEX_DEC("u32SystemGainMax", AE_obj.stAuto.stSysGainRange.u32Max);
		ECHO_HEX_DEC("u8Speed", AE_obj.stAuto.u8Speed);
		ECHO_HEX_DEC("u8Tolerance", AE_obj.stAuto.u8Tolerance);
		ECHO_HEX_DEC("u16HistRatioSlope", AE_obj.stAuto.u16HistRatioSlope);
		ECHO_HEX_DEC("u8MaxHistOffset", AE_obj.stAuto.u8MaxHistOffset);
		ECHO_HEX_DEC("AntiFlike.bEnable", AE_obj.stAuto.stAntiflicker.bEnable);
		ECHO_HEX_DEC("AntiFlike.enMode", AE_obj.stAuto.stAntiflicker.enMode);
		ECHO_HEX_DEC("AntiFlike.u8Frequency", AE_obj.stAuto.stAntiflicker.u8Frequency);		
		ECHO_FOR_LOOP_2("AeHistweight", i,j, AE_obj.stAuto.au8Weight);
		
	// CCM_obj
		printf("\033[31;1m[CCM]\033[0m\n");
		ECHO_HEX_DEC("u16HighColorTemp", CCM_obj.stAuto.u16HighColorTemp);
		ECHO_HEX_DEC("u16MidColorTemp", CCM_obj.stAuto.u16MidColorTemp);
		ECHO_HEX_DEC("u16LowColorTemp", CCM_obj.stAuto.u16LowColorTemp);

		ECHO_FOR_LOOP_1("au16HighCCM", i, CCM_obj.stAuto.au16HighCCM);
		ECHO_FOR_LOOP_1("au16MidCCM", i, CCM_obj.stAuto.au16MidCCM);
		ECHO_FOR_LOOP_1("au16LowCCM", i, CCM_obj.stAuto.au16LowCCM);
	// AWB_obj
		printf("\033[31;1m[AWB]\033[0m\n");
		ECHO_HEX_DEC("u16RefColorTemp", AWB_obj.stAuto.u16RefColorTemp);
		ECHO_FOR_LOOP_1("au16StaticWB", i, AWB_obj.stAuto.au16StaticWB);
		ECHO_FOR_LOOP_1("AwbPara", i, AWB_obj.stAuto.as32CurvePara);
	// SATURATION_obj
		printf("\033[31;1m[SATURATION]\033[0m\n");
		ECHO_FOR_LOOP_1("au8Sat", i, SATURATION_obj.stAuto.au8Sat);
	// BLACK_LEVEL_obj
		printf("\033[31;1m[BLACK_LEVEL]\033[0m\n");
		ECHO_FOR_LOOP_1("BlackLevel", i, BLACK_LEVEL_obj.au16BlackLevel);
	// SHARPEN_obj
		printf("\033[31;1m[SHARPEN]\033[0m\n");
		ECHO_FOR_LOOP_1("au8SharpenD", i, SHARPEN_obj.stAuto.au8SharpenD);
		ECHO_FOR_LOOP_1("au8SharpenUd", i, SHARPEN_obj.stAuto.au8SharpenUd);		
		ECHO_FOR_LOOP_1("au8OverShoot", i, SHARPEN_obj.stAuto.au8OverShoot);
		ECHO_FOR_LOOP_1("au8UnderShoot", i, SHARPEN_obj.stAuto.au8UnderShoot);
		ECHO_FOR_LOOP_1("au8UnderShoot", i, SHARPEN_obj.stAuto.au8UnderShoot);
		ECHO_FOR_LOOP_1("au8EdgeNoiseThd", i, SHARPEN_obj.stAuto.au8EdgeNoiseThd);	
		ECHO_FOR_LOOP_1("abEnLowLumaShoot", i, SHARPEN_obj.stAuto.abEnLowLumaShoot);
	// NR_obj
		printf("\033[31;1m[SNR]\033[0m\n");
		ECHO_FOR_LOOP_1("au8VarStrength", i, NR_obj.stAuto.au8VarStrength);
		ECHO_FOR_LOOP_1("au16Threshold", i, NR_obj.stAuto.au16Threshold);
	// DEMOSAIC_obj
		printf("\033[31;1m[DEMOSAIC]\033[0m\n");
		ECHO_HEX_DEC("u16UuSlope", DEMOSAIC_obj.u16UuSlope);
	// GAMMA_obj
		printf("\033[31;1m[GAMMA]\033[0m\n");
		ECHO_HEX_DEC("enCurveType", GAMMA_obj.enCurveType);
		ECHO_FOR_LOOP_1("GammaTable", i, GAMMA_obj.u16Table);

	// DRC_obj
		printf("\033[31;1m[DRC]\033[0m\n");
		ECHO_HEX_DEC("bEnable", DRC_obj.bEnable);
		ECHO_HEX_DEC("u8Strength", DRC_obj.stAuto.u8Strength);	

	// DP_obj
		printf("\033[31;1m[DefectPixel]\033[0m\n");
		ECHO_HEX_DEC("au16Slope", DP_obj.stAuto.au16Slope);
		ECHO_HEX_DEC("au16BlendRatio", DP_obj.stAuto.au16BlendRatio);

	// VPSS_U_obj
		printf("\033[31;1m[VPSS]\033[0m\n");
		ECHO_HEX_DEC("s32YPKStr", VpssU_obj.stNRParam_V1.s32YPKStr);
		ECHO_HEX_DEC("s32YSFStr", VpssU_obj.stNRParam_V1.s32YSFStr);
		ECHO_HEX_DEC("s32YTFStr", VpssU_obj.stNRParam_V1.s32YTFStr);
		ECHO_HEX_DEC("s32TFStrMax", VpssU_obj.stNRParam_V1.s32TFStrMax);
		ECHO_HEX_DEC("s32TFStrMov", VpssU_obj.stNRParam_V1.s32TFStrMov);
		ECHO_HEX_DEC("s32YSmthStr", VpssU_obj.stNRParam_V1.s32YSmthStr);
		ECHO_HEX_DEC("s32YSmthRat", VpssU_obj.stNRParam_V1.s32YSmthRat);
		ECHO_HEX_DEC("s32YSFStrDlt", VpssU_obj.stNRParam_V1.s32YSFStrDlt);
		ECHO_HEX_DEC("s32YSFStrDl", VpssU_obj.stNRParam_V1.s32YSFStrDl);
		ECHO_HEX_DEC("s32YTFStrDlt", VpssU_obj.stNRParam_V1.s32YTFStrDlt);
		ECHO_HEX_DEC("s32YTFStrDl", VpssU_obj.stNRParam_V1.s32YTFStrDl);
		ECHO_HEX_DEC("s32YSFBriRat", VpssU_obj.stNRParam_V1.s32YSFBriRat);
		ECHO_HEX_DEC("s32CSFStr", VpssU_obj.stNRParam_V1.s32CSFStr);
		ECHO_HEX_DEC("s32CTFstr", VpssU_obj.stNRParam_V1.s32CTFstr);
		ECHO_HEX_DEC("s32YTFMdWin", VpssU_obj.stNRParam_V1.s32YTFMdWin);
		return 0;
	}
	return -1;
}

static int
debug_all(LpIspCfgAttr ispCfgAttr)
{
	printf("\n\033[31;1m##### [%s] ##############################################################################################\n\n\033[0m", __func__);
	printf("\033[31;1miso = %d\033[0m\n\n", _isp_get_iso());
	debug_ae(ispCfgAttr);
	printf("\n");
	debug_awb(ispCfgAttr);
	printf("\n");
	debug_imp(ispCfgAttr);
	printf("\n\n");
	return 0;
}
#endif

static int _hi_isp_cfg_set_slow_framerate2(uint8_t bValue,LpIspCfgAttr ispCfgAttr)
{
	uint8_t Value = bValue ? 0x1:0x0;
	VENC_CHN_ATTR_S vencChannelAttr;
	ISP_PUB_ATTR_S stPubAttr;	
	int actual_fps;
	uint8_t  frame_rate;
	
	frame_rate = ispCfgAttr->impCfgAttr.src_framerate;
		
	SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));
	HI_MPI_VENC_GetChnAttr(0,&vencChannelAttr);
//	printf("old_FRate = %f\n",stPubAttr.f32FrameRate);

	actual_fps = frame_rate/((1 << Value)+1);

	switch(vencChannelAttr.stRcAttr.enRcMode){
		default:
		case VENC_RC_MODE_H264VBR:
			if(bValue == true && stPubAttr.f32FrameRate == frame_rate){//slow framerate			
				stPubAttr.f32FrameRate = actual_fps;
				if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate){
					vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate; 
				}else{
					vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate =	actual_fps;
				}	
				
				HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
				printf("new_FRate = %f\n",stPubAttr.f32FrameRate);

			}else if (bValue == false && stPubAttr.f32FrameRate < frame_rate ){//actual framerate	
				stPubAttr.f32FrameRate = actual_fps;
				if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate){
					vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate; 
				}else{
					vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate =	actual_fps;
				}
				
				HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
				printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
			}else{
			}
			break;
		
		case VENC_RC_MODE_H264CBR:
			if(bValue == true && stPubAttr.f32FrameRate == frame_rate){//slow framerate
				stPubAttr.f32FrameRate = actual_fps;
				if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
					vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
				}else{
					vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =	actual_fps;
				}	
				
				HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
				printf("new_FRate = %f\n",stPubAttr.f32FrameRate);

			}else if(bValue == false && stPubAttr.f32FrameRate < frame_rate){//actual framerate
				stPubAttr.f32FrameRate = actual_fps;
				if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
					vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
				}else{
					vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =	actual_fps;
				}
				
				HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
				printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
			}else{
			}
			break;		
		}
	return 0;

}

static int _hi_isp_cfg_set_slow_framerate(uint8_t bValue,LpIspCfgAttr ispCfgAttr)
{
	uint8_t Value = bValue ? 0x1:0x0;
	VENC_CHN_ATTR_S vencChannelAttr;
	ISP_PUB_ATTR_S stPubAttr;	
	int actual_fps;
	uint8_t  frame_rate;
	
	frame_rate = ispCfgAttr->impCfgAttr.src_framerate;
		
	SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));
	HI_MPI_VENC_GetChnAttr(0,&vencChannelAttr);
//	printf("old_FRate = %f\n",stPubAttr.f32FrameRate);

	actual_fps = frame_rate/(1 << Value);

	switch(vencChannelAttr.stRcAttr.enRcMode){
		default:
		case VENC_RC_MODE_H264VBR:
			if(bValue == true && stPubAttr.f32FrameRate == frame_rate){//slow framerate			
				stPubAttr.f32FrameRate = actual_fps;
				if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate){
					stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate;
					vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate; 
				}else{
					vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate =	actual_fps;
				}	
				
				HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
				printf("new_FRate = %f\n",stPubAttr.f32FrameRate);

			}else if (bValue == false && stPubAttr.f32FrameRate < frame_rate ){//actual framerate	
				stPubAttr.f32FrameRate = actual_fps;
				if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate){
					stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate;
					vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate; 
				}else{
					vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate =	actual_fps;
				}
				
				HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
				printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
			}else{
			}
			break;
		
		case VENC_RC_MODE_H264CBR:
			if(bValue == true && stPubAttr.f32FrameRate == frame_rate){//slow framerate
				stPubAttr.f32FrameRate = actual_fps;
				if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
					stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
					vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
				}else{
					vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =	actual_fps;
				}	
				
				HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
				printf("new_FRate = %f\n",stPubAttr.f32FrameRate);

			}else if(bValue == false && stPubAttr.f32FrameRate < frame_rate){//actual framerate
				stPubAttr.f32FrameRate = actual_fps;
				if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
					stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
					vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
				}else{
					vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =	actual_fps;
				}
				
				HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
				printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
			}else{
			}
			break;		
		}
	return 0;


}


int HI_ISP_cfg_set_ae(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	int i = 0,j = 0, sum = 0;
	ISP_DEV IspDev = 0;
	HI_U8 u8Weight[AE_ZONE_ROW][AE_ZONE_COLUMN] ;
	ISP_EXPOSURE_ATTR_S AE_obj;
	
	int iso = _isp_get_iso();

	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetExposureAttr(IspDev, &AE_obj))
	{
		
		AE_obj.stAuto.u8Compensation = ispCfgAttr->aeCfgAttr.AeCompensation[isDaylight];
		AE_obj.stAuto.stAGainRange.u32Max = ispCfgAttr->aeCfgAttr.MaxAgainTarget[isDaylight];
		AE_obj.stAuto.stDGainRange.u32Max = ispCfgAttr->aeCfgAttr.MaxDgainTarget[isDaylight];
		AE_obj.stAuto.stISPDGainRange.u32Max = ispCfgAttr->aeCfgAttr.MaxISPDgainTarget[isDaylight];
		AE_obj.stAuto.stSysGainRange.u32Max = ispCfgAttr->aeCfgAttr.MaxSystemGainTarget[isDaylight];
		AE_obj.stAuto.u8Speed = ispCfgAttr->aeCfgAttr.AeSpeed;
		AE_obj.stAuto.u8Tolerance = ispCfgAttr->aeCfgAttr.AeTolerance;
		AE_obj.stAuto.u16HistRatioSlope = ispCfgAttr->aeCfgAttr.AeHistslope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->aeCfgAttr.AeHistslope[isDaylight]))];
		AE_obj.stAuto.u8MaxHistOffset = ispCfgAttr->aeCfgAttr.AeHistOffset[isDaylight];

		AE_obj.stAuto.stAntiflicker.bEnable = iso < ispCfgAttr->impCfgAttr.AntiFlikeThreshold ? HI_TRUE : HI_FALSE;
		AE_obj.stAuto.stAntiflicker.u8Frequency = ispCfgAttr->impCfgAttr.flick_frequency;

		
		for(i = 0; i < AE_ZONE_ROW; i++){
			for(j = 0;j < AE_ZONE_COLUMN; j++){
				u8Weight[i][j] = ispCfgAttr->aeCfgAttr.AeHistweight[isDaylight][i *AE_ZONE_COLUMN + j];
				sum = sum + u8Weight[i][j] ;
			}
		}
		if(0 != sum ){
			memcpy(AE_obj.stAuto.au8Weight,u8Weight,sizeof(u8Weight));
		}
		
		return HI_SUCCESS == HI_MPI_ISP_SetExposureAttr(IspDev, &AE_obj) ? 0 : -1;
	}
	return -1;
}

int HI_ISP_cfg_get_ae(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	ISP_DEV IspDev = 0;
	int i = 0,j = 0;
	ISP_EXPOSURE_ATTR_S AE_obj;

	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetExposureAttr(IspDev, &AE_obj))
	{
		ispCfgAttr->aeCfgAttr.AeCompensation[isDaylight] = AE_obj.stAuto.u8Compensation;
		ispCfgAttr->aeCfgAttr.MaxAgainTarget[isDaylight] = AE_obj.stAuto.stAGainRange.u32Max;
		ispCfgAttr->aeCfgAttr.MaxDgainTarget[isDaylight] = AE_obj.stAuto.stDGainRange.u32Max;
		ispCfgAttr->aeCfgAttr.MaxISPDgainTarget[isDaylight] = AE_obj.stAuto.stISPDGainRange.u32Max;
		ispCfgAttr->aeCfgAttr.MaxSystemGainTarget[isDaylight] = AE_obj.stAuto.stSysGainRange.u32Max;
		ispCfgAttr->aeCfgAttr.AeSpeed = AE_obj.stAuto.u8Speed;
		ispCfgAttr->aeCfgAttr.AeTolerance = AE_obj.stAuto.u8Tolerance;
		ispCfgAttr->aeCfgAttr.AeHistslope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->aeCfgAttr.AeHistslope[isDaylight]))] = AE_obj.stAuto.u16HistRatioSlope;
		ispCfgAttr->aeCfgAttr.AeHistOffset[isDaylight] = AE_obj.stAuto.u8MaxHistOffset;

		for(i = 0; i < AE_ZONE_ROW; i++)
		{
			for(j = 0;j < AE_ZONE_COLUMN; j++)
			{
				ispCfgAttr->aeCfgAttr.AeHistweight[isDaylight][i *AE_ZONE_COLUMN + j] = AE_obj.stAuto.au8Weight[i][j];
			}
		}

		return 0;
	}
	return -1;
}

int HI_ISP_cfg_set_awb(int scene, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;

	ISP_DEV IspDev = 0;

	ISP_COLORMATRIX_ATTR_S CCM_obj;
	ISP_WB_ATTR_S AWB_obj;
	ISP_SATURATION_ATTR_S SATURATION_obj;
	ISP_BLACK_LEVEL_S BLACK_LEVEL_obj;

	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetCCMAttr(IspDev, &CCM_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetWBAttr(IspDev, &AWB_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetSaturationAttr(IspDev, &SATURATION_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetBlackLevelAttr(IspDev, &BLACK_LEVEL_obj))
	{
	// CCM_obj
		CCM_obj.stAuto.u16HighColorTemp = ispCfgAttr->awbCfgAttr.HighColorTemp[scene];
		CCM_obj.stAuto.u16MidColorTemp = ispCfgAttr->awbCfgAttr.MidColorTemp[scene];
		CCM_obj.stAuto.u16LowColorTemp = ispCfgAttr->awbCfgAttr.LowColorTemp[scene];

		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.HighCCM[scene]); i++)
		{
			CCM_obj.stAuto.au16HighCCM[i] = ispCfgAttr->awbCfgAttr.HighCCM[scene][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.MidCCM[scene]); i++)
		{
			CCM_obj.stAuto.au16MidCCM[i] = ispCfgAttr->awbCfgAttr.MidCCM[scene][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.LowCCM[scene]); i++)
		{
			CCM_obj.stAuto.au16LowCCM[i] = ispCfgAttr->awbCfgAttr.LowCCM[scene][i];
		}
	// AWB_obj
		AWB_obj.stAuto.u16RefColorTemp = ispCfgAttr->awbCfgAttr.WbRefTemp[scene];
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.StaticWB[scene]); i++)
		{
			AWB_obj.stAuto.au16StaticWB[i] = ispCfgAttr->awbCfgAttr.StaticWB[scene][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.AwbPara[scene]); i++)
		{
			AWB_obj.stAuto.as32CurvePara[i] = ispCfgAttr->awbCfgAttr.AwbPara[scene][i];
		}
	// SATURATION_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.Saturation[scene]); i++)
		{
			SATURATION_obj.stAuto.au8Sat[i] = ispCfgAttr->awbCfgAttr.Saturation[scene][i];
		}
	// BLACK_LEVEL_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.BlackLevel[scene]); i++)
		{
			BLACK_LEVEL_obj.au16BlackLevel[i] = ispCfgAttr->awbCfgAttr.BlackLevel[scene][i];
		}

		return HI_SUCCESS == HI_MPI_ISP_SetCCMAttr(IspDev, &CCM_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetWBAttr(IspDev, &AWB_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetSaturationAttr(IspDev, &SATURATION_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetBlackLevelAttr(IspDev, &BLACK_LEVEL_obj) ? 0 : -1;
	}
	return -1;
}

int HI_ISP_cfg_get_awb(int scene, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;

	ISP_DEV IspDev = 0;

	ISP_COLORMATRIX_ATTR_S CCM_obj;
	ISP_WB_ATTR_S AWB_obj;
	ISP_SATURATION_ATTR_S SATURATION_obj;
	ISP_BLACK_LEVEL_S BLACK_LEVEL_obj;

	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetCCMAttr(IspDev, &CCM_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetWBAttr(IspDev, &AWB_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetSaturationAttr(IspDev, &SATURATION_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetBlackLevelAttr(IspDev, &BLACK_LEVEL_obj))
	{
	// CCM_obj
		ispCfgAttr->awbCfgAttr.HighColorTemp[scene] = CCM_obj.stAuto.u16HighColorTemp;
		ispCfgAttr->awbCfgAttr.MidColorTemp[scene] = CCM_obj.stAuto.u16MidColorTemp;
		ispCfgAttr->awbCfgAttr.LowColorTemp[scene] = CCM_obj.stAuto.u16LowColorTemp;

		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.HighCCM[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.HighCCM[scene][i] = CCM_obj.stAuto.au16HighCCM[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.MidCCM[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.MidCCM[scene][i] = CCM_obj.stAuto.au16MidCCM[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.LowCCM[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.LowCCM[scene][i] = CCM_obj.stAuto.au16LowCCM[i];
		}
	// AWB_obj
		ispCfgAttr->awbCfgAttr.WbRefTemp[scene] = AWB_obj.stAuto.u16RefColorTemp;
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.StaticWB[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.StaticWB[scene][i] = AWB_obj.stAuto.au16StaticWB[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.AwbPara[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.AwbPara[scene][i] = AWB_obj.stAuto.as32CurvePara[i];
		}
	// SATURATION_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.Saturation[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.Saturation[scene][i] = SATURATION_obj.stAuto.au8Sat[i];
		}
	// BLACK_LEVEL_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.BlackLevel[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.BlackLevel[scene][i] = BLACK_LEVEL_obj.au16BlackLevel[i];
		}

		return 0;
	}
	return -1;
}

int HI_ISP_cfg_set_imp(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;

	ISP_DEV IspDev = 0;
	int iso = _isp_get_iso();

	ISP_SHARPEN_ATTR_S SHARPEN_obj;
	ISP_NR_ATTR_S NR_obj;
	ISP_DEMOSAIC_ATTR_S DEMOSAIC_obj;
	ISP_GAMMA_ATTR_S GAMMA_obj;
	ISP_DRC_ATTR_S DRC_obj;
	ISP_DP_DYNAMIC_ATTR_S DP_obj;
	VPSS_NR_PARAM_U   VpssU_obj ;

	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetSharpenAttr(IspDev, &SHARPEN_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetNRAttr(IspDev, &NR_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetGammaAttr(IspDev, &GAMMA_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDPDynamicAttr(IspDev, &DP_obj) \
				&& HI_SUCCESS == HI_MPI_VPSS_GetNRParam(IspDev,&VpssU_obj))
	{
	// SHARPEN_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltD[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8SharpenD[i] = ispCfgAttr->impCfgAttr.SharpenAltD[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltUd[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8SharpenUd[i] = ispCfgAttr->impCfgAttr.SharpenAltUd[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.OverShoot[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8OverShoot[i] = ispCfgAttr->impCfgAttr.OverShoot[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.UnderShoot[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8UnderShoot[i] = ispCfgAttr->impCfgAttr.UnderShoot[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.TextureNoiseThd[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8TextureNoiseThd[i] = ispCfgAttr->impCfgAttr.TextureNoiseThd[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeNoiseThd[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8EdgeNoiseThd[i] = ispCfgAttr->impCfgAttr.EdgeNoiseThd[isDaylight][i];
		}
		
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EnLowLumaShoot[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.abEnLowLumaShoot[i] = ispCfgAttr->impCfgAttr.EnLowLumaShoot[isDaylight][i];
		}
										
	// NR_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrVarStrength[isDaylight]); i++)
		{
			NR_obj.stAuto.au8VarStrength[i] = ispCfgAttr->impCfgAttr.SnrVarStrength[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrThreshold[isDaylight]); i++)
		{
			NR_obj.stAuto.au16Threshold[i] = ispCfgAttr->impCfgAttr.SnrThreshold[isDaylight][i];
		}

		
	// DEMOSAIC_obj
		DEMOSAIC_obj.u16UuSlope = ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))];

	// GAMMA_obj
		GAMMA_obj.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
		for(i = 0; i < ARRAY_NUM(gs_Gamma[ispCfgAttr->impCfgAttr.Gamma[isDaylight]]); i++)
		{
			GAMMA_obj.u16Table[i] = gs_Gamma[ispCfgAttr->impCfgAttr.Gamma[isDaylight]][i];
		}
		
	// DRC_obj
		//DRC_obj.stAuto.u8Strength = ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight];
		DRC_obj.bEnable =  (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE;
		DRC_obj.enOpType = OP_TYPE_MANUAL;
		DRC_obj.stManual.u8Strength = ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight];
	// DP_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelSlope); i++)
		{
			DP_obj.stAuto.au16Slope[i] = ispCfgAttr->impCfgAttr.DefectPixelSlope[i];
		}
		
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelBlendRatio); i++)
		{
			DP_obj.stAuto.au16BlendRatio[i]= ispCfgAttr->impCfgAttr.DefectPixelBlendRatio[i];
		}

	// VPSS_U
		VpssU_obj.stNRParam_V1.s32YPKStr = ispCfgAttr->impCfgAttr.YPKStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YPKStr[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YSFStr = ispCfgAttr->impCfgAttr.YSFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStr[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YTFStr = ispCfgAttr->impCfgAttr.YTFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFStr[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32TFStrMax = ispCfgAttr->impCfgAttr.TFStrMax[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFStrMax[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32TFStrMov = ispCfgAttr->impCfgAttr.TFStrMov[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFStrMov[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YSmthStr = ispCfgAttr->impCfgAttr.YSmthStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSmthStr[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YSmthRat = ispCfgAttr->impCfgAttr.YSmthRat[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSmthRat[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YSFStrDlt = ispCfgAttr->impCfgAttr.YSFStrDlt[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStrDlt[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YSFStrDl = ispCfgAttr->impCfgAttr.YSFStrDl[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStrDl[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YTFStrDlt = ispCfgAttr->impCfgAttr.YTFStrDlt[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFStrDlt[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YTFStrDl = ispCfgAttr->impCfgAttr.YTFStrDl[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFStrDl[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YSFBriRat = ispCfgAttr->impCfgAttr.YSFBriRat[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFBriRat[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32CSFStr = ispCfgAttr->impCfgAttr.CSFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.CSFStr[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32CTFstr = ispCfgAttr->impCfgAttr.CTFstr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.CTFstr[isDaylight]))];
		VpssU_obj.stNRParam_V1.s32YTFMdWin = ispCfgAttr->impCfgAttr.YTFMdWin[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFMdWin[isDaylight]))];


	// obj_fps	
		if(iso < ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			_hi_isp_cfg_set_slow_framerate(false,ispCfgAttr);
			
		}else if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			_hi_isp_cfg_set_slow_framerate(true,ispCfgAttr);
		}
		

		return HI_SUCCESS == HI_MPI_ISP_SetSharpenAttr(IspDev, &SHARPEN_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetNRAttr(IspDev, &NR_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetGammaAttr(IspDev, &GAMMA_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetDPDynamicAttr(IspDev, &DP_obj) \
				&& HI_SUCCESS == HI_MPI_VPSS_SetNRParam(IspDev,&VpssU_obj)? 0 : -1;
	}
	return -1;
}


int HI_ISP_cfg_set_imp_single(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	ISP_DEV IspDev = 0;
	int iso = _isp_get_iso();

	ISP_DEMOSAIC_ATTR_S DEMOSAIC_obj;
	VPSS_NR_PARAM_U   VpssU_obj ;
	ISP_DRC_ATTR_S DRC_obj;
	ISP_EXPOSURE_ATTR_S AE_obj; 
	
 	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
				&& HI_SUCCESS == HI_MPI_VPSS_GetNRParam(IspDev,&VpssU_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetExposureAttr(IspDev,&AE_obj))
	{
	// DEMOSAIC_obj
		if(DEMOSAIC_obj.u16UuSlope != ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))]){
			DEMOSAIC_obj.u16UuSlope = ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))];
			HI_MPI_ISP_SetDemosaicAttr(IspDev, &DEMOSAIC_obj);		
	//		printf("HI_MPI_ISP_SetDemosaicAttr\r\n");
		}

	// VpssU_obj
		static int cnt_num_old = 0;
		int cur_cnt_num = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YPKStr[isDaylight]));
		if(cnt_num_old != cur_cnt_num){		
			VpssU_obj.stNRParam_V1.s32YPKStr = ispCfgAttr->impCfgAttr.YPKStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YPKStr[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YSFStr = ispCfgAttr->impCfgAttr.YSFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStr[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YTFStr = ispCfgAttr->impCfgAttr.YTFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFStr[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32TFStrMax = ispCfgAttr->impCfgAttr.TFStrMax[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFStrMax[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32TFStrMov = ispCfgAttr->impCfgAttr.TFStrMov[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFStrMov[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YSmthStr = ispCfgAttr->impCfgAttr.YSmthStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSmthStr[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YSmthRat = ispCfgAttr->impCfgAttr.YSmthRat[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSmthRat[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YSFStrDlt = ispCfgAttr->impCfgAttr.YSFStrDlt[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStrDlt[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YSFStrDl = ispCfgAttr->impCfgAttr.YSFStrDl[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStrDl[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YTFStrDlt = ispCfgAttr->impCfgAttr.YTFStrDlt[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFStrDlt[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YTFStrDl = ispCfgAttr->impCfgAttr.YTFStrDl[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFStrDl[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YSFBriRat = ispCfgAttr->impCfgAttr.YSFBriRat[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFBriRat[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32CSFStr = ispCfgAttr->impCfgAttr.CSFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.CSFStr[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32CTFstr = ispCfgAttr->impCfgAttr.CTFstr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.CTFstr[isDaylight]))];
			VpssU_obj.stNRParam_V1.s32YTFMdWin = ispCfgAttr->impCfgAttr.YTFMdWin[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFMdWin[isDaylight]))];
	//		printf("HI_MPI_VPSS_SetNRParam\r\n");
			cnt_num_old = cur_cnt_num;
			HI_MPI_VPSS_SetNRParam(IspDev,&VpssU_obj);
		}
	
			char sensor_name[16];
	// obj_fps
			if(iso < ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true ){
				_hi_isp_cfg_set_slow_framerate(false,ispCfgAttr);
				
			}else if(SENSOR_MODEL_IMX225 == hi_isp_api_get_sensor_model(sensor_name) &&
				iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight] &&
				ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
				_hi_isp_cfg_set_slow_framerate2(true,ispCfgAttr);
			}else if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
				_hi_isp_cfg_set_slow_framerate(true,ispCfgAttr);
			}

	//obj_drc
	

	// obj_antiflicker
		if(AE_obj.stAuto.stAntiflicker.bEnable != (iso < ispCfgAttr->impCfgAttr.AntiFlikeThreshold ? HI_TRUE : HI_FALSE)){
			AE_obj.stAuto.stAntiflicker.bEnable = iso < ispCfgAttr->impCfgAttr.AntiFlikeThreshold ? HI_TRUE : HI_FALSE;
			AE_obj.stAuto.stAntiflicker.u8Frequency = ispCfgAttr->impCfgAttr.flick_frequency;
			SOC_CHECK(HI_MPI_ISP_SetExposureAttr(0,&AE_obj));
		}
		// DRC_obj
		
		static float iso_old = 0;
		int rank	 = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DrcAutoStrength_new[isDaylight]));
		float cur_iso = _isp_get_real_iso();
		if(cur_iso != iso_old)
		{
				DRC_obj.bEnable = (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE;
				DRC_obj.enOpType = OP_TYPE_MANUAL;
				if(ispCfgAttr->impCfgAttr.DrcAutoStrength_new[isDaylight][0] != 0)
				{
					int DRC_max,DRC_min;
					int iso_real[9] = {1,2,4,8,16,32,64,128,256};
					DRC_min = ispCfgAttr->impCfgAttr.DrcAutoStrength_new[isDaylight][rank];
					DRC_max = ispCfgAttr->impCfgAttr.DrcAutoStrength_new[isDaylight][rank+1];
					DRC_obj.stManual.u8Strength = LINE_COUNT(DRC_max,DRC_min,iso_real[rank+1],iso_real[rank],cur_iso);
				}
				SOC_CHECK(HI_MPI_ISP_SetDRCAttr(IspDev, &DRC_obj));
			
			iso_old = cur_iso;
		}
		return 0;
	}
	return -1;

}

int HI_ISP_cfg_set_ae_single(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	ISP_DEV IspDev = 0;
	ISP_EXPOSURE_ATTR_S AE_obj;
	ISP_EXP_INFO_S exp_obj;

	SOC_CHECK(HI_MPI_ISP_GetExposureAttr(IspDev, &AE_obj));
	SOC_CHECK( HI_MPI_ISP_QueryExposureInfo(IspDev, &exp_obj));

	static int exptime_old = 0;
	if(exp_obj.u32ExpTime != exptime_old)
	{
		if(exp_obj.u32ExpTime<21000&&exp_obj.u32ExpTime>0)
		{
			 AE_obj.stAuto.u8Compensation = 40;
			 SOC_CHECK( HI_MPI_ISP_SetExposureAttr(IspDev, &AE_obj));
		}
		else if(exp_obj.u32ExpTime>=45000 && exp_obj.u32ExpTime<70000)
		{
			AE_obj.stAuto.u8Compensation = 30;
			SOC_CHECK( HI_MPI_ISP_SetExposureAttr(IspDev, &AE_obj));
		}
		else if(exp_obj.u32ExpTime>=70000&&_isp_get_iso()>3)
		{
			AE_obj.stAuto.u8Compensation = 53;
			SOC_CHECK( HI_MPI_ISP_SetExposureAttr(IspDev, &AE_obj));
		}
		exptime_old = exp_obj.u32ExpTime;
	}
}




int HI_ISP_cfg_set_awb_single(int scene, LpIspCfgAttr ispCfgAttr)
{
	ISP_DEV IspDev = 0;
	ISP_COLOR_TONE_ATTR_S COLOR_obj;

	static int cnt_num_old = 0;
	int iso_real[9] = {1,2,4,8,16,32,64,128,256};
	int rank = count_from_iso(ARRAY_NUM(ispCfgAttr->awbCfgAttr.RedCastGain[scene]));
	float cur_iso;
  if(cnt_num_old != rank)
	{
	  int RED_max,RED_min;
	  cur_iso = _isp_get_real_iso();
	  RED_min = ispCfgAttr->awbCfgAttr.RedCastGain[scene][rank];
	  RED_max = ispCfgAttr->awbCfgAttr.RedCastGain[scene][rank+1];
	  COLOR_obj.u16RedCastGain = LINE_COUNT(RED_max,RED_min,iso_real[rank+1],iso_real[rank],cur_iso);
	  int Blue_max,Blue_min;
	  Blue_min = ispCfgAttr->awbCfgAttr.BlueCastGain[scene][rank];
	  Blue_max = ispCfgAttr->awbCfgAttr.BlueCastGain[scene][rank+1];
	  COLOR_obj.u16BlueCastGain = LINE_COUNT(Blue_max,Blue_min,iso_real[rank+1],iso_real[rank],cur_iso);
  
	  int Green_max,Green_min;
	  Green_min = ispCfgAttr->awbCfgAttr.GreenCastGain[scene][rank];
	  Green_max = ispCfgAttr->awbCfgAttr.GreenCastGain[scene][rank+1];
	  COLOR_obj.u16GreenCastGain = LINE_COUNT(Green_max,Green_min,iso_real[rank+1],iso_real[rank],cur_iso);
  
  
		  cnt_num_old = rank;
		  SOC_CHECK( HI_MPI_ISP_SetColorToneAttr(IspDev,&COLOR_obj));
	  }
}

int HI_ISP_cfg_get_imp(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;

	ISP_DEV IspDev = 0;

	ISP_SHARPEN_ATTR_S SHARPEN_obj;
	ISP_NR_ATTR_S NR_obj;
	ISP_DEMOSAIC_ATTR_S DEMOSAIC_obj;
	ISP_GAMMA_ATTR_S GAMMA_obj;
	ISP_DRC_ATTR_S DRC_obj;
	ISP_DP_DYNAMIC_ATTR_S DP_obj;
	VPSS_NR_PARAM_U   VpssU_obj ;

	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetSharpenAttr(IspDev, &SHARPEN_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetNRAttr(IspDev, &NR_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetGammaAttr(IspDev, &GAMMA_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDPDynamicAttr(IspDev, &DP_obj)\
				&& HI_SUCCESS == HI_MPI_VPSS_GetNRParam(IspDev,&VpssU_obj))
	{
	// SHARPEN_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltD[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenAltD[isDaylight][i] = SHARPEN_obj.stAuto.au8SharpenD[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltUd[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenAltUd[isDaylight][i] = SHARPEN_obj.stAuto.au8SharpenUd[i];
		}	

	// NR_obj	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrVarStrength[isDaylight]); i++)
		{			
			ispCfgAttr->impCfgAttr.SnrVarStrength[isDaylight][i] = NR_obj.stAuto.au8VarStrength[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrThreshold[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SnrThreshold[isDaylight][i] = NR_obj.stAuto.au16Threshold[i];
		}
	// DEMOSAIC_obj
		ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))] = DEMOSAIC_obj.u16UuSlope;
	// GAMMA_obj
		ispCfgAttr->impCfgAttr.Gamma[isDaylight] = GAMMA_obj.enCurveType;
	// VPSS_obj
//		ispCfgAttr->impCfgAttr.SfStrength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SfStrength[isDaylight]))] = VPSS_obj.s32GlobalStrength;
//		ispCfgAttr->impCfgAttr.GlobalStrength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.GlobalStrength[isDaylight]))] = VPSS_obj.s32GlobalStrength;

	// DRC_obj
		ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight] = 0;
		ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight] = DRC_obj.stAuto.u8Strength;
	// DP_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelSlope); i++)
		{
			ispCfgAttr->impCfgAttr.DefectPixelSlope[i] = DP_obj.stAuto.au16Slope[i];
		}	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelBlendRatio); i++)
		{
			ispCfgAttr->impCfgAttr.DefectPixelBlendRatio[i] = DP_obj.stAuto.au16BlendRatio[i];
		}	

	// VpssU_obj
		ispCfgAttr->impCfgAttr.YPKStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YPKStr[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YPKStr;
		ispCfgAttr->impCfgAttr.YSFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStr[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YSFStr;
		ispCfgAttr->impCfgAttr.YSFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStr[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YSFStr;
		ispCfgAttr->impCfgAttr.TFStrMax[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFStrMax[isDaylight]))] = VpssU_obj.stNRParam_V1.s32TFStrMax;
		ispCfgAttr->impCfgAttr.TFStrMov[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFStrMov[isDaylight]))] = VpssU_obj.stNRParam_V1.s32TFStrMov;
		ispCfgAttr->impCfgAttr.YSmthStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSmthStr[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YSmthStr;
		ispCfgAttr->impCfgAttr.YSmthRat[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSmthRat[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YSmthRat;
		ispCfgAttr->impCfgAttr.YSFStrDlt[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStrDlt[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YSFStrDlt;
		ispCfgAttr->impCfgAttr.YSFStrDl[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFStrDl[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YSFStrDl;
		ispCfgAttr->impCfgAttr.YTFStrDlt[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFStrDlt[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YTFStrDlt;
		ispCfgAttr->impCfgAttr.YTFStrDl[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFStrDl[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YTFStrDl;
		ispCfgAttr->impCfgAttr.YSFBriRat[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YSFBriRat[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YSFBriRat;
		ispCfgAttr->impCfgAttr.CSFStr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.CSFStr[isDaylight]))] = VpssU_obj.stNRParam_V1.s32CSFStr;
		ispCfgAttr->impCfgAttr.CTFstr[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.CTFstr[isDaylight]))] = VpssU_obj.stNRParam_V1.s32CTFstr;
		ispCfgAttr->impCfgAttr.YTFMdWin[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.YTFMdWin[isDaylight]))] = VpssU_obj.stNRParam_V1.s32YTFMdWin;


		return 0;
	}
	return -1;
}

int HI_ISP_cfg_set_all(int isDaylight, int scene, LpIspCfgAttr ispCfgAttr)
{
	if(NULL == ispCfgAttr){
		return -1;
	}

#ifdef INI_DEBUG_ECHO_OF_ALL
	return 0 == HI_ISP_cfg_set_ae(isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_set_awb(scene, ispCfgAttr) && 0 == HI_ISP_cfg_set_imp(isDaylight, ispCfgAttr) ? debug_register(isDaylight, scene) : -1;
#else
	return 0 == HI_ISP_cfg_set_ae(isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_set_awb(scene, ispCfgAttr) && 0 == HI_ISP_cfg_set_imp(isDaylight, ispCfgAttr) ? 0 : -1;
#endif
}

int HI_ISP_cfg_get_all(int isDaylight, int scene, LpIspCfgAttr ispCfgAttr)
{
	if(NULL == ispCfgAttr){
		return -1;
	}

#ifdef INI_DEBUG_ECHO_OF_ALL
	return 0 == HI_ISP_cfg_get_ae(isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_get_awb(scene, ispCfgAttr) && 0 == HI_ISP_cfg_get_imp(isDaylight, ispCfgAttr) ? debug_all(ispCfgAttr) : -1;
#else
	return 0 == HI_ISP_cfg_get_ae(isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_get_awb(scene, ispCfgAttr) && 0 == HI_ISP_cfg_get_imp(isDaylight, ispCfgAttr) ? 0 : -1;
#endif
}

int HI_ISP_cfg_load(const char *filepath, LpIspCfgAttr ispCfgAttr)
{
	if(NULL == ispCfgAttr){
		return -1;
	}

#ifdef INI_DEBUG_ECHO_OF_ALL
	return 0 == HI_ISP_cfg_ini_parse_init() ? 0 == HI_ISP_cfg_ini_load(filepath, ispCfgAttr, sizeof(StIspCfgAttr)) ? debug_all(ispCfgAttr) : -1 : -1;
#else
	return 0 == HI_ISP_cfg_ini_parse_init() ? HI_ISP_cfg_ini_load(filepath, ispCfgAttr, sizeof(StIspCfgAttr)) : -1;
#endif
}

