/**
 * @brief
 *  HTTP 服务器模块定义。
 *
 * @details
 *  模块调用前可以通过 NK_Log::setModLevel 打开标签 HTTPServer 查看打印信息。
 *
 */


#include <NkUtils/types.h>
#include <NkUtils/allocator.h>
#include <NkUtils/macro.h>
#include <NkUtils/http_utils.h>
#include <NkEmbedded/thread.h>
#include <NkEmbedded/socket.h>
#include <NkEmbedded/spook2.h>


#if !defined(NK_HTTP_SERVER_H_)
#define NK_HTTP_SERVER_H_
NK_CPP_EXTERN_BEGIN

/**
 * @brief
 *  Keep-Alive 默认等待时长，单位：秒。
 */
#define NK_HTTP_SERVER_DEF_KEEP_ALIVE (15)

/**
 * @brief
 *  服务接受最大用户名长度。
 */
#define NK_HTTP_SERVER_MAX_USER_NAME_SZ (32)

/**
 * @brief
 *  服务接受最大用户密码长度。
 */
#define NK_HTTP_SERVER_MAX_USER_PASSPHRASE_SZ (NK_HTTP_SERVER_MAX_USER_NAME_SZ)

/**
 * @brief
 *  服务最大 realm 长度。
 */
#define NK_HTTP_SERVER_MAX_REALM_SZ (32)

/**
 * @brief
 *  服务器最大随机字段长度。
 */
#define NK_HTTP_SERVER_MAX_NONCE_SZ (64)

/**
 * @macro
 *  服务器最大连接数。
 */
#define NK_HTTP_SERVER_MAX_CONNECTION (32U)


/**
 * @brief
 *  最大头域传输长度。
 */
#define NK_HTTP_HEAD_FIELD_SZ (1024 * 4)

/**
 * @brief
 *  服务器缺省名称。
 */
#define NK_HTTP_SERVER_NAME "nginx"


/**
 * @brief
 *  用户校验类型。
 */
#define NK_HTTP_ACCESS_AUTH_NONE    (('N'<<0) | ('A'<<8)) ///< 没有用户校验。
#define NK_HTTP_ACCESS_AUTH_BASIC   (('B'<<0) | ('S'<<8) | ('C'<<16)) ///< 基本校验。
#define NK_HTTP_ACCESS_AUTH_DIGEST  (('D'<<0) | ('G'<<8) | ('S'<<16) | ('T'<<24)) ///< 摘要 MD5 校验。
typedef NK_Size Nk_HTTPAccessAuth;

/**
 * @macro
 *  兼容性定义。
 */
#define NK_HTTP_ACCESS_AUTH_DIGEST_MD5 NK_HTTP_ACCESS_AUTH_DIGEST

/**
 * @brief
 *  HTTP 用户校验集合。
 */
typedef struct Nk_HTTPAccessUsers {
#define NK_This struct Nk_HTTPAccessUsers *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

	/**
	 * @brief
	 *  新建一个用户，并指定她的用户名和密码。
	 * @details
	 *  用户必须唯一，如果模块内已经存在相同用户名的用户，则会覆盖。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param username [in]
	 *  用户名，最大长度见 @ref NK_HTTP_SERVER_MAX_USER_NAME_SZ 定义。
	 * @param passphrase [in]
	 *  用户密码，最大长度见 @ref NK_HTTP_SERVER_MAX_USER_PASSPHRASE_SZ 定义。
	 *
	 * @return
	 *  新增成功返回 0，失败返回 -1。
	 */
	NK_Int
	(*addUser)(NK_This, const NK_Char username[], const NK_Char passphrase[]);

	/**
	 * @brief
	 *  删除一个用户。
	 * @details
	 *  用户名对应用户必须已经通过 NK_HTTPAccessUsers::addUser 成功新建。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param username [in]
	 *  用户名，最大长度见 @ref NK_HTTP_SERVER_MAX_USER_NAME_SZ 定义。
	 *
	 * @return
	 *  删除成功返回 0，失败返回 -1。
	 */
	NK_Int
	(*dropUser)(NK_This, const NK_Char username[]);

	/**
	 * @brief
	 *  判断用户是否存在。
	 * @details
	 *  判断用户存在的同时获取用户密码。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param username [in]
	 *  用户名，最大长度见 @ref NK_HTTP_SERVER_MAX_USER_NAME_SZ 定义。
	 * @param passphrase [out]
	 *  用户对应的用户密码，如果用户存在会从此参数上返回该用户的用户密码。
	 *
	 * @return
	 *  用户存在返回 NK_True，不存在返回 NK_False。
	 */
	NK_Boolean
	(*hasUser)(NK_This, const NK_Char username[], NK_Char passphrase[]);

	/**
	 * @brief
	 * @details
	 *  详见 http://www.ietf.org/rfc/rfc2617.txt。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param accessable [in]
	 *  校验方式。
	 * @param username [in]
	 *  设备用户名，对于摘要认证，用户名明文，对于 Basic 来说，\n
	 *  此值传入 HTTP 头域 Authorization 标签 Basic 字段后的 base64 编码字段。
	 * @param realm [in]
	 *  场景参数，摘要认证时有效。
	 * @param nonce [in]
	 *  碰撞随机数，摘要认证时有效。
	 * @param response [in]
	 *  摘要回复，摘要认证时有效。
	 *
	 * @return
	 *  校验成功返回 True，否则返回 False。
	 */
	NK_Boolean
	(*challenge)(NK_This, Nk_HTTPAccessAuth access, NK_PChar method, NK_PChar realm, NK_PChar nonce, NK_PChar uri, NK_PChar authorization);


	/**
	 * @brief
	 *  与 @ref NK_HTTPAccessUsers::challenge() 类似。
	 * @details
	 *  详见 http://www.ietf.org/rfc/rfc2617.txt。
	 */
	NK_Boolean
	(*challengeV2)(NK_This, NK_PChar method, NK_PChar realm, NK_PChar nonce, NK_PChar authorization);

#undef NK_This
} NK_HTTPAccessUsers;


