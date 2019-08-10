
CP := @cp -a
RM := @rm -Rf
LS := @ls -lh
DU := @du -sh
PRINTF := @printf "\033[32;1m\t%s\033[0m\n"

DEBUG_PATH := $(CURDIR)/Debug
RELEASE_PATH := $(CURDIR)/Release

SOC_MODEL ?= M388C
#SOC_MODEL ?= HI3516D
CROSS =
CC =
CFLAGS =
LDFLAGS =
DEF = -D_GNU_SOURCE
SRC =
OBJ =
DEP =
INC =
LIB =
SOC_PATH =


# IPCAM solution
ifeq "$(SOC_MODEL)" "HI3518A"
CROSS = arm-hisiv100nptl-linux-
SOC_PATH = $(CURDIR)/HI3518A
DEF += -DHI3518A
SRC += sdk_common.c
vpath %.c $(CURDIR)/HI3518A
SRC += hi3518a_audio.c
SRC += hi3518a_isp.c
SRC += hi3518a_isp_aptina_ar0130.c
SRC += hi3518a_isp_aptina_ar0330.c
SRC += hi3518a_isp_ov9712plus.c
SRC += hi3518a_isp_ov9712.c
SRC += hi3518a_isp_soih22.c
SRC += hi3518a_isp_sony_imx122.c
SRC += hi3518a_isp_gc1004.c
SRC += hi3518a_isp_aptina_ar0141.c
SRC += hi3518a_isp_sc1035.c
SRC += hi3518a_isp_ov2710.c
SRC += hi3518a_isp_soih42.c
SRC += hi3518a_isp_sc1045.c
SRC += hi3518a_isp_brigates_bg0701.c
SRC += hi3518a_isp_sony_imx225.c
SRC += hi3518a_isp_sc1135.c
SRC += hi_isp_api.c
SRC += hi_isp_cfg.c
SRC += hi_isp_cfg_parse.c
SRC += inifile.c
vpath %.c $(CURDIR)/HI3521
SRC += hi3521.c
SRC += hi3521_sys.c
SRC += hi3521_vin.c
SRC += hi3521_enc.c
endif

ifeq "$(SOC_MODEL)" "HI3518C"
CROSS = arm-hisiv100nptl-linux-
SOC_PATH = $(CURDIR)/HI3518A
DEF += -DHI3518C
SRC += sdk_common.c
vpath %.c $(CURDIR)/HI3518A
SRC += hi3518a_audio.c
SRC += hi3518a_isp.c
SRC += hi3518a_isp_aptina_ar0130.c
SRC += hi3518a_isp_aptina_ar0330.c
SRC += hi3518a_isp_ov9712plus.c
SRC += hi3518a_isp_ov9712.c
SRC += hi3518a_isp_soih22.c
SRC += hi3518a_isp_sony_imx122.c
SRC += hi3518a_isp_gc1004.c
SRC += hi3518a_isp_aptina_ar0141.c
SRC += hi3518a_isp_sc1035.c
SRC += hi3518a_isp_ov2710.c
SRC += hi3518a_isp_soih42.c
SRC += hi3518a_isp_sc1045.c
SRC += hi3518a_isp_brigates_bg0701.c
SRC += hi3518a_isp_sony_imx225.c
SRC += hi3518a_isp_sc1135.c
SRC += hi_isp_api.c
SRC += hi_isp_cfg.c
SRC += hi_isp_cfg_parse.c
SRC += inifile.c
vpath %.c $(CURDIR)/HI3521
SRC += hi3521.c
SRC += hi3521_sys.c
SRC += hi3521_vin.c
SRC += hi3521_enc.c
endif

# IPCAM solution
ifeq "$(SOC_MODEL)" "HI3518E"
CROSS = arm-hisiv100nptl-linux-
SOC_PATH = $(CURDIR)/HI3518A
DEF += -DHI3518E
SRC += sdk_common.c
vpath %.c $(CURDIR)/HI3518A
SRC += hi3518a_audio.c
SRC += hi3518a_isp.c
SRC += hi3518a_isp_aptina_ar0130.c
SRC += hi3518a_isp_aptina_ar0330.c
SRC += hi3518a_isp_ov9712plus.c
SRC += hi3518a_isp_ov9712.c
SRC += hi3518a_isp_soih22.c
SRC += hi3518a_isp_sony_imx122.c
SRC += hi3518a_isp_gc1004.c
SRC += hi3518a_isp_aptina_ar0141.c
SRC += hi3518a_isp_sc1035.c
SRC += hi3518a_isp_ov2710.c
SRC += hi3518a_isp_soih42.c
SRC += hi3518a_isp_sc1045.c
SRC += hi3518a_isp_brigates_bg0701.c
SRC += hi3518a_isp_sony_imx225.c
SRC += hi3518a_isp_sc1135.c
SRC += hi_isp_api.c
SRC += hi_isp_cfg.c
SRC += hi_isp_cfg_parse.c
SRC += inifile.c
vpath %.c $(CURDIR)/HI3521
SRC += hi3521.c
SRC += hi3521_sys.c
SRC += hi3521_vin.c
SRC += hi3521_enc.c
endif

