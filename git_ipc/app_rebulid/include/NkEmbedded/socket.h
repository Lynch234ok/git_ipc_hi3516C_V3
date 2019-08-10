/**
 * 套接字句柄定义。
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#if !defined(NK_EMB_SOCKET_H_)
#define NK_EMB_SOCKET_H_
NK_CPP_EXTERN_BEGIN

#ifdef _WIN32
#include <WinSock2.h>
#define NK_SOCKET SOCKET
#else
#define NK_SOCKET NK_Int
#endif // _WIN32

/**
 * Socket 模块句柄。
 */
typedef struct Nk_Socket {
#define NK_This struct Nk_Socket *const

	/**
	 * 模块基础类。
	 */
	NK_Object Object;

	/**
	 * Close NK_This socket.
	 */
	NK_Int
	(*close)(NK_This);

	/**
	 * By NK_This method you may turn address reuse mode for local Bind.\n
	 * It is good specially for UDP protocol.\n
	 * Using NK_This with TCP protocol is hazardous!
	 *
	 */
	NK_Int
	(*enableReuse)(NK_This, NK_Boolean enabled);

	/**
	 * Try set microsecond timeout for all sending and receiving operations, if socket provider can do it.\n
	 * (It not supported by all socket providers!)
	 *
	 */
	NK_Int
	(*setTimeout)(NK_This, NK_Size us);

	/**
	 * Try set microsecond timeout for all sending operations, if socket provider can do it.\n
	 * (It not supported by all socket providers!)
	 *
	 */
	NK_Int
	(*setSendTimeout)(NK_This, NK_Size us);


	/**
	 * Try get microsecond timeout for all sending operations, if socket provider can do it.\n
	 * (It not supported by all socket providers!)
	 *
	 */
	NK_Int
	(*getSendTimeout)(NK_This, NK_Size *us);


	/**
	 * Try set microsecond timeout for all receiving operations, if socket provider can do it.\n
	 * (It not supported by all socket providers!)
	 */
	NK_Int
	(*setRecvTimeout)(NK_This, NK_Size us);
	
	/**
	 * Try set microsecond timeout for all receiving operations, if socket provider can do it.\n
	 * (It not supported by all socket providers!)
	 */
	NK_Int
	(*getRecvTimeout)(NK_This, NK_Size *us);

	/**
	 * Connects socket to local IP address and PORT. IP address may be numeric ("192.168.74.50"). \n
	 * The same for PORT - it may be number port ('23').\n
	 * If port value is '0', system chooses itself and conects unused port in the range 1024 to 4096 (NK_This depending by operating system!). \n
	 * Structure LocalSin is filled after calling NK_This method.\n
	 * Warning: when you call : bind("0.0.0.0", 0); then is nothing done! In NK_This case is used implicit system bind instead.
	 *
	 */
	NK_Int
	(*bind)(NK_This, NK_PChar ip, NK_UInt16 port);

	/**
	 * Connects socket to remote IP address and PORT.\n
	 * The same rules as with Bind method are valid.\n
	 * The only exception is that PORT with 0 value will not be connected!\n
	 * Structures LocalSin and RemoteSin will be filled with valid values.\n
	 * When you call NK_This on non-created socket, then socket is created automaticly.\n
	 * Type of created socket is by Family property.\n
	 * If is used SF_IP4, then is created socket for IPv4.\n
	 * If is used SF_IP6, then is created socket for IPv6.\n
	 * When you have family on SF_Any (default!), then type of created socket is determined by address resolving of destination address. (Not work properly on prilimitary winsock IPv6 support!)
	 *
	 */
	NK_Int
	(*connect)(NK_This, NK_PChar ip, NK_UInt16 port);

	/**
	 * Connects socket to remote IP address and PORT in US timeout.\n
	 * The same rules as with Bind method are valid.\n
	 * The only exception is that PORT with 0 value will not be connected!\n
	 * Structures LocalSin and RemoteSin will be filled with valid values.\n
	 * When you call NK_This on non-created socket, then socket is created automaticly.\n
	 * Type of created socket is by Family property.\n
	 * If is used SF_IP4, then is created socket for IPv4.\n
	 * If is used SF_IP6, then is created socket for IPv6.\n
	 * When you have family on SF_Any (default!), then type of created socket is determined by address resolving of destination address. (Not work properly on prilimitary winsock IPv6 support!)
	 *
	 */
	NK_Int
	(*connectEx)(NK_This, NK_PChar ip, NK_UInt16 port, NK_Size us);

	/**
	 * Connects socket to remote IP address and PORT.\n
	 * The same rules as with Bind method are valid.\n
	 * The only exception is that PORT with 0 value will not be connected!\n
	 * Structures LocalSin and RemoteSin will be filled with valid values.\n
	 * When you call NK_This on non-created socket, then socket is created automaticly. \n
	 * Type of created socket is by Family property. If is used SF_IP4, then is created socket for IPv4. If is used SF_IP6, then is created socket for IPv6. When you have family on SF_Any (default!), then type of created socket is determined by address resolving of destination address. (Not work properly on prilimitary winsock IPv6 support!)\n
	 * Public procedure Listen; virtual;
	 *
	 */
	NK_Int
	(*listen)(NK_This, NK_Size backlog);

	/**
	 * Waits until new incoming connection comes.\n
	 * After it comes a new socket is automatically created (socket handler is returned by NK_This function as result).
	 *
	 */
	struct Nk_Socket *
	(*accept)(NK_This, NK_PChar peek_ip, NK_UInt16 *peer_port);

	/**
	 * Sends data of @ref length from @ref buffer address via connected socket.\n
	 * System automatically splits data to packets.
	 *
	 */
	NK_SSize
	(*sendBuffer)(NK_This, NK_PVoid buffer, NK_Size length);

	/**
	 * Sends Message of its own length from @ref msg.
	 */
	NK_SSize
	(*sendMsg)(NK_This, NK_PChar msg);


	/**
	 * @brief
	 *  在套接字超时之前尽量发送指定长度 @ref length 的数据。
	 * @details
	 *
	 * @param buffer [in]
	 *  发送数据内存所在地址。
	 * @param length [in]
	 *  发送内存长度。
	 *
	 * @return
	 *  已发送数据的长度，这里等于 @ref length，失败返回 -1。
	 *
	 */
	NK_SSize
	(*sendBufferEx)(NK_This, NK_PVoid buffer, NK_Size length);

	/**
	 * Sends Message of its own length from @ref msg.
	 */
	NK_SSize
	(*sendMsgEx)(NK_This, NK_PChar msg);

    /**
     * writeBuffers
     *
     * @param iovec
     * @param count
     * @return
     */
    NK_SSize
    (*sendChunks)(NK_This, NK_Size count, NK_PByte *chunks, NK_Size *lens);

	/**
	 * Note: NK_This is low-level receive function.\n
	 * You must be sure if data is waiting for read before call NK_This function for avoid deadlock!\n
	 * Waits until allocated buffer is filled by received data.\n
	 * Returns number of data received, which equals to LENGTH value under normal operation.\n
	 * If it is not equal the communication channel is possibly broken.\n
	 * On stream oriented sockets if is received 0 bytes, it mean 'socket is closed!"\n
	 * On datagram socket is readed first waiting datagram.\n
	 *
	 */
	NK_SSize
	(*recvBuffer)(NK_This, NK_PVoid buffer, NK_Size length);

	/**
	 *
	 */
	NK_SSize
	(*recvBufferEx)(NK_This, NK_PVoid buffer, NK_Size length);

	/**
	 * Same as RecvBuffer, but readed data stays in system input buffer.\n
	 * Warning: NK_This function not respect data in LineBuffer!\n
	 * Is not recommended to use NK_This function!
	 *
	 */
	NK_SSize
	(*peekBuffer)(NK_This, NK_PVoid buffer, NK_Size length);

	/**
	 * Return True, if you can read any data from socket or is incoming connection on TCP based socket.\n
	 * Status is tested for time Timeout (in milliseconds).\n
	 * If value in Timeout is 0, status is only tested and continue.\n
	 * If value in Timeout is -1, run is breaked and waiting for read data maybe forever.\n
	 * NK_This function is need only on special cases, when you need use RecvBuffer function directly!\n
	 * read functioms what have timeout as calling parameter, calling NK_This function internally.
	 *
	 */
	NK_Boolean
	(*canRead)(NK_This, NK_Int us);

	/**
	 * Return True, if you can to socket write any data (not full sending buffer).\n
	 * Status is tested for time Timeout (in milliseconds).\n
	 * If value in Timeout is 0, status is only tested and continue.\n
	 * If value in Timeout is -1, run is breaked and waiting for write data maybe forever.
	 *
	 */
	NK_Boolean
	(*canWrite)(NK_This, NK_Int us);

	/**
	 * Sends data of LENGTH from BUFFER address via connected socket.\n
	 * System automatically splits data to packets.
	 *
	 */
	NK_SSize
	(*sendBufferTo)(NK_This, NK_PByte buffer, NK_Size len, NK_PChar remoteIP, NK_UInt16 remotePort);

	/**
	 * Send content of stream to socket.\n
	 * It using SendBlock method and NK_This is compatible with streams in Indy library.
	 *
	 */
	NK_SSize
	(*recvBufferFrom)(NK_This, NK_PByte buffer, NK_Size len, NK_PChar remoteIP, NK_UInt16 *remotePort);

	/**
	 * Get Size of Socket receive buffer.\n
	 * If it is not supported by socket provider, it return as size one kilobyte.
	 *
	 */
	NK_Int
	(*getRecvBufferSize)(NK_This, NK_Int *ttl);

	/**
	 * Set Size of Socket receive buffer.\n
	 * If it is not supported by socket provider, it return as size one kilobyte.
	 *
	 */
	NK_Int
	(*setRecvBufferSize)(NK_This, NK_Int ttl);

	/**
	 * Get Size of Socket send buffer.\n
	 * If it is not supported by socket provider, it return as size one kilobyte.
	 *
	 */
	NK_Int
	(*getSendBufferSize)(NK_This, NK_Size *size);

	/**
	 * Set Size of Socket send buffer.\n
	 * If it is not supported by socket provider, it return as size one kilobyte.
	 *
	 */
	NK_Int
	(*setSendBufferSize)(NK_This, NK_Size size);

	/**
	 * Get Time-to-live value. (if system supporting it!)
	 *
	 */
	NK_Int
	(*getTTL)(NK_This, NK_Size *ttl);

	/**
	 * Set Time-to-live value. (if system supporting it!)
	 *
	 */
	NK_Int
	(*setTTL)(NK_This, NK_Size ttl);

	/**
	 *  Get Local Address.
	 */
	NK_Int
	(*localAddr)(NK_This, NK_PChar ipaddr, NK_UInt16 *port);

	/**
	 * Get Remove Address.
	 */
	NK_Int
	(*remoteAddr)(NK_This, NK_PChar ipaddr, NK_UInt16 *port);



#undef NK_This
} NK_Socket;


