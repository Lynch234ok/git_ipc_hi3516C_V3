/**
 * JSON 操作模块。
 */

#include <NkUtils/allocator.h>

#ifndef NK_UTILS_JSON_H_
#define NK_UTILS_JSON_H_
NK_CPP_EXTERN_BEGIN


/**
 * @brief
 *  JSON 对象类型定义。
 */
typedef enum Nk_JSONType
{
	/**
	 * 未知类型。
	 */
	NK_JS_OBJ_TYPE_UNDEF		= (-1),

	/**
	 * 空类型。
	 */
	NK_JS_OBJ_TYPE_NULL,

	/**
	 * 布尔类型。
	 */
	NK_JS_OBJ_TYPE_BOOLEAN,

	/**
	 * 数字类型。
	 */
	NK_JS_OBJ_TYPE_NUMBER,

	/**
	 * 字符串类型。
	 */
	NK_JS_OBJ_TYPE_STRING,

	/**
	 * 数组类型。
	 */
	NK_JS_OBJ_TYPE_ARRAY,

	/**
	 * 对象类型。
	 */
	NK_JS_OBJ_TYPE_OBJECT,

} NK_JSONType;


/**
 * JSON 操作句柄。
 */
typedef struct Nk_JSON
{
	/**
	 * 基础类对象。
	 */
	NK_Object Object;

} NK_JSON;

/**
 * @brief
 *  解析 JSON 文本到数据结构。
 *
 * @param[in] Alloctr
 *  模块内存分配器。
 *
 * @param[in] text
 *  解析文本。
 *
 * @retval JSON 数据对象
 *  解析成功。
 *
 * @retval Nil
 *  解析失败，可能是参数错误。
 */
NK_API NK_JSON *
NK_JSON_Parse(NK_Allocator *Alloctr, const NK_PChar text);

/**
 * @macro
 *  旧版本兼容。
 */
#define NK_JSON_ParseText NK_JSON_Parse

/**
 * @brief
 *  复制 JSON 数据对象，\n
 *  复制以后得到一个与原来数据结构内容一样的对象句柄，\n
 *  两者可以对立使用，互不引用。
 *
 * @param[in] Alloctr
 *  模块内存分配器。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @retval 复制的 JSON 数据对象
 *  操作成功。
 *
 * @retval Nil
 *  操作失败。
 */
NK_API NK_JSON *
NK_JSON_Duplicate(NK_Allocator *Alloctr, NK_JSON *Obj);

/**
 * @brief
 *  销毁 JSON 数据对象。
 *
 * @param[in] Obj_r
 *  JSON 数据对象引用。
 *
 * @retval 0
 *  销毁成功。
 *
 * @retval -1
 *  销毁失败。
 */
NK_API NK_Int
NK_JSON_Free(NK_JSON **Obj_r);

/**
 * @brief
 *  获取 JSON 数据对象所使用的内存分配器。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @retval 内存分配器对象。
 *  获取成功。
 *
 * @retval Nil
 *  获取失败，可能参数错误。
 */
NK_API NK_Allocator *
NK_JSON_Allocator(NK_JSON *Obj);


/**
 * @brief
 *  获取 JSON 对象类型。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @retval JSON 对象类型
 * 详见 @ref NK_JSONType。
 */
NK_API NK_JSONType
NK_JSON_TypeOf(NK_JSON *Obj);

/**
 * @macro
 *  就版本兼容。
 */
#define NK_JSON_SelfType NK_JSON_TypeOf


/**
 * @brief
 *  获取 JSON 数据对象键值。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @retval 对象对象名称。
 *  获取成功。
 *
 * @retval Nil
 *  获取失败，可能参数错误。
 */
NK_API NK_PChar
NK_JSON_KeyOf(NK_JSON *Obj);

/**
 * @macro
 *  就版本兼容。
 */
#define NK_JSON_Key NK_JSON_KeyOf

/**
 * @brief
 *  获取 JSON 数据对象上数值。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] def
 *  缺省值，当数据结构类型不匹配或者读取错误时会返回此值。
 *
 * @retval 数据对象的值。
 *  获取成功。
 *
 * @retval @ref def
 *  获取失败，返回缺省 @ref def 值。
 */
