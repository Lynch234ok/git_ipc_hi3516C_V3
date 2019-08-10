/**
 * 线程工具集。
 */

#include <NkEmbedded/mem_allocator.h>

#if !defined(NK_EMB_THREAD_H_)
# define NK_EMB_THREAD_H_
NK_CPP_EXTERN_BEGIN


/**
 * @macro
 *  线程传入最大参数个数。
 */
#define NK_THREAD_MAX_ARGC (32)


/**
 * @macro
 *  线程别名最大长度。
 */
#define NK_THREAD_NICKNAME_MAX_SZ (32)

/**
 * @macro
 *  默认允许线程连接数。
 */
#define NK_THREAD_DEF_BACKLOG (64)

/**
 * @macro
 *  最大允许线程连接数。
 */
#define NK_THREAD_MAX_BACKLOG (128)

/**
 * @macro
 *  线程栈区最小值（单位：字节）。
 */
#define NK_THREAD_STACK_MIN (1024 * 16)


/**
 * @macro
 * 线程栈区最大值（单位：字节）。
 */
#define NK_THREAD_STACK_MAX (1024 * 1024 * 8)


/**
 * @macro
 * 线程栈区默认值（单位：字节）。
 */
#define NK_THREAD_STACK_DEF (1024 * 1024 * 1)


/**
 * 线程状态定义。
 */
typedef enum Nk_ThreadStatus {

	/// 线程启动中标识
	NK_THREAD_ST_PREPARE = (0),//!< NK_THREAD_ST_PREPARE
	/// 线程已释放标识
	NK_THREAD_ST_TERMINATED,   //!< NK_THREAD_ST_TERMINATED
	/// 线程挂起
	NK_THREAD_ST_IDLE,         //!< NK_THREAD_ST_IDLE
	/// 线程正在运行
	NK_THREAD_ST_RUNNING,      //!< NK_THREAD_ST_RUNNING

} NK_ThreadStatus;

/**
 * @brief
 *  单个线程信息。
 */
typedef struct Nk_ThreadInfo {

	/// 线程 ID
	NK_Int id;
	/// 线程名称
	NK_Char nickname[NK_THREAD_NICKNAME_MAX_SZ];
	/// 线程状态
	NK_ThreadStatus status;
	/// 线程系统 ID
	NK_UInt32 threadid;
	/// 线程栈区空间大小
	NK_Size stack_kb;
	/// 线程运行时长
	NK_Size uptime_s;

} NK_ThreadInfo;


/**
 * @brief
 *  线程库全局初始化，\n
 *  在调用 @ref NK_Thread_Create() 前必须确认已调用此次接口初始化线程相关全局属性。\n
 *  通常在应用启动开始尽可能考前的位置调用此方法，以确保其他线程方法调用成功。
 *
 * @param[in] Alloctr
 *  保留参数，没有任何作用。
 *
 * @param[in] backlog
 *  最大线程并发数，传入 0 表示使用默认连接数 @ref NK_THREAD_DEF_BACKLOG。
 *
 * @retval 0
 *  初始化成功。
 *
 * @retval -1
 *  初始化失败。
 */
NK_API NK_Int
NK_Thread_GlobalStartup(NK_Allocator *Alloctr, NK_Size backlog);


/**
 * @brief
 *  线程库全局去初始化。\n
 *  通常在应用销毁最后位置调用此接口。\n
 *  接口内部会等待所有线程回收完毕后才返回，\n
 *  因此在线程设计上必须确保不能出现死循环的问题。
 *
 */
NK_API NK_Void
NK_Thread_GlobalCleanup();


/**
 * @brief
 *  Thread 模块句柄。
 */
