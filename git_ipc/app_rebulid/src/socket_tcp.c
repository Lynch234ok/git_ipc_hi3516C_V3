
#include "socket_tcp.h"
#include <linux/tcp.h>

#include <assert.h>

static int tcp_close(struct SOCKET_TCP *const tcp)
{
	int ret = close(tcp->sock);
	if(0 == ret){
		tcp->sock = -1;
	}
	return ret;
}

static int tcp_connect(struct SOCKET_TCP *const tcp, const char *serv_addr, in_port_t serv_port)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serv_port);
	addr.sin_addr.s_addr = inet_addr(serv_addr);
	return connect(tcp->sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
}

static int tcp_bind(struct SOCKET_TCP *const tcp, const char *in_addr, in_port_t in_port)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(!in_addr ? INADDR_ANY : inet_addr(in_addr));
	addr.sin_port = htons(in_port);
	return bind(tcp->sock, (struct sockaddr *) &addr, sizeof(addr));
}

static int tcp_listen(struct SOCKET_TCP *const tcp, int backlog)
{
	return listen(tcp->sock, backlog);
}

static int tcp_select(struct SOCKET_TCP *const tcp, int *flags, time_t timeo_s, suseconds_t timeo_us)
{
	int ret = -1;
	if(NULL != flags && *flags > 0){
		struct timeval timeo;
		fd_set read_fds, write_fds, exep_fds;
		fd_set *pread_fds = NULL, *pwrite_fds = NULL, *pexep_fds = NULL;

		timeo.tv_sec = timeo_s;
		timeo.tv_usec = timeo_us;

		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		FD_ZERO(&exep_fds);
		if(*flags & kT_SELECT_READ){
			FD_SET(tcp->sock, &read_fds);
			pread_fds = &read_fds;
		}
		if(*flags & kT_SELECT_WRITE){
			FD_SET(tcp->sock, &write_fds);
			pwrite_fds = &write_fds;
		}
		if(*flags & kT_SELECT_EXCPT){
			FD_SET(tcp->sock, &exep_fds);
			pexep_fds = &exep_fds;
		}
		ret = select(tcp->sock + 1, pread_fds, pwrite_fds, pexep_fds, &timeo);
		if(ret > 0){
			// clear flag
			*flags = 0;
			if(NULL != pread_fds && FD_ISSET(tcp->sock, pread_fds)){
				*flags |= kT_SELECT_READ;
			}
			if(NULL != pwrite_fds && FD_ISSET(tcp->sock, pwrite_fds)){
				*flags |= kT_SELECT_WRITE;
			}
			if(NULL != pexep_fds && FD_ISSET(tcp->sock, pexep_fds)){
				*flags |= kT_SELECT_EXCPT;
			}
		}
	}
	return ret;
}

static int tcp_accept(struct SOCKET_TCP *const tcp, struct sockaddr *addr, socklen_t *addrlen)
{
	return accept(tcp->sock, addr, addrlen);
}

static int tcp_send(struct SOCKET_TCP *const tcp, const void *msg, size_t len, int flags)
{
	flags |= MSG_NOSIGNAL;
	return send(tcp->sock, msg, len, flags);
}

static int tcp_send2(struct SOCKET_TCP *const tcp, const void *msg, size_t len, int flags)
{
	int left_len = len;
	int send_len = 0;
	const void *p = msg; // from beginning
	time_t begin_time = time(NULL);
	time_t curr_time = begin_time;

	while(left_len > 0){
		if(curr_time - begin_time > tcp->recv_timeout_s)
		{
			printf("%s ERROR: send failed send_len:%d\n", __FUNCTION__, left_len);
			return -1;
		}
		send_len = tcp->send(tcp, p, left_len, flags);
		curr_time = time(NULL);
		if(send_len < 0){
			if(EINTR == errno || EAGAIN == errno){
				continue;
			}
			printf("%s ERROR: send failed errno:%d\n", __FUNCTION__, errno);
			return -1;
		} 
		// proc the left
		left_len -= send_len;
		p += send_len;
		if(left_len > 0)
			usleep(1000);
			//printf("resend: %d:%d\r\n", len, len - left_len);
	}
	return len - left_len;
}

static ssize_t tcp_recv(struct SOCKET_TCP *const tcp, void *buf, size_t len, int flags)
{
	flags |= MSG_NOSIGNAL;
	return recv(tcp->sock, buf, len, flags);
}

