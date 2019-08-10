#include "../sdk_trace.h"

#include "hi_isp_cfg_parse.h"
#include "hi_isp_cfg.h"
#include "hi3518a.h"

#include <math.h>

extern const HI_U16 gs_Gamma[7][GAMMA_NODE_NUMBER];

static int
_isp_get_iso(void)
{
	uint32_t ret_iso = 0;
	ISP_INNER_STATE_INFO_EX_S pstInnerStateInfo;
	SOC_CHECK(HI_MPI_ISP_QueryInnerStateInfoEx(&pstInnerStateInfo));
//	ret_iso = (pstInnerStateInfo.u32AnalogGain * ((pstInnerStateInfo.u32DigitalGain * (pstInnerStateInfo.u32ISPDigitalGain >> 10)) >> 4)) >> 6;
	ret_iso = ((pstInnerStateInfo.u32AnalogGain>> 10)* ((pstInnerStateInfo.u32DigitalGain* pstInnerStateInfo.u32ISPDigitalGain)))>>10;

	return ret_iso / 1024;
}

static int
count_from_iso(int n)
{
	int iso = _isp_get_iso();
	return 0 == n || 0 == iso ? 0 : (int)(log(iso) / log(2) + 0.41) / (8 / n);
}

#ifdef INI_DEBUG_ECHO_OF_ALL
static void
debug_ae(LpIspCfgAttr ispCfgAttr)
{
	int i = 0, j = 0;
	printf("\033[31;1m[AE]\033[32;1m\n");
	for(i = 0; i < 2; i++)
	{
		printf("AeCompensation_%d:\t0x%02x\n", i, ispCfgAttr->aeCfgAttr.AeCompensation[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("MaxAgainTarget_%d:\t%d\n", i, ispCfgAttr->aeCfgAttr.MaxAgainTarget[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("MaxDgainTarget_%d:\t%d\n", i, ispCfgAttr->aeCfgAttr.MaxDgainTarget[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("MaxISPDgainTarget_%d:\t%d\n", i, ispCfgAttr->aeCfgAttr.MaxISPDgainTarget[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("MaxSystemGainTarget_%d:\t%d\n", i, ispCfgAttr->aeCfgAttr.MaxSystemGainTarget[i]);
	}
	printf("\n");
	printf("AeExpStep:\t\t0x%02x\n", ispCfgAttr->aeCfgAttr.AeExpStep);
	printf("AeExpTolerance:\t\t0x%02x\n\n", ispCfgAttr->aeCfgAttr.AeExpTolerance);
	for(i = 0; i < 2; i++)
	{
		printf("AeHistslope_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->aeCfgAttr.AeHistslope[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->aeCfgAttr.AeHistslope[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("AeHistOffset_%d:\t\t0x%02x\n", i, ispCfgAttr->aeCfgAttr.AeHistOffset[i]);
	}
	printf("\n");

	for(i = 0; i < 2; i++)
	{
		printf("AeHistweight_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->aeCfgAttr.AeHistweight[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->aeCfgAttr.AeHistweight[i][j]);
		}
		printf("\n");
	}
}

static void
debug_awb(LpIspCfgAttr ispCfgAttr)
{
	int i = 0, j = 0;
	printf("\033[31;1m[AWB]\033[32;1m\n");
	for(i = 0; i < 2; i++)
	{
		printf("HighColorTemp_%d:\t%d\n", i, ispCfgAttr->awbCfgAttr.HighColorTemp[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("HighCCM_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->awbCfgAttr.HighCCM[0]); j++)
		{
			printf("0x%04x\t", ispCfgAttr->awbCfgAttr.HighCCM[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("MidColorTemp_%d:\t\t%d\n", i, ispCfgAttr->awbCfgAttr.MidColorTemp[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("MidCCM_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->awbCfgAttr.MidCCM[0]); j++)
		{
			printf("0x%04x\t", ispCfgAttr->awbCfgAttr.MidCCM[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("LowColorTemp_%d:\t\t%d\n", i, ispCfgAttr->awbCfgAttr.LowColorTemp[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("LowCCM_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->awbCfgAttr.LowCCM[0]); j++)
		{
			printf("0x%04x\t", ispCfgAttr->awbCfgAttr.LowCCM[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("WbRefTemp_%d:\t\t%d\n", i, ispCfgAttr->awbCfgAttr.WbRefTemp[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("StaticWB_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->awbCfgAttr.StaticWB[0]); j++)
		{
			printf("0x%03x\t", ispCfgAttr->awbCfgAttr.StaticWB[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("AwbPara_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->awbCfgAttr.AwbPara[0]); j++)
		{
			printf("%d\t", ispCfgAttr->awbCfgAttr.AwbPara[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("Saturation_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->awbCfgAttr.Saturation[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->awbCfgAttr.Saturation[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("BlackLevel_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->awbCfgAttr.BlackLevel[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->awbCfgAttr.BlackLevel[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

static void
debug_imp(LpIspCfgAttr ispCfgAttr)
{
	int i = 0, j = 0;

	printf("\033[31;1m[IMP]\033[32;1m\n");
	for(i = 0; i < 2; i++)
	{
		printf("SharpenAltD_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltD[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->impCfgAttr.SharpenAltD[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("SharpenAltUd_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltUd[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->impCfgAttr.SharpenAltUd[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("SnrThresh_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrThresh[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->impCfgAttr.SnrThresh[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("DemosaicUuSlope_%d:\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->impCfgAttr.DemosaicUuSlope[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("Gamma_%d:\t\t%d\n", i, ispCfgAttr->impCfgAttr.Gamma[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("SfStrength_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->impCfgAttr.SfStrength[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->impCfgAttr.SfStrength[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("DrcThreshold[%d]:\t%d\n", i, ispCfgAttr->impCfgAttr.DrcThreshold[i]);
	}
	printf("\n");
	for(i = 0; i < 2; i++)
	{
		printf("DrcSlope[%d]:\t\t0x%02hx\n", i, ispCfgAttr->impCfgAttr.DrcSlope[i]);
	}
	printf("\n");
	printf("DefectPixelSlope:\t");
	for(i = 0; i < 4; i++)
	{
		printf("0x%x\t", ispCfgAttr->impCfgAttr.DefectPixelSlope[i]);
	}
	printf("\n");
	printf("DefectPixelThresh:\t");
	for(i = 0; i < 4; i++)
	{
		printf("0x%x\t", ispCfgAttr->impCfgAttr.DefectPixelThresh[i]);
	}
	printf("\n\n");
	for(i = 0; i < 2; i++)
	{
		printf("ChnSp_%d:\t\t", i);
		for(j = 0; j < ARRAY_NUM(ispCfgAttr->impCfgAttr.ChnSp[0]); j++)
		{
			printf("0x%02x\t", ispCfgAttr->impCfgAttr.ChnSp[i][j]);
		}
		printf("\n");
	}
	printf("\n\n");
	for(i = 0; i < 2; i++)
	{
		printf("SlowFrameRateHighHold[%d]:\t%d\n", i, ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[i]);
	}
	printf("\n\n");
	for(i = 0; i < 2; i++)
	{
		printf("SlowFrameRateLowHold[%d]:\t%d\n", i, ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[i]);
	}
	printf("\n\n");
	printf("AntiFlikeThreshold:\t%d\n", ispCfgAttr->impCfgAttr.AntiFlikeThreshold);
}

static int
debug_register(int isDaylight, int scene)
{
	int i = 0;
	int iso = _isp_get_iso();
	ISP_AE_ATTR_EX_S obj_ae;
	ISP_COLORMATRIX_S obj_color;
	ISP_AWB_ATTR_S obj_awb;
	ISP_SATURATION_ATTR_S obj_saturation;
	ISP_BLACK_LEVEL_S obj_black_level;
	ISP_SHARPEN_ATTR_S obj_sharpen;
	ISP_DENOISE_ATTR_S obj_denoise;
	ISP_DEMOSAIC_ATTR_S obj_demosaic;
	ISP_GAMMA_TABLE_S obj_gamma;
	VPSS_GRP_PARAM_S obj_vpss;
	ISP_DRC_ATTR_S obj_drc;
	ISP_DP_ATTR_S obj_dp;
	VPSS_CHN_SP_PARAM_S obj_chnsp;
	HI_U8 obj_fps;
	ISP_ANTIFLICKER_S obj_antiflicker;

	if(0 == HI_MPI_ISP_GetAEAttrEx(&obj_ae) && 0 == HI_MPI_ISP_GetCCM(&obj_color) && 0 == HI_MPI_ISP_GetAWBAttr(&obj_awb) \
				&& 0 == HI_MPI_ISP_GetSaturationAttr(&obj_saturation) && 0 == HI_MPI_ISP_GetBlackLevelAttr(&obj_black_level) \
				&& 0 == HI_MPI_ISP_GetSharpenAttr(&obj_sharpen) && 0 == HI_MPI_ISP_GetDenoiseAttr(&obj_denoise) \
				&& 0 == HI_MPI_ISP_GetDemosaicAttr(&obj_demosaic) && 0 == HI_MPI_ISP_GetGammaTable(&obj_gamma) \
				&& 0 == HI_MPI_VPSS_GetGrpParam(0, &obj_vpss) && 0 == HI_MPI_ISP_GetDRCAttr(&obj_drc) \
				&& 0 == HI_MPI_ISP_GetDefectPixelAttr(&obj_dp) && 0 == HI_MPI_VPSS_GetChnSpParam(0, 1, &obj_chnsp)
				&& 0 == HI_MPI_ISP_GetSlowFrameRate(&obj_fps) && 0 == HI_MPI_ISP_GetAntiFlickerAttr(&obj_antiflicker))
	{
		printf("\n\033[31;1m##### [%s] ##############################################################################################\n\n", __func__);
	// iso
		printf("\033[31;1m[ISO]\033[32;1m\n");
		printf("iso:\t\t\t%d\n\n", iso);
	//param
		printf("\033[31;1m[Param]\033[32;1m\n");
		printf("isDaylight:\t\t%d\n", isDaylight);
		printf("scene:\t\t\t%d\n\n", scene);
	// obj_ae
		printf("\033[31;1m[AE]\033[32;1m\n");
		printf("u8ExpCompensation:\t%d\n\n", obj_ae.u8ExpCompensation);

		printf("u32AGainMax:\t\t%d\n", obj_ae.u32AGainMax);
		printf("u32AGainMin:\t\t%d\n\n", obj_ae.u32AGainMin);

		printf("u32DGainMax:\t\t%d\n", obj_ae.u32DGainMax);
		printf("u32DGainMin:\t\t%d\n\n", obj_ae.u32DGainMin);

		printf("u32ISPDGainMax:\t\t%d\n", obj_ae.u32ISPDGainMax);
		printf("u32SystemGainMax:\t%d\n\n", obj_ae.u32SystemGainMax);
		
		printf("u8ExpStep:\t\t%d\n", obj_ae.u8ExpStep);
		printf("s16ExpTolerance:\t%d\n\n", obj_ae.s16ExpTolerance);

		printf("u16HistRatioSlope:\t%d\n", obj_ae.u16HistRatioSlope);
		printf("u8MaxHistOffset:\t%d\n\n", obj_ae.u8MaxHistOffset);
	// obj_awb
		printf("\033[31;1m[AWB]\033[32;1m\n");
		printf("as32CurvePara[6]:\t%d\t%d\t%d\t%d\t%d\t%d\n", obj_awb.stAWBCalibration.as32CurvePara[0], obj_awb.stAWBCalibration.as32CurvePara[1], obj_awb.stAWBCalibration.as32CurvePara[2], obj_awb.stAWBCalibration.as32CurvePara[3], obj_awb.stAWBCalibration.as32CurvePara[4], obj_awb.stAWBCalibration.as32CurvePara[5]);
		printf("au16StaticWB[4]:\t%d\t%d\t%d\t%d\n", obj_awb.stAWBCalibration.au16StaticWB[0], obj_awb.stAWBCalibration.au16StaticWB[1], obj_awb.stAWBCalibration.au16StaticWB[2], obj_awb.stAWBCalibration.au16StaticWB[3]);
		printf("u16RefTemp:\t\t%d\n\n\n", obj_awb.stAWBCalibration.u16RefTemp);
	// obj_black_level
		printf("\033[31;1m[BlackLevel]\033[32;1m\n");
		printf("au16BlackLevel[4]:\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\n\n", obj_black_level.au16BlackLevel[0], obj_black_level.au16BlackLevel[1], obj_black_level.au16BlackLevel[2], obj_black_level.au16BlackLevel[3]);
	// obj_color
		printf("\033[31;1m[CCM]\033[32;1m\nu16HighColorTemp:\t%d\n", obj_color.u16HighColorTemp);
		printf("au16HighCCM[9]:\t\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\n\n", obj_color.au16HighCCM[0], obj_color.au16HighCCM[1], obj_color.au16HighCCM[2], obj_color.au16HighCCM[3], obj_color.au16HighCCM[4], obj_color.au16HighCCM[5], obj_color.au16HighCCM[6], obj_color.au16HighCCM[7], obj_color.au16HighCCM[8]);
		printf("u16MidColorTemp:\t%d\n", obj_color.u16MidColorTemp);
		printf("au16MidCCM[9]:\t\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\n\n", obj_color.au16MidCCM[0], obj_color.au16MidCCM[1], obj_color.au16MidCCM[2], obj_color.au16MidCCM[3], obj_color.au16MidCCM[4], obj_color.au16MidCCM[5], obj_color.au16MidCCM[6], obj_color.au16MidCCM[7], obj_color.au16MidCCM[8]);
		printf("u16LowColorTemp:\t%d\n", obj_color.u16LowColorTemp);
		printf("au16LowCCM[9]:\t\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\t0x%04hx\n\n", obj_color.au16LowCCM[0], obj_color.au16LowCCM[1], obj_color.au16LowCCM[2], obj_color.au16LowCCM[3], obj_color.au16LowCCM[4], obj_color.au16LowCCM[5], obj_color.au16LowCCM[6], obj_color.au16LowCCM[7], obj_color.au16LowCCM[8]);
	// obj_saturation
		printf("au8Sat[8]:\t\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\n\n", obj_saturation.au8Sat[0], obj_saturation.au8Sat[1], obj_saturation.au8Sat[2], obj_saturation.au8Sat[3], obj_saturation.au8Sat[4], obj_saturation.au8Sat[5], obj_saturation.au8Sat[6], obj_saturation.au8Sat[7]);
	// obj_sharpen
		printf("\033[31;1m[Sharpen]\033[32;1m\nu8SharpenAltD[8]:\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\n", obj_sharpen.u8SharpenAltD[0], obj_sharpen.u8SharpenAltD[1], obj_sharpen.u8SharpenAltD[2], obj_sharpen.u8SharpenAltD[3], obj_sharpen.u8SharpenAltD[4], obj_sharpen.u8SharpenAltD[5], obj_sharpen.u8SharpenAltD[6], obj_sharpen.u8SharpenAltD[7]);
		printf("u8SharpenAltUd[8]:\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\n\n", obj_sharpen.u8SharpenAltUd[0], obj_sharpen.u8SharpenAltUd[1], obj_sharpen.u8SharpenAltUd[2], obj_sharpen.u8SharpenAltUd[3], obj_sharpen.u8SharpenAltUd[4], obj_sharpen.u8SharpenAltUd[5], obj_sharpen.u8SharpenAltUd[6], obj_sharpen.u8SharpenAltUd[7]);
	// obj_denoise
		printf("\033[31;1m[DENOISE]\033[32;1m\nu8SnrThresh[8]:\t\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\t0x%02hx\n\n", obj_denoise.u8SnrThresh[0], obj_denoise.u8SnrThresh[1], obj_denoise.u8SnrThresh[2], obj_denoise.u8SnrThresh[3], obj_denoise.u8SnrThresh[4], obj_denoise.u8SnrThresh[5], obj_denoise.u8SnrThresh[6], obj_denoise.u8SnrThresh[7]);
	// obj_demosaic
		printf("\033[31;1m[DEMOSAIC]\033[32;1m\nu8UuSlope:\t\t0x%02hx\n\n", obj_demosaic.u8UuSlope);
	// obj_gamma
		printf("\033[31;1m[GAMMA]\033[32;1m\nenGammaCurve:\t\t%d\n", obj_gamma.enGammaCurve);
		printf("u16Gamma[12]:\t\t");
		for(i = 0; i < 12; i++)
		{
			printf("0x%03hx\t", obj_gamma.u16Gamma[i]);
		}
		printf("......\n\n");
	// obj_vpss
		printf("\033[31;1m[VPSS]\033[32;1m\nu32SfStrength:\t\t0x%02hx\n\n", obj_vpss.u32SfStrength);
	// obj_drc
		printf("\033[31;1m[DRC]\033[32;1m\nbDRCEnable:\t\t%d\n", obj_drc.bDRCEnable);
		printf("u32SlopeMax:\t\t0x%02x\n\n", obj_drc.u32SlopeMax);
	// obj_dp
		printf("\033[31;1m[Defect Pixel]\033[32;1m\nDynamicBadPixelSlope:\t0x%x\n", obj_dp.u16DynamicBadPixelSlope);
		printf("DynamicBadPixelThresh:\t0x%x\n\n", obj_dp.u16DynamicBadPixelThresh);
	// obj_chnsp
		printf("\033[31;1m[VPSS CHNSP]\033[32;1m\nu32LumaGain:\t\t0x%02x\n\n", obj_chnsp.u32LumaGain);
	// obj_fps
		printf("\033[31;1m[FPS]\033[32;1m\nSlowFrameRate\t\t%d\n", obj_fps);
		printf("iso\t\t\t%d\n\n", iso);
	// obj_antiflicker
		printf("\033[31;1m[AntiFlicker]\033[32;1m\nbEnable\t\t\t%d\n\n\033[0m", obj_antiflicker.bEnable);
		return 0;
	}
	else
	{
		printf("\n\033[32;1m********************************************ERROR: %s********************************************\033[0m\n", __func__);
		return -1;
	}
}

static int
debug_all(LpIspCfgAttr ispCfgAttr)
{
	printf("\n\033[31;1m##### [%s] ##############################################################################################\n\n", __func__);
	debug_ae(ispCfgAttr);
	printf("\n");
	debug_awb(ispCfgAttr);
	printf("\n");
	debug_imp(ispCfgAttr);
	printf("\033[0m\n\n");
	return 0;
}
#endif

static int _hi_isp_cfg_set_slow_framerate(uint8_t Value)   
{
	int actual_fps;
	VENC_CHN_ATTR_S vencChannelAttr;
	VI_CHN_ATTR_S vi_chn_attr;

	
	SOC_CHECK(HI_MPI_VI_GetChnAttr(0, &vi_chn_attr));
	actual_fps = vi_chn_attr.s32FrameRate/(Value>>4);

	printf("slow frame rate:%d\n", actual_fps);
	HI_MPI_VENC_GetChnAttr(0,&vencChannelAttr); 
	switch(vencChannelAttr.stRcAttr.enRcMode){
		default:
		case VENC_RC_MODE_H264VBRv2:
			if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate){//slow framerate
				
				vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate;
			}else{//actual framerate
				vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = vi_chn_attr.s32FrameRate;
			}
			//vencChannelAttr.stRcAttr.stAttrH264Vbr.u32Gop = vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate*2;
			break;
		case VENC_RC_MODE_H264CBRv2:
			if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32TargetFrmRate){//slow framerate
				vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32TargetFrmRate;
			}else{//actual framerate
				vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate = vi_chn_attr.s32FrameRate;
			}
			//vencChannelAttr.stRcAttr.stAttrH264Cbr.u32Gop = vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate*2;
			break;		
	}


	if(32 == Value || 48 == Value){
		SOC_CHECK(HI_MPI_ISP_SetSlowFrameRate(Value));
		usleep(100*1000);
		HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr);
	}else{	//16
		HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr); 
		usleep(100*1000);
		SOC_CHECK(HI_MPI_ISP_SetSlowFrameRate(0x10));	
	}

}

//use sensor clk to slow framerate for ar0330
static int _hi_isp_cfg_set_slow_framerate2(uint8_t bValue)
{
	uint8_t Value = bValue ? 0x1:0x0;
	int actual_fps;
	VENC_CHN_ATTR_S vencChannelAttr;
	VI_CHN_ATTR_S vi_chn_attr;

	SOC_CHECK(HI_MPI_VI_GetChnAttr(0, &vi_chn_attr));
	actual_fps = vi_chn_attr.s32FrameRate/(1 << Value);

	HI_MPI_VENC_GetChnAttr(0,&vencChannelAttr); 
	switch(vencChannelAttr.stRcAttr.enRcMode){
		default:
		case VENC_RC_MODE_H264VBRv2:
			if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate){//slow framerate
				
				vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate;
			}else{//actual framerate
				vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = vi_chn_attr.s32FrameRate;
			}
			//vencChannelAttr.stRcAttr.stAttrH264Vbr.u32Gop = vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate*2;
			break;
		case VENC_RC_MODE_H264CBRv2:
			if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32TargetFrmRate){//slow framerate
				vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32TargetFrmRate;
			}else{//actual framerate
				vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate = vi_chn_attr.s32FrameRate;
			}
			//vencChannelAttr.stRcAttr.stAttrH264Cbr.u32Gop = vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate*2;
			break;		
	}
	HI_U32 cur_sensor_clock, sensor_clock;//0 for 12MHz, 1 for 24MHz
	HI_MPI_SYS_GetReg(0x20030030, &cur_sensor_clock);
	sensor_clock = Value ? 0 : 5;
	if(sensor_clock != cur_sensor_clock){
		printf("set ar0330 clk to %dMHz\r\n", Value ? 12 : 24);
		HI_MPI_SYS_SetReg(0x20030030, sensor_clock);
	}	
}


int HI_ISP_cfg_set_ae(int isDaylight, LpIspCfgAttr ispCfgAttr)

{
	int i = 0 ,j = 0;
	int sum = 0;
	HI_U8 u8Weight[WEIGHT_ZONE_ROW][WEIGHT_ZONE_COLUMN] ;
	ISP_AE_ATTR_EX_S obj_ae;

	if(NULL != ispCfgAttr && 0 == HI_MPI_ISP_GetAEAttrEx(&obj_ae))
	{
		obj_ae.u8ExpCompensation = ispCfgAttr->aeCfgAttr.AeCompensation[isDaylight];
		obj_ae.u32AGainMax = ispCfgAttr->aeCfgAttr.MaxAgainTarget[isDaylight];
		obj_ae.u32DGainMax = ispCfgAttr->aeCfgAttr.MaxDgainTarget[isDaylight];
		obj_ae.u32ISPDGainMax = ispCfgAttr->aeCfgAttr.MaxISPDgainTarget[isDaylight];
		obj_ae.u32SystemGainMax = ispCfgAttr->aeCfgAttr.MaxSystemGainTarget[isDaylight];
		obj_ae.u8ExpStep = ispCfgAttr->aeCfgAttr.AeExpStep;
		obj_ae.s16ExpTolerance = ispCfgAttr->aeCfgAttr.AeExpTolerance;
		obj_ae.u16HistRatioSlope = ispCfgAttr->aeCfgAttr.AeHistslope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->aeCfgAttr.AeHistslope[isDaylight]))];
		obj_ae.u8MaxHistOffset = ispCfgAttr->aeCfgAttr.AeHistOffset[isDaylight];
		
		for(i = 0; i < WEIGHT_ZONE_ROW; i++){
			for(j = 0;j < WEIGHT_ZONE_COLUMN; j++){
				u8Weight[i][j] = ispCfgAttr->aeCfgAttr.AeHistweight[isDaylight][i *WEIGHT_ZONE_COLUMN + j];
				sum = sum + u8Weight[i][j] ;
			}
		}
		if(0 != sum ){
			memcpy(obj_ae.u8Weight,u8Weight,sizeof(u8Weight));
		}
		

		return HI_MPI_ISP_SetAEAttrEx(&obj_ae);
	}
	return -1;
}

int HI_ISP_cfg_get_ae(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	int i = 0 ,j=0;
	ISP_AE_ATTR_EX_S obj_ae;

	if(NULL != ispCfgAttr && 0 == HI_MPI_ISP_GetAEAttrEx(&obj_ae))
	{
		ispCfgAttr->aeCfgAttr.AeCompensation[isDaylight] = obj_ae.u8ExpCompensation;
		ispCfgAttr->aeCfgAttr.MaxAgainTarget[isDaylight] = obj_ae.u32AGainMax;
		ispCfgAttr->aeCfgAttr.MaxDgainTarget[isDaylight] = obj_ae.u32DGainMax;
		ispCfgAttr->aeCfgAttr.MaxISPDgainTarget[isDaylight] = obj_ae.u32ISPDGainMax;
		ispCfgAttr->aeCfgAttr.MaxSystemGainTarget[isDaylight] = obj_ae.u32SystemGainMax;
		ispCfgAttr->aeCfgAttr.AeExpStep = obj_ae.u8ExpStep;
		ispCfgAttr->aeCfgAttr.AeExpTolerance = obj_ae.s16ExpTolerance;
		ispCfgAttr->aeCfgAttr.AeHistslope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->aeCfgAttr.AeHistslope[isDaylight]))] = obj_ae.u16HistRatioSlope;
		
		for(i = 0; i < WEIGHT_ZONE_ROW; i++)
		{
			for(j = 0;j < WEIGHT_ZONE_COLUMN; j++)
			{
				ispCfgAttr->aeCfgAttr.AeHistweight[isDaylight][i *WEIGHT_ZONE_COLUMN + j] = obj_ae.u8Weight[i][j];
			}
		}
		return 0;
	}
	return -1;
}

int HI_ISP_cfg_set_awb(int scene, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;
	ISP_COLORMATRIX_S obj_color;
	ISP_AWB_ATTR_S obj_awb;
	ISP_SATURATION_ATTR_S obj_saturation;
	ISP_BLACK_LEVEL_S obj_black_level;

	if(NULL != ispCfgAttr && 0 == HI_MPI_ISP_GetCCM(&obj_color) && 0 == HI_MPI_ISP_GetAWBAttr(&obj_awb) \
				&& 0 == HI_MPI_ISP_GetSaturationAttr(&obj_saturation) && 0 == HI_MPI_ISP_GetBlackLevelAttr(&obj_black_level))
	{
	// obj_color
		obj_color.u16HighColorTemp = ispCfgAttr->awbCfgAttr.HighColorTemp[scene];
		for(i = 0; i < ARRAY_NUM(obj_color.au16HighCCM); i++)
		{
			obj_color.au16HighCCM[i] = ispCfgAttr->awbCfgAttr.HighCCM[scene][i];
		}
		obj_color.u16MidColorTemp = ispCfgAttr->awbCfgAttr.MidColorTemp[scene];
		for(i = 0; i < ARRAY_NUM(obj_color.au16MidCCM); i++)
		{
			obj_color.au16MidCCM[i] = ispCfgAttr->awbCfgAttr.MidCCM[scene][i];
		}
		obj_color.u16LowColorTemp = ispCfgAttr->awbCfgAttr.LowColorTemp[scene];
		for(i = 0; i < ARRAY_NUM(obj_color.au16LowCCM); i++)
		{
			obj_color.au16LowCCM[i] = ispCfgAttr->awbCfgAttr.LowCCM[scene][i];
		}
	// obj_awb
		obj_awb.stAWBCalibration.u16RefTemp = ispCfgAttr->awbCfgAttr.WbRefTemp[scene];
		for(i = 0; i < ARRAY_NUM(obj_awb.stAWBCalibration.au16StaticWB); i++)
		{
			obj_awb.stAWBCalibration.au16StaticWB[i] = ispCfgAttr->awbCfgAttr.StaticWB[scene][i];
		}
		for(i = 0; i < ARRAY_NUM(obj_awb.stAWBCalibration.as32CurvePara); i++)
		{
			obj_awb.stAWBCalibration.as32CurvePara[i] = ispCfgAttr->awbCfgAttr.AwbPara[scene][i];
		}
	// obj_saturation
		for(i = 0; i < ARRAY_NUM(obj_saturation.au8Sat); i++)
		{
			obj_saturation.au8Sat[i] = ispCfgAttr->awbCfgAttr.Saturation[scene][i];
		}
	// obj_black_level
		for(i = 0; i < ARRAY_NUM(obj_black_level.au16BlackLevel); i++)
		{
			obj_black_level.au16BlackLevel[i] = ispCfgAttr->awbCfgAttr.BlackLevel[scene][i];
		}
	// set to HI_MPI_ISP
		return (0 == HI_MPI_ISP_SetCCM(&obj_color) && 0 == HI_MPI_ISP_SetAWBAttr(&obj_awb) \
				&& 0 == HI_MPI_ISP_SetSaturationAttr(&obj_saturation) && 0 == HI_MPI_ISP_SetBlackLevelAttr(&obj_black_level)) ? 0 : -1;
	}
	return -1;
}

int HI_ISP_cfg_get_awb(int scene, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;
	ISP_COLORMATRIX_S obj_color;
	ISP_AWB_ATTR_S obj_awb;
	ISP_SATURATION_ATTR_S obj_saturation;
	ISP_BLACK_LEVEL_S obj_black_level;

	if(NULL != ispCfgAttr && 0 == HI_MPI_ISP_GetCCM(&obj_color) && 0 == HI_MPI_ISP_GetAWBAttr(&obj_awb) \
				&& 0 == HI_MPI_ISP_GetSaturationAttr(&obj_saturation) && 0 == HI_MPI_ISP_GetBlackLevelAttr(&obj_black_level))
	{
	// obj_color
		ispCfgAttr->awbCfgAttr.HighColorTemp[scene] = obj_color.u16HighColorTemp;
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.HighCCM[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.HighCCM[scene][i] = obj_color.au16HighCCM[i];
		}
		ispCfgAttr->awbCfgAttr.MidColorTemp[scene] = obj_color.u16MidColorTemp;
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.MidCCM[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.MidCCM[scene][i] = obj_color.au16MidCCM[i];
		}
		ispCfgAttr->awbCfgAttr.LowColorTemp[scene] = obj_color.u16LowColorTemp;
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.LowCCM[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.LowCCM[scene][i] = obj_color.au16LowCCM[i];
		}
	// obj_awb
		ispCfgAttr->awbCfgAttr.WbRefTemp[scene] = obj_awb.stAWBCalibration.u16RefTemp;
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.StaticWB[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.StaticWB[scene][i] = obj_awb.stAWBCalibration.au16StaticWB[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.AwbPara[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.AwbPara[scene][i] = obj_awb.stAWBCalibration.as32CurvePara[i];
		}
	// obj_saturation
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.Saturation[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.Saturation[scene][i] = obj_saturation.au8Sat[i];
		}
	// obj_black_level
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->awbCfgAttr.BlackLevel[scene]); i++)
		{
			ispCfgAttr->awbCfgAttr.BlackLevel[scene][i] = obj_black_level.au16BlackLevel[i];
		}
		return 0;
	}
	return -1;
}

int HI_ISP_cfg_set_imp(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;
	int iso = _isp_get_iso();
	ISP_SHARPEN_ATTR_S obj_sharpen;
	ISP_DENOISE_ATTR_S obj_denoise;
	ISP_DEMOSAIC_ATTR_S obj_demosaic;
	ISP_GAMMA_TABLE_S obj_gamma;
	VPSS_GRP_PARAM_S obj_vpss;
	ISP_DRC_ATTR_S obj_drc;
	ISP_DP_ATTR_S obj_dp;
	VPSS_CHN_SP_PARAM_S obj_chnsp;
	HI_U8 obj_fps, cur_fps;
	ISP_ANTIFLICKER_S obj_antiflicker;

	if(NULL != ispCfgAttr && 0 == HI_MPI_ISP_GetSharpenAttr(&obj_sharpen) && 0 == HI_MPI_ISP_GetDenoiseAttr(&obj_denoise) \
				&& 0 == HI_MPI_ISP_GetDemosaicAttr(&obj_demosaic) && 0 == HI_MPI_ISP_GetGammaTable(&obj_gamma) \
				&& 0 == HI_MPI_VPSS_GetGrpParam(0, &obj_vpss) && 0 == HI_MPI_ISP_GetDRCAttr(&obj_drc) \
				&& 0 == HI_MPI_ISP_GetDefectPixelAttr(&obj_dp) && 0 == HI_MPI_ISP_GetSlowFrameRate(&obj_fps) \
				&& 0 == HI_MPI_ISP_GetAntiFlickerAttr(&obj_antiflicker))
	{
	// obj_sharpen
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltD[isDaylight]); i++)
		{
			obj_sharpen.u8SharpenAltD[i] = ispCfgAttr->impCfgAttr.SharpenAltD[isDaylight][i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltUd[isDaylight]); i++)
		{
			obj_sharpen.u8SharpenAltUd[i] = ispCfgAttr->impCfgAttr.SharpenAltUd[isDaylight][i];
		}
	// obj_denoise
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrThresh[isDaylight]); i++)
		{
			obj_denoise.u8SnrThresh[i] = ispCfgAttr->impCfgAttr.SnrThresh[isDaylight][i];
		}
	// obj_demosaic
		obj_demosaic.u8UuSlope = ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))];
	// obj_gamma
		obj_gamma.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
		for(i = 0; i < GAMMA_NODE_NUMBER; i++)
		{
			obj_gamma.u16Gamma[i] = gs_Gamma[ispCfgAttr->impCfgAttr.Gamma[isDaylight]][i];
		}
	// obj_vpss
		obj_vpss.u32SfStrength = ispCfgAttr->impCfgAttr.SfStrength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SfStrength[isDaylight]))];
		obj_vpss.u32TfStrength = obj_vpss.u32SfStrength / 2;
		obj_vpss.u32ChromaRange = obj_vpss.u32SfStrength;
	// obj_drc
		//obj_drc.bDRCEnable = (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE;
		obj_drc.u32StrengthTarget = ispCfgAttr->impCfgAttr.DrcSlope[isDaylight]+ ispCfgAttr->impCfgAttr.DrcSlope[isDaylight]*(ispCfgAttr->userAttr.DrcStrength - 3)/2;
		//obj_drc.u32SlopeMax = ispCfgAttr->impCfgAttr.DrcSlope[isDaylight]+ ispCfgAttr->impCfgAttr.DrcSlope[isDaylight]*(ispCfgAttr->userAttr.DrcStrength - 3)/2;
		obj_drc.u32SlopeMax = 0xe0;
	// obj_dp
		obj_dp.u16DynamicBadPixelSlope = ispCfgAttr->impCfgAttr.DefectPixelSlope[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelSlope))];
		obj_dp.u16DynamicBadPixelThresh = ispCfgAttr->impCfgAttr.DefectPixelThresh[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelThresh))];
	// obj_chnsp
		obj_chnsp.u32LumaGain = ispCfgAttr->impCfgAttr.ChnSp[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.ChnSp[isDaylight]))];
	// obj_fps	
		if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHigherHold[isDaylight])
		{			
			cur_fps = 48;
		}
		else if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight])
		{
			cur_fps = 32;
		}
		else if(iso < ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[isDaylight])
		{
			cur_fps = 16;
		}else{
			cur_fps = obj_fps;
		}
		
		if(cur_fps != obj_fps && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			_hi_isp_cfg_set_slow_framerate(cur_fps);
		}
	// obj_antiflicker
		//obj_antiflicker.bEnable = iso < ispCfgAttr->impCfgAttr.AntiFlikeThreshold ? HI_TRUE : HI_FALSE;

		return (0 == HI_MPI_ISP_SetSharpenAttr(&obj_sharpen) && 0 == HI_MPI_ISP_SetDenoiseAttr(&obj_denoise) \
			&& 0 == HI_MPI_ISP_SetDemosaicAttr(&obj_demosaic) && 0 == HI_MPI_ISP_SetGammaTable(&obj_gamma) \
			&& 0 == HI_MPI_VPSS_SetGrpParam(0, &obj_vpss) && 0 == HI_MPI_ISP_SetDRCAttr(&obj_drc) \
			&& 0 == HI_MPI_ISP_SetDefectPixelAttr(&obj_dp)) && 0 == HI_MPI_VPSS_SetChnSpParam(0, 1, &obj_chnsp) \
			/*&& 0 == HI_MPI_ISP_SetAntiFlickerAttr(&obj_antiflicker)*/ ? 0 : -1;
	}
	return -1;

}

int HI_ISP_cfg_set_imp_single(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;
	int iso = _isp_get_iso();
	ISP_DEMOSAIC_ATTR_S obj_demosaic;
	VPSS_GRP_PARAM_S obj_vpss;
	ISP_DRC_ATTR_S obj_drc;
	ISP_DP_ATTR_S obj_dp;
	VPSS_CHN_SP_PARAM_S obj_chnsp;
	HI_U8 obj_fps,cur_fps;
	
	static int fps_cnt = 0, fps_cnt_16 =0, fps_cnt_32=0, fps_cnt_48 =0;

 	if(NULL != ispCfgAttr && 0 == HI_MPI_ISP_GetDemosaicAttr(&obj_demosaic) && 0 == HI_MPI_VPSS_GetGrpParam(0, &obj_vpss) \
				&& 0 == HI_MPI_ISP_GetDRCAttr(&obj_drc) && 0 == HI_MPI_ISP_GetDefectPixelAttr(&obj_dp) \
				&& 0 == HI_MPI_VPSS_GetChnSpParam(0, 1, &obj_chnsp) \
				&&0 == HI_MPI_ISP_GetSlowFrameRate(&obj_fps))
	{
	// obj_demosaic
		if(obj_demosaic.u8UuSlope != ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))]){
			obj_demosaic.u8UuSlope = ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))];
			HI_MPI_ISP_SetDemosaicAttr(&obj_demosaic);
			printf("HI_MPI_ISP_SetDemosaicAttr\r\n");
		}
	// obj_vpss
		if(obj_vpss.u32SfStrength != ispCfgAttr->impCfgAttr.SfStrength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SfStrength[isDaylight]))]){
			obj_vpss.u32SfStrength = ispCfgAttr->impCfgAttr.SfStrength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SfStrength[isDaylight]))];
			obj_vpss.u32TfStrength = obj_vpss.u32SfStrength / 2;
			obj_vpss.u32ChromaRange = obj_vpss.u32SfStrength;
			HI_MPI_VPSS_SetGrpParam(0, &obj_vpss);
			printf("HI_MPI_VPSS_SetGrpParam\r\n");
		}
	// obj_drc
		if(obj_drc.bDRCEnable != (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE){
			obj_drc.bDRCEnable = (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE;
			obj_drc.u32StrengthTarget = ispCfgAttr->impCfgAttr.DrcSlope[isDaylight];
			//HI_MPI_ISP_SetDRCAttr(&obj_drc);
			//printf("HI_MPI_ISP_SetDRCAttr\r\n");
		}
	// obj_dp
		if(obj_dp.u16DynamicBadPixelSlope != ispCfgAttr->impCfgAttr.DefectPixelSlope[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelSlope))]){
			obj_dp.u16DynamicBadPixelSlope = ispCfgAttr->impCfgAttr.DefectPixelSlope[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelSlope))];
			obj_dp.u16DynamicBadPixelThresh = ispCfgAttr->impCfgAttr.DefectPixelThresh[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelThresh))];
			HI_MPI_ISP_SetDefectPixelAttr(&obj_dp);
		}

	// obj_chnsp
		if(obj_chnsp.u32LumaGain != ispCfgAttr->impCfgAttr.ChnSp[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.ChnSp[isDaylight]))]){
			obj_chnsp.u32LumaGain = ispCfgAttr->impCfgAttr.ChnSp[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.ChnSp[isDaylight]))];
			HI_MPI_VPSS_SetChnSpParam(0, 1, &obj_chnsp);
			printf("HI_MPI_VPSS_SetChnSpParam\r\n");
		}
	// obj_fps	
	//	printf("[%d]********iso =%d********\n",__LINE__,iso);
		if(48 == obj_fps){
			if(iso <= ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[isDaylight]/3)
			{
				cur_fps = 16;
				fps_cnt_16 += 1;
			}else if((iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight]/3) && (iso <= ispCfgAttr->impCfgAttr.SlowFrameRateHigherHold[isDaylight]/3)){
				cur_fps = 32;					
				fps_cnt_32 += 1;
			}else{
				cur_fps = 48;
				fps_cnt_48 += 1;
			}		
		}else if(32 == obj_fps){
			if(iso > (ispCfgAttr->impCfgAttr.SlowFrameRateHigherHold[isDaylight]/2))
			{
				cur_fps = 48;
				fps_cnt_48 += 1;
			}else if(iso <= ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[isDaylight]/2)
			{
				cur_fps = 16;				
				fps_cnt_16 += 1;
			}else{
				cur_fps = 32;
				fps_cnt_32 += 1;
			}
		}else if(16 == obj_fps){
		
			if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHigherHold[isDaylight]){		
				cur_fps = 48;
				fps_cnt_48 += 1;
			}
			else if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight]){		
				cur_fps = 32;
				fps_cnt_32 += 1;
			}else{
				cur_fps = 16;				
				fps_cnt_16 += 1;
			}
		}else{
			cur_fps = obj_fps;

		}

		fps_cnt += 1;

		if(5 == fps_cnt){
			if( fps_cnt_16 >= 5 || fps_cnt_32 >= 5 || fps_cnt_48 >= 5){
				if(cur_fps != obj_fps && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
				printf("_hi_isp_cfg_set_slow_framerate 0x%x\r\n",cur_fps);
				_hi_isp_cfg_set_slow_framerate(cur_fps);
				}				
			}
			fps_cnt = 0;
			fps_cnt_16 = 0;		
			fps_cnt_32 = 0;
			fps_cnt_48 = 0;
		}

		return 0;
	}
	return -1;

}


