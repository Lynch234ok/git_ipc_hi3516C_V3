
#include "bsp_jagpio.h"

static TJA_SysCtrl bsp_sysCtrl;
static sysHandle bsp_sysCtrlHandle = NULL;

int BSP_read_reg(unsigned reg_addr, unsigned *val32)
{
	if (!bsp_sysCtrlHandle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}

	return bsp_sysCtrlHandle->sys_read_reg(reg_addr, val32);
}

void BSP_write_reg(unsigned reg_addr, unsigned val32)
{
	if (!bsp_sysCtrlHandle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}
	
	bsp_sysCtrlHandle->sys_write_reg(reg_addr, val32);
}

unsigned BSP_gpio_pin_read(int gpio_group, int gpio_pin)
{
	if (!bsp_sysCtrlHandle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return 0;
	}

	return bsp_sysCtrlHandle->gpio_pin_read(gpio_group, gpio_pin);
}

void BSP_gpio_pin_write(int gpio_group, int gpio_pin, unsigned char val)
{
	if (!bsp_sysCtrlHandle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	bsp_sysCtrlHandle->gpio_pin_write(gpio_group, gpio_pin, val);
}



void BSP_SysCreate(TJA_SysCtrl Sys_arg)
{
	TJA_SysCtrl* Public = bsp_sysCtrlHandle = &bsp_sysCtrl;

	Public->sys_read_reg		= Sys_arg.sys_read_reg;
	Public->sys_write_reg		= Sys_arg.sys_write_reg;
	Public->gpio_pin_read		= Sys_arg.gpio_pin_read;
	Public->gpio_pin_write		= Sys_arg.gpio_pin_write;
}

