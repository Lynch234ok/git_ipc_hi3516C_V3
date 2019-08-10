
/**
 * AES Crypt.
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#if !defined(NK_UTILS_AES_H_)
# define NK_UTILS_AES_H_
NK_CPP_EXTERN_BEGIN

/**
 * @brief
 *  AES 模块句柄。\n
 */
typedef struct Nk_AES {
#define NK_This struct Nk_AES *const

	/**
	 * @brief
	 *  模块基础接口。
	 */
	NK_Object Object;

	/**
	 * @brief
	 *  数据加密编码。\n
	 *
	 * @param[in] origin
	 * 	原始数据。\n
	 *
	 * @param[in] len
	 *  原始数据长度。\n
	 *
	 * @param[out] crypto
	 *  加密编码输出，当编码成功后从这里返回加密后的数据，所在栈区内存长度必须大于等于 @ref len 最小 16 大端对齐长度，\n
	 *  例如 @ref len 的长度为 1，@ref crypto 所在栈区长度必须大于等于 16。\n
	 *
	 * @retval -1
	 *  编码失败，可能是参数错误。\n
	 *
	 * @retval 译码原始数据长度
	 *  编码成功，编码加密数据输出到 @ref crypto 所在栈区内存位置。\n
	 *
	 */
	NK_SSize
	(*encrypt)(NK_This, NK_PByte origin, NK_Size len, NK_PByte crypto);

	/**
	 * @brief
	 *  数据解密译码。\n
	 *
	 * @param[in] crypto
	 * 	已加密编码的数据，长度必须 16 字节对齐。\n
	 *
	 * @param[in] len
	 *  已加密编码的数据长度，必须 16 字节对齐。\n
	 *
	 * @param[out] origin
	 *  解密译码输出，当译码成功后从这里返回原始数据，所在栈区内存长度必须大于等于 @ref len。\n
	 *
	 * @retval -1
	 *  译码失败，可能是参数错误。\n
	 *
	 * @retval 译码原始数据长度
	 *  译码成功，译码原始数据输出到 @ref origin 所在栈区内存位置。\n
	 *
	 */
	NK_SSize
	(*decrypt)(NK_This, NK_PByte crypto, NK_Size len, NK_PByte origin);

#undef NK_This
} NK_AES;


/**
 * @brief
 *  创建 AES 模块句柄。\n
 *
 * @param[in] Alloctr
 *  内存分配器。\n
 *
 * @param[in] key
 *  用户密钥，密钥长度可以为 16/24/32 字节，分别对应 AES128/AES192/AES256 编译码运算。\n
 *
 * @param[in] key_len
 *  用户密钥长度，值只能是 16/24/32，传入其他值会创建句柄失败。\n
 *
 * @retval AES 模块句柄
 *  创建成功。\n
 *
 * @retval NK_Nil
 *  创建失败，可能是参数错误导致。\n
 *
 */
NK_API NK_AES *
NK_AES_Create(NK_Allocator *Alloctr, const NK_PByte key, NK_Size key_len);

/**
 * @brief
 *  销毁 AES 模块句柄。\n
 *
 * @param[in] AES_r
 *  AES 模块句柄内存引用。\n
 *
 * @retval 0
 *  销毁成功。\n
 *
 * @retval -1
 *  销毁失败。\n
 */
NK_API NK_Int
NK_AES_Free(NK_AES **AES_r);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_AES_H_ */

