/******************************************************************************

  Copyright (C), 2009-2019, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_math.h
  Version       : Initial Draft
  Author        : c55300
  Created       : 2009/04/09
  Last Modified :
  Description   : mathematical functions.
  Function List :
  History       :
  1.Date        : 2009/04/09
    Author      : c55300
    Modification: Created file
    
  2.Date        : 2009/05/04
    Author      : c55300
    Modification: Add ENDIAN and VALUE_BETWEEN.

  3.Date        : 2009/05/21
    Author      : c55300
    Modification: Add macro of FRACTION operation and CMP.
    
******************************************************************************/
#ifndef __HI_MATH_H__
#define __HI_MATH_H__

/*****************************************************************************
 description
 *************
 ABS(x)                 absolute value of x
 SIGN(x)                sign of x
 CMP(x,y)               0 if x==y; 1 if x>y; -1 if x<y
 
 MAX2(x,y)              maximum of x and y
 MIN2(x,y)              minimum of x and y
 MAX3(x,y,z)            maximum of x, y and z
 MIN3(x,y,z)            minimun of x, y and z
 MEDIAN(x,y,z)          median of x,y,z
 MEAN2(x,y)             mean of x,y
 
 CLIP3(x,min,max)       clip x within [min,max]
 WRAP_MAX(x,max,min)    wrap to min if x equal max
 WRAP_MIN(x,min,max)    wrap to max if x equal min
 
 MULTI_OF_2_POWER(x,a)  whether x is multiple of a(a must be power of 2)
 CEILING_2_POWER(x,a)   ceiling x to multiple of a(a must be power of 2)
 FLOOR_2_POWER(x,a)     floor x to multiple of a(a must be power of 2)
 
 Example:
 CEILING_2_POWER(5,4) = 8
 FLOOR_2_POWER(5,4)   = 4

 ENDIAN32(x,y)              little endian <---> big endian
 VALUE_BETWEEN(x,min.max)   True if x is between [min,max] inclusively.
 FRACTION32(de,nu)          fraction: nu(minator) / de(nominator).
 NUMERATOR32(x)              of x(x is fraction)
 DENOMINATOR32(x)           Denominator of x(x is fraction)
******************************************************************************/

#define ABS(x)          ( (x) >= 0 ? (x) : (-(x)) )
#define SIGN(x)         ( (x) >= 0 ? 1 : -1 )
#define CMP(x,y)        ( ((x) == (y)) ? 0 : (((x) > (y)) ? 1 : -1))

#define MAX2(x,y)       ( (x)>(y) ? (x):(y) )
#define MIN2(x,y)       ( (x)<(y) ? (x):(y) )
#define MAX3(x,y,z)     ( (x)>(y) ? MAX2(x,z) : MAX2(y,z) )
#define MIN3(x,y,z)     ( (x)<(y) ? MIN2(x,z) : MIN2(y,z) )
#define MEDIAN(x,y,z)   ( (x)+(y)+(z) - MAX3(x,y,z) - MIN3(x,y,z) )
#define MEAN2(x,y)      ( ((x)+(y)) >> 1 )

#define CLIP3(x,min,max)        ( (x)<(min) ? (min) : ((x)>(max)?(max):(x)) )
#define WRAP_MAX(x,max,min)     ( (x)>=(max) ? (min) : (x) )
#define WRAP_MIN(x,min,max)     ( (x)<=(min) ? (max) : (x) )

#define MULTI_OF_2_POWER(x,a)    ( !( (x) & ((a) - 1) ) )
#define CEILING_2_POWER(x,a)     ( ( (x) + ((a) - 1) ) & ( ~((a) - 1) ) )
#define FLOOR_2_POWER(x,a)       ( (x) & ( ~((a) - 1) ) )

#define  ENDIAN32( x )						\
	(   ( (x) << 24 ) |						\
		( ( (x) & 0x0000ff00 ) << 8 ) |		\
		( ( (x) & 0x00ff0000 ) >> 8 ) |		\
		( ( (x) >> 24 ) & 0x000000ff )  )

#define VALUE_BETWEEN(x,min,max) (((x) >= (min)) && ((x) <= (max)))

/* represent fraction in 32 bit. LSB 16 is numerator, MSB 16 is denominator */
/* It is integer if denominator is 0. */
#define FRACTION32(de,nu)       ( ((de) << 16) | (nu) ) 
#define NUMERATOR32(x)          ( (x) & 0xffff)
#define DENOMINATOR32(x)        ( (x) >> 16 )  

#endif /* __HI_MATH_H__ */