static ssize_t tcp_recv2(struct SOCKET_TCP *const tcp, void *buf, size_t size, int flags)
{
	int ret=0;
	int recv_len = 0;
	void *pbuf = buf; // the head of buffer
	time_t begin_time = time(NULL);
	time_t curr_time = begin_time;
	while(recv_len < size)	{
		if(curr_time - begin_time > tcp->recv_timeout_s)
		{
			printf("%s ERROR: recv failed recv_len:%d\n", __FUNCTION__, recv_len);
			return -1;
		}
		ret = tcp->recv(tcp, pbuf,size - recv_len, flags);
		curr_time = time(NULL);	
		if(ret <= 0){
			return -1;
		}else{
			pbuf += ret;
			recv_len += ret;
		}
	}
	return recv_len;
}

static ssize_t tcp_peek(struct SOCKET_TCP *const tcp, void *buf, size_t len)
{
	return tcp->recv(tcp, buf, len, MSG_PEEK);
}

static int tcp_send_timeout(struct SOCKET_TCP *const tcp, const void *msg, size_t len, time_t timeout_s)
{
	int ret = 0;
	size_t send_len = 0;
	int send_errno = 0;
	time_t const base_time = time(NULL);

	while(send_len < len){
		if(time(NULL) - base_time > timeout_s){
			// send timeout
			// only send a part of buffer
			break;
		}

		ret = tcp->send2(tcp, msg + send_len, len - send_len, MSG_DONTWAIT);
		send_errno = errno;
		if(ret < 0){
			if(EAGAIN == send_errno || EINTR == send_errno){
				// need to retry
				continue;
			}
			return ret;
		}
		send_len += ret;
	}
	return send_len;
}


static int tcp_getsockname(struct SOCKET_TCP *const tcp, struct sockaddr *name, socklen_t *namelen)
{
	return getsockname(tcp->sock, name, namelen);
}

static int tcp_getpeername(struct SOCKET_TCP *const tcp, struct sockaddr *name, socklen_t *namelen)
{
	return getpeername(tcp->sock, name, namelen);
}

static int tcp_set_send_timeout(struct SOCKET_TCP *const tcp, time_t timeout_s, suseconds_t timeout_us)
{
	struct timeval tv = { .tv_sec = timeout_s, .tv_usec = timeout_us, };
	tcp->send_timeout_s = timeout_s;//fix me
	return setsockopt(tcp->sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

static int tcp_set_recv_timeout(struct SOCKET_TCP *const tcp, time_t timeout_s, suseconds_t timeout_us)
{
	struct timeval tv = { .tv_sec = timeout_s, .tv_usec = timeout_us, };
	tcp->recv_timeout_s = timeout_s;
	return setsockopt(tcp->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

static int tcp_set_reuser_addr(struct SOCKET_TCP *const tcp, bool flag)
{
	int reuse_on = flag ? 1 : 0;
	return setsockopt(tcp->sock, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(reuse_on));
}

static int tcp_set_nodelay(struct SOCKET_TCP *const tcp, bool flag)
{
	int nodelay_on = flag ? 1 : 0;
	return setsockopt(tcp->sock, IPPROTO_TCP, TCP_NODELAY, &nodelay_on, sizeof(nodelay_on));
}


static lpSOCKET_TCP tcp_create_r(int sock, lpSOCKET_TCP result)
{
	
	// create a system socket
	result->sock = sock;
	result->recv_timeout_s = 5;//set default time
	result->send_timeout_s = 5;
	
	// install the interfaces
	result->close = tcp_close;
	result->connect = tcp_connect;
	result->bind = tcp_bind;
	result->listen = tcp_listen;
	result->select = tcp_select;
	result->accept = tcp_accept;
	result->send = tcp_send;
	result->send2 = tcp_send2;
	result->send_timeout = tcp_send_timeout;
	result->recv = tcp_recv;
	result->recv2 = tcp_recv2;
	result->peek = tcp_peek;
	result->getsockname = tcp_getsockname;
	result->getpeername = tcp_getpeername;
	
	// sock option
	result->set_send_timeout = tcp_set_send_timeout;
	result->set_recv_timeout = tcp_set_recv_timeout;
	result->set_reuser_addr = tcp_set_reuser_addr;
	result->set_nodelay = tcp_set_nodelay;

	// default to set reuse
	result->set_reuser_addr(result, true);
	
	// success
	return result;
}

lpSOCKET_TCP socket_tcp_r(lpSOCKET_TCP result)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	return tcp_create_r(sock, result);
}

lpSOCKET_TCP socket_tcp2_r(int sock, lpSOCKET_TCP result)
{
	return tcp_create_r(sock, result);
}


