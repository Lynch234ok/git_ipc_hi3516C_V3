#ifndef __DRV_HEADERS_H__
#define __DRV_HEADERS_H__

#include "linux/version.h"

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,24) //rough judge
#include <asm/hardware.h>
#include <asm/semaphore.h>
#include <linux/smp_lock.h>
#ifdef MODULE
#include <linux/compile.h>
#endif //MODULE
#endif

#include <asm/delay.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>

#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/workqueue.h>

#ifdef  __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
#define ioctl unlocked_ioctl
#define DEFINE_IOCTL(k,x,y,z) long k (struct file *x, unsigned y, unsigned long z)
#else
#define DEFINE_IOCTL(k,x,y,z) int  k (struct inode *inode, \
                                      struct file *x, unsigned y, unsigned long z)
#endif
#endif //__KERNEL__

#endif //__DRV_HEADERS_H__
