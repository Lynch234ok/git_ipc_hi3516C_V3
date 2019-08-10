/**
 * HTTP 协议基础工具集定义。
 *
 */

#include <NkUtils/types.h>
#include <NkUtils/allocator.h>
#include <NkUtils/str_list.h>
#include <NkUtils/assert.h>

#ifndef NK_HTTP_UTILS_H_
# define NK_HTTP_UTILS_H_
NK_CPP_EXTERN_BEGIN


/**
 * HTTP 换行符定义。
 */
#define NK_HTTP_CRLF "\r\n"

/**
 * HTTP 回复码定义。
 */
typedef enum Nk_HTTPCode
{
	NK_HTTP_CODE_CONTINUE = (100),                       //!< NK_HTTP_CODE_CONTINUE
	NK_HTTP_CODE_SWITCHING_PROTOCOLS = (101),            //!< NK_HTTP_CODE_SWITCHING_PROTOCOLS
	NK_HTTP_CODE_OK = (200),                             //!< NK_HTTP_CODE_OK
	NK_HTTP_CODE_CREATED = (201),                        //!< NK_HTTP_CODE_CREATED
	NK_HTTP_CODE_ACCEPTED = (202),                       //!< NK_HTTP_CODE_ACCEPTED
	NK_HTTP_CODE_NON_AUTHORITATIVE_INFOMATION = (203),   //!< NK_HTTP_CODE_NON_AUTHORITATIVE_INFOMATION
	NK_HTTP_CODE_NO_CONTENT = (204),                     //!< NK_HTTP_CODE_NO_CONTENT
	NK_HTTP_CODE_RESET_CONTENT = (205),                  //!< NK_HTTP_CODE_RESET_CONTENT
	NK_HTTP_CODE_PARTIAL_CONTENT = (206),                //!< NK_HTTP_CODE_PARTIAL_CONTENT
	NK_HTTP_CODE_MULTIPLE_CHOICES = (300),               //!< NK_HTTP_CODE_MULTIPLE_CHOICES
	NK_HTTP_CODE_MOVED_PERMANENTLY = (301),              //!< NK_HTTP_CODE_MOVED_PERMANENTLY
	NK_HTTP_CODE_FOUND = (302),                          //!< NK_HTTP_CODE_FOUND
	NK_HTTP_CODE_SEE_OTHER = (303),                      //!< NK_HTTP_CODE_SEE_OTHER
	NK_HTTP_CODE_NOT_MODIFIED = (304),                   //!< NK_HTTP_CODE_NOT_MODIFIED
	NK_HTTP_CODE_USE_PROXY = (305),                      //!< NK_HTTP_CODE_USE_PROXY
	NK_HTTP_CODE_TEMPORARY_REDIRECT = (307),             //!< NK_HTTP_CODE_TEMPORARY_REDIRECT
	NK_HTTP_CODE_BAD_REQUEST = (400),                    //!< NK_HTTP_CODE_BAD_REQUEST
	NK_HTTP_CODE_UNAUTHORIZED = (401),                   //!< NK_HTTP_CODE_UNAUTHORIZED
	NK_HTTP_CODE_PAYMENT_REQUIRED = (402),               //!< NK_HTTP_CODE_PAYMENT_REQUIRED
	NK_HTTP_CODE_FORBIDDEN = (403),                      //!< NK_HTTP_CODE_FORBIDDEN
	NK_HTTP_CODE_NOT_FOUND = (404),                      //!< NK_HTTP_CODE_NOT_FOUND
	NK_HTTP_CODE_METHOD_NOT_ALLOWED = (405),             //!< NK_HTTP_CODE_METHOD_NOT_ALLOWED
	NK_HTTP_CODE_NOT_ACCEPTABLE = (406),                 //!< NK_HTTP_CODE_NOT_ACCEPTABLE
	NK_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED = (407),  //!< NK_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED
	NK_HTTP_CODE_REQUEST_TIME_OUT = (408),               //!< NK_HTTP_CODE_REQUEST_TIME_OUT
	NK_HTTP_CODE_CONFLICT = (409),                       //!< NK_HTTP_CODE_CONFLICT
	NK_HTTP_CODE_GONE = (410),                           //!< NK_HTTP_CODE_GONE
	NK_HTTP_CODE_LENGTH_REQUIRED = (411),                //!< NK_HTTP_CODE_LENGTH_REQUIRED
	NK_HTTP_CODE_PRECONDITION_FAILED = (412),            //!< NK_HTTP_CODE_PRECONDITION_FAILED
	NK_HTTP_CODE_REQUEST_ENTITY_TOO_LARGE = (413),       //!< NK_HTTP_CODE_REQUEST_ENTITY_TOO_LARGE
	NK_HTTP_CODE_REQUEST_URI_TOO_LARGE = (414),          //!< NK_HTTP_CODE_REQUEST_URI_TOO_LARGE
	NK_HTTP_CODE_UNSUPPORTED_MEDIA_TYPE = (415),         //!< NK_HTTP_CODE_UNSUPPORTED_MEDIA_TYPE
	NK_HTTP_CODE_REQUESTED_RANGE_NOT_SATISFIABLE = (416),//!< NK_HTTP_CODE_REQUESTED_RANGE_NOT_SATISFIABLE
	NK_HTTP_CODE_EXPECTATION_FAILED = (417),             //!< NK_HTTP_CODE_EXPECTATION_FAILED
	NK_HTTP_CODE_INTERNAL_SERVER_ERROR = (500),          //!< NK_HTTP_CODE_INTERNAL_SERVER_ERROR
	NK_HTTP_CODE_NOT_IMPLEMENTED = (501),                //!< NK_HTTP_CODE_NOT_IMPLEMENTED
	NK_HTTP_CODE_BAD_GATEWAY = (502),                    //!< NK_HTTP_CODE_BAD_GATEWAY
	NK_HTTP_CODE_SERVICE_UNAVAILABLE = (503),            //!< NK_HTTP_CODE_SERVICE_UNAVAILABLE
	NK_HTTP_CODE_GATEWAY_TIME_OUT = (504),               //!< NK_HTTP_CODE_GATEWAY_TIME_OUT
	NK_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED = (505),     //!< NK_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED

} NK_HTTPCode;


