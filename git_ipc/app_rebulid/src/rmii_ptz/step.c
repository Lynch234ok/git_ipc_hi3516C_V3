#include <stdio.h>
#include <stdbool.h>
#include "app_gpio.h"
#include "sdk/sdk_api.h"

#include "step.h"

#define STEP_GPIO_BASE_ADDR 0x20140000
//GPIO2_3 
#define STEP_GPIO_HA0_MULTI_PIN 0x200f0024
#define STEP_GPIO_HA0_GROUP (2)
#define STEP_GPIO_HA0_PIN (3)
//GPIO3_2
#define STEP_GPIO_HA1_MULTI_PIN 0x200f0058
#define STEP_GPIO_HA1_GROUP (3)
#define STEP_GPIO_HA1_PIN (2)
//GPIO4_2
#define STEP_GPIO_HB0_MULTI_PIN 0x200f003c
#define STEP_GPIO_HB0_GROUP (4)
#define STEP_GPIO_HB0_PIN (2)
//GPIO4_3
#define STEP_GPIO_HB1_MULTI_PIN 0x200f0038
#define STEP_GPIO_HB1_GROUP (4)
#define STEP_GPIO_HB1_PIN (3)
//GPIO5_3
#define STEP_GPIO_HLIMIT_MULTI_PIN 0x200f00C0
#define STEP_GPIO_HLIMIT_GROUP (5)
#define STEP_GPIO_HLIMIT_PIN (3)
//GPIO4_6
#define STEP_GPIO_VA0_MULTI_PIN 0x200f004c
#define STEP_GPIO_VA0_GROUP (4)
#define STEP_GPIO_VA0_PIN (6)
//GPIO4_7
#define STEP_GPIO_VA1_MULTI_PIN 0x200f0048
#define STEP_GPIO_VA1_GROUP (4)
#define STEP_GPIO_VA1_PIN (7)
//GPIO3_0
#define STEP_GPIO_VB0_MULTI_PIN 0x200f0030
#define STEP_GPIO_VB0_GROUP (3)
#define STEP_GPIO_VB0_PIN (0)
//GPIO3_1
#define STEP_GPIO_VB1_MULTI_PIN 0x200f0034
#define STEP_GPIO_VB1_GROUP (3)
#define STEP_GPIO_VB1_PIN (1)
//GPIO5_2
#define STEP_GPIO_VLIMIT_MULTI_PIN 0x200f00BC
#define STEP_GPIO_VLIMIT_GROUP (5)
#define STEP_GPIO_VLIMIT_PIN (2)







static bool old_step_vlimit = false, old_step_hlimit = false;

static uint32_t step_gpio_get_dir_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = STEP_GPIO_BASE_ADDR + (gpio_group<<16) + 0x400;
	return ret_val;
}

static uint32_t step_gpio_get_data_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = STEP_GPIO_BASE_ADDR + (gpio_group<<16) + 0x3fc;
	return ret_val;
}

