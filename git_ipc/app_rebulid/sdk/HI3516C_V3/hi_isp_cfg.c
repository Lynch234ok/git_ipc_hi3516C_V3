#include "../sdk_trace.h"

#include "hi_isp_cfg_parse.h"
#include "hi_isp_cfg.h"
#include "hi3516c.h"
#include "hi_isp_api.h"

#include <math.h>
extern const HI_U16 gs_Gamma[17][GAMMA_NODE_NUMBER];

static uint8_t _is_slow_framerate = 2;

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
		}else if(iso< 512){
			ret = 8;
		}else{
			ret=9;
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
	}
	else if(n >= 4){
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
	ECHO_HEX_DEC("AeSpeed", (int)ispCfgAttr->aeCfgAttr.AeSpeed);
	ECHO_HEX_DEC("AeTolerance", (int)ispCfgAttr->aeCfgAttr.AeTolerance);
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
	printf("\033[31;1m[IMP]\033[0m\n\n");
	ECHO_FOR_LOOP_2("SharpenD", i, j, ispCfgAttr->impCfgAttr.SharpenD);
	ECHO_FOR_LOOP_2("SharpenUd", i, j, ispCfgAttr->impCfgAttr.SharpenUd);
	ECHO_FOR_LOOP_2("OverShoot", i, j, ispCfgAttr->impCfgAttr.OverShoot);
	ECHO_FOR_LOOP_2("UnderShoot", i, j, ispCfgAttr->impCfgAttr.UnderShoot);
	ECHO_FOR_LOOP_2("EdgeThr", i, j, ispCfgAttr->impCfgAttr.EdgeThr);
	ECHO_FOR_LOOP_2("TextureThr", i, j, ispCfgAttr->impCfgAttr.TextureThr);
	ECHO_FOR_LOOP_2("SharpenEdge", i, j, ispCfgAttr->impCfgAttr.SharpenEdge);

	ECHO_FOR_LOOP_2("FineStr", i, j, ispCfgAttr->impCfgAttr.FineStr);
	ECHO_FOR_LOOP_2("CoringWeight", i, j, ispCfgAttr->impCfgAttr.CoringWeight);
	
	ECHO_FOR_LOOP_2("EdgeSmoothThr", i, j, ispCfgAttr->impCfgAttr.EdgeSmoothThr);
	ECHO_FOR_LOOP_2("EdgeSmoothSlope", i, j, ispCfgAttr->impCfgAttr.EdgeSmoothSlope);
	ECHO_FOR_LOOP_2("AntiAliasThr", i, j, ispCfgAttr->impCfgAttr.AntiAliasThr);
	ECHO_FOR_LOOP_2("AntiAliasSlope", i, j, ispCfgAttr->impCfgAttr.AntiAliasSlope);
	ECHO_FOR_LOOP_2("NrCoarseStr", i, j, ispCfgAttr->impCfgAttr.NrCoarseStr);
	ECHO_FOR_LOOP_2("DetailEnhanceStr", i, j, ispCfgAttr->impCfgAttr.DetailEnhanceStr);
	ECHO_FOR_LOOP_2("NoiseSuppressStr", i, j, ispCfgAttr->impCfgAttr.NoiseSuppressStr);
	ECHO_FOR_LOOP_2("SharpenLumaStr", i, j, ispCfgAttr->impCfgAttr.SharpenLumaStr);
	
	ECHO_FOR_LOOP_0("Gamma", i, ispCfgAttr->impCfgAttr.Gamma);
	ECHO_FOR_LOOP_0("DrcThreshold", i, ispCfgAttr->impCfgAttr.DrcThreshold);
	ECHO_FOR_LOOP_0("DrcAutoStrength", i, ispCfgAttr->impCfgAttr.DrcAutoStrength);
	
	ECHO_FOR_LOOP_1("DefectPixelStrength", i, ispCfgAttr->impCfgAttr.DefectPixelStrength);
	ECHO_FOR_LOOP_1("DefectPixelBlendRatio", i, ispCfgAttr->impCfgAttr.DefectPixelBlendRatio);
	
	ECHO_FOR_LOOP_0("SlowFrameRateHighHold", i, ispCfgAttr->impCfgAttr.SlowFrameRateHighHold);
	ECHO_FOR_LOOP_0("SlowFrameRateLowHold", i, ispCfgAttr->impCfgAttr.SlowFrameRateLowHold);
	ECHO_HEX_DEC("AntiFlikeThreshold", (int)ispCfgAttr->impCfgAttr.AntiFlikeThreshold);	

	ECHO_FOR_LOOP_2("IES0", i, j, ispCfgAttr->impCfgAttr.IES0);
	ECHO_FOR_LOOP_2("SBS0", i, j, ispCfgAttr->impCfgAttr.SBS0);
	ECHO_FOR_LOOP_2("SBS1", i, j, ispCfgAttr->impCfgAttr.SBS1);
	ECHO_FOR_LOOP_2("SBS2", i, j, ispCfgAttr->impCfgAttr.SBS2);
	ECHO_FOR_LOOP_2("SBS3", i, j, ispCfgAttr->impCfgAttr.SBS3);
	ECHO_FOR_LOOP_2("SDS0", i, j, ispCfgAttr->impCfgAttr.SDS0);
	ECHO_FOR_LOOP_2("SDS1", i, j, ispCfgAttr->impCfgAttr.SDS1);
	ECHO_FOR_LOOP_2("SDS2", i, j, ispCfgAttr->impCfgAttr.SDS2);
	ECHO_FOR_LOOP_2("SDS3", i, j, ispCfgAttr->impCfgAttr.SDS3);
	ECHO_FOR_LOOP_2("STH0", i, j, ispCfgAttr->impCfgAttr.STH0);
	ECHO_FOR_LOOP_2("STH1", i, j, ispCfgAttr->impCfgAttr.STH1);
	ECHO_FOR_LOOP_2("STH2", i, j, ispCfgAttr->impCfgAttr.STH2);
	ECHO_FOR_LOOP_2("STH3", i, j, ispCfgAttr->impCfgAttr.STH3);
	ECHO_FOR_LOOP_2("MDP", i, j, ispCfgAttr->impCfgAttr.MDP);
	ECHO_FOR_LOOP_2("MATH1", i, j, ispCfgAttr->impCfgAttr.MATH1);

	ECHO_FOR_LOOP_2("MATH2", i, j, ispCfgAttr->impCfgAttr.MATH2);
	ECHO_FOR_LOOP_2("Pro3", i, j, ispCfgAttr->impCfgAttr.Pro3);
	ECHO_FOR_LOOP_2("MDDZ1", i, j, ispCfgAttr->impCfgAttr.MDDZ1);
	ECHO_FOR_LOOP_2("MDDZ2", i, j, ispCfgAttr->impCfgAttr.MDDZ2);
	ECHO_FOR_LOOP_2("TFS1", i, j, ispCfgAttr->impCfgAttr.TFS1);
	ECHO_FOR_LOOP_2("TFS2", i, j, ispCfgAttr->impCfgAttr.TFS2);

	ECHO_FOR_LOOP_2("SFC", i, j, ispCfgAttr->impCfgAttr.SFC);
	ECHO_FOR_LOOP_2("TFC", i, j, ispCfgAttr->impCfgAttr.TFC);
	ECHO_FOR_LOOP_2("TPC", i, j, ispCfgAttr->impCfgAttr.TPC);
	ECHO_FOR_LOOP_2("TRC", i, j, ispCfgAttr->impCfgAttr.TRC);

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
	VPSS_GRP_NRX_PARAM_S  VpssU_obj;
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
			&& HI_SUCCESS == HI_MPI_VPSS_GetGrpNRXParam(IspDev, &VpssU_obj) \
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
		ECHO_FOR_LOOP_1("au16SharpenUd", i, SHARPEN_obj.stAuto.au16SharpenUd);		
		ECHO_FOR_LOOP_1("au8OverShoot", i, SHARPEN_obj.stAuto.au8OverShoot);
		ECHO_FOR_LOOP_1("au8UnderShoot", i, SHARPEN_obj.stAuto.au8UnderShoot);
		ECHO_FOR_LOOP_1("au8TextureThr", i, SHARPEN_obj.stAuto.au8TextureThr);
		ECHO_FOR_LOOP_1("au8shootSupStr", i, SHARPEN_obj.stAuto.au8shootSupStr);	
		ECHO_FOR_LOOP_1("au8SharpenEdge", i, SHARPEN_obj.stAuto.au8SharpenEdge);
		ECHO_FOR_LOOP_1("au8EdgeThr", i, SHARPEN_obj.stAuto.au8EdgeThr);
		ECHO_FOR_LOOP_1("au8DetailCtrl", i, SHARPEN_obj.stAuto.au8DetailCtrl);

	// NR_obj
		printf("\033[31;1m[SNR]\033[0m\n");
		ECHO_FOR_LOOP_1("au8FineStr", i, NR_obj.stAuto.au8FineStr);
		ECHO_FOR_LOOP_1("au16CoringWeight", i, NR_obj.stAuto.au16CoringWeight);
	// DEMOSAIC_obj
		printf("\033[31;1m[DEMOSAIC]\033[0m\n");
		ECHO_HEX_DEC("au16EdgeSmoothThr", DEMOSAIC_obj.stAuto.au16EdgeSmoothThr);
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
		ECHO_HEX_DEC("au16Strength", DP_obj.stAuto.au16Strength);
		ECHO_HEX_DEC("au16BlendRatio", DP_obj.stAuto.au16BlendRatio);
#if 0
	// VPSS_U_obj
		printf("\033[31;1m[VPSS]\033[0m\n");
		ECHO_HEX_DEC("IES0", VpssU_obj.stNRSParam_V2.IES0);
		ECHO_HEX_DEC("SBS0", VpssU_obj.stNRSParam_V2.SBS0);
		ECHO_HEX_DEC("SBS1", VpssU_obj.stNRSParam_V2.SBS1);
		ECHO_HEX_DEC("SBS2", VpssU_obj.stNRSParam_V2.SBS2);
		ECHO_HEX_DEC("SBS3", VpssU_obj.stNRSParam_V2.SBS3);
		ECHO_HEX_DEC("SDS0", VpssU_obj.stNRSParam_V2.SDS0);
		ECHO_HEX_DEC("SDS1", VpssU_obj.stNRSParam_V2.SDS1);
		ECHO_HEX_DEC("SDS2", VpssU_obj.stNRSParam_V2.SDS2);
		ECHO_HEX_DEC("SDS3", VpssU_obj.stNRSParam_V2.SDS3);
		ECHO_HEX_DEC("STH0", VpssU_obj.stNRSParam_V2.STH0);
		ECHO_HEX_DEC("STH1", VpssU_obj.stNRSParam_V2.STH1);
		ECHO_HEX_DEC("STH2", VpssU_obj.stNRSParam_V2.STH2);
		ECHO_HEX_DEC("STH3", VpssU_obj.stNRSParam_V2.STH3);
		ECHO_HEX_DEC("MDP", VpssU_obj.stNRSParam_V2.MDP);
		ECHO_HEX_DEC("MATH1", VpssU_obj.stNRSParam_V2.MATH1);

		ECHO_HEX_DEC("MATH2", VpssU_obj.stNRSParam_V2.MATH2);
		ECHO_HEX_DEC("Pro3", VpssU_obj.stNRSParam_V2.Pro3);
		ECHO_HEX_DEC("MDDZ1", VpssU_obj.stNRSParam_V2.MDDZ1);
		ECHO_HEX_DEC("MDDZ2", VpssU_obj.stNRSParam_V2.MDDZ2);
		ECHO_HEX_DEC("TFS1", VpssU_obj.stNRSParam_V2.TFS1);
		ECHO_HEX_DEC("TFS2", VpssU_obj.stNRSParam_V2.TFS2);
		ECHO_HEX_DEC("SFC", VpssU_obj.stNRSParam_V2.SFC);
		ECHO_HEX_DEC("TFC", VpssU_obj.stNRSParam_V2.TFC);
		ECHO_HEX_DEC("TPC", VpssU_obj.stNRSParam_V2.TPC);
		ECHO_HEX_DEC("TRC", VpssU_obj.stNRSParam_V2.TRC);
#endif	
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

static int _hi_isp_cfg_set_slow_framerate(uint8_t bValue,LpIspCfgAttr ispCfgAttr)
{
	char sensor_name[16];
	if(SENSOR_MODEL_APTINA_AR0237 == hi_isp_api_get_sensor_model(sensor_name)){
			
		uint8_t Value = bValue ? 0x1:0x0;
		VENC_CHN_ATTR_S vencChannelAttr;
		ISP_PUB_ATTR_S stPubAttr;	
		int actual_fps;
		uint8_t  frame_rate;	
		int i = 0;
		uint8_t pub_frame_rate;
		
		frame_rate = ispCfgAttr->impCfgAttr.src_framerate;
			
		SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));
		pub_frame_rate = (uint8_t)stPubAttr.f32FrameRate;
		
		for(i = 0; i <= 1; i++){
		
			HI_MPI_VENC_GetChnAttr(i,&vencChannelAttr);
		//	printf("old_FRate = %f\n",stPubAttr.f32FrameRate);
		
			actual_fps = frame_rate/(1 << Value);
			if(vencChannelAttr.stVeAttr.enType == PT_H264)
			{
				switch(vencChannelAttr.stRcAttr.enRcMode){
					default:
					case VENC_RC_MODE_H264AVBR:
						if(bValue == true && pub_frame_rate == frame_rate){//slow framerate 		
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264AVbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH264AVbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264AVbr.fr32DstFrmRate;
							}else{
								vencChannelAttr.stRcAttr.stAttrH264AVbr.u32SrcFrmRate = actual_fps;
							}	
							HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
							SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
							_is_slow_framerate = 1;
							printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
		
						}else if (bValue == false && pub_frame_rate < frame_rate ){//actual framerate	
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264AVbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH264AVbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264AVbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH264AVbr.u32SrcFrmRate = actual_fps;
							}
							HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
							SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
							_is_slow_framerate = 0;
							printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
						}else{
						}
						break;
					
					case VENC_RC_MODE_H264CBR:
						if(bValue == true && pub_frame_rate == frame_rate){//slow framerate
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
							}else{
								vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =	actual_fps;
							}	
							HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
							SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
							_is_slow_framerate = 1;
							printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
		
						}else if(bValue == false && pub_frame_rate < frame_rate){//actual framerate
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =	actual_fps;
							}
							HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
							SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
							_is_slow_framerate = 0;
							printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
						}else{
						}
						break;		
					}
				}
				else if(vencChannelAttr.stVeAttr.enType ==	PT_H265)
				{
					switch(vencChannelAttr.stRcAttr.enRcMode){
					default:
					case VENC_RC_MODE_H265AVBR:
						if(bValue == true && pub_frame_rate == frame_rate){//slow framerate
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate = actual_fps;
							}	
							HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
							SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
							_is_slow_framerate = 1;
		
						}else if(bValue == false && pub_frame_rate < frame_rate){//actual framerate
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate = actual_fps;
							}
							HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
							SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
							_is_slow_framerate = 0;
						}else{
						}
						break;
					
					case VENC_RC_MODE_H265CBR:
						if(bValue == true && pub_frame_rate == frame_rate){//slow framerate
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate =	actual_fps;
							}		
							HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
							SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
							_is_slow_framerate = 1;
		
						}else if(bValue == false && pub_frame_rate < frame_rate){//actual framerate 
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate =	actual_fps;
							}
							HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
							SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
							_is_slow_framerate = 0;
						}else{
						}
						break;		
					}
				}
				else{
				}
		}

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
		if(ispCfgAttr->aeCfgAttr.StarLightModeEnable){
			if(ispCfgAttr->aeCfgAttr.LowLightModeEnable){
				if(ispCfgAttr->aeCfgAttr.LowLightExptimeFlag && ispCfgAttr->aeCfgAttr.GainThresholdFlag){
					AE_obj.stAuto.stExpTimeRange.u32Max = ispCfgAttr->aeCfgAttr.LowLightMaxExptime[isDaylight];
					AE_obj.stAuto.u32GainThreshold = ispCfgAttr->aeCfgAttr.GainThreshold[isDaylight];
				}
			}else{
				if(ispCfgAttr->aeCfgAttr.StarMaxExptimeFlag && ispCfgAttr->aeCfgAttr.GainThresholdFlag){
					AE_obj.stAuto.stExpTimeRange.u32Max = ispCfgAttr->aeCfgAttr.StarMaxExptime[isDaylight];
					AE_obj.stAuto.u32GainThreshold = ispCfgAttr->aeCfgAttr.GainThreshold[isDaylight];
				}
			}
		}else{
			if(ispCfgAttr->aeCfgAttr.MaxExptimeFlag){
				AE_obj.stAuto.stExpTimeRange.u32Max = ispCfgAttr->aeCfgAttr.MaxExptime[isDaylight];
			}
		}
		
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
		ispCfgAttr->aeCfgAttr.StarMaxExptime[isDaylight] = AE_obj.stAuto.stExpTimeRange.u32Max;	
		ispCfgAttr->aeCfgAttr.MaxExptime[isDaylight] = AE_obj.stAuto.stExpTimeRange.u32Max;
		ispCfgAttr->aeCfgAttr.GainThreshold[isDaylight] = AE_obj.stAuto.u32GainThreshold;	
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

