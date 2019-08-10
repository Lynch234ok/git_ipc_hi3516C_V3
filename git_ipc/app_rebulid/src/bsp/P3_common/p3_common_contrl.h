
#ifndef _V2_CONTRL_H
#define _V2_CONTRL_H

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


//ir-cut led :GPIO7_7
#define IRCUT_LED_GPIO_PINMUX_ADDR 				0x200f00fc
#define IRCUT_LED_GPIO_PINMUX_VALUE				1
#define IRCUT_LED_GPIO_DIR_ADDR 				0x201b0400
#define IRCUT_LED_GPIO_DATA_ADDR 				0x201b03fc
#define IRCUT_LED_GPIO_GROUP 					7
#define IRCUT_LED_GPIO_PIN 						7
#define IRCUT_LED_VALUE							1

//ir-cut photoswitch read:GPIO7_6   0x201B0400
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 		0x200f00f8
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_VALUE 	1
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 		0x201b0400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 		0x201b03fc
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 			7
#define IRCUT_PHOTOSWITCH_GPIO_PIN 				6

//new hardware ir-cut control :GPIO8_0    0x201c0400
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 		0x200f0100
#define NEW_IRCUT_CTRL_GPIO_PINMUX_VALUE		1
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 			0x201c0400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 			0x201c03fc
#define NEW_IRCUT_CTRL_GPIO_PIN 				0
#define NEW_IRCUT_CTRL_GPIO_GROUP 				8

//old hardware ir-cut control :GPIO8_1
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 			0x200f0104
#define IRCUT_CTRL_GPIO_PINMUX_VALUE			1
#define IRCUT_CTRL_GPIO_DIR_ADDR 				0x201c0400
#define IRCUT_CTRL_GPIO_DATA_ADDR 				0x201c03fc
#define IRCUT_CTRL_GPIO_PIN 					1
#define IRCUT_CTRL_GPIO_GROUP 					8

#define SOUND_EN_GPIO_PINMUX_ADDR				0x200f0068
#define SOUND_EN_GPIO_PINMUX_VALUE				0
#define SOUND_EN_GPIO_DIR_ADDR					0x20180400
#define SOUND_EN_GPIO_DATA_ADDR					0x201803fc
#define SOUND_EN_GROUP							4
#define SOUND_EN_PIN							5
#define SOUND_EN_GPIO_PINMUX_ADDR_1				0x201200c8
#define SOUND_AO_EN_PIN							29
#define SOUND_AI_EN_PIN							31
#define SOUND_EN_VALUE							0
#define SOUND_VOL_CTRL_ADDR                     0x201200d8
#define SOUND_VOL_CTRL_BIT                      24
#define SOUND_VOL_VALUE                         0x16
#define SOUND_PLUS_CTRL_ADDR                    0x201200c8
#define SOUND_PLUS_CTRL_BIT                     10
#define SOUND_PLUS_CTRL_VALUE                   0x8

#define KEY_GPIO_PINMUX_ADDR					0x200f00cc
#define KEY_GPIO_PINMUX_VALUE  					0
#define KEY_GPIO_DIR_ADDR    					0x201a0400
#define KEY_GPIO_PULL_UP_ADDR    				0x200f08f8
#define KEY_GPIO_PULL_UP_BIT					8
#define KEY_GPIO_GROUP							6
#define KEY_GPIO_PIN							3

#define LED_GPIO_PINMUX_ADDR					0x200f00d0
#define LED_GPIO_PINMUX_VALUE  					0
#define LED_GPIO_GROUP							6
#define LED_GPIO_PIN							4

#define SENSOR_RESET_GPIO_GOUP					0
#define SENSOR_RESET_GPIO_PIN					5
#define SENSOR_RESET_GPIO_VALUE					0

#ifdef __cplusplus
	}
#endif

#endif