typedef struct Nk_Thread {
#define NK_This struct Nk_Thread *const

	/**
	 * @brief
	 *  模块基础接口。
	 */
	NK_Object Object;

	/**
	 * @brief
	 *  挂起线程。\n
	 *  根据线程内部调度需要挂起一段时间，\n
	 *  调用此接口后，线程进入 IDLE 状态，\n
	 *  以释放 CPU 资源维持 CPU 占用率动态平衡，\n
	 *  此方法只允许在线程内部运行。
	 *
	 * @param[in] s
	 *  挂起的秒数。
	 *
	 * @param[in] ms
	 *  挂起的毫秒数。
	 *
	 * @param[in] us
	 *  挂起的微秒数。
	 *
	 */
	NK_Void
	(*suspend)(NK_This, NK_Int s, NK_Int ms, NK_Int us);

	/**
	 * @brief
	 *  获取或设置线程别称。
	 * @details
	 *  主要用于调试时线程名称区分。\n
	 *  在线程内部调用此方法时，会设置线程别名取别名，\n
	 *  在线程外部调用获取线程别名，传入 @ref name 参数无效。
	 *
	 * @param[in] name
	 *  线程设置的别名，如果传入 Nil 则单纯读取名称。
	 *
	 * @return
	 *  线程别名。
	 *
	 */
	NK_PChar
	(*nickname)(NK_This, const NK_PChar name);


	/**
	 * @brief
	 *  终止线程，并返回当前线程终止标识。\n
	 *  此接口在线程回调内部和外部意义不同，\n
	 *  在线程外部调用此接口，会向线程发送一个终止请求，此时线程回调内部同样调用此接口可以判断线程是否被终止，\n
	 *  在线程内部调用主要用于一些循环逻辑判断，通过判断这个接口返回值决定是否须要退出循环体。\n
	 *
	 * @return
	 *  在线程实现回调内调用返回线程终止标识。
	 *
	 */
	NK_Boolean
	(*terminate)(NK_This);


	/**
	 * @brief
	 *  获取当前线程运行时长（单位：秒）。
	 *
	 * @return
	 *  当前线程运行时长（单位：秒）。
	 */
	NK_Size
	(*uptime)(NK_This);

#undef NK_This
} NK_Thread;

/**
 * @brief
 *  线程回调实现定义。
 */
typedef NK_Void (*NK_ThreadFunc)(NK_Thread *const Thread, NK_Int argc, NK_PVoid argv[]);

/**
 * @brief
 *  创建一个后台线程。
 *
 * @param[in] nickname
 *  线程别名，主要用于调试时的信息区分。
 *
 * @param[in] detached
 *  分离线程标识，当为 True 为线程分离模式，\n
 *  在分离县城模式下，调用 @ref NK_Thread::terminate() 会立即返回，\n
 *  但线程可能未完全退出。
 *
 * @param[in] Func
 *  线程流程实现回调。
 *
 * @param[in] argc
 *  传入线程参数集数量，最大值 @ref NK_THREAD_MAX_ARGC。
 *
 * @param[in] argv
 *  传入线程参数集，最大可以传入 @ref NK_THREAD_MAX_ARGC 个参数。
 *
 * @retval Thread 线程句柄
 *  创建成功。
 *
 * @retval Nil
 *  创建失败。
 */
NK_API NK_Thread *
NK_Thread_Create(NK_PChar nickname, NK_Boolean detached, NK_Size stack_size,
		NK_ThreadFunc Func, NK_Int argc, NK_PVoid argv[]);

/**
 * @macro
 *  匿名创建快速操作定义，线程别名默认为传入函数名称，\n
 *  创建线程成功后，可以通过 @ref NK_Thread::nickname() 方法更改线程别名。
 */