int HI_ISP_cfg_get_imp(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	int i = 0;
	ISP_SHARPEN_ATTR_S obj_sharpen;
	ISP_DENOISE_ATTR_S obj_denoise;
	ISP_DEMOSAIC_ATTR_S obj_demosaic;
	ISP_GAMMA_TABLE_S obj_gamma;
	VPSS_GRP_PARAM_S obj_vpss;
	ISP_DRC_ATTR_S obj_drc;
	ISP_DP_ATTR_S obj_dp;

	if(NULL != ispCfgAttr && 0 == HI_MPI_ISP_GetSharpenAttr(&obj_sharpen) && 0 == HI_MPI_ISP_GetDenoiseAttr(&obj_denoise) \
				&& 0 == HI_MPI_ISP_GetDemosaicAttr(&obj_demosaic) && 0 == HI_MPI_ISP_GetGammaTable(&obj_gamma) \
				&& 0 == HI_MPI_VPSS_GetGrpParam(0, &obj_vpss) && 0 == HI_MPI_ISP_GetDRCAttr(&obj_drc) \
				&& 0 == HI_MPI_ISP_GetDefectPixelAttr(&obj_dp))
	{
	// obj_sharpen
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltD[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenAltD[isDaylight][i] = obj_sharpen.u8SharpenAltD[i];
		}
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenAltUd[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenAltUd[isDaylight][i] = obj_sharpen.u8SharpenAltUd[i];
		}
	// obj_denoise
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrThresh[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SnrThresh[isDaylight][i] = obj_denoise.u8SnrThresh[i];
		}
	// obj_demosaic
		ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))] = obj_demosaic.u8UuSlope;
	// obj_gamma
		ispCfgAttr->impCfgAttr.Gamma[isDaylight] = obj_gamma.enGammaCurve;
	// obj_vpss
		ispCfgAttr->impCfgAttr.SfStrength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.SfStrength[isDaylight]))] = obj_vpss.u32SfStrength;
	// obj_drc
		ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight] = 0;
		ispCfgAttr->impCfgAttr.DrcSlope[isDaylight] = (uint8_t)(obj_drc.u32StrengthTarget & 0xFF);
	// obj_dp
		ispCfgAttr->impCfgAttr.DefectPixelSlope[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelSlope))] = obj_dp.u16DynamicBadPixelSlope;
		ispCfgAttr->impCfgAttr.DefectPixelThresh[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelThresh))] = obj_dp.u16DynamicBadPixelThresh;

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

