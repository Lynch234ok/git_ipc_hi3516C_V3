
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "hi_defines.h"
#include "hi_comm_isp.h"
#include "hi_comm_vpss.h"
#include "mpi_isp.h"
#include "mpi_vpss.h"
#include "mpi_ae.h"

#include "isp_nrx_auto.h"

static __inline int iClip2(int x, int b) {{ if (x < 0) x = 0; };{ if (x > b) x = b; }; return x; }
static __inline int iMin2(int a, int b) {{ if (a > b) a = b; }; return a; }
static __inline int iMax2(int a, int b) {{ if (a > b) b = a; }; return b; }

static __inline HI_U32 MapISO(HI_U32 iso)
{
    HI_U32   j,  i = (iso >= 200);

    if (iso < 72) return (HI_U32)iMax2(iso, -3);

    i += ( (iso >= (200 << 1)) + (iso >= (400 << 1)) + (iso >= (400 << 2)) + (iso >= (400 << 3)) + (iso >= (400 << 4)) );
    i += ( (iso >= (400 << 5)) + (iso >= (400 << 6)) + (iso >= (400 << 7)) + (iso >= (400 << 8)) + (iso >= (400 << 9)) );
    i += ( (iso >= (400 << 10))+ (iso >= (400 << 11))+ (iso >= (400 << 12))+ (iso >= (400 << 13))+ (iso >= (400 << 14)));
    j  = ( (iso >  (112 << i)) + (iso >  (125 << i)) + (iso >  (141 << i)) + (iso >  (158 << i)) + (iso >  (178 << i)) );

    return (i * 6 + j + (iso >= 80) + (iso >= 90) + (iso >= 100) - 3);
}

typedef struct hiSCENEAUTO_INIPARAM_NRS_X
{
    /* sharpen */
	HI_S32 s32IES0;
	HI_S32 s32IEF[4];

	/* spacial filter */
	HI_S32 s32SBS0[3];
	HI_S32 s32SBS1[3];
	HI_S32 s32SBS2[3];
	HI_S32 s32SBS3[3];
	HI_S32 s32SDS0[3];
	HI_S32 s32SDS1[3];
	HI_S32 s32SDS2[3];
	HI_S32 s32SDS3[3];
	HI_S32 s32STH0[3];
	HI_S32 s32STH1[3];
	HI_S32 s32STH2[3];
	HI_S32 s32STH3[3];
	HI_S32 s32SBF0[3];
	HI_S32 s32SBF1[3];
	HI_S32 s32SBF2[3];
	HI_S32 s32SBF3[3];
	HI_S32 s32SFR0;
	HI_S32 s32SFR1;
	HI_S32 s32SFR2;
	HI_S32 s32SFR3;
	HI_S32 s32STR0;
	HI_S32 s32STR1;
	HI_S32 s32STR2;

	/*motion detetion  */
	HI_S32 s32MATH1;
	HI_S32 s32MATH2;
	HI_S32 s32MATE1;
	HI_S32 s32MATE2;
	HI_S32 s32MABW1;
	HI_S32 s32MABW2;
	HI_S32 s32MATW1;
	HI_S32 s32MATW2;

    /*time filter */
	HI_S32 s32TFS1;
	HI_S32 s32TFS2;
    HI_S32 s32TFR1[2];
	HI_S32 s32TFR2[2];
	HI_S32 s32TSR1;
	HI_S32 s32TSR2;
	HI_S32 s32TSS1;
	HI_S32 s32TSS2;
	HI_S32 s32TSDZ1;
	HI_S32 s32TSDZ2;

	/* color filter */
	HI_S32 s32SFC;
	HI_S32 s32TFC;
	HI_S32 s32TPC;
	HI_S32 s32TRC;
} SCENEAUTO_INIPARAM_NRS_X, ADPT_SCENEAUTO_3DNR_ATTR_X, SCENEAUTO_INIPARAM_3DNRCFG_X;

typedef struct hiSCENEAUTO_INIPARAM_3DNR_X
{
    HI_BOOL bUsed;
    HI_BOOL BoolRefMGValue;
    HI_U32 u323DnrIsoCount;
    HI_U32 pu323DnrIsoThresh[16];
    SCENEAUTO_INIPARAM_NRS_X pst3dnrParam[16];
} SCENEAUTO_INIPARAM_3DNR_X;

