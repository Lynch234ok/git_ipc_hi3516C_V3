
/**
 * PlainINI 文本解析器。
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#if !defined(NK_UTILS_PLAIN_PlainINI_H_)
# define NK_UTILS_PLAIN_PlainINI_H_
NK_CPP_EXTERN_BEGIN

/**
 * PlainINI 模块句柄。
 */
typedef struct Nk_PlainINI {
#define NK_This struct Nk_PlainINI *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;


	/**
	 * 写入一个键值。
	 *
	 * @param[in]			section				键值所在的段名称。
	 * @param[in]			key					键值所在的关键字名称，如果传入为 Nil，则尝试创建一个段。
	 * @param[in]			value				键值的数值。
	 *
	 * @return		写入成功返回 0，失败返回 -1。
	 */
	NK_Int
	(*write)(NK_This, NK_PChar section, NK_PChar key, NK_PChar value);


	/**
	 * 写入一个整型键值。\n
	 * 参考 @ref NK_PlainINI::write()。
	 *
	 */
	NK_Int
	(*write2)(NK_This, NK_PChar section, NK_PChar key, NK_Int value);

	/**
	 * 写入一个浮点型键值。\n
	 * 参考 @ref NK_PlainINI::write()。
	 *
	 */
	NK_Int
	(*write3)(NK_This, NK_PChar section, NK_PChar key, NK_DFloat value);


	/**
	 * 读取一个键值。\n
	 *
	 * @param[in]			section				键值所在的段名称。
	 * @param[in]			key					键值所在的关键字名称。
	 * @param[in]			def					当读取失败时，默认数值会作为结果返回。
	 *
	 * @return		读取成功返回键值对应的数值，读取失败返回传入的 def。
	 */
	NK_PChar
	(*read)(NK_This, NK_PChar section, NK_PChar key, NK_PChar def);

	/**
	 * 读取一个整型键值。\n
	 * 参考 @ref NK_PlainINI::read()。
	 *
	 */
	NK_Int
	(*read2)(NK_This, NK_PChar section, NK_PChar key, NK_Int def);


	/**
	 * 读取一个浮点型键值。\n
	 * 参考 @ref NK_PlainINI::read()。
	 *
	 */
	NK_DFloat
	(*read3)(NK_This, NK_PChar section, NK_PChar key, NK_DFloat def);


	/**
	 * 在所在键值前加入注释。
	 *
	 * @param[in]			section				键值所在的段名称。
	 * @param[in]			key					键值所在的关键字名称，如果传入为 Nil 则在段之前加入注释。
	 * @param[in]			cmt					注释内容。
	 * @return
	 */
	NK_Int
	(*comment)(NK_This, NK_PChar section, NK_PChar key, NK_PChar cmt);

	/**
	 * 生成文本。
	 */
	NK_Int
	(*toText)(NK_This, NK_PChar text, NK_Size *text_len);


#undef NK_This
} NK_PlainINI;


/**
 * 创建 ini 模块句柄。
 */
NK_API NK_PlainINI *
NK_PlainINI_Create(NK_Allocator *Alloctr);


/**
 * 解析 ini 文本并创建 ini 模块句柄。\n
 * 内部调用 @ref NK_PlainINI_Create() 方法。
 *
 */
NK_API NK_PlainINI *
NK_PlainINI_Parse(NK_Allocator *Alloctr, NK_PChar text, NK_Size len);

/**
 * 销毁 PlainINI 模块句柄。
 */
NK_API NK_Void
NK_PlainINI_Free(NK_PlainINI **PlainINI_r);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_PLAIN_PlainINI_H_ */
