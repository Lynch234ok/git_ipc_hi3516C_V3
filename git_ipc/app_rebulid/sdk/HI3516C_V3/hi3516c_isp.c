
#include "hi3516c.h"
#include "hi3516c_isp_sensor.h"
#include "sdk/sdk_debug.h"
#include "sdk/sdk_api.h"
#include "sdk/sdk_isp_def.h"
#include "hi_isp_api.h"
#include "hi_isp.h"
#include "signal.h"
#include "hi_spi.h"
#include "hi3516c_isp_i2c.h"
#include "hi3516c_isp_gamma.h"
#include <sys/prctl.h>

#include "isp_nrx_auto.h"

//#include "hi_ssp.h"
#include "hi_isp_cfg.h"
#include "sdk_common.h"

#define HI3518A_VIN_DEV (0)
#define HI3518A_VIN_CHN (0)

#define GPIO_BASE_ADDR 0x12140000

#define ISP_GPIO_DAYLIGHT (0)
#define ISP_GPIO_NIGHT (1)

#define IRLED_CONTROL_MODE (0)
#define LIGHT_CONTROL_MODE (1)
#define SMART_CONTROL_MODE (2)

struct HI3518A_ERR_MAP
{
	uint32_t errno;
	const char* str;
};

static struct HI3518A_ERR_MAP _hi3518a_err_map[] =
{
	// sys
	{ 0xA0028003, "HI_ERR_SYS_ILLEGAL_PARAM", },
	{ 0xA0028006, "HI_ERR_SYS_NULL_PTR", },
	{ 0xA0028009, "HI_ERR_SYS_NOT_PERM", },
	{ 0xA0028010, "HI_ERR_SYS_NOTREADY", },
	{ 0xA0028012, "HI_ERR_SYS_BUSY", },
	{ 0xA002800C, "HI_ERR_SYS_NOMEM", },

	// venc
	{ 0xA0078001, "HI_ERR_VENC_INVALID_DEVID", },
	{ 0xA0078002, "HI_ERR_VENC_INVALID_CHNID", },
	{ 0xA0078003, "HI_ERR_VENC_ILLEGAL_PARAM", },
	{ 0xA0078004, "HI_ERR_VENC_EXIST", },
	{ 0xA0078005, "HI_ERR_VENC_UNEXIST", },
	{ 0xA0078006, "HI_ERR_VENC_NULL_PTR", },
	{ 0xA0078007, "HI_ERR_VENC_NOT_CONFIG", },
	{ 0xA0078008, "HI_ERR_VENC_NOT_SUPPORT", },
	{ 0xA0078009, "HI_ERR_VENC_NOT_PERM", },
	{ 0xA007800C, "HI_ERR_VENC_NOMEM", },
	{ 0xA007800D, "HI_ERR_VENC_NOBUF", },
	{ 0xA007800E, "HI_ERR_VENC_BUF_EMPTY", },
	{ 0xA007800F, "HI_ERR_VENC_BUF_FULL", },
	{ 0xA0078010, "HI_ERR_VENC_SYS_NOTREADY", },

	// vpss
	{ 0xA0088001, "HI_ERR_VPSS_INVALID_DEVID", },
	{ 0xA0088002, "HI_ERR_VPSS_INVALID_CHNID", },
	{ 0xA0088003, "HI_ERR_VPSS_ILLEGAL_PARAM", },
	{ 0xA0088004, "HI_ERR_VPSS_EXIST", },
	{ 0xA0088005, "HI_ERR_VPSS_UNEXIT", },
	{ 0xA0088006, "HI_ERR_VPSS_NULL_PTR", },
	{ 0xA0086008, "HI_ERR_VPSS_NOT_SUPPORT", },
	{ 0xA0088009, "HI_ERR_VPSS_NOT_PERM", },
	{ 0xA008800C, "HI_ERR_VPSS_NOMEM", },
	{ 0xA008800D, "HI_ERR_VPSS_NOBUF" },
	{ 0xA0088010, "HI_ERR_VPSS_NOTREADY", },
	{ 0xA0088012, "HI_ERR_VPSS_BUSY", },
};


const char* SOC_strerror(uint32_t errno)
{
	int i = 0;
	for(i = 0; i < (int)(sizeof(_hi3518a_err_map) / sizeof(_hi3518a_err_map[0])); ++i){
		if(errno == _hi3518a_err_map[i].errno){
			return _hi3518a_err_map[i].str;
		}
	}
	return "UNKNOWN ERROR!";
}

typedef struct _hi_isp_strength
{
	int strength;
	int daylight_val;
	int night_val;
}stHiIspStrength,*lpHiIspStrength;

typedef struct _hi_isp_attr
{
	stBSPApi bsp_api;
	stSensorApi api;
	emSENSOR_MODEL sensor_type;
	uint32_t gpio_status_old;// ISP_GPIO_DAYLIGHT;//daytime
	stSensorColorMaxValue color_max_value;
	uint8_t ircut_auto_switch_enable;// HI_TRUE;
	uint8_t ircut_control_mode;
	int sensor_resolution_width;
	int sensor_resolution_height;
	uint8_t src_framerate;
	uint8_t lowlight_mode;
	stHiIspStrength aeCompensition;
	stHiIspStrength aeHistSlope;
	stHiIspStrength aeHistOffset;
	stHiIspStrength wdr;
//	stHiIspStrength denoise3d_GlobalStrength;
	stHiIspStrength YPKStrength;
	stHiIspStrength YSFStrength;
	stHiIspStrength YTFStrength;
	stHiIspStrength YSmthStrength;
	
	uint8_t isp_auto_drc_enabled;
	stSensorAfAttr AfAttr;
	uint8_t isp_framerate_status;	
	uint8_t filter_frequency;
	LpIspCfgAttr ispCfgAttr;
	WDR_MODE_E isp_wdr_mode;
	bool vpss_flip;
	bool vpss_mirror;
	pthread_t isp_run_pid;
	bool starlight_mode_enable;

	int (*sensor_set_mirror)(bool mirror);
	int (*sensor_set_flip)(bool flip);
	int (*sensor_get_name)(char *sensor_name);

	uint8_t color_to_black_rank;
	uint32_t color_to_black_val;
	
	pthread_t md_alarm_pid;
	bool md_alarm_thread_trigger;
	bool md_lock;
	uint8_t ircut_switch_control_mode;
}stHiIspAttr, *lpHiIspAttr;

static stHiIspAttr _isp_attr;

combo_dev_attr_t SUBLVDS_4lane_SENSOR_AR0237_12BIT_ATTR =
{
    .devno = 0,
    /* input mode */
    .input_mode = INPUT_MODE_SUBLVDS,
    {
        .lvds_attr = {
            .img_size        = {1920, 1080},
            .raw_data_type   = RAW_DATA_12BIT,
            .wdr_mode        = HI_WDR_MODE_NONE,            
            .sync_mode       = LVDS_SYNC_MODE_SOF,
            .vsync_type      = {LVDS_VSYNC_NORMAL, 0, 0},
            .fid_type        = {LVDS_FID_NONE, HI_TRUE},
            .data_endian     = LVDS_ENDIAN_LITTLE,
            .sync_code_endian = LVDS_ENDIAN_LITTLE, 
            .lane_id = {0, 1, 2, 3},
            .sync_code =  {
				{{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005}},


				{{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005}},


				{{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005}},


				{{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005},
				{0x003,0x007,0x001,0x005}},

            } 
        }
    }
};

static combo_dev_attr_t MIPI_CMOS3V3_ATTR =
{
    /* input mode */
    .input_mode = INPUT_MODE_CMOS,
    {
        
    }
};

combo_dev_attr_t SUBLVDS_4lane_SENSOR_AR0237_12BIT_ATTR_WDR =
{
    .devno = 0,
    /* input mode */
    .input_mode = INPUT_MODE_SUBLVDS,
    {
        .lvds_attr = {
            .img_size        = {1920, 1080},
            .raw_data_type   = RAW_DATA_12BIT,
            .wdr_mode        = HI_WDR_MODE_2F,            
            .sync_mode       = LVDS_SYNC_MODE_SOF,
            .vsync_type      = {LVDS_VSYNC_NORMAL, 0, 0},
            .fid_type        = {LVDS_FID_IN_DATA, HI_TRUE},
            .data_endian     = LVDS_ENDIAN_LITTLE,
            .sync_code_endian = LVDS_ENDIAN_LITTLE, 
            .lane_id = {0, 1, 2, 3},
            .sync_code =  {
				{{0x003,0x007,0x001,0x005},
				{0x043,0x047,0x041,0x045},
				{0x023,0x027,0x021,0x025},
				{0x083,0x087,0x081,0x085}},

				{{0x003,0x007,0x001,0x005},
				{0x043,0x047,0x041,0x045},
				{0x023,0x027,0x021,0x025},
				{0x083,0x087,0x081,0x085}},

				{{0x003,0x007,0x001,0x005},
				{0x043,0x047,0x041,0x045},
				{0x023,0x027,0x021,0x025},
				{0x083,0x087,0x081,0x085}},

				{{0x003,0x007,0x001,0x005},
				{0x043,0x047,0x041,0x045},
				{0x023,0x027,0x021,0x025},
				{0x083,0x087,0x081,0x085}},


            } 
        }
    }
};


static combo_dev_attr_t LVDS_4lane_IMX307_12BIT_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_SUBLVDS,
    {
        .lvds_attr =
        {
			.img_size = {1920, 1080},
			.raw_data_type = RAW_DATA_12BIT,
			.wdr_mode = HI_WDR_MODE_NONE,
			.vsync_type      = {LVDS_VSYNC_NORMAL, 0, 0},
			.fid_type        = {LVDS_FID_NONE, HI_FALSE},
			.sync_mode = LVDS_SYNC_MODE_SAV,
			.data_endian = LVDS_ENDIAN_BIG,
			.sync_code_endian = LVDS_ENDIAN_BIG,
			.lane_id = {3,1,0,2},//{2, 0,3, 1},
			.sync_code =
		   {
				{{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0}},

				{{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0}},

				{{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0}},

				{{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0}},
		   }
        }
    }
};

static combo_dev_attr_t LVDS_4lane_IMX307_12BIT_LQD_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_SUBLVDS,
    {
        .lvds_attr =
        {
			.img_size = {1920, 1080},
			.raw_data_type = RAW_DATA_12BIT,
			.wdr_mode = HI_WDR_MODE_NONE,
			.vsync_type      = {LVDS_VSYNC_NORMAL, 0, 0},
			.fid_type        = {LVDS_FID_NONE, HI_FALSE},
			.sync_mode = LVDS_SYNC_MODE_SAV,
			.data_endian = LVDS_ENDIAN_BIG,
			.sync_code_endian = LVDS_ENDIAN_BIG,
			.lane_id = {2,0,1,3},//{2, 0,3, 1},
			.sync_code =
		   {
				{{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0}},

				{{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0}},

				{{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0}},

				{{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0},
				{0xab0,0xb60,0x800,0x9d0}},
		   }
        }
    }
};

#include "hi_i2c.h"
static int _hi_sdk_isp_init_isp_default_value(void);//Declaration
static int HI_SDK_ISP_set_isp_sensor_value(void);
#define SLOW_FRAMERATE_DISABLE		(0)
#define SLOW_FRAMERATE_LOWLIGHT		(1)
#define SLOW_FRAMERATE_STARLIGHT	(2)
static int _hi_sdk_isp_set_slow_framerate_mode(void);
static int _hi_sdk_isp_set_slow_framerate(uint8_t bValue);//Declaration
static int _hi_sdk_isp_set_slow_framerate_new(uint8_t bValue);//Declaration
static bool _is_imx307_LQD();

int HI_SDK_ISP_set_wdr_mode_ini(uint8_t mode);


static void delay_ms(int ms) { 
    usleep(ms*1000);
}

static HI_S32 hi_isp_SetMipiAttr()
{
    HI_S32 fd;
    combo_dev_attr_t *pstcomboDevAttr;

    /* mipi reset unrest */
    fd = open("/dev/hi_mipi", O_RDWR);
    if (fd < 0)
    {
        printf("warning: open hi_mipi dev failed\n");
        return -1;
    }

	
	if (_isp_attr.sensor_type == SENSOR_MODEL_APTINA_AR0237)
	 {
		 if(_isp_attr.isp_wdr_mode == WDR_MODE_2To1_LINE){
			 pstcomboDevAttr = &SUBLVDS_4lane_SENSOR_AR0237_12BIT_ATTR_WDR;
		 }else if (_isp_attr.isp_wdr_mode == WDR_MODE_NONE){
			 pstcomboDevAttr = &SUBLVDS_4lane_SENSOR_AR0237_12BIT_ATTR;
		 }

	 }

	if (_isp_attr.sensor_type == SENSOR_MODEL_SC2235 || _isp_attr.sensor_type == SENSOR_MODEL_SC2232)
	 {
			 pstcomboDevAttr = &MIPI_CMOS3V3_ATTR;
	 }

	if(_isp_attr.sensor_type == SENSOR_MODEL_IMX307){
		if(_is_imx307_LQD()){
			pstcomboDevAttr = &LVDS_4lane_IMX307_12BIT_LQD_ATTR;
		}else{
			pstcomboDevAttr = &LVDS_4lane_IMX307_12BIT_ATTR;
		}
	}

    /* 1. reset mipi */
    ioctl(fd, HI_MIPI_RESET_MIPI, &pstcomboDevAttr->devno);

    /* 2. reset sensor */
    ioctl(fd, HI_MIPI_RESET_SENSOR, &pstcomboDevAttr->devno);

	
    /* 3. set mipi attr */
    if (ioctl(fd, HI_MIPI_SET_DEV_ATTR, pstcomboDevAttr))
    {
        printf("set mipi attr failed\n");
        close(fd);
        return -1;
    }
	

    usleep(10000);
   // 4. unreset mipi 
    ioctl(fd, HI_MIPI_UNRESET_MIPI, &pstcomboDevAttr->devno);

    //5. unreset sensor
    ioctl(fd, HI_MIPI_UNRESET_SENSOR, &pstcomboDevAttr->devno);

    close(fd);

    return HI_SUCCESS;
}




static uint32_t isp_gpio_get_dir_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = GPIO_BASE_ADDR + gpio_group*0x1000 + 0x400;
	return ret_val;
}

static uint32_t isp_gpio_get_data_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = GPIO_BASE_ADDR + gpio_group*0x1000 + 0x3fc;
	return ret_val;
}

static uint32_t isp_gpio_pin_read(int gpio_group, int gpio_pin)
{
	uint32_t reg_val = 0;

	//pin dir :in
	sdk_sys->read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	sdk_sys->write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);

	//read pin
	sdk_sys->read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	//reg_val &= (1<<gpio_pin);
	reg_val = (reg_val>>gpio_pin)&0x1;
	return reg_val;
}

static void isp_gpio_pin_write(int gpio_group, int gpio_pin, uint8_t val)
{
	uint32_t reg_val = 0;
	
	//pin dir :out
	sdk_sys->read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val |= (1<<gpio_pin);
	sdk_sys->write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);
	
	sdk_sys->read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	reg_val |= (val<<gpio_pin);
	sdk_sys->write_reg(isp_gpio_get_data_addr(gpio_group), reg_val);
}


static void isp_ircut_switch(uint8_t bEnable)//0:daytime   1:night
{
	static uint32_t old_saturation = 0;
	ISP_WB_INFO_S pstWBInfo;
	ISP_SATURATION_ATTR_S pstSatAttr;
	
	if(!old_saturation){
			SOC_CHECK(HI_MPI_ISP_QueryWBInfo(0,&pstWBInfo));//	pstWBInfo.u16Saturation
			printf("old saturation =%d \n",pstWBInfo.u16Saturation);
//			old_saturation = pstWBInfo.u16Saturation;
			
		
		}
	if(!bEnable){
		printf("daylight mode!\r\n");	

		SOC_CHECK(HI_MPI_ISP_GetSaturationAttr(0,&pstSatAttr));
		pstSatAttr.enOpType = OP_TYPE_AUTO;		
		SOC_CHECK(HI_MPI_ISP_SetSaturationAttr(0,&pstSatAttr));
		
		//isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
		//isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
		//isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 1);	

		if (_isp_attr.sensor_type == SENSOR_MODEL_IMX307){
			_isp_attr.bsp_api.BSP_SET_IR_LED(true);
		}else{
			_isp_attr.bsp_api.BSP_SET_IR_LED(false);
		}

		_isp_attr.bsp_api.BSP_IRCUT_SWITCH(ISP_GPIO_DAYLIGHT);
		_isp_attr.gpio_status_old = ISP_GPIO_DAYLIGHT;
		//usleep(1000*150);

		SOC_CHECK(HI_MPI_ISP_QueryWBInfo(0,&pstWBInfo));				
	    printf("new saturation =%d \r\n",pstWBInfo.u16Saturation);

		
		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_ALLDAY){
			_hi_sdk_isp_set_slow_framerate(true);
		}else{			
			_hi_sdk_isp_set_slow_framerate(false);
		}

		SDK_ENC_SetRcParam(0);// day
		
	}else{			
		printf("night mode!\r\n");
		SOC_CHECK(HI_MPI_ISP_GetSaturationAttr(0,&pstSatAttr));
		pstSatAttr.enOpType = OP_TYPE_MANUAL;
     	pstSatAttr.stManual.u8Saturation = 0;

		SOC_CHECK(HI_MPI_ISP_SetSaturationAttr(0,&pstSatAttr));


		//isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1);//IR LED on
		//isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 1);//IR-CUT on
		//isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
        if (_isp_attr.sensor_type == SENSOR_MODEL_IMX307){
            _isp_attr.bsp_api.BSP_SET_IR_LED(false);
        }else{
            _isp_attr.bsp_api.BSP_SET_IR_LED(true);
        }
		_isp_attr.bsp_api.BSP_IRCUT_SWITCH(ISP_GPIO_NIGHT);
		_isp_attr.gpio_status_old = ISP_GPIO_NIGHT;
		//usleep(1000*150);

		SOC_CHECK(HI_MPI_ISP_QueryWBInfo(0,&pstWBInfo));				
	    printf("new saturation =%d \r\n",pstWBInfo.u16Saturation);
		
		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_CLOSE){
			_hi_sdk_isp_set_slow_framerate(true);
		}else{
			_hi_sdk_isp_set_slow_framerate(false);
		}

		SDK_ENC_SetRcParam(1);//night 

	}
	HI_SDK_ISP_set_isp_sensor_value();

	if(_isp_attr.ispCfgAttr){
		HI_ISP_cfg_set_all(_isp_attr.gpio_status_old, 1, _isp_attr.ispCfgAttr);
	}
}

