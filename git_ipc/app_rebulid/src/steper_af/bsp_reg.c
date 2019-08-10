
#include "bsp_reg.h"



static int sys_read_reg(uint32_t reg_addr, uint32_t *val32)
{
	if(HI_SUCCESS == HI_MPI_SYS_GetReg((HI_U32)reg_addr, (HI_U32*)val32)){
		return 0;
	}
	return -1;
}

static int sys_write_reg(uint32_t reg_addr, uint32_t val32)
{
	if(HI_SUCCESS == HI_MPI_SYS_SetReg((HI_U32)reg_addr, (HI_U32)val32)){
		return 0;
	}
	return -1;
}

static int set_pin_fun(int regNum, int FunSel)
{
	uint32_t reg_addr = GPIO_MUX_BASE_ADDR + regNum*4;

	return sys_write_reg(reg_addr, FunSel);
}

static uint32_t isp_gpio_get_dir_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = GPIO_BASE_ADDR + gpio_group*0x10000 + 0x400;
	return ret_val;
}

static uint32_t isp_gpio_get_data_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = GPIO_BASE_ADDR + gpio_group*0x10000 + 0x3fc;
	return ret_val;
}
/*
static uint32_t isp_gpio_pin_read(int gpio_group, int gpio_pin)
{
	uint32_t reg_val = 0;

	//pin dir :in
	sys_read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	sys_write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);

	//read pin
	sys_read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= (1<<gpio_pin);
	return reg_val;
}
*/
static void isp_gpio_pin_write(int gpio_group, int gpio_pin, uint8_t val)
{
	uint32_t reg_val = 0;
	
	//pin dir :out
	sys_read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val |= (1<<gpio_pin);
	sys_write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);
	
	sys_read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	reg_val |= (val<<gpio_pin);
	sys_write_reg(isp_gpio_get_data_addr(gpio_group), reg_val);
}

static stRegCtrl _RegCtrl={
	.reg_read_pin_data = NULL,//isp_gpio_pin_read,
	.reg_write_pin_data = isp_gpio_pin_write,
	.reg_set_pin_fun = set_pin_fun,
};

pstRegCtrl pRegCtrl = &_RegCtrl;

static inline JA_Void
BSP_IO_ACTIVE(JA_Int const reg_id, JA_Int const pin_fun
			, JA_Int const gpio_group, JA_Int const pin_id, JA_Boolean act)
{
	pRegCtrl->reg_set_pin_fun(reg_id, pin_fun);
	pRegCtrl->reg_write_pin_data(gpio_group, pin_id, act? 1:0);
}

/// zoom电机节拍控制。
inline JA_Void
BSP_IO_PAN_CTRL(JA_UInt32 io_ctrl)
{
    BSP_IO_ACTIVE( REG_ID_ZOOM_AP, PIN_FUN_ZOOM_AP
    			, GPIO_GROUP_ZOOM_AP, PIN_ID_ZOOM_AP, !!(io_ctrl & (1<<0)) );

	BSP_IO_ACTIVE( REG_ID_ZOOM_BP, PIN_FUN_ZOOM_BP
    			, GPIO_GROUP_ZOOM_BP, PIN_ID_ZOOM_BP, !!(io_ctrl & (1<<1)) );

	BSP_IO_ACTIVE( REG_ID_ZOOM_AN, PIN_FUN_ZOOM_AN
    			, GPIO_GROUP_ZOOM_AN, PIN_ID_ZOOM_AN, !!(io_ctrl & (1<<2)) );
	
	BSP_IO_ACTIVE( REG_ID_ZOOM_BN, PIN_FUN_ZOOM_BN
    			, GPIO_GROUP_ZOOM_BN, PIN_ID_ZOOM_BN, !!(io_ctrl & (1<<3)) );
}

/// foucus电机节拍控制。
inline JA_Void
BSP_IO_TILT_CTRL(JA_UInt32 io_ctrl)
{
    BSP_IO_ACTIVE( REG_ID_FOCUS_AP, PIN_FUN_FOCUS_AP
    			, GPIO_GROUP_FOCUS_AP, PIN_ID_FOCUS_AP, (io_ctrl & (1<<0)) );

	BSP_IO_ACTIVE( REG_ID_FOCUS_BP, PIN_FUN_FOCUS_BP
    			, GPIO_GROUP_FOCUS_BP, PIN_ID_FOCUS_BP, (io_ctrl & (1<<1)) );
	
	BSP_IO_ACTIVE( REG_ID_FOCUS_AN, PIN_FUN_FOCUS_AN
    			, GPIO_GROUP_FOCUS_AN, PIN_ID_FOCUS_AN, (io_ctrl & (1<<2)) );
    
	BSP_IO_ACTIVE( REG_ID_FOCUS_BN, PIN_FUN_FOCUS_BN
    			, GPIO_GROUP_FOCUS_BN, PIN_ID_FOCUS_BN, (io_ctrl & (1<<3)) );

}