/**
 * SockTCP 模块句柄。
 */
typedef struct Nk_SockTCP {
#define NK_This struct Nk_SockTCP *const

	/**
	 * 套接字模块基础类。
	 */
	NK_Socket Socket;


	/**
	 * Set TCP Nodelay
	 *
	 * @param enabled
	 * @return -1 failed, 0 succeed.
	 */
	NK_Int
	(*enableNoDelay)(NK_This, NK_Boolean enabled);


	/**
	 * @brief
	 *  获取当前 TCP 状态。
	 *
	 * @param NK_This [in]
	 *  this 指针。
	 *
	 * @return
	 *  根据当前 TCP 句柄状态返回对应的状态字段，错误返回 ""；
	 *
	 * @retval "ESTABLISHED"
	 * @retval "SYN_SENT"
	 * @retval "SYN_RECV"
	 * @retval "FIN_WAIT1"
	 * @retval "FIN_WAIT2"
	 * @retval "TIME_WAIT"
	 * @retval "CLOSE"
	 * @retval "CLOSE_WAIT"
	 * @retval "LAST_ACK"
	 * @retval "LISTEN"
	 * @retval "CLOSING"
	 */
	NK_PChar
	(*state)(NK_This);



#undef NK_This
} NK_SockTCP;


/**
 * SockUDP 模块句柄。
 */
