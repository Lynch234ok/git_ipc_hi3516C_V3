


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include "socket_tcp.h"

#ifndef TCP_LISTENER_H_
#define TCP_LISTENER_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef int (*fTCP_LISTENER_ACCEPT_HOCK)(int accpet_sock);

extern int tcp_listener_start(void *const listener, bool background, fTCP_LISTENER_ACCEPT_HOCK acceptHock);
extern int tcp_listener_stop(void *const listener);
extern bool tcp_listener_is_on(void *const t);

extern void *tcp_listener_create(in_port_t listen_port);
extern void tcp_listener_release(void *const listener);

#ifdef __cplusplus
};
#endif
#endif //TCP_LISTENER_H_

