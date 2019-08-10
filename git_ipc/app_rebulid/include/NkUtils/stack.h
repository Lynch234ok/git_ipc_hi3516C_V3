/**
 * NK_Stack 定义。
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#if !defined(NK_UTILS_STACK_H_)
# define NK_UTILS_STACK_H_
NK_CPP_EXTERN_BEGIN

/**
 * Stack 模块句柄。
 */
typedef struct Nk_Stack {
#define NK_This struct Nk_Stack *const

	/**
	 * 模块基础接口。
	 * 实现模块基本接口调用。
	 */
	NK_Object Object;

	/**
	 * 获取列表长度。
	 */
	NK_Size
	(*length)(NK_This);

	/**
	 * @brief
	 *  向栈顶部增加一个数据。
	 * @details
	 *
	 *
	 * @param data [in]
	 *  数据所在内存起始地址。
	 * @param datalen [in]
	 *  数据所在内存长度。
	 *
	 * @retval
	 *  增加成功返回 0，否则返回 -1。
	 *
	 */
	NK_Int
	(*push)(NK_This, NK_PVoid data, NK_Size datalen);

	/**
	 * @brief
	 *  从栈顶部获取一个数据。
	 * @details
	 * 从容器中移除一组数据。\n
	 *
	 * @retval
	 *  获取成功返回数据长度u，失败返回 -1。
	 */
	NK_SSize
	(*pop)(NK_This, NK_PVoid data, NK_Size datamax);

	/**
	 * @brief
	 *  从栈底部获取一个数据。
	 *
	 * @retval
	 *  获取成功返回数据长度u，失败返回 -1。
	 */
	NK_SSize
	(*shift)(NK_This, NK_PVoid data, NK_Size datamax);



	/**
	 * @brief
	 *  查看栈中某一个数据。
	 *
	 * @retval
	 *  获取成功返回数据长度u，失败返回 -1。
	 */
	NK_SSize
	(*peek)(NK_This, NK_Int fromLow, NK_PVoid data, NK_Size datamax);


#undef NK_This
} NK_Stack;


/**
 * 创建 Stack 模块句柄。
 */
NK_API NK_Stack *
NK_Stack_Create(NK_Allocator *Alloctr);

/**
 * 销毁 Stack 模块句柄。
 */
NK_API NK_Void
NK_Stack_Free(NK_Stack **Stack_r);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_STACK_H_ */
