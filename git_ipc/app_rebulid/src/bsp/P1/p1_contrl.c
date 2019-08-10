
#include "p1_contrl.h"
#include "../commom/bsp_commom.h"
#include "../higpio/higpio.h"
#include <hi_type.h>
#include <hi_i2c.h>

#define CG5162TC_I2C_ADDR 	(0X20)

static int CG5162TC_i2c_read(int addr, uint16_t* ret_data)
{
	int fd = -1;
	int ret;
	const unsigned char sensor_i2c_addr	=	CG5162TC_I2C_ADDR;
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
	fd = open("/dev/hi_i2c", 0);
	if(fd<0)
	{
	    printf("Open hi_i2c error!\n");
	    return -1;
	}

	i2c_data.dev_addr = sensor_i2c_addr;
	i2c_data.reg_addr = addr;
	i2c_data.addr_byte_num = sensor_addr_byte;
	i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);
	*ret_data = i2c_data.data;
	close(fd);
	
	return 0;
}

static int CG5162TC_i2c_write(int addr, int data)
{
	int fd = -1;
	int ret;
	const unsigned char sensor_i2c_addr	=	CG5162TC_I2C_ADDR;
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;

	fd = open("/dev/hi_i2c", 0);
	if(fd<0)
	{
	    printf("Open hi_i2c error!\n");
	    return -1;
	}
    
	i2c_data.dev_addr = sensor_i2c_addr;
	i2c_data.reg_addr = addr;
	i2c_data.addr_byte_num = sensor_addr_byte;
	i2c_data.data = data;
	i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

	if (ret)
	{
	    printf("hi_i2c write faild!\n");
	    return -1;
	}

	close(fd);
	return 0;
}

static int Init_CG5162TC(void)
{
	//CG5162TC Power down
	CG5162TC_i2c_write(0x03, 0x6);
	usleep(500);
	//CG5162TC active
	CG5162TC_i2c_write(0x03, 0x4);

	//set Integration time
	CG5162TC_i2c_write(0x04, 0x13); //0x01 --> TIG 2.7ms

	//write cgain
	CG5162TC_i2c_write(0x05, 0x0f);
}

#define NIGHT_LUX		4
#define DAY_LUX			7

static int Read_CG5162TC(void)
{
	float Lux;
	static int re_val = 0;
	uint16_t cgain = 0, ch0_value = 0, ch1_value = 0, tmp_value = 0;

	//power down
	CG5162TC_i2c_read(0x05, &cgain);
	//active
	CG5162TC_i2c_read(0x20, &tmp_value);
	usleep(1000*55);
	
	//read ch0
	CG5162TC_i2c_read(0x21, &tmp_value);
	ch0_value = tmp_value;
	CG5162TC_i2c_read(0x22, &tmp_value);
	ch0_value |= (tmp_value << 8);

	//read ch1
	CG5162TC_i2c_read(0x23, &tmp_value);
	ch1_value = tmp_value;
	CG5162TC_i2c_read(0x24, &tmp_value);
	ch1_value |= (tmp_value << 8);
/*
	if (ch0_value < 65535) {

		Lux = (ch0_value - ch1_value) * (15 / cgain) * (400 / 51.3) * 0.022;
	}else{

		Lux = (ch0_value) * (15 / cgain) * (400 / 51.3) * 0.022;
	}
*/
	Lux = (ch0_value < 0xffff)? (ch0_value - ch1_value) : ch0_value;
	//printf("%s:%f-%d-%d\n", __FUNCTION__, Lux, ch0_value, ch1_value);
	
	if (Lux >= DAY_LUX) {
		
		re_val = 0;
	}else if (Lux <= NIGHT_LUX){

		re_val = 1;
	}

	return re_val;
}

/******************************************************/