typedef struct Nk_SockUDP {
#define NK_This struct Nk_SockUDP *const

	/**
	 * 套接字模块基础类。
	 */
	NK_Socket Socket;

	/**
	 * Enable or disable sending of broadcasts.\n
	 * If seting OK, result is 0.\n
	 * NK_This method is not supported in SOCKS5 mode!\n
	 * IPv6 does not support broadcasts! In NK_This case you must use Multicasts instead.
	 *
	 */
	NK_Int
	(*enableBroadcast)(NK_This, NK_Boolean enabled);

	/**
	 * 套接字加入一个既定的多播组。
	 *
	 * @param[in]			multicastIP				多播组地址。
	 *
	 */
	NK_Int
	(*addMulticast)(NK_This, NK_PChar multicastIP, NK_PChar ifrIP);

	/**
	 * 退出多播组。
	 *
	 * @param[in]			multicastIP				多播组地址。
	 *
	 */
	NK_Int
	(*dropMulticast)(NK_This, NK_PChar multicastIP, NK_PChar ifrIP);

	/**
	 * All sended multicast datagrams is loopbacked to your interface too. (you can read your sended datas.)\n
	 * You can disable NK_This feature by NK_This function.\n
	 * NK_This function not working on some Windows systems!
	 *
	 */
	NK_Int
	(*enableMulticastLoop)(NK_This, NK_Boolean enabled);

	/**
	 * Get Time-to-live value for multicasts packets.\n
	 * It define number of routers for transfer of datas.\n
	 * If you set NK_This to 1 (dafault system value), then multicasts packet goes only to you local network.\n
	 * If you need transport multicast packet to worldwide,\n
	 * then increase NK_This value, but be carefull,\n
	 * lot of routers on internet does not transport multicasts packets!\n
	 *
	 */
	NK_Int
	(*getMulticastTTL)(NK_This, NK_Size *ttl);

	/**
	 * Set Time-to-live value for multicasts packets.
	 *
	 */
	NK_Int
	(*setMulticastTTL)(NK_This, NK_Size ttl);


#undef NK_This
} NK_SockUDP;


