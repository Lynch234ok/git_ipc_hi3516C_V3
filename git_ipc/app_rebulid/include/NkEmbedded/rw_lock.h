/**
 * 读写锁。
 */



#include <NkUtils/object.h>
#include <NkEmbedded/mem_allocator.h>

#ifndef NK_RW_LOCK_H_
#define NK_RW_LOCK_H_
NK_CPP_EXTERN_BEGIN

/**
 * RWLock 模块句柄。
 */
typedef struct NK_RWLock {
#define NK_This struct NK_RWLock *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

	/**
	 * 阻塞读取锁。\n
	 * 对于读取锁，可以对同一个读写锁上多次读取锁。\n
	 * 一旦上读取锁成功，在对应读取锁解锁成功后才能上写锁。\n
	 * 但是上读取锁后，可以再上多次读取锁。
	 *
	 * @return	该接口在正常情况下会一直等待上锁成功才会返回 0。
	 *
	 */
	int
	(*rdLock)(NK_This);

	/**
	 * 非阻塞读取琐。\n
	 * 参考 @ref NK_RWLock::rdLock。
	 *
	 * @return	上锁成功返回 0，失败返回 -1。
	 *
	 */
	int
	(*tryRdLock)(NK_This);

	/**
	 * 阻塞写入锁。
	 * 对于写入锁，可以对同一个读写锁只能上一次写入锁。\n
	 * 一旦上写入锁成功，在解锁成功后才能上读锁。
	 *
	 *
	 * @return	该接口在正常情况下会一直等待上锁成功才会返回 0。
	 *
	 */
	int
	(*wrLock)(NK_This);

	/**
	 * 非阻塞写入锁。\n
	 * 参考 @ref NK_RWLock::wrLock。
	 *
	 * @return	上锁成功返回 0，失败返回 -1。
	 *
	 */
	int
	(*tryWrLock)(NK_This);

	/**
	 * 解除锁。\n
	 * 若当前上了写入锁则解除写入锁，\n
	 * 若当前上了读取锁则解除一次读取锁。\n
	 * 解除读取锁必须调用解除锁接口次数与上读取锁次数匹配。
	 *
	 * @return	解锁成功返回 0，失败返回 -1。
	 *
	 */
	int
	(*unlock)(NK_This);


#undef NK_This
} NK_RWLock;


/**
 * 创建 RWLock 模块句柄。
 */
NK_API NK_RWLock *
NK_RWLock_Create(NK_Allocator *Alloctr);

/**
 * 销毁 RWLock 模块句柄。
 */
NK_API NK_Void
NK_RWLock_Free(NK_RWLock **RWLock_r);


NK_CPP_EXTERN_END
#endif /* JAE_RW_LOCK_H_ */

