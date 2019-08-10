/**
 *
 * MD5 运算类。
 *
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#if !defined(NK_UTILS_MD5_H_)
# define NK_UTILS_MD5_H_
NK_CPP_EXTERN_BEGIN

/**
 * MD5 哈希运算长度。
 */
#define NK_MD5_HASH_SZ (16)

/**
 * MD5 哈希运算明文长度。
 */
#define NK_MD5_HASH_PLAIN_SZ (NK_MD5_HASH_SZ * 2)

/**
 * CheckSum 模块句柄。
 */
typedef struct Nk_CheckSum {
#define NK_This struct Nk_CheckSum *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

	/**
	 * @brief
	 *  更新累计数据。
	 *
	 * @param bytes [in]
	 *  运算的数据序列。
	 * @param len [in]
	 *  数据序列长度。
	 *
	 * @return
	 *  更新成功返回 0，否则返回 -1。
	 *
	 */
	NK_Int
	(*update)(NK_This, const NK_PVoid bytes, NK_Size len);

	/**
	 * @brief
	 *  计算截至当前累加值的结果。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param stack [out]
	 *  运算结果缓冲。
	 * @param stacklen [in]
	 *  传入栈缓冲 @ref stack 长度。
	 *
	 * @retval 0			获取结果成功。
	 * @retval -1			获取结果失败。
	 */
	NK_PChar
	(*result)(NK_This, NK_Boolean plain, NK_PVoid stack, NK_Size stacklen);


#undef NK_This
} NK_CheckSum;


/**
 * 创建 MD5 模块句柄。
 */
NK_API NK_CheckSum *
NK_MD5_Create(NK_Allocator *Alloctr);

/**
 * 销毁 MD5 模块句柄。
 */
NK_API NK_Void
NK_MD5_Free(NK_CheckSum **MD5_r);

/**
 * @macro
 *  快速转换宏，@ref __md5 所在栈区长度必须大于 @ref NK_MD5_HASH_PLAIN_SZ。
 */
#define NK_MD5_HASH_BUF(__buf, __buf_len, __plain, __md5) \
	do {\
		NK_CheckSum *__CheckSum;\
		NK_Allocator *__Alloctr = NK_Nil;\
		NK_Byte __mem[1024];\
		\
		__Alloctr = NK_Alloc_Create(__mem, sizeof(__mem));\
		\
		if (NK_Nil != __Alloctr) {\
			__CheckSum = NK_MD5_Create(__Alloctr);\
			if (NK_Nil != __CheckSum) {\
				__CheckSum->update(__CheckSum, __buf, __buf_len);\
				__CheckSum->result(__CheckSum, __plain, (__md5), NK_MD5_HASH_PLAIN_SZ + 1);\
				NK_MD5_Free(&__CheckSum);\
			}\
			NK_Alloc_Free(&__Alloctr, NK_Nil);\
		}\
	} while (0)

/**
 * @macro
 *  快速转换宏定义。
 */
#define NK_MD5_HASH_STR(__str, __plain, __md5) NK_MD5_HASH_BUF((__str), strlen(__str), (__plain), (__md5))




NK_CPP_EXTERN_END
#endif /* NK_UTILS_MD5_H_ */
