cmd_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.o := arm-hisiv100nptl-linux-gcc -Wp,-MD,/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/.odm_interface.o.d  -nostdinc -isystem /opt/hisi-linux-nptl/arm-hisiv100-linux/bin/../lib/gcc/arm-hisiv100-linux-uclibcgnueabi/4.4.1/include -I/root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-hi3518/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -marm -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -O1 -Wno-unused-variable -Wno-unused-value -Wno-unused-label -Wno-unused-parameter -Wno-unused-function -Wno-unused -I/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include -I/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/platform -DCONFIG_RTL8188E -DCONFIG_MP_INCLUDED -DCONFIG_TRAFFIC_PROTECT -DCONFIG_LOAD_PHY_PARA_FROM_FILE -DCONFIG_LITTLE_ENDIAN -DCONFIG_PLATFORM_ACTIONS_HI3518X  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(odm_interface)"  -D"KBUILD_MODNAME=KBUILD_STR(rtl8188eus)" -c -o /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.o /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.c

source_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.o := /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.c

deps_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.o := \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_precomp.h \
    $(wildcard include/config/intel/proxim.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_types.h \
    $(wildcard include/config/hw/antenna/diversity.h) \
    $(wildcard include/config/sw/antenna/diversity.h) \
    $(wildcard include/config/ant/switch.h) \
    $(wildcard include/config/no/2g/diversity.h) \
    $(wildcard include/config/no/5g/diversity.h) \
    $(wildcard include/config/not/support/antdiv.h) \
    $(wildcard include/config/2g/support/antdiv.h) \
    $(wildcard include/config/5g/support/antdiv.h) \
    $(wildcard include/config/2g5g/support/antdiv.h) \
    $(wildcard include/config/usb/hci.h) \
    $(wildcard include/config/pci/hci.h) \
    $(wildcard include/config/sdio/hci.h) \
    $(wildcard include/config/gspi/hci.h) \
    $(wildcard include/config/little/endian.h) \
    $(wildcard include/config/big/endian.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/drv_types.h \
    $(wildcard include/config/suspend/refine.h) \
    $(wildcard include/config/80211n/ht.h) \
    $(wildcard include/config/80211ac/vht.h) \
    $(wildcard include/config/intel/widi.h) \
    $(wildcard include/config/beamforming.h) \
    $(wildcard include/config/p2p.h) \
    $(wildcard include/config/tdls.h) \
    $(wildcard include/config/wapi/support.h) \
    $(wildcard include/config/drvext.h) \
    $(wildcard include/config/mp/included.h) \
    $(wildcard include/config/br/ext.h) \
    $(wildcard include/config/iol.h) \
    $(wildcard include/config/ioctl/cfg80211.h) \
    $(wildcard include/config/1t1r.h) \
    $(wildcard include/config/2t2r.h) \
    $(wildcard include/config/tx/early/mode.h) \
    $(wildcard include/config/bt/coexist.h) \
    $(wildcard include/config/adaptor/info/caching/file.h) \
    $(wildcard include/config/layer2/roaming.h) \
    $(wildcard include/config/dualmac/concurrent.h) \
    $(wildcard include/config/80211d.h) \
    $(wildcard include/config/special/setting/for/funai/tv.h) \
    $(wildcard include/config/load/phy/para/from/file.h) \
    $(wildcard include/config/multi/vir/ifaces.h) \
    $(wildcard include/config/concurrent/mode.h) \
    $(wildcard include/config/usb/vendor/req/mutex.h) \
    $(wildcard include/config/usb/vendor/req/buffer/prealloc.h) \
    $(wildcard include/config/mac/loopback/driver.h) \
    $(wildcard include/config/ieee80211w.h) \
    $(wildcard include/config/ap/mode.h) \
    $(wildcard include/config/wfd.h) \
    $(wildcard include/config/autosuspend.h) \
    $(wildcard include/config/pno/support.h) \
    $(wildcard include/config/pno/set/debug.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/drv_conf.h \
    $(wildcard include/config/android.h) \
    $(wildcard include/config/platform/android.h) \
    $(wildcard include/config/validate/ssid.h) \
    $(wildcard include/config/signal/display/dbm.h) \
    $(wildcard include/config/has/earlysuspend.h) \
    $(wildcard include/config/resume/in/workqueue.h) \
    $(wildcard include/config/android/power.h) \
    $(wildcard include/config/wakelock.h) \
    $(wildcard include/config/vendor/req/retry.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/autoconf.h \
    $(wildcard include/config/single/img.h) \
    $(wildcard include/config/disable/odm.h) \
    $(wildcard include/config/platform/actions/atm702x.h) \
    $(wildcard include/config/cfg80211/force/compatible/2/6/37/under.h) \
    $(wildcard include/config/debug/cfg80211.h) \
    $(wildcard include/config/drv/issue/prov/req.h) \
    $(wildcard include/config/set/scan/deny/timer.h) \
    $(wildcard include/config/h2clbk.h) \
    $(wildcard include/config/embedded/fwimg.h) \
    $(wildcard include/config/file/fwimg.h) \
    $(wildcard include/config/xmit/ack.h) \
    $(wildcard include/config/active/keep/alive/check.h) \
    $(wildcard include/config/recv/reordering/ctrl.h) \
    $(wildcard include/config/tcp/csum/offload/rx.h) \
    $(wildcard include/config/support/usb/int.h) \
    $(wildcard include/config/usb/interrupt/in/pipe.h) \
    $(wildcard include/config/ips.h) \
    $(wildcard include/config/ips/level/2.h) \
    $(wildcard include/config/lps.h) \
    $(wildcard include/config/lps/lclk.h) \
    $(wildcard include/config/xmit/thread/mode.h) \
    $(wildcard include/config/antenna/diversity.h) \
    $(wildcard include/config/hwport/swap.h) \
    $(wildcard include/config/runtime/port/switch.h) \
    $(wildcard include/config/sta/mode/scan/under/ap/mode.h) \
    $(wildcard include/config/tsf/reset/offload.h) \
    $(wildcard include/config/interrupt/based/txbcn.h) \
    $(wildcard include/config/interrupt/based/txbcn/early/int.h) \
    $(wildcard include/config/interrupt/based/txbcn/bcn/ok/err.h) \
    $(wildcard include/config/nativeap/mlme.h) \
    $(wildcard include/config/hostapd/mlme.h) \
    $(wildcard include/config/find/best/channel.h) \
    $(wildcard include/config/no/wireless/handlers.h) \
    $(wildcard include/config/wifi/test.h) \
    $(wildcard include/config/p2p/remove/group/info.h) \
    $(wildcard include/config/dbg/p2p.h) \
    $(wildcard include/config/p2p/ps.h) \
    $(wildcard include/config/p2p/ips.h) \
    $(wildcard include/config/p2p/op/chk/social/ch.h) \
    $(wildcard include/config/cfg80211/onechannel/under/concurrent.h) \
    $(wildcard include/config/p2p/chk/invite/ch/list.h) \
    $(wildcard include/config/p2p/invite/iot.h) \
    $(wildcard include/config/tdls/autosetup.h) \
    $(wildcard include/config/tdls/autocheckalive.h) \
    $(wildcard include/config/skb/copy.h) \
    $(wildcard include/config/led.h) \
    $(wildcard include/config/sw/led.h) \
    $(wildcard include/config/led/handled/by/cmd/thread.h) \
    $(wildcard include/config/iol/new/generation.h) \
    $(wildcard include/config/iol/read/efuse/map.h) \
    $(wildcard include/config/iol/llt.h) \
    $(wildcard include/config/iol/efuse/patch.h) \
    $(wildcard include/config/iol/ioreg/cfg.h) \
    $(wildcard include/config/iol/ioreg/cfg/dbg.h) \
    $(wildcard include/config/global/ui/pid.h) \
    $(wildcard include/config/layer2/roaming/resume.h) \
    $(wildcard include/config/long/delay/issue.h) \
    $(wildcard include/config/new/signal/stat/process.h) \
    $(wildcard include/config/deauth/before/connect.h) \
    $(wildcard include/config/br/ext/brname.h) \
    $(wildcard include/config/tx/mcast2uni.h) \
    $(wildcard include/config/check/ac/lifetime.h) \
    $(wildcard include/config/minimal/memory/usage.h) \
    $(wildcard include/config/usb/tx/aggregation.h) \
    $(wildcard include/config/usb/rx/aggregation.h) \
    $(wildcard include/config/prealloc/recv/skb.h) \
    $(wildcard include/config/reduce/usb/tx/int.h) \
    $(wildcard include/config/easy/replacement.h) \
    $(wildcard include/config/use/usb/buffer/alloc/xx.h) \
    $(wildcard include/config/use/usb/buffer/alloc/tx.h) \
    $(wildcard include/config/use/usb/buffer/alloc/rx.h) \
    $(wildcard include/config/usb/vendor/req/buffer/dynamic/allocate.h) \
    $(wildcard include/config/usb/support/async/vdn/req.h) \
    $(wildcard include/config/only/one/out/ep/to/low.h) \
    $(wildcard include/config/out/ep/wifi/mode.h) \
    $(wildcard include/config/mp/iwpriv/support.h) \
    $(wildcard include/config/platform/mn10300.h) \
    $(wildcard include/config/power/saving.h) \
    $(wildcard include/config/platform/ti/dm365.h) \
    $(wildcard include/config/attempt/to/fix/ap/beacon/error.h) \
    $(wildcard include/config/debug.h) \
    $(wildcard include/config/debug/rtl871x.h) \
    $(wildcard include/config/proc/debug.h) \
    $(wildcard include/config/error/detect.h) \
    $(wildcard include/config/error/detect/int.h) \
    $(wildcard include/config/error/reset.h) \
    $(wildcard include/config/single/xmit/buf.h) \
    $(wildcard include/config/single/recv/buf.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/basic_types.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/posix_types.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/osdep_service.h \
    $(wildcard include/config/use/vmalloc.h) \
    $(wildcard include/config/ap/wowlan.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/osdep_service_linux.h \
    $(wildcard include/config/tcp/csum/offload/tx.h) \
    $(wildcard include/config/efuse/config/file.h) \
    $(wildcard include/config/file.h) \
    $(wildcard include/config/usb/suspend.h) \
  include/linux/version.h \
  include/linux/spinlock.h \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/typecheck.h \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
  include/linux/bitops.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/bitops.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/system.h \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/cpu/32v6k.h) \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/arch/has/barriers.h) \
    $(wildcard include/config/arm/dma/mem/bufferable.h) \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/linux/linkage.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/linkage.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/irqflags.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/hwcap.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/outercache.h \
    $(wildcard include/config/outer/cache/sync.h) \
    $(wildcard include/config/outer/cache.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/memory.h \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/page/offset.h) \
    $(wildcard include/config/thumb2/kernel.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/dram/size.h) \
    $(wildcard include/config/dram/base.h) \
    $(wildcard include/config/have/tcm.h) \
    $(wildcard include/config/arm/patch/phys/virt.h) \
    $(wildcard include/config/arm/patch/phys/virt/16bit.h) \
  include/linux/const.h \
  arch/arm/mach-hi3518/include/mach/memory.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/sizes.h \
  include/asm-generic/sizes.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/sparsemem.h) \
  include/asm-generic/cmpxchg-local.h \
  include/asm-generic/cmpxchg.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/arch_hweight.h \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/lock.h \
  include/asm-generic/bitops/le.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/arm/thumbee.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/vfpv3.h) \
    $(wildcard include/config/iwmmxt.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
    $(wildcard include/config/cpu/use/domains.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/compaction.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /opt/hisi-linux-nptl/arm-hisiv100-linux/bin/../lib/gcc/arm-hisiv100-linux-uclibcgnueabi/4.4.1/include/stdarg.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/printk.h \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/dynamic_debug.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/div64.h \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/prove/rcu.h) \
  include/linux/rwlock_types.h \
  include/linux/spinlock_up.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/processor.h \
    $(wildcard include/config/have/hw/breakpoint.h) \
    $(wildcard include/config/arm/errata/754327.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/hw_breakpoint.h \
  include/linux/rwlock.h \
  include/linux/spinlock_api_up.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/atomic.h \
    $(wildcard include/config/generic/atomic64.h) \
  include/asm-generic/atomic64.h \
  include/asm-generic/atomic-long.h \
  include/linux/errno.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/kmemcheck.h) \
    $(wildcard include/config/failslab.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/debug/slab.h) \
    $(wildcard include/config/slab.h) \
  include/linux/gfp.h \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/cgroup/mem/res/ctlr.h) \
    $(wildcard include/config/no/bootmem.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/have/memoryless/nodes.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/have/arch/pfn/valid.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  include/linux/wait.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/current.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/cache.h \
    $(wildcard include/config/arm/l1/cache/shift.h) \
    $(wildcard include/config/aeabi.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/seqlock.h \
  include/linux/nodemask.h \
  include/linux/bitmap.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/string.h \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  include/generated/bounds.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/page.h \
    $(wildcard include/config/cpu/copy/v3.h) \
    $(wildcard include/config/cpu/copy/v4wt.h) \
    $(wildcard include/config/cpu/copy/v4wb.h) \
    $(wildcard include/config/cpu/copy/feroceon.h) \
    $(wildcard include/config/cpu/copy/fa.h) \
    $(wildcard include/config/cpu/xscale.h) \
    $(wildcard include/config/cpu/copy/v6.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/glue.h \
  include/asm-generic/getorder.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/memory/hotremove.h) \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/have/arch/mutex/cpu/relax.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
    $(wildcard include/config/sched/book.h) \
    $(wildcard include/config/use/percpu/numa/node/id.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
  include/linux/smp.h \
    $(wildcard include/config/use/generic/smp/helpers.h) \
  include/linux/percpu.h \
    $(wildcard include/config/need/per/cpu/embed/first/chunk.h) \
    $(wildcard include/config/need/per/cpu/page/first/chunk.h) \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/pfn.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/virtual.h) \
  include/linux/slub_def.h \
    $(wildcard include/config/slub/stats.h) \
    $(wildcard include/config/slub/debug.h) \
    $(wildcard include/config/sysfs.h) \
  include/linux/workqueue.h \
    $(wildcard include/config/debug/objects/work.h) \
    $(wildcard include/config/freezer.h) \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  include/linux/math64.h \
  include/linux/jiffies.h \
  include/linux/timex.h \
  include/linux/param.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/param.h \
    $(wildcard include/config/hz.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/timex.h \
  arch/arm/mach-hi3518/include/mach/timex.h \
  include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects/free.h) \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/kobject_ns.h \
  include/linux/kref.h \
  include/linux/kmemleak.h \
    $(wildcard include/config/debug/kmemleak.h) \
  include/linux/module.h \
    $(wildcard include/config/symbol/prefix.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/debug/set/module/ronx.h) \
  include/linux/stat.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/stat.h \
  include/linux/kmod.h \
  include/linux/sysctl.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/rcu/torture/test.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/tree/preempt/rcu.h) \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/no/hz.h) \
    $(wildcard include/config/tiny/rcu.h) \
    $(wildcard include/config/tiny/preempt/rcu.h) \
    $(wildcard include/config/debug/objects/rcu/head.h) \
    $(wildcard include/config/preempt/rt.h) \
  include/linux/completion.h \
  include/linux/rcutiny.h \
  include/linux/elf.h \
  include/linux/elf-em.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/elf.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/user.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/tracepoint.h \
  include/linux/jump_label.h \
    $(wildcard include/config/jump/label.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/module.h \
    $(wildcard include/config/arm/unwind.h) \
  include/trace/events/module.h \
  include/trace/define_trace.h \
  include/linux/netdevice.h \
    $(wildcard include/config/dcb.h) \
    $(wildcard include/config/wlan.h) \
    $(wildcard include/config/ax25.h) \
    $(wildcard include/config/mac80211/mesh.h) \
    $(wildcard include/config/tr.h) \
    $(wildcard include/config/net/ipip.h) \
    $(wildcard include/config/net/ipgre.h) \
    $(wildcard include/config/ipv6/sit.h) \
    $(wildcard include/config/ipv6/tunnel.h) \
    $(wildcard include/config/netpoll.h) \
    $(wildcard include/config/rps.h) \
    $(wildcard include/config/xps.h) \
    $(wildcard include/config/rfs/accel.h) \
    $(wildcard include/config/net/poll/controller.h) \
    $(wildcard include/config/fcoe.h) \
    $(wildcard include/config/wireless/ext.h) \
    $(wildcard include/config/vlan/8021q.h) \
    $(wildcard include/config/net/dsa.h) \
    $(wildcard include/config/net/ns.h) \
    $(wildcard include/config/net/dsa/tag/dsa.h) \
    $(wildcard include/config/net/dsa/tag/trailer.h) \
    $(wildcard include/config/netpoll/trap.h) \
    $(wildcard include/config/proc/fs.h) \
  include/linux/if.h \
  include/linux/socket.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/socket.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/sockios.h \
  include/linux/sockios.h \
  include/linux/uio.h \
  include/linux/hdlc/ioctl.h \
  include/linux/if_ether.h \
  include/linux/skbuff.h \
    $(wildcard include/config/nf/conntrack.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/nf/defrag/ipv4.h) \
    $(wildcard include/config/nf/defrag/ipv6.h) \
    $(wildcard include/config/xfrm.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/net/cls/act.h) \
    $(wildcard include/config/ipv6/ndisc/nodetype.h) \
    $(wildcard include/config/net/dma.h) \
    $(wildcard include/config/network/secmark.h) \
    $(wildcard include/config/network/phy/timestamping.h) \
  include/linux/kmemcheck.h \
  include/linux/mm_types.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/want/page/debug/flags.h) \
    $(wildcard include/config/aio.h) \
    $(wildcard include/config/mm/owner.h) \
    $(wildcard include/config/mmu/notifier.h) \
    $(wildcard include/config/transparent/hugepage.h) \
  include/linux/auxvec.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/auxvec.h \
  include/linux/prio_tree.h \
  include/linux/rbtree.h \
  include/linux/page-debug-flags.h \
    $(wildcard include/config/page/poisoning.h) \
    $(wildcard include/config/page/debug/something/else.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/mmu.h \
    $(wildcard include/config/cpu/has/asid.h) \
  include/linux/net.h \
  include/linux/random.h \
  include/linux/ioctl.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/irqnr.h \
    $(wildcard include/config/generic/hardirqs.h) \
  include/linux/fcntl.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
  include/linux/textsearch.h \
  include/linux/err.h \
  include/net/checksum.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/uaccess.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/checksum.h \
  include/linux/in6.h \
  include/linux/dmaengine.h \
    $(wildcard include/config/async/tx/enable/channel/switch.h) \
    $(wildcard include/config/dma/engine.h) \
    $(wildcard include/config/async/tx/dma.h) \
  include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
    $(wildcard include/config/devtmpfs.h) \
    $(wildcard include/config/sysfs/deprecated.h) \
  include/linux/ioport.h \
  include/linux/klist.h \
  include/linux/pm.h \
    $(wildcard include/config/pm.h) \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/pm/runtime.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/device.h \
    $(wildcard include/config/dmabounce.h) \
  include/linux/pm_wakeup.h \
  include/linux/dma-mapping.h \
    $(wildcard include/config/has/dma.h) \
    $(wildcard include/config/have/dma/attrs.h) \
    $(wildcard include/config/need/dma/map/state.h) \
  include/linux/dma-attrs.h \
  include/linux/bug.h \
  include/linux/scatterlist.h \
    $(wildcard include/config/debug/sg.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/scatterlist.h \
  include/asm-generic/scatterlist.h \
    $(wildcard include/config/need/sg/dma/length.h) \
  include/linux/mm.h \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/ksm.h) \
    $(wildcard include/config/debug/pagealloc.h) \
    $(wildcard include/config/hibernation.h) \
    $(wildcard include/config/hugetlbfs.h) \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  include/linux/range.h \
  include/linux/bit_spinlock.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/pgtable.h \
    $(wildcard include/config/highpte.h) \
  include/asm-generic/4level-fixup.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/proc-fns.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/glue-proc.h \
    $(wildcard include/config/cpu/arm610.h) \
    $(wildcard include/config/cpu/arm7tdmi.h) \
    $(wildcard include/config/cpu/arm710.h) \
    $(wildcard include/config/cpu/arm720t.h) \
    $(wildcard include/config/cpu/arm740t.h) \
    $(wildcard include/config/cpu/arm9tdmi.h) \
    $(wildcard include/config/cpu/arm920t.h) \
    $(wildcard include/config/cpu/arm922t.h) \
    $(wildcard include/config/cpu/arm925t.h) \
    $(wildcard include/config/cpu/arm926t.h) \
    $(wildcard include/config/cpu/arm940t.h) \
    $(wildcard include/config/cpu/arm946e.h) \
    $(wildcard include/config/cpu/arm1020.h) \
    $(wildcard include/config/cpu/arm1020e.h) \
    $(wildcard include/config/cpu/arm1022.h) \
    $(wildcard include/config/cpu/arm1026.h) \
    $(wildcard include/config/cpu/mohawk.h) \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/cpu/v6k.h) \
    $(wildcard include/config/cpu/v7.h) \
  arch/arm/mach-hi3518/include/mach/vmalloc.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/pgtable-hwdef.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/pageflags/extended.h) \
    $(wildcard include/config/arch/uses/pg/uncached.h) \
    $(wildcard include/config/memory/failure.h) \
    $(wildcard include/config/swap.h) \
    $(wildcard include/config/s390.h) \
  include/linux/huge_mm.h \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
  include/linux/vm_event_item.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/io.h \
  arch/arm/mach-hi3518/include/mach/io.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/dma-mapping.h \
  include/linux/dma-debug.h \
    $(wildcard include/config/dma/api/debug.h) \
  include/asm-generic/dma-coherent.h \
    $(wildcard include/config/have/generic/dma/coherent.h) \
  include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
    $(wildcard include/config/timerfd.h) \
  include/linux/timerqueue.h \
  include/linux/if_packet.h \
  include/linux/if_link.h \
  include/linux/netlink.h \
  include/linux/capability.h \
  include/linux/pm_qos_params.h \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/miscdevice.h \
  include/linux/major.h \
  include/linux/delay.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/delay.h \
  include/linux/rculist.h \
  include/linux/ethtool.h \
  include/linux/compat.h \
    $(wildcard include/config/nfsd.h) \
    $(wildcard include/config/nfsd/deprecated.h) \
  include/net/net_namespace.h \
    $(wildcard include/config/ipv6.h) \
    $(wildcard include/config/ip/dccp.h) \
    $(wildcard include/config/netfilter.h) \
    $(wildcard include/config/wext/core.h) \
    $(wildcard include/config/net.h) \
  include/net/netns/core.h \
  include/net/netns/mib.h \
    $(wildcard include/config/xfrm/statistics.h) \
  include/net/snmp.h \
  include/linux/snmp.h \
  include/linux/u64_stats_sync.h \
  include/net/netns/unix.h \
  include/net/netns/packet.h \
  include/net/netns/ipv4.h \
    $(wildcard include/config/ip/multiple/tables.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/ip/mroute.h) \
    $(wildcard include/config/ip/mroute/multiple/tables.h) \
  include/net/inet_frag.h \
  include/net/netns/ipv6.h \
    $(wildcard include/config/ipv6/multiple/tables.h) \
    $(wildcard include/config/ipv6/mroute.h) \
    $(wildcard include/config/ipv6/mroute/multiple/tables.h) \
  include/net/dst_ops.h \
  include/linux/percpu_counter.h \
  include/net/netns/dccp.h \
  include/net/netns/x_tables.h \
    $(wildcard include/config/bridge/nf/ebtables.h) \
  include/linux/netfilter.h \
    $(wildcard include/config/netfilter/debug.h) \
    $(wildcard include/config/nf/nat/needed.h) \
  include/linux/in.h \
  include/net/netns/xfrm.h \
  include/linux/xfrm.h \
  include/linux/seq_file_net.h \
  include/linux/seq_file.h \
  include/net/dsa.h \
  include/linux/interrupt.h \
    $(wildcard include/config/irq/forced/threading.h) \
    $(wildcard include/config/generic/irq/probe.h) \
  include/linux/irqreturn.h \
  include/linux/hardirq.h \
    $(wildcard include/config/virt/cpu/accounting.h) \
    $(wildcard include/config/irq/time/accounting.h) \
  include/linux/ftrace_irq.h \
    $(wildcard include/config/ftrace/nmi/enter.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/hardirq.h \
    $(wildcard include/config/local/timers.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/irq.h \
  arch/arm/mach-hi3518/include/mach/irqs.h \
  include/linux/irq_cpustat.h \
  include/trace/events/irq.h \
  include/linux/circ_buf.h \
  include/linux/semaphore.h \
  include/linux/sem.h \
    $(wildcard include/config/sysvipc.h) \
  include/linux/ipc.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/ipcbuf.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/sembuf.h \
  include/linux/sched.h \
    $(wildcard include/config/sched/debug.h) \
    $(wildcard include/config/lockup/detector.h) \
    $(wildcard include/config/detect/hung/task.h) \
    $(wildcard include/config/core/dump/default/elf/headers.h) \
    $(wildcard include/config/sched/autogroup.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/audit.h) \
    $(wildcard include/config/cgroups.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/fanotify.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/posix/mqueue.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/perf/events.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/fair/group/sched.h) \
    $(wildcard include/config/rt/group/sched.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/rcu/boost.h) \
    $(wildcard include/config/compat/brk.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/futex.h) \
    $(wildcard include/config/fault/injection.h) \
    $(wildcard include/config/latencytop.h) \
    $(wildcard include/config/have/unstable/sched/clock.h) \
    $(wildcard include/config/debug/stack/usage.h) \
    $(wildcard include/config/cgroup/sched.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/signal.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/signal.h \
  include/asm-generic/signal-defs.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/sigcontext.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/pid.h \
  include/linux/proportions.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/resource.h \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  include/linux/latencytop.h \
  include/linux/cred.h \
    $(wildcard include/config/debug/credentials.h) \
    $(wildcard include/config/user/ns.h) \
  include/linux/key.h \
  include/linux/selinux.h \
    $(wildcard include/config/security/selinux.h) \
  include/linux/aio.h \
  include/linux/aio_abi.h \
  include/linux/etherdevice.h \
    $(wildcard include/config/have/efficient/unaligned/access.h) \
  /root/nfs/gm_ipc/hi3518a_sdk/Hi3518_SDK_V1.0.9.0/osdrv/kernel/linux-3.0.y/arch/arm/include/asm/unaligned.h \
  include/linux/unaligned/le_byteshift.h \
  include/linux/unaligned/be_byteshift.h \
  include/linux/unaligned/generic.h \
  include/linux/wireless.h \
  include/net/iw_handler.h \
    $(wildcard include/config/wext/priv.h) \
  include/linux/if_arp.h \
  include/linux/rtnetlink.h \
  include/linux/if_addr.h \
  include/linux/neighbour.h \
  include/linux/ip.h \
  include/linux/kthread.h \
  include/linux/vmalloc.h \
  include/linux/usb.h \
    $(wildcard include/config/usb/devicefs.h) \
    $(wildcard include/config/usb/mon.h) \
    $(wildcard include/config/usb/device/class.h) \
  include/linux/mod_devicetable.h \
  include/linux/usb/ch9.h \
    $(wildcard include/config/size.h) \
    $(wildcard include/config/att/one.h) \
    $(wildcard include/config/att/selfpower.h) \
    $(wildcard include/config/att/wakeup.h) \
    $(wildcard include/config/att/battery.h) \
  include/linux/fs.h \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/fsnotify.h) \
    $(wildcard include/config/ima.h) \
    $(wildcard include/config/fs/posix/acl.h) \
    $(wildcard include/config/debug/writecount.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
  include/linux/limits.h \
  include/linux/blk_types.h \
    $(wildcard include/config/blk/dev/integrity.h) \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
  include/linux/rculist_bl.h \
  include/linux/list_bl.h \
  include/linux/path.h \
  include/linux/radix-tree.h \
  include/linux/fiemap.h \
  include/linux/quota.h \
    $(wildcard include/config/quota/netlink/interface.h) \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/dqblk_qtree.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/inet.h \
  include/linux/pm_runtime.h \
    $(wildcard include/config/pm/runtime/clk.h) \
    $(wildcard include/config/have/clk.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_byteorder.h \
    $(wildcard include/config/platform/mstar389.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/byteorder/little_endian.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/wlan_bssdef.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/wifi.h \
    $(wildcard include/config/rtl8712fw.h) \
    $(wildcard include/config/error.h) \
    $(wildcard include/config/method/flash.h) \
    $(wildcard include/config/method/ethernet.h) \
    $(wildcard include/config/method/label.h) \
    $(wildcard include/config/method/display.h) \
    $(wildcard include/config/method/e/nfc.h) \
    $(wildcard include/config/method/i/nfc.h) \
    $(wildcard include/config/method/nfc.h) \
    $(wildcard include/config/method/pbc.h) \
    $(wildcard include/config/method/keypad.h) \
    $(wildcard include/config/method/vpbc.h) \
    $(wildcard include/config/method/ppbc.h) \
    $(wildcard include/config/method/vdisplay.h) \
    $(wildcard include/config/method/pdisplay.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/ieee80211.h \
    $(wildcard include/config/rtl8711fw.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/drv_types_linux.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_debug.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_rf.h \
    $(wildcard include/config/.h) \
    $(wildcard include/config/1t.h) \
    $(wildcard include/config/2t.h) \
    $(wildcard include/config/1r.h) \
    $(wildcard include/config/2r.h) \
    $(wildcard include/config/1t2r.h) \
    $(wildcard include/config/turbo.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_ht.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_cmd.h \
    $(wildcard include/config/event/thread/mode.h) \
    $(wildcard include/config/c2h/wk.h) \
    $(wildcard include/config/c2h/packet/en.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/cmd_osdep.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_security.h \
    $(wildcard include/config/gtk/ol.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_xmit.h \
    $(wildcard include/config/tx/aggregation.h) \
    $(wildcard include/config/platform/arm/sunxi.h) \
    $(wildcard include/config/platform/arm/sun6i.h) \
    $(wildcard include/config/platform/arm/sun7i.h) \
    $(wildcard include/config/platform/arm/sun8i.h) \
    $(wildcard include/config/platform/mstar.h) \
    $(wildcard include/config/rtl8812a.h) \
    $(wildcard include/config/rtl8821a.h) \
    $(wildcard include/config/rtl8192e.h) \
    $(wildcard include/config/rtl8723b.h) \
    $(wildcard include/config/rtl8192d.h) \
    $(wildcard include/config/sdio/tx/tasklet.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/xmit_osdep.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_recv.h \
    $(wildcard include/config/recv/thread/mode.h) \
    $(wildcard include/config/rx/indicate/queue.h) \
    $(wildcard include/config/bsd/rx/use/mbuf.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/recv_osdep.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_efuse.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_sreset.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_intf.h \
    $(wildcard include/config/wowlan.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_com.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/HalVerDef.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_pg.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_phy.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_phy_reg.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_com_reg.h \
    $(wildcard include/config/usedk.h) \
    $(wildcard include/config/no/usedk.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_com_phycfg.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_com_led.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_qos.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_pwrctrl.h \
    $(wildcard include/config/lps/rpwm/timer.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_mlme.h \
    $(wildcard include/config/arp/keep/alive.h) \
    $(wildcard include/config/dfs.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/mlme_osdep.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_io.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_ioctl.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_ioctl_set.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_ioctl_query.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_ioctl_rtl.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/osdep_intf.h \
    $(wildcard include/config/r871x/test.h) \
    $(wildcard include/config/rf/gain/offset.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/../os_dep/linux/rtw_proc.h \
  include/linux/proc_fs.h \
    $(wildcard include/config/proc/devicetree.h) \
    $(wildcard include/config/proc/kcore.h) \
  include/linux/magic.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_eeprom.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/sta_info.h \
    $(wildcard include/config/atmel/rc/patch.h) \
    $(wildcard include/config/auto/ap/mode.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_event.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_mlme_ext.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_ap.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_version.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_odm.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_p2p.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_mp.h \
    $(wildcard include/config/rtl8192c.h) \
    $(wildcard include/config/rtl8723a.h) \
    $(wildcard include/config/rtl8188e.h) \
    $(wildcard include/config/txt.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_br_ext.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_iol.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/ip.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/if_ether.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/ethernet.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/circ_buf.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtw_android.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/usb_osintf.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/usb_vendor_req.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/usb_ops.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/usb_ops_linux.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/usb_hal.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm.h \
    $(wildcard include/config/phy/reg/pg/value/type.h) \
    $(wildcard include/config/bb/phy/reg.h) \
    $(wildcard include/config/bb/agc/tab.h) \
    $(wildcard include/config/bb/agc/tab/2g.h) \
    $(wildcard include/config/bb/agc/tab/5g.h) \
    $(wildcard include/config/bb/phy/reg/pg.h) \
    $(wildcard include/config/bb/phy/reg/mp.h) \
    $(wildcard include/config/bb/agc/tab/diff.h) \
    $(wildcard include/config/rf/radio.h) \
    $(wildcard include/config/rf/txpwr/lmt.h) \
    $(wildcard include/config/fw/nic.h) \
    $(wildcard include/config/fw/nic/2.h) \
    $(wildcard include/config/fw/ap.h) \
    $(wildcard include/config/fw/mp.h) \
    $(wildcard include/config/fw/wowlan.h) \
    $(wildcard include/config/fw/wowlan/2.h) \
    $(wildcard include/config/fw/ap/wowlan.h) \
    $(wildcard include/config/fw/bt.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_HWConfig.h \
    $(wildcard include/config/mp.h) \
    $(wildcard include/config/tc.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_debug.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_RegDefine11AC.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_RegDefine11N.h \
    $(wildcard include/config/anta/11n.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_AntDiv.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/HalPhyRf.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/HalPhyRf_8188e.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/../HalPhyRf.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/Hal8188ERateAdaptive.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_hal.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/hal_data.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/../hal/OUTSRC/odm_precomp.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_spec.h \
    $(wildcard include/config/err.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/Hal8188EPhyReg.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/Hal8188EPhyCfg.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_rf.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_dm.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_recv.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8192c_recv.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_xmit.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_cmd.h \
    $(wildcard include/config/eid.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_led.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/Hal8188EPwrSeq.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/HalPwrSeqCmd.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/include/rtl8188e_sreset.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_reg.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/HalHWImg8188E_MAC.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/HalHWImg8188E_RF.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/HalHWImg8188E_BB.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/HalHWImg8188E_FW.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/Hal8188EReg.h \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/odm_RegConfig8188E.h \
    $(wildcard include/config/h/8188e.h) \
  /root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/rtl8188e/odm_RTL8188E.h \

/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.o: $(deps_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.o)

$(deps_/root/nfs/git_ipc/gm_ipc/driver/rtl8188eus_v4.3.0.3/hal/OUTSRC/odm_interface.o):
