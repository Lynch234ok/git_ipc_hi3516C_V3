/**
 * 内存分配器实现。
 * 通过此模块可以处理动态内存分配以及管理。
 * 此模块不支持线程安全。
 *
 */

#include "object.h"

#ifndef NK_UTILS_ALLOCATOR_H_
#define NK_UTILS_ALLOCATOR_H_
NK_CPP_EXTERN_BEGIN

/**
 * @brief
 *  内存分配器模块句柄。
 *  通过内存分配器，可以实现对一块相对较大的内存进行动态分配。
 *
 */
typedef struct Nk_Allocator {
#define NK_This struct Nk_Allocator *const Allocator

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

	/**
	 * @brief
	 *  申请内存块。
	 *
	 * @param[in] size
	 *  申请内存块的大小。
	 *
	 * @retval 内存块起始地址
	 *  复制成功。
	 *
	 * @retval Nil
	 *  复制失败，可能内存可用空间不足。
	 *
	 */
	NK_PVoid
	(*alloc)(NK_This, NK_Size size);


	/**
	 * @brief
	 *  重新分配内存。\n
	 *  资源满足的情况下会在原有内存上扩充空间。\n
	 *
	 * @param[in] ptr
	 *  上一次分配的内存空间起始地址。
	 *
	 * @param[in] size
	 *  重新申请的内存大小。
	 *
	 * @retval 内存块起始地址
	 *  复制成功。
	 *
	 * @retval Nil
	 *  复制失败，可能内存可用空间不足，失败后原来申请的内存仍然可以接续使用。\n
	 *  因此要保留原有的内存地址以便释放。
	 */
	NK_PVoid
	(*realloc)(NK_This, NK_PVoid ptr, NK_Size size);

	/**
	 * @brief
	 *  复制字符串到新申请的一块内存空间并返回。\n
	 *  模块会根据传入字符串的长度内部调用 @ref NK_Allocator::alloc() 方法申请合适的内存空间，\n
	 *  然后把字符串数据拷贝到内存空间中，\n
	 *  克隆后得到的内存空间必须通过 @ref NK_Allocator::freep() 方法回收。
	 *
	 * @param[in] str
	 *  复制的字符串。
	 *
	 * @retval 内存块起始地址
	 *  复制成功。
	 *
	 * @retval Nil
	 *  复制失败，可能内存可用空间不足。
	 */
	NK_PChar
	(*strdup)(NK_This, const NK_PChar str);

	/**
	 * @brief
	 *  与 @ref NK_Allocator::strdup() 相似，最大复制 n 个字符。
	 *
	 * @param[in] str
	 *  复制的字符串。
	 *
	 * @param[in] n
	 *  复制字节的最大长度。
	 *
	 * @retval 内存块起始地址
	 *  复制成功。
	 *
	 * @retval Nil
	 *  复制失败，可能内存可用空间不足。
	 */
	NK_PChar
	(*strndup)(NK_This, NK_PChar str, NK_Size n);

	/**
	 * @brief
	 *  复制内存块。\n
	 *  复制以后，会产生一块新的内存块，内用与原内存块相同。\n
	 *  产生的内存块须要通过 @ref NK_Allocator::freep() 方法回收。\n
	 *  调用此接口时必须保证 @ref mem 所在内存块可用长度必须大于等于 @ref size，否则会产生潜在的错误。
	 *
	 * @param[in] mem
	 *  复制的内存块其实空间。
	 *
	 * @param[in] size
	 *  复制字节的最大长度。
	 *
	 * @retval 新内存空间起始地址
	 *  复制成功。
	 *
	 * @retval Nil
	 *  复制失败。
	 */
	NK_PVoid
	(*memdup)(NK_This, NK_PVoid mem, NK_Size size);


	/**
	 * @brief
	 *  回收做分配的内存空间。
	 *
	 * @param[in] ptr
	 *  由 @ref NK_Allocator::alloc() 或者 @ref NK_Allocator::realloc() 分配的内存块起始地址。
	 *
	 */
	NK_Void
	(*freep)(NK_This, NK_PVoid ptr);

	/**
	 * @brief
	 *  获取所分配的内存地址对应内存块大小（单位：字节）。
	 *
	 * @param[in] ptr
	 *  由 @ref NK_Allocator::alloc() 或者 @ref NK_Allocator::realloc() 分配的内存块起始地址。
	 *
	 * @retval 内存地址对应内存块大小（单位：字节）
	 *  获取成功。
	 *
	 */
	NK_SSize
	(*length)(NK_This, NK_PVoid ptr);

	/**
	 * @brief
	 *  获取内存分配器可分配的最大空间（单位：字节）。
	 *
	 * @retval 内存分配器可分配的最大空间（单位：字节）
	 *  获取成功。
	 */
	NK_SSize
	(*size)(NK_This);

	/**
	 * @brief
	 *  获取内存分配器使用过程中曾经的峰值（单位：字节）。
	 *
	 * @retval 内存分配器使用过程中曾经的峰值（单位：字节）
	 *  获取成功。
	 */
	NK_SSize
	(*peak)(NK_This);

	/**
	 * @brief
	 *  获取内存分配器内部缓冲使用量（单位：字节）。
	 *
	 * @retval 内存分配器内部缓冲使用量（单位：字节）
	 *  获取成功。
	 */
	NK_SSize
	(*usage)(NK_This);

#undef NK_This
} NK_Allocator;

/**
 * @brief
 *  创建模块句柄。\n
 *  传入一块固定大小的内存区域，由内存分配器接管。\n
 *  内存分配器接管后，用户可以根据须要在内存区域内动态分配内存块。\n
 *  由于算法调度开销，传入内存块大小不能小于 1K 字节，否则会创建失败，\n
 *  建议使用时内存区域大于 4K 字节。
 *
 * @param[in] mem
 *  用于分配的内存区域起始位置。
 *
 * @param[in] len
 *  用于分配的内存区域大小，必须大于 1K。
 *
 * @retval 模块对象句柄
 *  创建成功。
 *
 * @retval Nil
 *  创建失败，可能是从参数错误。
 */
NK_API NK_Allocator *
NK_Alloc_Create(NK_PByte mem, NK_Size len);

/**
 * @brief
 *  销毁模块对象句柄。\n
 *
 * @param[in] Allocator_r
 *  模块对象句柄引用。
 *
 * @param[out] mem_r
 *  释放成功时，从此变量获取调用 @ref NK_Alloc_Create() 创建时传入的 @ref mem 变量值。
 *
 * @retval 内存分配器占用内存空间大小
 *  释放成功。
 *
 * @retval -1
 *  释放失败。
 */
NK_API NK_SSize
NK_Alloc_Free(NK_Allocator **Allocator_r, NK_PByte *mem_r);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_ALLOCATOR_H_ */