static void isp_white_light_switch(uint8_t bEnable)
{
	if(bEnable){
		_isp_attr.bsp_api.BSP_SET_WHITE_LIGHT_LED(true);
	}else{
		_isp_attr.bsp_api.BSP_SET_WHITE_LIGHT_LED(false);
	}
}

static void isp_smartmode_isp_switch(uint8_t bEnable)
{
	ISP_SATURATION_ATTR_S pstSatAttr;
	SOC_CHECK(HI_MPI_ISP_GetSaturationAttr(0,&pstSatAttr));
	if(bEnable){
		pstSatAttr.enOpType = OP_TYPE_AUTO;
		SOC_CHECK(HI_MPI_ISP_SetSaturationAttr(0,&pstSatAttr));

		_isp_attr.bsp_api.BSP_IRCUT_SWITCH(ISP_GPIO_DAYLIGHT);
		_isp_attr.gpio_status_old = ISP_GPIO_DAYLIGHT;

		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_ALLDAY){
			_hi_sdk_isp_set_slow_framerate(true);
		}else{
			_hi_sdk_isp_set_slow_framerate(false);
		}

	}else{
		pstSatAttr.enOpType = OP_TYPE_MANUAL;
		pstSatAttr.stManual.u8Saturation = 0;
		SOC_CHECK(HI_MPI_ISP_SetSaturationAttr(0,&pstSatAttr));

		_isp_attr.bsp_api.BSP_IRCUT_SWITCH(ISP_GPIO_NIGHT);
		_isp_attr.gpio_status_old = ISP_GPIO_NIGHT;

		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_CLOSE){
			_hi_sdk_isp_set_slow_framerate(false);
		}else{
			_hi_sdk_isp_set_slow_framerate(true);
		}
	}

	HI_SDK_ISP_set_isp_sensor_value();
	if(_isp_attr.ispCfgAttr){
		HI_ISP_cfg_set_all(_isp_attr.gpio_status_old, 1, _isp_attr.ispCfgAttr);
	}
}


static uint32_t gains_calculate(void)
{
	uint32_t ret_iso = 1;

	ISP_DEV IspDev = 0;
    ISP_EXP_INFO_S stExpInfo ;	
 	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(IspDev,&stExpInfo));
//	ret_iso = (stExpInfo.u32AGain* ((stExpInfo.u32DGain* (stExpInfo.u32ISPDGain>> 10))>>4))>>6;
	ret_iso = (uint32_t)((unsigned long long)stExpInfo.u32AGain* (unsigned long long)stExpInfo.u32DGain* (unsigned long long)stExpInfo.u32ISPDGain >> 20);

	return ret_iso;
}

static uint32_t isp_get_iso()
{
	ISP_DEV IspDev = 0;
    ISP_EXP_INFO_S stExpInfo ;
 	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(IspDev,&stExpInfo));
	return stExpInfo.u32AGain*stExpInfo.u32DGain*100;

}

static uint8_t sdk_isp_calculate_exposure(uint32_t old_state)
{

	uint8_t ret_val = 0;
	//HI_U32 switch_area[2] = {0x20fa, 0x2544}; 
	HI_U32 switch_area[2] = {0x2844, 0x67};

	ISP_DEV IspDev = 0;
    ISP_EXP_INFO_S stExpInfo ;		
 	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(IspDev,&stExpInfo));
	//u32Exposure[0x400,0xFFFFFFF];
	
	printf("aveLum = 0x%04x/%02x\r\n",stExpInfo.u32Exposure, stExpInfo.u8AveLum);
	if(!old_state){
		if(stExpInfo.u32Exposure > switch_area[0]){
			ret_val = 1;
		}else{
			ret_val = 0;
		}
		if(stExpInfo.u32Exposure < switch_area[1]){
			ret_val = 0;
		}else{
			ret_val = 1;
		}
	}

	printf("old_state:%d/%d\r\n", old_state, ret_val);
	

	return ret_val;//0:daytime 1:night
}


static int isp_get_isp_strength_value(lpHiIspStrength val)
{
	int ret_val = 0;
	switch(_isp_attr.gpio_status_old){
		default:
		case ISP_GPIO_DAYLIGHT:
		{
			ret_val = val->daylight_val + val->daylight_val*(val->strength - 3)/2;
		}
		break;
		case ISP_GPIO_NIGHT:
		{
			ret_val = val->night_val + val->night_val*(val->strength - 3)/2;
		}
		break;
	}
	if(ret_val> 255){
		ret_val = 255;
	}
	if(ret_val < 0){
		ret_val = 0;
	}
	return ret_val;
}


/*

static int isp_get_isp_value(lpHiIspStrength val)
{
	int ret_val = 0;
	switch(_isp_attr.gpio_status_old){
		default:
		case ISP_GPIO_DAYLIGHT:
		{
			ret_val = val->daylight_val;
		}
		break;
		case ISP_GPIO_NIGHT:
		{
			ret_val = val->night_val;
		}
		break;
	}

	if(ret_val < 0){
		ret_val = 0;
	}
	return ret_val;
}
*/
static int isp_get_denoise3d_global_strength_value(lpHiIspStrength val)
{
	int ret_val = 0;
	switch(_isp_attr.gpio_status_old){
		default:
		case ISP_GPIO_DAYLIGHT:
		{
			ret_val = val->daylight_val + val->daylight_val*(val->strength - 3)/2;
		}
		break;
		case ISP_GPIO_NIGHT:
		{
			ret_val = val->night_val + val->night_val*(val->strength - 3)/2;
		}
		break;
	}
	if(ret_val> 1408){
		ret_val = 1408;
	}
	if(ret_val < 0){
		ret_val = 0;
	}

	return ret_val;
}

static int isp_get_3DNR_YPK_strength_value(lpHiIspStrength val)
{
	int ret_val = 0;
	switch(_isp_attr.gpio_status_old){
		default:
		case ISP_GPIO_DAYLIGHT:
		{
			ret_val = val->daylight_val + val->daylight_val*(val->strength - 3)/2;
		}
		break;
		case ISP_GPIO_NIGHT:
		{
			ret_val = val->night_val + val->night_val*(val->strength - 3)/2;
		}
		break;
	}
	if(ret_val> 63){
		ret_val = 63;
	}
	if(ret_val < 0){
		ret_val = 0;
	}
	return ret_val;
}

static int isp_get_3DNR_YSF_strength_value(lpHiIspStrength val)
{
	int ret_val = 0;
	switch(_isp_attr.gpio_status_old){
		default:
		case ISP_GPIO_DAYLIGHT:
		{
			ret_val = val->daylight_val + val->daylight_val*(val->strength - 3)/2;
		}
		break;
		case ISP_GPIO_NIGHT:
		{
			ret_val = val->night_val + val->night_val*(val->strength - 3)/2;
		}
		break;
	}
	if(ret_val> 200){
		ret_val = 200;
	}
	if(ret_val < 0){
		ret_val = 0;
	}
	return ret_val;
}

static int isp_get_3DNR_YTF_strength_value(lpHiIspStrength val)
{
	int ret_val = 0;
	switch(_isp_attr.gpio_status_old){
		default:
		case ISP_GPIO_DAYLIGHT:
		{
			ret_val = val->daylight_val + val->daylight_val*(val->strength - 3)/2;
		}
		break;
		case ISP_GPIO_NIGHT:
		{
			ret_val = val->night_val + val->night_val*(val->strength - 3)/2;
		}
		break;
	}
	if(ret_val> 128){
		ret_val = 128;
	}
	if(ret_val < 0){
		ret_val = 0;
	}
	return ret_val;
}





int HI_SDK_ISP_sensor_flicker(uint8_t bEnable, uint8_t frequency, uint8_t mode)
{	
	ISP_EXPOSURE_ATTR_S pstExpAttr;	
	ISP_DEV IspDev = 0;
	SOC_CHECK(HI_MPI_ISP_GetExposureAttr(IspDev,&pstExpAttr));
	if(bEnable != 0xff){
		pstExpAttr.stAuto.stAntiflicker.bEnable = bEnable;
	}else{
		printf("pstAntiflicker.bEnable = %d\r\n", pstExpAttr.stAuto.stAntiflicker.bEnable);
		pstExpAttr.stAuto.stAntiflicker.bEnable = HI_TRUE;
	}
	pstExpAttr.stAuto.stAntiflicker.bEnable = HI_FALSE;//HI_TRUE;
	
	if(frequency){
		pstExpAttr.stAuto.stAntiflicker.u8Frequency = frequency;
		_isp_attr.filter_frequency = frequency;
		if(_isp_attr.ispCfgAttr){			
			_isp_attr.ispCfgAttr->impCfgAttr.flick_frequency = _isp_attr.filter_frequency;
		}
	}else{
		pstExpAttr.stAuto.stAntiflicker.u8Frequency = _isp_attr.filter_frequency;
	}
	printf("%s---%d:%d\r\n", __FUNCTION__, frequency, bEnable);
	pstExpAttr.stAuto.stAntiflicker.enMode = mode ? ISP_ANTIFLICKER_AUTO_MODE:ISP_ANTIFLICKER_NORMAL_MODE;
	SOC_CHECK(HI_MPI_ISP_SetExposureAttr(IspDev,&pstExpAttr));
	printf("%s-%d:%d/%d\r\n", __FUNCTION__, __LINE__, pstExpAttr.stAuto.stAntiflicker.bEnable,pstExpAttr.stAuto.stAntiflicker.u8Frequency);
	return 0;

}
int sensor_gpio_reset(){
	isp_gpio_pin_write(0, 7, 1); //reset sensor  GPIO0_7  hi3516cv300
	usleep(2000);
	isp_gpio_pin_write(0, 7, 0); //reset sensor 
	usleep(2000);
	isp_gpio_pin_write(0, 7, 1); //reset sensor 
	usleep(2000);

	return 0;

}

emSENSOR_MODEL HI_SDK_ISP_sensor_check()
{
	//save file for sensor check in case of saving time for checking sensor
	FILE* fd = NULL;
	char sensor_type[4] = {0};
	char cmd[32];
	fd = fopen(SENSOR_TYPE_FILE, "rb");

	if(fd){
		fread(sensor_type, sizeof(sensor_type), 1, fd);
		_isp_attr.sensor_type = atoi(sensor_type);
		fclose(fd);
		if(_isp_attr.sensor_type < SENSOR_MODEL_CNT){
			printf("get sensor type from file:%d\n", _isp_attr.sensor_type);
			goto reset_sensor;
		}
	}else{
		//need to check sensor by I2C
	}
	
	do{
		if(SENSOR_AR0237_probe()){
            _isp_attr.sensor_type = SENSOR_MODEL_APTINA_AR0237;
			printf("_isp_attr.sensor_type = SENSOR_MODEL_APTINA_AR0237\n");
            break;
        }
        if(SENSOR_SC2235_probe()){
            _isp_attr.sensor_type = SENSOR_MODEL_SC2235;
			printf("_isp_attr.sensor_type = SENSOR_MODEL_APTINA_SC2235\n");
            break;
        }
		if(SENSOR_SC2232_probe()){
			_isp_attr.sensor_type = SENSOR_MODEL_SC2232;
			printf("_isp_attr.sensor_type = SENSOR_MODEL_APTINA_SC2232\n");
			break;
		}
		if(IMX307_probe()){
			_isp_attr.sensor_type = SENSOR_MODEL_IMX307;
			printf("_isp_attr.sensor_type = SENSOR_MODEL_IMX307\n");
			break;
		}

	}while(0);

	//set a file for for sensor check in case of saving time for checking sensor
	snprintf(cmd, sizeof(cmd), "echo %d > %s", _isp_attr.sensor_type, SENSOR_TYPE_FILE);
	system(cmd);

reset_sensor:

	sensor_gpio_reset();

	//init mipi interface
	hi_isp_SetMipiAttr();

	return _isp_attr.sensor_type;
}

static stSwitchIrcutInfo ircut_state_info = {0};


//for pwm light
static pthread_t _pwm_light_pid = NULL;
static bool _pmw_light_proc_triger = false;

static void isp_pwm_ctrl(uint32_t exp, uint16_t radio)
{
	if(_isp_attr.bsp_api.BSP_SET_PWM_DUTY_CYCLE){
		_isp_attr.bsp_api.BSP_SET_PWM_DUTY_CYCLE(1, radio);
	}
}

#define ISP_CAL_HIST_REFLECT_THRESHOLD_POINT (64)
#define ISP_CAL_HIST_REFLECT_THRESHOLD (0.4)

static uint8_t _is_light_reflect()
{
	uint8_t ret = 0;
	uint64_t darkCount = 0, totalCount = 0;
	int i = 0;
	ISP_STATISTICS_S ispStat;
	HI_MPI_ISP_GetStatistics(0, &ispStat);
	for(i = 0; i < ISP_CAL_HIST_REFLECT_THRESHOLD_POINT; i++){
		darkCount += ispStat.stAEStat.au32Hist1024Value[i];
	}
	totalCount = darkCount;
	for(i = ISP_CAL_HIST_REFLECT_THRESHOLD_POINT; i < HIST_1024_NUM-1; i++){
		totalCount += ispStat.stAEStat.au32Hist1024Value[i];
	}
	//printf("dark/light:%llu/%llu=%f\n", darkCount, totalCount, (float)darkCount/totalCount);

	if((float)darkCount/totalCount > ISP_CAL_HIST_REFLECT_THRESHOLD){
		ret = 1;
	}
	return ret;
}

#define PWM_STEP_COUNT (37)

static void _isp_pwm_ctrl_proc(void)
{
	uint16_t pwm_step[PWM_STEP_COUNT] = {
		0,1,2,3,4,
		5,7,10,15,20,30,40,50,60,80,100,130,160,190,220,250,
		280,320,360,400,410,450,500,550,600,650,700,750,800,850,900,1000
	};

	uint16_t step_intervel[PWM_STEP_COUNT] = {
		1000,1000,700,500,500,
		500,500,400,400,400,300,300,300,300,300,300,200,200,200,150,150,
		150,150,100,100,50,50,50,0,0,0,0,0,0,0,0,0
	};
	uint32_t exposure;
	uint32_t expTarget;
	uint32_t tolerance;
	uint32_t expThreshold;
	uint16_t step[64];
	uint8_t stepCount;
	uint8_t isReflect;
	SDK_ISP_PWM_LIGHT_CONTROL Ctrl;
	ISP_EXP_INFO_S stExpInfo ;

    prctl(PR_SET_NAME, "_isp_pwm_ctrl_proc");

	if(SENSOR_MODEL_IMX307 == _isp_attr.sensor_type){
		expTarget = 90000*25;
		tolerance = 300000;
		expThreshold = 90000*170;
		stepCount = PWM_STEP_COUNT;
		memcpy(step, pwm_step, sizeof(pwm_step));
		Ctrl = isp_pwm_ctrl;
	}

	//turn off the light
	isp_pwm_ctrl(0, 0);

	while(_pmw_light_proc_triger){
		if(HI_SUCCESS == HI_MPI_ISP_QueryExposureInfo(0,&stExpInfo)){
			exposure =  ((HI_U64)stExpInfo.u32AGain * (HI_U64)stExpInfo.u32DGain * (HI_U64)stExpInfo.u32ISPDGain) 
	                      * ((HI_U64)stExpInfo.u32ExpTime)>> 30;
			isReflect = _is_light_reflect();
			if(ISP_GPIO_DAYLIGHT == _isp_attr.gpio_status_old){
				SDK_ISP_PWM_light_linkage(exposure, expTarget, tolerance, expThreshold, step, stepCount, step_intervel, isReflect, Ctrl);
			}
		}
		usleep(500000);//500ms
	}

	//turn off the light
	isp_pwm_ctrl(0, 0);

	pthread_exit(0);
}

