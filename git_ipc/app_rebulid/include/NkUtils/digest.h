/**
 * 摘要综合运算类。
 */

#include <NkUtils/md5.h>
#include <NkUtils/sha1.h>
#include <NkUtils/crc.h>

#if !defined(NK_UTILS_DIGEST_H_)
# define NK_UTILS_DIGEST_H_
NK_CPP_EXTERN_BEGIN

/**
 * 摘要定义分类。
 */
typedef enum Nk_DigestClassify
{
	/**
	 * 未定义。
	 */
	NK_DIGEST_CLASS_UNDEF			= (-1),

	/**
	 * CRC8 运算。
	 */
	NK_DIGEST_CLASS_CRC8,

	/**
	 * CRC16 运算。
	 */
	NK_DIGEST_CLASS_CRC16,

	/**
	 * CRC32 运算。
	 */
	NK_DIGEST_CLASS_CRC32,

	/**
	 * MD5 运算。
	 */
	NK_DIGEST_CLASS_MD5,

	/**
	 * SHA1 运算。
	 */
	NK_DIGEST_CLASS_SHA1,



} NK_DigestClassify;


/**
 * 句柄定义。
 */
typedef NK_CheckSum NK_Digest;


/**
 * 初始化摘要运算类。
 */
#define NK_Digest_Create(__Alloc, __classify) \
	((NK_DIGEST_CLASS_CRC8 == (__classify)) ?\
			NK_Nil /* 未实现。 */ \
			: (NK_DIGEST_CLASS_CRC16 == (__classify)) ?\
					NK_Nil /* 未实现。 */ \
					: (NK_DIGEST_CLASS_CRC32 == (__classify)) ?\
							NK_CRC_Create(__Alloc, NK_CRC_ALG_CRC32) \
							: (NK_DIGEST_CLASS_MD5 == (__classify)) ?\
									NK_MD5_Create(__Alloc) \
									:(NK_DIGEST_CLASS_SHA1 == (__classify)) ?\
											NK_SHA1_Create(__Alloc) \
											: NK_Nil)
/**
 * 释放摘要运算类。
 */
#define NK_Digest_Free(__Digest_r) \
	(__Digest_r)[0]->Object.free((__Digest_r)[0]->Object)


NK_CPP_EXTERN_END
#endif /* NK_UTILS_CHECK_SUM_H_ */