#define NK_Thread_AnonymousCreate(__detached, __stack_size, __Func, __argc, __argv) \
		NK_Thread_Create(#__Func "()", (__detached), (__stack_size), (__Func), (__argc), (__argv))

/**
 * @macro
 *  创建非分离式线程快速操作定义。
 */
#define NK_THREAD_JOINABLE_CREATE(__Func, __argc, __argv) \
		NK_Thread_AnonymousCreate(NK_False, NK_THREAD_STACK_DEF, __Func, (__argc), (__argv))

/**
 * @macro
 *  创建无参数非分离式线程快速操作定义。
 */
#define NK_THREAD_JOINABLE_CREATE_NARG(__Func) \
		NK_Thread_AnonymousCreate(NK_False, NK_THREAD_STACK_DEF, __Func, 0, NK_Nil)

/**
 * @macro
 *  创建分离式线程快速操作定义。
 */
#define NK_THREAD_DETACHED_CREATE(__Func, __argc, __argv) \
		NK_Thread_AnonymousCreate(NK_True, NK_THREAD_STACK_DEF, __Func, (__argc), (__argv))

/**
 * @macro
 *  创建无参数分离式线程快速操作定义。
 */
#define NK_THREAD_DETACHED_CREATE_NARG(__Func) \
		NK_Thread_AnonymousCreate(NK_True, NK_THREAD_STACK_DEF, __Func, 0, NK_Nil)

/**
 * @brief
 *  终止线程，内部会调用 @ref NK_Thread::terminate() 方法。
 *
 * @param[in] Thread_r
 *  线程句柄引用，释放成功后原有句柄会至 Nil。
 *
 * @retval 0
 *  释放成功。
 *
 * @retval -1
 *  释放失败。
 */
NK_API NK_Int
NK_Thread_Free(NK_Thread **Thread_r);

/**
 * @brief
 *  线程信息输出。
 *
 * @param[out] Infos
 *  栈区线程信息缓存，获取成功后从这里输出。
 *
 * @param[in] max
 *  Infos 在栈区的数组长度。
 *
 * @retval 返回 @ref NK_ThreadInfo 数据结构的个数
 *  信息输出成功。
 *
 * @retval 0
 *  没有信息输出。
 */
NK_API NK_Size
NK_Thread_Dump(NK_ThreadInfo *Infos, NK_Size max);

/**
 * @macro
 *  线程信息快速打印。
 */
#define NK_THREAD_DUMP() \
	do {\
		NK_Int __i = 0;\
		NK_Char __text[32];\
		NK_PChar __status = "Running";\
		NK_ThreadInfo Infos[128];\
		NK_Size threadn = 0;\
		NK_Size stack_kb = 0;\
		\
		printf("+"); for (__i = 0; __i < 84; ++__i) printf("-"); printf("+\r\n");\
		printf("| %4s | %32s | %8s | %10s | %16s |\r\n", "ID", "Name", "Status", "Stack(kB)", "Uptime");\
		printf("+"); for (__i = 0; __i < 84; ++__i) printf("-"); printf("+\r\n");\
		\
		threadn = NK_Thread_Dump(Infos, sizeof(Infos) / sizeof(Infos[0]));\
		for (__i = 0; __i < threadn; ++__i) {\
			if (NK_THREAD_ST_IDLE == Infos[__i].status) __status = "Idle";\
			snprintf(__text, sizeof(__text), "%02d:%02d:%02d",\
					Infos[__i].uptime_s / 3600, (Infos[__i].uptime_s / 60) % 60, Infos[__i].uptime_s % 60);\
			printf("| %4d | %32s | %8s | %8dkB | %16s |\r\n",\
					__i, Infos[__i].nickname, __status, Infos[__i].stack_kb, __text);\
			stack_kb += Infos[__i].stack_kb;\
		}\
		\
		printf("+"); for (__i = 0; __i < 84; ++__i) printf("-"); printf("+\r\n");\
		printf("| %4d | %32s | %8s | %8dkB | %16s |\r\n", threadn, "", "", stack_kb, "");\
		printf("+"); for (__i = 0; __i < 84; ++__i) printf("-"); printf("+\r\n");\
	} while (0)



NK_CPP_EXTERN_END
#endif /* NK_EMB_THREAD_H_ */