NK_API NK_Boolean
NK_JSON_GetBoolean(NK_JSON *Obj, NK_Boolean def);

/**
 * @brief
 *  获取 JSON 数据对象上数值。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] def
 *  缺省值，当数据结构类型不匹配或者读取错误时会返回此值。
 *
 * @retval 数据对象的值。
 *  获取成功。
 *
 * @retval @ref def
 *  获取失败，返回缺省 @ref def 值。
 */
NK_API NK_DFloat
NK_JSON_GetNumber(NK_JSON *Obj, NK_DFloat def);

/**
 * @brief
 *  获取 JSON 数据对象上数值。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] def
 *  缺省值，当数据结构类型不匹配或者读取错误时会返回此值。
 *
 * @retval 数据对象的值。
 *  获取成功。
 *
 * @retval @ref def
 *  获取失败，返回缺省 @ref def 值。
 */
NK_API NK_PChar
NK_JSON_GetString(NK_JSON *Obj, NK_PChar def);



/**
 * 获取数组 JSON 对象的长度。
 *
 */
NK_API NK_SSize
NK_JSON_ArraySize(NK_JSON *Array);


NK_API NK_JSON *
NK_JSON_Parent(NK_JSON *Obj);

/**
 * 获取数组中的元素对象。
 */
NK_API NK_JSON *
NK_JSON_ArrayOf(NK_JSON *Array, NK_Size which);

/**
 * @macro
 *  旧版本兼容。
 */
#define NK_JSON_IndexOf NK_JSON_ArrayOf

/**
 * 获取对象中的元素对象。
 */
NK_API NK_JSON *
NK_JSON_IndexOfName(NK_JSON *Obj, NK_PChar key);

/**
* 不区分大小写获取对象中的元素对象。
*/
NK_API NK_JSON *
NK_JSON_IndexOfCaseName(NK_JSON *Obj, NK_PChar key);


/**
 * 创建空类型 JSON 对象。
 *
 * @param[in]			Alloctr			模块内存分配器。
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateNull(NK_Allocator *Alloctr);

/**
 * 创建布尔类型 JSON 对象。
 *
 * @param[in]			Alloctr			模块内存分配器。
 * @param[in]			b				布尔变量，选择 True 或 False。
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateBoolean(NK_Allocator *Alloctr, NK_Boolean b);


/**
 *
 * @param[in]			Alloctr			模块内存分配器。
 * @param number
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateNumber(NK_Allocator *Alloctr, NK_DFloat number);


/**
 *
 * @param[in]			Alloctr			模块内存分配器。
 * @param str
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateString(NK_Allocator *Alloctr, NK_PChar str);


/**
 *
 * @param[in]			Alloctr			模块内存分配器。
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateArray(NK_Allocator *Alloctr);


/**
 *
 * @param[in]			Alloctr			模块内存分配器。
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateObject(NK_Allocator *Alloctr);


/**
 *
 * @param[in]			Alloctr			模块内存分配器。
 * @param numbers
 * @param len
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateIntArray(NK_Allocator *Alloctr, NK_Int *numbers, NK_Size len);


/**
 *
 * @param[in]			Alloctr			模块内存分配器。
 * @param numbers
 * @param len
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateFloatArray(NK_Allocator *Alloctr, NK_Float *numbers, NK_Size len);


/**
 *
 * @param[in]			Alloctr			模块内存分配器。
 * @param numbers
 * @param len
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateDFloatArray(NK_Allocator *Alloctr, NK_DFloat *numbers, NK_Size len);


/**
 *
 * @param[in]			Alloctr			模块内存分配器。
 * @param strings
 * @param len
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_CreateStringArray(NK_Allocator *Alloctr, NK_PChar *strings, NK_Size len);


/**
 *
 * @param Array
 * @param Item
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_JSON *
NK_JSON_AddItemToArray(NK_JSON *Array, NK_JSON *Item);


/**
 * @brief
 *  往 JSON 对象中加入一个新的子对象成员。\n
 *  注意的是，如果 JSON 对象中本身存在键值为 @ref key 的值，调用此接口不会将其覆盖，\n
 *  而是新增一个键值一样的子对象，这样可以兼容某些协议的数据包要求。\n
 *  但本模块只支持操作第一个添加的键值对应的数据。
 *
 * @param[in] Obj
 *  JSON 数据结构对象。
 *
 * @param[in] key
 *  JSON 数据结构对应的键值。
 *
 * @param[in] Item
 *  加入的 JSON 对象。
 *
 * @retval JSON 对象。
 *  加入新对象成员成功，并返回新对象成员的句柄。\n
 *
 * @retval Nil
 * 	加入失败。\n
 *
 */