static uint32_t step_gpio_pin_read(int gpio_group, int gpio_pin)
{
	uint32_t reg_val = 0;

	//pin dir :in
	/*sdk_sys->read_reg(step_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	sdk_sys->write_reg(step_gpio_get_dir_addr(gpio_group), reg_val);*/

	//read pin
	sdk_sys->read_reg(step_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= (1<<gpio_pin);
	return reg_val;
}

static void step_gpio_pin_write(int gpio_group, int gpio_pin, uint8_t val)
{
	uint32_t reg_val = 0;
	
	//pin dir :out
	/*sdk_sys->read_reg(step_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val |= (1<<gpio_pin);
	sdk_sys->write_reg(step_gpio_get_dir_addr(gpio_group), reg_val);*/
	
	sdk_sys->read_reg(step_gpio_get_data_addr(gpio_group), &reg_val);
	//HI_MPI_SYS_GetReg(step_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	reg_val |= (val<<gpio_pin);
	//HI_MPI_SYS_SetReg(step_gpio_get_data_addr(gpio_group), reg_val);
	sdk_sys->write_reg(step_gpio_get_data_addr(gpio_group), reg_val);
}

#if 1
int step_gpio_init()
{
	uint32_t reg_val = 0;	

	//GPIO2_3
	sdk_sys->write_reg(STEP_GPIO_HA0_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_HA0_GROUP), &reg_val);
	reg_val |= (1<<STEP_GPIO_HA0_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_HA0_GROUP), reg_val);
	//GPIO3_2
	sdk_sys->write_reg(STEP_GPIO_HA1_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_HA1_GROUP), &reg_val);
	reg_val |= (1<<STEP_GPIO_HA1_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_HA1_GROUP), reg_val);
	//GPIO4_2
	sdk_sys->write_reg(STEP_GPIO_HB0_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_HB0_GROUP), &reg_val);
	reg_val |= (1<<STEP_GPIO_HB0_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_HB0_GROUP), reg_val);
	//GPIO4_3
	sdk_sys->write_reg(STEP_GPIO_HB1_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_HB1_GROUP), &reg_val);
	reg_val |= (1<<STEP_GPIO_HB1_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_HB1_GROUP), reg_val);
	//GPIO5_3
	sdk_sys->write_reg(STEP_GPIO_HLIMIT_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_HLIMIT_GROUP), &reg_val);
	reg_val &= ~(1<<STEP_GPIO_HLIMIT_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_HLIMIT_GROUP), reg_val);
	//GPIO4_6
	sdk_sys->write_reg(STEP_GPIO_VA0_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_VA0_GROUP), &reg_val);
	reg_val |= (1<<STEP_GPIO_VA0_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_VA0_GROUP), reg_val);
	//GPIO4_7
	sdk_sys->write_reg(STEP_GPIO_VA1_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_VA1_GROUP), &reg_val);
	reg_val |= (1<<STEP_GPIO_VA1_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_VA1_GROUP), reg_val);
	//GPIO3_0
	sdk_sys->write_reg(STEP_GPIO_VB0_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_VB0_GROUP), &reg_val);
	reg_val |= (1<<STEP_GPIO_VB0_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_VB0_GROUP), reg_val);
	//GPIO3_1
	sdk_sys->write_reg(STEP_GPIO_VB1_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_VB1_GROUP), &reg_val);
	reg_val |= (1<<STEP_GPIO_VB1_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_VB1_GROUP), reg_val);
	//GPIO5_2
	sdk_sys->write_reg(STEP_GPIO_VLIMIT_MULTI_PIN, 0);
	sdk_sys->read_reg(step_gpio_get_dir_addr(STEP_GPIO_VLIMIT_GROUP), &reg_val);
	reg_val &= ~(1<<STEP_GPIO_VLIMIT_PIN);
	sdk_sys->write_reg(step_gpio_get_dir_addr(STEP_GPIO_VLIMIT_GROUP), reg_val);

	old_step_hlimit = step_gpio_pin_read(STEP_GPIO_HLIMIT_GROUP, STEP_GPIO_HLIMIT_PIN);
	old_step_vlimit = step_gpio_pin_read(STEP_GPIO_VLIMIT_GROUP, STEP_GPIO_VLIMIT_PIN);
	//APP_GPIO_get_pin("steper HLIMT", &old_step_hlimit);
	//APP_GPIO_get_pin("steper VLIMT", &old_step_vlimit);
}

void stepH_stop()
{
	step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 0);
	step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 0);
	step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 0);
	step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 0);
}

