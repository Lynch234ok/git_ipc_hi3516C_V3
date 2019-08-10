
/**
 * ���� X264 �ļ����ڴ�ʵ����Ч���ٵ�֡ץȡ��
 * �����ļ�������ý����ԡ�
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
 * ���� 264 �ļ����ڴ档
 * ���ڻ���ʱ�����ļ����������ڴ��У�����ʱ���ڴ����ıȽϴ�
 * ����������Ե���Ƶ�ļ�������̫��
 *
 * @param[in]		file_path			264 �ļ�����·����
 *
 * @return		ģ������
 */
extern NK_X264Cache *
X264Cache_Open(const NK_PChar file_path);

/**
 * �ر��ļ���������
 *
 */
extern NK_Void
X264Cache_Close(NK_X264Cache *Cache);


/**
 * Դ���롣
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
 * H.264 NALU ����ʼ�붨�塣
 */
static const NK_Byte
H264_LONG_NALU_START_CODE[] = { 0x00, 0x00, 0x00, 0x01, };

/**
 * H.264 NALU �̿�ʼ�붨�塣
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
	 * ƫ��һ�»��塣
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
		 * �ؼ�֡��Ҫ���һ�¡�
		 */
		do {

			NK_PByte sub_nalu = NK_Nil;
			NK_Size sub_nalu_start_code_len = 0;
			NK_Size sub_nalu_len = 0;

			type = 0;
			NK_EXPECT_VERBOSE_RETURN(0 == NK_H264Utils_ExtractNALU(Cache->offset, Cache->mem + Cache->size - Cache->offset,
						&sub_nalu, &sub_nalu_start_code_len, &sub_nalu_len), -1);

			/**
			 * ƫ��һ�»��塣
			 */
			Cache->offset += sub_nalu_len;
			if (Cache->offset == Cache->mem + Cache->size) {
				Cache->offset = Cache->mem;
			}

			/**
			 * �ı� nalu ���ȡ�
			 */
			a_nalu_len += sub_nalu_len;

			/**
			 * ��ȡ���͡�
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

	/// ���Դ��ļ���
	fID = fopen(file_path, "rb");
	if (!fID) {
		NK_Log()->error("x264: Open File (Path = %s) Failed.", file_path);
		return NK_Nil;
	}

	/// ��ʼ�������

	/// ��ʼ���ļ����塣
	Cache = (NK_X264Cache *)(calloc(sizeof(NK_X264Cache), 1));
	/// �����ļ����ȡ�
	fseek(fID, 0, SEEK_END);
	Cache->size = ftell(fID);
	fseek(fID, 0, SEEK_SET);

	/// ��ȡ�ļ��������С�
	Cache->mem = calloc(Cache->size, 1);
	Cache->offset = Cache->mem;
	readn = fread(Cache->mem, 1, Cache->size, fID);
	Cache->counter = 0;

	NK_Log()->debug("x264: File (Path = %s Size = %u) Cached.",
			file_path, Cache->size);

	/// �ر��ļ���
	fclose(fID);
	fID = NK_Nil;

	/// �����ӿڡ�
	Cache->fetch = x264Cache_fetch;

	return Cache;

}

NK_Void
X264Cache_Close(NK_X264Cache *Cache)
{
	/// �ͷ��ļ����������ġ�
	free(Cache->mem);
	Cache->mem = NK_Nil;

	free(Cache);
	Cache = NK_Nil;

	free(Cache);
	Cache = NK_Nil;
}


NK_CPP_EXTERN_END
#endif /* N1_TEST_X264_CACHE_H_ */
