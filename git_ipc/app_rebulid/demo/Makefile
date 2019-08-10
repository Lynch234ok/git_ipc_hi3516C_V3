#DEVICE_MODEL ?= hi3518a_inc
DEVICE_MODEL   ?= hi3518e_inc
#DEVICE_MODEL   ?= hi3518c_inc
#DEVICE_MODEL   ?= hi3516d_inc
#DEVICE_MODEL   ?= hi3516a_inc


INC_PATH ?= $(CURDIR)/
LIB_PATH ?= $(CURDIR)/
BIN_PATH ?= $(CURDIR)/
DBG_PATH ?= $(CURDIR)/
CROSS ?= arm-hisiv100nptl-linux-

	
SDK_NAME := JaSdk$(SOC_MODEL)
SDK_LIB_A := $(CURDIR)/../lib/$(CROSS)/lib$(SDK_NAME).a


API_NAME := JaApi
API_LIB_A := $(CURDIR)/../lib/$(CROSS)/lib$(API_NAME).a


L_SRC_LIB := -L$(LIB_PATH) $(patsubst %,-l%,$(SRC_LIB))	
SRC := $(wildcard *.c)
INC := $(addprefix -I, $(INC_PATH)) -I$(CURDIR)/../sdk/HI3518A/include -I$(INC_PATH)/../include -I$(INC_PATH)/api -I$(CURDIR)# FIXME
OBJ := $(patsubst %.c,$(DBG_PATH)/%.o,$(SRC))
DEP := $(patsubst %.c,$(DBG_PATH)/%.d,$(SRC))
LIB := $(patsubst %,$(LIB_PATH)/%.a,$(SRC_LIB))

SRC_CFLAGS := -D_GNU_SOURCE -DLINUX=2 -DOS_USER=\"$(shell echo $(USER))\" -g3 -O0 -Wall \
	-DPRODUCT_CLASS=\"$(PRODUCT_CLASS)\" -DPRODUCT_MODEL=\"$(PRODUCT_MODEL)\" -DSOC_MODEL=\"$(SOC_MODEL)\" -DSERISE_CODE=\"$(SERISE_CODE)\" \
	$(SRC_FUNCTION_DEF) $(DEF_EXT)
	
CC := $(CROSS)gcc
CXX := $(CROSS)g++
AR := $(CROSS)ar
STRIP := $(CROSS)strip
CFLAGS := $(SRC_CFLAGS) $(INC)
LDFLAGS_OPEN := -lcyassl -lz -luuid
LDFLAGS :=  $(L_SRC_LIB) $(LDFLAGS_EXT) $(L_SRC_LIB) $(SDK_LIB_A) $(API_LIB_A) $(LDFLAGS_OPEN) -lpthread -lm -ldl -lrt -lNkN1API -lNkUtils

	
###########################################################
###########################################################

TARGET := $(BIN_PATH)/demo_app

.PHONY : clean distclean

$(DBG_PATH)/%.o : %.c
	@mkdir -p $(shell dirname $@)
	@echo -e "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<


$(TARGET): $(SDK_LIB_A) $(OBJ)
	@echo $(FUNCTION_DEF_LIB)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo $(SRC_CFLAGS)
	$(CC) $^ -o $@_rel $(LDFLAGS)
	$(STRIP) $@_rel


all: srclib $(TARGET)
sinclude $(DEP)

clean:
	@rm -Rf $(DBG_PATH)/* $(TARGET)

distclean: clean
	@rm -Rf $(LIB_OUT)


