
#ifndef _P1_CONTRL_H
#define _P1_CONTRL_H

#include <stdlib.h>
#include <stdio.h>

#include <stdbool.h> 

#ifdef __cplusplus 
	extern "C" { 
#endif

//ir-cut led
#define IRCUT_LED_GPIO_PINMUX_ADDR 				0x200f00bc
#define IRCUT_LED_GPIO_PINMUX_VALUE				0
#define IRCUT_LED_GPIO_DIR_ADDR 				0x20190400
#define IRCUT_LED_GPIO_DATA_ADDR 				0x201903fc
#define IRCUT_LED_GPIO_PIN 						2
#define IRCUT_LED_GPIO_GROUP 					5
#define IRCUT_LED_VALUE							0

//new hardware ir-cut control :GPIO9_2
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 		0x200f0128
#define NEW_IRCUT_CTRL_GPIO_PINMUX_VALUE		0
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 			0x20140400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 			0x201403fc
#define NEW_IRCUT_CTRL_GPIO_PIN 				2
#define NEW_IRCUT_CTRL_GPIO_GROUP 				0

//old hardware ir-cut control :GPIO9_3
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 			0x200f0020
#define IRCUT_CTRL_GPIO_PINMUX_VALUE			0
#define IRCUT_CTRL_GPIO_DIR_ADDR 				0x20160400
#define IRCUT_CTRL_GPIO_DATA_ADDR 				0x201603fc
#define IRCUT_CTRL_GPIO_PIN 					2
#define IRCUT_CTRL_GPIO_GROUP 					2

//ir-cut photoswitch read:GPIO0_6
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 		0x200f0138
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_VALUE		0
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 		0x20140400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 		0x201403fc
#define IRCUT_PHOTOSWITCH_GPIO_PIN 				6
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 			0

#define SOUND_EN_GPIO_PINMUX_ADDR				0x200f012c
#define SOUND_EN_GPIO_PINMUX_VALUE				0
#define SOUND_EN_GPIO_DIR_ADDR					0x20140400
#define SOUND_EN_GPIO_DATA_ADDR					0x201403fc
#define SOUND_EN_PIN							4
#define SOUND_EN_GROUP							0
#define SOUND_EN_VALUE							0

#define KEY_GPIO_GROUP							0
#define KEY_GPIO_PIN							0
#define KEY_GPIO_PINMUX_ADDR					0x200f0120
#define KEY_GPIO_PINMUX_VALUE  					0

#define DAYLIGHT								0
#define NIGHTLIGHT								1
#define IRLED_DAY								0
#define IRLED_NIGHT								1

#define SENSOR_RESET_GPIO_GOUP					0		
#define SENSOR_RESET_GPIO_PIN					5			
#define SENSOR_RESET_GPIO_VALUE					0			

#ifdef __cplusplus 
	}
#endif

#endif

