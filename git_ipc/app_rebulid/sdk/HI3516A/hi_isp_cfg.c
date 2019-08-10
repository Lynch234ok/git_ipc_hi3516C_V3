#include "../sdk_trace.h"

#include "hi_isp_cfg_parse.h"
#include "hi_isp_cfg.h"
#include "hi3516a.h"

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

static int
count_from_iso(int n)
{
	int iso = _isp_get_iso();
	int ret = 0;
	
	if(0 == n || 0 == iso){
		return 0;
	}
	
	if(n >= 8){
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
		if(iso < 16){
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
	if(0 == n || 0 == iso){
		return 0;
	}else if(iso >= 2 && iso <= 128){
		ret = ((int)(log(iso) / log(2) + 0.41) / (8 / n) - 1);
	}else{
		ret = (int)(log(iso) / log(2)) / (8 / n);
	}
	
	if(ret < 0 ){
		ret = 0;
	}else if(ret >= n){
		ret = n - 1;
	}

*/
	return  ret ;
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
	ECHO_FOR_LOOP_2("SharpenRGB", i, j, ispCfgAttr->impCfgAttr.SharpenRGB);
	ECHO_FOR_LOOP_2("SnrThresh", i, j, ispCfgAttr->impCfgAttr.SnrThresh);
	ECHO_FOR_LOOP_2("DemosaicUuSlope", i, j, ispCfgAttr->impCfgAttr.DemosaicUuSlope);
	ECHO_FOR_LOOP_0("Gamma", i, ispCfgAttr->impCfgAttr.Gamma);
	ECHO_FOR_LOOP_2("GlobalStrength", i, j, ispCfgAttr->impCfgAttr.GlobalStrength);
	ECHO_FOR_LOOP_0("DrcThreshold", i, ispCfgAttr->impCfgAttr.DrcThreshold);
	ECHO_FOR_LOOP_0("DrcAutoStrength", i, ispCfgAttr->impCfgAttr.DrcAutoStrength);
	ECHO_FOR_LOOP_0("DrcSlopeMax", i, ispCfgAttr->impCfgAttr.DrcSlopeMax);
	ECHO_FOR_LOOP_0("DrcSlopeMin", i, ispCfgAttr->impCfgAttr.DrcSlopeMin);
	ECHO_FOR_LOOP_0("DrcWhiteLevel", i, ispCfgAttr->impCfgAttr.DrcWhiteLevel);
	ECHO_FOR_LOOP_1("DefectPixelSlope", i, ispCfgAttr->impCfgAttr.DefectPixelSlope);
	ECHO_FOR_LOOP_1("DefectPixelThresh", i, ispCfgAttr->impCfgAttr.DefectPixelThresh);
	ECHO_FOR_LOOP_0("SlowFrameRateHighHold", i, ispCfgAttr->impCfgAttr.SlowFrameRateHighHold);
	ECHO_FOR_LOOP_0("SlowFrameRateLowHold", i, ispCfgAttr->impCfgAttr.SlowFrameRateLowHold);
	ECHO_HEX_DEC("AntiFlikeThreshold", ispCfgAttr->impCfgAttr.AntiFlikeThreshold);	
	ECHO_FOR_LOOP_2("Chroma_SF_Strength", i, j, ispCfgAttr->impCfgAttr.Chroma_SF_Strength);
	ECHO_FOR_LOOP_2("Chroma_TF_Strength", i, j, ispCfgAttr->impCfgAttr.Chroma_TF_Strength);
	ECHO_FOR_LOOP_2("IE_Strength", i, j, ispCfgAttr->impCfgAttr.IE_Strength);
	ECHO_FOR_LOOP_2("Luma_MotionThresh", i, j, ispCfgAttr->impCfgAttr.Luma_MotionThresh);
	ECHO_FOR_LOOP_2("Luma_SF_MoveArea", i, j, ispCfgAttr->impCfgAttr.Luma_SF_MoveArea);
	ECHO_FOR_LOOP_2("Luma_SF_StillArea", i, j, ispCfgAttr->impCfgAttr.Luma_SF_StillArea);
	ECHO_FOR_LOOP_2("Luma_TF_Strength", i, j, ispCfgAttr->impCfgAttr.Luma_TF_Strength);
	ECHO_FOR_LOOP_2("DeSand_Strength", i, j, ispCfgAttr->impCfgAttr.DeSand_Strength);
	
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
	ISP_DRC_ATTR_S DRC_obj;
	ISP_DP_ATTR_S DP_obj;
	if(HI_SUCCESS == HI_MPI_ISP_GetExposureAttr(IspDev, &AE_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetCCMAttr(IspDev, &CCM_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetWBAttr(IspDev, &AWB_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetSaturationAttr(IspDev, &SATURATION_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetBlackLevelAttr(IspDev, &BLACK_LEVEL_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetSharpenAttr(IspDev, &SHARPEN_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetNRAttr(IspDev, &NR_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetGammaAttr(IspDev, &GAMMA_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
			&& HI_SUCCESS == HI_MPI_ISP_GetDPAttr(IspDev, &DP_obj))
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
		ECHO_FOR_LOOP_1("au8SharpenRGB", i, SHARPEN_obj.stAuto.au8SharpenRGB);
	// NR_obj
		printf("\033[31;1m[SNR]\033[0m\n");
		ECHO_FOR_LOOP_1("au8Thresh", i, NR_obj.stAuto.au8Thresh);
	// DEMOSAIC_obj
		printf("\033[31;1m[DEMOSAIC]\033[0m\n");
		ECHO_HEX_DEC("u8UuSlope", DEMOSAIC_obj.u8UuSlope);
	// GAMMA_obj
		printf("\033[31;1m[GAMMA]\033[0m\n");
		ECHO_HEX_DEC("enCurveType", GAMMA_obj.enCurveType);
		ECHO_FOR_LOOP_1("GammaTable", i, GAMMA_obj.u16Table);

	// DRC_obj
		printf("\033[31;1m[DRC]\033[0m\n");
		ECHO_HEX_DEC("bEnable", DRC_obj.bEnable);
		ECHO_HEX_DEC("u32Strength", DRC_obj.stAuto.u32Strength);	
		ECHO_HEX_DEC("u32SlopeMax", DRC_obj.u32SlopeMax);
		ECHO_HEX_DEC("u32SlopeMin", DRC_obj.u32SlopeMin);
		ECHO_HEX_DEC("u32WhiteLevel", DRC_obj.u32WhiteLevel);
	// DP_obj
		printf("\033[31;1m[DefectPixel]\033[0m\n");
		ECHO_HEX_DEC("u16Slope", DP_obj.stDynamicAttr.u16Slope);
		ECHO_HEX_DEC("u16Thresh", DP_obj.stDynamicAttr.u16Thresh);
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


int _hi_isp_cfg_set_slow_framerate(uint8_t bValue,LpIspCfgAttr ispCfgAttr)
{
	uint8_t Value = bValue ? 0x1:0x0;
	VENC_CHN_ATTR_S vencChannelAttr;
	ISP_PUB_ATTR_S stPubAttr;	
	int actual_fps;
	uint8_t  frame_rate;
	int ret = 0;
	int i = 0;
	uint8_t pub_frame_rate;
	
	
	frame_rate = ispCfgAttr->impCfgAttr.src_framerate;
	SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));
//	printf("old_FRate = %f\n",stPubAttr.f32FrameRate);
	pub_frame_rate = (uint8_t)stPubAttr.f32FrameRate;


	for(i = 0; i <= 1; i++){
		ret = HI_MPI_VENC_GetChnAttr(i,&vencChannelAttr);	
	//	printf("enType =%d \n",vencChannelAttr.stVeAttr.enType);
		if(ret != 0){
			return -1;
		}

		actual_fps = frame_rate/(1 << Value);
		if(vencChannelAttr.stVeAttr.enType == PT_H264){

			switch(vencChannelAttr.stRcAttr.enRcMode){
				default:
				case VENC_RC_MODE_H264VBR:
		
			 		if(bValue == true && pub_frame_rate == frame_rate){//slow framerate 		
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate =  actual_fps;
						}	

					}else if (bValue == false && pub_frame_rate < frame_rate ){//actual framerate	
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate =  actual_fps;
						}
					}
					break;
				
				case VENC_RC_MODE_H264CBR:
					if(bValue == true && pub_frame_rate == frame_rate){//slow framerate
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =  actual_fps;
						}		
						
					}else if(bValue == false && pub_frame_rate < frame_rate){//actual framerate
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =  actual_fps;
						}
					}
					break;		
				}
			



		}else if(vencChannelAttr.stVeAttr.enType ==  PT_H265){
			switch(vencChannelAttr.stRcAttr.enRcMode){
				default:
				case VENC_RC_MODE_H265VBR:
					if(bValue == true && pub_frame_rate == frame_rate){//slow framerate 		
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate =	actual_fps;
						}	
			
					}else if (bValue == false && pub_frame_rate < frame_rate ){//actual framerate	
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate =	actual_fps;
						}
					}
					break;
				
				case VENC_RC_MODE_H265CBR:
					if(bValue == true && pub_frame_rate == frame_rate){//slow framerate
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate =	actual_fps;
						}		
						
					}else if(bValue == false && pub_frame_rate < frame_rate){//actual framerate
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate =	actual_fps;
						}
					}
					break;		
				}
		
		}

		HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
		SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));

	}
	
