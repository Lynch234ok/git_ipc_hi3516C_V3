#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/fs.h>

#include "hi_motor.h"
#include "hi_motor_user.h"

struct hi35xx_mt_attr g_hi35xx_mt_attr[HI35XX_MOTOR_NUM];

static int hi35xx_mt_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int hi35xx_mt_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long hi35xx_mt_ioctl(struct file *filp, unsigned int command, unsigned long arg)
{
	return _hi35xx_mt_ioctl(filp, command, arg);
}


static const struct file_operations hi35xx_mt_fops = {
	.owner = THIS_MODULE,
	.open = hi35xx_mt_open,
	.release = hi35xx_mt_release,
	.unlocked_ioctl = hi35xx_mt_ioctl,
};

static struct miscdevice hi35xx_mt_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hi35xx_motor",
	.fops = &hi35xx_mt_fops,
};

static __init int hi_motor_init(void)
{
	int ret = 0;

	ret = misc_register(&hi35xx_mt_miscdev);
	if (ret) {
		printk(KERN_ERR "misc_register %s failed.\n", hi35xx_mt_miscdev.name);
		goto error0;
	}

	ret = hi35xx_motor_init();
	if (unlikely(ret)) {
		himotor_trace(HIMOTOR_FATAL, "init himotor failed!\n");
		goto error1;
	}

	return 0;

error1:
	misc_deregister(&hi35xx_mt_miscdev);
error0:
	return ret;
}

static void __exit hi_motor_exit(void)
{
	hi35xx_motor_exit();
	misc_deregister(&hi35xx_mt_miscdev);
	return;
}

module_init(hi_motor_init);
module_exit(hi_motor_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Hisilicon_BVT_OSDRV");