/**
 * 通过文件名称获取文件的 MIME。\n
 * 接口根据文件后缀获取 MIME，因此文件名称必须带有后缀，如 file.txt。
 *
 */
NK_API NK_PChar
NK_HTTPUtils_FileMIME(NK_PChar file_name);


/**
 * 获取 HTTP 返回码对应的默认消息。
 *
 * @param[in]		code		HTTP 返回码。
 *
 * @return	返回码对应消息。
 */
NK_API NK_PChar
NK_HTTPUtils_ReasonPhrase(NK_UInt32 code);


/**
 * @brief
 *
 * @details
 *  不对 @ * / + 符号进行转码。
 *
 * @param str [in]
 *  原始字符串。
 * @param trans [out]
 *  编码后的结果所在内存起始位置。
 * @param trans_len [in,out]
 *  传入编码内存大小，返回编码后字符串长度。
 *
 * @return
 *  编码成功返回 0，并在 @ref trans 内存区域返回编码结果，失败返回 -1。
 *
 */
NK_API NK_Int
NK_HTTPUtils_Escape(NK_PChar str, NK_PChar trans, NK_Size *trans_len);

/**
 * 解码 URI 字符串。\n
 * 不支持 UTF-8 编码转换。\n
 * 传入的 @ref enc 和 @ref uri 可以指向同一块内存块。
 *
 * @param str [in]
 *  原始解码字符串。
 * @param trans [out]
 *  解码后的结果所在内存起始位置。
 * @param trans_len [in,out]
 *  传入解码内存大小，返回解码后字符串长度。
 *
 * @return
 *  解码成功返回 0，并在 @ref trans 内存区域返回解码结果，失败返回 -1。
 */
NK_API NK_Int
NK_HTTPUtils_Unescape(NK_PChar str, NK_PChar trans, NK_Size *trans_len);

/**
 * 编码 URI 字符串，\n
 * 不支持 UTF-8 编码转换。
 *
 * @details
 *  不对符号 ~ ! @ # $ & * ( ) = : / , ; ? + 进行转码。
 *
 * @param str [in]
 *  原始字符串。
 * @param trans [out]
 *  编码后的结果所在内存起始位置。
 * @param trans_len [in,out]
 *  传入编码内存大小，返回编码后字符串长度。
 *
 * @return
 *  编码成功返回 0，并在 @ref trans 内存区域返回编码结果，失败返回 -1。
 */