/**
 * @brief
 *  创建校验用户集数据结构句柄。
 *
 * @param Alloctr [in]
 *
 * @return
 *  创建成功返回用户集句柄，否则返回 Nil。
 */
NK_API NK_HTTPAccessUsers *
NK_HTTPServer_CreateAccessUsers(NK_Allocator *Alloctr);

/**
 * @brief
 *  释放校验用户集。
 *
 * @param HTTPAccessUsers_r [in]
 *  校验用户集数据结构。
 *
 * @return
 */
NK_API NK_Int
NK_HTTPServer_FreeAccessUsers(NK_HTTPAccessUsers **HTTPAccessUsers_r);


/**
 * @brief
 *  HTTP 服务器配置数据结构。
 */
typedef struct Nk_HTTPServerConfiguration {

	/**
	 * 最大内部附件缓冲，单位：字节，缺省为 1M。
	 */
	NK_Size maxAttachedCache;

	/**
	 * 资源可写标识，缺省为 False。
	 */
	NK_Boolean rcWritable;

	/**
	 * 使用 Keep-Alive 标识。
	 */
	NK_Boolean useKeepAlive;

	/**
	 * Keep-Alive 默认等待时长，\n
	 * 当 @ref NK_HTTPServerConfiguration::useKeepAlive 为 NK_True 此参数有效，\n
	 * 数值必须大于等于 @ref NK_HTTP_SERVER_DEF_KEEP_ALIVE 定义值。
	 */
	NK_Size maxKeepAlive;

} NK_HTTPServerConfiguration;


/**
 * @brief
 *  HTTP 服务器响应会话。
 * @details
 *
 */
typedef struct Nk_HTTPServerSession {

	/**
	 * 服务器使用到的内存分配器。
	 */
	NK_Allocator *Alloctr;

	/**
	 * Keep-Alive 剩余时间，单位：秒。
	 */
	NK_Size keepalive;

	/**
	 * 用户校验参数。
	 */
	NK_Char realm[NK_HTTP_SERVER_MAX_REALM_SZ], nonce[NK_HTTP_SERVER_MAX_NONCE_SZ];

	/**
	 * 当前 HTTP 会话校验方式。
	 */
	Nk_HTTPAccessAuth accessable;

	/**
	 * 当前 HTTP 会话校验用户。
	 */
	NK_HTTPAccessUsers *AccessUsers;

	struct {

		/**
		 * 请求头域报文原始内容。
		 */
		NK_Char headfield[NK_HTTP_HEAD_FIELD_SZ];

		/**
		 * 请求头域报文有效长度。
		 */
		NK_Size headfieldlen;

		/**
		 * 请求头域数据结构。
		 */
		NK_HTTPHeadField *HeadField;

		/**
		 * 请求数据内容。
		 */
		struct {

			/**
			 * 数据内容的大小，数据内容有可能保存在内存中，或在系统缓冲。
			 */
			NK_Size len;

			/**
			 * 数据内容在内存中的起始地址，但有请求数据内容不代表此值不为 Nil。\n
			 * 当接收到带数据的 HTTP 请求，如 PUT 和 POST 等，\n
			 * 服务器会尝试把报文中，如果报文长度小于用户配置 @ref NK_HTTPServerConfiguration::maxAttachedCache 并且\n
			 * @ref Alloctr 有足够的内存分配空间时会把数据保存到 @ref Alloctr 所分配的内存空间，\n
			 * @ref data 指向该内存空间的地址。
			 */
			NK_PVoid data;


		} Content;

	} Request;

	struct {

		/**
		 * 回复报文头域数据结构。
		 */
		NK_HTTPHeadField *HeadField;

		/**
		 * 回复数据内容。
		 */
		struct {

			/**
			 * 服务器回复数据区起始地址。\n
			 * @ref NK_HTTP_RESP_PUT_DATA() 等宏调用会影响该数值。
			 */
			NK_PVoid data;

			/**
			 * 服务器回复数据长度，及缓冲最大长度。\n
			 */
			NK_Size len, max;

			/**
			 * 服务器回复的文件，\n
			 * @ref NK_HTTP_RESP_PUT_FILE() 宏调用会影响该数值。
			 */
			NK_Char filepath[256];

		} Content;

	} Response;

} NK_HTTPServerSession;

