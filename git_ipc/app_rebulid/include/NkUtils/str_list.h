
#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#if !defined(NK_STR_LIST_H_)
# define NK_STR_LIST_H_
NK_CPP_EXTERN_BEGIN

/**
 * StrList 模块句柄。
 */
typedef struct Nk_StrList {
#define NK_This struct Nk_StrList *const

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
	 * 向容器中插入以 @ref key 词条为索引 data 指向的内存区域，长度为 @ref data_len 的一组数据。\n
	 * 容器会根据传入长度，在容器内部复制一分副本数据。\n
	 *
	 * @param[in]		overwrite		覆盖标识，设置为 False 时，如果容器中已经存在该关键字，并且不覆盖并追加一条关键字一样的数据。
	 * @param[in]		key				词条字符串，长度必须大于 0，大小写敏感。
	 * @param[in]		data			词条对应的数据。
	 * @param[in]		data_len		数据长度，模块会根据长度在内部保存相等长度的内存块。
	 *
	 * @retval 0		插入数据成功。
	 * @retval -1		插入数据失败。
	 *
	 */
	NK_Int
	(*insert)(NK_This, NK_Boolean overwrite, NK_PChar key, NK_PVoid data, NK_Size data_len);

	/**
	 * 从容器中移除一组数据。\n
	 *
	 * @return 	移除成功返回 0，不存在该关键字返回 -1。
	 */
	NK_Int
	(*remove)(NK_This, NK_PChar key);

	/**
	 * 通过插入的先后次序所对应序号查找数据。
	 *
	 * @param[in]		id				数据序号，从 0 开始，小于 @ref length() 所获取的长度。
	 * @param[out]		key_r			键值引用指针。
	 * @param[out]		data_r			数据引用指针，若调用者没有传入栈区缓冲，查找数据成功后指针将指向容器内缓冲，反之则指向栈区缓冲。
	 * @param[out]		data			栈区数据缓冲，若调用者传入栈区缓冲，数据调用时更具备可靠性，数据查找成功后将会拷贝一份到栈区。
	 * @param[in]		data_len		栈区缓冲大小。
	 *
	 * @return	查找成功返回 0，失败返回 -1。
	 */
	NK_Int
	(*indexOf)(NK_This, NK_Int id, NK_PChar *key_r, NK_PVoid *data_r, NK_PByte data, NK_Size *data_len);

	/**
	 * 通过关键字查找数据，参考 @ref indexOf()。
	 *
	 * @param[in]		key				数据关键字。
	 * @param[out]		data_r			数据引用指针，若调用者没有传入栈区缓冲，查找数据成功后指针将指向容器内缓冲，反之则指向栈区缓冲。
	 * @param[out]		data			栈区数据缓冲，若调用者传入栈区缓冲，数据调用时更具备可靠性，数据查找成功后将会拷贝一份到栈区。
	 * @param[in]		data_len		栈区缓冲大小。
	 *
	 * @return	查找成功返回 0，失败返回 -1。
	 *`
	 */
	NK_Int
	(*indexOfKey)(NK_This, NK_PChar key, NK_PVoid *data_r, NK_PByte data, NK_Size *data_len);

	/**
	 * 获取某一关键字在模块中保存的数量。
	 *
	 * @param[in]		key				关键字。
	 *
	 * @return	参数错误返回 -1，获取 @ref key 关键字在模块内保存的个数，如果不存在该关键字则返回 0。
	 */
	NK_SSize
	(*hasKey)(NK_This, NK_PChar key);


	/**
	 * 抽取数据。
	 * 功能与 @ref find 相似。\n
	 * 功能上差异时，当该接口调用后，容器将会释放数据，因此调用该接口时必须传入有效栈区缓冲以接管数据。\n
	 * 功能相当于 @ref find + @ref remove 组合使用。
	 *
	 */
	NK_Int
	(*detach)(NK_This, NK_PChar key, NK_PVoid *bytes_r, NK_PByte stack, NK_Size *stack_len);


#undef NK_This
} NK_StrList;


/**
 * 创建 StrList 模块句柄。
 */
NK_API NK_StrList *
NK_StrList_Create(NK_Allocator *Alloctr);

/**
 * 销毁 StrList 模块句柄。
 */
NK_API NK_Void
NK_StrList_Free(NK_StrList **StrList_r);


NK_CPP_EXTERN_END
#endif /* NK_STR_LIST_H_ */