NK_API NK_JSON *
NK_JSON_AddItemToObject(NK_JSON *Obj, NK_PChar key, NK_JSON *Item);

/**
 *
 * @param[in]			Obj				JSON 数据结构对象。
 * @param key
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_JSON *
NK_JSON_AddNullToObject(NK_JSON *Obj, NK_PChar key);


/**
 *
 * @param[in]			Obj				JSON 数据结构对象。
 * @param key
 * @param b
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_JSON *
NK_JSON_AddBooleanToObject(NK_JSON *Obj, NK_PChar key, NK_Boolean b);


/**
 *
 * @param[in]			Obj				JSON 数据结构对象。
 * @param key
 * @param number
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_JSON *
NK_JSON_AddNumberToObject(NK_JSON *Obj, NK_PChar key, NK_DFloat number);


/**
 *
 * @param[in]			Obj				JSON 数据结构对象。
 * @param key
 * @param str
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_JSON *
NK_JSON_AddStringToObject(NK_JSON *Obj, NK_PChar key, NK_PChar str);

/**
 *
 * @param Array
 * @param which
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_DetachItemFromArray(NK_JSON *Array, NK_Size which);


/**
 *
 * @param[in]			Obj				JSON 数据结构对象。
 * @param key
 *
 * @return	对象数据对象。
 */
NK_API NK_JSON *
NK_JSON_DetachItemFromObject(NK_JSON *Obj, NK_PChar key);

/**
 *
 * @param Array
 * @param which
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_Int
NK_JSON_DropItemFromArray(NK_JSON *Array, NK_Size which);


/**
 *
 * @param[in]			Obj				JSON 数据结构对象。
 * @param key
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_Int
NK_JSON_DropItemFromObject(NK_JSON *Obj, NK_PChar key);


/**
 *
 * @param Array
 * @param which
 * @param Item
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_JSON *
NK_JSON_SetItemInArray(NK_JSON *Array, NK_Size which, NK_JSON *Item);


/**
 * 替换对象结构中的某个子对象。
 *
 * @param[in]			Obj				JSON 数据结构对象。
 * @param key
 * @param Item
 *
 * @return	加入成功返回 0，错误返回 -1。
 */
NK_API NK_JSON *
NK_JSON_SetItemInObject(NK_JSON *Obj, NK_PChar key, NK_JSON *Item);

/**
 * @brief
 *  更新 JSON 数据对象内容。
 */
NK_API NK_JSON *
NK_JSON_SetItem(NK_JSON *Obj, NK_JSON *Item);

/**
 * @brief
 *  检索 JSON 数据对象的同级下一个对象。
 */
NK_API NK_JSON *
NK_JSON_NextItem(NK_JSON *Obj);

/**
 * @brief
 *  强制转换对象类型为数组。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] type
 *  转换的数据类型。
 *
 * @retval 0
 *  强制转换成功。
 *
 * @retval -1
 *  强制转换失败。
 */
NK_API NK_Int
NK_JSON_ForceType(NK_JSON *Obj, NK_JSONType type);

/**
 * @macro
 *  JSON 路径最大文本长度。
 */
#define NK_JS_PATH_MAX_LEN (1024)

