
IMAGE_PATH := "$(CURDIR)/image/"
IMAGE_ROOTFS := $(IMAGE_PATH)rootfs.squash.uboot
IMAGE_UBOOTENV := $(IMAGE_PATH)u-boot-env.bin



all : $(IMAGE_UBOOTENV) $(IMAGE_ROOTFS) 

include Makefile.tools
$(IMAGE_ROOTFS) : $(MK_ROM_BIN)
	@make -f Makefile.rootfs
	$< hi3507_ipcam.ini $(IMAGE_PATH) ./

$(IMAGE_UBOOTENV) : $(MK_UBOOT_ENV)
	$< $(IMAGE_PATH)u_boot_env.txt $(IMAGE_PATH)u_boot_env.bin 128K


