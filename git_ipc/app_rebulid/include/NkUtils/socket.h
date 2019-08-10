/**
 * 套接字句柄定义。
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#ifndef NK_UTILS_SOCKET_H_
#define NK_UTILS_SOCKET_H_
NK_CPP_EXTERN_BEGIN

/**
 * Mutex 模块句柄。
 */
typedef struct Nk_Socket
{
#define THIS struct Nk_Socket *const

	/**
	 * 模块基础类。
	 */
	NK_Object Object;


	/**
	 * By this method you may turn address reuse mode for local Bind.\n
	 * It is good specially for UDP protocol.\n
	 * Using this with TCP protocol is hazardous!
	 *
	 */
	NK_Int
	(*enableReuse)(THIS, NK_Boolean enabled);

	/**
	 * Try set timeout for all sending and receiving operations, if socket provider can do it.\n
	 * (It not supported by all socket providers!)
	 *
	 */
	NK_Int
	(*setTimeout)(THIS, NK_Size timeout);

	/**
	 * Try set timeout for all sending operations, if socket provider can do it.\n
	 * (It not supported by all socket providers!)
	 *
	 */
	NK_Int
	(*setSendTimeout)(THIS, NK_Size timeout);

	/**
	 * Try set timeout for all receiving operations, if socket provider can do it.\n
	 * (It not supported by all socket providers!)
	 */
	NK_Int
	(*setRecvTimeout)(THIS, NK_Size timeout);


	/**
	 * Connects socket to local IP address and PORT. IP address may be numeric ("192.168.74.50"). \n
	 * The same for PORT - it may be number port ('23').\n
	 * If port value is '0', system chooses itself and conects unused port in the range 1024 to 4096 (this depending by operating system!). \n
	 * Structure LocalSin is filled after calling this method.\n
	 * Warning: when you call : bind("0.0.0.0", 0); then is nothing done! In this case is used implicit system bind instead.
	 *
	 */
	NK_Int
	(*bind)(THIS, NK_PChar ip, NK_UInt16 port);

	/**
	 * Connects socket to remote IP address and PORT.\n
	 * The same rules as with Bind method are valid.\n
	 * The only exception is that PORT with 0 value will not be connected!\n
	 * Structures LocalSin and RemoteSin will be filled with valid values.\n
	 * When you call this on non-created socket, then socket is created automaticly.\n
	 * Type of created socket is by Family property.\n
	 * If is used SF_IP4, then is created socket for IPv4.\n
	 * If is used SF_IP6, then is created socket for IPv6.\n
	 * When you have family on SF_Any (default!), then type of created socket is determined by address resolving of destination address. (Not work properly on prilimitary winsock IPv6 support!)
	 *
	 */
	NK_Int
	(*connect)(THIS, NK_PChar ip, NK_UInt16 port);

	/**
	 * Connects socket to remote IP address and PORT.\n
	 * The same rules as with Bind method are valid.\n
	 * The only exception is that PORT with 0 value will not be connected!\n
	 * Structures LocalSin and RemoteSin will be filled with valid values.\n
	 * When you call this on non-created socket, then socket is created automaticly. \n
	 * Type of created socket is by Family property. If is used SF_IP4, then is created socket for IPv4. If is used SF_IP6, then is created socket for IPv6. When you have family on SF_Any (default!), then type of created socket is determined by address resolving of destination address. (Not work properly on prilimitary winsock IPv6 support!)\n
	 * Public procedure Listen; virtual;
	 *
	 *
	 */
	NK_Int
	(*listen)(THIS);



	struct Nk_Socket *
	(*accept)(THIS);



	NK_SSize
	(*sendBufferTo)(THIS, NK_PByte buffer, NK_Size len, NK_PChar remoteIP, NK_UInt16 remotePort);

	NK_SSize
	(*recvBufferFrom)(THIS, NK_PByte buffer, NK_Size len, NK_PChar remoteIP, NK_UInt16 *remotePort);


#undef THIS
} NK_Socket;




/**
 * SockUDP 模块句柄。
 */
typedef struct Nk_SockUDP
{
#define THIS struct Nk_SockUDP *const

	/**
	 * 套接字模块基础类。
	 */
	NK_Socket Socket;

	/**
	 * Enable or disable sending of broadcasts.\n
	 * If seting OK, result is 0.\n
	 * This method is not supported in SOCKS5 mode!\n
	 * IPv6 does not support broadcasts! In this case you must use Multicasts instead.
	 *
	 */
	NK_Int
	(*enableBroadcast)(THIS, NK_Boolean enabled);

	/**
	 * 套接字加入一个既定的多播组。
	 *
	 * @param[in]			multicastIP				多播组地址。
	 *
	 */
	NK_Int
	(*addMulticast)(THIS, NK_PChar multicastIP);

	/**
	 * 退出多播组。
	 *
	 * @param[in]			multicastIP				多播组地址。
	 *
	 */
	NK_Int
	(*dropMulticast)(THIS, NK_PChar multicastIP);

	/**
	 * All sended multicast datagrams is loopbacked to your interface too. (you can read your sended datas.)\n
	 * You can disable this feature by this function.\n
	 * This function not working on some Windows systems!
	 *
	 */
	NK_Int
	(*enableMulticastLoop)(THIS, NK_Boolean enabled);

	/**
	 * Get Time-to-live value for multicasts packets.\n
	 * It define number of routers for transfer of datas.\n
	 * If you set this to 1 (dafault system value), then multicasts packet goes only to you local network.\n
	 * If you need transport multicast packet to worldwide,\n
	 * then increase this value, but be carefull,\n
	 * lot of routers on internet does not transport multicasts packets!\n
	 *
	 */
	NK_Int
	(*getMulticastTTL)(THIS, NK_Size *ttl);

	/**
	 * Set Time-to-live value for multicasts packets.
	 *
	 */
	NK_Int
	(*setMulticastTTL)(THIS, NK_Size ttl);


#undef THIS
} NK_SockUDP;




NK_CPP_EXTERN_END
#endif /* NK_UTILS_SOCKET_H_ */

