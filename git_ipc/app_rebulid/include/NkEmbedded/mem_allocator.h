/**
 * 基于 NK_Allocator 上继承的内存分配器。\n
 * 兼容操作系统内存分配器，同时支持线程安全。
 */

#include <NkUtils/allocator.h>

#ifndef NK_MEM_ALLOCATOR_H_
#define NK_MEM_ALLOCATOR_H_
NK_CPP_EXTERN_BEGIN

/**
 * @brief
 *  MemAllocator 模块句柄。
 */
typedef NK_Allocator NK_MemAllocator;

/**
 * @brief
 *  获取操作系统内存分配器。\n
 *  通过模块句柄调用操作系统内存分配器，\n
 *  基于 POSIX 标准的 malloc() 和 free() 等接口封装。
 *
 * @return 内存分配器模块句柄。
 */
NK_API NK_MemAllocator *
NK_MemAlloc_OS();

/**
 * @brief
 *  创建模块句柄。
 *
 * @param[in] mem
 *  用作内存分配的内存块起始地址。
 *
 * @param[in] len
 *  用作内存分配的内存快长度。
 *
 * @return 内存分配器模块句柄。
 */
NK_API NK_MemAllocator *
NK_MemAlloc_Create(NK_PByte mem, NK_Size len);

/**
 * @brief
 *  销毁模块句柄。
 */
NK_API NK_SSize
NK_MemAlloc_Free(NK_MemAllocator **Alloctr_r, NK_PByte *mem);

NK_CPP_EXTERN_END
#endif /* NK_MEM_ALLOCATOR_H_ */