/**
 * @macro
 *  设置 HTTP 回复码。
 */
#define NK_HTTP_RESP_SET_STATUS(__Session, __stat_code) \
	do {\
		if (!(__Session)->Response.HeadField) {\
			(__Session)->Response.HeadField = NK_HTTPUtils_CreateHeadField ((__Session)->Alloctr, "HTTP", 1, 1);\
			NK_HTTP_HEAD_FIELD_SERVER((__Session)->Response.HeadField, "nginx", 0, 0); \
			NK_HTTP_HEAD_FIELD_CONTENT_LENGTH((__Session)->Response.HeadField, 0); \
			(__Session)->Response.Content.data = NK_Nil;\
			(__Session)->Response.Content.len = 0;\
			(__Session)->Response.Content.max = 0;\
		}\
		if (NK_Nil != (__Session)->Response.HeadField) {\
			(__Session)->Response.HeadField->setResponse ((__Session)->Response.HeadField, (__stat_code), NK_Nil);\
		}\
	} while (0)

#define NK_HTTP_RESP_SET_HEAD(__Session, __opt, __optv) \
	do {\
		if (NK_Nil != (__Session)->Response.HeadField) {\
			(__Session)->Response.HeadField->addOption((__Session)->Response.HeadField, NK_True, (__opt), "%s", (__optv));\
		}\
	} while (0)

#define NK_HTTP_RESP_PUT_DATA(__Session, __data, __len) \
	do {\
		if (!(__Session)->Response.HeadField) {\
			NK_HTTP_RESP_SET_STATUS (__Session, NK_HTTP_CODE_OK);\
		}\
		if (NK_Nil != (__Session)->Response.Content.data) {\
			(__Session)->Alloctr->freep ((__Session)->Alloctr, (__Session)->Response.Content.data);\
			(__Session)->Response.Content.data = NK_Nil;\
		}\
		(__Session)->Response.Content.max = NK_ALIGN_BIG_END ((__len), 1024 * 4);\
		(__Session)->Response.Content.data = (__Session)->Alloctr->alloc ((__Session)->Alloctr, (__Session)->Response.Content.max);\
		if (NK_Nil != (__Session)->Response.Content.data) {\
			memcpy ((__Session)->Response.Content.data, (__data), (__len));\
			(__Session)->Response.Content.len = (__len);\
			NK_HTTP_HEAD_FIELD_CONTENT_LENGTH((__Session)->Response.HeadField, (__Session)->Response.Content.len);\
		} else {\
			(__Session)->Response.Content.max = 0;\
		}\
	} while (0)


#define NK_HTTP_RESP_ADD_DATA(__Session, __data, __len) \
	do {\
		if (!(__Session)->Response.Content.data) {\
			NK_HTTP_RESP_PUT_DATA(__Session, __data, __len);\
		} else {\
			if ((__len) + (__Session)->Response.Content.len > (__Session)->Response.Content.max) {\
				(__Session)->Response.Content.data = (__Session)->Alloctr->realloc ((__Session)->Alloctr, (__Session)->Response.Content.data, \
						NK_ALIGN_BIG_END (((__len) + (__Session)->Response.Content.len), 1024 * 4));\
			}\
			if (NK_Nil != (__Session)->Response.Content.data) {\
				(__Session)->Response.Content.max = (__len) + (__Session)->Response.Content.len;\
				memcpy ((__Session)->Response.Content.data + (__Session)->Response.Content.len, (__data), (__len));\
				(__Session)->Response.Content.len += (__len);\
				NK_HTTP_HEAD_FIELD_CONTENT_LENGTH((__Session)->Response.HeadField, (__Session)->Response.Content.len);\
			}\
		}\
	} while (0)