static void _isp_pwm_light_init()
{
    pthread_attr_t pthread_attr;
    int nRet = 0;
	_pmw_light_proc_triger = true;

	nRet = pthread_attr_init(&pthread_attr);
	if(0 == nRet)
	{
        pthread_attr_setstacksize(&pthread_attr, 131072);
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
        if (0 != pthread_create(&_pwm_light_pid, &pthread_attr, _isp_pwm_ctrl_proc, NULL)){
            printf("%s: create isp pwm light thread failed!\n", __FUNCTION__);
        }
        pthread_attr_destroy(&pthread_attr);
	}

}

static void _isp_pwm_light_deinit()
{
	if(_pwm_light_pid){
		_pmw_light_proc_triger = false;
		pthread_join(_pwm_light_pid, 0);
		_pwm_light_pid = 0;
	}
}

static void isp_ircut_contrl_init()
{
	stIspIrcutSwitch isp_ircut_control={
		.isp_ircut_switch = isp_ircut_switch,
		.isp_white_light_switch = isp_white_light_switch,
		.isp_smartmode_isp_switch = isp_smartmode_isp_switch,
	};
	sdk_isp_ircut_switch_init(isp_ircut_control);
	if(_isp_attr.ispCfgAttr){
		ircut_state_info.day_to_night_factor = _isp_attr.ispCfgAttr->impCfgAttr.DaylightToNight;
		ircut_state_info.night_to_day_factor = _isp_attr.ispCfgAttr->impCfgAttr.NightToDaylight[0];
		memcpy(ircut_state_info.to_day_factor,_isp_attr.ispCfgAttr->impCfgAttr.NightToDaylight,sizeof(ircut_state_info.to_day_factor));
	}else{
		ircut_state_info.day_to_night_factor = 45;
		ircut_state_info.night_to_day_factor = 10;

		ircut_state_info.to_day_factor[0] = 10;
		ircut_state_info.to_day_factor[1] = 7;
		ircut_state_info.to_day_factor[2] = 4;
		ircut_state_info.to_day_factor[3] = 2;
		ircut_state_info.to_day_factor[4] = 1;
		ircut_state_info.to_day_factor[5] = 0.5;
		ircut_state_info.to_day_factor[6] = 0.25;
		ircut_state_info.to_day_factor[7] = 0.125;

	}

	ircut_state_info.daynight_mode_state = _isp_attr.gpio_status_old;
	ircut_state_info.pre_daynight_mode_state = _isp_attr.gpio_status_old;

	ircut_state_info.very_likely_reflect = 0;
	ircut_state_info.night_stable_cnt = 0;
	ircut_state_info.day_stable_cnt = 0;
	ircut_state_info.j = 0;
	ircut_state_info.detect_to_day_cnt  = 0;
	ircut_state_info.detect_to_night_cnt = 0;
	ircut_state_info.pre_expose_factor = 0;


	ircut_state_info.md_alarm_state = false;
	ircut_state_info.md_alarm_switch = false;
	ircut_state_info.md_alarm_lock = false;
	ircut_state_info.md_alarm_open_light = false;
	ircut_state_info.md_alarm_stop = false;

}

int HI_SDK_ISP_get_colortoblack_range(uint8_t *val)
{
	*val = _isp_attr.color_to_black_rank;
	return 0;
}

int  HI_SDK_ISP_set_colortoblack_range(uint8_t colortoblackrange)
{
	int i;
	if(colortoblackrange<1){
		colortoblackrange=1;
	}
	if(colortoblackrange>9){
		colortoblackrange=9;
	}
	_isp_attr.color_to_black_rank = colortoblackrange;
	_isp_attr.color_to_black_val = _isp_attr.ispCfgAttr->impCfgAttr.ColortoblackVal[_isp_attr.color_to_black_rank - 1];
	return 0;
}

static void isp_ircut_switch_linkage_control_init()
{
	ISP_EXP_INFO_S stExpInfo ;
	ISP_PUB_ATTR_S stPubAttr;

	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(0,&stExpInfo));
	SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));

	float max_exposure_time = 1/stPubAttr.f32FrameRate * 1000;
	float cur_exposure_time = stExpInfo.u32ExpTime/1000;
	float cur_gain = gains_calculate()/1024;

	ircut_state_info.day_to_night_factor =  _isp_attr.color_to_black_val;

	if(cur_gain > 1){
		ircut_state_info.cur_expose_factor = cur_gain;
	}else{
		ircut_state_info.cur_expose_factor = cur_exposure_time / max_exposure_time * cur_gain;
	}

}


static void* isp_md_alarm_cal(void* param)
{
	prctl(PR_SET_NAME, "isp_md_alarm_cal");
	uint32_t alarm_keep_count = 0;
	pthread_detach(pthread_self());
	while(_isp_attr.md_alarm_thread_trigger){
		isp_md_alarm_delay_cal(&ircut_state_info, &alarm_keep_count,_isp_attr.md_lock);
		usleep(10000);
	}
	pthread_exit(0);
}

int HI_SDK_ISP_ircut_auto_switch(int vin, uint8_t type)//1:software   0: hardware 
{
	if(type != ISP_IRCUT_CONTROL_MODE_HARDWARE && type != ISP_IRCUT_CONTROL_MODE_SOFTWARE && type != ISP_IRCUT_CONTROL_MODE_IRCUTLINKAGE){
		return -1;
	}
	if(sdk_vin){
		sdk_vin->get_md_alarm_state(&(ircut_state_info.md_alarm_state));
	}

	ircut_state_info.daynight_mode_state = _isp_attr.gpio_status_old;
	if(_isp_attr.ircut_auto_switch_enable){
			if(_isp_attr.ircut_control_mode == ISP_IRCUT_CONTROL_MODE_IRCUTLINKAGE){
				
				uint32_t gpio_status_cur, gpio_status_old;
				isp_ircut_switch_linkage_control_init();
				gpio_status_cur = _isp_attr.bsp_api.BSP_GET_PHOTOSWITCH();
				isp_ircut_switch_linkage_control(&ircut_state_info, gpio_status_cur);
				_isp_attr.gpio_status_old = ircut_state_info.daynight_mode_state;
				
			}else if(_isp_attr.ircut_control_mode == ISP_IRCUT_CONTROL_MODE_HARDWARE){
				uint32_t gpio_status_cur;
				gpio_status_cur = _isp_attr.bsp_api.BSP_GET_PHOTOSWITCH();
				if(_isp_attr.ircut_switch_control_mode == IRLED_CONTROL_MODE){
					isp_ircut_switch_hardware_control(&_isp_attr.gpio_status_old, gpio_status_cur);
				}else if(_isp_attr.ircut_switch_control_mode == LIGHT_CONTROL_MODE){
					ISP_EXP_INFO_S stExpInfo;
					ISP_PUB_ATTR_S stPubAttr;

					SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(0,&stExpInfo));
					SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));

					float max_exposure_time = 1/stPubAttr.f32FrameRate * 1000;
					float cur_exposure_time = stExpInfo.u32ExpTime/1000;
					float cur_gain = gains_calculate()/1024;

					ircut_state_info.cur_expose_factor = cur_exposure_time / max_exposure_time * cur_gain;

					isp_ircut_switch_hardware_control_lightmode(gpio_status_cur, &ircut_state_info);
					_isp_attr.gpio_status_old = ircut_state_info.daynight_mode_state;
				}else if(_isp_attr.ircut_switch_control_mode == SMART_CONTROL_MODE){
					isp_ircut_switch_hardware_control_smartmode(&_isp_attr.gpio_status_old, gpio_status_cur, &ircut_state_info);
				}

			} else if(_isp_attr.ircut_control_mode == ISP_IRCUT_CONTROL_MODE_SOFTWARE) {
				ISP_EXP_INFO_S stExpInfo;
				ISP_PUB_ATTR_S stPubAttr;

				SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(0,&stExpInfo));
				SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));

				float max_exposure_time = 1/stPubAttr.f32FrameRate * 1000; // ms
				float cur_exposure_time = stExpInfo.u32ExpTime/1000;
				float cur_gain = gains_calculate()/1024;   //ret_gain
				ircut_state_info.cur_expose_factor = cur_exposure_time / max_exposure_time * cur_gain;
                if(_isp_attr.sensor_type !=  SENSOR_MODEL_IMX307)
                {
                    isp_ircut_switch_sofeware_control_2(&ircut_state_info);
                }
				_isp_attr.gpio_status_old = ircut_state_info.daynight_mode_state;
			}

	}else{
			//do nothing
	}
	return 0;
}


int HI_SDK_ISP_set_mirror(int vin, bool mirror)
{
	if(_isp_attr.sensor_set_mirror){		
		printf("%s:mirror =%d\n",__FUNCTION__,mirror);
		_isp_attr.sensor_set_mirror(mirror);///\D7\F3\D3\D2
	}

/*
	printf("******%s***mirror =%d**\n",__FUNCTION__,mirror);
		VPSS_GRP VpssGrp = 0;
		VPSS_CHN VpssChn = 1;//main stream
		VPSS_CHN_ATTR_S stVpssChnAttr;
	
		_isp_attr.vpss_mirror = mirror;
	
		for(VpssChn = 1;VpssChn <= 3;VpssChn++){
			SOC_CHECK(HI_MPI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr));
			stVpssChnAttr.bMirror = mirror ? HI_TRUE : HI_FALSE;
			SOC_CHECK(HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr));
		}
	

*/
	return 0;
}

int HI_SDK_ISP_set_flip(int vin, bool flip)
{  
	if(_isp_attr.sensor_set_flip){
		printf("%s:flip =%d\n",__FUNCTION__,flip);
		_isp_attr.sensor_set_flip(flip);//\C9\CF\CF\C2
	}

/* 
	printf("******%s**flip =%d ***\n",__FUNCTION__,flip);
		
	VI_CHN_ATTR_S vi_chn_attr;
	SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
	vi_chn_attr.bFlip = flip ? HI_TRUE : HI_FALSE;
	SOC_CHECK(HI_MPI_VI_SetChnAttr(vin, &vi_chn_attr));
*/
	return 0;
}

int HI_SDK_ISP_set_saturation(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32SatuVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("saturation set:%d\r\n", val);

	return 0;
}

int HI_SDK_ISP_get_saturation(int vin, uint16_t *val)
{

	ISP_WB_INFO_S pstWBInfo;
	SOC_CHECK(HI_MPI_ISP_QueryWBInfo(0,&pstWBInfo));//	pstWBInfo.u16Saturation
	printf("saturation get:%d\r\n", pstWBInfo.u16Saturation);
	*val = pstWBInfo.u16Saturation;
	return 0;
}


int HI_SDK_ISP_set_contrast(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32ContrVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("contrast set:%d\r\n", val);
	return 0;
}

int HI_SDK_ISP_set_brightness(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32LumaVal= val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("brightness set:%d\r\n", val);
	return 0;
}

int HI_SDK_ISP_set_hue(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32HueVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("hue set:%d\r\n", val);
	return 0;
}


int HI_SDK_ISP_set_advance_lowlight_enable(uint8_t bEnable)
{
	printf("%s:%d\r\n", __FUNCTION__, bEnable);     // 0   1   2  3 
	
	_isp_attr.lowlight_mode = bEnable;

	if(_isp_attr.ispCfgAttr){
		_isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate = _isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO ? true : false;
		_isp_attr.ispCfgAttr->impCfgAttr.src_framerate = _isp_attr.src_framerate;
		_isp_attr.ispCfgAttr->aeCfgAttr.LowLightModeEnable = _isp_attr.isp_framerate_status;
		_isp_attr.ispCfgAttr->aeCfgAttr.StarLightModeEnable = _isp_attr.starlight_mode_enable == ISP_LOWLIGHT_MODE_STARLIGHT ? true : false;
		printf("AutoSlowFrameRate:%d\r\n", _isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate);
	}

	if(_isp_attr.sensor_type == SENSOR_MODEL_APTINA_AR0237){	
		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_ALLDAY || _isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_STARLIGHT){
			_isp_attr.isp_framerate_status = true;
			_hi_sdk_isp_set_slow_framerate(true);
		}else if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_NIGHT && _isp_attr.gpio_status_old == ISP_GPIO_NIGHT){
			_isp_attr.isp_framerate_status = true;
			_hi_sdk_isp_set_slow_framerate(true);
		}else if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO){
			_isp_attr.isp_framerate_status = true;
			_hi_sdk_isp_set_slow_framerate(true);
		}else{
			_isp_attr.isp_framerate_status = false;
			_hi_sdk_isp_set_slow_framerate(false);
		}

	}else{
		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_NIGHT && _isp_attr.gpio_status_old == ISP_GPIO_NIGHT ){
			_hi_sdk_isp_set_slow_framerate_new(SLOW_FRAMERATE_LOWLIGHT);
		}else if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO || _isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_ALLDAY){
			_hi_sdk_isp_set_slow_framerate_new(SLOW_FRAMERATE_LOWLIGHT);
		}else if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_STARLIGHT){
			_hi_sdk_isp_set_slow_framerate_new(SLOW_FRAMERATE_STARLIGHT);
		}else{
			_hi_sdk_isp_set_slow_framerate_new(SLOW_FRAMERATE_DISABLE);
		}
	}
	return 0;
}


int HI_SDK_ISP_set_src_framerate(unsigned int framerate)
{
	ISP_DEV IspDev = 0;
	ISP_PUB_ATTR_S stPubAttr;	
	ISP_EXPOSURE_ATTR_S stExpAttr;
	
	SOC_CHECK(HI_MPI_ISP_GetPubAttr(IspDev,&stPubAttr));
	
	if(framerate <= 25){
		stPubAttr.f32FrameRate = 25;
		_isp_attr.filter_frequency = 50;
	}else{
		stPubAttr.f32FrameRate= 30;
		_isp_attr.filter_frequency = 60;
	}

//	_isp_attr.src_framerate = stPubAttr.f32FrameRate;
//	SOC_CHECK(HI_MPI_ISP_SetPubAttr(IspDev,&stPubAttr));


	SOC_CHECK(HI_MPI_ISP_GetExposureAttr(IspDev,&stExpAttr));
	stExpAttr.stAuto.stAntiflicker.bEnable = HI_TRUE; 
	stExpAttr.stAuto.stAntiflicker.u8Frequency = _isp_attr.filter_frequency;	
	SOC_CHECK(HI_MPI_ISP_SetExposureAttr(IspDev,&stExpAttr));
	
//	sleep(2);

	_hi_sdk_isp_init_isp_default_value();

	return 0;
}

int HI_SDK_ISP_get_sharpen(uint8_t *val)
{	
	
    ISP_DEV IspDev = 0;
    ISP_SHARPEN_ATTR_S SharpenAttr;
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(IspDev,&SharpenAttr));
	
	int iso = isp_get_iso();

	if(iso < 200){
		*val = SharpenAttr.stAuto.au16SharpenUd[0];
	}else if(iso < 400){
		*val = SharpenAttr.stAuto.au16SharpenUd[1];
	}else if(iso < 800){
		*val = SharpenAttr.stAuto.au16SharpenUd[2];
	}else if(iso < 1600){
		*val = SharpenAttr.stAuto.au16SharpenUd[3];
	}else if(iso < 3200){
		*val = SharpenAttr.stAuto.au16SharpenUd[4];
	}else if(iso < 6400){
		*val = SharpenAttr.stAuto.au16SharpenUd[5];
	}else if(iso < 12800){
		*val = SharpenAttr.stAuto.au16SharpenUd[6];
	}else if(iso < 25600){
		*val = SharpenAttr.stAuto.au16SharpenUd[7];
	}else if(iso < 51200){
		*val = SharpenAttr.stAuto.au16SharpenUd[8];
	}else if(iso < 102400){
		*val = SharpenAttr.stAuto.au16SharpenUd[9];
	}else if(iso < 204800){
		*val = SharpenAttr.stAuto.au16SharpenUd[10];
	}else if(iso < 409600){	
	    *val = SharpenAttr.stAuto.au16SharpenUd[11];
	}else if(iso < 819200){
		*val = SharpenAttr.stAuto.au16SharpenUd[12];
	}else if(iso < 1638400){
		*val = SharpenAttr.stAuto.au16SharpenUd[13];
	}else if(iso < 3276800){
		*val = SharpenAttr.stAuto.au16SharpenUd[14];
	}else {
		*val = SharpenAttr.stAuto.au16SharpenUd[15];
	};

	return 0;
}


