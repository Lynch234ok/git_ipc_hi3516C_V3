#include <asm/uaccess.h>
#include "hi_motor_user.h"
#include "hi_motor.h"

long _hi35xx_mt_ioctl(struct file *filp, unsigned int command, unsigned long arg)
{
	put_user(AfTimerTick, (unsigned long *)arg);

	return 0;
}
