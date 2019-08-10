
/**
 * 缓冲 X264 文件到内存实现有效快速的帧抓取，
 * 用于文件仿真流媒体测试。
 */


#include <NkUtils/types.h>
#include <NkUtils/h264_utils.h>

#ifndef N1_TEST_X264_CACHE_H_
#define N1_TEST_X264_CACHE_H_
NK_CPP_EXTERN_BEGIN

typedef struct Nk_X264Cache
{
	NK_PByte mem;
	NK_Size size;
	NK_PByte offset;
	NK_SSize counter;

	NK_SSize
	(*fetch)(struct Nk_X264Cache *const Cache, NK_PByte *data);

} NK_X264Cache;

/**
 * 载入 264 文件到内存。
 * 由于缓冲时整个文件均拷贝到内存中，运行时对内存消耗比较大。
 * 因此用作测试的视频文件不建议太大。
 *
 * @param[in]		file_path			264 文件所在路径。
 *
 * @return		模块句柄。
 */
extern NK_X264Cache *
X264Cache_Open(const NK_PChar file_path);

/**
 * 关闭文件缓冲句柄。
 *
 */
extern NK_Void
X264Cache_Close(NK_X264Cache *Cache);


/**
 * 源代码。
 */
#define _GNU_SOURCE
#include <NkUtils/log.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <NkUtils/assert.h>
#include <NkEmbedded/avc_aac_dcr.h>

typedef struct MEDIABUF_USER {
	uint32_t forbidden_zero;
}stMEDIABUF_USER, *lpMEDIABUF_USER;

/**
 * H.264 NALU 长开始码定义。
 */
static const NK_Byte
H264_LONG_NALU_START_CODE[] = { 0x00, 0x00, 0x00, 0x01, };

/**
 * H.264 NALU 短开始码定义。
 */
static const NK_Byte
H264_SHORT_NALU_START_CODE[] = { 0x00, 0x00, 0x01, };


static NK_SSize
x264Cache_fetch(struct Nk_X264Cache *const Cache, NK_PByte *data)
{
	NK_PByte a_nalu = NK_Nil;
	NK_Size a_nalu_start_code_len = 0;
	NK_Size a_nalu_len = 0;
	NK_Byte type = 0;


	NK_EXPECT_VERBOSE_RETURN (0 == NK_H264Utils_ExtractNALU(Cache->offset, Cache->mem + Cache->size - Cache->offset,
			&a_nalu, &a_nalu_start_code_len, &a_nalu_len), -1);

	type = a_nalu[a_nalu_start_code_len] & 0x1f;

	/**
	 * 偏移一下缓冲。
	 */
	Cache->offset += a_nalu_len;
	if (Cache->offset == Cache->mem + Cache->size) {
		Cache->offset = Cache->mem;
	}

	if (0x01 == type) {

		//NK_HEX_DUMP(a_nalu, 8);
		*data = a_nalu;
		return a_nalu_len;

	} else {

		/**
		 * 关键帧须要组合一下。
		 */
		do {

			NK_PByte sub_nalu = NK_Nil;
			NK_Size sub_nalu_start_code_len = 0;
			NK_Size sub_nalu_len = 0;

			type = 0;
			NK_EXPECT_VERBOSE_RETURN(0 == NK_H264Utils_ExtractNALU(Cache->offset, Cache->mem + Cache->size - Cache->offset,
						&sub_nalu, &sub_nalu_start_code_len, &sub_nalu_len), -1);

			/**
			 * 偏移一下缓冲。
			 */
			Cache->offset += sub_nalu_len;
			if (Cache->offset == Cache->mem + Cache->size) {
				Cache->offset = Cache->mem;
			}

			/**
			 * 改变 nalu 长度。
			 */
			a_nalu_len += sub_nalu_len;

			/**
			 * 获取类型。
			 */
			type = sub_nalu[sub_nalu_start_code_len] & 0x1f;

		} while (0 != type && 0x01 != type && 0x05 != type);

		//NK_HEX_DUMP(a_nalu, 9);
		*data = a_nalu;
		return a_nalu_len;
	}

	return 0;
}


NK_X264Cache *
X264Cache_Open(const NK_PChar file_path)
{
	FILE *fID = NK_Nil;
	NK_X264Cache *Cache = NK_Nil;
	ssize_t readn = 0;

	/// 尝试打开文件。
	fID = fopen(file_path, "rb");
	if (!fID) {
		NK_Log()->error("x264: Open File (Path = %s) Failed.", file_path);
		return NK_Nil;
	}

	/// 初始化句柄。

	/// 初始化文件缓冲。
	Cache = (NK_X264Cache *)(calloc(sizeof(NK_X264Cache), 1));
	/// 计算文件长度。
	fseek(fID, 0, SEEK_END);
	Cache->size = ftell(fID);
	fseek(fID, 0, SEEK_SET);

	/// 读取文件到缓冲中。
	Cache->mem = calloc(Cache->size, 1);
	Cache->offset = Cache->mem;
	readn = fread(Cache->mem, 1, Cache->size, fID);
	Cache->counter = 0;

	NK_Log()->debug("x264: File (Path = %s Size = %u) Cached.",
			file_path, Cache->size);

	/// 关闭文件。
	fclose(fID);
	fID = NK_Nil;

	/// 公共接口。
	Cache->fetch = x264Cache_fetch;

	return Cache;

}

NK_Void
X264Cache_Close(NK_X264Cache *Cache)
{
	/// 释放文件缓冲上下文。
	free(Cache->mem);
	Cache->mem = NK_Nil;

	free(Cache);
	Cache = NK_Nil;

	free(Cache);
	Cache = NK_Nil;
}


NK_CPP_EXTERN_END
#endif /* N1_TEST_X264_CACHE_H_ */
