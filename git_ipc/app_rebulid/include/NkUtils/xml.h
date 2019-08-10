
#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#if !defined(NK_UTILS_XML_H_)
# define NK_UTILS_XML_H_
NK_CPP_EXTERN_BEGIN

/**
 * @brief
 *  XML 模块句柄。
 */
typedef struct Nk_XML {
#define NK_This struct Nk_XML *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

#undef NK_This
} NK_XML;


/**
 * @brief
 *  创建 XML 模块句柄。
 *
 * @param[in] Alloctr
 *  模块内存分配器。
 *
 * @param[in] name
 *  创建元素名称。
 *
 * @retval XML 模块句柄。
 *  创建成功。
 *
 * @retval Nil
 *  创建 XML 模块失败。
 *
 */
NK_API NK_XML *
NK_XML_Create(NK_Allocator *Alloctr, const NK_PChar name);

/**
 * @brief
 *  解析 XML 文本成数据结构。
 *
 * @param[in] Alloctr
 *  模块内存分配器。
 *
 * @param[in] text
 *  解析文本。
 *
 * @param[in] len
 *  解析文本长度。
 *
 * @retval XML 模块句柄。
 *  解析成功。
 *
 * @retval Nil
 *  解析失败。
 */
NK_API NK_XML *
NK_XML_Parse(NK_Allocator *Alloctr, const NK_PChar text, NK_Size len);

/**
 * @brief
 *  获取模块内存分配器。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @retval 内存分配器句柄。
 *  获取成功。
 *
 */
NK_API NK_Allocator *
NK_XML_Allocator(NK_XML *XML);

/**
 * 销毁 XML 模块句柄。
 */
NK_API NK_Int
NK_XML_Free(NK_XML **XML_r);

/**
 * @brief
 *  设置 XML 所在元素 @ref name 属性。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @param[in] name
 *  属性名称。
 *
 * @param[in] value
 *  属性内容。
 *
 * @retval XML 元素句柄。
 */
NK_API NK_XML *
NK_XML_SetAttr(NK_XML *XML, const NK_PChar name, const NK_PChar value);

/**
 * @brief
 *  获取所在元素 @ref name 属性。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @param[in] name
 *  元素名称。
 *
 * @param[in] def
 *  缺省值，当未找到 @ref name 属性时会缺省返回此值。
 *
 * @retval 属性值
 *  获取成功。
 *
 * @retval @ref def
 *  获取失败，当未找到 @ref name 时返回缺省值。
 *
 * @retval Nil
 *  获取失败，可能参数错误。
 */
NK_API const NK_PChar
NK_XML_GetAttr(NK_XML *XML, const NK_PChar name, const NK_PChar def);

/**
 * @brief
 *  设置元素文本。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @param[in] text
 *  元素文本，传入 Nil 表示清空当前文本。
 *
 * @retval 元素句柄
 *  设置成功。
 *
 * @retval Nil
 *  设置失败，可能是传入参数错误。
 */
NK_API NK_XML *
NK_XML_SetText(NK_XML *XML, const NK_PChar text);

/**
 * @macro
 *  清空元素文本快速操作。
 */
#define NK_XML_ClearText(__XML, __text) NK_XML_SetText((__XML), (__text))

/**
 * @brief
 *  获取元素文本。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @retval 元素文本
 *  获取成功。
 */
NK_API const NK_PChar
NK_XML_GetText(NK_XML *XML);

/**
 * @brief
 *  追加一个元素。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @param[in] Element
 *  元素句柄。
 *
 * @retval 新追加的元素句柄。
 *  追加成功。
 *
 * @retval Nil
 *  追加失败，如果元素 @ref Element 是动态创建的注意释放。
 */
NK_API NK_XML *
NK_XML_AddElement(NK_XML *XML, NK_XML *Element);

/**
 * @brief
 *  追加一个元素并制定其 ID。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @param[in] Element
 *  元素句柄。
 *
 * @param[in] id
 *  元素 ID。
 *
 * @retval 新追加的元素句柄。
 *  追加成功。
 *
 * @retval Nil
 *  追加失败，如果元素 @ref Element 是动态创建的注意释放。
 */
NK_API NK_XML *
NK_XML_PutElement(NK_XML *XML, NK_XML *Element, NK_Int id);

/**
 * @brief
 *  以名称方式追加元素。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @param[in] name
 *  元素名称。
 *
 * @retval 新追加的元素句柄。
 *  追加成功。
 *
 * @retval Nil
 *  追加失败。
 */
NK_API NK_XML *
NK_XML_AddElementByName(NK_XML *XML, const NK_PChar name);

/**
 * @brief
 *  以名称方式追加元素。
 *
 * @param[in] XML
 *  模块句柄。
 *
 * @param[in] name
 *  元素名称。
 *
 * @param[in] id
 *  元素 ID。
 *
 * @retval 新追加的元素句柄。
 *  追加成功。
 *
 * @retval Nil
 *  追加失败。
 */
NK_API NK_XML *
NK_XML_PutElementByName(NK_XML *XML, const NK_PChar name, NK_Int id);

/**
 * @brief
 *  获取 XML 元素下 @ref name 的子元素数量。\n
 *  此接口不会返回失败，当传入参数有误时会返回 0 表示没有元素。
 *
 * @param[in] XML
 *  XML 数据结构。
 *
 * @param[in] name
 *  元素名称。
 *
 * @retval @ref name 对应元素的数量。
 *  获取成功。
 *
 */
NK_API NK_Size
NK_XML_HasElement(NK_XML *XML, const NK_PChar name);

/**
 * @brief
 *  对于同名元素集合，获取下一个同名元素。
 *
 * @param[in] XML
 *  XML 数据结构。
 *
 * @retval 下一个同名元素。
 *  获取成功。
 *
 * @retval Nil
 *  没有下一个同名元素。
 */
NK_API NK_XML *
NK_XML_NextElement(NK_XML *XML);

/**
 * @brief
 *  获取子元素。
 *
 * @param[in] XML
 *
 *
 * @param name
 * @return
 */
NK_API NK_XML *
NK_XML_GetElement(NK_XML *XML, const NK_PChar name);

/**
 * 伏击
 * @param XML
 * @param name
 * @param id
 * @return
 */
NK_API NK_XML *
NK_XML_GetElementByID(NK_XML *XML, const NK_PChar name, NK_Int id);

/**
 * @brief
 *  生成 XML 文本。\n
 *  生成文本以后，内存会保存在模块句柄内部，\n
 *  在用户调用 NK_XML_Free() 或 NK_XML::Object::free() 方法时统一释放。
 *
 * @param[in] XML
 *  XML 数据结构。
 *
 * @retval XML 文本起始地址
 *  生成成功。
 *
 * @retval Nil
 *  生成失败，可能是参数错误。
 */
NK_API const NK_PChar
NK_XML_ToText(NK_XML *XML);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_XML_H_ */