int HI_SDK_ISP_set_sharpen(uint8_t val, uint8_t bManual)
{
	
	ISP_SHARPEN_ATTR_S   stSharpenAttr;
	
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(0,&stSharpenAttr));
	stSharpenAttr.bEnable = HI_TRUE;
	if(bManual){
		printf("Sharpen set:%d\n",val);
		stSharpenAttr.enOpType =  OP_TYPE_MANUAL;
		stSharpenAttr.stManual.u8SharpenD = val / 3;	
		stSharpenAttr.stManual.u16SharpenUd = val / 3;
	}else{
		stSharpenAttr.enOpType =  OP_TYPE_AUTO;
	}
	
 	SOC_CHECK(HI_MPI_ISP_SetSharpenAttr(0,&stSharpenAttr)); 
	return 0;
}

int HI_SDK_ISP_set_scene_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
    ISP_DRC_ATTR_S pstDRC;
	
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));
	switch(mode){
		default:
		case ISP_SCENE_MODE_AUTO:
			HI_SDK_ISP_sensor_flicker(0xff,0,1);
			pstDRC.bEnable = HI_TRUE;
			switch(_isp_attr.sensor_type){
				default:

	 			case SENSOR_MODEL_APTINA_AR0237:
					break;	
				case SENSOR_MODEL_SC2235:
					break;

			} 
			break;
		case ISP_SCENE_MODE_INDOOR:
			HI_SDK_ISP_sensor_flicker(0xff,0,0);
			pstDRC.bEnable = HI_TRUE;
			switch(_isp_attr.sensor_type){
				default:
		case SENSOR_MODEL_APTINA_AR0237:
					break;
		case SENSOR_MODEL_SC2235:
					break;
			}
			break;
		case ISP_SCENE_MODE_OUTDOOR:
			HI_SDK_ISP_sensor_flicker(0xff,0,1);
			pstDRC.bEnable = HI_FALSE;
			switch(_isp_attr.sensor_type){
				default:
			case SENSOR_MODEL_APTINA_AR0237:
					break;
			case SENSOR_MODEL_SC2235:
					break;
			}
			break;
	}
	_isp_attr.isp_auto_drc_enabled = pstDRC.bEnable;
	//SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&pstDRC));
	return 0;

}

int HI_SDK_ISP_set_WB_mode(uint32_t mode)
{
	ISP_AWB_ATTR_EX_S  pstAWBAttrEx;
	SOC_CHECK(HI_MPI_ISP_GetAWBAttrEx(0,&pstAWBAttrEx));
	switch(mode){
	default:
	case ISP_SCENE_MODE_AUTO:
		pstAWBAttrEx.u8ZoneRadius = 16;
		break;
	case ISP_SCENE_MODE_INDOOR:		
		pstAWBAttrEx.u8ZoneRadius = 8;
		break;
	case ISP_SCENE_MODE_OUTDOOR:
		
		pstAWBAttrEx.u8ZoneRadius = 16;
		break;
	}
	SOC_CHECK(HI_MPI_ISP_SetAWBAttrEx(0,&pstAWBAttrEx));

	return 0;
}

int HI_SDK_ISP_set_ircut_control_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	_isp_attr.ircut_control_mode = mode;
	return 0;
}
static int _hi_sdk_isp_set_slow_framerate_mode()
{
	if(_isp_attr.ispCfgAttr){
		_isp_attr.ispCfgAttr->aeCfgAttr.StarLightModeEnable = _isp_attr.starlight_mode_enable;
		_isp_attr.ispCfgAttr->aeCfgAttr.LowLightModeEnable = _isp_attr.isp_framerate_status;

		if(_isp_attr.isp_framerate_status && _isp_attr.ispCfgAttr->aeCfgAttr.LowLightExptimeFlag){
			_isp_attr.ispCfgAttr->aeCfgAttr.LowLightModeEnable = true;
		}else{
			_isp_attr.ispCfgAttr->aeCfgAttr.LowLightModeEnable = false;
		}
		if(!_isp_attr.ispCfgAttr->aeCfgAttr.GainThresholdFlag){
			_isp_attr.ispCfgAttr->aeCfgAttr.StarLightModeEnable = _isp_attr.starlight_mode_enable = false;
		}

	}
	if(_isp_attr.starlight_mode_enable){		
		if(_isp_attr.isp_framerate_status){
			printf("low_light mode is [on] \n");
		}else{		
			printf("starlight mode is [on] \n");
		}
	}else{
		printf("starlight/low_light mode is [off] \n");
	}
	HI_SDK_ISP_set_isp_sensor_value();	
	HI_ISP_cfg_set_ae(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT ? 0 : 1, _isp_attr.ispCfgAttr);
	return 0;

}


int HI_SDK_ISP_set_ircut_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	_isp_attr.md_lock = HI_TRUE;
	isp_white_light_switch(0);
	if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){
		isp_smartmode_isp_switch(1);
	}else{
		isp_smartmode_isp_switch(0);
	}

	switch(mode){
		default:
		case ISP_IRCUT_MODE_AUTO:
			_isp_attr.ircut_auto_switch_enable = HI_TRUE;
			_isp_attr.ircut_switch_control_mode = IRLED_CONTROL_MODE;
			break;
		case ISP_IRCUT_MODE_DAYLIGHT:
			_isp_attr.ircut_auto_switch_enable = HI_FALSE;
			isp_ircut_switch(0);
			break;
		case ISP_IRCUT_MODE_NIGHT:
			_isp_attr.ircut_auto_switch_enable = HI_FALSE;
			isp_ircut_switch(1);
			break;
		case ISP_IRCUT_MODE_LIGHTMODE:
			_isp_attr.ircut_auto_switch_enable = HI_TRUE;
			isp_ircut_switch(0);
			_isp_attr.ircut_switch_control_mode = LIGHT_CONTROL_MODE;
			break;
		case ISP_IRCUT_MODE_SMARTMODE:
			_isp_attr.ircut_auto_switch_enable = HI_TRUE;
			_isp_attr.ircut_switch_control_mode = SMART_CONTROL_MODE;
			_isp_attr.md_lock = HI_FALSE;	
            if(_isp_attr.sensor_type !=  SENSOR_MODEL_IMX307) {
                isp_white_light_switch(0);
            }
			break;
	}
	return 0;
}

int HI_SDK_ISP_set_WDR_enable(uint8_t bEnable)
{
	return 0;//For test,del wdr mode
	
	printf("\n\n\n%s enable=%d\n",__FUNCTION__,bEnable);
 	if(_isp_attr.sensor_type ==  SENSOR_MODEL_APTINA_AR0237)
	{	
		sensor_gpio_reset();	
		HI_SDK_ISP_set_wdr_mode_ini(bEnable);	
	}else{
		//SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));
		//pstDRC.bEnable = bEnable;
		//_isp_attr.isp_auto_drc_enabled = bEnable;			
		//SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&pstDRC));
	}
	  return 0;
}

int HI_SDK_ISP_set_WDR_strength(uint8_t val)
{
	return 0;
	printf("%s:%d\r\n", __FUNCTION__, val);
	/////////FIXme
	ISP_DRC_ATTR_S pstDRC;
	
	uint32_t drc_value;
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));

	if(val >5){
		val = 5;
	}
	if(val < 1){
		val = 1;
	}
	_isp_attr.wdr.strength = val;
	
	drc_value = isp_get_isp_strength_value(&_isp_attr.wdr);
	
//	pstDRC.bEnable = _isp_attr.isp_auto_drc_enabled;
	pstDRC.bEnable = HI_FALSE;//HI_TRUE
	pstDRC.enOpType = OP_TYPE_AUTO;//OP_TYPE_MANUAL;
//	pstDRC.stManual.u8Strength = drc_value;
	SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&pstDRC));

	return 0;
}



int HI_SDK_ISP_get_denoise_strength(uint8_t *val)
{		
//	VPSS_GRP_PARAM_S vpss_grp_param;
//	SOC_CHECK(HI_MPI_VPSS_GetGrpParam(0, &vpss_grp_param));
//	*val = vpss_grp_param.s32GlobalStrength;
	*val = _isp_attr.YPKStrength.strength;
	return 0;
}



int HI_SDK_ISP_set_denoise_strength(uint8_t val)
{

	return 0;
}

int HI_SDK_ISP_set_advance_anti_fog_enable(uint8_t bEnable)
{
	ISP_DEFOG_ATTR_S pstDefogAttr;
	SOC_CHECK(HI_MPI_ISP_GetDeFogAttr(0,&pstDefogAttr));
	pstDefogAttr.bEnable = bEnable;
	SOC_CHECK(HI_MPI_ISP_SetDeFogAttr(0,&pstDefogAttr));
	return 0;
}


int HI_SDK_ISP_set_advance_gamma_table(uint8_t val)
{

	return 0;
}

int HI_SDK_ISP_set_advance_defect_pixel_enable(uint8_t bEnable)
{

	return 0;
}

int HI_SDK_ISP_get_color_max_value(stSensorColorMaxValue *ret_value)
{
	memcpy(ret_value, &_isp_attr.color_max_value, sizeof(stSensorColorMaxValue));
	return 0;
}


emSENSOR_MODEL HI_SDK_ISP_get_sensor_model(char *sensor_name)
{
    _isp_attr.sensor_get_name(sensor_name);
	return _isp_attr.sensor_type;
}

int HI_SDK_ISP_set_sensor_resolution(uint32_t width, uint32_t height)
{
	if(NULL == width || NULL == height){
		//set default resolution
	switch(_isp_attr.sensor_type){
		default:
		
		case SENSOR_MODEL_APTINA_AR0237:
			_isp_attr.sensor_resolution_width = 1920;
			_isp_attr.sensor_resolution_height = 1080;
			break;
		case SENSOR_MODEL_SC2235:
			_isp_attr.sensor_resolution_width = 1920;
			_isp_attr.sensor_resolution_height = 1080;
			break;
		}
	}
	else{
		_isp_attr.sensor_resolution_width = width;
		_isp_attr.sensor_resolution_height = height;
	}
	return 0;
}

int HI_SDK_ISP_get_sensor_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
	*ret_width = _isp_attr.sensor_resolution_width;
	*ret_height = _isp_attr.sensor_resolution_height;
	return 0;
}

static bool IS_FILE_EXIST(const char *filePath)
{
	return (-1 != access(filePath, F_OK));
}


int HI_SDK_ISP_get_sensor_defect_pixel_table(void )
{

	return -1;
}

int HI_SDK_ISP_set_sensor_defect_pixel_table(void )
{ 
	return -1;
}

int HI_SDK_ISP_get_daynight_mode()
{ 
	return _isp_attr.gpio_status_old;
}
	 

static void do_isp_common(uint32_t iso)
{
	int m_iso = iso / 1024;
	static uint8_t framerate_status = false;
	uint8_t isp_auto_drc_status;
	uint8_t AntiFlicker_status;
	
	//ISP_EXPOSURE_ATTR_S stExpAttr;	
	//VPSS_NR_PARAM_U unNrParam;
	//ISP_DRC_ATTR_S stDRCAttr;
	
	//SOC_CHECK(HI_MPI_ISP_GetExposureAttr(0,&stExpAttr));
   // SOC_CHECK(HI_MPI_VPSS_GetNRParam(0, &unNrParam));
	//SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&stDRCAttr));


#if 0

	switch(_isp_attr.sensor_type){
		default:
		
		case SENSOR_MODEL_APTINA_AR0237:
		{
			static HI_S32	 s32YPKStr[8]={15,15,15,15,15,15,15,15};
			static HI_S32	 s32YSFStr[8]={113,113,120,120,120,135,171,171};
			static HI_S32	 s32YTFStr[8]={70,80,85,91,92,95,110,110};
			static HI_S32	 s32TFStrMax[8]={13,13,13,13,13,13,13,13,13};
			static HI_S32	 s32YSmthRat[8]={20,20,20,20,28,28,28,28};
			static HI_S32	 s32YSFBriRat[8]={ 40,40,40,40,40,40,50,50};
			static HI_S32	 s32CSFStr[8]={44,44,44,44,44,44,50,50};

			if(m_iso<2)
			{
				
			}
			else if(m_iso<4)
			{
				

			}
			else if(m_iso<8)
			{
				

			}
			else if(m_iso<16)
			{
				

			}
			else if(m_iso<32)
			{
				

			}
			else if(m_iso<64)
			{
				

			}
			else if(m_iso<128)
			{
				

			}
			else//m_iso>128
			{
				

			}

		}
		break;
		
		}
#endif	

		HI_ISP_cfg_set_imp_single(_isp_attr.gpio_status_old, _isp_attr.ispCfgAttr);
		
		
}


static void do_isp_sensor(uint32_t iso){

	ISP_DEFOG_ATTR_S DefogAttr;
	ISP_GAMMA_ATTR_S GammaAttr;
	ISP_SHADING_ATTR_S ShadingAttr;

	HI_U8 au8DefogLut[256] = {
		109,110,111,112,113,114,115,117,118,119,120,121,122,123,125,126,127,128,129,
		130,132,133,134,135,136,137,139,140,141,142,143,144,146,147,148,149,150,151,
		152,154,155,156,157,158,159,160,162,163,164,165,166,167,168,169,171,172,173,
		174,175,176,177,178,179,180,182,183,184,185,186,187,188,189,190,191,193,194,
		195,196,197,198,199,201,202,203,204,205,206,207,209,210,211,212,213,214,215,
		216,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,234,
		235,236,237,238,239,239,240,241,242,242,243,244,244,245,246,246,247,247,248,
		248,249,249,249,250,250,250,251,251,251,252,252,252,252,253,253,253,253,253,
		253,253,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
		254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
		254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
		254,254,254,254,254,254,254,254,255};

	switch(_isp_attr.sensor_type){
		default:
		
		case SENSOR_MODEL_APTINA_AR0237:
			 break;
		case SENSOR_MODEL_SC2235:
			 break;
		case SENSOR_MODEL_IMX307:
			HI_MPI_ISP_GetDeFogAttr(0, &DefogAttr);
			if(iso < 32*1024){
				DefogAttr.bEnable = HI_TRUE;
				DefogAttr.enOpType = OP_TYPE_MANUAL;
				DefogAttr.stManual.u8strength = 50;
				DefogAttr.bUserLutEnable = HI_FALSE;
			}else{
				DefogAttr.bEnable = HI_TRUE;
				DefogAttr.enOpType = OP_TYPE_MANUAL;
				DefogAttr.stManual.u8strength = 50;
				DefogAttr.bUserLutEnable = HI_TRUE;
				memcpy(DefogAttr.au8DefogLut, au8DefogLut, sizeof(DefogAttr.au8DefogLut));
			}
			//HI_MPI_ISP_SetDeFogAttr(0, &DefogAttr);
			break;
		case SENSOR_MODEL_SC2232:
			HI_MPI_ISP_GetMeshShadingAttr(0, &ShadingAttr);
			if(_isp_attr.gpio_status_old == ISP_GPIO_NIGHT){
				HI_MPI_ISP_GetDeFogAttr(0, &DefogAttr);
				HI_MPI_ISP_GetGammaAttr(0, &GammaAttr);
				memset(DefogAttr.au8DefogLut, 255, sizeof(DefogAttr.au8DefogLut));
				if(iso < 12*1024){
					ShadingAttr.u16MeshStrength = 1500;
					DefogAttr.bEnable = HI_TRUE;
					DefogAttr.enOpType = OP_TYPE_MANUAL;
					DefogAttr.stManual.u8strength = 100;
					DefogAttr.bUserLutEnable = HI_FALSE;
					memcpy(GammaAttr.u16Table, gs_Gamma[10], sizeof(gs_Gamma[10]));
				}else if(iso < 16*1024){
					ShadingAttr.u16MeshStrength = 1000;
					DefogAttr.bEnable = HI_TRUE;
					DefogAttr.enOpType = OP_TYPE_MANUAL;
					DefogAttr.stManual.u8strength = 100;
					DefogAttr.bUserLutEnable = HI_FALSE;
					memcpy(GammaAttr.u16Table, gs_Gamma[13], sizeof(gs_Gamma[13]));
				}else if(iso < 29*1024){
					ShadingAttr.u16MeshStrength = 700;
					DefogAttr.bEnable = HI_TRUE;
					DefogAttr.enOpType = OP_TYPE_MANUAL;
					DefogAttr.stManual.u8strength = 150;
					DefogAttr.bUserLutEnable = HI_FALSE;
					memcpy(GammaAttr.u16Table, gs_Gamma[13], sizeof(gs_Gamma[13]));
				}else{
					ShadingAttr.u16MeshStrength = 700;
					DefogAttr.bEnable = HI_TRUE;
					DefogAttr.enOpType = OP_TYPE_MANUAL;
					DefogAttr.stManual.u8strength = 170;
					DefogAttr.bUserLutEnable = HI_FALSE;
					memcpy(GammaAttr.u16Table, gs_Gamma[12], sizeof(gs_Gamma[12]));
				}
				//HI_MPI_ISP_SetDeFogAttr(0, &DefogAttr);
				//HI_MPI_ISP_SetGammaAttr(0, &GammaAttr);
				//HI_MPI_ISP_SetMeshShadingAttr(0, &ShadingAttr); //FIXME:
			}
			break;
		}
}



uint32_t HI_SDK_ISP_get_gain(void)
{
	uint32_t ret_gain;
	ret_gain = gains_calculate();

	//all the sensor
	do_isp_common(ret_gain);
	
	//the own isp setting for each sensor
	do_isp_sensor(ret_gain);
	return ret_gain;
}

