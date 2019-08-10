
include ./Makefile.in

DEV_SOC ?= "N18C"
SRC := $(SOURCES)
INC := $(patsubst %,-I%,$(INCLUDES))

TOOL_CC := gcc -m32 -g3 -Wall -DLINUX -D_GNU_SOURCE $(INC) $(SRC) -DPRODUCT_MODEL=\"$(PRODUCT_MODEL)\" -DSOC_MODEL=\"$(SOC_MODEL)\" -DPC_TOOLS

TOOL_ROM_BIN := mk_rom_bin
TOOL_UBOOTENV := mk_uboot_env

.PHONY : all clean

all : $(TOOL_UBOOTENV) $(TOOL_ROM_BIN)

$(TOOL_ROM_BIN) : $(SRC)
	$(TOOL_CC) firmware_tool_rom_bin.c -o $@ $(CURDIR)/../../lib/x86/libJaApi.a -lpthread -lrt

$(TOOL_UBOOTENV) : $(SRC)
	$(TOOL_CC) firmware_tool_uboot_env.c -o $@ $(CURDIR)/../../lib/x86/libJaApi.a -lpthread -lrt

clean:
	rm -Rf $(TOOL_ROM_BIN) $(TOOL_UBOOTENV)
	
