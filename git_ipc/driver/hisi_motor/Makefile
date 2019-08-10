ARCH := arm
#CROSS_COMPILE := arm-hisiv100nptl-linux-
CROSS_COMPILE := arm-hisiv300-linux-
#KBUILD_EXTRA_SYMBOLS = /home/green/share/driver/linux-3.4.y/Module.symvers
KERNELDIR ?= /home/green/share/driver/linux-3.4.y/
PWD  := $(shell pwd)

obj-m := hi_motor.o
hi_motor-y += himt.o hi3518a.o hi_motor_ioctl.o

default:
	$(MAKE) -C $(KERNELDIR)  M=$(PWD) modules
	$(CROSS_COMPILE)gcc -o test_mt test_mt.c
clean:
	@make -C $(KERNELDIR) M=$(PWD) clean
	rm hi_motor.ko -rf;
	rm test_mt;
	rm *.mod.o *.o -rf;
	rm *.order  *.symvers -rf;
	rm *.mod.c -rf;