void stepH(int step){
	switch(step){
		case 0:
		{
			step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 1);
			step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 0);
		}break;
		case 1:
		{
			//step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 0);
			step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 1);
		}break;
		case 2:
		{
			step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 0);
			//step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 0);
			//step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 1);
		}break;
		case 3:
		{
			//step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 0);
			step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 1);
		}break;
		case 4:
		{
			//step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 1);
			step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 0);
			//step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 0);
			//step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 1);
		}break;
		case 5:
		{
			//step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 0);
			step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 1);
		}break;
		case 6:
		{
			//step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 1);
			step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 0);
			//step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 0);

		}break;
		case 7:
		{
			step_gpio_pin_write(STEP_GPIO_HA0_GROUP, STEP_GPIO_HA0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HA1_GROUP, STEP_GPIO_HA1_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HB0_GROUP, STEP_GPIO_HB0_PIN, 1);
			//step_gpio_pin_write(STEP_GPIO_HB1_GROUP, STEP_GPIO_HB1_PIN, 0);
		}break;
		default:
			break;
	}
}

void stepV_stop()
{
	step_gpio_pin_write(STEP_GPIO_VA0_GROUP, STEP_GPIO_VA0_PIN, 0);
	step_gpio_pin_write(STEP_GPIO_VA1_GROUP, STEP_GPIO_VA1_PIN, 0);
	step_gpio_pin_write(STEP_GPIO_VB0_GROUP, STEP_GPIO_VB0_PIN, 0);
	step_gpio_pin_write(STEP_GPIO_VB1_GROUP, STEP_GPIO_VB1_PIN, 0);	
}

void stepV(int step){
	switch(step){
		case 0:
		{
			step_gpio_pin_write(STEP_GPIO_VA0_GROUP, STEP_GPIO_VA0_PIN, 1);
			step_gpio_pin_write(STEP_GPIO_VB1_GROUP, STEP_GPIO_VB1_PIN, 0);
		}break;
		case 1:
		{
			step_gpio_pin_write(STEP_GPIO_VA1_GROUP, STEP_GPIO_VA1_PIN, 1);
		}break;
		case 2:
		{
			step_gpio_pin_write(STEP_GPIO_VA0_GROUP, STEP_GPIO_VA0_PIN, 0);
		}break;
		case 3:
		{
			step_gpio_pin_write(STEP_GPIO_VB0_GROUP, STEP_GPIO_VB0_PIN, 1);
		}break;
		case 4:
		{
			step_gpio_pin_write(STEP_GPIO_VA1_GROUP, STEP_GPIO_VA1_PIN, 0);
		}break;
		case 5:
		{
			step_gpio_pin_write(STEP_GPIO_VB1_GROUP, STEP_GPIO_VB1_PIN, 1);
		}break;
		case 6:
		{
			step_gpio_pin_write(STEP_GPIO_VB0_GROUP, STEP_GPIO_VB0_PIN, 0);
		}break;
		case 7:
		{
			step_gpio_pin_write(STEP_GPIO_VA0_GROUP, STEP_GPIO_VA0_PIN, 1);
		}break;
		default:
			break;
	}
}

int step_get_vlimit(int operation)
{
	bool status;
	int ret = 0;
	status = step_gpio_pin_read(STEP_GPIO_VLIMIT_GROUP, STEP_GPIO_VLIMIT_PIN);
	if(old_step_vlimit != status && old_step_vlimit != 0){
		//stop steper when falling edge
		ret = 1;
	}else{
		ret = 0;
	}
	old_step_vlimit = status;
	return ret;
}

int step_get_hlimit(int operation)
{
	bool status;
	int ret = 0;
	//APP_GPIO_get_pin("steper HLIMT", &status);
	status = step_gpio_pin_read(STEP_GPIO_HLIMIT_GROUP, STEP_GPIO_HLIMIT_PIN);
	if(old_step_hlimit != status && old_step_hlimit != 0){
		//stop steper when falling edge
		ret = 1;
	}else{
		ret = 0;
	}
	old_step_hlimit = status;
	return ret;

}


#else

