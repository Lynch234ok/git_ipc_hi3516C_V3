#ifndef ___ZW_HIK_DH_XM__H___
#define ___ZW_HIK_DH_XM__H___

typedef enum {
	PROTOCOL_SEARCH_TYPE_ZW = 1,
	PROTOCOL_SEARCH_TYPE_HIK,
	PROTOCOL_SEARCH_TYPE_DH,
	PROTOCOL_SEARCH_TYPE_XM,
	PROTOCOL_SEARCH_TYPE_TST,
}ePROTOCOL_SEARCH_TYPE;

typedef struct Search_Device_Type{
	int type;
	struct timespec cpu_clock;   	//cpu时钟，也就是运行时间
//	char peerip[32];
	char mac[32];					//发送搜索的设备的MAC
}stSearch_Device_Type,*lpSearch_Device_Type;

/****************************************/
//设置当前搜索发出设备信息。
//device [in]		设置当前搜索发出的设备信息。
//return 			0设置成功，非零设置失败。
/****************************************/
extern int protocol_search_set_type(lpSearch_Device_Type device);

/****************************************/
//检查当前搜索是不是跟上一个搜索是同一个设备发出来的。
//device [in]		当前检查的搜索发出的设备。
//return 			检查返回结果：0 不是相同设备发出或者不是同一个时间发出; 1 同一个设备，同一个时间发出的搜索。 -1,错误
/****************************************/
extern int protocol_search_check_type(lpSearch_Device_Type device);

#endif