static void P1_GPIO_Init(void)
{
	uint32_t reg_val = 0;

	//muxpin	init IR LED ctl pin
	BSP_write_reg(IRCUT_LED_GPIO_PINMUX_ADDR, IRCUT_LED_GPIO_PINMUX_VALUE);//GPIO0_0
	//pin dir :out
	BSP_read_reg(IRCUT_LED_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_LED_GPIO_PIN);
	BSP_write_reg(IRCUT_LED_GPIO_DIR_ADDR, reg_val);

	//muxpin	init IRCUT ctrl pin1
	BSP_write_reg(NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR, NEW_IRCUT_CTRL_GPIO_PINMUX_VALUE);//GPIO0_2
	//pin dir :out
	BSP_read_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<NEW_IRCUT_CTRL_GPIO_PIN);
	BSP_write_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);

	//muxpin	init IRCUT ctrl pin2
	BSP_write_reg(IRCUT_CTRL_GPIO_PINMUX_ADDR, IRCUT_CTRL_GPIO_PINMUX_VALUE);//GPIO0_4
	//pin dir :out
	BSP_read_reg(IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_CTRL_GPIO_PIN);
	BSP_write_reg(IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);

	//muxpin
	BSP_write_reg(IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR, IRCUT_PHOTOSWITCH_GPIO_PINMUX_VALUE);//GPIO0_6
	//pin dir :in
	BSP_read_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, &reg_val);
	reg_val &= ~(1<<IRCUT_PHOTOSWITCH_GPIO_PIN);
	BSP_write_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, reg_val);

	//muxpin	init sound pin
	BSP_write_reg(SOUND_EN_GPIO_PINMUX_ADDR, SOUND_EN_GPIO_PINMUX_VALUE);//GPIO0_3
	//pin dir :out
	BSP_read_reg(SOUND_EN_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<SOUND_EN_PIN);
	BSP_write_reg(SOUND_EN_GPIO_DIR_ADDR, reg_val);

	BSP_write_reg(KEY_GPIO_PINMUX_ADDR, KEY_GPIO_PINMUX_VALUE);//init key pinmux
	BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
	BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
	Init_CG5162TC();//init Photosensitive
}

static void P1_IRCUT_Switch(bool DNmode)
{
	if (!DNmode) {

		BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 1);//IR-CUT off
		BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 1);
		usleep(1000*150);
		//BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);
		//BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
	}else{

		BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
		BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
		usleep(1000*150);
		//BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);
		//BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
	}
}

static void P1_Speaker_Enable(bool Enable)
{
	if (Enable) {

		BSP_gpio_pin_write(SOUND_EN_GROUP, SOUND_EN_PIN, SOUND_EN_VALUE);
	}else{

		BSP_gpio_pin_write(SOUND_EN_GROUP, SOUND_EN_PIN, 1 - SOUND_EN_VALUE);
	}

	system("himm 0x20050074 0x01012424");
}

static int P1_Get_Photo_Val(void)
{
	return Read_CG5162TC();
}

static int P1_Get_Key_Val(int *val)
{
    if(val == NULL) {
        printf("%s failed!\n", __FUNCTION__);
        return -1;
    }
    memset(val, 1, KEY_MAX_NUM);
    int read_value = 1;
    int i = 0;

    if (0 == BSP_gpio_pin_read(KEY_GPIO_GROUP, KEY_GPIO_PIN)) {
		read_value = 0;
	}else{
		read_value = 1;
	}
    val[0] = read_value;

    for(i = 1; i < KEY_MAX_NUM; i++) {
        val[i] = -1;
    }

    return 0;
}

static void P1_IR_Led(bool Enable)
{
	if (!Enable) {

		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1-IRCUT_LED_VALUE);
	}else{

		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, IRCUT_LED_VALUE);
	}
}

static void P1_WHITE_LIGHT_Led(bool Enable)
{

}

static void P1_Led_Contrl(int LedID, bool EnableOne, bool EnableTwo)
{

}

static void P1_Senser_reset(void)
{
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
}

//有in和out类型
static int P1_Alarm(bool OCmode)
{

}

static int P1_RTC_Read(void* arg)
{

}

static int P1_RTC_Write(void* arg)
{

}

static void P1_Audio_set_volume_val(int ai_gain, int ai_vol, int ao_gain, int ao_vol)
{

}

static void P1_Wifi_power_enable(bool enable)
{

}

static void P1_Sd_power_enable(bool enable)
{

}

void BSP_ContrlInit(int val, int audioHwSpec, int model_name, bool ledEnabled)
{
	TJA_BSPCommom P1_cmInterface = {

		.GPIO_Init			= P1_GPIO_Init,
		.IRCUT_Switch 		= P1_IRCUT_Switch,
		.Speaker_Enable 	= P1_Speaker_Enable,
		.Get_Photo_Val 		= P1_Get_Photo_Val,
		.Get_Key_Val 		= P1_Get_Key_Val,
		.IR_Led 			= P1_IR_Led,
		.WHITE_LIGHT_Led	= P1_WHITE_LIGHT_Led,
		.Alarm 				= P1_Alarm,
		.RTC_Read 			= P1_RTC_Read,
		.RTC_Write 			= P1_RTC_Write,
		.Led_Contrl			= P1_Led_Contrl,
		.Senser_Reset		= P1_Senser_reset,
		.Audio_set_volume_val = P1_Audio_set_volume_val,
		.Wifi_power_enable  = P1_Wifi_power_enable,
		.Sd_power_enable    = P1_Sd_power_enable,
	};
	
	BSP_sysCreate();
	BSP_ContrlCreate(P1_cmInterface);
}