/**
 * 创建 TCP 套接字。
 */
NK_API NK_SockTCP *
NK_Socket_CreateTCP(NK_Allocator *Alloctr);


/**
 * 创建一个 UDP 句柄。
 *
 */
NK_API NK_SockUDP *
NK_Socket_CreateUDP(NK_Allocator *Alloctr);


/**
 * 释放套接字句柄。
 *
 */
NK_API NK_Int
NK_Socket_Free(NK_Socket **SockUDP_r);



/**
 * @brief
 *  接管操作系统 UDP 套接字。
 *
 * @details
 *  接管套接字以后通过 @ref NK_SockTCP 句柄进行句柄操作，\n
 *  句柄释放时会对套接字进行关闭操作，\n
 *  如果外部逻辑需要在对套接字进行关闭，可以通过 dup 方法复制一个套接字传入此方法接管。
 *
 * @param Alloctr [in]
 *  内存分配器。
 * @param s [in]
 *  套接字描述符。
 *
 * @return
 *  创建成功返回 @ref NK_SockUDP 句柄，否则返回 Nil。
 */
NK_API NK_SockUDP *
NK_Socket_TakeUDP(NK_Allocator *Alloctr, NK_SOCKET  s);

/**
 * @brief
 *  见 @ref NK_SockUDP_Take() 方法。
 *
 */
NK_API NK_SockTCP *
NK_Socket_TakeTCP(NK_Allocator *Alloctr, NK_SOCKET s);


NK_CPP_EXTERN_END
#endif /* NK_EMB_SOCKET_H_ */