void HI_SDK_ISP_set_AF_attr(stSensorAfAttr *pAfAttr)
{
	if(_isp_attr.AfAttr.param){
		free(_isp_attr.AfAttr.param);
		_isp_attr.AfAttr.param = NULL;
	}
	_isp_attr.AfAttr.param = pAfAttr->param;
	_isp_attr.AfAttr.af_callback = pAfAttr->af_callback;
	
}

static int _hi_sdk_isp_set_slow_framerate(uint8_t bValue)
{	
	if(_isp_attr.sensor_type == SENSOR_MODEL_APTINA_AR0237){
		uint8_t Value = bValue ? 0x1:0x0;
		VENC_CHN_ATTR_S vencChannelAttr;
		ISP_PUB_ATTR_S stPubAttr;	
		int actual_fps;
		int ret = 0;
		int i = 0;
		
		_isp_attr.isp_framerate_status = Value;
		SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));
		
		for(i = 0; i <= 1; i++){
			ret = HI_MPI_VENC_GetChnAttr(i,&vencChannelAttr);
				if(ret != 0){
					return -1;
				}
				printf("old_FRate = %f\n",stPubAttr.f32FrameRate);
				actual_fps = _isp_attr.src_framerate/(1 << Value);
				if(vencChannelAttr.stVeAttr.enType == PT_H264)
				{
					switch(vencChannelAttr.stRcAttr.enRcMode){
						default:
						case VENC_RC_MODE_H264AVBR:
							if(bValue == true){//slow framerate 		
								stPubAttr.f32FrameRate = actual_fps;
								if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264AVbr.fr32DstFrmRate){
									vencChannelAttr.stRcAttr.stAttrH264AVbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264AVbr.fr32DstFrmRate; 
								}else{
									vencChannelAttr.stRcAttr.stAttrH264AVbr.u32SrcFrmRate = actual_fps;
								}	
				
							}else{//actual framerate		
								stPubAttr.f32FrameRate = actual_fps;
								if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264AVbr.fr32DstFrmRate){
									vencChannelAttr.stRcAttr.stAttrH264AVbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264AVbr.fr32DstFrmRate; 
								}else{
									vencChannelAttr.stRcAttr.stAttrH264AVbr.u32SrcFrmRate = actual_fps;
								}
							}
							break;
						
						case VENC_RC_MODE_H264CBR:
							if(bValue == true){//slow framerate
								stPubAttr.f32FrameRate = actual_fps;
								if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
									vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
								}else{
									vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =	actual_fps;
								}		
								
							}else{//actual framerate	
								stPubAttr.f32FrameRate = actual_fps;
								if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
									vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
								}else{
									vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =	actual_fps;
								}
							}
							break;		
						}
				}
				else if(vencChannelAttr.stVeAttr.enType ==	PT_H265)
				{
					switch(vencChannelAttr.stRcAttr.enRcMode){
					default:
					case VENC_RC_MODE_H265AVBR:
						if(bValue == true){//slow framerate 		
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate = actual_fps;
							}	
				
						}else{//actual framerate		
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate = actual_fps;
							}
						}
						break;
					
					case VENC_RC_MODE_H265CBR:
						if(bValue == true){//slow framerate
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate =	actual_fps;
							}		
							
						}else{//actual framerate	
							stPubAttr.f32FrameRate = actual_fps;
							if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate){
								vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate; 
							}else{
								vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate =	actual_fps;
							}
						}
						break;		
					}
		
				}
				else
				{}
					
				HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
				SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));
		
		}
		
		printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
	}

	
	return 0;

}

static int _hi_sdk_isp_set_slow_framerate_new(uint8_t bValue)
{

	switch(bValue){
		case SLOW_FRAMERATE_DISABLE:
			_isp_attr.isp_framerate_status = false;
			_isp_attr.starlight_mode_enable = false;
		break;
		case SLOW_FRAMERATE_LOWLIGHT:
			_isp_attr.isp_framerate_status = true;
			_isp_attr.starlight_mode_enable = true;
		break;
		case SLOW_FRAMERATE_STARLIGHT:
			_isp_attr.isp_framerate_status = false;
			_isp_attr.starlight_mode_enable = true;
		break;
		default:
			break;
	}
	_hi_sdk_isp_set_slow_framerate_mode();
	return 0;
}

static int _hi_sdk_isp_init_isp_default_value(void)
{	
	
	 ISP_GAMMA_ATTR_S GammaAttr;
	 ISP_EXPOSURE_ATTR_S stExpAttr;

	 SOC_CHECK(HI_MPI_ISP_GetGammaAttr(0, &GammaAttr));
	 SOC_CHECK(HI_MPI_ISP_GetExposureAttr(0,&stExpAttr));
	switch(_isp_attr.sensor_type){
		default:
		
		case SENSOR_MODEL_APTINA_AR0237:
		{
			_isp_attr.aeCompensition.strength = 3;
			_isp_attr.aeCompensition.daylight_val = 0x38;
			_isp_attr.aeCompensition.night_val = 0x38;
			
			_isp_attr.YPKStrength.strength = 3;
			_isp_attr.YPKStrength.daylight_val = 0;
			_isp_attr.YPKStrength.night_val = 0;

			_isp_attr.YSFStrength.strength = 3; 			
			_isp_attr.YSFStrength.daylight_val = 100;			
			_isp_attr.YSFStrength.night_val = 100;
			
			_isp_attr.YTFStrength.strength = 3;
			_isp_attr.YTFStrength.daylight_val = 64;
			_isp_attr.YTFStrength.night_val = 64;
			
			_isp_attr.wdr.strength = 3;
			_isp_attr.wdr.daylight_val = 0x35;
			_isp_attr.wdr.night_val = 0x35;

			GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
			memcpy(GammaAttr.u16Table, gs_Gamma[8], sizeof(gs_Gamma[8]));

		}
					
		break;
		case SENSOR_MODEL_SC2232:
		case SENSOR_MODEL_SC2235:
		{

			_isp_attr.aeCompensition.strength = 3;
			_isp_attr.aeCompensition.daylight_val = 70;
			_isp_attr.aeCompensition.night_val = 60;
			
			_isp_attr.YPKStrength.strength = 3;
			_isp_attr.YPKStrength.daylight_val = 0;
			_isp_attr.YPKStrength.night_val = 0;

			_isp_attr.YSFStrength.strength = 3; 			
			_isp_attr.YSFStrength.daylight_val = 100;			
			_isp_attr.YSFStrength.night_val = 100;
			
			_isp_attr.YTFStrength.strength = 3;
			_isp_attr.YTFStrength.daylight_val = 64;
			_isp_attr.YTFStrength.night_val = 64;
			
			_isp_attr.wdr.strength = 3;
			_isp_attr.wdr.daylight_val = 0x35;
			_isp_attr.wdr.night_val = 0x35;

			GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
			memcpy(GammaAttr.u16Table, gs_Gamma[6], sizeof(gs_Gamma[6]));		
	/**/
		}
		break;
		
	}

//	SOC_CHECK(HI_MPI_ISP_SetGammaAttr(0, &GammaAttr));


//////////////////////////////////////////////////////////////////////////
//	HI_SDK_ISP_get_sensor_defect_pixel_table();

	return 0;
}

int HI_SDK_ISP_set_isp_sensor_value(void)////mode  0:daytime   1:night
{
    int i;
	VPSS_GRP VpssGrp = 0;
	ISP_EXPOSURE_ATTR_S ExpAttr;

	ISP_DP_DYNAMIC_ATTR_S DPCAttr;
    ISP_WDR_MODE_S stWdrMode;
    ISP_CA_ATTR_S CAAttr;
    ISP_DEFOG_ATTR_S DefogAttr;

    ISP_DRC_ATTR_S DRCAttr;
	VPSS_GRP_ATTR_S pstVpssGrpAttr;
	VPSS_GRP_SHARPEN_ATTR_S pstGrpSharpenAttr;
	ISP_WB_ATTR_S stWBAttr;

	VI_DCI_PARAM_S stDciParam;

	ISP_SHARPEN_ATTR_S sharpenAttr;
	ISP_SHADING_ATTR_S ShadingAttr;

    HI_U16 RatioLut[128] = {
                        36,50,65,79,94,108,123,137,151,166,180,195,209,223,237,251,
                        265,279,293,306,320,334,348,361,375,388,402,415,428,441,454,467,
                        479,491,503,515,526,538,549,560,571,582,593,604,615,627,638,650,
                        662,675,688,702,716,731,746,761,775,790,804,817,829,840,850,859,
                        866,872,878,883,888,892,895,898,899,900,899,897,894,889,883,875,
                        866,854,839,822,802,780,758,734,709,685,660,636,613,592,572,555,
                        540,527,516,506,498,490,484,478,472,468,463,459,455,451,447,443,
                        438,433,428,423,419,415,411,407,403,399,396,392,389,385,381,377
                        };
    HI_S32 ISORatio[16] = {1300,1300,1250,1200,1150,1100,1050,1000,950,900,900,800,800,800,800,800};
	HI_U8 skinGain[16] = {190, 190, 160, 128, 128, 128, 128, 128, 0, 0, 0, 0, 0, 0, 0, 0};//sc2232
	HI_U8 RGain[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};//sc2232
	HI_U8 BGain[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};//sc2232
	HI_U8 defoglut[256] = {
		109,110,111,112,113,114,115,117,118,119,120,121,122,123,125,126,127,128,129,130,132,133,134,135,
		136,137,139,140,141,142,143,144,146,147,148,149,150,151,152,154,155,156,157,158,159,160,162,163,
		164,165,166,167,168,169,171,172,173,174,175,176,177,178,179,180,182,183,184,185,186,187,188,189,
		190,191,193,194,195,196,197,198,199,201,202,203,204,205,206,207,209,210,211,212,213,214,215,216,
		218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,234,235,236,237,238,239,239,
		240,241,242,242,243,244,244,245,246,246,247,247,248,248,249,249,249,250,250,250,251,251,251,252,
		252,252,252,253,253,253,253,253,253,253,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
		254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,254,254,254,254,254,254,
		254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,255};//sc2232
	HI_U8 sharpen_lumawgt[ISP_YUV_SHPLUMA_NUM] = {64,64,64,64,64,80,88,96,96,176,240,255,255,255,255,255,255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
	HI_U8 vpsssharpen_lumawgt[ISP_YUV_SHPLUMA_NUM] = {8, 24, 39, 60, 81, 93, 105, 114, 120, 132, 146, 154, 173,
		187, 195, 222, 246, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
	HI_U8 vpsssharpenud[2][16] = {{0, 0, 5, 5, 3, 3, 3, 3, 3, 3, 16, 16, 16, 16, 16, 16},
								{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 16, 16, 16, 16, 16, 16}};//sc2232
	HI_U8 vpsssharpend[2][16] = {{0, 0, 55, 55, 45, 45, 45, 40, 35, 30, 44, 44, 44, 44, 64, 64},
								{45, 45, 45, 45, 45, 45, 45, 45, 45, 44, 44, 44, 44, 44, 64, 64}};//sc2232
	HI_U8 vpsstexture[2][16] = {{0, 0, 9, 9, 9, 9, 9, 9, 9, 32, 32, 32, 32, 32, 32, 32},
								{2, 2, 2, 0, 0, 0, 0, 0, 0, 32, 32, 32, 32, 32, 32, 32}};//sc2232
	HI_U8 vpsssharpenedge[2][16] = {{0, 0, 7, 7, 5, 5, 5, 5, 5, 5, 224, 224, 224, 224, 224, 224},
								{6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 60, 60, 60, 60, 60, 224}};//sc2232
	HI_U8 vpssedgethd[2][16] = {{0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 60, 60, 60, 60, 100, 100},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 224, 224, 224, 224, 224, 224}};//sc2232
	HI_U8 vpssovershoot[2][16] = {{0, 0, 47, 47, 47, 19, 19, 16, 19, 70, 68, 68, 68, 68, 68, 68},
								{40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 68, 68, 68, 68, 68, 68}};//sc2232
	HI_U8 vpssundershoot[2][16] = {{0, 0, 56, 56, 56, 109, 110, 100, 90, 90, 90, 90, 90, 90, 90, 90},
								{32, 32, 32, 32, 32, 40, 40, 40, 40, 40, 32, 88, 88, 88, 88, 88, 88}};//sc2232
	HI_U8 vpssshootsupstr[2][16] = {{0, 0, 34, 34, 0, 0, 0, 0, 10, 90, 90, 90, 90, 90, 90, 90},
								{19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 0, 0, 0, 0, 0, 0}};//sc2232
	HI_U8 vpssdetailctrl[2][16] = {{0, 0, 105, 105, 95, 76, 68, 58, 40, 30, 128, 128, 128, 128, 128, 128},
								{105, 105, 105, 105, 95, 76, 68, 58, 40, 30, 128, 128, 128, 128, 128, 128}};//sc2232
	HI_U32 shadingGain[289] = {
		2629, 2219, 1887, 1680, 1510, 1407, 1329, 1285, 1267, 1288, 1328, 1406, 1521, 1680, 1921, 2236, 
		2771, 2501, 2128, 1821, 1618, 1457, 1355, 1279, 1236, 1223, 1236, 1279, 1356, 1461, 1620, 1837, 
		2154, 2586, 2397, 2026, 1747, 1549, 1400, 1297, 1225, 1183, 1164, 1181, 1224, 1301, 1405, 1554, 
		1767, 2053, 2459, 2297, 1951, 1682, 1498, 1348, 1249, 1177, 1134, 1117, 1133, 1174, 1245, 1347, 
		1495, 1690, 1971, 2343, 2230, 1893, 1635, 1454, 1312, 1214, 1141, 1094, 1083, 1095, 1136, 1208, 
		1307, 1449, 1643, 1911, 2267, 2183, 1856, 1608, 1425, 1286, 1185, 1111, 1063, 1046, 1061, 1109, 
		1182, 1282, 1419, 1605, 1865, 2196, 2122, 1825, 1579, 1398, 1269, 1170, 1090, 1045, 1028, 1044, 
		1089, 1160, 1262, 1397, 1583, 1837, 2169, 2128, 1810, 1572, 1391, 1259, 1158, 1080, 1035, 1029, 
		1032, 1076, 1148, 1249, 1385, 1562, 1820, 2150, 2139, 1801, 1567, 1391, 1260, 1153, 1076, 1037, 
		1026, 1024, 1072, 1148, 1248, 1382, 1562, 1813, 2141, 2127, 1809, 1587, 1400, 1267, 1164, 1085, 
		1041, 1025, 1032, 1080, 1154, 1254, 1388, 1572, 1823, 2161, 2165, 1841, 1600, 1414, 1286, 1178, 
		1102, 1053, 1036, 1051, 1098, 1166, 1266, 1411, 1591, 1848, 2185, 2205, 1869, 1635, 1448, 1306, 
		1203, 1129, 1079, 1065, 1074, 1119, 1190, 1296, 1435, 1624, 1880, 2232, 2256, 1921, 1676, 1483, 
		1341, 1242, 1163, 1117, 1101, 1110, 1156, 1227, 1329, 1472, 1662, 1937, 2299, 2342, 1999, 1744, 
		1528, 1390, 1282, 1206, 1163, 1142, 1162, 1199, 1272, 1376, 1522, 1723, 2015, 2391, 2469, 2090, 
		1810, 1594, 1447, 1339, 1261, 1217, 1194, 1211, 1254, 1328, 1435, 1589, 1797, 2108, 2513, 2599, 
		2197, 1912, 1674, 1523, 1398, 1325, 1281, 1260, 1278, 1317, 1394, 1505, 1664, 1893, 2214, 2684, 
		2762, 2305, 1998, 1749, 1593, 1469, 1389, 1339, 1313, 1336, 1384, 1454, 1572, 1744, 1976, 2336, 
		2895};//sc2232

    SOC_CHECK(HI_MPI_ISP_GetDPDynamicAttr(0, &DPCAttr));
	SOC_CHECK(HI_MPI_ISP_GetExposureAttr(0,&ExpAttr));
    //get CA
    SOC_CHECK(HI_MPI_ISP_GetCAAttr(0, &CAAttr));
    //get DeFog
    SOC_CHECK(HI_MPI_ISP_GetDeFogAttr(0, &DefogAttr));

    SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&DRCAttr));
	SOC_CHECK(HI_MPI_VPSS_GetGrpAttr(VpssGrp, &pstVpssGrpAttr));
	SOC_CHECK(HI_MPI_VPSS_GetGrpSharpen(VpssGrp, &pstGrpSharpenAttr));

	SOC_CHECK(HI_MPI_VI_GetDCIParam(0, &stDciParam));

	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(0, &sharpenAttr));
	
	SOC_CHECK(HI_MPI_ISP_GetWBAttr(0,&stWBAttr));
	if(_isp_attr.starlight_mode_enable){
		ExpAttr.stAuto.enAEMode = AE_MODE_SLOW_SHUTTER;		
	}else{
		ExpAttr.stAuto.enAEMode = AE_MODE_FIX_FRAME_RATE;	
	}
	switch(_isp_attr.sensor_type){
		case SENSOR_MODEL_APTINA_AR0237:
			{
				DPCAttr.bSupTwinkleEn = HI_TRUE;

                SOC_CHECK(HI_MPI_ISP_GetWDRMode(HI3518A_VIN_DEV, &stWdrMode));
                if (stWdrMode.enWDRMode)  //wdr mode
                {
                    //set CA data
                    CAAttr.bEnable = HI_TRUE;
                    for (i = 0 ; i < 128; i++) {
                       CAAttr.au16YRatioLut[i] = RatioLut[i];
                    }
                    for (i = 0 ; i < 16; i++) {
                        CAAttr.as32ISORatio[i] = ISORatio[i];
                    }
					
				    //set DeFog data
					DefogAttr.bEnable = HI_TRUE;
					DefogAttr.enOpType = OP_TYPE_MANUAL;
					DefogAttr.stManual.u8strength = 60;
					
                }
            }
		break;

		case SENSOR_MODEL_SC2235:
		{
		stWBAttr.stAuto.u16Speed = 512;
		//set DeFog data
		pstVpssGrpAttr.bSharpenEn = HI_TRUE;
		pstGrpSharpenAttr.enOpType = SHARPEN_OP_TYPE_MANUAL;
		pstGrpSharpenAttr.stSharpenManualAttr.u8SharpenUd	= 0;
		pstGrpSharpenAttr.stSharpenManualAttr.u8SharpenD	= 38;
		pstGrpSharpenAttr.stSharpenManualAttr.u8TextureThr	= 0;
		pstGrpSharpenAttr.stSharpenManualAttr.u8SharpenEdge = 0;
		pstGrpSharpenAttr.stSharpenManualAttr.u8EdgeThd 	= 0;
		pstGrpSharpenAttr.stSharpenManualAttr.u8OverShoot	= 30;
		pstGrpSharpenAttr.stSharpenManualAttr.u8UnderShoot	= 40;
		pstGrpSharpenAttr.stSharpenManualAttr.u8ShootSupStr = 0;
		pstGrpSharpenAttr.stSharpenManualAttr.u8DetailCtrl	= 0;

		DefogAttr.bEnable = HI_TRUE;
		DefogAttr.enOpType = OP_TYPE_MANUAL;
		DefogAttr.stManual.u8strength = 50;

		DRCAttr.u8SecondPole = 150;
		DRCAttr.u8Stretch = 38;

		stDciParam.bEnable = true;
		stDciParam.u32BlackGain = 0;
		stDciParam.u32ContrastGain = 30;
		stDciParam.u32LightGain = 0;

		DPCAttr.bSupTwinkleEn = HI_TRUE;

                SOC_CHECK(HI_MPI_ISP_GetWDRMode(HI3518A_VIN_DEV, &stWdrMode));
                if (stWdrMode.enWDRMode)  //wdr mode
                {
                    //set CA data
                    CAAttr.bEnable = HI_TRUE;
                    for (i = 0 ; i < 128; i++) {
                       CAAttr.au16YRatioLut[i] = RatioLut[i];
                    }
                    for (i = 0 ; i < 16; i++) {
                        CAAttr.as32ISORatio[i] = ISORatio[i];
                    }
					
				    //set DeFog data
					DefogAttr.bEnable = HI_TRUE;
					DefogAttr.enOpType = OP_TYPE_MANUAL;
					DefogAttr.stManual.u8strength = 60;
					
                }
            }
		break;
		case SENSOR_MODEL_SC2232:
			HI_MPI_ISP_GetMeshShadingAttr(0, &ShadingAttr);
			DefogAttr.bEnable = HI_TRUE;
			DefogAttr.enOpType = OP_TYPE_MANUAL;
			memcpy(sharpenAttr.stAuto.au8SkinGain, skinGain, sizeof(skinGain));
			memcpy(sharpenAttr.stAuto.au8RGain, RGain, sizeof(RGain));
			memcpy(sharpenAttr.stAuto.au8BGain, BGain, sizeof(BGain));
#if 0
			memcpy(ShadingAttr.au32RGain, shadingGain, sizeof(ShadingAttr.au32RGain));
			memcpy(ShadingAttr.au32GbGain, shadingGain, sizeof(ShadingAttr.au32GbGain));
			memcpy(ShadingAttr.au32GrGain, shadingGain, sizeof(ShadingAttr.au32GrGain));
			memcpy(ShadingAttr.au32BGain, shadingGain, sizeof(ShadingAttr.au32BGain));
#endif
			if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
				pstVpssGrpAttr.bSharpenEn = HI_TRUE;
				pstGrpSharpenAttr.enOpType = SHARPEN_OP_TYPE_AUTO;
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenUd, vpsssharpenud[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenUd));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenD, vpsssharpend[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenD));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8TextureThr, vpsstexture[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8TextureThr));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenEdge, vpsssharpenedge[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenEdge));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8EdgeThd, vpssedgethd[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8EdgeThd));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8OverShoot, vpssovershoot[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8OverShoot));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8UnderShoot, vpssundershoot[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8UnderShoot));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8ShootSupStr, vpssshootsupstr[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8ShootSupStr));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8DetailCtrl, vpssdetailctrl[0], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8DetailCtrl));
				memcpy(sharpenAttr.u8LumaWgt, sharpen_lumawgt, sizeof(sharpenAttr.u8LumaWgt));

				DefogAttr.stManual.u8strength = 120;
				DefogAttr.bUserLutEnable = HI_FALSE;

				DRCAttr.u8Asymmetry  = 2;
				DRCAttr.u8SecondPole = 197;
				DRCAttr.u8Stretch    = 60;
				DRCAttr.u8Compress   = 100;

