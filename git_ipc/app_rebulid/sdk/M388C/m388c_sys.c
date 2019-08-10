
#include "sdk/sdk_api.h"
#include "sdk_trace.h"

typedef struct SDK_SYS_ATTR {


}stSDK_SYS_ATTR;

typedef struct SDK_SYS_HI3521 {
	stSDK_SYS_API api;
	stSDK_SYS_ATTR attr;
}stSDK_SYS_HI3521, *lpSDK_SYS_HI3521;

static stSDK_SYS_HI3521 _sdk_sys;
lpSDK_SYS_API sdk_sys = NULL;



static int sys_read_reg(uint32_t reg_addr, uint32_t *val32)
{
	return -1;
}

static int sys_write_reg(uint32_t reg_addr, uint32_t val32)
{
	return -1;
}

static int sys_read_mask_reg(uint32_t reg_addr, uint32_t mask32, uint32_t *val32)
{
	return -1;
}

static int sys_write_mask_reg(uint32_t reg_addr, uint32_t mask32, uint32_t val32)
{
	return -1;
}

static float sys_temperature()
{
	return -1;
}
static uint32_t sys_get_chip_id(uint32_t *chip_id)
{
	*chip_id = 0x388c2;
	return 0x388c2;
}

static stSDK_SYS_HI3521 _sdk_sys = {
	.api = {
		.read_reg = sys_read_reg,
		.write_reg = sys_write_reg,
		.read_mask_reg = sys_read_mask_reg,
		.write_mask_reg = sys_write_mask_reg,
		.temperature = sys_temperature,
		.get_chip_id = sys_get_chip_id,
	},
};

int SDK_init_sys(const char* solution)
{
	if(NULL == sdk_sys){
		// init the handle
		sdk_sys = (lpSDK_SYS_API)(&_sdk_sys);
		return 0;
	}
	return -1;
}

int SDK_destroy_sys()
{
	if(NULL != sdk_sys){
		
		// destroy the handle
		sdk_sys = NULL;
		return 0;
	}
	return -1;
}