NK_API NK_Int
NK_HTTPUtils_EncodeURI(NK_PChar str, NK_PChar trans, NK_Size *trans_len);

/**
 * 解码 URI 字符串。\n
 * 不支持 UTF-8 编码转换。\n
 * 传入的 @ref enc 和 @ref uri 可以指向同一块内存块。
 *
 * @param str [in]
 *  原始解码字符串。
 * @param trans [out]
 *  解码后的结果所在内存起始位置。
 * @param trans_len [in,out]
 *  传入解码内存大小，返回解码后字符串长度。
 *
 * @return
 *  解码成功返回 0，并在 @ref trans 内存区域返回解码结果，失败返回 -1。
 */
NK_API NK_Int
NK_HTTPUtils_DecodeURI(NK_PChar str, NK_PChar trans, NK_Size *trans_len);


/**
 *
 * @details
 *  不对符号 ~ ! * ( ) 进行转码。
 *
 * @param str [in]
 *  原始字符串。
 * @param trans [out]
 *  编码后的结果所在内存起始位置。
 * @param trans_len [in,out]
 *  传入编码内存大小，返回编码后字符串长度。
 *
 * @return
 *  编码成功返回 0，并在 @ref trans 内存区域返回编码结果，失败返回 -1。
 *
 */
NK_API NK_Int
NK_HTTPUtils_EncodeURIComponent(NK_PChar str, NK_PChar trans, NK_Size *trans_len);

/**
 * 解码 URI 字符串。\n
 * 不支持 UTF-8 编码转换。\n
 * 传入的 @ref enc 和 @ref uri 可以指向同一块内存块。
 *
 *
 * @param str [in]
 *  原始解码字符串。
 * @param trans [out]
 *  解码后的结果所在内存起始位置。
 * @param trans_len [in,out]
 *  传入解码内存大小，返回解码后字符串长度。
 *
 * @return
 *  解码成功返回 0，并在 @ref trans 内存区域返回解码结果，失败返回 -1。
 */
NK_API NK_Int
NK_HTTPUtils_DecodeURIComponent(NK_PChar str, NK_PChar trans, NK_Size *trans_len);




/**
 * URL 数据结构。
 */
#pragma pack(push, 4)
typedef struct Nk_HTTPURL
{
	NK_PChar protocol;
	NK_PChar host;
	NK_UInt16 port;
	NK_PChar abs_path;
	NK_PChar query;

	/**
	 * 保留缓冲。
	 */
	NK_Byte reserved[1024 * 5];
} NK_HTTPURL;
#pragma pack(pop)

/**
 * 打印 @ref NK_HTTPURL 数据结构。
 */
#define NK_HTTP_URL_DUMP(__URL) \
	do{\
		NK_TermTable Tbl;\
		NK_TermTbl_BeginDraw(&Tbl, "URL", 96, 4);\
		NK_TermTbl_PutKeyValue(&Tbl, NK_True, "Protocol", "%s", (__URL)->protocol);\
		NK_TermTbl_PutKeyValue(&Tbl, NK_True, "Host", "%s", (__URL)->host);\
		NK_TermTbl_PutKeyValue(&Tbl, NK_True, "Port", "%d", (NK_Int)((__URL)->port));\
		NK_TermTbl_PutKeyValue(&Tbl, NK_True, "Absolute Path", "%s", (__URL)->abs_path);\
		if (NK_Nil != (__URL)->query) {\
			NK_TermTbl_PutKeyValue(&Tbl, NK_True, "Query String", "%s", (__URL)->query);\
		}\
		NK_TermTbl_EndDraw(&Tbl);\
	} while(0)


/**
 * @brief
 *  压缩 URI。去掉 @ref uri 所在字符串开始和结束位置连续的空格，\n
 *  去掉字符串中间出现连续的 / 和 \ 字符。\n
 *
 * @param[in] uri
 *  传入 URI 字符串。\n
 *
 * @param[out] stack
 *  栈区内存位置。\n
 *
 * @param[in] stacklen
 *  栈区内存长度。\n
 *
 * @retval >=0
 *  压缩后的 URI 长度，结果在 @ref stack。\n
 *
 * @retval -1。
 *  压缩失败。\n
 */
NK_API NK_SSize
NK_HTTPUtils_StripURI(NK_PChar uri, NK_PChar stack, NK_Size stacklen);


/**
 * 解析 URL 字符串。
 */
