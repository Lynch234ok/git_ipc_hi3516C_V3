#include <stdio.h>
#include <time.h> 
#include "zw_hik_dh_xm.h"

stSearch_Device_Type search_Device ;

/****************************************/
//���õ�ǰ���������豸��Ϣ��
//device [in]		���õ�ǰ�����������豸��Ϣ��
//return 			0���óɹ�����������ʧ�ܡ�
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
//��鵱ǰ�����ǲ��Ǹ���һ��������ͬһ���豸�������ġ�
//device [in]		��ǰ���������������豸��
//return 			0 ������ͬ�豸���������߲���ͬһ��ʱ�䷢��; 1 ͬһ���豸��ͬһ��ʱ�䷢���������� -1,����
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