#define NK_HTTP_RESP_PUT_TEXT(__Session, __text) \
	do {\
		NK_HTTP_RESP_PUT_DATA(__Session, (__text), strlen(__text));\
	} while (0)


#define NK_HTTP_RESP_ADD_TEXT(__Session, __text) \
	do {\
		NK_HTTP_RESP_ADD_DATA(__Session, (__text), strlen(__text));\
	} while (0)


#define NK_HTTP_RESP_PUT_FILE(__Session, __filepath) \
	do {\
		FILE *fID = NK_Nil;\
		NK_Size filesz = 0;\
		if (NK_Nil != (__Session)->Response.Content.data) {\
			(__Session)->Alloctr->freep ((__Session)->Alloctr, (__Session)->Response.Content.data);\
			(__Session)->Response.Content.data = NK_Nil;\
		}\
		fID = fopen((__filepath), "rb");\
		if (!fID) {\
			NK_HTTP_RESP_SET_STATUS(__Session, NK_HTTP_CODE_NOT_FOUND);\
		} else {\
			NK_HTTP_RESP_SET_STATUS(__Session, NK_HTTP_CODE_OK);\
			fseek(fID, 0, SEEK_END); filesz = ftell(fID);\
			NK_HTTP_HEAD_FIELD_CONTENT_TYPE((__Session)->Response.HeadField, NK_HTTPUtils_FileMIME(__filepath));\
			NK_HTTP_HEAD_FIELD_CONTENT_LENGTH((__Session)->Response.HeadField, filesz);\
			if (filesz > 0) {\
				snprintf((__Session)->Response.Content.filepath, sizeof((__Session)->Response.Content.filepath), "%s", (__filepath));\
			}\
			fclose(fID);\
		}\
	} while (0)


/**
 * @brief
 *  当前会话作用户校验。
 * @brief
 *  根据用户传入的校验方式和用户数据，对当前会话进行校验，影响 @ref __Session 会话上下文的 HTTP 返回码以及 WWW-Authenticate 标签，\n
 *  当用户校验成功时，返回码会设置成 200，当用户校验失败时返回码会设置成 401，并根据用户校验需求设置 WWW-Authenticate 标签。
 *
 * @param __Session [in,out]
 *  会话上下问信息。
 *
 * @return
 *  用户校验通过返回 True，校验失败返回 False。
 */
NK_API NK_Boolean
NK_HTTPServer_Authorize(NK_HTTPServerSession *Session);

/**
 *
 * @param context
 * @return
 */
typedef NK_Void (*NK_HTTPServerOnCGI)(NK_PVoid ctx, NK_Thread *Thread, NK_SockTCP *ConnTCP, NK_HTTPServerSession *Session);


/**
 * @brief
 *  HTTPServer 模块句柄。
 * @details
 *  定义 HTTPServer 相关属性以及公开接口方法。
 *
 */