NK_API NK_Int
NK_HTTPUtils_ParseURL(NK_PChar url, NK_HTTPURL *URL);

/**
 * @brief
 *  解析查询字符串。\n
 */
NK_API NK_StrList *
NK_HTTPUtils_ParseQuery(NK_Allocator *Alloctr, NK_PChar query);


/**
 * HTTPHeadField 模块句柄。
 */
typedef struct Nk_HTTPHeadField {
#define NK_This struct Nk_HTTPHeadField *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

	/**
	 * 请求/回复标识。
	 */
	NK_Boolean isRequest;

	/**
	 * 协议。
	 */
	NK_PChar protocol;

	/**
	 * 版本号。
	 */
	NK_UInt32 ver_maj, ver_min;

	/**
	 * 当 isRequest 为 True 时有效。
	 */
	struct {
		NK_PChar method;
		NK_PChar abs_path;
		NK_PChar query;
	};
	/**
	 * 当 isRequest 为 False 时有效。
	 */
	struct {
		NK_UInt32 code;
		NK_PChar reason_phrase;
	};

	/**
	 * 设置协议。
	 */
	NK_Int
	(*setProtocol)(NK_This, NK_PChar protocol, NK_Int ver_maj, NK_Int ver_min);

	/**
	 * 设置成请求数据头。
	 */
	NK_Int
	(*setRequest)(NK_This, NK_PChar method, NK_PChar abs_path, NK_PChar query);

	/**
	 * 设置成回复数据头。
	 */
	NK_Int
	(*setResponse)(NK_This, NK_UInt32 status_code, NK_PChar reason_phrase);

	/**
	 * 头域加入查询讯息。\n
	 * 请求模式时有效，否则返回 -1。
	 *
	 */
	NK_Int
	(*addQuery)(NK_This, NK_PChar key, NK_PChar fmt, ...);

	/**
	 * 头域加入查询讯息。\n
	 * 请求模式时有效，否则返回 -1。
	 *
	 * @return		关键字不存在或者删除成功返回 0，参数错误时返回 -1。
	 */
	NK_Int
	(*dropQuery)(NK_This, NK_PChar key);

	/**
	 * 获取 Query 的数量。
	 */
	NK_Size
	(*numberOfQuery)(NK_This);

	/**
	 * 通过序号获取 Query 信息。
	 *
	 * @param[in]			id				Query 变量所在序号。
	 * @param[out]			key				Query 变量所在关键字。
	 * @param[out]			value			Query 变量关键字对应数值。
	 *
	 * @return		获取成功返回 0，否则返回 -1。
	 */
	NK_Int
	(*indexOfQuery)(NK_This, NK_Int id, NK_PChar *key, NK_PChar *value);

	/**
	 * 获取头域查询讯息。\n
	 * 请求模式时有效，否则返回 -1。
	 *
	 */
	NK_PChar
	(*getQuery)(NK_This, NK_PChar key, NK_PChar def);

	/**
	 * 获取头域某一查询讯息的个数。
	 *
	 */
	NK_Boolean
	(*hasQuery)(NK_This, NK_PChar key);

	/**
	 * 头域加入选项信息。
	 *
	 */
	NK_Int
	(*addOption)(NK_This, NK_Boolean overwrite, NK_PChar opt, NK_PChar fmt, ...);

	/**
	 * 头域加入选项信息。
	 *
	 * @return		关键字不存在或者删除成功返回 0，参数错误时返回 -1。
	 */
	NK_Int
	(*dropOption)(NK_This, NK_Boolean all, NK_PChar opt);

	/**
	 * 获取标签数量。
	 *
	 * @return		标签选项的数量。
	 */
	NK_Size
	(*numberOfOption)(NK_This);

	/**
	 * 通过标签序号获取标签名称和信息。
	 *
	 * @param[in]			id				标签选项序号。
	 * @param[out]			key				序号对应下标签名称。
	 * @param[out]			value			序号对应下标签信息。
	 *
	 * @return		获取成功返回 0，在 @ref opt 和 @ref value 中获取标签选项结果。
	 */
	NK_Int
	(*indexOfOption)(NK_This, NK_Int id, NK_PChar *key, NK_PChar *value);


	/**
	 * 通过标签选项名称获取其信息。
	 *
	 * @param[in]			key				标签选项的名称。
	 * @param[in]			def				默认标签选项的数值，当 @ref opt 不存在的时候会使用默认值作为返回，可以传入 Nil。
	 *
	 * @return		返回标签选项的数值，当标签不存在的时候会返回 @ref def 默认值。
	 */
	NK_PChar
	(*getOption)(NK_This, NK_PChar key, NK_PChar def);

	/**
	 * 获取头域某一选项的个数。
	 *
	 */
	NK_SSize
	(*hasOption)(NK_This, NK_PChar opt);

	/**
	 * 生成文本数据。
	 *
	 */
	NK_Int
	(*toText)(NK_This, NK_PChar text, NK_Size *text_len);

#undef NK_This
} NK_HTTPHeadField;

