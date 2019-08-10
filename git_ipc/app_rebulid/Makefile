#MODEL ?= hi3518a
#MODEL ?= hi3518e
#MODEL ?= hi3518e_w
#MODEL ?= hi3518c
#MODEL ?= hi3516c
#MODEL ?= hi3518a_s
#MODEL ?= hi3518a_p
#MODEL ?= hi3516a
#MODEL ?= hi3516d
#MODEL ?= m388c2g
#MODEL ?= m388c1g
#MODEL ?= hi3518e_v2
#MODEL ?= hi3516c_v2
#MODEL ?= hi3518e_ws
#MODEL ?= hi3518c_ws
#MODEL ?= hi3518e_wsc
#MODEL ?= hi3518a_wsc
LDFLAGS_EXT := 
DEF_EXT :=

ifeq ($(MODEL),hi3518a)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518a_inc_config.ini
endif

ifeq ($(MODEL),hi3518e)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518e_inc_config.ini
endif

ifeq ($(MODEL),hi3518e_w)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518e_w_inc_config.ini
endif

ifeq ($(MODEL),hi3518c)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518c_inc_config.ini
endif

ifeq ($(MODEL),hi3516c)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3516c_inc_config.ini
endif

ifeq ($(MODEL),hi3518a_s)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518a_s_inc_config.ini
endif

ifeq ($(MODEL),hi3518a_p)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518a_p_inc_config.ini
endif

ifeq ($(MODEL),hi3516a)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3516a_inc_config.ini
endif

ifeq ($(MODEL),hi3516d)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3516d_inc_config.ini
endif

ifeq ($(MODEL),m388c2g)
CONFIG_DIR_SRC := $(CURDIR)/configs/m388c2g_inc_config.ini
endif

ifeq ($(MODEL),m388c1g)
CONFIG_DIR_SRC := $(CURDIR)/configs/m388c1g_inc_config.ini
endif

ifeq ($(MODEL),hi3518e_v2)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518e_v2_inc_config.ini
endif

ifeq ($(MODEL),hi3516c_v2)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3516c_v2_inc_config.ini
endif

ifeq ($(MODEL),hi3518e_ws)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518e_ws_inc_config.ini
endif

ifeq ($(MODEL),hi3518c_ws)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518c_ws_inc_config.ini
endif

ifeq ($(MODEL),hi3518e_wsc)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518e_wsc_inc_config.ini
endif

ifeq ($(MODEL),hi3518a_wsc)
CONFIG_DIR_SRC := $(CURDIR)/configs/hi3518a_wsc_inc_config.ini
endif



ifndef MODEL
include  $(CURDIR)/config.ini
endif 


INC_PATH := $(CURDIR)/include
LIB_PATH := $(CURDIR)/lib/$(CROSS)
BIN_PATH := $(CURDIR)/bin
DBG_PATH := $(CURDIR)/debug
PAK_PATH := $(CURDIR)/package_demo


SRC_FUNCTION_DEF := $(foreach str,$(FUNCTION_DEF),-D$(str)) 
#JUAN ONVIF NVT lib
ifeq (ONVIFNVT,$(findstring ONVIFNVT, $(FUNCTION_DEF)))
LDFLAGS_EXT += "-lonvif"
endif
#Danale p2p lib
ifeq (DANALE,$(findstring DANALE,$(FUNCTION_DEF)))
LDFLAGS_EXT += "-ldana_video"
endif
#UDX p2p lib
ifeq (P2P,$(findstring P2P,$(FUNCTION_DEF)))
ifeq ("-DDANA_P2P",$(findstring "-DDANA_P2P",$(DEF_EXT)))
LDFLAGS_EXT += "-lDANASDK"
LDFLAGS_EXT += "-ldana_video_plus"
else
LDFLAGS_EXT += "-lP2PSDKDevice"
endif
LDFLAGS_EXT += "-larq"
LDFLAGS_EXT += "-lIOTDev"
LDFLAGS_EXT += "-lIOTLink"
LDFLAGS_EXT += "-lzlog"
LDFLAGS_EXT += "-lstdc++"
endif
#ants lib
ifeq (ANTS,$(findstring ANTS,$(FUNCTION_DEF)))
LDFLAGS_EXT += "-lantsserver"
LDFLAGS_EXT += "-lpublic"
LDFLAGS_EXT += "-ltinyxml"
endif
# N1 Reference Library
ifeq (N1,$(findstring N1,$(FUNCTION_DEF)))
#LDFLAGS_EXT += "-lNkN1API"
#LDFLAGS_EXT += "-lNkUtils"
LDFLAGS_EXT += "-lNkN1SDK"
LDFLAGS_EXT += "-lNkEmbedded"
LDFLAGS_EXT += "-lNkUtils"
LDFLAGS_EXT += "-lNkCipher"
LDFLAGS_EXT += "-lpthread"
endif

