
/**
 * @brief
 *  SPook 2.0 模块。\n
 *  此模块实现通过 TCP 插件服务方式监听 TCP 传输请求，\n
 *  通过此模块可以快速实现 TCP 被动式请求的服务器，如 HTTP、RTSP 等。
 */


//#include <NkUtils/allocator.h>
#include <NkEmbedded/socket.h>
#include <NkEmbedded/thread.h>

#if !defined(NK_EMB_SPOOK2_H_)
#define NK_EMB_SPOOK2_H_
NK_CPP_EXTERN_BEGIN


struct NK_SPook2;
typedef struct NK_SPook2 NK_SPook2;


typedef struct NK_SPook2ListenerEvent {

	/**
	 * 加入监听器事件，调用 @ref NK_SPook2::addListener() 方法成功后会触发此事件。
	 *
	 * @param[in]		SPook		当前事件调用的 SPook 模块句柄。
	 * @param[in]		ctx			SPook 模块句柄传入上下文，由 @ref NK_SPook2_Create() 方法指定。
	 *
	 */
	NK_Void
	(*onAdd)(NK_SPook2 *SPook, NK_PVoid ctx);

	/**
	 * 删除监听器事件，调用 @ref NK_SPook2::dropListener() 方法成功后会触发此事件。
	 *
	 * @param[in]		SPook		当前事件调用的 SPook 模块句柄。
	 * @param[in]		ctx			SPook 模块句柄传入上下文，由 @ref NK_SPook2_Create() 方法指定。
	 *
	 */
	NK_Void
	(*onDrop)(NK_SPook2 *SPook, NK_PVoid ctx);

	/**
	 * 探测报文事件，当模块接收到一个报文，并不清除属于那个监听器的时，
	 * 会触发此事件判断。
	 *
	 * @param[in]		SPook		当前事件调用的 SPook 模块句柄。
	 * @param[in]		ctx			SPook 模块句柄传入上下文，由 @ref NK_SPook2_Create() 方法指定。
	 * @param[in]		msg			探测报文数据。
	 * @param[in]		len		探测报文长度。
	 *
	 * @return
	 * 当报文确定不属于此监听器时返回 -1，模块将不会在当前会话继续触发探测，
	 * 当模块未能确定报文内容是否属于当前监听器时，返回 0，模块在下一个探测周期会继续探测，
	 * 当模块确定当前报文数据当前监听器，返回 @ref msg_len，模块会继续触发 @ref onEventLoop 事件。
	 *
	 */
	NK_SSize
	(*onProbe)(NK_SPook2 *SPook, NK_PVoid ctx, NK_PByte msg, NK_Size len);

	/**
	 * 事件循环事件，当会话连接到模块，并确认会话属于当前监听器时，
	 * 会触发此事件。
	 *
	 * @param[in]		SPook		当前事件调用的 SPook 模块句柄。
	 * @param[in]		ctx			SPook 模块句柄传入上下文，由 @ref NK_SPook2_Create() 方法指定。
	 * @param[in]		Thread		当前线程控制器，可以调用 @ref NK_Thread::suspend() 和 NK_Thread::terminate() 进行线程相应调度。
	 * @param[in]		ConnTCP		会话 TCP 连接句柄。
	 *
	 *
	 */
	NK_Void
	(*onEventLoop)(NK_SPook2 *SPook, NK_PVoid ctx, NK_Thread *Thread, NK_SockTCP *ConnTCP);


	/**
	 * @brief
	 *  事件上下文。
	 */
	NK_PVoid ctx;

} NK_SPook2ListenerEvent;


struct NK_SPook2 {
#define NK_This struct NK_SPook2 *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

	/**
	 * 返回监听端口。
	 *
	 */
	NK_UInt16
	(*port)(NK_This);

	/**
	 * @brief
	 *  重置监听端口。
	 * @details
	 *  重置端口后，只能重原来新的端口上才能监听链接。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param port [in]
	 *  监听端口号。
	 *
	 * @return
	 *  重置成功返回 0，否则返回 -1。
	 *
	 */
	NK_Int
	(*reset)(NK_This, NK_UInt16 port);

	/**
	 * 加入一个监听器。
	 */
	NK_Int
	(*addListener)(NK_This, NK_PChar name, NK_SPook2ListenerEvent *Event);

	/**
	 * 移除一个监听器。
	 */
	NK_Int
	(*dropListener)(NK_This, NK_PChar name);

	NK_Int
	(*listSet)(NK_This, NK_Boolean black_white);

	NK_Int
	(*listAdd)(NK_This, NK_PChar ipv4);

	NK_Int
	(*listDrop)(NK_This, NK_PChar ipv4);

	NK_Int
	(*listEmpty)(NK_This);

#undef NK_This
};

/**
 * 创建 SPook 句柄。
 *
 */
NK_API NK_SPook2 *
NK_SPook2_Create(NK_Allocator *Alloctr, NK_UInt16 port, NK_Size backlog);

/**
 * 释放 SPook 模块句柄。
 *
 */
NK_API NK_Int
NK_SPook2_Free(NK_SPook2 **SPook);


NK_CPP_EXTERN_END
#endif /* NK_EMB_SPOOK2_H_ */

