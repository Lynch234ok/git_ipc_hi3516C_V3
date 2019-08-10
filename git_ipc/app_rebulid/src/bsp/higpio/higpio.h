
#ifndef _HI_GPIO_H
#define _HI_GPIO_H

#ifdef __cplusplus 
	extern "C" { 
#endif

#if defined(HI3516C_V3) || defined(HI3516E_V1)

#define GPIO_BASE_ADDR 			0x12140000
#define GPIO_MUX_BASE_ADDR		0x12040000
#else

#define GPIO_BASE_ADDR 			0x20140000
#define GPIO_MUX_BASE_ADDR		0x200F0000
#endif

extern void BSP_sysCreate(void);

#ifdef __cplusplus 
	}
#endif


#endif