int step_gpio_init()
{
	//GPIO2_3
	APP_GPIO_add("steper HA+", 0x200f0024, 0xffffffff, 0, 0x20160400, 0x8, 0, 0x8, 0x20160020, 0x8);
	//GPIO3_2
	APP_GPIO_add("steper HA-", 0x200f0058, 0xffffffff, 0, 0x20170400, 0x4, 0, 0x4, 0x20170010, 0x4);
	//GPIO4_2
	APP_GPIO_add("steper HB+", 0x200f003c, 0xffffffff, 0, 0x20180400, 0x4, 0, 0x4, 0x20180010, 0x4);
	//GPIO4_3
	APP_GPIO_add("steper HB-", 0x200f0038, 0xffffffff, 0, 0x20180400, 0x8, 0, 0x8, 0x20180020, 0x8);
	//GPIO5_3
	APP_GPIO_add("steper HLIMT", 0x200f00C0, 0xffffffff, 0, 0x20190400, 0x8, 0, 0x8, 0x20190020, 0x8);
	//GPIO4_6
	APP_GPIO_add("steper VA+", 0x200f004c, 0xffffffff, 0, 0x20180400, 0x40, 0, 0x40, 0x20180100, 0x40);
	//GPIO4_7
	APP_GPIO_add("steper VA-", 0x200f0048, 0xffffffff, 0, 0x20180400, 0x80, 0, 0x80, 0x20180200, 0x80);
	//GPIO3_0
	APP_GPIO_add("steper VB+", 0x200f0030, 0xffffffff, 0, 0x20170400, 0x1, 0, 0x1, 0x20170004, 0x1);
	//GPIO3_1
	APP_GPIO_add("steper VB-", 0x200f0034, 0xffffffff, 0, 0x20170400, 0x2, 0, 0x2, 0x20170008, 0x2);
	//GPIO5_2
	APP_GPIO_add("steper VLIMT", 0x200f00BC, 0xffffffff, 0, 0x20190400, 0x4, 0, 0x4, 0x20190010, 0x4);

	APP_GPIO_get_pin("steper HLIMT", &old_step_hlimit);
	APP_GPIO_get_pin("steper VLIMT", &old_step_vlimit);
}

void stepH_stop()
{
	APP_GPIO_set_pin("steper HA+", 1);
	APP_GPIO_set_pin("steper HA-", 1);
	APP_GPIO_set_pin("steper HB+", 1);
	APP_GPIO_set_pin("steper HB-", 1);
}

void stepH(int step){
	switch(step){
		case 0:
		{
			APP_GPIO_set_pin("steper HA+", 0);
			//APP_GPIO_set_pin("steper HA-", 1);
			//APP_GPIO_set_pin("steper HB+", 1);
			APP_GPIO_set_pin("steper HB-", 1);
		}break;
		case 1:
		{
			//APP_GPIO_set_pin("steper HA+", 0);
			APP_GPIO_set_pin("steper HA-", 0);
			//APP_GPIO_set_pin("steper HB+", 1);
			//APP_GPIO_set_pin("steper HB-", 1);
		}break;
		case 2:
		{
			APP_GPIO_set_pin("steper HA+", 1);
			//APP_GPIO_set_pin("steper HA-", 0);
			//APP_GPIO_set_pin("steper HB+", 1);
			//APP_GPIO_set_pin("steper HB-", 1);
		}break;
		case 3:
		{
			//APP_GPIO_set_pin("steper HA+", 1);
			//APP_GPIO_set_pin("steper HA-", 0);
			APP_GPIO_set_pin("steper HB+", 0);
			//APP_GPIO_set_pin("steper HB-", 1);
		}break;
		case 4:
		{
			//APP_GPIO_set_pin("steper HA+", 1);
			APP_GPIO_set_pin("steper HA-", 1);
			//APP_GPIO_set_pin("steper HB+", 0);
			//APP_GPIO_set_pin("steper HB-", 1);
		}break;
		case 5:
		{
			//APP_GPIO_set_pin("steper HA+", 1);
			//APP_GPIO_set_pin("steper HA-", 1);
			//APP_GPIO_set_pin("steper HB+", 0);
			APP_GPIO_set_pin("steper HB-", 0);
		}break;
		case 6:
		{
			//APP_GPIO_set_pin("steper HA+", 1);
			//APP_GPIO_set_pin("steper HA-", 1);
			APP_GPIO_set_pin("steper HB+", 1);
			//APP_GPIO_set_pin("steper HB-", 0);
		}break;
		case 7:
		{
			APP_GPIO_set_pin("steper HA+", 0);
			//APP_GPIO_set_pin("steper HA-", 1);
			//APP_GPIO_set_pin("steper HB+", 1);
			//APP_GPIO_set_pin("steper HB-", 0);
		}break;
		default:
			break;
	}
}

