

#ifndef __MPEG2TS_H__
#define __MPEG2TS_H__

#include <stdint.h>
#include <stdbool.h>


#define TRANSPORT_PACKET_SIZE (1)

#pragma pack(1)

typedef struct
{
	//byte 0
	unsigned char sync_byte; //ͬ���ֽڣ��̶�Ϊ0x47 ����ʾ�������һ��TS���飬��Ȼ��������е������ǲ������0x47��

	//byte 1
	unsigned char PID_high5 :5; //PID��ͷ5λ
	unsigned char transport_priority :1; //�������ȼ�λ��1��ʾ�����ȼ���������ƿ����õ�����������ò��š�
	unsigned char payload_unit_start_indicator :1; //���λ�����е㸴�ӣ�������˼����Ч���صĿ�ʼ��־�����ݺ�����Ч���ص����ݲ�ͬ����Ҳ��ͬ��
	unsigned char transport_error_indicator :1; //��������־λ��һ�㴫�����Ļ��Ͳ��ᴦ���������

	//byte 2
	unsigned char PID_low8; //PID�ĺ�8λ������Ƚ���Ҫ��ָ�������������Ч�������ݵ����ͣ���������������������ʲô���ݡ�

	//byte 3
	unsigned char continuity_counter :4; //һ��4bit�ļ���������Χ0-15��������ͬ��PID��TS���鴫��ʱÿ�μ�1����15����0����������Щ������ǲ������ġ�
	unsigned char adaptation_field_control :2; //��ʾTS�����ײ������Ƿ�����е����ֶκ���Ч���ء�01������Ч���أ�10���������ֶΣ�11���е����ֶκ���Ч���ء�Ϊ00�Ļ������������д����շ���û�е����ֶ�
	unsigned char transport_scrambling_control :2; //���ܱ�־λ����ʾTS������Ч���صļ���ģʽ��TS�����ײ�(Ҳ����ǰ����32bit)�ǲ�Ӧ�����ܵģ�00��ʾδ���ܡ�

	//byte 4-187
	unsigned char data[188 - 4];
} _MPEGTS_PACKET; //32bit


typedef struct TRANSPORT_PACKET
{
	uint8_t syn_byte;
	//!< ����1B�����ֶΣ���ֵΪ0x47�����ֶ���MPEG-2TS�Ĵ��Ͱ���ʶ����

	uint8_t PID_bit12_8 : 5;
	//!< ���13b���ȵ��ֶΣ���ʾ�洢�ڴ��Ͱ�����Ч���������ݵ����͡�
	//!< PID=0x0000��ʾ���ɵ�����Ϊ��Ŀ������
	//!< PID=0x0001��ʾ���ɵ�����Ϊ�������ʱ�
	//!< PID=0x0003~0x000FΪ������
	//!< PID=0x1FFF��ʾ���ɵ����ݿհ���
	//!< ����PIDֵ��ʾ���ɵ�����Ϊ��Ŀӳ���������Ϣ���Լ����û�����������Ƶ/��Ƶ����PES���ȡ�

	uint16_t transport_priority : 1;
		//!< ����һ��1b���ȵ��ֶΡ������ֶ���Ϊ1����ʾ��صİ�������������ͬPID�����ֶ�Ϊ��0�� �İ��и��ߵ�����Ȩ�����Ը��ݴ��ֶ�ȷ����һ��ԭʼ�������ݵĴ������ȼ���
	uint16_t payload_unit_start_indicator : 1;
	//!< ��ʱ1b���ȵ��ֶΡ����ֶ�������ʾTS������Ч������PES������PSI���ݵ������
	uint16_t transport_error_indicator : 1;
	//!< ����һ��1b���ȵ��ֶΡ�ֵΪ1ʱ����ʾ����صĴ��Ͱ���������һ�����ɾ����Ĵ���ֻ���ڴ������֮�󣬸�θ���ܱ�������0��

	uint8_t PID_bit7_0; //!< Pid�ĵ�8λ
	
	uint8_t continuity_counter : 4;
		//!< ����һ��4b���ȵ��ֶΡ����ž�����ͬ��PID TS�������Ӷ����ӣ������ﵽ���ʱ���лָ�Ϊ��0������������ֶο���ֵadaptation_field_controlΪ��00����10���������������������ӡ�
	uint8_t adaptation_field_control : 2;
	//!< ����һ��2b�����ֶΣ���ʾ���������ײ��Ƿ�����е����ֶκ�/����Ч���ɡ�
	uint8_t transport_scrambling_control : 2;
	//!< ����һ��2b�����ֶΡ����ֶ�����ָʾ����������Ч���ɵļ��ŷ�ʽ��������������ײ����������ֶΣ���Ӧ�ñ����š����ڿհ���transport_scrambling_control��ֵΪ��00����
	
	uint8_t adaptation_field_or_payload[188 - 4];
	
}TRANSPORT_PACKET_t;
#pragma pack()


extern uint32_t mp2ts_pcr_base_us(uint64_t t_us);
extern uint32_t mp2ts_pcr_ext_us(uint64_t t_us);

extern uint32_t mp2ts_pcr_base_s(uint32_t t_s);
extern uint32_t mp2ts_pcr_ext_s(uint32_t t_s);


#endif //__MPEG2TS_H__