/**
 * @brief
 *  通过 JSON 路径查询对象元素。\n
 *  当 @ref Item 为 Nil 时表示读取该路径上的对象并返回，\n
 *  反之则更新该路径上的对象。\n
 *  路径表达式符合 JSONPath 标准的子集，详见 http://goessner.net/articles/JsonPath/index.html。\n
 *  目前支持通过 “.” 和 “[]” 表达式索引，如：\n
 *  \n
 *  store.book[0].title \n
 *  $.store[0].color \n
 *  $.store.persion[2][0][0] \n
 *  $.store.persion[(@.length-2)][(@.length-1)][0] \n
 *  $.store['bicycle'].color \n
 *  其它复杂表达式暂不支持。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] jpath
 *  JSON 描述路径，最大长度为 @ref NK_JS_PATH_MAX_LEN 个字节。\n
 *
 * @retval JSON 数据对象
 *  查询成功。
 *
 * @retval Nil
 *  查询失败，如果是查询不到对应的路径或者可能是由于参数错误导致。
 *
 */
NK_API NK_JSON *
NK_JSON_QueryPath(NK_JSON *Obj, NK_PChar jpath);

/**
 * @brief
 *  通过 JSON 路径应用对象元素。\n
 *  JSON 路径规则见 @ref NK_JSON_QueryPath()。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] jpath
 *  JSON 描述路径，最大长度为 @ref NK_JS_PATH_MAX_LEN 个字节。\n
 *
 * @param[in] Item
 *  当为 Nil 时表示读取该路径上的对象并返回，\n
 *  反之则更新该路径上的对象并返回。\n
 *
 * @retval JSON 数据对象
 *  查询成功。
 *
 * @retval Nil
 *  查询失败，如果是读查询（@ref Item 为 Nil）可能是找不到对应的路径，\n
 *  如果是更新查询可能是由于参数错误导致。
 *
 */
NK_API NK_JSON *
NK_JSON_ApplyPath(NK_JSON *Obj, NK_PChar jpath, NK_JSON *Item);

/**
 * @brief
 *  通过 JSON 路径删除对象元素。\n
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] jpath
 *  JSON 描述路径，最大长度为 @ref NK_JS_PATH_MAX_LEN 个字节。\n
 *
 * @retval 0
 * 	删除成功。
 *
 * @retval -1
 *  删除失败。
 */
NK_API NK_Int
NK_JSON_DropPath(NK_JSON *Obj, NK_PChar jpath);

/**
 * @brief
 *  查询元素集合，通过 JSON 路径查询一系列相关元素集合。
 *  查询成功后会返回一个由 @ref Obj 内部内存分配器创建的 JSON 数组，
 *  在用户对 JSON 数组使用后需要手动调用 @ref NK_JSON_Free() 方法将其释放。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] jpath
 *  JSON 集合描述路径。
 *
 * @retval JSON 数组
 *  查询到的数据在 JSON 集合内，可通过 @ref NK_JSON_ArrayOf() 对集合内容进行遍历，\n
 *  如果查询失败，数据集合为空。
 *
 * @retval Nil
 *  查询失败，可能参数错误。
 */
NK_JSON *
NK_JSON_QueryCollection(NK_JSON *Obj, NK_PChar jpath);


/**
 * @macro
 *  @ref NK_JSON_QueryPath() 快速操作。\n
 *  在路径上获取一个数据对象操作。
 */
#define NK_JSON_GetItemOnPath(__Obj, __jpath) \
	NK_JSON_QueryPath((__Obj), (__jpath))

/**
 * @macro
 *  @ref NK_JSON_QueryPath() 快速操作。\n
 *  在路径上获取一个布尔数据操作，如果该路径上不存在返回缺省值 @ref __def。
 */
#define NK_JSON_GetBooleanOnPath(__Obj, __jpath, __def) \
	NK_JSON_GetBoolean(NK_JSON_GetItemOnPath((__Obj), (__jpath)), (__def))

/**
 * @macro
 *  @ref NK_JSON_QueryPath() 快速操作。\n
 *  在路径上获取一个数字数据操作，如果该路径上不存在返回缺省值 @ref __def。
 */
