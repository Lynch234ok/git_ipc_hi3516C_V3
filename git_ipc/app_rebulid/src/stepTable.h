#ifndef _STEP_TABLE_H
#define _STEP_TABLE_H

#define	FOCUS_SIZE_2812		1

#if	FOCUS_SIZE_2812
#define CALCULATE_INTER		(2)	//计算间隔，每隔CALCULATE_INTER个zoom步就计算一次focus步
#define FOUCUS_UP_LIMIT		(222)
#define LOW_ZOOM_STEP		(80)
#define HIGH_ZOOM_STEP		(2000)
#define LOW_FOCUS_STEP		(20)
#define HIGH_FOCUS_STEP		(2322 - FOUCUS_UP_LIMIT)

#define ALPHA_STEP			(1.0295)
#define BELTA_STEP			(-5.4873)


#define ALPHA_SQ			(0.0004)
#define BELTA_SQ			(0.1324)
#define DELTA_SQ			(582.56)


#define SEG_NUM				14
extern int ZOOM_TBL[]; 
extern int FOCUS_TBL[];


#elif FOCUS_SIZE_2808

#endif


#endif
