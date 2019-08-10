KERNEL_BUILD_PATH ?= 
PLATFORM_NAME ?=

obj-m := hi3515.o gpio_i2c.o
hi3515-y := hi3515_drv.o
gpio_i2c-y := gpio_i2c_drv.o

.PHOHY: default clean

default:
	@make -C $(KERNEL_BUILD_PATH) M=$(PWD) modules
clean:
	@make -C $(KERNEL_BUILD_PATH) M=$(PWD) clean

