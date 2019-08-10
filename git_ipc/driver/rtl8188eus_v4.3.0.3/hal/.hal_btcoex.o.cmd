cmd_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.o := arm-hisiv100nptl-linux-gcc -Wp,-MD,/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/.hal_btcoex.o.d  -nostdinc -isystem /opt/hisi-linux-nptl/arm-hisiv100-linux/bin/../lib/gcc/arm-hisiv100-linux-uclibcgnueabi/4.4.1/include -I/root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-hi3518/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -marm -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -O1 -Wno-unused-variable -Wno-unused-value -Wno-unused-label -Wno-unused-parameter -Wno-unused-function -Wno-unused -I/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include -I/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/platform -DCONFIG_RTL8188E -DCONFIG_MP_INCLUDED -DCONFIG_TRAFFIC_PROTECT -DCONFIG_LOAD_PHY_PARA_FROM_FILE -DCONFIG_LITTLE_ENDIAN -DCONFIG_PLATFORM_ACTIONS_HI3518X  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(hal_btcoex)"  -D"KBUILD_MODNAME=KBUILD_STR(rtl8188eus)" -c -o /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.o /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.c

source_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.o := /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.c

deps_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.o := \
    $(wildcard include/config/bt/coexist.h) \
    $(wildcard include/config/lps/lclk.h) \
    $(wildcard include/config/concurrent/mode.h) \
    $(wildcard include/config/dualmac/concurrent.h) \
    $(wildcard include/config/p2p.h) \
    $(wildcard include/config/pci/hci.h) \
    $(wildcard include/config/usb/hci.h) \
    $(wildcard include/config/sdio/hci.h) \
    $(wildcard include/config/gspi/hci.h) \

/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.o: $(deps_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.o)

$(deps_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/hal_btcoex.o):
