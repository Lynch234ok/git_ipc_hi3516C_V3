#include <errno.h>
#include "sdk/sdk_debug.h"
#include "sdk/sdk_api.h"
#include "sdk/sdk_isp_def.h"
#include "msg_broker.h"
#include "msg_format.h"

extern void send_msg(MsgContext *msg, const uint8_t *data, size_t data_size);

#define ISP_GPIO_DAYLIGHT (0)
#define ISP_GPIO_NIGHT (1)

#define SENSOR_TYPE_FILE    "/media/conf/sensor_type"

typedef struct _vatics_isp_attr
{
	stSensorApi api;
	emSENSOR_MODEL sensor_type;
	uint8_t ircut_auto_switch_enable;// HI_TRUE;
	uint8_t ircut_control_mode;
	uint32_t gpio_status_old;// ISP_GPIO_DAYLIGHT;//daytime
	uint32_t saturation;
}stVatIspAttr,*lpVatIspAttr;

static stVatIspAttr _isp_attr;

int gpio_read(int fd, char* value, int len)
{
	int ret = -1;
	if(fd < 0)
	{
		printf("gpio fd error !: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}
	
	ret = read(fd, value, len);
	
	ret = ((ret == -1) ? -errno : 0);
	if(ret < 0)
	{
		printf("gpio_read failed to (%s): %s, %s, %d\n", strerror(errno), __FILE__,  __func__, __LINE__);
	}
	
	return ret;
}

int gpio_write(int fd, char* value, int len)
{
	int ret = -1;
	if(fd < 0)
	{
		printf("gpio fd error !: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}
	
	ret = write(fd, value, len);	
	
	ret = ((ret == -1) ? -errno : 0);
	if(ret < 0)
	{
		printf("gpio_write failed to (%s): %s, %s, %d\n", strerror(errno), __FILE__,  __func__, __LINE__);
	}
	
	return ret;
}

#define IMAGE_TYPE_SATURATION (0)
#define IMAGE_TYPE_CONTRAST (1)
#define IMAGE_TYPE_BRIGHTNESS (2)
#define IMAGE_TYPE_HUE	(3)
static int va_msg_set_image(int type, uint8_t value)//type  0:saturation 1:contrast 2:brightness 3:hue
{
	MsgContext msg_context;
	msg_fec_t fec_msg;
	char hostname[32];
	int stream_tmp = 0;//stream;
	int msg_value;
	
	memset(&msg_context, 0, sizeof(MsgContext));
	memset(&fec_msg, 0, sizeof(msg_fec_t));

	snprintf(hostname, sizeof(hostname), "source0");
	msg_context.host = hostname;

	switch(type){
		default:
		case IMAGE_TYPE_SATURATION:
			msg_context.cmd = "setSaturation";
			msg_value = value*2;
			break;
		case IMAGE_TYPE_CONTRAST:
			msg_context.cmd = "setContrast";
			msg_value = value*2;
			break;
		case IMAGE_TYPE_BRIGHTNESS:
			msg_context.cmd = "setBright";
			msg_value = value*2;
			break;
		case IMAGE_TYPE_HUE:
			msg_context.cmd = "setHue";
			msg_value = value - 50;
			break;			
	}	
	printf("%s:%d\n", msg_context.cmd, msg_value);
	send_msg(&msg_context, (int*)&msg_value, sizeof(unsigned int));

}

static uint32_t light_status(void)
{
	static uint32_t ret_val = ISP_GPIO_DAYLIGHT;
	int ret;
    FILE *fp=NULL;
    char file_dir[256], file_buf[5]={0};

	sprintf(file_dir, "/sys/devices/platform/vpl_gpadc/adc");
	fp=fopen(file_dir, "r");
	if(fp)
	{
        ret=fread(file_buf, sizeof(file_buf)-1, 1, fp);
		if(ret>0)
		{
			ret_val = ISP_GPIO_NIGHT;
		}
		else
		{
			ret_val = ISP_GPIO_DAYLIGHT;
		}
		fclose(fp);
	}
	
	return ret_val;
}

static void isp_ircut_mode(uint32_t IOmode)
{
	if(ISP_GPIO_DAYLIGHT == IOmode)
	{
		_isp_attr.gpio_status_old = ISP_GPIO_DAYLIGHT;
		va_msg_set_image(IMAGE_TYPE_SATURATION, _isp_attr.saturation);
	 	int fd = open("/sys/class/gpio/gpio0/direction", O_RDWR);
		if(fd >= 0)
		{
			gpio_write(fd, "out", 3);
			close(fd);
		}
		
		fd = open("/sys/class/gpio/gpio0/value", O_RDWR);
		if(fd >= 0)
		{
			gpio_write(fd, "0", 1);
		    close(fd);
		}				
		
        fd = open("/sys/class/gpio/gpio3/direction", O_RDWR);
		if(fd >= 0)
		{
			gpio_write(fd, "out", 3);
			close(fd);
		}
		
       	fd = open("/sys/class/gpio/gpio3/value", O_RDWR);
       	if(fd >= 0)
		{
			gpio_write(fd, "1", 1);
			close(fd);
		}	
	}
	else
	{
		_isp_attr.gpio_status_old = ISP_GPIO_NIGHT;
		va_msg_set_image(IMAGE_TYPE_SATURATION, 0);
		int fd = open("/sys/class/gpio/gpio0/direction", O_RDWR);
		if(fd >= 0)
		{
			gpio_write(fd, "out", 3);
			close(fd);
		}
		
		fd = open("/sys/class/gpio/gpio0/value", O_RDWR);
		if(fd >= 0)
		{
			gpio_write(fd, "1", 1);
			close(fd);
		}				
 		       	     
        fd = open("/sys/class/gpio/gpio3/direction", O_RDWR);
		if( fd >= 0)
		{
			gpio_write(fd, "out", 3);
			close(fd);
		}
		
		fd = open("/sys/class/gpio/gpio3/value", O_RDWR);
		if(fd >= 0)
		{
			gpio_write(fd, "0", 1);
			close(fd);							
		}				

	}
}

static void isp_ircut_common()
{
	int fd = open("/sys/class/gpio/gpio0/direction", O_RDWR);
	if(fd >= 0)
	{
		gpio_write(fd, "out", 3);
		close(fd);
	}
	
	fd = open("/sys/class/gpio/gpio0/value", O_RDWR);
	if(fd >= 0)
	{
		gpio_write(fd, "0", 1);	
		close(fd);
	}			

	fd = open("/sys/class/gpio/gpio3/direction", O_RDWR);
	if(fd >= 0)
	{
		gpio_write(fd, "out", 3);
		close(fd);
	}
	
	fd = open("/sys/class/gpio/gpio3/value", O_RDWR);
	if(fd >= 0)
	{
		gpio_write(fd, "0", 1);
		close(fd);
	}			

}

static void va_isp_ircut_switch(uint32_t IOmode)//0: daytime   1: night
{
	//do GPIO init in rc.local
	/*static int ExportFlag = 0;
	if(ExportFlag == 0)
	{
		int fd = open("/sys/class/gpio/export", O_WRONLY);
		if(fd){
			write(fd, "0", 1);
			write(fd, "3", 1);

			close(fd);

			ExportFlag = 1;
		}
	}*/
	
	printf("va_isp_ircut_switch %s\n", IOmode == ISP_GPIO_DAYLIGHT?"daylight":"night");
	isp_ircut_mode(IOmode);
	usleep(1000*150);
	isp_ircut_common();
}

emSENSOR_MODEL va_isp_api_get_sensor_model()
{
	return _isp_attr.sensor_type;
}

void va_isp_api_mirror_flip(unsigned char mode)
{

}
void va_isp_api_test_mode(unsigned char enable){}
void va_isp_api_light_mode(unsigned char mode){}
void va_isp_api_set_hue(unsigned short val)
{
	va_msg_set_image(IMAGE_TYPE_HUE, val);
}

void va_isp_api_set_saturation(unsigned char val)//val should be 0,1,2,3,4,5,6,7
{
	_isp_attr.saturation = val;
	if(ISP_GPIO_NIGHT != _isp_attr.gpio_status_old){
		va_msg_set_image(IMAGE_TYPE_SATURATION, _isp_attr.saturation);
	}
}
unsigned char  va_isp_api_get_saturation()
{
	return _isp_attr.saturation;
}
void va_isp_api_set_brightness(unsigned char val)
{
	va_msg_set_image(IMAGE_TYPE_BRIGHTNESS, val);
}
void va_isp_api_set_contrast(unsigned char val)
{
	va_msg_set_image(IMAGE_TYPE_CONTRAST, val);
}

void va_isp_api_color_mode(unsigned char mode)
{

}

void va_isp_api_reg_write(uint16_t addr,uint16_t val)
{

}

uint16_t va_isp_api_reg_read(uint16_t addr)
{
	return 0;
}

void va_isp_api_spec_reg_write(uint8_t page, uint16_t addr,uint16_t val)
{

}
uint16_t va_isp_api_spec_reg_read(uint8_t page, uint16_t addr)
{
	return 0;
}


stSensorColorMaxValue va_isp_api_get_color_max_value()
{
	stSensorColorMaxValue ret_value = {0};
	return ret_value;
}

void va_isp_api_set_shutter(unsigned char val)
{

}

void va_isp_api_ircut_auto_switch(uint8_t type, uint8_t bEnable)//1:software   0: hardware
{
	//ircut µÄ¼ì²âºÍÇÐ»»	
	
	if(_isp_attr.ircut_auto_switch_enable == true)
	{
		uint32_t cur_status = light_status();
		if(_isp_attr.gpio_status_old != cur_status && ircut_edge_detect(&cur_status)){
			va_isp_ircut_switch(cur_status);
		}
	}
}

void va_isp_api_vi_flicker(uint8_t bEnable,uint8_t frequency, uint8_t mode)
{
}

uint8_t va_isp_api_get_sharpen(void)
{
	return 0;
}

void va_isp_api_set_sharpen(uint8_t val, uint8_t bManual)
{
}

void va_isp_api_set_scene_mode(uint32_t mode)
{
}

void va_isp_api_set_WB_mode(uint32_t mode)
{
}

void va_isp_api_set_ircut_control_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	_isp_attr.ircut_control_mode = mode;
}

void va_isp_api_set_ircut_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	switch(mode){
		default:
		case ISP_IRCUT_MODE_AUTO:
			_isp_attr.ircut_auto_switch_enable = true;
			break;
			
		case ISP_IRCUT_MODE_DAYLIGHT:
			_isp_attr.ircut_auto_switch_enable = false;
			va_isp_ircut_switch(ISP_GPIO_DAYLIGHT);
			break;
			
		case ISP_IRCUT_MODE_NIGHT:
			_isp_attr.ircut_auto_switch_enable = false;
			va_isp_ircut_switch(ISP_GPIO_NIGHT);
			break;
	}
}

void va_isp_api_set_WDR_enable(uint8_t bEnable)
{
}

void va_isp_api_set_WDR_strength(uint8_t val)
{
}

void va_isp_api_set_exposure_mode(uint32_t mode)
{
}

void va_isp_api_set_AEcompensation(uint8_t val)
{
}

void va_isp_api_set_denoise_enable(uint8_t bEnable)
{
}

uint8_t va_isp_api_get_denoise_strength(void)
{
	return 0;
}

void va_isp_api_set_denoise_strength(uint8_t val)
{
}

void va_isp_api_set_anti_fog_enable(uint8_t bEnable)
{
}

void va_isp_api_set_lowlight_enable(uint8_t bEnable)
{
}

void va_isp_api_set_gamma_table(uint8_t val)
{
}

void va_isp_api_set_defect_pixel_enable(uint8_t bEnable)
{
}

void va_isp_api_set_src_framerate(uint32_t val)
{
}

void va_isp_api_set_sensor_resolution(uint32_t width, uint32_t height)
{
}

void va_isp_api_get_sensor_resolution(uint32_t* ret_width, uint32_t* ret_height)
{
	if(ret_width && ret_height){
		*ret_width = 1920;
		*ret_height = 1080;
	}	
}

uint8_t va_isp_api_get_gain(void)
{
	return 0;
}

void va_isp_api_set_af_attr(stSensorAfAttr *pAfAttr)
{
}

int va_isp_api_ini_load(const char *filepath)
{
	return 0;
}

void va_isp_api_get_wdr_mode(uint8_t *bEnable)
{
}


int va_isp_api_set_ircut_switch_to_day_array(float * night_to_day_array)
{
	return 0;
}


static stVatIspAttr _isp_attr = {
	.api = {
		.SENSOR_MODEL_GET=va_isp_api_get_sensor_model,
		.MIRROR_FLIP_SET=va_isp_api_mirror_flip,
		.HUE_SET=va_isp_api_set_hue,
		.CONTRAST_SET=va_isp_api_set_contrast,
		.BRIGHTNESS_SET=va_isp_api_set_brightness,
		.SATURATION_SET=va_isp_api_set_saturation,
		.LIGHT_MODE_SET=va_isp_api_light_mode,
		.TEST_MODE_SET=va_isp_api_test_mode,
		.COLOR_MODE_SET=va_isp_api_color_mode,
		.REG_READ=va_isp_api_reg_read,
	 	.REG_WRITE=va_isp_api_reg_write,
		.SPEC_RED_WRITE=va_isp_api_spec_reg_write,
		.SPEC_REG_READ=va_isp_api_spec_reg_read,
		.GET_COLOR_MAX_VALUE=va_isp_api_get_color_max_value,
		.SHUTTER_SET=va_isp_api_set_shutter,
		.IRCUT_AUTO_SWITCH=va_isp_api_ircut_auto_switch,
		.VI_FLICKER=va_isp_api_vi_flicker,
		.SHARPEN_SET=va_isp_api_set_sharpen,
		.SHARPEN_GET=va_isp_api_get_sharpen,
		.SCENE_MODE_SET=va_isp_api_set_scene_mode,
		.WB_MODE_SET=va_isp_api_set_WB_mode,
		.IRCUT_CONTROL_MODE_SET=va_isp_api_set_ircut_control_mode,
		.IRCUT_MODE_SET=va_isp_api_set_ircut_mode,
		.WDR_MODE_ENABLE=va_isp_api_set_WDR_enable,
		.WDR_STRENGTH_SET=va_isp_api_set_WDR_strength,
		.EXPOSURE_MODE_SET=va_isp_api_set_exposure_mode,
		.AE_COMPENSATION_SET=va_isp_api_set_AEcompensation,
		.DENOISE_ENABLE=va_isp_api_set_denoise_enable,
		.DENOISE_STRENGTH_SET=va_isp_api_set_denoise_strength,
		.DENOISE_STRENGTH_GET=va_isp_api_get_denoise_strength,
		.ANTI_FOG_ENABLE=va_isp_api_set_anti_fog_enable,
		.LOWLIGHT_ENABLE=va_isp_api_set_lowlight_enable,
		.GAMMA_TABLE_SET=va_isp_api_set_gamma_table,
		.DEFECT_PIXEL_ENABLE=va_isp_api_set_defect_pixel_enable,
		.SRC_FRAMERATE_SET=va_isp_api_set_src_framerate,
		.SENSOR_RESOLUTION_GET=va_isp_api_get_sensor_resolution,
		.SENSOR_RESOLUTION_SET=va_isp_api_set_sensor_resolution,
		.GAIN_GET=va_isp_api_get_gain,
		.AF_CALLBACK_SET = va_isp_api_set_af_attr,
		.INI_LOAD=va_isp_api_ini_load,
		.WDR_MODE_GET= va_isp_api_get_wdr_mode,		
		.IRCUT_SWITCH_TO_DAY_ARRAY_SET = va_isp_api_set_ircut_switch_to_day_array,
	},
	.sensor_type = SENSOR_MODEL_APTINA_AR0330,
	.saturation = 25,
};

static emSENSOR_MODEL va_isp_api_set_sensor_model()
{
    char type[8];
    FILE *file = NULL;
    file = fopen(SENSOR_TYPE_FILE, "rb");
    if(file == NULL) {
        printf("Err:read sensor type file fail!\n");
        return -1;
    }
    fread(type, sizeof(type), 1, file);
    fclose(file);
    if(!strncmp(type, "SC2045", 6)) {
        return SENSOR_MODEL_SMARTSENS_SC2045;

    }
    else if(!strncmp(type, "AR0330", 6)) {
        return SENSOR_MODEL_APTINA_AR0330;

    }
    else {
        return SENSOR_MODEL_SMARTSENS_SC2045;

    }

}



int M388C_SDK_ISP_init(lpSensorApi*api,  lpBSPApi *bsp_api)
{
	va_isp_ircut_switch(ISP_GPIO_DAYLIGHT);
	*api = &_isp_attr.api;
    _isp_attr.sensor_type = va_isp_api_set_sensor_model();
	return 0;
}

int M388C_SDK_ISP_destroy()
{
	return 0;
}

int SDK_ISP_init(lpSensorApi *api, lpBSPApi *bsp_api){
	M388C_SDK_ISP_init(api, bsp_api);
	return 0;
}

int SDK_ISP_destroy()
{
	M388C_SDK_ISP_destroy();
	return 0;
}

emSENSOR_MODEL SDK_ISP_sensor_check()
{
	return va_isp_api_set_sensor_model();
}



