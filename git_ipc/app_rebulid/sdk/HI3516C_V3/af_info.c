#include <stdio.h>
#include <string.h>

#include "hi_type.h"
#include "mpi_isp.h"

#define BLEND_SHIFT 6
#define ALPHA 64 // 1
#define BELTA 54 // 0.85

#if 0
static int AFWeight[8][8] = {
	{1,1,1,1,1,1,1,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,1,1,1,1,1,1,1,},
};
#else
static int AFWeight[8][8] = {
	{1,1,1,1,1,1,1,1,},
	{1,1,1,2,2,1,1,1,},
	{1,1,2,2,2,2,1,1,},
	{1,1,2,2,2,2,1,1,},
	{1,1,2,2,2,2,1,1,},
	{1,1,2,2,2,2,1,1,},
	{1,1,1,2,2,1,1,1,},
	{1,1,1,1,1,1,1,1,},
};
#endif

static ISP_FOCUS_STATISTICS_CFG_S stFocusCfg = {
	{1, 8, 8, 1920, 1080, 1, 0, {0}, 1, {1, 1, 240, 240, 0}, {1, 0x9bff}, 0xf0},
	{1, {1, 1, 1}, {188, 414, -330, 486, -461, 400, -328}, {7, 0, 3, 1}, {1, 0, 255, 0, 220, 8, 14}, {127, 12, 2047} },
	{0, {1, 1, 0}, {200, 200, -110, 461, -415, 0, 0}, {6, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0 }, {15, 12, 2047} },
	{{20, 16, 0, -16, -20}, {1, 0, 255, 0, 220, 8, 14}, {38, 12, 1800}},
	{{-12, -24, 0, 24, 12}, {1, 0, 255, 0, 220, 8, 14}, {15, 12, 2047}},
	{4, {0, 0}, {1, 1}, 0}
};

int AFStatistics_Init(void)
{
	HI_S32 s32Ret;

	ISP_DEV IspDev;
	ISP_STATISTICS_CFG_S stIspStaticsCfg;
	ISP_PUB_ATTR_S stPubattr;

	IspDev = 0;
	s32Ret  = HI_MPI_ISP_GetStatisticsConfig(IspDev, &stIspStaticsCfg);
	s32Ret |= HI_MPI_ISP_GetPubAttr(IspDev, &stPubattr);

	stFocusCfg.stConfig.u16Vsize = stPubattr.stWndRect.u32Height;
	stFocusCfg.stConfig.u16Hsize = stPubattr.stWndRect.u32Width;
	stIspStaticsCfg.unKey.bit1AfStat = 0x1;
	if (HI_SUCCESS != s32Ret) {
		printf("HI_MPI_ISP_GetStatisticsConfig error!(s32Ret = 0x%x)\n",s32Ret);
		return -1;
	}

	memcpy(&stIspStaticsCfg.stFocusCfg, &stFocusCfg, sizeof(stFocusCfg));

	s32Ret = HI_MPI_ISP_SetStatisticsConfig(IspDev, &stIspStaticsCfg);
	if (HI_SUCCESS != s32Ret) {
		printf("HI_MPI_ISP_SetStatisticsConfig error!(s32Ret = 0x%x)\n",s32Ret);
		return -1;
	}

	return 0;
}

int AFStatistics_Exit(void)
{

	return 0;
}

int AFStatistics_GetFV(unsigned long * Fv1, unsigned long * Fv2, int TimeOut)
{
	HI_S32 s32Ret;

	ISP_VD_INFO_S stVdInfo;
	ISP_STATISTICS_S stIspStatics;

	ISP_DEV IspDev;

	IspDev = 0;

	s32Ret  = HI_MPI_ISP_GetVDEndTimeOut(IspDev, &stVdInfo, TimeOut);
	if (HI_SUCCESS != s32Ret) {
		printf("HI_MPI_ISP_GetVDEndTimeOut error!(s32Ret = 0x%x)\n",s32Ret);
		return -1;
	}

	s32Ret == HI_MPI_ISP_GetStatistics(IspDev, &stIspStatics);
	if (HI_SUCCESS != s32Ret) {
		printf("HI_MPI_ISP_GetStatistics error!(s32Ret = 0x%x)\n",s32Ret);
		return -1;
	}

	{
		HI_U32 u32SumFv1 = 0;
		HI_U32 u32SumFv2 = 0;
		HI_U32 u32WgtSum = 0;
		HI_U32 u32Fv1_n, u32Fv2_n, u32Fv1, u32Fv2;
		int ii, jj;

		for ( ii = 0 ; ii < stFocusCfg.stConfig.u16Vwnd; ii ++ ) {
			for ( jj = 0 ; jj < stFocusCfg.stConfig.u16Hwnd; jj ++ ) {
				HI_U32 u32H1 = stIspStatics.stFocusStat.stZoneMetrics[ii][jj].u16h1;
				HI_U32 u32H2 = stIspStatics.stFocusStat.stZoneMetrics[ii][jj].u16h2;
				HI_U32 u32V1 = stIspStatics.stFocusStat.stZoneMetrics[ii][jj].u16v1;
				HI_U32 u32V2 = stIspStatics.stFocusStat.stZoneMetrics[ii][jj].u16v2;

				u32Fv1_n = (u32H1 * ALPHA + u32V1 * ((1<<BLEND_SHIFT) - ALPHA)) >> BLEND_SHIFT;
				u32Fv2_n = (u32H2 * BELTA + u32V2 * ((1<<BLEND_SHIFT) - BELTA)) >> BLEND_SHIFT;

				u32SumFv1 += AFWeight[ii][jj] * u32Fv1_n;
				u32SumFv2 += AFWeight[ii][jj] * u32Fv2_n;
				u32WgtSum += AFWeight[ii][jj];
			}

			u32Fv1 = u32SumFv1 / u32WgtSum;
			u32Fv2 = u32SumFv2 / u32WgtSum;

			if(Fv1) {
				*Fv1 = u32Fv1;
			}

			if(Fv2) {
				*Fv2 = u32Fv2;
			}
		}
	}

	return 0;
}

int af_test(void)
{
	unsigned long Fv1 = 0;
	unsigned long Fv2 = 0;

	if(0 != AFStatistics_Init()) {
		return -1;
	}

	if(0 == AFStatistics_GetFV(&Fv1, &Fv2, 5000)) {
		printf("\r\n AF [%d, %d] \r\n", Fv1, Fv2);
	}

	AFStatistics_Exit();

	return HI_SUCCESS;
}
