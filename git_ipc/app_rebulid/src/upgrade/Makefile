#include ../../plat.mk
UPGRADE_BIN:=upgrade
SRC:=../bsp/watchdog.c  upgrade_demo.c upgrade_cgi.c
##马上展开
CROSS_COMPILE ?= arm-hisiv500-linux-
CC=gcc
STRIP=strip

###-DCOMPILE_PRODUCT_ID=1只用于编译通过而已
CFLAGS:= -os  -I../ -I../../include/ -I./ -DMK_UPGRADE_APP -lpthread

$(UPGRADE_BIN):clean
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(SRC) -o $(UPGRADE_BIN)
	$(CROSS_COMPILE)$(STRIP)  $(UPGRADE_BIN)

clean:
	rm $(UPGRADE_BIN) -rf

