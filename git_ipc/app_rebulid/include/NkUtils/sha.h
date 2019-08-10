
/**
 *
 * SHA1 运算类。
 *
 */

#include <NkUtils/md5.h>

#if !defined(NK_UTILS_SHA_H_)
# define NK_UTILS_SHA_H_
NK_CPP_EXTERN_BEGIN


/**
 * @macro
 *  SHAn 哈希运算长度。
 */
#define NK_SHA_HASH_SZ(__n) \
	((1 == (__n)) ? 20 : 0)

/**
 * @macro
 *  SHAn 哈希运算明文长度。
 */
#define NK_SHA_HASH_PLAIN_SZ(__n) (NK_SHA_HASH_SZ(__n) * 2)

/**
 * @macro
 *  SHA1 哈希运算长度。
 */
#define NK_SHA1_HASH_SZ (NK_SHA_HASH_SZ(1))

/**
 * @macro
 *  SHA1 哈希运算明文长度。
 */
#define NK_SHA1_HASH_PLAIN_SZ (NK_SHA_HASH_PLAIN_SZ(1))

/**
 * @brief
 *  创建 SHAn 模块句柄。
 *
 * @param Alloctr [in]
 *  内存分配器。
 * @param n [in]
 *  对应 SHA1，SHA224，SHA256，SHA512 等，目前只支持 SHA1，此值为 1。
 *
 * @return
 *  创建成功返回句柄，失败返回 -1。
 *
 */
NK_API NK_CheckSum *
NK_SHA_Create(NK_Allocator *Alloctr, NK_Size n);

/**、
 * @brief
 *  销毁 SHAn 模块句柄。
 */
NK_API NK_Void
NK_SHA_Free(NK_CheckSum **SHA1_r);


/**
 * @macro
 *  快速转换宏，@ref __sha1 所在栈区长度必须大于 @ref NK_SHA1_HASH_PLAIN_SZ。
 */
#define NK_SHA_HASH_BUF(__n, __buf, __buf_len, __plain, __sha1) \
	do {\
		NK_CheckSum *__CheckSum;\
		NK_Allocator *__Alloctr = NK_Nil;\
		NK_Byte __mem[1024];\
		\
		__Alloctr = NK_Alloc_Create(__mem, sizeof(__mem));\
		\
		if (NK_Nil != __Alloctr) {\
			__CheckSum = NK_SHA_Create(__Alloctr, __n);\
			if (NK_Nil != __CheckSum) {\
				__CheckSum->update(__CheckSum, __buf, __buf_len);\
				__CheckSum->result(__CheckSum, __plain, (__sha1), NK_SHA1_HASH_PLAIN_SZ + 1);\
				NK_SHA_Free(&__CheckSum);\
			}\
			NK_Alloc_Free(&__Alloctr, NK_Nil);\
		}\
	} while (0)

/**
 * @macro
 *  快速转换宏定义。
 */
#define NK_SHA_HASH_STR(__n, __str, __plain, __sha1) NK_SHA_HASH_BUF((__n), (__str), strlen(__str), (__plain), (__sha1))

/**
 * @macro
 *  SHA1 缓冲快速运算转换。
 */
#define NK_SHA1_HASH_BUF(__buf, __buf_len, __plain, __sha1) NK_SHA_HASH_BUF(1, __buf, __buf_len, __plain, __sha1)

/**
 * @macro
 *  SHA1 字符串快速转换宏定义。
 */
#define NK_SHA1_HASH_STR(__str, __plain, __sha1) NK_SHA_HASH_BUF(1, (__str), strlen(__str), (__plain), (__sha1))


NK_CPP_EXTERN_END
#endif /* NK_UTILS_SHA_H_ */
