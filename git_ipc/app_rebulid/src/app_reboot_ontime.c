#ifdef REBOOT_ONTIME
#include "app_reboot_ontime.h"
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <NkUtils/log.h>
#include <NkUtils/assert.h>
#include <time.h>
#include <base/ja_process.h>
#include <app_debug.h>
#include <string.h>
#include "global_runtime.h"
#include "generic.h"

static NK_Boolean is_reboot_ontime_enable()
{
    FILE *pFile;
    char buf[128] = {0};
    char *index, *addr;
    int flag = 0;
    /* 判断G_DEBUG_FLAG_FILE字段reboot是否为0，
       是则返回false表示disable此功能，默认返回true */
    if(IS_FILE_EXIST(G_DEBUG_FLAG_FILE)) {
        pFile = fopen(G_DEBUG_FLAG_FILE, "rb");
        if(pFile) {
            /* 只是简单做了个读取ini文件内容的操作 */
            if(fread(buf, 1, sizeof(buf), pFile) > 0) {
                index = strstr(buf, "reboot");
                if(index) {
                    addr = index + strlen("reboot=");
                    if(addr) {
                        flag = atoi(addr);
                        if(flag == 0) {
							fclose(pFile);
                            return NK_False;
                        }
                        else {
							fclose(pFile);
                            return NK_True;
                        }
                    }
                }
            }
            fclose(pFile);
        }
    }

	return NK_True;
}

static void *before_reboot_system(void)
{
#if defined(WIFI)
	APP_WIFI_exit_wifi();
#endif
#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
    NK_TFRECORD_Stop(0);
#else
#if defined(TFCARD)
	TFCARD_destroy();
#endif
#endif
}

NK_Boolean REBOOT_ONTIME_is_flag_exist()
{
	return NK_REBOOT_ONTIME_is_flag_exist();
}


NK_Int REBOOT_ONTIME_init(NK_Int hourNum)
{
	if(is_reboot_ontime_enable()){
		NK_REBOOT_ONTIME_init(hourNum, before_reboot_system);
	}else{
		APP_TRACE("please enable the reboot_ontime switch in configfile!");
	}
	
	return 0;
}

NK_Void REBOOT_ONTIME_destroy()
{
	NK_REBOOT_ONTIME_destroy();
}

#endif
