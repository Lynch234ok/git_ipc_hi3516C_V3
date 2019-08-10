
#include "higpio.h"
#include "../jagpio/bsp_jagpio.h"
#include <hi_type.h>

static int sys_read_reg(unsigned reg_addr, unsigned *val32)
{
	if(HI_SUCCESS == HI_MPI_SYS_GetReg((HI_U32)reg_addr, (HI_U32*)val32)){
		return 0;
	}
	return -1;
}

static int sys_write_reg(unsigned reg_addr, unsigned val32)
{
	if(HI_SUCCESS == HI_MPI_SYS_SetReg((HI_U32)reg_addr, (HI_U32)val32)){
		return 0;
	}
	return -1;
}

static unsigned int isp_gpio_get_dir_addr(int gpio_group)
{
	unsigned ret_val;
#if defined(HI3516C_V3) || defined(HI3516E_V1)
	ret_val = GPIO_BASE_ADDR + gpio_group*0x1000 + 0x400;
#else	
	ret_val = GPIO_BASE_ADDR + gpio_group*0x10000 + 0x400;
#endif

	return ret_val;
}

static unsigned int isp_gpio_get_data_addr(int gpio_group)
{
	unsigned ret_val;
#if defined(HI3516C_V3) || defined(HI3516E_V1)	
	ret_val = GPIO_BASE_ADDR + gpio_group*0x1000 + 0x3fc;
#else	
		
	ret_val = GPIO_BASE_ADDR + gpio_group*0x10000 + 0x3fc;
#endif

	return ret_val;
}

static unsigned int isp_gpio_pin_read(int gpio_group, int gpio_pin)
{
	unsigned int reg_val = 0;

	//pin dir :in
	sys_read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	sys_write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);

	//read pin
	sys_read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= (1<<gpio_pin);
	return reg_val;
}

static void isp_gpio_pin_write(int gpio_group, int gpio_pin, unsigned char val)
{
	unsigned int reg_val = 0;
	
	//pin dir :out
	sys_read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val |= (1<<gpio_pin);
	sys_write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);
	
	sys_read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	reg_val |= (val<<gpio_pin);
	sys_write_reg(isp_gpio_get_data_addr(gpio_group), reg_val);
}

void BSP_sysCreate(void)
{
	TJA_SysCtrl Hi_interface = {

		.sys_read_reg		= sys_read_reg,
		.sys_write_reg		= sys_write_reg,
		.gpio_pin_read		= isp_gpio_pin_read,
		.gpio_pin_write		= isp_gpio_pin_write,
	};
	
	BSP_SysCreate(Hi_interface);
}