void stepV_stop()
{
	APP_GPIO_set_pin("steper VA+", 1);
	APP_GPIO_set_pin("steper VA-", 1);
	APP_GPIO_set_pin("steper VB+", 1);
	APP_GPIO_set_pin("steper VB-", 1);	
}

void stepV(int step){
	switch(step){
		case 0:
		{
			APP_GPIO_set_pin("steper VA+", 0);
			APP_GPIO_set_pin("steper VA-", 1);
			APP_GPIO_set_pin("steper VB+", 1);
			APP_GPIO_set_pin("steper VB-", 1);
		}break;
		case 1:
		{
			APP_GPIO_set_pin("steper VA+", 0);
			APP_GPIO_set_pin("steper VA-", 0);
			APP_GPIO_set_pin("steper VB+", 1);
			APP_GPIO_set_pin("steper VB-", 1);
		}break;
		case 2:
		{
			APP_GPIO_set_pin("steper VA+", 1);
			APP_GPIO_set_pin("steper VA-", 0);
			APP_GPIO_set_pin("steper VB+", 1);
			APP_GPIO_set_pin("steper VB-", 1);
		}break;
		case 3:
		{
			APP_GPIO_set_pin("steper VA+", 1);
			APP_GPIO_set_pin("steper VA-", 0);
			APP_GPIO_set_pin("steper VB+", 0);
			APP_GPIO_set_pin("steper VB-", 1);
		}break;
		case 4:
		{
			APP_GPIO_set_pin("steper VA+", 1);
			APP_GPIO_set_pin("steper VA-", 1);
			APP_GPIO_set_pin("steper VB+", 0);
			APP_GPIO_set_pin("steper VB-", 1);
		}break;
		case 5:
		{
			APP_GPIO_set_pin("steper VA+", 1);
			APP_GPIO_set_pin("steper VA-", 1);
			APP_GPIO_set_pin("steper VB+", 0);
			APP_GPIO_set_pin("steper VB-", 0);
		}break;
		case 6:
		{
			APP_GPIO_set_pin("steper VA+", 1);
			APP_GPIO_set_pin("steper VA-", 1);
			APP_GPIO_set_pin("steper VB+", 1);
			APP_GPIO_set_pin("steper VB-", 0);
		}break;
		case 7:
		{
			APP_GPIO_set_pin("steper VA+", 0);
			APP_GPIO_set_pin("steper VA-", 1);
			APP_GPIO_set_pin("steper VB+", 1);
			APP_GPIO_set_pin("steper VB-", 0);
		}break;
		default:
			break;
	}
}

int step_get_vlimit()
{
	bool status;
	int ret = 0;
	APP_GPIO_get_pin("steper VLIMT", &status);
	if(old_step_vlimit != status && old_step_vlimit != 0){
		//stop steper when falling edge
		ret = 1;
	}else{
		ret = 0;
	}
	old_step_vlimit = status;
	return ret;
}

int step_get_hlimit()
{
	bool status;
	int ret = 0;
	APP_GPIO_get_pin("steper HLIMT", &status);
	if(old_step_hlimit != status && old_step_hlimit != 0){
		//stop steper when falling edge
		ret = 1;
	}else{
		ret = 0;
	}
	old_step_hlimit = status;
	return ret;

}

#endif