# OSS Cloud and alarm message push
ifeq ("-DMSG_PICTURE",$(findstring "-DMSG_PICTURE",$(DEF_EXT)))
LDFLAGS_EXT += "-lOSSCloud"
LDFLAGS_EXT += "-loss_c_media_sdk_client_static"
LDFLAGS_EXT += "-loss_c_sdk_static"
LDFLAGS_EXT += "-lmxml"
LDFLAGS_EXT += "-laprutil-1"
LDFLAGS_EXT += "-lapr-1"
LDFLAGS_EXT += "-lcurl"
LDFLAGS_EXT += "-lmbedtls"
LDFLAGS_EXT += "-lmbedx509"
LDFLAGS_EXT += "-lmbedcrypto"
endif

LDFLAGS_EXT += "-ljson-c"
LDFLAGS_EXT += "-lsecuredat"

ifeq (CLOUD_REC,$(findstring CLOUD_REC,$(DEF_EXT)))
LDFLAGS_EXT += -lOSSCloud -loss_c_media_sdk_client_static -loss_c_sdk_static -laprutil-1 -lapr-1 -lcurl -lmbedx509 -lmxml -lNkUtils -lwolfssl -lmbedtls -lmbedcrypto 
endif
MAKE := @make

.PHONY: target all src shell sdk api sdk_clean tarball image config
	
#info:
#	@echo $(PRODUCT_CLASS)
#	@echo $(PRODUCT_MODEL)
#	@echo $(SOC_MODEL)
#

target: src

src: srclib
	echo $(PRODUCT_CLASS)
	$(MAKE) -C src CROSS="$(CROSS)" \
		PRODUCT_CLASS="$(PRODUCT_CLASS)" \
		PRODUCT_MODEL="$(PRODUCT_MODEL)" \
		SOC_MODEL="$(SOC_MODEL)" \
		INC_PATH="$(INC_PATH)" \
		LIB_PATH="$(LIB_PATH)" \
		BIN_PATH="$(BIN_PATH)" \
		DBG_PATH="$(DBG_PATH)" \
		SERISE_CODE="$(SERISE_CODE)" \
		FUNCTION_DEF_LIB="$(shell echo $(FUNCTION_DEF) | tr A-Z a-z )" \
		SRC_FUNCTION_DEF="$(SRC_FUNCTION_DEF)" \
		LDFLAGS_EXT="$(LDFLAGS_EXT)" \
		DEVICE_MODEL="$(DEVICE_MODEL)" \
		DEF_EXT="$(DEF_EXT)"	\
		PRODUCT_NAME="$(PRODUCT_NAME)" \



