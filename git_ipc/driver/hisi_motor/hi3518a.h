#ifndef __HI_3518A_MOTOR_H_H__
#define __HI_3518A_MOTOR_H_H__

#include <linux/io.h>
#include "hi_motor_user.h"

#define SYS_CTRL_BASE		IO_ADDRESS(0x20050000)
#define SC_CTRL_OFFSET		0x0
#define TIMEREN2OV		(0x1 << 20)
#define TIMEREN2SEL		(0x1 << 19)

#define HI3518A_TIMER2_BASE	IO_ADDRESS(0x20010000)
#define HI3518A_TIMER2_LOAD	0x0
#define HI3518A_TIMER2_VALUE	0x4
#define HI3518A_TIMER2_CONTROL	0x8
#define HI3518A_TIMER2_INTCLR	0xc
#define HI3518A_TIMER2_RIS	0x10
#define HI3518A_TIMER2_MIS	0x14

#define CFG_TIMER_ENABLE	(1 << 7)
#define CFG_TIMER_PERIODIC	(1 << 6)
#define CFG_TIMER_INTMASK	(1 << 5)
#define CFG_TIMER_32BIT		(1 << 1)
#define CFG_TIMER_ONESHOT	(1 << 0)

#define CFG_TIMER_CONTROL	(CFG_TIMER_ENABLE | CFG_TIMER_PERIODIC | CFG_TIMER_INTMASK | CFG_TIMER_32BIT)

//#define HI3518A_TIMER2_IRQ	4
#define HI3518A_TIMER2_IRQ	36
#define HI3518A_TIMER2_NAME	"hi3518a_timer2"

/*
 * may be you should define those stuff if you want to reuse those code for other chips
 */
#define HI35XX_TIMER_RATE	3000000
#if 0
#define HI35XX_MIN_RESOLUTION	100	/* 10 ms */
#define HI35XX_MIN_RESOLUTION	200	/* 5 ms */
#define HI35XX_MIN_RESOLUTION	250	/* 4 ms */
#define HI35XX_MIN_RESOLUTION	500	/* 2 ms */
#define HI35XX_MIN_RESOLUTION	1000	/* 1 ms */
#endif
#define HI35XX_MIN_RESOLUTION	4000	/* 250 us */
#define HI35XX_TIMER_COUNTER	(HI35XX_TIMER_RATE / HI35XX_MIN_RESOLUTION)
#define HI35XX_MOTOR_NUM	2
#define HI35XX_MOTOR_PULSE_NUM	4

extern volatile unsigned long AfTimerTick;
extern int hi35xx_motor_init(void);
extern int hi35xx_motor_exit(void);

#endif