#define NK_JSON_GetNumberOnPath(__Obj, __jpath, __def) \
	NK_JSON_GetNumber(NK_JSON_GetItemOnPath((__Obj), (__jpath)), (__def))

/**
 * @macro
 *  @ref NK_JSON_QueryPath() 快速操作。\n
 *  在路径上获取一个文本数据操作，如果该路径上不存在返回缺省值 @ref __def。
 */
#define NK_JSON_GetStringOnPath(__Obj, __jpath, __def) \
	NK_JSON_GetString(NK_JSON_GetItemOnPath((__Obj), (__jpath)), (__def))

/**
 * @macro
 *  @ref NK_JSON_QueryPath() 快速操作。\n
 *  在路径上设置一个数据对象操作，如果该路径上不存在增加该对象，如果存在则对其更新。
 */
#define NK_JSON_SetItemOnPath(__Obj, __jpath, __Item) \
	NK_JSON_ApplyPath((__Obj), (__jpath), (__Item))

/**
 * @macro
 *  @ref NK_JSON_QueryPath() 快速操作。\n
 *  在路径上设置一个布尔数据操作，如果该路径上不存在增加该对象，如果存在则对其更新。
 */
#define NK_JSON_SetBooleanOnPath(__Obj, __jpath, __b) \
	do {\
		NK_JSON *__Bl = NK_JSON_CreateBoolean(NK_JSON_Allocator(__Obj), __b);\
		if (NK_Nil != __Bl) {\
			if (!NK_JSON_ApplyPath((__Obj), (__jpath), __Bl)) {\
				NK_JSON_Free(&__Bl);\
			}\
		}\
	} while (0)

/**
 * @macro
 *  @ref NK_JSON_QueryPath() 快速操作。\n
 *  在路径上设置一个数字数据操作，如果该路径上不存在增加该对象，如果存在则对其更新。
 */
#define NK_JSON_SetNumberOnPath(__Obj, __jpath, __num) \
	do {\
		NK_JSON *__Num = NK_JSON_CreateNumber(NK_JSON_Allocator(__Obj), __num);\
		if (NK_Nil != __Num) {\
			if (!NK_JSON_ApplyPath((__Obj), (__jpath), __Num)) {\
				NK_JSON_Free(&__Num);\
			}\
		}\
	} while (0)

/**
 * @macro
 *  @ref NK_JSON_QueryPath() 快速操作。\n
 *  在路径上设置一个文本数据操作，如果该路径上不存在增加该对象，如果存在则对其更新。
 */
#define NK_JSON_SetStringOnPath(__Obj, __jpath, __str) \
	do {\
		NK_JSON *__Str = NK_JSON_CreateString(NK_JSON_Allocator(__Obj), __str);\
		if (NK_Nil != __Str) {\
			if (!NK_JSON_ApplyPath((__Obj), (__jpath), __Str)) {\
				NK_JSON_Free(&__Str);\
			}\
		}\
	} while (0)


/**
 * @brief
 *  压缩 JSON 文本，\n
 *  去掉文本中 JSON 有效数据结以外的空格、换行和缩进等符号，\n
 *  达到节省文本消耗内存大小的目的。
 *
 * @param[in,out] jsonp
 *  JSON 文本数据。
 *
 * @return 0
 *  操作成功。
 */
NK_API NK_Int
NK_JSON_Vacuum(NK_PChar jsonp);


/**
 * @brief
 *  转换 JSON 数据对象成文本。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] pretty
 *  美化标识，为 True 输出更适合阅读，但占据较大的内存空间。
 *
 * @param[out] text
 *  栈区内存的起始地址，转换成功后结果会保存在这个区域。
 *
 * @param[in,out] size
 *  传入栈区内存的大小，返回生成文本长度。
 *
 * @retval 0
 *  转换成功。
 *
 * @retval -1
 *  转换失败，可能参数错误。
 */
NK_API NK_Int
NK_JSON_ToText(NK_JSON *Obj, NK_Boolean pretty, NK_PChar text, NK_Size *size);