int HI_ISP_cfg_set_awb(int scene, int isDaylight, LpIspCfgAttr ispCfgAttr)
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

		if(0 == isDaylight) {
			AWB_obj.enOpType = OP_TYPE_AUTO;
		}
		else {
			AWB_obj.enOpType = OP_TYPE_MANUAL;
		}

		return HI_SUCCESS == HI_MPI_ISP_SetCCMAttr(IspDev, &CCM_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetWBAttr(IspDev, &AWB_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetSaturationAttr(IspDev, &SATURATION_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetBlackLevelAttr(IspDev, &BLACK_LEVEL_obj) ? 0 : -1;
	}
	return -1;
}

int HI_ISP_cfg_get_awb(int scene, int isDaylight, LpIspCfgAttr ispCfgAttr)
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
	VPSS_GRP_NRX_PARAM_S  VpssU_obj;

	VpssU_obj.enNRVer = VPSS_NR_V2;//select the VPSS_NR_V2 mode
	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetSharpenAttr(IspDev, &SHARPEN_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetNRAttr(IspDev, &NR_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetGammaAttr(IspDev, &GAMMA_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDPDynamicAttr(IspDev, &DP_obj) \
				&& HI_SUCCESS == HI_MPI_VPSS_GetGrpNRXParam(IspDev,&VpssU_obj))
	{

	// SHARPEN_obj
	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenD[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8SharpenD[i] = ispCfgAttr->impCfgAttr.SharpenD[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenUd[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au16SharpenUd[i] = ispCfgAttr->impCfgAttr.SharpenUd[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.OverShoot[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8OverShoot[i] = ispCfgAttr->impCfgAttr.OverShoot[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.UnderShoot[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8UnderShoot[i] = ispCfgAttr->impCfgAttr.UnderShoot[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.TextureThr[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8TextureThr[i] = ispCfgAttr->impCfgAttr.TextureThr[isDaylight][i];
		}

		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeThr[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8EdgeThr[i] = ispCfgAttr->impCfgAttr.EdgeThr[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenEdge[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8SharpenEdge[i] = ispCfgAttr->impCfgAttr.SharpenEdge[isDaylight][i];
		}
		
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.ShootSupStr[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8shootSupStr[i] = ispCfgAttr->impCfgAttr.ShootSupStr[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DetailCtrl[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8DetailCtrl[i] = ispCfgAttr->impCfgAttr.DetailCtrl[isDaylight][i];
		}

		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeFiltStr[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8EdgeFiltStr[i] = ispCfgAttr->impCfgAttr.EdgeFiltStr[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.JagCtrl[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8JagCtrl[i] = ispCfgAttr->impCfgAttr.JagCtrl[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.NoiseLumaCtrl[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8NoiseLumaCtrl[i] = ispCfgAttr->impCfgAttr.NoiseLumaCtrl[isDaylight][i];
		}
										
	// NR_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.FineStr[isDaylight]); i++)
		{
			NR_obj.stAuto.au8FineStr[i] = ispCfgAttr->impCfgAttr.FineStr[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.CoringWeight[isDaylight]); i++)
		{
			NR_obj.stAuto.au16CoringWeight[i] = ispCfgAttr->impCfgAttr.CoringWeight[isDaylight][i];
		}
		
		
	// DEMOSAIC_obj
	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeSmoothThr[isDaylight]); i++)
		{
			DEMOSAIC_obj.stAuto.au16EdgeSmoothThr[i] = ispCfgAttr->impCfgAttr.EdgeSmoothThr[isDaylight][i];

		}
		
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeSmoothSlope[isDaylight]); i++)
		{
			DEMOSAIC_obj.stAuto.au16EdgeSmoothSlope[i] = ispCfgAttr->impCfgAttr.EdgeSmoothSlope[isDaylight][i];
		}
		
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.AntiAliasThr[isDaylight]); i++)
		{
			DEMOSAIC_obj.stAuto.au16AntiAliasThr[i] = ispCfgAttr->impCfgAttr.AntiAliasThr[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.AntiAliasSlope[isDaylight]); i++)
		{
			DEMOSAIC_obj.stAuto.au16AntiAliasSlope[i] = ispCfgAttr->impCfgAttr.AntiAliasSlope[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.NrCoarseStr[isDaylight]); i++)
		{
			DEMOSAIC_obj.stAuto.au16NrCoarseStr[i] = ispCfgAttr->impCfgAttr.NrCoarseStr[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DetailEnhanceStr[isDaylight]); i++)
		{
			DEMOSAIC_obj.stAuto.au8DetailEnhanceStr[i] = ispCfgAttr->impCfgAttr.DetailEnhanceStr[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.NoiseSuppressStr[isDaylight]); i++)
		{
			DEMOSAIC_obj.stAuto.au16NoiseSuppressStr[i] = ispCfgAttr->impCfgAttr.NoiseSuppressStr[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenLumaStr[isDaylight]); i++)
		{
			DEMOSAIC_obj.stAuto.au16SharpenLumaStr[i] = ispCfgAttr->impCfgAttr.SharpenLumaStr[isDaylight][i];
		}
		
	// GAMMA_obj
		GAMMA_obj.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
		for(i = 0; i < ARRAY_NUM(gs_Gamma[ispCfgAttr->impCfgAttr.Gamma[isDaylight]]); i++)
		{
			GAMMA_obj.u16Table[i] = gs_Gamma[ispCfgAttr->impCfgAttr.Gamma[isDaylight]][i];
		}
		
	// DRC_obj
#if defined(HI3516C_V3)	
		ISP_WDR_MODE_S stWdrMode;
		HI_MPI_ISP_GetWDRMode(0, &stWdrMode);
		if (stWdrMode.enWDRMode)  //wdr mode
		{		
			DRC_obj.bEnable = (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE;
			DRC_obj.enOpType = OP_TYPE_AUTO;//OP_TYPE_MANUAL;
			DRC_obj.stAuto.u8Strength = ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight];//205
		}else{
			DRC_obj.bEnable = HI_FALSE;
		}
#elif defined(HI3516E_V1)
		DRC_obj.bEnable  = HI_TRUE;
		DRC_obj.enOpType = OP_TYPE_MANUAL;
		DRC_obj.stManual.u8Strength = ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight][count_from_iso(16)];
#endif

		 
	// DP_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelStrength); i++)
		{
			DP_obj.stAuto.au16Strength[i] = ispCfgAttr->impCfgAttr.DefectPixelStrength[i];
		}
		
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelBlendRatio); i++)
		{
			DP_obj.stAuto.au16BlendRatio[i]= ispCfgAttr->impCfgAttr.DefectPixelBlendRatio[i];
		}
#if 0
	// VPSS_U
		VpssU_obj.stNRSParam_V2.IES0 = ispCfgAttr->impCfgAttr.IES0[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IES0[isDaylight]))];
		VpssU_obj.stNRSParam_V2.SBS0 = ispCfgAttr->impCfgAttr.SBS0[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS0[isDaylight]))];
		VpssU_obj.stNRSParam_V2.SBS1 = ispCfgAttr->impCfgAttr.SBS1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS1[isDaylight]))];
		VpssU_obj.stNRSParam_V2.SBS2 = ispCfgAttr->impCfgAttr.SBS2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS2[isDaylight]))];
		VpssU_obj.stNRSParam_V2.SBS3 = ispCfgAttr->impCfgAttr.SBS3[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS3[isDaylight]))];
		VpssU_obj.stNRSParam_V2.SDS0 = ispCfgAttr->impCfgAttr.SDS0[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IES0[isDaylight]))];
		VpssU_obj.stNRSParam_V2.SDS1 = ispCfgAttr->impCfgAttr.SDS1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS1[isDaylight]))];
		VpssU_obj.stNRSParam_V2.SDS2 = ispCfgAttr->impCfgAttr.SDS2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS2[isDaylight]))];
		VpssU_obj.stNRSParam_V2.SDS3 = ispCfgAttr->impCfgAttr.SDS3[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS3[isDaylight]))];
		VpssU_obj.stNRSParam_V2.STH0 = ispCfgAttr->impCfgAttr.STH0[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IES0[isDaylight]))];
		VpssU_obj.stNRSParam_V2.STH1 = ispCfgAttr->impCfgAttr.STH1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH1[isDaylight]))];
		VpssU_obj.stNRSParam_V2.STH2 = ispCfgAttr->impCfgAttr.STH2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH2[isDaylight]))];
		VpssU_obj.stNRSParam_V2.STH3 = ispCfgAttr->impCfgAttr.STH3[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH3[isDaylight]))];
		
		VpssU_obj.stNRSParam_V2.MDP = ispCfgAttr->impCfgAttr.MDP[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDP[isDaylight]))];
		VpssU_obj.stNRSParam_V2.MATH1 = ispCfgAttr->impCfgAttr.MATH1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MATH1[isDaylight]))];
		VpssU_obj.stNRSParam_V2.MATH2 = ispCfgAttr->impCfgAttr.MATH2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MATH2[isDaylight]))];
		VpssU_obj.stNRSParam_V2.Pro3 = ispCfgAttr->impCfgAttr.Pro3[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Pro3[isDaylight]))];
		VpssU_obj.stNRSParam_V2.MDDZ1 = ispCfgAttr->impCfgAttr.MDDZ1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDDZ1[isDaylight]))];
		VpssU_obj.stNRSParam_V2.MDDZ2 = ispCfgAttr->impCfgAttr.MDDZ2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDDZ2[isDaylight]))];
		VpssU_obj.stNRSParam_V2.TFS1 = ispCfgAttr->impCfgAttr.TFS1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFS1[isDaylight]))];
		VpssU_obj.stNRSParam_V2.TFS2 = ispCfgAttr->impCfgAttr.TFS2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFS2[isDaylight]))];

		VpssU_obj.stNRSParam_V2.SFC = ispCfgAttr->impCfgAttr.SFC[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SFC[isDaylight]))];
		VpssU_obj.stNRSParam_V2.TFC = ispCfgAttr->impCfgAttr.TFC[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFC[isDaylight]))];
		VpssU_obj.stNRSParam_V2.TPC = ispCfgAttr->impCfgAttr.TPC[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TPC[isDaylight]))];
		VpssU_obj.stNRSParam_V2.TRC = ispCfgAttr->impCfgAttr.TRC[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TRC[isDaylight]))];
