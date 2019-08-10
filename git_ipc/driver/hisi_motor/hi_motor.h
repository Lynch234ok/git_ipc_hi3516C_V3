#ifndef __HI_MOTRO_H_H__
#define __HI_MOTRO_H_H__

#include <linux/atomic.h>
#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/fs.h>

#include "hi_motor_user.h"
#include "hi3518a.h"

#ifdef CONFIG_ARCH_HI3518
#include "hi3518a.h"
#endif /* CONFIG_ARCH_HI3518 */


#define himotor_trace(level, fmt, args...) \
	do { \
		if (level <= HI_MOTOR_DBG_LEVEL) \
			printk(KERN_INFO "himotor:[Func:%s Line:%d]:" fmt , __func__, __LINE__, ##args); \
	} while (0)

#define HIMOTOR_DEBUG		4
#define HIMOTOR_INFO		3
#define HIMOTOR_ERR		2
#define HIMOTOR_FATAL		1

#define HI_MOTOR_DBG_LEVEL	3

extern long _hi35xx_mt_ioctl(struct file *filp, unsigned int command, unsigned long arg);

#endif
