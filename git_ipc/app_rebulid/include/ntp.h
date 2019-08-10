
#ifndef __NTP_H__
#define __NTP_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

extern int NTP_start(const char* server_domain, const char* server_backup_one, const char* server_backup_two, const char* server_ip, int timeout, int minutesWest);

#endif //__NTP_H__

