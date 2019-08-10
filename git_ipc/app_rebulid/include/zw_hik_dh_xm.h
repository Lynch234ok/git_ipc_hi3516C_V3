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
	struct timespec cpu_clock;   	//cpuʱ�ӣ�Ҳ��������ʱ��
//	char peerip[32];
	char mac[32];					//�����������豸��MAC
}stSearch_Device_Type,*lpSearch_Device_Type;

/****************************************/
//���õ�ǰ���������豸��Ϣ��
//device [in]		���õ�ǰ�����������豸��Ϣ��
//return 			0���óɹ�����������ʧ�ܡ�
/****************************************/
extern int protocol_search_set_type(lpSearch_Device_Type device);

/****************************************/
//��鵱ǰ�����ǲ��Ǹ���һ��������ͬһ���豸�������ġ�
//device [in]		��ǰ���������������豸��
//return 			��鷵�ؽ����0 ������ͬ�豸�������߲���ͬһ��ʱ�䷢��; 1 ͬһ���豸��ͬһ��ʱ�䷢���������� -1,����
/****************************************/
extern int protocol_search_check_type(lpSearch_Device_Type device);

#endif