//	printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
	
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
	ISP_DP_ATTR_S DP_obj;
	VPSS_GRP_PARAM_V2_S VpssV2_obj;	
	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetSharpenAttr(IspDev, &SHARPEN_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetNRAttr(IspDev, &NR_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetGammaAttr(IspDev, &GAMMA_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDPAttr(IspDev, &DP_obj) \
				&& HI_SUCCESS == HI_MPI_VPSS_GetGrpParamV2(IspDev,&VpssV2_obj))
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
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenRGB[isDaylight]); i++)
		{
			SHARPEN_obj.stAuto.au8SharpenRGB[i] = ispCfgAttr->impCfgAttr.SharpenRGB[isDaylight][i];
		}
		
	// NR_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrThresh[isDaylight]); i++)
		{
			NR_obj.stAuto.au8Thresh[i] = ispCfgAttr->impCfgAttr.SnrThresh[isDaylight][i];
		}
	// DEMOSAIC_obj
		DEMOSAIC_obj.u8UuSlope = ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))];

	// GAMMA_obj
		GAMMA_obj.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
		for(i = 0; i < ARRAY_NUM(gs_Gamma[ispCfgAttr->impCfgAttr.Gamma[isDaylight]]); i++)
		{
			GAMMA_obj.u16Table[i] = gs_Gamma[ispCfgAttr->impCfgAttr.Gamma[isDaylight]][i];
		}
		

	// DRC_obj
		DRC_obj.bEnable = (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE;
		DRC_obj.stAuto.u32Strength = ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight];
		DRC_obj.u32SlopeMax = ispCfgAttr->impCfgAttr.DrcSlopeMax[isDaylight];
		DRC_obj.u32SlopeMin = ispCfgAttr->impCfgAttr.DrcSlopeMin[isDaylight];
		DRC_obj.u32WhiteLevel = ispCfgAttr->impCfgAttr.DrcWhiteLevel[isDaylight];

	// DP_obj
		DP_obj.stDynamicAttr.u16Slope = ispCfgAttr->impCfgAttr.DefectPixelSlope[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))];
		DP_obj.stDynamicAttr.u16Thresh = ispCfgAttr->impCfgAttr.DefectPixelThresh[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))];

	// VPSS_V2_obj
		VpssV2_obj.Chroma_SF_Strength = ispCfgAttr->impCfgAttr.Chroma_SF_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Chroma_SF_Strength[isDaylight]))];
		VpssV2_obj.Chroma_TF_Strength = ispCfgAttr->impCfgAttr.Chroma_TF_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Chroma_TF_Strength[isDaylight]))];
		VpssV2_obj.IE_Strength = ispCfgAttr->impCfgAttr.IE_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IE_Strength[isDaylight]))];
		VpssV2_obj.Luma_MotionThresh = ispCfgAttr->impCfgAttr.Luma_MotionThresh[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Luma_MotionThresh[isDaylight]))];
		VpssV2_obj.Luma_SF_MoveArea = ispCfgAttr->impCfgAttr.Luma_SF_MoveArea[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Luma_SF_MoveArea[isDaylight]))];
		VpssV2_obj.Luma_SF_StillArea = ispCfgAttr->impCfgAttr.Luma_SF_StillArea[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Luma_SF_StillArea[isDaylight]))];
		VpssV2_obj.Luma_TF_Strength = ispCfgAttr->impCfgAttr.Luma_TF_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Luma_TF_Strength[isDaylight]))];
		VpssV2_obj.DeSand_Strength = ispCfgAttr->impCfgAttr.DeSand_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DeSand_Strength[isDaylight]))];

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
				&& HI_SUCCESS == HI_MPI_ISP_SetDPAttr(IspDev, &DP_obj) \
				&& HI_SUCCESS == HI_MPI_VPSS_SetGrpParamV2(IspDev,&VpssV2_obj)? 0 : -1;
	}
	return -1;
}


