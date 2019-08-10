
#ifndef _V3_CONTRL_H
#define _V3_CONTRL_H

#include <stdlib.h>
#include <stdio.h>

#include <stdbool.h> 

#ifdef __cplusplus 
	extern "C" { 
#endif

#define DAYLIGHT								0
#define NIGHTLIGHT								1
#define IRLED_DAY								0
#define IRLED_NIGHT								1


#if defined(HI3516E_V1)
//new hardware ir-cut control :GPIO0_0    16ev100
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 		0x120400F8
#define NEW_IRCUT_CTRL_GPIO_PINMUX_VALUE		0
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 			0x12140400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 			0x121403fc
#define NEW_IRCUT_CTRL_GPIO_PIN 				1
#define NEW_IRCUT_CTRL_GPIO_GROUP 				0

//old hardware ir-cut control :GPIO0_1
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 			0x120400F4
#define IRCUT_CTRL_GPIO_PINMUX_VALUE			0
#define IRCUT_CTRL_GPIO_DIR_ADDR 				0x12140400
#define IRCUT_CTRL_GPIO_DATA_ADDR 				0x121403fc
#define IRCUT_CTRL_GPIO_PIN 					0
#define IRCUT_CTRL_GPIO_GROUP 					0

//ir-cut photoswitch read:GPIO8_2   16EV100
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 		0x12040104
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_VALUE 	0
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 		0x12148400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 		0x121483fc
#define IRCUT_PHOTOSWITCH_GPIO_PIN 				2
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 			8

#if 0
//ir-cut led :GPIO6_7   16EV100
#define IRCUT_LED_GPIO_PINMUX_ADDR 				0x1204000C
#define IRCUT_LED_GPIO_PINMUX_VALUE				0
#define IRCUT_LED_GPIO_DIR_ADDR 				0x12146400
#define IRCUT_LED_GPIO_DATA_ADDR 				0x121463fc
#define IRCUT_LED_GPIO_PIN 						7
#define IRCUT_LED_GPIO_GROUP 					6
#define IRCUT_LED_VALUE							1
#endif

//ir-cut white_light :GPIO6_6   16EV100
#define WHITE_LED_GPIO_PINMUX_ADDR				0x12040008
#define WHITE_LED_GPIO_PINMUX_VALUE				0
#define WHITE_LED_GPIO_DIR_ADDR					0x12146400
#define WHITE_LED_GPIO_DATA_ADDR				0x121463fc
#define WHITE_LED_GPIO_PIN						6
#define WHITE_LED_GPIO_GROUP					6
#define WHITE_LED_VALUE							1

#define WHITE_LED_GPIO6_7_PINMUX_ADDR           0x1204000C
#define WHITE_LED_GPIO6_7_PINMUX_VALUE          0
#define WHITE_LED_GPIO6_7_DIR_ADDR              0x12146400
#define WHITE_LED_GPIO6_7_DATA_ADDR             0x121463fc
#define WHITE_LED_GPIO6_7_PIN                   7
#define WHITE_LED_GPIO6_7_GROUP                 6
#define WHITE_LED_GPIO6_7_VALUE                 1

#else
//old hardware ir-cut control :GPIO6_6	   16CV300
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 			0x12040008
#define IRCUT_CTRL_GPIO_PINMUX_VALUE			0
#define IRCUT_CTRL_GPIO_DIR_ADDR 				0x12140400
#define IRCUT_CTRL_GPIO_DATA_ADDR 				0x121403fc
#define IRCUT_CTRL_GPIO_PIN 					6
#define IRCUT_CTRL_GPIO_GROUP 					6

//new hardware ir-cut control :GPIO6_5    16CV300
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 		0x12040004
#define NEW_IRCUT_CTRL_GPIO_PINMUX_VALUE		0
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 			0x12140400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 			0x121403fc
#define NEW_IRCUT_CTRL_GPIO_PIN 				5
#define NEW_IRCUT_CTRL_GPIO_GROUP 				6

//ir-cut photoswitch read:GPIO8_0   16CV300
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 		0x120400FC
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_VALUE 	0
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 		0x12140400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 		0x121403fc
#define IRCUT_PHOTOSWITCH_GPIO_PIN 				0
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 			8

//ir-cut led :GPIO8_1   16CV300
#define IRCUT_LED_GPIO_PINMUX_ADDR 				0x12040100
#define IRCUT_LED_GPIO_PINMUX_VALUE				0
#define IRCUT_LED_GPIO_DIR_ADDR 				0x12140400
#define IRCUT_LED_GPIO_DATA_ADDR 				0x121403fc
#define IRCUT_LED_GPIO_PIN 						1
#define IRCUT_LED_GPIO_GROUP 					8
#define IRCUT_LED_VALUE							0

//ir-cut white_light :GPIO8_1   16CV300
#define WHITE_LED_GPIO_PINMUX_ADDR				0x12040100
#define WHITE_LED_GPIO_PINMUX_VALUE				0
#define WHITE_LED_GPIO_DIR_ADDR					0x12140400
#define WHITE_LED_GPIO_DATA_ADDR				0x121403fc
#define WHITE_LED_GPIO_PIN						1
#define WHITE_LED_GPIO_GROUP					8
#define WHITE_LED_VALUE							0

#endif


#if defined(HI3516E_V1)
//GPIO8_0
#define SOUND_EN_GPIO_PINMUX_ADDR				0x120400FC
#define SOUND_EN_GPIO_PINMUX_VALUE				0
#define SOUND_EN_GPIO_DIR_ADDR					0x12148400
#define SOUND_EN_GPIO_DATA_ADDR					0x121483fc
#define SOUND_EN_GPIO_GROUP						8
#define SOUND_EN_GPIO_PIN						0

#define SOUND_EN_ADDR				            0x113200B4
#define SOUND_AO_EN_PIN							11
#define SOUND_AI_EN_PIN							2
#define SOUND_EN_VALUE							0
#define SOUND_VOL_CTRL_ADDR                     0x113200D8  //ADCL Contrl 
#define SOUND_VOL_CTRL_BIT                      24
#define OLD_SOUND_VOL_VALUE                     0x16
#define NEW_SOUND_VOL_VALUE                     0x1e		//0db
#define SOUND_PLUS_CTRL_ADDR                    0x113200B8 // Line_in_gain
#define SOUND_PLUS_CTRL_BIT                     8
#define SOUND_PLUS_CTRL_VALUE                   0x47  		//34db
#define SOUND_PLUS_CTRL_VALUE_LARGE             0x4f		//50db
#define SOUND_CHN_SET_BIT						24
#define SOUND_CHN_SET_VALUE						0x33

#define SOUND_AO_PLUS_CTRL_ADDR                 0x113200D4
#define SOUND_AO_PLUS_CTRL_BIT                  24
#define SOUND_AO_PLUS_CTRL_VALUE                0x14
#else

/*
#define SOUND_EN_GPIO_PINMUX_ADDR				0x200f0068
#define SOUND_EN_GPIO_PINMUX_VALUE				0
#define SOUND_EN_GPIO_DIR_ADDR					0x20180400
#define SOUND_EN_GPIO_DATA_ADDR					0x201803fc
#define SOUND_EN_GPIO_GROUP						4
#define SOUND_EN_GPIO_PIN						5
*/
#define SOUND_EN_ADDR				            0x113200B4
#define SOUND_AO_EN_PIN							11
#define SOUND_AI_EN_PIN							2
#define SOUND_EN_VALUE							0
#define SOUND_VOL_CTRL_ADDR                     0x113200D8  //ADCL Contrl 
#define SOUND_VOL_CTRL_BIT                      24
#define OLD_SOUND_VOL_VALUE                     0x16
#define NEW_SOUND_VOL_VALUE                     0x1e		//0db
#define SOUND_PLUS_CTRL_ADDR                    0x113200B8 // Line_in_gain
#define SOUND_PLUS_CTRL_BIT                     8
#define SOUND_PLUS_CTRL_VALUE                   0x47  		//34db
#define SOUND_PLUS_CTRL_VALUE_LARGE             0x4f		//50db
#define SOUND_CHN_SET_BIT						24
#define SOUND_CHN_SET_VALUE						0x33

#endif

#define KEY_GPIO_PINMUX_ADDR					0x1204008C
#define KEY_GPIO_PINMUX_VALUE  					0
#define KEY_GPIO_DIR_ADDR    					0x12143400
#define KEY_GPIO_PULL_UP_ADDR    				0x120408AC
#define KEY_GPIO_PULL_UP_BIT					1
#define KEY_GPIO_GROUP							3
#define KEY_GPIO_PIN							2

//LED  16CV300
#define LED_GPIO_PINMUX_ADDR					0x12040094
#define LED_GPIO_PINMUX_VALUE  					0
#define LED_GPIO_DIR_ADDR    					0x12143400
#define LED_GPIO_GROUP							3
#define LED_GPIO_PIN							0

//sensor reset 16cv300
#define SENSOR_RESET_GPIO_PINMUX_ADDR 			0x12040038
#define SENSOR_RESET_GPIO_PINMUX_VALUE 			0
#define SENSOR_RESET_GPIO_GOUP					0
#define SENSOR_RESET_GPIO_PIN					7
#define SENSOR_RESET_GPIO_VALUE					0

#define WIFI_POWER_GPIO_DIR_ADDR                0x12140400
#define WIFI_POWER_GPIO_GROUP                   0
#define WIFI_POWER_GPIO_PIN                     2

#define SD_POWER_GPIO_PINMUX_ADDR               0x1204006C
#define SD_POWER_GPIO_PINMUX_VALUE              0
#define SD_POWER_GPIO_DIR_ADDR                  0x12144400
#define SD_POWER_GPIO_GROUP                     4
#define SD_POWER_GPIO_PIN                       0

#ifdef __cplusplus 
	}
#endif

#endif

