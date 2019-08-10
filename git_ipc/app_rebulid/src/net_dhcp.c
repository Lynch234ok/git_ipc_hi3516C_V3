#include "net_dhcp.h"
#include "arp.h"
#include "app_debug.h"
#include "base/ja_process.h"
#include <sys/prctl.h>
#include "base/cross_thread.h"
#include "global_runtime.h"

typedef struct device_ip
{
	char ip[16];
}dev_ip_t;

static void *arp_check_ip_proc(void *param)
{
	char *sz_query_ip = (char *)param;
	char query_mac[32][18];
	int *ret;
	prctl(PR_SET_NAME, "arp_check_ip_proc");
	ret = (int *)malloc(sizeof(int));
	//*ret= ARP_query(sz_query_ip, query_mac);
	*ret= ARP_send_request(sz_query_ip, query_mac, 32, 2);
	if (*ret < 0) {
		APP_TRACE("arp query ip: %s failed", sz_query_ip);
	} else if (*ret == 0) {
		APP_TRACE("arp query ip: %s not found", sz_query_ip);
	} else {
		APP_TRACE("arp query ip: %s found : %d(%s)", sz_query_ip, *ret, query_mac);
	}
	pthread_exit(ret);
}

static void multi_work_start(int nworks, int *result, pthread_t *pid, void *(*thread_proc)(void *), void **param_list)
{
#define MAX_WORK			(128)
#define DEFAULT_WORK_NUM	(32)
	int i;

	if (nworks == 0)
		nworks = DEFAULT_WORK_NUM;
	else if (nworks < 0 || nworks > MAX_WORK) {
		nworks = MAX_WORK;
	}

	for ( i  = 0; i < nworks; i++) {
		if (JA_THREAD_init0(&pid[i], thread_proc, param_list[i], NULL, 0, NULL, 131072, 0) == 0) {
			result[i] = 0;
		} else {
			result[i] = -1;
		}
	}
}

static void multi_work_stop(int nworks, int *result, pthread_t *pid)
{
	int i;
	int *ret;
	for ( i  = 0; i < nworks; i++) {
		if (result[i] == 0) {
			if (pthread_join(pid[i], &ret) == 0) {
				result[i] = *ret;
				free(ret);
			} else {
				result[i] = -1;
			}
		}
	}
}

int NET_check_ip_conflict(char *ip)
{
	int result = 1;
	pthread_t pid = 0;
	int *ret;
	if(ip){
		if(JA_THREAD_init0(&pid, arp_check_ip_proc, ip, NULL, 0, NULL, 131072, 0) == 0){
			if (pthread_join(pid, &ret) == 0){
				result = *ret;
				free(ret);
			}
		}
	}
	return result;
}