/**
 * @brief
 *  转换 JSON 数据对象成文本。\n
 *  与 @ref NK_JSON_ToText() 的差异在于结构从返回值中获得，\n
 *  返回得到的数据指针不需要额外释放。
 *
 * @param[in] Obj
 *  JSON 数据对象。
 *
 * @param[in] pretty
 *  美化标识，为 True 输出更适合阅读，但占据较大的内存空间。
 *
 * @retval 文本数据的起始位置。
 *  转换成功。
 *
 * @retval Nil
 *  转换失败，可能参数错误。
 */
NK_API NK_PChar
NK_JSON_Stringify(NK_JSON *Obj, NK_Boolean pretty);

/**
 * @brief
 *  转换 JSON 数据对象成文本。
 * @details
 *  转换 @ref Obj 数据结构成 JSON 文本，结果存至 @ref stack 内存空间。
 *
 * @param Obj [in]
 *  JSON 数据对象。
 * @param pretty [in]
 *  美化标识，为 True 输出更适合阅读，但占据较大的内存空间。
 * @param stack [out]
 *  缓存内存空间地址起始位置。
 * @param stacklen [in]
 *  缓存内存空间的大小。
 *
 * @return
 *  转换成功返回文本长度，结果存于 @ref stack 中，转换失败返回 -1。
 *
 */
NK_API NK_SSize
NK_JSON_StringifyV2(NK_JSON *Obj, NK_Boolean pretty, NK_PChar stack, NK_Size stacklen);

/**
 * @brief
 *  转换成 Javascript 变量定义文本。
 *
 *
 */
NK_API NK_SSize
NK_JSON_ToJSVar(NK_JSON *Obj, NK_PChar key, NK_PChar stack, NK_Size stacklen);


/**
 * @macro
 *  JSON 数据对象遍历操作。
 */
#define NK_JSON_FOREACH(__Obj, __Child) \
	for((__Child) = NK_JSON_ArrayOf((__Obj), 0); NK_Nil != (__Child); (__Child) = NK_JSON_NextItem((__Child)))

/**
 * @macro
 *  清空 JSON 数据对象操作。
 */
#define NK_JSON_EMPTY(__Obj) \
	do {\
		if (NK_Nil != (__Obj)) {\
			while (NK_JSON_ArraySize(__Obj) > 0) {\
				NK_JSON_DropItemFromArray((__Obj), 0);\
			}\
		}\
	} while (0)

/**
 * @brief
 *  创建差异补丁。\n
 *  补丁同样是一个 JSON 数据对象，由 @ref Alloctr 创建，\n
 *  用户在使用该对象之后按照 JSON 数据对象释放方法 @ref NK_JSON_Free() 释放该对象，否则会造成内存泄漏。\n
 *  详见 https://tools.ietf.org/html/rfc7396。
 *
 * @param[in] Alloctr
 *  创建差异补丁的内存分配器。
 *
 * @param[in] Objsrc
 *  对比差异源对象。
 *
 * @param[in] Objdst
 *  对比差异目标对象。
 *
 * @retval JSON 数据对象
 *  对比差异成功。
 *
 * @retval Nil
 *  对比差异失败。
 */
NK_API NK_JSON *
NK_JSON_CreatePatch(NK_Allocator *Alloctr, NK_JSON *Objsrc, NK_JSON *Objdst);

/**
 * @brief
 *  合并补丁到数据对象。\n
 *  合并之后，@ref Patch 对象不会被释放，\n
 *  用户使用 @ref NK_JSON_Free() 方法释放该对象，否则会造成内存泄漏。\n
 *  详见 https://tools.ietf.org/html/rfc7396。
 *
 * @param[in] Obj
 *  目标对象，此方法会直接根据补丁内容修改补丁
 *
 * @param[in] Patch
 *  通过 @ref NK_JSON_CreatePatch() 方法创建的补丁对象。
 *
 * @return
 */
NK_API NK_JSON *
NK_JSON_MergePatch(NK_JSON *Obj, NK_JSON *Patch);

NK_CPP_EXTERN_END
#endif /* NK_UTILS_JSON_H_ */