//				ShadingAttr.enOpType = OP_TYPE_MANUAL;
//				ShadingAttr.bEnable = HI_FALSE;
			}else{
				pstVpssGrpAttr.bSharpenEn = HI_TRUE;
				pstGrpSharpenAttr.enOpType = SHARPEN_OP_TYPE_AUTO;
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenUd, vpsssharpenud[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenUd));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenD, vpsssharpend[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenD));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8TextureThr, vpsstexture[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8TextureThr));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenEdge, vpsssharpenedge[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenEdge));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8EdgeThd, vpssedgethd[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8EdgeThd));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8OverShoot, vpssovershoot[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8OverShoot));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8UnderShoot, vpssundershoot[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8UnderShoot));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8ShootSupStr, vpssshootsupstr[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8ShootSupStr));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8DetailCtrl, vpssdetailctrl[1], sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8DetailCtrl));
				memcpy(sharpenAttr.u8LumaWgt, sharpen_lumawgt, sizeof(sharpenAttr.u8LumaWgt));

				DefogAttr.stManual.u8strength = 90;
				DefogAttr.bUserLutEnable = HI_FALSE;

				DRCAttr.u8Asymmetry  = 2;
				DRCAttr.u8SecondPole = 150;
				DRCAttr.u8Stretch    = 60;
				DRCAttr.u8Compress   = 100;

//				ShadingAttr.enOpType = OP_TYPE_MANUAL;
//				ShadingAttr.bEnable = HI_TRUE;
			}
//			HI_MPI_ISP_SetMeshShadingAttr(0, &ShadingAttr);
			DefogAttr.bUserLutEnable = HI_TRUE;
			memset(DefogAttr.au8DefogLut, 255, sizeof(DefogAttr.au8DefogLut));
			break;
			
		case SENSOR_MODEL_IMX307:
			{
				HI_U8 skinGain_imx307[16] = {127, 127, 50, 20, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};//imx307
				HI_U8 RGain_imx307[16] = {30, 20, 20, 20, 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};//imx307
				HI_U8 BGain_imx307[16] = {30, 20, 20, 20, 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};//imx307
				
				HI_U8 au8SharpenUd[16] = {0, 0, 0, 0, 0, 5, 12, 12, 12, 12, 0, 0, 0, 0, 0, 0};
				HI_U8 au8SharpenD[16] = {0, 0, 0, 0, 0, 60, 60, 60, 60, 60, 0, 0, 0, 0, 0, 0};
				HI_U8 au8TextureThr[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				HI_U8 au8SharpenEdge[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				HI_U8 au8EdgeThd[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				HI_U8 au8OverShoot[16] = {0, 0, 0, 0, 0, 50, 60, 60, 60, 60, 0, 0, 0, 0, 0, 0};
				HI_U8 au8UnderShoot[16] = {0, 0, 0, 0, 0, 60, 60, 60, 60, 60, 0, 0, 0, 0, 0, 0};
				HI_U8 au8ShootSupStr[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				HI_U8 au8DetailCtrl[16] = {128, 128, 128, 128, 128, 65, 65, 65, 65, 65, 0, 0, 0, 0, 0, 0};
		
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
					ExpAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame = 2;
					ExpAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame = 0;
				}else{
					ExpAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame = 0;
					ExpAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame = 5;
				}
				
				DefogAttr.bEnable = HI_TRUE;
				DefogAttr.enOpType = OP_TYPE_MANUAL;
				DefogAttr.stManual.u8strength = 50;
				DefogAttr.bUserLutEnable = HI_TRUE;

				memcpy(sharpenAttr.stAuto.au8SkinGain, skinGain_imx307, sizeof(skinGain_imx307));
				memcpy(sharpenAttr.stAuto.au8RGain, RGain_imx307, sizeof(RGain_imx307));
				memcpy(sharpenAttr.stAuto.au8BGain, BGain_imx307, sizeof(BGain_imx307));
		

				ExpAttr.stAuto.enAEMode = AE_MODE_SLOW_SHUTTER;
				
				//set vpss sharpen
				pstVpssGrpAttr.bSharpenEn = HI_TRUE;
				pstGrpSharpenAttr.enOpType = SHARPEN_OP_TYPE_AUTO;
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenUd, au8SharpenUd, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenUd));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenD, au8SharpenD, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenD));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8TextureThr, au8TextureThr, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8TextureThr));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenEdge, au8SharpenEdge, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8SharpenEdge));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8EdgeThd, au8EdgeThd, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8EdgeThd));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8OverShoot, au8OverShoot, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8OverShoot));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8UnderShoot, au8UnderShoot, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8UnderShoot));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8ShootSupStr, au8ShootSupStr, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8ShootSupStr));
				memcpy(pstGrpSharpenAttr.stSharpenAutoAttr.au8DetailCtrl, au8DetailCtrl, sizeof(pstGrpSharpenAttr.stSharpenAutoAttr.au8DetailCtrl));	

			}		
			break;
		default:
			break;
	}

	SOC_CHECK(HI_MPI_ISP_SetExposureAttr(0,&ExpAttr));

	SOC_CHECK(HI_MPI_ISP_SetDPDynamicAttr(0, &DPCAttr));

    //set CA
    SOC_CHECK(HI_MPI_ISP_SetCAAttr(0, &CAAttr));

	//set DeFog
	SOC_CHECK(HI_MPI_ISP_SetDeFogAttr(0,  &DefogAttr));
	
	SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&DRCAttr));

	SOC_CHECK(HI_MPI_VPSS_SetGrpAttr(VpssGrp, &pstVpssGrpAttr));
	SOC_CHECK(HI_MPI_VPSS_SetGrpSharpen(VpssGrp, &pstGrpSharpenAttr));

	SOC_CHECK(HI_MPI_VI_SetDCIParam(0, &stDciParam));
	SOC_CHECK(HI_MPI_ISP_SetSharpenAttr(0, &sharpenAttr));

	SOC_CHECK(HI_MPI_ISP_SetWBAttr(0,&stWBAttr));
	return 0;
}

int HI_SDK_ISP_ini_load(const char *filepath)
{
	if(NULL == _isp_attr.ispCfgAttr){
		_isp_attr.ispCfgAttr = (LpIspCfgAttr)calloc(sizeof(StIspCfgAttr), 1);
	}
	if(-1 == HI_ISP_cfg_load(filepath, _isp_attr.ispCfgAttr)){
		if(_isp_attr.ispCfgAttr){
			free(_isp_attr.ispCfgAttr);
		}
		_isp_attr.ispCfgAttr = NULL;
		return -1;
	}else{
		_isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate = _isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO ? true : false;
		_isp_attr.ispCfgAttr->impCfgAttr.flick_frequency = _isp_attr.filter_frequency;
		_isp_attr.ispCfgAttr->aeCfgAttr.StarLightModeEnable = _isp_attr.starlight_mode_enable;
		isp_ircut_contrl_init();

        if(_isp_attr.sensor_type ==  SENSOR_MODEL_IMX307)
        {
            _isp_pwm_light_init();
        }

		return HI_ISP_cfg_set_all(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT ? 0 : 1, 1, _isp_attr.ispCfgAttr);
	}
	
}

int HI_SDK_ISP_get_starlight_mode(bool *enable)
{
	*enable = _isp_attr.starlight_mode_enable;
	return 0;
}


int HI_SDK_ISP_get_wdr_mode(uint8_t *bEnable)
{
	ISP_WDR_MODE_S stWdrMode;
	ISP_DRC_ATTR_S pstDRC;
			    
	if(_isp_attr.sensor_type ==SENSOR_MODEL_APTINA_AR0237)
	{		
		SOC_CHECK(HI_MPI_ISP_GetWDRMode(HI3518A_VIN_DEV, &stWdrMode));
		if (stWdrMode.enWDRMode)  //wdr mode
		{	
			*bEnable = 1;
		}else{
			*bEnable = 0;
		}
	}else{	
		SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));		
		*bEnable = pstDRC.bEnable;
	}
	
	return 0;

}

static int float2int(float fVal)
{
    float retVal = 0.0f;

    retVal = fVal + ((fVal >= 0.0f) ? (0.50f) : (-0.50f));

    return (int)retVal;

}

int HI_SDK_ISP_get_cur_fps()
{
    float vi_fps;
    ISP_EXP_INFO_S stExpInfo;
    ISP_PUB_ATTR_S stPubAttr;
    VENC_CHN_ATTR_S venc_ch_attr;
    int i_cur_fps;
    float f_cur_fps;
    float val;
    float dstFrmRate;
    float srcFrmRate;
    SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(0, &stExpInfo));
    SOC_CHECK(HI_MPI_VENC_GetChnAttr(0, &venc_ch_attr));
    switch(venc_ch_attr.stRcAttr.enRcMode) {
        default:
        case VENC_RC_MODE_H265AVBR:
        {
            dstFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH265AVbr.fr32DstFrmRate;
            srcFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH265AVbr.u32SrcFrmRate;
            break;
        }
        case VENC_RC_MODE_H265VBR:
        {
            dstFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate;
            srcFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate;
            break;
        }
        case VENC_RC_MODE_H265CBR:
        {
           dstFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate;
           srcFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate;
            break;
        }
        case VENC_RC_MODE_H264FIXQP:
        {
            dstFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH265FixQp.fr32DstFrmRate;
            srcFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH265FixQp.u32SrcFrmRate;
            break;
        }
    }

    /* stExpInfo.u32Fps / 100 * (dstFrmRate / srcFrmRate) */
    val = dstFrmRate / srcFrmRate;
    vi_fps = stExpInfo.u32Fps / 100;
    if(srcFrmRate <= vi_fps) {
        f_cur_fps = srcFrmRate * val;
    }
    else {
        f_cur_fps = vi_fps * val;
    }
    i_cur_fps = float2int(f_cur_fps);
    return i_cur_fps;

}

#define SET_VI_DEV_ATTR_AR0237(info) \
{\
			info.enIntfMode = VI_MODE_MIPI;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
			info.stDevRect.s32X = 0;\
            info.stDevRect.s32Y = 0;\
            info.stDevRect.u32Width  = 1920;\
            info.stDevRect.u32Height = 1080;\
}

#define SET_VI_DEV_ATTR_SC2235(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFF0000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
			info.stDevRect.s32X = 0;\
            info.stDevRect.s32Y = 0;\
            info.stDevRect.u32Width  = 1920;\
            info.stDevRect.u32Height = 1080;\
}


#define SET_VI_DEV_ATTR_IMX307(info) \
{\
			info.enIntfMode = VI_MODE_LVDS;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
			info.stDevRect.s32X = 0;\
            info.stDevRect.s32Y = 30;\
            info.stDevRect.u32Width  = 1920;\
            info.stDevRect.u32Height = 1080;\
}


