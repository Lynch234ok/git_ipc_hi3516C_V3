
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "hi3516c.h"

typedef struct SDK_SYS_ATTR {


}stSDK_SYS_ATTR;

typedef struct SDK_SYS_HI3521 {
	stSDK_SYS_API api;
	stSDK_SYS_ATTR attr;
}stSDK_SYS_HI3521, *lpSDK_SYS_HI3521;

static stSDK_SYS_HI3521 _sdk_sys;
lpSDK_SYS_API sdk_sys = NULL;


static void mpp_vb_conf_clear(VB_CONF_S* p_vb_conf)
{
	p_vb_conf->u32MaxPoolCnt = 0;
}

static int mpp_vb_conf_add_block(VB_CONF_S* p_vb_conf, int block_size, int block_count)
{
	if(p_vb_conf->u32MaxPoolCnt < VB_MAX_COMM_POOLS){
		p_vb_conf->astCommPool[p_vb_conf->u32MaxPoolCnt].u32BlkSize = block_size;
		p_vb_conf->astCommPool[p_vb_conf->u32MaxPoolCnt].u32BlkCnt = block_count;
		++p_vb_conf->u32MaxPoolCnt;
		return 0;
	}
	return -1;
}

static void hi_mpp_destroy()
{
	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();
}

static void hi_mpp_init(const char *solution)
{
	MPP_SYS_CONF_S sys_conf;
	VB_CONF_S vb_conf;
	printf("%s-%d\n", __FUNCTION__, __LINE__);
	hi_mpp_destroy();
	printf("%s-%d\n", __FUNCTION__, __LINE__);
	memset(&vb_conf, 0, sizeof(vb_conf));
	mpp_vb_conf_clear(&vb_conf);
	if(!strcmp(solution, "hi3518e_v2-inception")){
		///18ev200 1080p
		mpp_vb_conf_add_block(&vb_conf, 1536 * 1536 * 3/2 + 48, 2);// 3
		mpp_vb_conf_add_block(&vb_conf, 320 * 320 * 3/2 + 100, 5);//5	3	
		//mpp_vb_conf_add_block(&vb_conf, 352 * 288 * 3/2 + 100, 1);//vda  352 x 288  9

		//18ev200 960p
	//	mpp_vb_conf_add_block(&vb_conf, 1280 * 960 * 3/2 + 48, 3);//5
	//	mpp_vb_conf_add_block(&vb_conf, 640 * 360 * 3/2 + 48, 3);//640 x 360
	//	mpp_vb_conf_add_block(&vb_conf, 352 * 288 * 3/2 + 100, 1);//vda  352 x 288
	}else if(!strcmp(solution, "hi3516c_v2-inception")){
		mpp_vb_conf_add_block(&vb_conf, 1920 * 1080 * 3/2 + 48, 5);//  3 for normal   2 for LowDelay    4
		mpp_vb_conf_add_block(&vb_conf, 720 * 576 * 3/2 + 100, 4);//5		   3
		mpp_vb_conf_add_block(&vb_conf, 352 * 288 * 3/2 + 100, 4);//vda  352 x 288  9       1
	}else{
		printf("UNKWON sulotion!!--%s\n", solution);
		mpp_vb_conf_add_block(&vb_conf, 1280 * 720 * 2, 10);
		mpp_vb_conf_add_block(&vb_conf, 640 * 480 * 2, 10);
		mpp_vb_conf_add_block(&vb_conf, 320 * 240 * 2, 10);
	}
	printf("%s-%d\n", __FUNCTION__, __LINE__);
	SOC_CHECK(HI_MPI_VB_SetConf(&vb_conf));
	SOC_CHECK(HI_MPI_VB_Init());
	printf("%s-%d\n", __FUNCTION__, __LINE__);
	memset(&sys_conf, 0, sizeof(sys_conf));
    sys_conf.u32AlignWidth = 64;
    SOC_CHECK(HI_MPI_SYS_SetConf(&sys_conf));
    SOC_CHECK(HI_MPI_SYS_Init());
	printf("%s-%d\n", __FUNCTION__, __LINE__);
}

HI_S32 HI_MPI_SYS_SetReg(HI_U32 u32Addr, HI_U32 u32Value)
{
    HI_U32 *pu32Addr = NULL;
    HI_U32 u32MapLen = sizeof(u32Value);

    pu32Addr = (HI_U32 *)HI_MPI_SYS_Mmap(u32Addr, u32MapLen);
    if(NULL == pu32Addr)
    {
        return HI_FAILURE;
    }

    *pu32Addr = u32Value;

    return HI_MPI_SYS_Munmap(pu32Addr, u32MapLen);
}

HI_S32 HI_MPI_SYS_GetReg(HI_U32 u32Addr, HI_U32 *pu32Value)
{
    HI_U32 *pu32Addr = NULL;
    HI_U32 u32MapLen;

    if (NULL == pu32Value)
    {
        return HI_ERR_SYS_NULL_PTR;
    }

    u32MapLen = sizeof(*pu32Value);
    pu32Addr = (HI_U32 *)HI_MPI_SYS_Mmap(u32Addr, u32MapLen);
    if(NULL == pu32Addr)
    {
        return HI_FAILURE;
    }

    *pu32Value = *pu32Addr;

    return HI_MPI_SYS_Munmap(pu32Addr, u32MapLen);
}

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
	int ret = -1;
	ret = HI_MPI_SYS_GetChipId(chip_id);
	printf("chip id is %#x\n", *chip_id); 
	return ret;
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
	//	hi_mpp_init(solution);

		// init the handle
		sdk_sys = (lpSDK_SYS_API)(&_sdk_sys);
		return 0;
	}
	return -1;
}

int SDK_destroy_sys()
{
	if(NULL != sdk_sys){
	//	hi_mpp_destroy();

		// destroy the handle
		sdk_sys = NULL;
		return 0;
	}
	return -1;
}



