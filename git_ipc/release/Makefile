include ../app_rebulid/config.ini

ifeq (WIFI,$(findstring WIFI, $(FUNCTION_DEF)))
IMAGE_APP_WIFI_DIR_SRC := $(IMAGE_APP_HOME_DIR_SRC)/wifi_tools/$(CROSS)
endif

ifeq (TFCARD,$(findstring TFCARD, $(FUNCTION_DEF)))
IMAGE_APP_SDCARD_DIR_SRC := $(IMAGE_APP_HOME_DIR_SRC)/sdcard_tools/$(CROSS)
endif

ifeq (DIAL,$(findstring DIAL, $(FUNCTION_DEF)))
IMAGE_APP_DIAL_TOOLS_BINS := $(IMAGE_APP_DIAL_TOOLS_DIR)/bin/*
IMAGE_APP_DIAL_TOOLS_PPP := $(IMAGE_APP_DIAL_TOOLS_DIR)/ppp
IMAGE_APP_DIAL_TOOLS_IPUP := $(IMAGE_APP_DIAL_TOOLS_DIR)/ip-up
endif

.PHONY: app_sync
all : $(IMAGE_UBOOTENV) app_sync $(IMAGE_ROOTFS) 

app_sync:
	#in order to exit Makefile if rootfs is not exist
	if [ ! -e $(MODEL_ROOTFS) ];then touch $(MODEL_ROOTFS)/exist;fi
	rm -rf resource/
	rm -Rf $(IMAGE_APP_HOME_DIR_DST)
	cp -a $(IMAGE_APP_RESOURCE_DIR_SRC) $(IMAGE_APP_RESOURCE_DIR_DST)
	mkdir -p $(IMAGE_APP_HOME_DIR_DST)
	mkdir -p $(IMAGE_APP_HOME_DIR_DST)/resource/Px
	cp -a $(IMAGE_APP_EXEC_SRC) $(IMAGE_APP_EXEC_DST)
	cp -a $(IMAGE_APP_SHELL_SRC) $(IMAGE_APP_SHELL_DST)
	cp -a $(IMAGE_APP_FLASH_MAP_SRC) $(IMAGE_APP_FLASH_MAP_DST)
	cp -a $(IMAGE_APP_NETSDK_DIR_SRC_MODEL1) $(IMAGE_APP_NETSDK_DIR_DST_MODEL1)
	cp -a $(IMAGE_APP_NETSDK_DIR_SRC_MODEL2) $(IMAGE_APP_NETSDK_DIR_DST_MODEL2)
	cp -a $(IMAGE_APP_NETSDK_DIR_SRC_MODEL1) $(IMAGE_APP_NETSDK_DIR_RESOURCE_DST_MODEL1)
	cp -a $(IMAGE_APP_NETSDK_DIR_SRC_MODEL2) $(IMAGE_APP_NETSDK_DIR_RESOURCE_DST_MODEL2)
	cp -a $(IMAGE_LINUX_MODULE_SRC)  $(IMAGE_LINUX_MODULE_DST)
	cp -a $(IMAGE_APP_WIFI_DIR_SRC)  $(IMAGE_APP_WIFI_DIR_DST)
	cp -a $(IMAGE_APP_SDCARD_DIR_SRC)  $(IMAGE_APP_SDCARD_DIR_DST)
	cp -a $(IMAGE_APP_JA_TOOLS_DIR_SRC)  $(IMAGE_APP_JA_TOOLS_DIR_DST)
	# for ppp
ifeq (DIAL,$(findstring DIAL, $(FUNCTION_DEF)))
	rm -rf $(MODEL_ROOTFS)/usr/bin/chat
	mkdir -p $(MODEL_ROOTFS)/etc/ppp/
	cp -a $(IMAGE_APP_DIAL_TOOLS_PPP) $(IMAGE_APP_SHELL_DST)/
	cp -a $(IMAGE_APP_DIAL_TOOLS_BINS) $(MODEL_ROOTFS)/usr/sbin/
	cp -a $(IMAGE_APP_DIAL_TOOLS_IPUP) $(MODEL_ROOTFS)/etc/ppp/
endif

ifeq ($(strip $(PRODUCT_NAME)), $(filter $(PRODUCT_NAME), P3  C3-S-NE P6 F5))
	mkdir -p $(IMAGE_APP_ISP_CFG_INI_DST)
	cp -a $(IMAGE_APP_PX_ISP_CFG_INI_SRC)  $(IMAGE_APP_PX_ISP_CFG_INI_DST)
	cp -a $(IMAGE_APP_CX_ISP_CFG_INI_SRC)  $(IMAGE_APP_CX_ISP_CFG_INI_DST)
	mkdir -p $(IMAGE_APP_MODEL_CONF_DIR_DST)
	cp -ap $(IMAGE_APP_PX_MODEL_CONF_DIR_SRC)  $(IMAGE_APP_PX_MODEL_CONF_DIR_DST)
	cp -ap $(IMAGE_APP_CX_MODEL_CONF_DIR_SRC)  $(IMAGE_APP_CX_MODEL_CONF_DIR_DST)
else ifeq ($(strip $(PRODUCT_NAME)), $(filter $(PRODUCT_NAME), P2_720 ))
	mkdir -p $(IMAGE_APP_ISP_CFG_INI_DST)
	mkdir -p $(IMAGE_APP_MODEL_CONF_DIR_DST)
	cp -a $(IMAGE_APP_ISP_CFG_INI_SRC)  $(IMAGE_APP_ISP_CFG_INI_DST)
	cp -a $(IMAGE_APP_MODEL_CONF_DIR_SRC)  $(IMAGE_APP_MODEL_CONF_DIR_DST)
else
	cp -a $(IMAGE_APP_ISP_CFG_INI_SRC)  $(IMAGE_APP_ISP_CFG_INI_DST)
	cp -a $(IMAGE_APP_MODEL_CONF_DIR_SRC)  $(IMAGE_APP_MODEL_CONF_DIR_DST)
endif
	#cp -a $(IMAGE_APP_HOME_DIR_SRC)/shell/updater.sh $(IMAGE_APP_SHELL_DST)
	cp -a $(IMAGE_APP_LICENSE_DIR_SRC)  $(IMAGE_APP_LICENSE_DIR_DST) 
	
cp_resource :
#	rm -rf $(IMAGE_ROOTFS_DATA)/web
#	tar -h -C ../app_rebulid/src \
		-c web/ \
		--exclude="*svn" \
		| tar -x -C $(IMAGE_ROOTFS_DATA) -f -
#	find $(IMAGE_ROOTFS_DATA)/web -type f -name "*.*" | xargs gzip -9
#	rm -rf $(IMAGE_ROOTFS_DATA)/font
#	tar -h -C ../app_rebulid/src \
		-c font/ \
		--exclude="*svn" \
		| tar -x -C $(IMAGE_ROOTFS_DATA) -f -


include Makefile.tools
$(IMAGE_ROOTFS) :$(MK_ROM_BIN)
	@make -f Makefile.rootfs SOC_MODEL=$(DEVICE_MODEL)
#	$(MK_ROM_BIN) $(FLASH_MAP_INI) $(IMAGE_PATH) ./
	$< ./flashmap/$(FLASH_MAP_INI) $(IMAGE_PATH) ./

$(IMAGE_UBOOTENV) : $(MK_UBOOT_ENV)
#	$(MK_UBOOT_ENV) $(IMAGE_PATH)$(UBOOT_ENV_TXT) $(IMAGE_PATH)uboot_env.bin $(UBOOT_ENV_SIZE)
	$< $(IMAGE_PATH)$(UBOOT_ENV_TXT) $(IMAGE_PATH)uboot_env.bin $(UBOOT_ENV_SIZE)