ifeq "$(SOC_MODEL)" "HI3516C"
CROSS = arm-hisiv100nptl-linux-
SOC_PATH = $(CURDIR)/HI3518A
DEF += -DHI3518A
SRC += sdk_common.c
vpath %.c $(CURDIR)/HI3518A
SRC += hi3518a_audio.c
SRC += hi3518a_isp.c
SRC += hi3518a_isp_aptina_ar0130.c
SRC += hi3518a_isp_aptina_ar0330.c
SRC += hi3518a_isp_ov9712plus.c
SRC += hi3518a_isp_ov9712.c
SRC += hi3518a_isp_soih22.c
SRC += hi3518a_isp_sony_imx122.c
SRC += hi3518a_isp_gc1004.c
SRC += hi3518a_isp_aptina_ar0141.c
SRC += hi3518a_isp_sc1035.c
SRC += hi3518a_isp_ov2710.c
SRC += hi3518a_isp_soih42.c
SRC += hi3518a_isp_sc1045.c
SRC += hi3518a_isp_brigates_bg0701.c
SRC += hi3518a_isp_sony_imx225.c
SRC += hi3518a_isp_sc1135.c
SRC += hi_isp_api.c
SRC += hi_isp_cfg.c
SRC += hi_isp_cfg_parse.c
SRC += inifile.c
vpath %.c $(CURDIR)/HI3521
SRC += hi3521.c
SRC += hi3521_sys.c
SRC += hi3521_vin.c
SRC += hi3521_enc.c
endif

ifeq "$(SOC_MODEL)" "HI3516A"
CROSS = arm-hisiv300-linux-
SOC_PATH = $(CURDIR)/HI3516A
DEF += -DHI3516A
vpath %.c $(CURDIR)/HI3516A
endif

ifeq "$(SOC_MODEL)" "HI3516D"
CROSS = arm-hisiv300-linux-
SOC_PATH = $(CURDIR)/HI3516A
DEF += -DHI3516A
vpath %.c $(CURDIR)/HI3516A
endif

ifeq "$(SOC_MODEL)" "M388C"
CROSS = arm-linux-
SOC_PATH = $(CURDIR)/M388C
DEF += -DM388C
vpath %.c $(CURDIR)/M388C
endif


ifeq "$(SOC_MODEL)" "HI3518E_V2"
CROSS = arm-hisiv300-linux-
SOC_PATH = $(CURDIR)/HI3518E_V2
DEF += -DHI3518E_V2
vpath %.c $(CURDIR)/HI3518E_V2
endif


ifeq "$(SOC_MODEL)" "HI3516C_V2"
CROSS = arm-hisiv300-linux-
SOC_PATH = $(CURDIR)/HI3518E_V2
DEF += -DHI3516C_V2
vpath %.c $(CURDIR)/HI3518E_V2
endif


ifeq "$(SOC_MODEL)" "HI3516C_V3"
CROSS = arm-hisiv500-linux-
SOC_PATH = $(CURDIR)/HI3516C_V3
DEF += -DHI3516C_V3
vpath %.c $(CURDIR)/HI3516C_V3
endif

ifeq "$(SOC_MODEL)" "HI3516E_V1"
CROSS = arm-hisiv500-linux-
SOC_PATH = $(CURDIR)/HI3516C_V3
DEF += -DHI3516E_V1
vpath %.c $(CURDIR)/HI3516C_V3
endif

SRC_TMP += $(wildcard *.c $(SOC_PATH)/*.c)
SRC += $(notdir $(SRC_TMP))
OBJ = $(addprefix $(RELEASE_PATH)/, $(patsubst %.c, %.o, $(SRC)))
DEP = $(addprefix $(DEBUG_PATH)/, $(patsubst %.c, %.d, $(SRC)))
CC = $(CROSS)gcc
AR = $(CROSS)ar
INC = -I$(SOC_PATH)/include -I$(SOC_PATH) -I$(CURDIR) -I$(CURDIR)/../src
LIB = $(wildcard $(SOC_PATH)/lib/*.a)
CFLAGS = $(DEF) $(INC) -g3 -O0 -Wall -Wl,--gc-sections --function-sections -DPRODUCT_MODEL=\"$(PRODUCT_MODEL)\" -D$(PRODUCT_NAME)

TARGET_LIB := libJaSdk$(SOC_MODEL).a

FUN_DEF := $(SRC_FUNCTION_DEF) $(DEF_EXT)

.PHONY: all clean

all: clean $(TARGET_LIB) install
	$(PRINTF) "DEF = $(DEF)"
	$(PRINTF) "CC = $(CC)"
	$(PRINTF) "CFLAGS = $(CFLAGS)"
	$(PRINTF) "SRC = $(SRC)"
	$(PRINTF) "OBJ = $(OBJ)"
	$(PRINTF) "LIB = $(LIB)"
	$(PRINTF) "DEP = $(DEP)"

clean:
	@rm -Rf $(CURDIR)/*.o $(TARGET_LIB) $(DEBUG_PATH)/*.o $(RELEASE_PATH)/*.o

install: $(TARGET_LIB)
	$(CP) $< ../lib/$(CROSS)
	#$(RM) $(CURDIR)/../include/sdk
	#$(CP) sdk $(CURDIR)/../include/

$(TARGET_LIB): $(OBJ)
	for i in $(LIB) ; do $(AR) x $$i; done
	$(AR) -r $@  $^ *.o
#	$(AR) -r $@  $^ $(wildcard *.o)
	$(RM) *.o
#	@$(AR) -t $@
	$(DU) $@

ifneq ($(MAKECMDGOALS), clean)
-include $(DEP)
endif

$(DEBUG_PATH)/%.d: %.c
	$(PRINTF) "[DEP] $@"; $(CC) -MM -MF"$@" -MT"$(@:.d=.o)" $(FUN_DEF) $(DEF) $(INC) $<

$(RELEASE_PATH)/%.o: %.c
	$(PRINTF) "[ CC]  $@"; $(CC) -c $(CFLAGS) $(FUN_DEF) -o "$@" $<
