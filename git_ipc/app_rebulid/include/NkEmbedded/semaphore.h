/**
 * 信号量。
 */

#include <NkUtils/object.h>
#include <NkEmbedded/mem_allocator.h>

#ifndef NK_SEMAPHORE_H_
#define NK_SEMAPHORE_H_
NK_CPP_EXTERN_BEGIN

/**
 * Sem 模块句柄。
 */
typedef struct Nk_Sem {
#define NK_This struct Nk_Sem *const

	/**
	 * 模块基础类。
	 *
	 */
	NK_Object Object;

	/**
	 * @brief
	 *  非阻塞等待信号量等待信号量。\n
	 *
	 * @details
	 *  非阻塞的方式去获取一个信号量，获取成功后，\n
	 *  模块内部信号量的值会减小 1，获取当前信号量的值可以通过 @ref NK_Sem::value() 方法。
	 *
	 * @return
	 *  信号量大于0时立即返回，否则返回 -1。
	 */
	NK_Int
	(*trywait)(NK_This);

	/**
	 * @brief
	 *  超时阻塞等待信号量等待信号量。\n
	 *
	 * @details
	 *  通过一定时间内超时等待获取信号量，获取到信号量以后，\n
	 *  模块内部信号量的值会减小 1。
	 *
	 * @param ms [in]
	 *  超时的毫秒数，若为 0 则一直等待。
	 *
	 * @return
	 *  成功返回0，否则返回-1。
	 */
	NK_Int
	(*pend)(NK_This, NK_Size ms);

	/**
	 * @brief
	 *  发送信号量。
	 *
	 * @details
	 *  发送一个信号量，发送成功以后模块内部信号两的值会增加 1，\n
	 * 获取当前信号量的值可以通过 @ref NK_Sem::value() 方法。
	 *
	 * @return
	 *  成功返回 0，否则返回 -1，可能由于模块已经被销毁。
	 */
	NK_Int
	(*post)(NK_This);

	/**
	 * @brief
	 *  获取当前信号量的值。
	 *
	 */
	NK_Size
	(*value)(NK_This);

#undef NK_This
} NK_Sem;

/**
 * 创建 Sem 模块句柄。
 */
NK_API NK_Sem *
NK_Sem_Create(NK_Allocator *Alloctr, NK_Integer value);

/**
 * 销毁 Sem 模块句柄。
 */
NK_API NK_Void
NK_Sem_Free(NK_Sem **Sem_r);


NK_CPP_EXTERN_END
#endif /* NK_SEMAPHORE_H_ */

