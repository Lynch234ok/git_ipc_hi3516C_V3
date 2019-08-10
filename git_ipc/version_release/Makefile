#DEVICE_MODEL ?= hi3507_inc
#DEVICE_MODEL ?= hi3518a_inc
DEVICE_MODEL ?= hi3518c_inc

# Hisilicon Hi3507 inception
ifeq ($(DEVICE_MODEL),hi3507_inc)
INI_FILE := hi3507_ipcam.ini
UBOOT_ENV_TXT := u_boot_env_07.txt
ARM-STRIP := arm-hismall-linux-strip
WEB_NAME := web
endif
# Hisilicon Hi3518a inception
ifeq ($(DEVICE_MODEL),hi3518a_inc)
INI_FILE := HI3518A_ipcam.ini
UBOOT_ENV_TXT := u_boot_env_18a.txt
ARM_STRIP := arm-hisiv100nptl-linux-strip
WEB_NAME := web_18a
endif
# Hisilicon Hi3518c inception
ifeq ($(DEVICE_MODEL),hi3518c_inc)
INI_FILE := HI3518C_ipcam.ini
UBOOT_ENV_TXT := u_boot_env_18c.txt
ARM_STRIP := arm-hisiv100nptl-linux-strip
WEB_NAME := web_18c
endif
# Hisilicon Hi3516c inception
ifeq ($(DEVICE_MODEL),hi3516c_inc)
INI_FILE := HI3516C_ipcam.ini
UBOOT_ENV_TXT := u_boot_env_16c.txt
ARM_STRIP := arm-hisiv100nptl-linux-strip
WEB_NAME := web
endif

IMAGE_PATH := "$(CURDIR)/image/"
IMAGE_ROOTFS := $(IMAGE_PATH)rootfs.squash.uboot
IMAGE_UBOOTENV := $(IMAGE_PATH)u-boot-env.bin

IMAGE_ROOTFS_APP  := rootfs/root/app/ipcam_app
IMAGE_ROOTFS_DATA := rootfs/root/data
IMAGE_ROOTFS_SBIN_PATH := rootfs/sbin/



.PHONY: cp_app cp_resource
all : $(IMAGE_UBOOTENV) cp_app cp_resource $(IMAGE_ROOTFS) 

cp_app :
	cp ../app_rebulid_release/src/ipcam_app $(IMAGE_ROOTFS_APP)
	$(ARM_STRIP) $(IMAGE_ROOTFS_APP)
#	rm -f $(IMAGE_ROOTFS_SBIN_PATH)RSERIAL_NUM
#	rm -f $(IMAGE_ROOTFS_SBIN_PATH)WSERIAL_NUM
#	rm -f $(IMAGE_ROOTFS_SBIN_PATH)READ_INFO
#	ln -s $(IMAGE_ROOTFS_APP) $(IMAGE_ROOTFS_SBIN_PATH)RSERIAL_NUM
#	ln -s $(IMAGE_ROOTFS_APP) $(IMAGE_ROOTFS_SBIN_PATH)WSERIAL_NUM
#	ln -s $(IMAGE_ROOTFS_APP) $(IMAGE_ROOTFS_SBIN_PATH)READ_INFO
	
cp_resource :
	rm -rf $(IMAGE_ROOTFS_DATA)/web
	tar -h -C ../app_rebulid_release/src \
		-c web/ \
		--exclude="*svn" \
		| tar -x -C $(IMAGE_ROOTFS_DATA) -f -
#	find $(IMAGE_ROOTFS_DATA)/web -type f -name "*.*" | xargs gzip -9
	rm -rf $(IMAGE_ROOTFS_DATA)/font
	tar -h -C ../app_rebulid_release/src \
		-c font/ \
		--exclude="*svn" \
		| tar -x -C $(IMAGE_ROOTFS_DATA) -f -


include Makefile.tools
$(IMAGE_ROOTFS) : $(MK_ROM_BIN)
	@make -f Makefile.rootfs SOC_MODEL=$(DEVICE_MODEL)
	$< $(INI_FILE) $(IMAGE_PATH) ./

$(IMAGE_UBOOTENV) : $(MK_UBOOT_ENV)
	$< $(IMAGE_PATH)$(UBOOT_ENV_TXT) $(IMAGE_PATH)u_boot_env.bin 128K