/**
 * 打印输出数据结构 NK_HTTPHeadField。
 */
#define NK_HTTP_HEAD_FIELD_DUMP(__HeadField) \
	do{\
		NK_TermTable Table;\
		NK_Size number = 0;\
		NK_PChar key = NK_Nil;\
		NK_PChar value = NK_Nil;\
		NK_Int i = 0;\
		\
		NK_CHECK_POINT();\
		NK_TermTbl_BeginDraw(&Table, "HTTP Head Filed", 96, 4);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Protocol", "%s %u.%u",\
				(__HeadField)->protocol, (__HeadField)->ver_maj, (__HeadField)->ver_min);\
		if ((__HeadField)->isRequest) {\
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Method", "%s", (__HeadField)->method);\
			NK_TermTbl_PutKeyValue(&Table, NK_True, "Absolute Path", "%s", (__HeadField)->abs_path);\
			number = (__HeadField)->numberOfQuery((__HeadField));\
			NK_TermTbl_PutText(&Table, NK_True, "Query String (%u)", number);\
			for (i = 0; i < (NK_Int)number; ++i) {\
				if (0 == (__HeadField)->indexOfQuery((__HeadField), i, &key, &value)) {\
					NK_TermTbl_PutKeyValue(&Table, (i == number - 1), key, "%s", value);\
				}\
			}\
		} else {\
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Code", "%u", (__HeadField)->code);\
			NK_TermTbl_PutKeyValue(&Table, NK_True, "Reason Phrase", "%s", (__HeadField)->reason_phrase);\
		}\
		\
		number = (__HeadField)->numberOfOption((__HeadField));\
		NK_TermTbl_PutText(&Table, NK_True, "Option Tag (%u)", number);\
		for (i = 0; i < (NK_Int)number; ++i) {\
			if (0 == (__HeadField)->indexOfOption((__HeadField), i, &key, &value)) {\
				NK_TermTbl_PutKeyValue(&Table, (i == number - 1), key, "%s", value);\
			}\
		}\
		NK_TermTbl_EndDraw(&Table);\
	} while(0)



/**
 * 加入 Host 选项标签。
 */
#define NK_HTTP_HEAD_FIELD_HOST(__HeadField, __str_host) \
	do {\
		if (NK_Nil != (__HeadField)) {\
			(__HeadField)->dropOption((__HeadField), NK_True, "Host");\
			(__HeadField)->addOption((__HeadField), NK_False, "Host", "%s", __str_host);\
		}\
	} while(0)

/**
 * 加入 Server 选项标签。
 */
#define NK_HTTP_HEAD_FIELD_SERVER(__HeadField, __str_server, __uint_ver1, __uint_ver2) \
	do {\
		if (NK_Nil != (__HeadField)) {\
			(__HeadField)->dropOption((__HeadField), NK_True, "Server");\
			if (0 != (__uint_ver1) && 0 != (__uint_ver2)) {\
				(__HeadField)->addOption((__HeadField), NK_True, "Server", "%s / %u.%u", (__str_server), (__uint_ver1), (__uint_ver2));\
			} else {\
				(__HeadField)->addOption((__HeadField), NK_True, "Server", "%s", (__str_server));\
			}\
		}\
	} while(0)

/**
 * 加入 Content-Type 选项标签。
 */
#define NK_HTTP_HEAD_FIELD_CONTENT_TYPE(__HeadField, __str_type) \
	do {\
		if (NK_Nil != (__HeadField)) {\
			(__HeadField)->dropOption((__HeadField), NK_True, "Content-Type");\
			(__HeadField)->addOption((__HeadField), NK_False, "Content-Type", "%s", (__str_type));\
		}\
	} while(0)

/**
 * 加入 Content-Length 选项标签。
 */
#define NK_HTTP_HEAD_FIELD_CONTENT_LENGTH(__HeadField, __uint_len) \
	do {\
		if (NK_Nil != (__HeadField)) {\
			(__HeadField)->dropOption((__HeadField), NK_True, "Content-Length");\
			(__HeadField)->addOption((__HeadField), NK_False, "Content-Length", "%u", (__uint_len));\
		}\
	} while(0)

/**
 * 加入 Content-Length 选项标签。
 */
#define NK_HTTP_HEAD_FIELD_CONNECTION(__HeadField, __uint_alive_s) \
	do {\
		if (NK_Nil != (__HeadField)) {\
			(__HeadField)->dropOption((__HeadField), NK_True, "Connection");\
			(__HeadField)->dropOption((__HeadField), NK_True, "Keep-Alive");\
			(__HeadField)->addOption((__HeadField), NK_False, "Connection", "%s", (__uint_alive_s > 0) ? "keep-alive" : "close");\
			if (__uint_alive_s > 0) {\
				NK_UInt32 timeout = __uint_alive_s;\
				timeout = timeout < 60U ? timeout : 60U;\
				(__HeadField)->addOption((__HeadField), NK_False, "Keep-Alive", "timeout=%u, max=%u", timeout, 60U);\
			}\
		}\
	} while(0)



/**
 * @brief
 *  获取报文中 HTTP 头的大小。
 *  此方法可以解析 HTTP 或类 HTTP 协议报文，
 *  解析成功后可以获取协议名称以及协议版本号。
 *
 * @param[in] package
 * 	用于分析的数据包。
 *
 * @param[in,out] pack_size
 * 	传入 @package 数据包长度，返回解析到的头域大小。
 *
 * @param[out] protocol
 * 	协议名称，当此方法解析成功时从这里返回协议名称。
 *
 * @param[out] ver_maj
 * 	协议主版本号，当此方法解析成功时从这里返回协议主版本号。
 *
 * @param[out] ver_min
 * 	协议次版本号，当此方法解析成功时从这里返回协议次版本号。
 *
 * @return
 *  如果报文格式符合 HTTP 头或者类 HTTP，返回 0，否则返回 -1。
 *
 */
NK_API NK_Int
NK_HTTPUtils_ExtractHeadField(NK_PChar package, NK_Size *pack_size, NK_PChar protocol, NK_Int *ver_maj, NK_Int *ver_min);

/**
 * 从报文中解析 HTTP 头的数据结构并获取句柄。\n
 * 内部会调用 @ref NK_HTTPUtils_ExtractHeadField() 和 @ref NK_HTTPUtils_CreateHeadField()。\n
 * 解析时候会尝试区匹配 @ref protocol 协议名称，如果协议名称传入 Nil 则不指定协议解析，\n
 * 如果协议名称不匹配，接口会返回失败。
 *
 */
NK_API NK_HTTPHeadField *
NK_HTTPUtils_ParseHeadField(NK_Allocator *Alloctr, NK_PChar protocol, NK_PChar package, NK_Size *len);

/**
 * @brief
 *  创建 HTTPHeadField 模块句柄。
 *
 * @return
 *  创建成功返回 HTTP 头域句柄，否则返回 Nil。
 */
NK_API NK_HTTPHeadField *
NK_HTTPUtils_CreateHeadField(NK_Allocator *Alloctr, NK_PChar protocol, NK_UInt32 ver_maj, NK_UInt32 ver_min);

/**
 * @brief
 *  销毁 HTTPHeadField 模块句柄。
 *
 * @return
 *  创建成功返回 HTTP 头域句柄，否则返回 Nil。
 */
NK_API NK_Int
NK_HTTPUtils_FreeHeadField(NK_HTTPHeadField **Field_r);


/**
 * @brief
 *  创建一个简单的报文。
 * @detais
 *  该方法会会把 @ref Headfield 头域数据结构展开成报文，并合并请求内容到用户指定的内存空间 @ref stack。\n
 *  数据结构展开以后 @ref Headfield 数据结构不会被释放，用户需要在外部对其进行释放，\n
 *  方法内部会更新 @ref Headfield 数据结构内的 Content-Type 标签。
 *
 * @param Headfield [in]
 *  头域数据结构。
 * @param content [in]
 *  发送内容内存起始地址，传入 Nil 则表示没有报文内容。
 * @param contentlen [in]
 *  发送内容的长度，传入 0 则表示没有报文。
 * @param stack [out]
 *  返回报文的所在内存起始地址，当打包报文成功以后从这里获取报文结果。
 * @param stacklen [in]
 *  返回报文的可用内存空间大小，如果空间不足以存放头域和内容则打包失败。
 *
 * @return
 *  创建成功返回报文长度，创建失败返回 -1。
 *
 */
NK_API NK_SSize
NK_HTTPUtils_SimplePacket(NK_HTTPHeadField *Headfield, NK_PByte content, NK_Size contentlen, NK_PByte stack, NK_Size stacklen);

/**
 * @brief
 *  创建一个简单的报文。
 * @detais
 *  该方法会会把 @ref Headfield 头域数据结构展开成报文，并合并请求内容到用户指定的内存空间 @ref stack。\n
 *  数据结构展开以后 @ref Headfield 数据结构不会被释放，用户需要在外部对其进行释放，\n
 *  方法内部会更新 @ref Headfield 数据结构内的 Content-Type 标签。
 *
 * @param ver [in]
 *  报文版本号（如：1.0），传入 Nil 则默认 1.1 版本。
 * @param method [in]
 *  请求方式。
 * @param abspath [in]
 *  请求绝对路径（如：http://192.168.0.1:10080/index.html?var=0），内部会解析其协议以及地址。
 * @param agent [in]
 *  代理名称，与报文头域 User-Agent 标签相关，传入 Nil 则表示不使用。
 * @param contenttype [in]
 *  发送内容类型，与报文头域 Content-Type 标签相关，传入 Nil 则表示不使用。
 * @param content [in]
 *  发送内容内存起始地址，传入 Nil 则表示没有报文内容。
 * @param contentlen [in]
 *  发送内容的长度，传入 0 则表示没有报文。
 * @param stack [out]
 *  返回报文的所在内存起始地址，当打包报文成功以后从这里获取报文结果。
 * @param stacklen [in]
 *  返回报文的可用内存空间大小，如果空间不足以存放头域和内容则打包失败。
 *
 * @return
 *  创建成功返回报文长度，创建失败返回 -1。
 *
 */
NK_API NK_SSize
NK_HTTPUtils_SimplePacketRequest(NK_PChar ver, NK_PChar method, NK_PChar abspath, NK_PChar agent, NK_PChar contenttype, NK_PByte content, NK_Size contentlen, NK_PByte stack, NK_Size stacklen);


/**
 * @brief
 *  创建一个简单的报文。
 * @detais
 *  该方法会会把 @ref Headfield 头域数据结构展开成报文，并合并请求内容到用户指定的内存空间 @ref stack。\n
 *  数据结构展开以后 @ref Headfield 数据结构不会被释放，用户需要在外部对其进行释放，\n
 *  方法内部会更新 @ref Headfield 数据结构内的 Content-Type 标签。
 *
 * @param ver [in]
 *  报文版本号（如：1.0），传入 Nil 则默认 1.1 版本。
 * @param protocol [in]
 *  请求协议，传入 Nil 默认 HTTP。
 * @param code [in]
 *  回复码。
 * @param server [in]
 *  服务名称，与报文头域 Server 标签相关，传入 Nil 则表示不使用。
 * @param contenttype [in]
 *  发送内容类型，与报文头域 Content-Type 标签相关，传入 Nil 则表示不使用。
 * @param content [in]
 *  发送内容内存起始地址，传入 Nil 则表示没有报文内容。
 * @param contentlen [in]
 *  发送内容的长度，传入 0 则表示没有报文。
 * @param stack [out]
 *  返回报文的所在内存起始地址，当打包报文成功以后从这里获取报文结果。
 * @param stacklen [in]
 *  返回报文的可用内存空间大小，如果空间不足以存放头域和内容则打包失败。
 *
 * @return
 *  创建成功返回报文长度，创建失败返回 -1。
 *
 */
NK_API NK_SSize
NK_HTTPUtils_SimplePacketResponse(NK_PChar ver, NK_PChar protocol, NK_HTTPCode code, NK_PChar server, NK_PChar contenttype, NK_PByte content, NK_Size contentlen, NK_PByte stack, NK_Size stacklen);




NK_CPP_EXTERN_END
#endif /* NK_HTTP_UTILS_H_ */

