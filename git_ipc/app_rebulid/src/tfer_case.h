
#include "NkEmbedded/tfer.h"
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <errno.h>
#include <stdarg.h>
#include <NkUtils/log.h>
#include "generic.h"


static inline NK_Int
SCRIPT(NK_PChar fmt, ...)
{
	NK_Char script[1024 * 4];
	va_list var;

	va_start(var, fmt);
	vsnprintf(script, sizeof(script), fmt, var);
	va_end(var);

	return system(script);
}

static inline NK_SSize
SCRIPT2(NK_PChar result, NK_Size result_max, NK_PChar fmt, ...)
{
	NK_Char script[1024 * 4];
	va_list var;
	FILE *fID = NK_Nil;
	NK_SSize readn = -1;

	va_start(var, fmt);
	vsnprintf(script, sizeof(script), fmt, var);
	va_end(var);

	fID = popen(script, "r");
	if (!fID) {
		NK_Log()->error("Test: /bin/sh \"%s\" Error.", script);
		return -1;
	}

	readn = fread(result, 1, result_max, fID);
	pclose(fID);
	fID = NK_Nil;

	return readn;
}


static NK_Boolean
tf_onDetectTF(NK_PVoid ctx, NK_Int id)
{
	return IS_FILE_EXIST("/dev/mmcblk0p1");
}

static NK_Int
tf_onMountTF(NK_PVoid ctx, NK_Int id, NK_PChar fs_path)
{
	SCRIPT("mkdir -p %s", fs_path);

	//if(0 == mount(deviceName, mountDIR, "vfat", 0, NULL)){
	if(0 == mount("/dev/mmcblk0p1", fs_path, "vfat", MS_NOATIME, NULL)){

		/// 挂在成功。
		NK_Log()->info("Recorder: Mount Record Disk.");

		// success to mount
		SCRIPT("ls -l %s/*.REC", fs_path);
		SCRIPT("rm -f %s/*.REC", fs_path);
		SCRIPT("mount");

		return 0;

	}

	if (EBUSY == errno) {

		/// 上个运行周期已经挂载。
		NK_Log()->info("Recorder: TF Mounted.");
		return 0;

	} else {

	}

	//perror("mount");
	NK_Log()->error("Recorder: Mount Record Disk Failed (%d, %s).", errno, strerror(errno));

	return -1;
}

static NK_Int
tf_onUmountTF(NK_PVoid ctx, NK_Int id, NK_PChar fs_path)
{
	if (0 == umount2(fs_path, MNT_DETACH)) { // like 'umount -l'
		NK_Log()->info("Recorder: Unmount Record Disk.");
		SCRIPT("mount");
		return 0;
	}

	return -1;
}


static NK_Size
tf_onGetCapacity(NK_PVoid ctx, NK_Int id, NK_PChar fs_path)
{
	struct statvfs StatFS;
	NK_Int64 capacity = 0;

	/**
	 * 使用系统方法获取文件系统空间大小。
	 */
	if (statvfs(fs_path, &StatFS) < 0) {
		return 0;
	}

	capacity = StatFS.f_blocks;
	capacity *= StatFS.f_bsize;
	/**
	 * 转换成 MB 单位。
	 */
	capacity /= 1024;
	capacity /= 1024;

	NK_Log()->error("Case: TF Capacity %dMB.", (NK_SSize)capacity);
	return (NK_Size)capacity;
}


static NK_Size
tf_onGetFreeSpace(NK_PVoid ctx, NK_Int id, NK_PChar fs_path)
{
	struct statvfs StatFS;
	NK_Int64 free_size = 0;

	/**
	 * 使用系统方法获取文件系统空间大小。
	 */
	if (statvfs(fs_path, &StatFS) < 0) {
		return 0;
	}

	free_size = StatFS.f_bavail;
	free_size *= StatFS.f_bsize;
	free_size /= 1024;
	free_size /= 1024; ///< 转换成 MB 单位。

	NK_Log()->debug("Case: TF Free Space %dMB.", (NK_SSize)free_size);
	return (NK_Size)free_size;
}

static void tfer_case()
{
	NK_TFerEventSet TFerEventSet;
	TFerEventSet.onDetectTF = tf_onDetectTF;
	TFerEventSet.onMountTF = tf_onMountTF;
	TFerEventSet.onUmountTF = tf_onUmountTF;
	TFerEventSet.onGetCapacity = tf_onGetCapacity;
	TFerEventSet.onGetFreeSpace = tf_onGetFreeSpace;

	NK_TFer *TFer = NK_TFer_Create(NK_Nil, 1, &TFerEventSet, NK_Nil);

	getchar();
	getchar();

	NK_TFer_Free(&TFer);

}

