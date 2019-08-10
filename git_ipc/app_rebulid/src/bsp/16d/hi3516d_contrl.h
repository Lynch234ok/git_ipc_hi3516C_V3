
#ifndef _HI3516D_COMMOM_CONTRL_H
#define _HI3516D_COMMOM_CONTRL_H

#include <stdlib.h>
#include <stdio.h>
//#include <ctype.h>
//#include <sys/types.h>
//#include <string.h> 
//#include <mpi_sys.h>

#include <stdbool.h> 

#ifdef __cplusplus 
	extern "C" { 
#endif

#define DAYLIGHT								0
#define NIGHTLIGHT								1
#define IRLED_DAY								0
#define IRLED_NIGHT								1


//ir-cut led :GPIO7_7
#define IRCUT_LED_GPIO_PINMUX_ADDR 				0x200f0064
#define IRCUT_LED_GPIO_PINMUX_VALUE				0
#define IRCUT_LED_GPIO_DIR_ADDR 				0x20140400
#define IRCUT_LED_GPIO_DATA_ADDR 				0x201403fc
#define IRCUT_LED_GPIO_PIN 						5
#define IRCUT_LED_GPIO_GROUP 					8
#define IRCUT_LED_VALUE							1

//new hardware ir-cut control :GPIO8_0    0x201c0400
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 		0x200f0068
#define NEW_IRCUT_CTRL_GPIO_PINMUX_VALUE		0
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 			0x20140400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 			0x201403fc
#define NEW_IRCUT_CTRL_GPIO_PIN 				6
#define NEW_IRCUT_CTRL_GPIO_GROUP 				8

//old hardware ir-cut control :GPIO8_1
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 			0x200f006c
#define IRCUT_CTRL_GPIO_PINMUX_VALUE			0
#define IRCUT_CTRL_GPIO_DIR_ADDR 				0x20140400
#define IRCUT_CTRL_GPIO_DATA_ADDR 				0x201403fc
#define IRCUT_CTRL_GPIO_PIN 					7
#define IRCUT_CTRL_GPIO_GROUP 					8

//ir-cut photoswitch read:GPIO7_6   0x201B0400
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 		0x200f0060
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_VALUE 	0
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 		0x20140400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 		0x201403fc
#define IRCUT_PHOTOSWITCH_GPIO_PIN 				4
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 			8

#define SOUND_EN_GPIO_PINMUX_ADDR               0x201200c8
#define SOUND_AO_EN_PIN                         29
#define SOUND_AI_EN_PIN                         31
#define SOUND_EN_VALUE                          0
#define SOUND_VOL_CTRL_ADDR                     0x201200d8
#define SOUND_VOL_CTRL_BIT                      24
#define OLD_SOUND_VOL_VALUE                     0xe
#define NEW_SOUND_VOL_VALUE                     0x2
#define SOUND_PLUS_CTRL_ADDR                    0x201200c8
#define SOUND_PLUS_CTRL_BIT                     8
#define SOUND_PLUS_CTRL_VALUE                   0x5c

#define AO_POWER_GPIO_PINMUX_ADDR				0x200f00a8
#define AO_POWER_GPIO_PINMUX_VALUE  			0
#define AO_POWER_GPIO_GROUP						10
#define AO_POWER_GPIO_PIN						6

#define KEY_GPIO_PINMUX_ADDR					0x200f0078
#define KEY_GPIO_PINMUX_VALUE  					0
#define KEY_GPIO_GROUP							9
#define KEY_GPIO_PIN							2

#define KEY_REC_GPIO_PINMUX_ADDR				0x200f0130
#define KEY_REC_GPIO_PINMUX_VALUE  				1
#define KEY_REC_GPIO_GROUP						3
#define KEY_REC_GPIO_PIN						1

#define LED_GPIO_PINMUX_ADDR					0x200f0080
#define LED_GPIO_PINMUX_VALUE  					0
#define LED_GPIO_GROUP							9
#define LED_GPIO_PIN							4

#define LED_REC_GPIO_PINMUX_ADDR				0x200f012c
#define LED_REC_GPIO_PINMUX_VALUE  				1
#define LED_REC_GPIO_GROUP						3
#define LED_REC_GPIO_PIN						0

#define SENSOR_RESET_GPIO_GOUP					0
#define SENSOR_RESET_GPIO_PIN					0
#define SENSOR_RESET_GPIO_VALUE					0

#define WIFI_POWER_GPIO_PINMUX_ADDR				0x200f00ac
#define WIFI_POWER_GPIO_PINMUX_VALUE  			0
#define WIFI_POWER_GPIO_DIR_ADDR				0x201e0400
#define WIFI_POWER_GPIO_GROUP					10
#define WIFI_POWER_GPIO_PIN						7

#define SD_POWER_GPIO_PINMUX_ADDR				0x200f00b4
#define SD_POWER_GPIO_PINMUX_VALUE  			0
#define SD_POWER_GPIO_GROUP					    2
#define SD_POWER_GPIO_PIN						1

#ifdef __cplusplus 
	}
#endif

#endif