srclib:
	rm -f $(BIN_PATH)/ipcam_app
	$(MAKE) -C src srclib CROSS="@$(CROSS)" \
		PRODUCT_CLASS="$(PRODUCT_CLASS)" \
		PRODUCT_MODEL="$(PRODUCT_MODEL)" \
		SOC_MODEL="$(SOC_MODEL)" \
		INC_PATH="$(INC_PATH)" \
		LIB_PATH="$(LIB_PATH)" \
		BIN_PATH="$(BIN_PATH)" \
		DBG_PATH="$(DBG_PATH)" \
		SERISE_CODE="$(SERISE_CODE)" \
		FUNCTION_DEF_LIB="$(shell echo $(FUNCTION_DEF) | tr A-Z a-z )" \
		SRC_FUNCTION_DEF="$(SRC_FUNCTION_DEF)" \
		DEVICE_MODEL="$(DEVICE_MODEL)" \
		DEF_EXT="$(DEF_EXT)"	\
		PRODUCT_NAME="$(PRODUCT_NAME)" \


shell:
	$(MAKE) -C shell CROSS="@$(CROSS)" \
		INC_PATH="$(INC_PATH)" \
		LIB_PATH="$(LIB_PATH)" \
		BIN_PATH="$(BIN_PATH)" \
		DBG_PATH="$(DBG_PATH)" \

	
sdk:
	$(MAKE) --directory=sdk SOC_MODEL="$(SOC_MODEL)" \
	PRODUCT_MODEL="$(PRODUCT_MODEL)" \
	SRC_FUNCTION_DEF="$(SRC_FUNCTION_DEF)" \
	DEF_EXT="$(DEF_EXT)"	\
	PRODUCT_NAME="$(PRODUCT_NAME)" \

api:
	$(MAKE) --directory=api CROSS=$(CROSS)

	
tarball:
	$(MAKE) --directory=sdk clean

checkinclude:
	rm -rf include/sdk
	cp -a api/include/* include
	cp -a sdk/sdk include

image: src
	#cd ../release
	$(MAKE) -C ../release DEVICE_MODEL="$(DEVICE_MODEL)" PRODUCT_MODEL="$(PRODUCT_MODEL)" FUNCTION_DEF="$(FUNCTION_DEF)" MODEL_ROOTFS="$(MODEL_ROOTFS)" CROSS="$(CROSS)"

all: clean sdk srclib src image

demoapp:
	echo $(PRODUCT_CLASS)
	rm -f $(BIN_PATH)/demo_app
	$(MAKE) -C demo CROSS="$(CROSS)" \
		PRODUCT_CLASS="$(PRODUCT_CLASS)" \
		PRODUCT_MODEL="$(PRODUCT_MODEL)" \
		SOC_MODEL="$(SOC_MODEL)" \
		INC_PATH="$(INC_PATH)" \
		LIB_PATH="$(LIB_PATH)" \
		BIN_PATH="$(BIN_PATH)" \
		DBG_PATH="$(DBG_PATH)" \
		FUNCTION_DEF_LIB="$(shell echo $(FUNCTION_DEF) | tr A-Z a-z )" \
		SRC_FUNCTION_DEF="$(SRC_FUNCTION_DEF)" \
		LDFLAGS_EXT="$(LDFLAGS_EXT)" \
		DEVICE_MODEL="$(DEVICE_MODEL)" \
		DEF_EXT="$(DEF_EXT)"	\

demo: clean sdk demoapp

packet_demo: api
	rm -f IPC_demo*.tar.gz 
	rm -rf $(PAK_PATH)
	mkdir $(PAK_PATH)
	mkdir $(PAK_PATH)/bin
	cp -a demo $(PAK_PATH)
	cp -a include $(PAK_PATH)
	cp -a sdk $(PAK_PATH)
	cp -a Makefile $(PAK_PATH)
	cp -a lib $(PAK_PATH)
	tar -zcvf `date "+IPC_demo-%Y%m%d-%H%M-$(MODEL_NAME).tar.gz"` package_demo;

config:
	rm -f $(CURDIR)/config.ini
	cp -a  $(CONFIG_DIR_SRC)  $(CURDIR)/config.ini
#eg: make config MODEL=hi3518a

clean:
	rm -f $(shell find $(CURDIR) -name *.o)
	rm -f $(shell find $(CURDIR) -name *.d)