#endif
	// obj_fps	
		if(iso < ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			_hi_isp_cfg_set_slow_framerate(false,ispCfgAttr);
			
		}else if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			_hi_isp_cfg_set_slow_framerate(true,ispCfgAttr);
		}

		return HI_SUCCESS == HI_MPI_ISP_SetSharpenAttr(IspDev, &SHARPEN_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetNRAttr(IspDev, &NR_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetGammaAttr(IspDev, &GAMMA_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_SetDPDynamicAttr(IspDev, &DP_obj) \
				&& HI_SUCCESS == HI_MPI_VPSS_SetGrpNRXParam(IspDev,&VpssU_obj)\
				&& HI_SUCCESS == HI_MPI_ISP_SetDemosaicAttr(IspDev, &DEMOSAIC_obj)? 0 : -1;
				
	}

	return -1;
}


int HI_ISP_cfg_set_imp_single(int isDaylight, LpIspCfgAttr ispCfgAttr)
{

	ISP_DEV IspDev = 0;
	int iso = _isp_get_iso();

	ISP_DEMOSAIC_ATTR_S DEMOSAIC_obj;
	VPSS_GRP_NRX_PARAM_S  VpssU_obj;
	ISP_DRC_ATTR_S DRC_obj;
	ISP_EXPOSURE_ATTR_S AE_obj; 
	VpssU_obj.enNRVer = VPSS_NR_V2;//select the VPSS_NR_V2 mode
 	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_VPSS_GetGrpNRXParam(IspDev,&VpssU_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetExposureAttr(IspDev,&AE_obj))
	{

#if 0
	// VpssU_obj
		static int cnt_num_old = 0;
		int cur_cnt_num = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IES0[isDaylight]));
		int vpss_max,vpss_min,rank;
		float cur_iso = _isp_get_real_iso();
		int iso_real[10] = {1,2,4,8,16,32,64,128,256,512,1024};	
		if(cnt_num_old != cur_cnt_num){		
			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IES0[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.IES0[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.IES0[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.IES0 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS0[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SBS0[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SBS0[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SBS0 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS1[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SBS1[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SBS1[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SBS1 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS2[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SBS2[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SBS2[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SBS2 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS3[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SBS3[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SBS3[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SBS3 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS0[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SDS0[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SDS0[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SDS0 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS1[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SDS1[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SDS1[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SDS1 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS2[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SDS2[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SDS2[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SDS2 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS3[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SDS3[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SDS3[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SDS3 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH0[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.STH0[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.STH0[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.STH0 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH1[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.STH1[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.STH1[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.STH1 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH2[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.STH2[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.STH2[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.STH2 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH3[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.STH3[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.STH3[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.STH3 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDP[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.MDP[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.MDP[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.MDP = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MATH1[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.MATH1[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.MATH1[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.MATH1 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MATH2[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.MATH2[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.MATH2[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.MATH2 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Pro3[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.Pro3[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.Pro3[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.Pro3 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDDZ1[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.MDDZ1[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.MDDZ1[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.MDDZ1 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDDZ2[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.MDDZ2[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.MDDZ2[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.MDDZ2 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFS1[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.TFS1[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.TFS1[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.TFS1 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFS2[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.TFS2[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.TFS2[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.TFS2 = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SFC[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.SFC[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.SFC[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.SFC = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFC[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.TFC[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.TFC[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.TFC = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TPC[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.TPC[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.TPC[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.TPC = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);

			rank = count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TRC[isDaylight]));
			vpss_min = ispCfgAttr->impCfgAttr.TRC[isDaylight][rank];
			vpss_max = ispCfgAttr->impCfgAttr.TRC[isDaylight][rank+1];
			VpssU_obj.stNRSParam_V2.TRC = LINE_COUNT(vpss_max,vpss_min,iso_real[rank+1],iso_real[rank],cur_iso);	
	//		printf("HI_MPI_VPSS_SetGrpNRSParam\r\n");
			cnt_num_old = cur_cnt_num;
			HI_MPI_VPSS_SetGrpNRXParam(IspDev,&VpssU_obj);
		}
#endif
	
	// obj_fps
		if(iso < ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			if(0  != _is_slow_framerate){
				_hi_isp_cfg_set_slow_framerate(false, ispCfgAttr);
			}
		}else if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			if(1 != _is_slow_framerate){
				_hi_isp_cfg_set_slow_framerate(true, ispCfgAttr);
			}
		}

	//obj_drc
#if defined(HI3516C_V3)	
		if(DRC_obj.bEnable != ((iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE))
		{
			ISP_WDR_MODE_S stWdrMode;
			HI_MPI_ISP_GetWDRMode(0, &stWdrMode);
			if (stWdrMode.enWDRMode)  //wdr mode
			{		
				DRC_obj.bEnable = (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE;
				DRC_obj.enOpType = OP_TYPE_AUTO;//OP_TYPE_MANUAL;
				DRC_obj.stAuto.u8Strength = ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight];//205
			}else{
				DRC_obj.bEnable = HI_FALSE;
			}
			HI_MPI_ISP_SetDRCAttr(IspDev, &DRC_obj);
		}
#elif defined(HI3516E_V1)
		{
			DRC_obj.bEnable  = HI_TRUE;
			DRC_obj.enOpType = OP_TYPE_MANUAL;
			DRC_obj.stManual.u8Strength = ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight][count_from_iso(16)];
			HI_MPI_ISP_SetDRCAttr(IspDev, &DRC_obj);
		}
#endif

	// obj_antiflicker
		if(AE_obj.stAuto.stAntiflicker.bEnable != (iso < ispCfgAttr->impCfgAttr.AntiFlikeThreshold ? HI_TRUE : HI_FALSE)){
			AE_obj.stAuto.stAntiflicker.bEnable = iso < ispCfgAttr->impCfgAttr.AntiFlikeThreshold ? HI_TRUE : HI_FALSE;
			AE_obj.stAuto.stAntiflicker.u8Frequency = ispCfgAttr->impCfgAttr.flick_frequency;
			SOC_CHECK(HI_MPI_ISP_SetExposureAttr(0,&AE_obj));
		}

		return 0;
	}
	return -1;

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
	VPSS_GRP_NRX_PARAM_S  VpssU_obj;
	
	VpssU_obj.enNRVer = VPSS_NR_V2;//select the VPSS_NR_V2 mode
	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetSharpenAttr(IspDev, &SHARPEN_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetNRAttr(IspDev, &NR_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetGammaAttr(IspDev, &GAMMA_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDPDynamicAttr(IspDev, &DP_obj)\
				&& HI_SUCCESS == HI_MPI_VPSS_GetGrpNRXParam(IspDev,&VpssU_obj))
	{
	// SHARPEN_obj
	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenD[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenD[isDaylight][i] = SHARPEN_obj.stAuto.au8SharpenD[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenUd[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenUd[isDaylight][i] = SHARPEN_obj.stAuto.au16SharpenUd[i];
		}	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.OverShoot[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.OverShoot[isDaylight][i] = SHARPEN_obj.stAuto.au8OverShoot[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.UnderShoot[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.UnderShoot[isDaylight][i] = SHARPEN_obj.stAuto.au8UnderShoot[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeThr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.EdgeThr[isDaylight][i] = SHARPEN_obj.stAuto.au8EdgeThr[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.TextureThr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.TextureThr[isDaylight][i] = SHARPEN_obj.stAuto.au8TextureThr[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenEdge[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenEdge[isDaylight][i] = SHARPEN_obj.stAuto.au8SharpenEdge[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.ShootSupStr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.ShootSupStr[isDaylight][i] = SHARPEN_obj.stAuto.au8shootSupStr[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DetailCtrl[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.DetailCtrl[isDaylight][i] = SHARPEN_obj.stAuto.au8DetailCtrl[i];
		}

		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeFiltStr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.EdgeFiltStr[isDaylight][i] = SHARPEN_obj.stAuto.au8EdgeFiltStr[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.JagCtrl[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.JagCtrl[isDaylight][i] = SHARPEN_obj.stAuto.au8JagCtrl[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.NoiseLumaCtrl[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.NoiseLumaCtrl[isDaylight][i] = SHARPEN_obj.stAuto.au8NoiseLumaCtrl[i];
		}

	// NR_obj	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.FineStr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.FineStr[isDaylight][i] = NR_obj.stAuto.au8FineStr[i] ;
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.CoringWeight[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.CoringWeight[isDaylight][i] = NR_obj.stAuto.au16CoringWeight[i] ;
		}
		
	// DEMOSAIC_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeSmoothThr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.EdgeSmoothThr[isDaylight][i] =DEMOSAIC_obj.stAuto.au16EdgeSmoothSlope[i];
		}	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.EdgeSmoothSlope[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.EdgeSmoothSlope[isDaylight][i] =DEMOSAIC_obj.stAuto.au16EdgeSmoothSlope[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.AntiAliasThr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.AntiAliasThr[isDaylight][i] =DEMOSAIC_obj.stAuto.au16AntiAliasThr[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.AntiAliasSlope[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.AntiAliasSlope[isDaylight][i] =DEMOSAIC_obj.stAuto.au16AntiAliasSlope[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.NrCoarseStr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.NrCoarseStr[isDaylight][i] =DEMOSAIC_obj.stAuto.au16NrCoarseStr[i];
		}	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DetailEnhanceStr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.DetailEnhanceStr[isDaylight][i] =DEMOSAIC_obj.stAuto.au8DetailEnhanceStr[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.NoiseSuppressStr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.NoiseSuppressStr[isDaylight][i] =DEMOSAIC_obj.stAuto.au16NoiseSuppressStr[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenLumaStr[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenLumaStr[isDaylight][i] =DEMOSAIC_obj.stAuto.au16SharpenLumaStr[i];
		}
		
	// GAMMA_obj
		ispCfgAttr->impCfgAttr.Gamma[isDaylight] = GAMMA_obj.enCurveType;
	// VPSS_obj
//		ispCfgAttr->impCfgAttr.SfStrength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SfStrength[isDaylight]))] = VPSS_obj.s32GlobalStrength;
//		ispCfgAttr->impCfgAttr.GlobalStrength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.GlobalStrength[isDaylight]))] = VPSS_obj.s32GlobalStrength;

	// DRC_obj
		ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight] = 0;
		ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight][count_from_iso(16)] = DRC_obj.stAuto.u8Strength;

	// DP_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelStrength); i++)
		{
			ispCfgAttr->impCfgAttr.DefectPixelStrength[i] = DP_obj.stAuto.au16Strength[i];
		}	
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelBlendRatio); i++)
		{
			ispCfgAttr->impCfgAttr.DefectPixelBlendRatio[i] = DP_obj.stAuto.au16BlendRatio[i];
		}	
#if 0
	// VpssU_obj
		ispCfgAttr->impCfgAttr.IES0[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IES0[isDaylight]))]   = VpssU_obj.stNRSParam_V2.IES0 ;
		ispCfgAttr->impCfgAttr.SBS0[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS0[isDaylight]))]  = VpssU_obj.stNRSParam_V2.SBS0 ;
		ispCfgAttr->impCfgAttr.SBS1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS1[isDaylight]))] = VpssU_obj.stNRSParam_V2.SBS1 ;
		ispCfgAttr->impCfgAttr.SBS2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS2[isDaylight]))] = VpssU_obj.stNRSParam_V2.SBS2 ;
		ispCfgAttr->impCfgAttr.SBS3[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SBS3[isDaylight]))] = VpssU_obj.stNRSParam_V2.SBS3 ;
		ispCfgAttr->impCfgAttr.SDS0[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IES0[isDaylight]))] = VpssU_obj.stNRSParam_V2.SDS0  ;
		ispCfgAttr->impCfgAttr.SDS1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS1[isDaylight]))] = VpssU_obj.stNRSParam_V2.SDS1 ;
		ispCfgAttr->impCfgAttr.SDS2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS2[isDaylight]))] = VpssU_obj.stNRSParam_V2.SDS2  ;
		ispCfgAttr->impCfgAttr.SDS3[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SDS3[isDaylight]))] = VpssU_obj.stNRSParam_V2.SDS3;
		ispCfgAttr->impCfgAttr.STH0[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IES0[isDaylight]))] = VpssU_obj.stNRSParam_V2.STH0 ;
		ispCfgAttr->impCfgAttr.STH1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH1[isDaylight]))] = VpssU_obj.stNRSParam_V2.STH1 ;
		ispCfgAttr->impCfgAttr.STH2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH2[isDaylight]))] = VpssU_obj.stNRSParam_V2.STH2 ;
		ispCfgAttr->impCfgAttr.STH3[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.STH3[isDaylight]))] = VpssU_obj.stNRSParam_V2.STH3 ;
		
		ispCfgAttr->impCfgAttr.MDP[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDP[isDaylight]))] = VpssU_obj.stNRSParam_V2.MDP;
		ispCfgAttr->impCfgAttr.MATH1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MATH1[isDaylight]))] = VpssU_obj.stNRSParam_V2.MATH1 ;
		ispCfgAttr->impCfgAttr.MATH2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MATH2[isDaylight]))] = VpssU_obj.stNRSParam_V2.MATH2;
		ispCfgAttr->impCfgAttr.Pro3[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Pro3[isDaylight]))] = VpssU_obj.stNRSParam_V2.Pro3 ;
		ispCfgAttr->impCfgAttr.MDDZ1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDDZ1[isDaylight]))] = VpssU_obj.stNRSParam_V2.MDDZ1;
		ispCfgAttr->impCfgAttr.MDDZ2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.MDDZ2[isDaylight]))] = VpssU_obj.stNRSParam_V2.MDDZ2 ;
		ispCfgAttr->impCfgAttr.TFS1[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFS1[isDaylight]))] = VpssU_obj.stNRSParam_V2.TFS1;
		ispCfgAttr->impCfgAttr.TFS2[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFS2[isDaylight]))] = VpssU_obj.stNRSParam_V2.TFS2;

		ispCfgAttr->impCfgAttr.SFC[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SFC[isDaylight]))] = VpssU_obj.stNRSParam_V2.SFC ;
		ispCfgAttr->impCfgAttr.TFC[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TFC[isDaylight]))] = VpssU_obj.stNRSParam_V2.TFC ;
		ispCfgAttr->impCfgAttr.TPC[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TPC[isDaylight]))] = VpssU_obj.stNRSParam_V2.TPC ;
		ispCfgAttr->impCfgAttr.TRC[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.TRC[isDaylight]))] = VpssU_obj.stNRSParam_V2.TRC ;
#endif

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
	return 0 == HI_ISP_cfg_set_ae(isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_set_awb(scene, isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_set_imp(isDaylight, ispCfgAttr) ? debug_register(isDaylight, scene) : -1;
#else
	return 0 == HI_ISP_cfg_set_ae(isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_set_awb(scene, isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_set_imp(isDaylight, ispCfgAttr) ? 0 : -1;
#endif
}

int HI_ISP_cfg_get_all(int isDaylight, int scene, LpIspCfgAttr ispCfgAttr)
{
	if(NULL == ispCfgAttr){
		return -1;
	}

#ifdef INI_DEBUG_ECHO_OF_ALL
	return 0 == HI_ISP_cfg_get_ae(isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_get_awb(scene, isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_get_imp(isDaylight, ispCfgAttr) ? debug_all(ispCfgAttr) : -1;
#else
	return 0 == HI_ISP_cfg_get_ae(isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_get_awb(scene, isDaylight, ispCfgAttr) && 0 == HI_ISP_cfg_get_imp(isDaylight, ispCfgAttr) ? 0 : -1;
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