int HI_ISP_cfg_set_imp_single(int isDaylight, LpIspCfgAttr ispCfgAttr)
{
	ISP_DEV IspDev = 0;
	int iso = _isp_get_iso();
	int n;
	ISP_DEMOSAIC_ATTR_S DEMOSAIC_obj;
	ISP_DP_ATTR_S DP_obj;
	VPSS_GRP_PARAM_V2_S VpssV2_obj;
	ISP_DRC_ATTR_S DRC_obj;
	ISP_EXPOSURE_ATTR_S AE_obj; 
 	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \

				&& HI_SUCCESS == HI_MPI_ISP_GetDPAttr(IspDev, &DP_obj) \
				&& HI_SUCCESS == HI_MPI_VPSS_GetGrpParamV2(IspDev,&VpssV2_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetExposureAttr(IspDev,&AE_obj))
	{
	// DEMOSAIC_obj
		if(DEMOSAIC_obj.u8UuSlope != ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))]){
			DEMOSAIC_obj.u8UuSlope = ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))];
			HI_MPI_ISP_SetDemosaicAttr(IspDev, &DEMOSAIC_obj);
	//		printf("HI_MPI_ISP_SetDemosaicAttr\r\n");
		}

	// DP_obj
		if(DP_obj.stDynamicAttr.u16Slope != ispCfgAttr->impCfgAttr.DefectPixelSlope[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelSlope))]){
			DP_obj.stDynamicAttr.u16Slope = ispCfgAttr->impCfgAttr.DefectPixelSlope[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelSlope))];
			DP_obj.stDynamicAttr.u16Thresh = ispCfgAttr->impCfgAttr.DefectPixelThresh[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DefectPixelThresh))];
	//		printf("u16Slope =%d  u16Thresh=%d \n ",DP_obj.stDynamicAttr.u16Slope,DP_obj.stDynamicAttr.u16Thresh);
			HI_MPI_ISP_SetDPAttr(IspDev, &DP_obj);
		}
		for(n=0;n<=3;n++){
	//		printf("u16Slope[%d] =%d  u16Thresh[%d]=%d \n ",n,ispCfgAttr->impCfgAttr.DefectPixelSlope[n],n,ispCfgAttr->impCfgAttr.DefectPixelThresh[n]);
		}

	// VpssV2_obj
		static  int cnt_num_old = 0;
	    int cur_cnt_num =  count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Chroma_SF_Strength[isDaylight]));			
		if(cnt_num_old != cur_cnt_num){
			VpssV2_obj.Chroma_SF_Strength = ispCfgAttr->impCfgAttr.Chroma_SF_Strength[isDaylight][cur_cnt_num];
			VpssV2_obj.Chroma_TF_Strength = ispCfgAttr->impCfgAttr.Chroma_TF_Strength[isDaylight][cur_cnt_num];
			VpssV2_obj.IE_Strength = ispCfgAttr->impCfgAttr.IE_Strength[isDaylight][cur_cnt_num];
			VpssV2_obj.Luma_MotionThresh = ispCfgAttr->impCfgAttr.Luma_MotionThresh[isDaylight][cur_cnt_num];
			VpssV2_obj.Luma_SF_MoveArea = ispCfgAttr->impCfgAttr.Luma_SF_MoveArea[isDaylight][cur_cnt_num];
			VpssV2_obj.Luma_SF_StillArea = ispCfgAttr->impCfgAttr.Luma_SF_StillArea[isDaylight][cur_cnt_num];
			VpssV2_obj.Luma_TF_Strength = ispCfgAttr->impCfgAttr.Luma_TF_Strength[isDaylight][cur_cnt_num];
			VpssV2_obj.DeSand_Strength = ispCfgAttr->impCfgAttr.DeSand_Strength[isDaylight][cur_cnt_num];	
			HI_MPI_VPSS_SetGrpParamV2(IspDev,&VpssV2_obj);
			cnt_num_old = cur_cnt_num;
		//	printf("HI_MPI_VPSS_SetGrpParamV2\r\n");
		}	


	// obj_fps	
		if(iso < ispCfgAttr->impCfgAttr.SlowFrameRateLowHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			_hi_isp_cfg_set_slow_framerate(false,ispCfgAttr);
			
		}else if(iso > ispCfgAttr->impCfgAttr.SlowFrameRateHighHold[isDaylight] && ispCfgAttr->impCfgAttr.AutoSlowFrameRate == true){
			_hi_isp_cfg_set_slow_framerate(true,ispCfgAttr);
		}

	//obj_drc
		if(DRC_obj.bEnable != ((iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE)){
			DRC_obj.bEnable = (iso > ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight]) ? HI_FALSE : HI_TRUE;
			HI_MPI_ISP_SetDRCAttr(IspDev, &DRC_obj);
		}

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
	ISP_DP_ATTR_S DP_obj;
	VPSS_GRP_PARAM_V2_S VpssV2_obj;
	if(NULL != ispCfgAttr && HI_SUCCESS == HI_MPI_ISP_GetSharpenAttr(IspDev, &SHARPEN_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetNRAttr(IspDev, &NR_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDemosaicAttr(IspDev, &DEMOSAIC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetGammaAttr(IspDev, &GAMMA_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDRCAttr(IspDev, &DRC_obj) \
				&& HI_SUCCESS == HI_MPI_ISP_GetDPAttr(IspDev, &DP_obj)\
				&& HI_SUCCESS == HI_MPI_VPSS_GetGrpParamV2(IspDev,&VpssV2_obj))
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
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SharpenRGB[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SharpenRGB[isDaylight][i] = SHARPEN_obj.stAuto.au8SharpenUd[i];
		}
	// NR_obj
		for(i = 0; i < ARRAY_NUM(ispCfgAttr->impCfgAttr.SnrThresh[isDaylight]); i++)
		{
			ispCfgAttr->impCfgAttr.SnrThresh[isDaylight][i] = NR_obj.stAuto.au8Thresh[i];
		}
	// DEMOSAIC_obj
		ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))] = DEMOSAIC_obj.u8UuSlope;
	// GAMMA_obj
		ispCfgAttr->impCfgAttr.Gamma[isDaylight] = GAMMA_obj.enCurveType;

	// DRC_obj
		ispCfgAttr->impCfgAttr.DrcThreshold[isDaylight] = 0;
		ispCfgAttr->impCfgAttr.DrcAutoStrength[isDaylight] = DRC_obj.stAuto.u32Strength;
		ispCfgAttr->impCfgAttr.DrcSlopeMax[isDaylight] =  DRC_obj.u32SlopeMax;
		ispCfgAttr->impCfgAttr.DrcSlopeMin[isDaylight] =  DRC_obj.u32SlopeMin;
		ispCfgAttr->impCfgAttr.DrcWhiteLevel[isDaylight] =  DRC_obj.u32WhiteLevel;

		
	// DP_obj
		ispCfgAttr->impCfgAttr.DefectPixelSlope[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))] = DP_obj.stDynamicAttr.u16Slope;
		ispCfgAttr->impCfgAttr.DefectPixelThresh[count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DemosaicUuSlope[isDaylight]))] = DP_obj.stDynamicAttr.u16Thresh;

	// VpssV2_obj
		ispCfgAttr->impCfgAttr.Chroma_SF_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Chroma_SF_Strength[isDaylight]))] = VpssV2_obj.Chroma_SF_Strength;
		ispCfgAttr->impCfgAttr.Chroma_TF_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Chroma_TF_Strength[isDaylight]))] = VpssV2_obj.Chroma_TF_Strength;
		ispCfgAttr->impCfgAttr.IE_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.IE_Strength[isDaylight]))] = VpssV2_obj.IE_Strength;
		ispCfgAttr->impCfgAttr.Luma_MotionThresh[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Luma_MotionThresh[isDaylight]))] = VpssV2_obj.Luma_MotionThresh;
		ispCfgAttr->impCfgAttr.Luma_SF_MoveArea[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Luma_SF_MoveArea[isDaylight]))] = VpssV2_obj.Luma_SF_MoveArea;
		ispCfgAttr->impCfgAttr.Luma_SF_StillArea[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Luma_SF_StillArea[isDaylight]))] = VpssV2_obj.Luma_SF_StillArea;
		ispCfgAttr->impCfgAttr.Luma_TF_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.Luma_TF_Strength[isDaylight]))] = VpssV2_obj.Luma_TF_Strength;
		ispCfgAttr->impCfgAttr.DeSand_Strength[isDaylight][count_from_iso(ARRAY_NUM(ispCfgAttr->impCfgAttr.DeSand_Strength[isDaylight]))] = VpssV2_obj.DeSand_Strength;

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

