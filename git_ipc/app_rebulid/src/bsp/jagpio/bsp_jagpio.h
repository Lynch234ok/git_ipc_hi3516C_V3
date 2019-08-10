
#ifndef _JA_GPIO_H
#define _JA_GPIO_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h> 

#ifdef __cplusplus 
	extern "C" { 
#endif

typedef struct jaSysCtrl {

	int (*sys_read_reg)(unsigned reg_addr, unsigned *val32);
	void (*sys_write_reg)(unsigned reg_addr, unsigned val32);
	unsigned (*gpio_pin_read)(int gpio_group, int gpio_pin);
	void (*gpio_pin_write)(int gpio_group, int gpio_pin, unsigned char val);
}TJA_SysCtrl;

typedef  TJA_SysCtrl* 		sysHandle;


///<以下为公共接口
extern int BSP_read_reg(unsigned reg_addr, unsigned *val32);
extern void BSP_write_reg(unsigned reg_addr, unsigned val32);
extern unsigned int BSP_gpio_pin_read(int gpio_group, int gpio_pin);
extern void BSP_gpio_pin_write(int gpio_group, int gpio_pin, unsigned char val);

extern void BSP_SysCreate(TJA_SysCtrl Sys_arg);

#ifdef __cplusplus 
	}
#endif


#endif

