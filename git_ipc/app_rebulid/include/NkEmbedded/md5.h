/**
 *
 * MD5 运算类。
 *
 */

#include <NkUtils/md5.h>

#if !defined(NK_EMB_MD5_H_)
# define NK_EMB_MD5_H_
NK_CPP_EXTERN_BEGIN

/**
 * @macro 快速编码宏定义。
 *
 */
#define NK_MD5_BIN(__binary, __len, __md5) \
	do {\
		NK_Size len = 64;\
		NK_Byte mem[1024];\
		NK_PByte cache = NK_Nil;\
		NK_Size cache_len = 0;\
		NK_CheckSum *CheckSum;\
		NK_Allocator *Alloctr = NK_Nil;\
		Alloctr = NK_Alloc_Create(mem, sizeof(mem));\
		if (NK_Nil != Alloctr) {\
			CheckSum = NK_MD5_Create(Alloctr);\
			if (NK_Nil != CheckSum) {\
				cache = __binary;\
				cache_len = (__len);\
				while (cache_len > 0) {\
					NK_Size const update_len = cache_len >= 1024 ? 1024 : cache_len;\
					CheckSum->update(CheckSum, cache, update_len);\
					cache += update_len;\
					cache_len -= update_len;\
				}\
				CheckSum->result(CheckSum, (__md5), &len);\
				NK_MD5_Free(&CheckSum);\
			}\
			NK_Alloc_Free(&Alloctr, NK_Nil);\
		}\
	} while (0)

/**
 * @macro 快速编码宏定义。
 *
 */
#define NK_MD5_TEXT(__text, __md5) NK_MD5_BIN(__text, strlen(__text), __md5)


/**
 * @macro 快速编码宏定义。
 *
 */
#define NK_MD5_FILE(__file_path, __md5) \
	do {\
		NK_Size len = 64;\
		NK_Byte mem[1024];\
		NK_Byte cache[1024];\
		NK_Size cache_len = 0;\
		NK_CheckSum *CheckSum;\
		NK_Allocator *Alloctr = NK_Nil;\
		Alloctr = NK_Alloc_Create(mem, sizeof(mem));\
		if (NK_Nil != Alloctr) {\
			CheckSum = NK_MD5_Create(Alloctr);\
			if (NK_Nil != CheckSum) {\
				FILE *fID = fopen(__file_path, "rb");\
				if (NK_Nil != fID) {\
					while ((cache_len = fread(cache, 1, sizeof(cache), fID)) > 0) {\
						CheckSum->update(CheckSum, cache, cache_len);\
					}\
					fclose(fID);\
				}\
				CheckSum->result(CheckSum, NK_True, (__md5), &len);\
				NK_MD5_Free(&CheckSum);\
			}\
			NK_Alloc_Free(&Alloctr, NK_Nil);\
		}\
	} while (0)


NK_CPP_EXTERN_END
#endif /* NK_EMB_MD5_H_ */