static stHiIspAttr _isp_attr = {
	.api = {
		.SENSOR_MODEL_GET=hi_isp_api_get_sensor_model,
		.MIRROR_FLIP_SET=hi_isp_api_mirror_flip,
		.HUE_SET=hi_isp_api_set_hue,
		.CONTRAST_SET=hi_isp_api_set_contrast,
		.BRIGHTNESS_SET=hi_isp_api_set_brightness,
		.SATURATION_SET=hi_isp_api_set_saturation,
		.LIGHT_MODE_SET=hi_isp_api_light_mode,
		.TEST_MODE_SET=hi_isp_api_test_mode,
		.COLOR_MODE_SET=hi_isp_api_color_mode,
		.REG_READ=hi_isp_api_reg_read,
	 	.REG_WRITE=hi_isp_api_reg_write,
		.SPEC_RED_WRITE=hi_isp_api_spec_reg_write,
		.SPEC_REG_READ=hi_isp_api_spec_reg_read,
		.GET_COLOR_MAX_VALUE=hi_isp_api_get_color_max_value,
		.SHUTTER_SET=hi_isp_api_set_shutter,
		.IRCUT_AUTO_SWITCH=hi_isp_api_ircut_auto_switch,
		.VI_FLICKER=hi_isp_api_vi_flicker,
		.SHARPEN_SET=hi_isp_api_set_sharpen,
		.SHARPEN_GET=hi_isp_api_get_sharpen,
		.SCENE_MODE_SET=hi_isp_api_set_scene_mode,
		.WB_MODE_SET=hi_isp_api_set_WB_mode,
		.STARLIGHT_MODE_GET=hi_isp_api_get_starlight_mode,
		.COLORTOBLACK_RANGE_SET = hi_isp_api_set_colortoblack_range,
		.COLORTOBLACK_RANGE_GET = hi_isp_api_get_colortoblack_range,	
		.IRCUT_CONTROL_MODE_SET=hi_isp_api_set_ircut_control_mode,
		.IRCUT_MODE_SET=hi_isp_api_set_ircut_mode,
		.WDR_MODE_ENABLE=hi_isp_api_set_WDR_enable,
		.WDR_STRENGTH_SET=hi_isp_api_set_WDR_strength,
		.EXPOSURE_MODE_SET=hi_isp_api_set_exposure_mode,
		.AE_COMPENSATION_SET=hi_isp_api_set_AEcompensation,
		.DENOISE_ENABLE=hi_isp_api_set_denoise_enable,
		.DENOISE_STRENGTH_SET=hi_isp_api_set_denoise_strength,
		.DENOISE_STRENGTH_GET=hi_isp_api_get_denoise_strength,
		.ANTI_FOG_ENABLE=hi_isp_api_set_anti_fog_enable,
		.LOWLIGHT_ENABLE=hi_isp_api_set_lowlight_enable,
		.GAMMA_TABLE_SET=hi_isp_api_set_gamma_table,
		.DEFECT_PIXEL_ENABLE=hi_isp_api_set_defect_pixel_enable,
		.SRC_FRAMERATE_SET=hi_isp_api_set_src_framerate,
		.SENSOR_RESOLUTION_GET=hi_isp_api_get_sensor_resolution,
		.SENSOR_RESOLUTION_SET=hi_isp_api_set_sensor_resolution,
		.GAIN_GET=hi_isp_api_get_gain,
		.AF_CALLBACK_SET = hi_isp_api_set_af_attr,
		.INI_LOAD=hi_isp_api_ini_load,	
		.WDR_MODE_GET= hi_isp_api_get_wdr_mode,
		.COLORTOBLACK_RANGE_SET = hi_isp_api_set_colortoblack_range,
		.COLORTOBLACK_RANGE_GET = hi_isp_api_get_colortoblack_range,
		.GET_CUR_FPS = hi_isp_api_get_cur_fps,
		.DAYNIGHT_MODE_GET      = hi_isp_api_get_daynight_mode,
	},
	.sensor_type = SENSOR_MODEL_APTINA_AR0130,
	.gpio_status_old = ISP_GPIO_DAYLIGHT,// ISP_GPIO_DAYLIGHT;//daytime
	.color_max_value = {
		.HueMax = 100,
		.SaturationMax = 100,
		.ContrastMax = 100,
		.BrightnessMax = 100,
	},
	.ircut_auto_switch_enable = HI_TRUE,// HI_TRUE;
	.ircut_switch_control_mode = IRLED_CONTROL_MODE,
	.lowlight_mode = 0,
	.isp_auto_drc_enabled = HI_TRUE,
	.AfAttr.param = NULL,
	.AfAttr.af_callback = NULL,
	.isp_framerate_status = HI_FALSE,
	.filter_frequency = 50,
	.starlight_mode_enable = HI_FALSE,
	.sensor_set_mirror = NULL,	
	.sensor_set_flip = NULL,
	.md_lock = HI_TRUE,
	.md_alarm_pid = NULL,
};

 HI_S32  HI_SDK_ISP_vi_start_dev(VI_DEV ViDev)
 {	 
	 VI_CHN_ATTR_S stChnAttr;
	 ISP_WDR_MODE_S stWdrMode;
	 VI_DEV_ATTR_S vi_dev_attr_720p_30fps;
	 VI_WDR_ATTR_S stWdrAttr;
	 
	 memset(&vi_dev_attr_720p_30fps, 0, sizeof(vi_dev_attr_720p_30fps));
	 
	 stWdrMode.enWDRMode = _isp_attr.isp_wdr_mode;
	 switch(_isp_attr.sensor_type){
		 default:
		 case SENSOR_MODEL_APTINA_AR0237:
			 SET_VI_DEV_ATTR_AR0237(vi_dev_attr_720p_30fps);		 //TO DO
			 break;
		case SENSOR_MODEL_SC2235:
			 SET_VI_DEV_ATTR_SC2235(vi_dev_attr_720p_30fps);		 //TO DO
			 break;
	 }	 
	 SOC_CHECK(HI_MPI_ISP_SetWDRMode(0, &stWdrMode));
	 SOC_CHECK(HI_MPI_VI_SetDevAttr(ViDev, &vi_dev_attr_720p_30fps));	 
	 SOC_CHECK(HI_MPI_ISP_GetWDRMode(0, &stWdrMode));
 
	 
	 stWdrAttr.enWDRMode = stWdrMode.enWDRMode;
	 stWdrAttr.bCompress = HI_FALSE;
 
	 SOC_CHECK(HI_MPI_VI_SetWDRAttr(ViDev, &stWdrAttr));

	 
	 SOC_CHECK(HI_MPI_VI_EnableDev(ViDev));
	 return 0;
 }

 
int HI_SDK_ISP_set_wdr_mode_ini(uint8_t mode)
{	 
	ISP_DEV IspDev = 0; 
	VI_CHN ViChn = 0;
	VI_DEV ViDev = HI3518A_VIN_DEV;  //0
	ISP_DRC_ATTR_S pstDRC;
	ISP_WDR_MODE_S stWDRMode;
	ISP_INNER_STATE_INFO_S stInnerStateInfo = {0};
	
    HI_S32 s32Tries = 25;

	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));
	pstDRC.bEnable = mode;
	_isp_attr.isp_auto_drc_enabled = mode; 		 
	SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&pstDRC));
	if(0 == mode){
		/* switch to linear mode */
		printf("******switch to linear mode******\n");
		_isp_attr.isp_wdr_mode = WDR_MODE_NONE;
		stWDRMode.enWDRMode = WDR_MODE_NONE;
			
	}else if(1 == mode){
		/* switch to 2to1 line WDR mode*/
		printf("******switch to 2to1 line WDR mode******\n");	
		_isp_attr.isp_wdr_mode = WDR_MODE_2To1_LINE;		 
		stWDRMode.enWDRMode = WDR_MODE_2To1_LINE;
		
	}else{
	}


    // 1. Disable Chn and Dev
	SOC_CHECK(HI_MPI_VI_DisableChn(ViChn));
	SOC_CHECK(HI_MPI_VI_DisableDev(ViDev));

	
    // 2. configure MIPI Attr
	SOC_CHECK(hi_isp_SetMipiAttr());

    // 3. Set WDR Mode to ISP
	SOC_CHECK(HI_MPI_ISP_SetWDRMode(IspDev, &stWDRMode));

	
	// 4. query WDR Switch Status
	while (s32Tries > 0)
	{
		HI_MPI_ISP_QueryInnerStateInfo(IspDev, &stInnerStateInfo); 	
		if (HI_TRUE == stInnerStateInfo.bWDRSwitchFinish)
		{			
			printf("wdr switch finish!!\n");
			break;
		}
		//printf("query ISP inner state ... ...\n");
		usleep(20000);
		s32Tries--;
	}

    // 5. Start Dev and Chn again
	SOC_CHECK(HI_SDK_ISP_vi_start_dev(ViDev));
	
    usleep(100000);
	SOC_CHECK(HI_MPI_VI_EnableChn(ViChn));


	//run senser default isp 
	_hi_sdk_isp_init_isp_default_value();

	HI_SDK_ISP_set_isp_sensor_value();
	return 0;

}
static HI_VOID* ISP_Thread_Run(HI_VOID *param)
{
    pthread_detach(pthread_self());
 //   ISP_DEV IspDev = 0;
    SOC_CHECK(HI_MPI_ISP_Run(HI3518A_VIN_DEV));

    return HI_NULL;
}

static HI_S32 pfunction_af_init(HI_S32 s32Handle, const ISP_AF_PARAM_S *pstAfParam)
{
	if (HI_NULL == pstAfParam)
    {
        printf("null pointer when af init default value!\n");
        return -1;
    }

	return 0;
}


#if 1


//图像权重
static int HI16EV1_AFWeight[8][8] = {
	{1,1,1,1,1,1,1,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,1,1,1,1,1,1,1,},
};

#define AF_BLEND_SHIFT 		6
#define AF_ALPHA 			29 // 0.45
#define AF_BELTA				54 // 0.85

static HI_S32 pfunction_af_run(HI_S32 s32Handle, const ISP_AF_INFO_S *pstAfInfo, ISP_AF_RESULT_S *pstAfResult, HI_S32 s32Rsv)
{
	int ret = 0, i=0, j=0;
	if(NULL != _isp_attr.AfAttr.af_callback){
		//printf("sock = %d\r\n", *_isp_attr.AfAttr.param);
		
		ISP_STATISTICS_CFG_S stIspStaticsCfg;
		HI_U32 u32Fv1_n, u32Fv1, u32Fv2, u32Fv3, u32Fv2_n;
		static ISP_FOCUS_STATISTICS_CFG_S stFocusCfg;
		
		HI_MPI_ISP_GetStatisticsConfig(0, &stIspStaticsCfg);	
		memcpy(&stFocusCfg, &stIspStaticsCfg.stFocusCfg, sizeof(ISP_FOCUS_STATISTICS_CFG_S));

		HI_U32 u32SumFv1 = 0;
		HI_U32 u32SumFv2 = 0;
		HI_U32 u32WgtSum = 0;

		//get data 
		for(i=0; i<stFocusCfg.stConfig.u16Vwnd; i++)
		{
			for(j=0; j<stFocusCfg.stConfig.u16Hwnd; j++)
			{
				HI_U32 u32H1 = pstAfInfo->pstAfStat->stZoneMetrics[i][j].u16h1;
				HI_U32 u32V1 = pstAfInfo->pstAfStat->stZoneMetrics[i][j].u16v1;
				
				HI_U32 u32H2 = pstAfInfo->pstAfStat->stZoneMetrics[i][j].u16h2;
				HI_U32 u32V2 = pstAfInfo->pstAfStat->stZoneMetrics[i][j].u16v2;
				
				u32Fv1_n = (u32H1 * AF_ALPHA + u32V1 * ((1<<AF_BLEND_SHIFT) - AF_ALPHA)) >> AF_BLEND_SHIFT;
				u32Fv2_n = (u32H2 * AF_BELTA + u32V2 * ((1<<AF_BLEND_SHIFT) - AF_BELTA)) >> AF_BLEND_SHIFT;

				u32SumFv1 += HI16EV1_AFWeight[i][j] * u32Fv1_n;
				u32SumFv2 += HI16EV1_AFWeight[i][j] * u32Fv2_n;
				u32WgtSum += HI16EV1_AFWeight[i][j];
				//printf("weight[%d][%d]=%d--%d\n", i,j, HI16EV1_AFWeight[i][j], u32Fv1_n);
			}
		}

		u32Fv1 = u32SumFv1;
		u32Fv2 = u32SumFv2;
		u32Fv3 = u32SumFv1 / u32WgtSum;
		
		ret = _isp_attr.AfAttr.af_callback((int)u32Fv1,(int)u32Fv2, (int)u32Fv3, _isp_attr.AfAttr.param);

		if(ret< 0){
			if(_isp_attr.AfAttr.param){
				free(_isp_attr.AfAttr.param);
				_isp_attr.AfAttr.param = NULL;
				_isp_attr.AfAttr.af_callback = NULL;
			}
		}
	}
	//printf("u16FocusMetrics = %d - %d\r\n", pstAfInfo->pstAfStat->u16FocusMetrics, pstAfResult->stStatAttr.bChange);

	return 0;
}

static HI_S32 pfunctin_af_ctrl(HI_S32 s32Handle, HI_U32 u32Cmd, HI_VOID *pValue)
{
	return 0;
}

static HI_S32 pfunction_af_exit(HI_S32 s32Handle)
{
	return 0;
}

static int isp_af_init_function(	ISP_AF_REGISTER_S *pAfRegister)
{
	printf("MTEST isp_af_init_function \n");
	memset(&pAfRegister->stAfExpFunc, 0, sizeof(ISP_AF_EXP_FUNC_S));
	pAfRegister->stAfExpFunc.pfn_af_init = pfunction_af_init;
	pAfRegister->stAfExpFunc.pfn_af_ctrl= pfunctin_af_ctrl;
	pAfRegister->stAfExpFunc.pfn_af_run = pfunction_af_run;
	pAfRegister->stAfExpFunc.pfn_af_exit = pfunction_af_exit;
}

static int hi16EV1_af_register_callback(ISP_AF_REGISTER_S *pAfRegister)
{
	printf("MTEST hi16EV1_af_register_callback \n");

	HI_S32 s32Ret = 0;
	ALG_LIB_S stLib;
	
	stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    s32Ret = HI_MPI_ISP_AFLibRegCallBack(0, &stLib, pAfRegister);
    if (HI_SUCCESS != s32Ret)
    {
        printf("sensor register callback function to AF lib failed!\n");
        return s32Ret;
    }
	return s32Ret;
}

#endif

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

static void hi_mpp_vb_destroy()
{
	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();
}

static void HI_SDK_mpp_vb_init(HI_U32 width, HI_U32 height)
{
	MPP_SYS_CONF_S sys_conf;
	VB_CONF_S vb_conf;
	hi_mpp_vb_destroy();
	memset(&vb_conf, 0, sizeof(vb_conf));
	mpp_vb_conf_clear(&vb_conf);
#if defined(HI3516C_V3)
	if(width * height <=  1920 * 1080){
		mpp_vb_conf_add_block(&vb_conf, 1920 * 1080 * 3/2 + 48, 7);
		mpp_vb_conf_add_block(&vb_conf, 720 * 576 * 3/2 + 100, 4);
		mpp_vb_conf_add_block(&vb_conf, 640 * 360 * 3/2 + 100, 6);
	}else{	
		mpp_vb_conf_add_block(&vb_conf, 1920 * 1080 * 3/2 + 48, 5);
		mpp_vb_conf_add_block(&vb_conf, 720 * 576 * 3/2 + 100, 4);
		mpp_vb_conf_add_block(&vb_conf, 640 * 360 * 3/2 + 100, 5);
	}
#elif defined (HI3516E_V1)
	if(width * height <=  1280 * 960){
		mpp_vb_conf_add_block(&vb_conf, 1280 * 960 * 3/2 + 48, 3);// 3    6220896    9216240   5
		mpp_vb_conf_add_block(&vb_conf, 650 * 400 * 3/2 + 100, 5);//5	3	
		mpp_vb_conf_add_block(&vb_conf, 400 * 300 * 3/2 + 100, 2);//vda  352 x 288	9
	}else if(width * height <=  1920 * 1080){
		//for low delay
		mpp_vb_conf_add_block(&vb_conf, 1920 * 1080 * 3/2 + 48, 1);//main (stream + JPEG)
		mpp_vb_conf_add_block(&vb_conf, 650 * 400 * 3/2 + 100, 5);//sub (stream + JPEG)  
		mpp_vb_conf_add_block(&vb_conf, 400 * 300 * 3/2 + 100, 2);//MD 
	}else{	
		mpp_vb_conf_add_block(&vb_conf, 1920 * 1080 * 3/2 + 48, 1);
		mpp_vb_conf_add_block(&vb_conf, 650 * 400 * 3/2 + 100, 5);
		mpp_vb_conf_add_block(&vb_conf, 400 * 300 * 3/2 + 100, 2);
	}

#endif	
	SOC_CHECK(HI_MPI_VB_SetConf(&vb_conf));
	SOC_CHECK(HI_MPI_VB_Init());
	memset(&sys_conf, 0, sizeof(sys_conf));
    sys_conf.u32AlignWidth = 64;
    SOC_CHECK(HI_MPI_SYS_SetConf(&sys_conf));
    SOC_CHECK(HI_MPI_SYS_Init());
}


