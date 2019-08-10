/**
 * 互斥锁。
 */

#include <NkUtils/object.h>
#include <NkEmbedded/mem_allocator.h>

#ifndef NK_MUTEX_H_
#define NK_MUTEX_H_
NK_CPP_EXTERN_BEGIN

/**
 * Mutex 模块句柄。
 */
typedef struct Nk_Mutex
{
#define NK_This struct Nk_Mutex *const

	/**
	 * 模块基础类。
	 */
	NK_Object Object;

	/**
	 * 非阻塞式锁定，上锁成功返回0，否则返回-1。
	 */
	NK_Int
	(*trylock)(NK_This);

	/**
	 * 阻塞式锁定，直至上锁成功后返回0。
	 */
	NK_Int
	(*lock)(NK_This);

	/**
	 * 解除锁定，返回0。
	 */
	NK_Int
	(*unlock)(NK_This);

#undef NK_This
} NK_Mutex;

/**
 * 创建 Mutex 模块句柄。
 */
NK_API NK_Mutex *
NK_Mutex_Create(NK_Allocator *Alloctr);

/**
 * 销毁 Mutex 模块句柄。
 */
NK_API NK_Void
NK_Mutex_Free(NK_Mutex **Mutex_r);


NK_CPP_EXTERN_END
#endif /* NK_MUTEX_H_ */