int NET_find_avai_ip(char *sz_l_ip, char *sz_l_netmask, char *ip_start, char *ip_end, 
	int need_n, int multi_tasks, int timeout_s, f_ip_found hook)
{
#define NUM_IP_CHECK_TRHEAD		(16)
#define IP_FOUND_DEF_TIMEOUT		(2)
#define DEF_SEARCH_TIMES			(5)
	int n_found = 0;
	int n_threads;
	int irevert=0;
	bool trigger = true;
	unsigned char query_ip[4];
	unsigned char local_ip[4];
	unsigned char u_start[4], u_end[4];
	unsigned char network[4], net_tmp[4];
	unsigned int i_network, i_mask, i_queryip, i_start, i_end;
	unsigned char netmask[4];
	char sz_query_ip[18];
	char query_mac[128][18];
	int index;
	time_t __tt;
	int i;
	pthread_t check_ip_pids[NUM_IP_CHECK_TRHEAD];
	int check_ip_result[NUM_IP_CHECK_TRHEAD];
	char *check_ip_args[NUM_IP_CHECK_TRHEAD];

	//pthread_detach(pthread_self());

	if (multi_tasks > 0 && multi_tasks < NUM_IP_CHECK_TRHEAD) {
		n_threads =  multi_tasks;
	} else 
		n_threads = NUM_IP_CHECK_TRHEAD;

	if (timeout_s == 0) 
		timeout_s = (need_n + multi_tasks - 1)/multi_tasks * DEF_SEARCH_TIMES;
	else if (timeout_s <= IP_FOUND_DEF_TIMEOUT) {
		timeout_s = IP_FOUND_DEF_TIMEOUT;
	}

	memset(check_ip_args, 0, sizeof(check_ip_args));
	ipstr2uint8(local_ip, sz_l_ip);
	ipstr2uint8(netmask, sz_l_netmask);

	network[0] = local_ip[0] & netmask[0];
	network[1] = local_ip[1] & netmask[1];
	network[2] = local_ip[2] & netmask[2];
	network[3] = local_ip[3] & netmask[3];
	i_network =  (network[0] << 24) | (network[1] << 16) | (network[2] << 8) | network[3];
	i_mask =  (netmask[0] << 24) | (netmask[1] << 16) | (netmask[2] << 8) | netmask[3];
	if (ip_start && ip_end) {
		ipstr2uint8(u_start, ip_start);
		ipstr2uint8(u_end, ip_end);
		i_start =  (u_start[0] << 24) | (u_start[1] << 16) | (u_start[2] << 8) | u_start[3];
		i_end =  (u_end[0] << 24) | (u_end[1] << 16) | (u_end[2] << 8) | u_end[3];
		net_tmp[0] = u_start[0] & netmask[0];
		net_tmp[1] = u_start[1] & netmask[1];
		net_tmp[2] = u_start[2] & netmask[2];
		net_tmp[3] = u_start[3] & netmask[3];
		if (i_network != ((net_tmp[0] << 24) | (net_tmp[1] << 16) | (net_tmp[2] << 8) | net_tmp[3])) {
			printf("invalid start ip: %s\n", ip_start);
			return -1;
		}
		net_tmp[0] = u_end[0] & netmask[0];
		net_tmp[1] = u_end[1] & netmask[1];
		net_tmp[2] = u_end[2] & netmask[2];
		net_tmp[3] = u_end[3] & netmask[3];
		if (i_network != ((net_tmp[0] << 24) | (net_tmp[1] << 16) | (net_tmp[2] << 8) | net_tmp[3])) {
			printf("invalid end ip: %s\n", ip_end);
			return -1;
		}
		if (!((i_start < i_end) && (i_start > i_network) && (i_end > i_network))) {
			printf("ERROR: start ip(%s) must < end ip(%s)\n", ip_start, ip_end);
			return -1;
		}
		i_queryip = i_start;
	}else {
		struct timespec timetic;
		unsigned char random;
		do{
			clock_gettime(CLOCK_MONOTONIC, &timetic);
			srand((unsigned) timetic.tv_nsec); 
			random = rand();
		}while(random <=2 || random > 254);
		printf("random = %d", random);
		i_start = (i_network & 0xffffff00)|random;
		i_end = i_network & 0xffffffff;
		i_queryip = i_start;
		/*i_start = i_network + 15;
		i_end = i_network & 0xffffffff;
		i_queryip = i_network + 15;*/
	}

	time(&__tt);
	index = 0;
	trigger = true;
	while(trigger)
	{
		char *thread_arg = NULL;
		int ret;
		dev_ip_t item;
		
		query_ip[0] = (i_queryip >> 24) & 0xff;
		query_ip[1] = (i_queryip >> 16) & 0xff;
		query_ip[2] = (i_queryip >> 8) & 0xff;
		query_ip[3] = (i_queryip >> 0) & 0xff;
		_ip_2string(query_ip, sz_query_ip);

		if (index < n_threads) {
			thread_arg = (char *)malloc(20);
			strcpy(thread_arg, sz_query_ip);
			check_ip_args[index] = thread_arg;
			check_ip_result[index] = -1;
			check_ip_pids[index] = 0;

			i_queryip++;
			
			if ((i_queryip & 0xff) == 0 || (i_queryip & 0xff) == 1 || (i_queryip & 0xff) == 255) {
				i_queryip += 3;
			} 
			index++;
		} else if (index == n_threads) {
			bool found = false;
			// threads start/stop
			multi_work_start(n_threads, check_ip_result, check_ip_pids,arp_check_ip_proc, check_ip_args);
			usleep(100*1000);
			multi_work_stop(n_threads, check_ip_result, check_ip_pids);
			//check results
			for ( i = 0; i < n_threads; i++) {
				ret = check_ip_result[i];
				//printf("thread %d args:%s ret:%d\n", i, check_ip_args[i], check_ip_result[i]);
				if (ret == 0) {
					found = true;
					if (hook) 
						hook(sz_l_ip, sz_l_netmask, check_ip_args[i], NULL);
					n_found++;
				}
				if (check_ip_args[i]) {
					free(check_ip_args[i]);
					check_ip_args[i] = NULL;
				}
				if (n_found >= need_n) {
					trigger = false;
					break;
				}
			}
			if (found == false) {
				if (i_mask >= 0xffffff00) {
					if (( i_end - need_n -1)> i_start){
						i_queryip = i_end - need_n - irevert++;
						if (i_queryip <= i_start)
							break;
					}else
						i_queryip += 3;
				} else if (i_mask >= 0xffff0000) {
					i_queryip += 0xff;
				} else if (i_mask >= 0xff000000){
					i_queryip += 0xffff;
				}
			}

			// reset index
			index = 0;
		}
		if ((time(NULL) - __tt) > timeout_s)  {
			printf("NET_find_avai_ip timeout!\n");
			break;
		}
	}
	printf("NET_find_avai_ip %d/%d, spent %ld s (timeout: %ds)\n", n_found, need_n, time(NULL) - __tt , timeout_s);

	for ( i = 0; i < n_threads; i++) {
		if (check_ip_args[i]) {
			free(check_ip_args[i]);
			check_ip_args[i] = NULL;
		}
	}
	
	return n_found;
}

#define NET_LAN_DHCP_SCRIPT IPCAM_ENV_HOME_DIR"/shell/landhcp.script"
void NET_openUdhcpc(char *interfaceName)
{
	char str[128];
	
	memset(str, 0, sizeof(str));
	sprintf(str, "udhcpc -q -s %s -i %s &", NET_LAN_DHCP_SCRIPT, interfaceName);
	NK_SYSTEM(str);
}