int HI_SDK_ISP_init(lpSensorApi*api, lpBSPApi bsp_api)
{
	 
//	pthread_t isp_tid = 0;
	VI_DEV_ATTR_S vi_dev_attr_720p_30fps;
//	VI_DEV_ATTR_EX_S vi_dev_attr_720p_30fps;

	ISP_PUB_ATTR_S stPubAttr;
	VI_CHN_ATTR_S stChnAttr;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_MODE_S stVpssChnMode;
	ISP_WDR_MODE_S stWdrMode;	
	ISP_AF_REGISTER_S stAfRegister;
	memcpy(&_isp_attr.bsp_api, bsp_api, sizeof(stBSPApi));
	HI_SDK_ISP_sensor_check();
//	isp_af_init_function(&stAfRegister);
	HI_SDK_ISP_set_sensor_resolution(NULL, NULL);
	memset(&stVpssGrpAttr, 0, sizeof(stVpssGrpAttr));
	isp_af_init_function(&stAfRegister);
	hi16EV1_af_register_callback(&stAfRegister);
	//init sensor
	//printf("sensor type:%d\r\n", _isp_attr.sensor_type);
	switch(_isp_attr.sensor_type){
		default:

			case SENSOR_MODEL_APTINA_AR0237:
			APTINA_AR0237_init((SENSOR_APTINA_AR0237_DO_I2CRD)ar0237_i2c_read, 
				(SENSOR_APTINA_AR0237_DO_I2CWR)ar0237_i2c_write); //&stAfRegister
			SET_VI_DEV_ATTR_AR0237(vi_dev_attr_720p_30fps);
			_isp_attr.sensor_set_mirror = AR0237_set_mirror;  //mirror
			_isp_attr.sensor_set_flip = AR0237_set_flip;      // flip
			_isp_attr.sensor_get_name = AR0237_get_sensor_name;
			break;
			case SENSOR_MODEL_SC2235:
			SmartSens_SC2235_init((SENSOR_SMARTSENS_SC2235_DO_I2CRD)sc2235_i2c_read, 
				(SENSOR_SMARTSENS_SC2235_DO_I2CWR)sc2235_i2c_write); //&stAfRegister
			SET_VI_DEV_ATTR_SC2235(vi_dev_attr_720p_30fps);
			
			_isp_attr.sensor_set_mirror = SC2235_set_mirror;  //mirror
			_isp_attr.sensor_set_flip = SC2235_set_flip;      // flip
			
			_isp_attr.sensor_get_name = SC2235_get_sensor_name;

			break;
			case SENSOR_MODEL_SC2232:
			SmartSens_SC2232_init();
			SET_VI_DEV_ATTR_SC2235(vi_dev_attr_720p_30fps);
			_isp_attr.sensor_set_mirror = SC2232_set_mirror;  //mirror
			_isp_attr.sensor_set_flip = SC2232_set_flip;      // flip
			_isp_attr.sensor_get_name = SC2232_get_sensor_name;
			break;
			case SENSOR_MODEL_IMX307:
			IMX307_init();
			SET_VI_DEV_ATTR_IMX307(vi_dev_attr_720p_30fps);
			_isp_attr.sensor_set_mirror = IMX307_set_mirror;  //mirror
			_isp_attr.sensor_set_flip = IMX307_set_flip;      // flip
			_isp_attr.sensor_get_name = IMX307_get_sensor_name;
			break;
	}
	
	//return 0;
	switch(_isp_attr.sensor_type){		
		default:

          case SENSOR_MODEL_APTINA_AR0237:
          
			stPubAttr.enBayer	   = BAYER_GRBG;
			stPubAttr.f32FrameRate			= 25;
			stPubAttr.stWndRect.s32X		= 0;
			stPubAttr.stWndRect.s32Y		= 0;
			stPubAttr.stWndRect.u32Width	= 1920;
			stPubAttr.stWndRect.u32Height	= 1080;
			
			stChnAttr.stCapRect.s32X = 0;
			stChnAttr.stCapRect.s32Y = 0;
			stChnAttr.stCapRect.u32Width  = 1920;
			stChnAttr.stCapRect.u32Height = 1080;
			stChnAttr.stDestSize.u32Width = 1920;
			stChnAttr.stDestSize.u32Height = 1080;
			
			stVpssGrpAttr.u32MaxW = 1920;
			stVpssGrpAttr.u32MaxH = 1080;
			
			stVpssChnMode.u32Width		 = 1920;		
			stVpssChnMode.u32Height 	 = 1080;
			
			stWdrMode.enWDRMode  = WDR_MODE_NONE;//WDR_MODE_2To1_LINE //WDR_MODE_NONE
        break;
	case SENSOR_MODEL_SC2232:
 	case SENSOR_MODEL_SC2235:
			stPubAttr.enBayer	   = BAYER_BGGR;
			stPubAttr.f32FrameRate			= 25;
			stPubAttr.stWndRect.s32X		= 0;
			stPubAttr.stWndRect.s32Y		= 0;
			stPubAttr.stWndRect.u32Width	= 1920;
			stPubAttr.stWndRect.u32Height	= 1080;
			
			stChnAttr.stCapRect.s32X = 0;
			stChnAttr.stCapRect.s32Y = 0;
			stChnAttr.stCapRect.u32Width  = 1920;
			stChnAttr.stCapRect.u32Height = 1080;
			stChnAttr.stDestSize.u32Width = 1920;
			stChnAttr.stDestSize.u32Height = 1080;
			
			stVpssGrpAttr.u32MaxW = 1920;
			stVpssGrpAttr.u32MaxH = 1080;
			
			stVpssChnMode.u32Width		 = 1920;		
			stVpssChnMode.u32Height 	 = 1080;
			
			stWdrMode.enWDRMode  = WDR_MODE_NONE;
        break;
		case SENSOR_MODEL_IMX307:
			stPubAttr.enBayer	   = BAYER_GBRG;
			stPubAttr.f32FrameRate			= 25;
			stPubAttr.stWndRect.s32X		= 0;
			stPubAttr.stWndRect.s32Y		= 0;
			stPubAttr.stWndRect.u32Width	= 1920;
			stPubAttr.stWndRect.u32Height	= 1080;

			stChnAttr.stCapRect.s32X = 0;
			stChnAttr.stCapRect.s32Y = 0;
			stChnAttr.stCapRect.u32Width  = 1920;
			stChnAttr.stCapRect.u32Height = 1080;
			stChnAttr.stDestSize.u32Width = 1920;
			stChnAttr.stDestSize.u32Height = 1080;

			stVpssGrpAttr.u32MaxW = 1920;
			stVpssGrpAttr.u32MaxH = 1080;

			stVpssChnMode.u32Width		 = 1920;
			stVpssChnMode.u32Height 	 = 1080;

			stWdrMode.enWDRMode  = WDR_MODE_NONE;
        break;
	}	


	int enc_width,enc_height;	
	
	enc_width = _isp_attr.sensor_resolution_width;
	enc_height = _isp_attr.sensor_resolution_height;

#if defined(HI3516E_V1)	
		if(-1 != access(MAIN_RESOLUTION, F_OK)){//exist
			
			FILE *fid = fopen(MAIN_RESOLUTION, "rb");
			char buf[16] = "";
			char tmp[5] = "";		
	
			int width,height;
			
			if(NULL != fid){	
				fread(buf,sizeof(buf),1,fid);
				fclose(fid);
				fid = NULL;
				sscanf(buf,"%d %4s %d",&width,tmp,&height);
	
				printf("width x height = %d %s %d \n",width, tmp, height);
				
				if(width >= 720 && width <=_isp_attr.sensor_resolution_width &&
					height >= 576 && height <= _isp_attr.sensor_resolution_height){ 	
	
					if(_isp_attr.sensor_resolution_width >= 1920 && _isp_attr.sensor_resolution_height >= 1080 &&
						width < 1920 && height <= 1080 ){
						stPubAttr.stWndRect.u32Width	 = 1280;
						stPubAttr.stWndRect.u32Height	  = 960;
						
						stChnAttr.stCapRect.u32Width  = 1280;
						stChnAttr.stCapRect.u32Height = 960;
						stChnAttr.stDestSize.u32Width = 1280;
						stChnAttr.stDestSize.u32Height = 960;
						
						stVpssGrpAttr.u32MaxW = 1280;
						stVpssGrpAttr.u32MaxH = 960;
										
						stVpssChnMode.u32Width		 = 1280;		
						stVpssChnMode.u32Height 	 = 960; 
					}else{
			
					}
					enc_width = width;
					enc_height = height;
	
				}else{
					char cmd[50]= "";
					sprintf(cmd, "rm -rf %s",MAIN_RESOLUTION);
					system(cmd);
				}
	
			}else{
	
				printf("open %s failed\n",MAIN_RESOLUTION);
			}
		}else{
			char cmd[48];
			snprintf(cmd, sizeof(cmd), "echo %d x %d > %s",stChnAttr.stDestSize.u32Width,stChnAttr.stDestSize.u32Height,MAIN_RESOLUTION);
			system(cmd);
			printf("Create %s !\n",MAIN_RESOLUTION);
	
		}
#endif




	HI_SDK_mpp_vb_init(enc_width,enc_height);

	_isp_attr.src_framerate = stPubAttr.f32FrameRate ;
	_isp_attr.isp_wdr_mode = stWdrMode.enWDRMode;


	{
		//isp init
		SOC_CHECK(HI_MPI_ISP_MemInit(HI3518A_VIN_DEV));
	/* 6. isp set WDR mode */
	
	SOC_CHECK(HI_MPI_ISP_SetWDRMode(0, &stWdrMode));
    /* 7. isp set pub attributes */
    /* note : different sensor, different ISP_PUB_ATTR_S define.
              if the sensor you used is different, you can change
              ISP_PUB_ATTR_S definition */
		//SOC_CHECK(HI_MPI_ISP_SetPubAttr(HI3518A_VIN_DEV, &stPubAttr));
		int ret;
		ret = HI_MPI_ISP_SetPubAttr(HI3518A_VIN_DEV, &stPubAttr);
    	SOC_CHECK(HI_MPI_ISP_Init(HI3518A_VIN_DEV));
  	
    	HI_MPI_ISP_GetPubAttr(HI3518A_VIN_DEV,&stPubAttr);



	//	pthread_t pid;
	    if (0 != pthread_create(&_isp_attr.isp_run_pid, 0, (void* (*)(void*))ISP_Thread_Run, NULL))
	    {
	        printf("%s: create isp running thread failed!\n", __FUNCTION__);
	        return HI_FAILURE;
	    }

	}
	{
	
		SOC_CHECK(HI_MPI_VI_SetDevAttr(HI3518A_VIN_DEV, &vi_dev_attr_720p_30fps));
		
	//	SOC_CHECK(HI_MPI_VI_SetDevAttrEx(HI3518A_VIN_DEV, &vi_dev_attr_720p_30fps));
#if 1
		    SOC_CHECK(HI_MPI_ISP_GetWDRMode(HI3518A_VIN_DEV, &stWdrMode));
		    
		    if (stWdrMode.enWDRMode)  //wdr mode
		    {
		        VI_WDR_ATTR_S stWdrAttr;

		        stWdrAttr.enWDRMode = stWdrMode.enWDRMode;
		        stWdrAttr.bCompress = HI_FALSE;

		        SOC_CHECK(HI_MPI_VI_SetWDRAttr(HI3518A_VIN_DEV, &stWdrAttr));
		    }
#endif
		SOC_CHECK(HI_MPI_VI_EnableDev(HI3518A_VIN_DEV));
	}
	{
	//	VI_CHN_ATTR_S stChnAttr;
	    /* step  5: config & start vicap dev */

	    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
	    stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;   /* sp420 or sp422 */

	    stChnAttr.bMirror = HI_FALSE;
	    stChnAttr.bFlip = HI_FALSE;

	    stChnAttr.s32SrcFrameRate = stPubAttr.f32FrameRate;
	    stChnAttr.s32DstFrameRate = stPubAttr.f32FrameRate;
	    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;

		
		SOC_CHECK(HI_MPI_VI_SetChnAttr(HI3518A_VIN_CHN, &stChnAttr));
		SOC_CHECK(HI_MPI_VI_EnableChn(HI3518A_VIN_CHN));
	}
	{
		VPSS_GRP VpssGrp = 0;		
	    VPSS_CHN VpssChn = 1; //  1
	    VPSS_CHN_ATTR_S stVpssChnAttr;

		stVpssGrpAttr.bIeEn = HI_FALSE;
		stVpssGrpAttr.bNrEn = HI_TRUE;
		stVpssGrpAttr.bHistEn = HI_FALSE;
		stVpssGrpAttr.bDciEn = HI_FALSE;
		stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
		stVpssGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		VPSS_GRP_PARAM_S stVpssParam;

		SOC_CHECK(HI_MPI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr));
			
	        /*** set vpss param ***/		
	/*	VPSS_NR_PARAM_U unNrParam = {{0}};
        HI_MPI_VPSS_GetNRParam(VpssGrp, &unNrParam);      
        HI_MPI_VPSS_SetNRParam(VpssGrp, &unNrParam);
 */
		SOC_CHECK(HI_MPI_VPSS_StartGrp(VpssGrp));
		MPP_CHN_S stSrcChn;
    	MPP_CHN_S stDestChn;
		stSrcChn.enModId  = HI_ID_VIU;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = 0;
    
        stDestChn.enModId  = HI_ID_VPSS;
        stDestChn.s32DevId = VpssGrp;
        stDestChn.s32ChnId = 0;
		
        SOC_CHECK(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn));

		stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
		stVpssChnMode.bDouble		 = HI_FALSE;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
		memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
		stVpssChnAttr.s32SrcFrameRate = -1;
		stVpssChnAttr.s32DstFrameRate = -1;

		SOC_CHECK(HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr));
		SOC_CHECK(HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, &stVpssChnMode));
		SOC_CHECK(HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn));
	}
	_hi_sdk_isp_init_isp_default_value();
	isp_ircut_switch(0);//default for daylight
	_isp_attr.md_alarm_thread_trigger = true;

    pthread_attr_t pthread_attr;
    int nRet;
    nRet = pthread_attr_init(&pthread_attr);
    if(0 == nRet)
    {
        pthread_attr_setstacksize(&pthread_attr, 131072);
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
        pthread_create(&_isp_attr.md_alarm_pid, &pthread_attr, isp_md_alarm_cal, NULL);
        pthread_attr_destroy(&pthread_attr);
    }

	*api = &_isp_attr.api;

	NR_X_AutoInit();

	return 0;
}
	
int HI_SDK_ISP_destroy()
{
	NR_X_AutoExit();

    if(_isp_attr.ispCfgAttr){
   	  free(_isp_attr.ispCfgAttr);
    }
   
    SOC_CHECK(HI_MPI_VI_DisableChn(HI3518A_VIN_CHN));
    SOC_CHECK(HI_MPI_VI_DisableDev(HI3518A_VIN_CHN));
	if(_isp_attr.md_alarm_thread_trigger){
		_isp_attr.md_alarm_thread_trigger = false;
		pthread_join(_isp_attr.md_alarm_pid, 0);
		_isp_attr.md_alarm_pid = NULL;
	}

    if(_isp_attr.sensor_type ==  SENSOR_MODEL_IMX307)
    {
        _isp_pwm_light_deinit();
    }

    SOC_CHECK(HI_MPI_ISP_Exit(0));
    if(_isp_attr.isp_run_pid){
   	 pthread_join(_isp_attr.isp_run_pid, 0);
   	 _isp_attr.isp_run_pid = 0;
    }
    
    hi_mpp_vb_destroy();
    
    return 0;
}

//for judgement of imx307 LQD package
/*********************************/
#define COUNT_FOR_DETERMINATION (10)
#define GPIO_IN_GROUP (0)
#define GPIO_IN_PIN (5)
#define GPIO_OUT_GROUP (0)
#define GPIO_OUT_PIN (6)

static int _init_gpio_dir()
{
	int ret = -1;

	//gpio muxpin setting
	if(sdk_sys){
		//gpio input
		sdk_sys->write_reg(0x12040028,0);
		//gpio output
		sdk_sys->write_reg(0x12040034,0);
		ret = 0;
	}

	return ret;
}

static bool _is_imx307_LQD()
{
	uint32_t reg_gpio_in = 0, reg_gpio_out = 0;
	int count = 0, i = 0;
	bool result = false;

	//gpio muxpin setting
	_init_gpio_dir();

	//judgement
	for(i = 0; i < COUNT_FOR_DETERMINATION; i++){
		reg_gpio_out = (~reg_gpio_out)&0x1;
		isp_gpio_pin_write(GPIO_OUT_GROUP, GPIO_OUT_PIN, reg_gpio_out);
		reg_gpio_in = isp_gpio_pin_read(GPIO_IN_GROUP, GPIO_IN_PIN);

		if(reg_gpio_in == reg_gpio_out){
			//short circuit
			count++;
		}
	}
	if(count >= COUNT_FOR_DETERMINATION){
		//imx307 LQD package
		result = true;
		printf("IMX307 LQD package!!\r\n");
	}

	//set gpio out to low level
	isp_gpio_pin_write(GPIO_OUT_GROUP, GPIO_OUT_PIN, 0);

	return result;
}

int IsDayNight(void)
{
	return (_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT);
}
/*********************************/

