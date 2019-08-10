#include <stdio.h>
#include <time.h> 
#include "zw_hik_dh_xm.h"

stSearch_Device_Type search_Device ;

/****************************************/
//设置当前搜索发出设备信息。
//device [in]		设置当前搜索发出的设备信息。
//return 			0设置成功，非零设置失败。
/****************************************/
int protocol_search_set_type(lpSearch_Device_Type device){
	if(NULL == device){
		return -1;
	}
	
	search_Device.type = device->type;
	search_Device.cpu_clock.tv_sec = device->cpu_clock.tv_sec;
	search_Device.cpu_clock.tv_nsec = device->cpu_clock.tv_nsec;
//	strcpy(&search_Device.peerip,device->peerip);
//	strcpy(&search_Device.mac,device->mac);
	return 0;
}

/****************************************/
//检查当前搜索是不是跟上一个搜索是同一个设备发出来的。
//device [in]		当前检查的搜索发出的设备。
//return 			0 不是相同设备发出，或者不是同一个时间发出; 1 同一个设备，同一个时间发出的搜索。 -1,错误
/****************************************/
int protocol_search_check_type(lpSearch_Device_Type device){
	if(NULL == device){
		return -1;
	}
	int ret = 0;
	int time_cha = device->cpu_clock.tv_sec - search_Device.cpu_clock.tv_sec;
	if(time_cha <= 1 && time_cha >= 0){
//		if(strcmp(search_Device.mac,device->mac) ==0){
			ret = 1;
//		}
	}else{
		search_Device.type = -1; 		
		search_Device.cpu_clock.tv_sec = 0;
//		memset(&(search_Device.mac),0,sizeof(&(search_Device.mac)));
	}
	return ret;
}