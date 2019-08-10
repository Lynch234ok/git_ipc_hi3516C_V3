
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "hi3516a.h"

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

static int sys_read_mask_reg(uint32_t reg_addr, uint32_t mask32, uint32_t *val32)
{
	if(0 == sys_read_reg(reg_addr, val32)){
		*val32 &= mask32;
		return 0;
	}
	return -1;
}

static int sys_write_mask_reg(uint32_t reg_addr, uint32_t mask32, uint32_t val32)
{
	uint32_t val32_r = 0;
	if(0 == sys_read_reg(reg_addr, &val32_r)){
		val32 &= mask32;
		val32_r &= ~mask32;
		return sys_write_reg(reg_addr, val32 | val32_r);
	}
	return -1;
}

// Hisilicon SPI read / write data
#pragma pack(1)
typedef union {
	 struct {
		uint32_t spi_wdata          : 8; // [7:0]
		uint32_t spi_rdata          : 8; // [15:8]
		uint32_t spi_addr           : 7; // [22:16]
		uint32_t spi_rw             : 1; // [23]
		uint32_t spi_start          : 1; // [24]
		uint32_t reserved           : 6; // [30:25]
		uint32_t spi_busy           : 1; // [31]
	 } bits;
	 // Define an unsigned member
	 uint32_t u32;
} stHI_SPI_RW;
#pragma pack()

static int sys_read_rtc(uint32_t reg_addr, uint32_t *val32)
{
	stHI_SPI_RW w_data, r_data;
	
	r_data.u32 = 0;
	w_data.u32 = 0;
	w_data.bits.spi_addr = (uint8_t) reg_addr;
	w_data.bits.spi_rw = 1; // 0 - write, 1 - read
	w_data.bits.spi_start = 0x1;

	if(0 == sdk_sys->write_reg(kHI_RTC_APB_SPI_RW, w_data.u32)){
		do{
			sdk_sys->read_reg(kHI_RTC_APB_SPI_RW, &r_data.u32);
			usleep(1000);
		}while(r_data.bits.spi_busy);
		*val32 = (uint32_t)r_data.bits.spi_rdata;
		return 0;
	}
	return -1;
}

static float sys_temperature()
{
	uint32_t reg = 0;
	if(0 == sys_read_rtc(0x20, &reg)){
		float ratio = 180.0 / 255.0;
		float rtc_temp = (float)reg * ratio - 40.0; // from -40 ~ 140
		return (float)(rtc_temp);
	}
	return -1;
}
static uint32_t sys_get_chip_id(uint32_t *chip_id)
{
	return HI_MPI_SYS_GetChipId(chip_id);
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
		//hi_mpp_init(solution);

		// init the handle
		sdk_sys = (lpSDK_SYS_API)(&_sdk_sys);
		return 0;
	}
	return -1;
}

int SDK_destroy_sys()
{
	if(NULL != sdk_sys){
		//hi_mpp_destroy();

		// destroy the handle
		sdk_sys = NULL;
		return 0;
	}
	return -1;
}



