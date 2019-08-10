DEVICE_MODEL ?= hi3518a_inc

INC_PATH ?= $(CURDIR)/
LIB_PATH ?= $(CURDIR)/
BIN_PATH ?= $(CURDIR)/
DBG_PATH ?= $(CURDIR)/
CROSS ?= arm-hisiv100nptl-linux-

	
SDK_NAME := JaSdk$(SOC_MODEL)
SDK_LIB_A := $(CURDIR)/../lib/$(CROSS)/lib$(SDK_NAME).a


API_NAME := JaApi
API_LIB_A := $(CURDIR)/../lib/$(CROSS)/lib$(API_NAME).a


SRC_LIB := \
		eseeclient ddns smtp sensor firmware upnp \
		httpd \
		usrm \
		unistruct \
		hls	\
		bsp \
		generic \
		sysconf \
		regRW	\
		rtspnvt	\
		mp4demux \
		bluetooth \
		device_binding \
		gsensor \
		wpa_supplicant \
		$(FUNCTION_DEF_LIB)\

ifeq ($(DEVICE_MODEL),hi3516a_inc)
SRC_LIB := \
		eseeclient ddns smtp sensor firmware upnp \
		httpd \
		usrm \
		unistruct \
		hls	\
		bsp16 \
		generic \
		sysconf \
		regRW	\
		mp4demux \
        bluetooth \
        device_binding \
        gsensor \
        wpa_supplicant \
		$(FUNCTION_DEF_LIB)\

endif


ifeq ($(DEVICE_MODEL),hi3516d_inc)
SRC_LIB := \
		eseeclient ddns smtp sensor firmware upnp \
		httpd \
		usrm \
		unistruct \
		hls	\
		bsp16 \
		generic \
		sysconf \
		regRW	\
		rtspnvt	\
		mp4demux \
        bluetooth \
        device_binding \
        gsensor \
        wpa_supplicant \
		$(FUNCTION_DEF_LIB)\

endif

ifeq ($(DEVICE_MODEL),hi3516e_v1_inc)
SRC_LIB := \
		eseeclient sensor firmware \
		httpd \
		usrm \
		unistruct \
		hls	\
		bsp \
		generic \
		sysconf \
		regRW	\
		rtspnvt	\
		mp4demux \
		device_binding \
		wpa_supplicant \
		$(FUNCTION_DEF_LIB)\
		upgrade \
		motor \

endif



L_SRC_LIB := -L$(LIB_PATH) $(patsubst %,-l%,$(SRC_LIB))	
SRC := $(wildcard *.c)
INC := $(addprefix -I, $(INC_PATH)) -I$(CURDIR)/../sdk/HI3518A/include -I$(INC_PATH)/../include -I$(INC_PATH)/api -I$(CURDIR)# FIXME
OBJ := $(patsubst %.c,$(DBG_PATH)/%.o,$(SRC))
DEP := $(patsubst %.c,$(DBG_PATH)/%.d,$(SRC))
LIB := $(patsubst %,$(LIB_PATH)/%.a,$(SRC_LIB))

SRC_CFLAGS := -D_GNU_SOURCE -DLINUX=2 -DOS_USER=\"$(shell echo $(USER))\" -O0 -Wl,--gc-sections --function-sections -Wall -g3 \
	-DPRODUCT_CLASS=\"$(PRODUCT_CLASS)\" -DPRODUCT_MODEL=\"$(PRODUCT_MODEL)\" -DSOC_MODEL=\"$(SOC_MODEL)\" -DSERISE_CODE=\"$(SERISE_CODE)\" \
	$(SRC_FUNCTION_DEF) $(DEF_EXT) -D$(SOC_MODEL)
	
CC := $(CROSS)gcc
CXX := $(CROSS)g++
AR := $(CROSS)ar
STRIP := $(CROSS)strip
CFLAGS := $(SRC_CFLAGS) $(INC)
LDFLAGS_OPEN := -lcyassl -lz -luuid

LDFLAGS := -Wl,--gc-sections $(L_SRC_LIB) $(LDFLAGS_EXT) $(L_SRC_LIB) -lmediabuffer -lrtsp $(SDK_LIB_A) $(API_LIB_A) $(LDFLAGS_OPEN) -laacdec -laacenc -lmp4_muxer -lavformat -lavcodec -lavutil -lmwlog -lbase -lpthread -lm -ldl -lrt
DEF := $(SRC_FUNCTION_DEF) $(DEF_EXT)

MK_SRC_LIB := make -f $(CURDIR)/Makefile.srclib \
	CC="$(CC)" CXX="$(CXX)" AR="$(AR)" CFLAGS="$(CFLAGS)" \
	INC_PATH="$(INC_PATH)" LIB_PATH="$(LIB_PATH)" BIN_PATH="$(BIN_PATH)" DBG_PATH="$(DBG_PATH)" \
	PRODUCT_NAME="$(PRODUCT_NAME)"	--directory=$(CURDIR)/

	
###########################################################
###########################################################

TARGET := $(BIN_PATH)/ipcam_app

.PHONY : clean distclean

$(DBG_PATH)/%.o : %.c
	@mkdir -p $(shell dirname $@)
	@echo -e "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -D$(PRODUCT_NAME) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<


$(TARGET): $(SDK_LIB_A) $(OBJ)
	@echo $(FUNCTION_DEF_LIB)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo $(SRC_CFLAGS)
	$(CC) $^ -o $@_rel $(LDFLAGS)
	$(STRIP) $@_rel
srclib:
	@for i in $(SRC_LIB) ; do if test -d `dirname $$i` ; \
		then $(MK_SRC_LIB)$$i || exit $$? ; fi ; done

$(SDK_LIB_A):
	make --directory=$(CURDIR)/../sdk SOC_MODEL=HI3518A

all: srclib $(TARGET)
sinclude $(DEP)

clean:
	@rm -Rf $(DBG_PATH)/* $(TARGET)

distclean: clean
	@rm -Rf $(LIB_OUT)