typedef struct Nk_HTTPServer {
#define NK_This struct Nk_HTTPServer *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

	/**
	 * @brief
	 *  获取服务器校验相关场景参数。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param dir
	 * @return
	 */
	NK_Int
	(*setRealm)(NK_This, NK_PChar realm, NK_PChar nonce);

	/**
	 * @brief
	 *  获取服务器校验相关场景参数。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param realm
	 * @param nonce
	 * @return
	 */
	NK_Int
	(*getRealm)(NK_This, NK_PChar realm, NK_PChar nonce);


	/**
	 * @brief
	 *  设置资源文件夹目录路径。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param dir [in]
	 *  目录路径。
	 *
	 * @return
	 *  设置成功返回 0，否则返回 -1。
	 */
	NK_Int
	(*setRcPath)(NK_This, NK_PChar path, Nk_HTTPAccessAuth accessable, NK_HTTPAccessUsers *AccessUsers);

	/**
	 * @brief
	 *  获取资源文件夹目录路径。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param
	 * @param path
	 * @return
	 */
	NK_Int
	(*getRcPath)(NK_This, NK_PChar path);


	/**
	 * @brief
	 *  加入 CGI 连接服务。
	 * @details
	 *  加入 @ref uri 对应路径下的 CGI 服务，\n
	 *  并通过 @ref access 和 @ref Users 配置其用户校验方式，\n
	 *  如果设置用该 CGI 路径需要进行校验，模块内部会根据 @ref access 和 @ref Users 的校验方式和用户信息进行校验，\n
	 *  校验失败会直接向对端返回 Unauthoried 结果，不会再出发 @ref OnCGI 事件，\n
	 *  如果用户希望自己控制 Unauthoried 结果返回，可以通过传入 @ref NK_HTTP_ACCESS_AUTH_NONE 不在模块内部进行用户校验，\n
	 *  在 @ref OnCGI 事件内通过调用 @ref NK_HTTPServer_Authorize() 方法进行用户校验并自定义回复。\n
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param uri [in]
	 *  当前 CGI 对应的 uri，如果该 uri 对应 CGI 已经存在则自动覆盖。
	 * @param access [in]
	 *  当前 CGI 会话使用校验方式。
	 * @param Users [in]
	 *  当前 CGI 会话校验用户集。
	 * @param OnEventLoop [in]
	 *  会话实现。
	 * @param ctx [in,out]
	 *  当前 CGI 会话用户上下文。
	 *
	 * @see Nk_HTTPAccessAuth
	 * @see NK_HTTPAccessUsers
	 * @see NK_HTTPServerOnCGI
	 *
	 * @return
	 *  添加成功返回 0，否则返回 -1。
	 */
	NK_Int
	(*addCGI)(NK_This, NK_PChar uri, Nk_HTTPAccessAuth access, NK_HTTPAccessUsers *Users, NK_HTTPServerOnCGI OnEventLoop, NK_PVoid ctx);

	/**
	 * @brief
	 *  移除 CGI 会话服务。
	 *
	 * param NK_This [in]
	 *  this 指针。
	 * @param uri
	 * @return
	 */
	NK_Int
	(*dropCGI)(NK_This, NK_PChar uri);

	/**
	 * @brief
	 *  启用事件监听。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param port
	 * @param backlog [in]
	 *  最大连接数，此值必须小于等于 @ref NK_HTTP_SERVER_MAX_CONNECTION 定义，\n
	 *  传入 0 时默认为 @ref NK_HTTP_SERVER_MAX_CONNECTION。
	 *
	 * @return
	 */
	NK_Int
	(*doEventLoop)(NK_This, NK_UInt16 port, NK_Size backlog);

	/**
	 * @brief
	 *  停止独立事件监听。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 * @param
	 * @return
	 */
	NK_Int
	(*finishEventLoop)(NK_This);


	/**
	 * @brief
	 *  连接 @ref NK_SPook2 事件监听。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 *
	 * @param SPook
	 * @return
	 */
	NK_Int
	(*attachSPook)(NK_This, NK_SPook2 *SPook, NK_PChar name);

	/**
	 * @brief
	 *  断开 @ref NK_SPook2 事件监听。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 *
	 * @return
	 */
	NK_Int
	(*detachSPook)(NK_This);


	/**
	 * @brief
	 *  获取服务器当前已连接数。
	 */
	NK_Size
	(*connection)(NK_This);


#undef NK_This
} NK_HTTPServer;


/**
 * @brief
 *  创建 HTTPServer 模块句柄。
 * @details
 *
 *
 * @param Alloctr [in]
 *  模块内部使用内存分配器。
 *
 * @return
 *  创建成功返回 HTTPServer 模块句柄，否则返回 Nil。
 */
NK_API NK_HTTPServer *
NK_HTTPServer_Create(NK_Allocator *Alloctr, NK_HTTPServerConfiguration *Configuration);

/**
 * @brief
 *  销毁 HTTPServer 模块句柄。
 * @details
 *  销毁成功后句柄将不能再使用，@ref HTTPServer_r 变量引用将置 Nil。
 *
 * @param HTTPServer_r [in]
 *  模块句柄引用。
 *
 * @return
 *  销毁成功返回 0，否则返回 -1。
 */
NK_API NK_Int
NK_HTTPServer_Free (NK_HTTPServer **HTTPServer_r);


/**
 * @brief
 *  生成 HTTP 头域 Date 标签内容。\n
 *
 * @param[in] utc
 *  UTC 时间。\n
 *
 * @param[out] stack
 *  数据栈区地址，返回数据内容。\n
 *
 * @param[in] stacklen
 *  数据栈区大小。\n
 *
 * @retval 日期时间字段
 *  成功。\n
 *
 * @retval Nil
 *  失败，可能是参数错误。\n
 *
 */
NK_API const NK_PChar
NK_HTTPServer_Date(NK_UTC1970 utc, NK_PChar stack, NK_Size stacklen);



NK_CPP_EXTERN_END
#endif /* NK_HTTP_SERVER_H_ */



