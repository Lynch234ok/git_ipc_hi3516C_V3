
obj-m := hiether.o

hiether-y := mdio.o mii-drv.o mac.o ctrl.o glb.o net-drv.o mm.o sys.o

KERNEL_MAKE := -C /home/program/Hi3511_VSSDK_V1.1.2.3_SPC003/source/os/linux-2.6.14/

all:
	make $(KERNEL_MAKE) M=$(PWD) modules
clean:
	make $(KERNEL_MAKE) M=$(PWD) clean