HI_VOID Interpolate_X(SCENEAUTO_INIPARAM_NRS_X *pst3dnrcfg, HI_U32 u32Mid,
                                const SCENEAUTO_INIPARAM_NRS_X *pstL3dnrcfg, HI_U32 u32Left,
                                const SCENEAUTO_INIPARAM_NRS_X *pstR3dnrcfg, HI_U32 u32Right)
{
    int j = 0;
    int k, left, right, i = ((u32Mid > 3) ? MapISO(u32Mid) : iMin2(95,-u32Mid));
    left  = ((u32Left  > 3) ? MapISO(u32Left) : iMin2(95,-u32Left) ); if (i <= left)  { *pst3dnrcfg = *pstL3dnrcfg; return; }
    right = ((u32Right > 3) ? MapISO(u32Right): iMin2(95,-u32Right)); if (i >= right) { *pst3dnrcfg = *pstR3dnrcfg; return; }
    k = (right - left); *pst3dnrcfg = *(( (i+((k * 3) >> 2)) < right ) ? pstL3dnrcfg : pstR3dnrcfg);
    pst3dnrcfg->s32SFC = ( ((right - i) * pstL3dnrcfg->s32SFC + (i - left) * pstR3dnrcfg->s32SFC + (k >> 1)) / k );
    pst3dnrcfg->s32TFC = ( ((right - i) * pstL3dnrcfg->s32TFC + (i - left) * pstR3dnrcfg->s32TFC + (k >> 1)) / k );
    pst3dnrcfg->s32TPC = ( ((right - i) * pstL3dnrcfg->s32TPC + (i - left) * pstR3dnrcfg->s32TPC + (k >> 1)) / k );
    pst3dnrcfg->s32TRC = ( ((right - i) * pstL3dnrcfg->s32TRC + (i - left) * pstR3dnrcfg->s32TRC + (k >> 1)) / k );
    pst3dnrcfg->s32IES0 = ( ((right - i) * pstL3dnrcfg->s32IES0 + (i - left) * pstR3dnrcfg->s32IES0 + (k >> 1)) / k );

    for(j = 0;j<3;j++) {
        pst3dnrcfg->s32SBS0[j] = ( ((right - i) * pstL3dnrcfg->s32SBS0[j] + (i - left) * pstR3dnrcfg->s32SBS0[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SBS1[j] = ( ((right - i) * pstL3dnrcfg->s32SBS1[j] + (i - left) * pstR3dnrcfg->s32SBS1[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SBS2[j] = ( ((right - i) * pstL3dnrcfg->s32SBS2[j] + (i - left) * pstR3dnrcfg->s32SBS2[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SBS3[j] = ( ((right - i) * pstL3dnrcfg->s32SBS3[j] + (i - left) * pstR3dnrcfg->s32SBS3[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SDS0[j] = ( ((right - i) * pstL3dnrcfg->s32SDS0[j] + (i - left) * pstR3dnrcfg->s32SDS0[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SDS1[j] = ( ((right - i) * pstL3dnrcfg->s32SDS1[j] + (i - left) * pstR3dnrcfg->s32SDS1[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SDS2[j] = ( ((right - i) * pstL3dnrcfg->s32SDS2[j] + (i - left) * pstR3dnrcfg->s32SDS2[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SDS3[j] = ( ((right - i) * pstL3dnrcfg->s32SDS3[j] + (i - left) * pstR3dnrcfg->s32SDS3[j] + (k >> 1)) / k );
        pst3dnrcfg->s32STH0[j] = ( ((right - i) * pstL3dnrcfg->s32STH0[j] + (i - left) * pstR3dnrcfg->s32STH0[j] + (k >> 1)) / k );
        pst3dnrcfg->s32STH1[j] = ( ((right - i) * pstL3dnrcfg->s32STH1[j] + (i - left) * pstR3dnrcfg->s32STH1[j] + (k >> 1)) / k );
        pst3dnrcfg->s32STH2[j] = ( ((right - i) * pstL3dnrcfg->s32STH2[j] + (i - left) * pstR3dnrcfg->s32STH2[j] + (k >> 1)) / k );
        pst3dnrcfg->s32STH3[j] = ( ((right - i) * pstL3dnrcfg->s32STH3[j] + (i - left) * pstR3dnrcfg->s32STH3[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SBF0[j] = ( ((right - i) * pstL3dnrcfg->s32SBF0[j] + (i - left) * pstR3dnrcfg->s32SBF0[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SBF1[j] = ( ((right - i) * pstL3dnrcfg->s32SBF1[j] + (i - left) * pstR3dnrcfg->s32SBF1[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SBF2[j] = ( ((right - i) * pstL3dnrcfg->s32SBF2[j] + (i - left) * pstR3dnrcfg->s32SBF2[j] + (k >> 1)) / k );
        pst3dnrcfg->s32SBF3[j] = ( ((right - i) * pstL3dnrcfg->s32SBF3[j] + (i - left) * pstR3dnrcfg->s32SBF3[j] + (k >> 1)) / k );
    }

    for(j=0;j<4;j++) {
        pst3dnrcfg->s32IEF[j] = ( ((right - i) * pstL3dnrcfg->s32IEF[j] + (i - left) * pstR3dnrcfg->s32IEF[j] + (k >> 1)) / k );
    }

	pst3dnrcfg->s32SFR0	 = ( ((right - i) * pstL3dnrcfg->s32SFR0	+ (i - left) * pstR3dnrcfg->s32SFR0	  + (k >> 1)) / k );
	pst3dnrcfg->s32SFR1	 = ( ((right - i) * pstL3dnrcfg->s32SFR1	+ (i - left) * pstR3dnrcfg->s32SFR1	  + (k >> 1)) / k );
	pst3dnrcfg->s32SFR2	 = ( ((right - i) * pstL3dnrcfg->s32SFR2	+ (i - left) * pstR3dnrcfg->s32SFR2	  + (k >> 1)) / k );
	pst3dnrcfg->s32SFR3	 = ( ((right - i) * pstL3dnrcfg->s32SFR3	+ (i - left) * pstR3dnrcfg->s32SFR3	  + (k >> 1)) / k );
	pst3dnrcfg->s32STR0	 = ( ((right - i) * pstL3dnrcfg->s32STR0	+ (i - left) * pstR3dnrcfg->s32STR0	  + (k >> 1)) / k );
	pst3dnrcfg->s32STR1	 = ( ((right - i) * pstL3dnrcfg->s32STR1	+ (i - left) * pstR3dnrcfg->s32STR1	  + (k >> 1)) / k );
	pst3dnrcfg->s32STR2	 = ( ((right - i) * pstL3dnrcfg->s32STR2	+ (i - left) * pstR3dnrcfg->s32STR2	  + (k >> 1)) / k );
	pst3dnrcfg->s32MATH1 = ( ((right - i) * pstL3dnrcfg->s32MATH1 	+ (i - left) * pstR3dnrcfg->s32MATH1  + (k >> 1)) / k );
	pst3dnrcfg->s32MATH2 = ( ((right - i) * pstL3dnrcfg->s32MATH2 	+ (i - left) * pstR3dnrcfg->s32MATH2  + (k >> 1)) / k );
	pst3dnrcfg->s32MATE1 = ( ((right - i) * pstL3dnrcfg->s32MATE1 	+ (i - left) * pstR3dnrcfg->s32MATE1  + (k >> 1)) / k );
	pst3dnrcfg->s32MATE2 = ( ((right - i) * pstL3dnrcfg->s32MATE2 	+ (i - left) * pstR3dnrcfg->s32MATE2  + (k >> 1)) / k );
	pst3dnrcfg->s32MABW1 = ( ((right - i) * pstL3dnrcfg->s32MABW1 	+ (i - left) * pstR3dnrcfg->s32MABW1  + (k >> 1)) / k );
	pst3dnrcfg->s32MABW2 = ( ((right - i) * pstL3dnrcfg->s32MABW2 	+ (i - left) * pstR3dnrcfg->s32MABW2  + (k >> 1)) / k );
	pst3dnrcfg->s32MATW1 = ( ((right - i) * pstL3dnrcfg->s32MATW1 	+ (i - left) * pstR3dnrcfg->s32MATW1  + (k >> 1)) / k );
	pst3dnrcfg->s32MATW2 = ( ((right - i) * pstL3dnrcfg->s32MATW2 	+ (i - left) * pstR3dnrcfg->s32MATW2  + (k >> 1)) / k );
	pst3dnrcfg->s32TFS1	 = ( ((right - i) * pstL3dnrcfg->s32TFS1	+ (i - left) * pstR3dnrcfg->s32TFS1	  + (k >> 1)) / k );
	pst3dnrcfg->s32TFS2	 = ( ((right - i) * pstL3dnrcfg->s32TFS2	+ (i - left) * pstR3dnrcfg->s32TFS2	  + (k >> 1)) / k );
	pst3dnrcfg->s32TSR1	 = ( ((right - i) * pstL3dnrcfg->s32TSR1	+ (i - left) * pstR3dnrcfg->s32TSR1	  + (k >> 1)) / k );
	pst3dnrcfg->s32TSR2	 = ( ((right - i) * pstL3dnrcfg->s32TSR2	+ (i - left) * pstR3dnrcfg->s32TSR2	  + (k >> 1)) / k );
	pst3dnrcfg->s32TSS1	 = ( ((right - i) * pstL3dnrcfg->s32TSS1	+ (i - left) * pstR3dnrcfg->s32TSS1	  + (k >> 1)) / k );
	pst3dnrcfg->s32TSS2	 = ( ((right - i) * pstL3dnrcfg->s32TSS2	+ (i - left) * pstR3dnrcfg->s32TSS2	  + (k >> 1)) / k );
	pst3dnrcfg->s32TSDZ1 = ( ((right - i) * pstL3dnrcfg->s32TSDZ1 	+ (i - left) * pstR3dnrcfg->s32TSDZ1  + (k >> 1)) / k );
	pst3dnrcfg->s32TSDZ2 = ( ((right - i) * pstL3dnrcfg->s32TSDZ2 	+ (i - left) * pstR3dnrcfg->s32TSDZ2  + (k >> 1)) / k );
	pst3dnrcfg->s32SFC	 = ( ((right - i) * pstL3dnrcfg->s32SFC	 	+ (i - left) * pstR3dnrcfg->s32SFC	  + (k >> 1)) / k );
	pst3dnrcfg->s32TFC	 = ( ((right - i) * pstL3dnrcfg->s32TFC	 	+ (i - left) * pstR3dnrcfg->s32TFC	  + (k >> 1)) / k );
	pst3dnrcfg->s32TPC	 = ( ((right - i) * pstL3dnrcfg->s32TPC	 	+ (i - left) * pstR3dnrcfg->s32TPC	  + (k >> 1)) / k );
	pst3dnrcfg->s32TRC	 = ( ((right - i) * pstL3dnrcfg->s32TRC	 	+ (i - left) * pstR3dnrcfg->s32TRC	  + (k >> 1)) / k );
	pst3dnrcfg->s32TFR1[0] = ( ((right - i) * pstL3dnrcfg->s32TFR1[0] + (i - left) * pstR3dnrcfg->s32TFR1[0] + (k >> 1)) / k );
	pst3dnrcfg->s32TFR1[1] = ( ((right - i) * pstL3dnrcfg->s32TFR1[1] + (i - left) * pstR3dnrcfg->s32TFR1[1] + (k >> 1)) / k );
	pst3dnrcfg->s32TFR2[0] = ( ((right - i) * pstL3dnrcfg->s32TFR2[0] + (i - left) * pstR3dnrcfg->s32TFR2[0] + (k >> 1)) / k );
	pst3dnrcfg->s32TFR2[1] = ( ((right - i) * pstL3dnrcfg->s32TFR2[1] + (i - left) * pstR3dnrcfg->s32TFR2[1] + (k >> 1)) / k );
}

HI_S32 CommSceneautoSet3DNRAttr_X(HI_S32 s32VpssGrp, ADPT_SCENEAUTO_3DNR_ATTR_X *pstAdpt3dnrAttr)
{
	int i;
	int ret;

	VPSS_GRP_NRX_PARAM_S stNRSParam;
    stNRSParam.enNRVer = VPSS_NR_V2;
	HI_MPI_VPSS_GetGrpNRXParam(s32VpssGrp,&stNRSParam);

    stNRSParam.enNRVer = VPSS_NR_V2;
    stNRSParam.stNRXParam_V2.IEy.IES = pstAdpt3dnrAttr->s32IES0;

    for(i = 0;i<3;i++) {
        stNRSParam.stNRXParam_V2.IEy.IEF[i] = pstAdpt3dnrAttr->s32IEF[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[0].SBS[i] = pstAdpt3dnrAttr->s32SBS0[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[1].SBS[i] = pstAdpt3dnrAttr->s32SBS1[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[2].SBS[i] = pstAdpt3dnrAttr->s32SBS2[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[3].SBS[i] = pstAdpt3dnrAttr->s32SBS3[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[0].SDS[i] = pstAdpt3dnrAttr->s32SDS0[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[1].SDS[i] = pstAdpt3dnrAttr->s32SDS1[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[2].SDS[i] = pstAdpt3dnrAttr->s32SDS2[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[3].SDS[i] = pstAdpt3dnrAttr->s32SDS3[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[0].STH[i] = pstAdpt3dnrAttr->s32STH0[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[1].STH[i] = pstAdpt3dnrAttr->s32STH1[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[2].STH[i] = pstAdpt3dnrAttr->s32STH2[i];
        stNRSParam.stNRXParam_V2.SFy.SFySub[3].STH[i] = pstAdpt3dnrAttr->s32STH3[i];
    }

    stNRSParam.stNRXParam_V2.IEy.IEF[3] = pstAdpt3dnrAttr->s32IEF[3];
    stNRSParam.stNRXParam_V2.SFy.SFySub[0].SBF0 = pstAdpt3dnrAttr->s32SBF0[0];
    stNRSParam.stNRXParam_V2.SFy.SFySub[0].SBF1 = pstAdpt3dnrAttr->s32SBF0[1];
    stNRSParam.stNRXParam_V2.SFy.SFySub[0].SBF2 = pstAdpt3dnrAttr->s32SBF0[2];
    stNRSParam.stNRXParam_V2.SFy.SFySub[1].SBF0 = pstAdpt3dnrAttr->s32SBF1[0];
    stNRSParam.stNRXParam_V2.SFy.SFySub[1].SBF1 = pstAdpt3dnrAttr->s32SBF1[1];
    stNRSParam.stNRXParam_V2.SFy.SFySub[1].SBF2 = pstAdpt3dnrAttr->s32SBF1[2];
    stNRSParam.stNRXParam_V2.SFy.SFySub[2].SBF0 = pstAdpt3dnrAttr->s32SBF2[0];
    stNRSParam.stNRXParam_V2.SFy.SFySub[2].SBF1 = pstAdpt3dnrAttr->s32SBF2[1];
    stNRSParam.stNRXParam_V2.SFy.SFySub[2].SBF2 = pstAdpt3dnrAttr->s32SBF2[2];
    stNRSParam.stNRXParam_V2.SFy.SFySub[3].SBF0 = pstAdpt3dnrAttr->s32SBF3[0];
    stNRSParam.stNRXParam_V2.SFy.SFySub[3].SBF1 = pstAdpt3dnrAttr->s32SBF3[1];
    stNRSParam.stNRXParam_V2.SFy.SFySub[3].SBF2 = pstAdpt3dnrAttr->s32SBF3[2];
    stNRSParam.stNRXParam_V2.SFy.SFySub[0].SFR = pstAdpt3dnrAttr->s32SFR0;
    stNRSParam.stNRXParam_V2.SFy.SFySub[1].SFR = pstAdpt3dnrAttr->s32SFR1;
    stNRSParam.stNRXParam_V2.SFy.SFySub[2].SFR = pstAdpt3dnrAttr->s32SFR2;
    stNRSParam.stNRXParam_V2.SFy.SFySub[3].SFR = pstAdpt3dnrAttr->s32SFR3;
    stNRSParam.stNRXParam_V2.TFy[0].STR = pstAdpt3dnrAttr->s32STR0;
    stNRSParam.stNRXParam_V2.TFy[1].STR = pstAdpt3dnrAttr->s32STR1;
    stNRSParam.stNRXParam_V2.TFy[2].STR = pstAdpt3dnrAttr->s32STR2;
    stNRSParam.stNRXParam_V2.TFy[1].TFS = pstAdpt3dnrAttr->s32TFS1;
    stNRSParam.stNRXParam_V2.TFy[2].TFS = pstAdpt3dnrAttr->s32TFS2;
    stNRSParam.stNRXParam_V2.TFy[1].TFR0 = pstAdpt3dnrAttr->s32TFR1[0];
    stNRSParam.stNRXParam_V2.TFy[1].TFR1 = pstAdpt3dnrAttr->s32TFR1[1];
    stNRSParam.stNRXParam_V2.TFy[2].TFR0 = pstAdpt3dnrAttr->s32TFR2[0];
    stNRSParam.stNRXParam_V2.TFy[2].TFR1 = pstAdpt3dnrAttr->s32TFR2[1];
    stNRSParam.stNRXParam_V2.TFy[1].TSR = pstAdpt3dnrAttr->s32TSR1;
    stNRSParam.stNRXParam_V2.TFy[2].TSR = pstAdpt3dnrAttr->s32TSR2;
    stNRSParam.stNRXParam_V2.TFy[1].TSS = pstAdpt3dnrAttr->s32TSS1;
    stNRSParam.stNRXParam_V2.TFy[2].TSS = pstAdpt3dnrAttr->s32TSS2;
    stNRSParam.stNRXParam_V2.TFy[1].TSDZ = pstAdpt3dnrAttr->s32TSDZ1;
    stNRSParam.stNRXParam_V2.TFy[2].TSDZ = pstAdpt3dnrAttr->s32TSDZ2;
    stNRSParam.stNRXParam_V2.MDy[1].MATH = pstAdpt3dnrAttr->s32MATH1;
    stNRSParam.stNRXParam_V2.MDy[2].MATH = pstAdpt3dnrAttr->s32MATH2;
    stNRSParam.stNRXParam_V2.MDy[1].MATE = pstAdpt3dnrAttr->s32MATE1;
    stNRSParam.stNRXParam_V2.MDy[2].MATE = pstAdpt3dnrAttr->s32MATE2;
    stNRSParam.stNRXParam_V2.MDy[1].MABW = pstAdpt3dnrAttr->s32MABW1;
    stNRSParam.stNRXParam_V2.MDy[2].MABW = pstAdpt3dnrAttr->s32MABW2;
    stNRSParam.stNRXParam_V2.MDy[1].MATW = pstAdpt3dnrAttr->s32MATW1;
    stNRSParam.stNRXParam_V2.MDy[2].MATW = pstAdpt3dnrAttr->s32MATW2;
    stNRSParam.stNRXParam_V2.NRc.SFC = pstAdpt3dnrAttr->s32SFC;
    stNRSParam.stNRXParam_V2.NRc.TRC = pstAdpt3dnrAttr->s32TRC;
    stNRSParam.stNRXParam_V2.NRc.TFC = pstAdpt3dnrAttr->s32TFC;
    stNRSParam.stNRXParam_V2.NRc.TPC = pstAdpt3dnrAttr->s32TPC;

    ret = HI_MPI_VPSS_SetGrpNRXParam(s32VpssGrp,&stNRSParam);
    if(ret) {
        printf("HI_MPI_VPSS_SetGrpNRXParam %d\n",ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Sceneauto_Set3DNR_X(HI_S32 s32VpssGrp, HI_U32 u32Iso, const SCENEAUTO_INIPARAM_3DNR_X st3dnrparam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;
    HI_S32 s32IsoLevel,s32IsoLevel1;
	SCENEAUTO_INIPARAM_NRS_X stSceneauto3dnr;

    s32IsoLevel = st3dnrparam.u323DnrIsoCount - 1;
    for (i = 0; i < st3dnrparam.u323DnrIsoCount; i++) {
       if (u32Iso <= st3dnrparam.pu323DnrIsoThresh[i]) {
    	   s32IsoLevel = i;
    	   break;
       }
    }

    s32IsoLevel1 = s32IsoLevel - 1;
    if (s32IsoLevel1 < 0) {
        s32IsoLevel1 = 0;
    }

    Interpolate_X(&stSceneauto3dnr, u32Iso,
    	   &st3dnrparam.pst3dnrParam[s32IsoLevel1], st3dnrparam.pu323DnrIsoThresh[s32IsoLevel1],
    	   &st3dnrparam.pst3dnrParam[s32IsoLevel], st3dnrparam.pu323DnrIsoThresh[s32IsoLevel]);

    s32Ret = CommSceneautoSet3DNRAttr_X(s32VpssGrp, &stSceneauto3dnr);
    if (HI_SUCCESS != s32Ret) {
        printf("CommSceneautoSet3DNR_X failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

extern int IsDayNight(void);

SCENEAUTO_INIPARAM_3DNR_X gDefaultNrX_SC2232[2] = {
	{ //Day Time
		.bUsed           = 1,
		.BoolRefMGValue  = HI_FALSE,
		.u323DnrIsoCount = 6,
		.pu323DnrIsoThresh = {102, 200, 400, 800, 1600, 3100},
		.pst3dnrParam = {
			{ //ISO100
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {6, 17, 12, 2},

			/*spacial filter*/
			.s32SBS0   = { 50, 100,  50},
			.s32SBS1   = { 80,  80, 0},
			.s32SBS2   = {  0,   0,  80},
			.s32SBS3   = {100, 100, 100},
			.s32SDS0   = { 50, 100,  50},
			.s32SDS1   = { 80,  80, 0},
			.s32SDS2   = {  0,   0,  80},
			.s32SDS3   = {100, 100, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 3},
			.s32SBF3   = {0, 0, 2},

			.s32SFR0  = 25,
			.s32SFR1  = 15,
			.s32SFR2  = 15,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 150,
			.s32MATH2 = 200,
			.s32MATE1 = 3,
			.s32MATE2 = 2,
			.s32MABW1 = 3,
			.s32MABW2 = 3,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 8,
			.s32TFS2 = 7,
			.s32TFR1 = {28, 28},
			.s32TFR2 = {28, 28},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 255,
			.s32TFC = 12,
			.s32TPC = 8,
			.s32TRC = 255,
			},

			{ //ISO200
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {6, 25, 12, 2},

			/*spacial filter*/
			.s32SBS0   = {150, 150, 150},
			.s32SBS1   = {100, 100, 0},
			.s32SBS2   = {  0,   0, 100},
			.s32SBS3   = {100, 100, 100},
			.s32SDS0   = {150, 150, 150},
			.s32SDS1   = {100, 100, 0},
			.s32SDS2   = {  0,   0, 100},
			.s32SDS3   = {100, 100, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 3},
			.s32SBF3   = {0, 0, 2},

			.s32SFR0  = 25,
			.s32SFR1  = 15,
			.s32SFR2  = 15,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 200,
			.s32MATH2 = 250,
			.s32MATE1 = 3,
			.s32MATE2 = 2,
			.s32MABW1 = 3,
			.s32MABW2 = 3,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 8,
			.s32TFS2 = 7,
			.s32TFR1 = {28, 28},
			.s32TFR2 = {28, 28},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 80,
			.s32TFC = 12,
			.s32TPC = 8,
			.s32TRC = 80,
			},

			{ //ISO400
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {7, 20, 6, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 250, 100},
			.s32SBS1   = {150, 150, 0},
			.s32SBS2   = {  0,   0, 150},
			.s32SBS3   = {100, 100, 100},
			.s32SDS0   = {100, 100, 100},
			.s32SDS1   = {150, 150, 0},
			.s32SDS2   = {  0,   0, 150},
			.s32SDS3   = {100, 100, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 0},
			.s32SBF3   = {0, 0, 2},

			.s32SFR0  = 31,
			.s32SFR1  = 20,
			.s32SFR2  = 20,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 150,
			.s32MATH2 = 300,
			.s32MATE1 = 5,
			.s32MATE2 = 1,
			.s32MABW1 = 3,
			.s32MABW2 = 3,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 11,
			.s32TFS2 = 9,
			.s32TFR1 = {28, 29},
			.s32TFR2 = {28, 28},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 80,
			.s32TFC = 12,
			.s32TPC = 8,
			.s32TRC = 80,
			},

			{ //ISO800
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {5, 20, 6, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 200, 100},
			.s32SBS1   = {150, 150, 0},
			.s32SBS2   = {  0,   0, 150},
			.s32SBS3   = {100, 100, 100},
			.s32SDS0   = {100, 100, 100},
			.s32SDS1   = {150, 150, 0},
			.s32SDS2   = {  0,   0, 150},
			.s32SDS3   = {100, 100, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 0},
			.s32SBF3   = {0, 0, 2},

			.s32SFR0  = 31,
			.s32SFR1  = 20,
			.s32SFR2  = 20,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 17,
			.s32STR2  = 17,

			/*motion detetion*/
			.s32MATH1 = 150,
			.s32MATH2 = 300,
			.s32MATE1 = 5,
			.s32MATE2 = 1,
			.s32MABW1 = 3,
			.s32MABW2 = 3,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 12,
			.s32TFS2 = 9,
			.s32TFR1 = {28, 29},
			.s32TFR2 = {28, 28},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 100,
			.s32TFC = 18,
			.s32TPC = 16,
			.s32TRC = 100,
			},

			{ //ISO1600
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {5, 24, 5, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 200, 100},
			.s32SBS1   = {200, 200, 0},
			.s32SBS2   = {  0,   0, 200},
			.s32SBS3   = {100, 100, 100},
			.s32SDS0   = {100, 100, 100},
			.s32SDS1   = {200, 200, 0},
			.s32SDS2   = {  0,   0, 200},
			.s32SDS3   = {100, 100, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 0},
			.s32SBF3   = {0, 0, 3},

			.s32SFR0  = 31,
			.s32SFR1  = 22,
			.s32SFR2  = 23,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 17,
			.s32STR2  = 5,

			/*motion detetion*/
			.s32MATH1 = 160,
			.s32MATH2 = 400,
			.s32MATE1 = 5,
			.s32MATE2 = 2,
			.s32MABW1 = 3,
			.s32MABW2 = 3,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 14,
			.s32TFS2 = 9,
			.s32TFR1 = {29, 29},
			.s32TFR2 = {31, 31},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 0,
			.s32TSS2 = 0,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 200,
			.s32TFC = 32,
			.s32TPC = 32,
			.s32TRC = 200,
			},

			{ //ISO3100
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {9, 25, 6, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 250, 100},
			.s32SBS1   = {150, 150, 0},
			.s32SBS2   = {  0,   0, 150},
			.s32SBS3   = {200, 200, 200},
			.s32SDS0   = {100, 100, 100},
			.s32SDS1   = {150, 150, 0},
			.s32SDS2   = {  0,   0, 150},
			.s32SDS3   = {200, 200, 200},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 3},
			.s32SBF3   = {0, 0, 2},

			.s32SFR0  = 31,
			.s32SFR1  = 20,
			.s32SFR2  = 20,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 250,
			.s32MATH2 = 300,
			.s32MATE1 = 5,
			.s32MATE2 = 1,
			.s32MABW1 = 3,
			.s32MABW2 = 3,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 12,
			.s32TFS2 = 9,
			.s32TFR1 = {28, 29},
			.s32TFR2 = {28, 28},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 80,
			.s32TFC = 12,
			.s32TPC = 8,
			.s32TRC = 80,
			}
		},
	}, { //Night Time
		.bUsed           = 1,
		.BoolRefMGValue  = HI_FALSE,
		.u323DnrIsoCount = 5,
		.pu323DnrIsoThresh = {200, 400, 800, 1600, 3100},
		.pst3dnrParam = {
			{ //ISO200
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {10, 20, 4, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 200, 100},
			.s32SBS1   = {100, 100, 0},
			.s32SBS2   = {  0,   0, 100},
			.s32SBS3   = { 33,  33, 100},
			.s32SDS0   = {100, 100, 100},
			.s32SDS1   = {100, 100, 0},
			.s32SDS2   = {  0,   0, 100},
			.s32SDS3   = { 33,  33, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {200, 200, 200},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 3},
			.s32SBF3   = {0, 0, 3},

			.s32SFR0  = 31,
			.s32SFR1  = 8,
			.s32SFR2  = 8,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 200,
			.s32MATH2 = 200,
			.s32MATE1 = 3,
			.s32MATE2 = 2,
			.s32MABW1 = 3,
			.s32MABW2 = 0,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 11,
			.s32TFS2 = 9,
			.s32TFR1 = {30, 30},
			.s32TFR2 = {30, 30},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 0,
			.s32TFC = 0,
			.s32TPC = 0,
			.s32TRC = 0,
			},

			{ //ISO400
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {10, 20, 4, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 200, 100},
			.s32SBS1   = {100, 100, 0},
			.s32SBS2   = {  0,   0, 100},
			.s32SBS3   = { 33,  33, 100},
			.s32SDS0   = {100, 100, 100},
			.s32SDS1   = {100, 100, 0},
			.s32SDS2   = {  0,   0, 100},
			.s32SDS3   = { 33,  33, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {200, 200, 200},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 3},
			.s32SBF3   = {0, 0, 3},

			.s32SFR0  = 31,
			.s32SFR1  = 8,
			.s32SFR2  = 8,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 200,
			.s32MATH2 = 200,
			.s32MATE1 = 3,
			.s32MATE2 = 2,
			.s32MABW1 = 3,
			.s32MABW2 = 0,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 11,
			.s32TFS2 = 9,
			.s32TFR1 = {30, 30},
			.s32TFR2 = {30, 30},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 0,
			.s32TFC = 0,
			.s32TPC = 0,
			.s32TRC = 0,
			},

			{ //ISO800
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {5, 20, 4, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 200, 100},
			.s32SBS1   = {150, 150, 0},
			.s32SBS2   = {  0,   0, 200},
			.s32SBS3   = { 33,  33, 100},
			.s32SDS0   = {100, 200, 100},
			.s32SDS1   = {150, 150, 0},
			.s32SDS2   = {  0,   0, 200},
			.s32SDS3   = { 33,  33, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 3},
			.s32SBF3   = {0, 0, 3},

			.s32SFR0  = 31,
			.s32SFR1  = 20,
			.s32SFR2  = 20,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 300,
			.s32MATH2 = 300,
			.s32MATE1 = 5,
			.s32MATE2 = 2,
			.s32MABW1 = 3,
			.s32MABW2 = 0,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 11,
			.s32TFS2 = 9,
			.s32TFR1 = {30, 30},
			.s32TFR2 = {30, 30},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 0,
			.s32TFC = 0,
			.s32TPC = 0,
			.s32TRC = 0,
			},

			{ //ISO1600
			/*sharpen*/
			.s32IES0   = 19,
			.s32IEF    = {5, 20, 2, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 100, 100},
			.s32SBS1   = {200, 200, 0},
			.s32SBS2   = {  0,   0, 200},
			.s32SBS3   = { 33,  33, 100},
			.s32SDS0   = {100, 100, 100},
			.s32SDS1   = {200, 200, 0},
			.s32SDS2   = {  0,   0, 200},
			.s32SDS3   = { 33,  33, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 3},
			.s32SBF3   = {0, 0, 3},

			.s32SFR0  = 31,
			.s32SFR1  = 20,
			.s32SFR2  = 20,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 300,
			.s32MATH2 = 400,
			.s32MATE1 = 3,
			.s32MATE2 = 2,
			.s32MABW1 = 3,
			.s32MABW2 = 0,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 12,
			.s32TFS2 = 9,
			.s32TFR1 = {30, 30},
			.s32TFR2 = {30, 30},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 0,
			.s32TFC = 0,
			.s32TPC = 0,
			.s32TRC = 0,
			},

			{ //ISO3100
			/*sharpen*/
			.s32IES0   = 10,
			.s32IEF    = {5, 20, 2, 2},

			/*spacial filter*/
			.s32SBS0   = {100, 250, 100},
			.s32SBS1   = {200, 200, 0},
			.s32SBS2   = {  0,   0, 200},
			.s32SBS3   = { 33,  33, 100},
			.s32SDS0   = {100, 100, 100},
			.s32SDS1   = {200, 200, 0},
			.s32SDS2   = {  0,   0, 200},
			.s32SDS3   = { 33,  33, 100},
			.s32STH0   = {999, 999, 999},
			.s32STH1   = {999, 999, 0},
			.s32STH2   = {999, 999, 999},
			.s32STH3   = {500, 500, 500},
			.s32SBF0   = {0, 0, 0},
			.s32SBF1   = {0, 0, 3},
			.s32SBF2   = {0, 0, 3},
			.s32SBF3   = {0, 0, 3},

			.s32SFR0  = 31,
			.s32SFR1  = 20,
			.s32SFR2  = 20,
			.s32SFR3  = 18,
			.s32STR0  = 0,
			.s32STR1  = 31,
			.s32STR2  = 31,

			/*motion detetion*/
			.s32MATH1 = 400,
			.s32MATH2 = 400,
			.s32MATE1 = 4,
			.s32MATE2 = 2,
			.s32MABW1 = 3,
			.s32MABW2 = 2,
			.s32MATW1 = 3,
			.s32MATW2 = 3,

			/*time filter*/
			.s32TFS1 = 13,
			.s32TFS2 = 9,
			.s32TFR1 = {30, 30},
			.s32TFR2 = {30, 30},
			.s32TSR1 = 0,
			.s32TSR2 = 0,
			.s32TSS1 = 1,
			.s32TSS2 = 1,
			.s32TSDZ1 = 0,
			.s32TSDZ2 = 0,

			/*color filter*/
			.s32SFC = 0,
			.s32TFC = 0,
			.s32TPC = 0,
			.s32TRC = 0,
			}
		},
	}
};

static void * NR_X_AutoThread(void * argv)
{
	HI_U32 u32Iso = 0;
    HI_U32 u32LastIso = 0;
	ISP_DEV IspDev = 0;
	ISP_EXP_INFO_S stIspExpInfo;

	int ii = 0;
	HI_S32 s32Ret;
	int * RunFlag = (int *)argv;

	while(*RunFlag) {
		//calculate iso value (iso = AGain * DGain * ISPDGain * 100 >> 30)
		s32Ret = HI_MPI_ISP_QueryExposureInfo(IspDev, &stIspExpInfo);
		if (HI_SUCCESS != s32Ret) {
			printf("HI_MPI_ISP_QueryExposureInfo failed\n");
			continue;
		}
		u32Iso = (HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain * 100 >>30;

		if(u32Iso != u32LastIso) {

			s32Ret = Sceneauto_Set3DNR_X(0, u32Iso, IsDayNight() ? gDefaultNrX_SC2232[0] : gDefaultNrX_SC2232[1]);
			if (HI_SUCCESS != s32Ret) {
				printf("normal mode: Sceneauto_SetNormal3DNR failed\n");
			}

			u32LastIso = u32Iso;
		}

		usleep(500000);
	}

	pthread_exit(NULL);
}

static pthread_t ThreadID = NULL;
static int NR_X_Flag = 0;

void NR_X_AutoInit(void)
{
	int ret;
	if(ThreadID == NULL) {
		NR_X_Flag = 1;
		ret = pthread_create(&ThreadID, NULL, NR_X_AutoThread, &NR_X_Flag);
		if(ret != 0) {
			printf("\n%s Invoked Failed!\n", __FUNCTION__);
		}
	}
}

void NR_X_AutoExit(void)
{
	if(ThreadID && NR_X_Flag) {
		NR_X_Flag = 0;

		pthread_join(ThreadID, NULL);
		ThreadID = NULL;
	}
}