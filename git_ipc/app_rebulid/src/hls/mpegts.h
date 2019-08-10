/*
 * mpegts.h
 *
 *  Created on: 2012-2-8
 *      Author: root
 */

#ifndef MPEGTS_H_
#define MPEGTS_H_

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


//http://www.360doc.com/content/11/1214/20/475245_172272916.shtml
//http://colibrios.googlecode.com/svn-history/r8/trunk/Stream/Streamer/src/TS.c
//http://www.etherguidesystems.com/Help/SDOs/MPEG/Syntax/TableSections/PAT.aspx
//http://dvd.sourceforge.net/dvdinfo/pes-hdr.html (pes)
//actamont.tuke.sk/pdf/2008/n1/23dalmi.pdf(sdt)
//http://hi.baidu.com/bamboolsu/blog/item/a30335d8367a3b2710df9bed.html(pts/dts��ʱ��Ķ��չ�ϵ)

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
} MPEGTS_PACKET; //32bit
#define MPEG_PID(_ptr) ((_ptr->PID_high5 << 8) | _ptr->PID_low8)
#define MPEG_SET_PID(_ptr, n) ((_ptr)->PID_high5 = ((n >> 8) & 0x1f), (_ptr)->PID_low8 = (n & 0xff))
#define MPEG_SYNC_BYTE_CONST (0x47)


typedef struct
{
	//byte 0
	unsigned char table_id;//�̶�Ϊ0x00 ����־�Ǹñ���PAT

	//byte 1
	unsigned char section_length_high4:4;
	unsigned char reserved:2;//0x03
	unsigned char zero:1;//0x00
	unsigned char section_syntax_indicator:1;//���﷨��־λ���̶�Ϊ1

	//byte 2
	unsigned char section_length_low8;//��ʾ����ֽں������õ��ֽ���(֮ǰ��3���ֽ�)������CRC32�����������ֽڼ���ǰ����ֽ�������188���������0XFF��䡣���������ֵ�Ƚϴ���PAT��ֳɼ����������䡣

	//byte 3-4
	unsigned char transport_stream_id_high8;
	unsigned char transport_stream_id_low8;//�ô�������ID��������һ��������������·���õ�����

	//byte 5
	unsigned char current_next_indicator:1;//��ʾ���͵�PAT�ǵ�ǰ��Ч������һ��PAT��Ч��

	unsigned char version_number:5;//��Χ0-31����ʾPAT�İ汾�ţ���ע��ǰ��Ŀ�İ汾�����Ǹ��ǳ����õĲ���������⵽����ֶθı�ʱ��˵��TS���еĽ�Ŀ�Ѿ��仯�ˣ������������������Ŀ��
	unsigned char reserved2:2;//0x03

	//byte 6
	unsigned char section_number;//�ֶεĺ��롣PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�

	//byte 7
	unsigned char last_section_number;//���һ���ֶεĺ���
} MPEGTS_PAT;
#define MPEGTS_PAT_SECTION_LENGTH(_ptr) ((_ptr->section_length_high4 << 8) | _ptr->section_length_low8)
#define MPEGTS_PAT_TRANSPORT_STREAM_ID(_ptr) ((_ptr->transport_stream_id_high8 << 8) | _ptr->transport_stream_id_low8)

typedef struct
{
	//byte 0
	unsigned char table_id;
	//byte 1
	unsigned char section_length_high4:4;
	unsigned char reserved:2;
	unsigned char zero:1;
	unsigned char section_syntax_indicator:1;
	//byte 2
	unsigned char section_length_low8;
	//byte 3
	unsigned char program_number_high8;
	//byte 4
	unsigned char program_number_low8;
	//byte 5
	unsigned char current_next_indicator:1;
	unsigned char version_number:5;
	unsigned char reserved2:2;
	//byte 6
	unsigned char section_number;
	//byte 7
	unsigned char last_section_number;
	//byte 8
	unsigned char PCR_PID_high5:5;
	unsigned char reserved3:3;
	//byte 9
	unsigned char PCR_PID_low8;
	//byte 10
	unsigned char program_info_length_high4:4;
	unsigned char reserved4:4;
	//byte 11
	unsigned char program_info_length_low8;
} MPEGTS_PMT; //12bytes
#define MPETTS_PMT_SECTION_LENGTH(_ptr) ((_ptr->section_length_high4 << 8) | _ptr->section_length_low8)
#define MPETTS_PMT_PROGRAM_NUMBER(_ptr) ((_ptr->program_number_high8 << 8) | _ptr->program_number_low8)
#define MPETTS_PMT_PCR_PID(_ptr) ((_ptr->PCR_PID_high5 << 8) | _ptr->PCR_PID_low8)
#define MPETTS_PMT_PROGRAM_INFO_LENGTH(_ptr) ((_ptr->program_info_length_high4 << 8) | _ptr->program_info_length_low8)

typedef struct
{
	//byte 0
	unsigned char stream_type;
	//byte 1
	unsigned char elementary_PID_high5:5;
	unsigned char reserved:3;
	//byte 2
	unsigned char elementary_PID_low8;
	//byte 3
	unsigned char ES_info_length_high4:4;
	unsigned char reserved2:4;
	//byte 4
	unsigned char ES_info_length_low8;
} MPEGTS_PMT_ENTRY;
#define MPEGTS_PMT_ENTRY_ELEMENTARY_PID(_ptr) ((_ptr->elementary_PID_high5 << 8) | _ptr->elementary_PID_low8)
#define MPEGTS_PMT_ENTRY_ES_INFO_LENGTN(_ptr) ((_ptr->ES_info_length_high4 << 8) | _ptr->ES_info_length_low8)

typedef struct
{
	//byte 0-1
	unsigned char program_number_high8;
	unsigned char program_number_low8;
	//byte 3
	unsigned char PID_high5:5;
	unsigned char reserved:3;
	//byte 4
	unsigned char PID_low8;


} MPEGTS_PAT_ENTRY;
#define MPEGTS_PAT_ENTRY_PROGROM_NUMBER(_ptr) ((_ptr->program_number_high8 << 8) | _ptr->program_number_low8)
#define MPEGTS_PAT_ENTRY_PID(_ptr) ((_ptr->PID_high5 << 8) | _ptr->PID_low8)

typedef struct
{
	//byte 0
	unsigned char adaptation_field_length;
	//byte 1
	//the adaptation_field_extension_flag is a 1-bit boolean field in a MPEG-2 adaptation_field that signals, when '1' 0r 'True' signals the presence of an adaptation field extension. A value of '0' signals that an adaptation field extension is not present in the adaptation field.
	unsigned char adaptation_field_extension_flag:1;
	//the transport_private_data_flag is a 1-bit boolean field in a MPEG-2 adaptation_field that signals, when '1' 0r 'True' that the adaption field contains one or more private data bytes. A value of '0' signals that the adaption field does not contain private data bytes.
	unsigned char transport_private_data_flag:1;
	//the splicing_point_flag is a 1-bit boolean field in a MPEG-2 adaptation_field that signals, when '1' 0r 'True' that indicates a splice_countdown field is present in the adaptation field, specifying the occurrence of a splicing point. A value of '0' signals that a splice_countdown field is not present in the adaptation field.
	unsigned char splicing_point_flag:1;
	//the OPCR_flag is a 1-bit boolean field in a MPEG-2 adaptation_field that signals, when '1' 0r 'True' that an original program clock reference will be found in this packet. Otherwise, this field contains a '0' or 'False.'
	unsigned char OPCR_flag:1;
	//the PCR_flag is a 1-bit boolean field in a MPEG-2 adaptation_field that signals, when '1' 0r 'True' that a program clock reference will be found in this packet. Otherwise, this field contains a '0' or 'False.'
	unsigned char PCR_flag:1;
	//the elementary_stream_priority_indicator is a 1-bit boolean field in a MPEG-2 adaptation_field that signals the priority of this packet. If set to '1' or 'True', signals that the packet has higher priority than other packets. If set to '0' or 'False', signals that the packet has the same priority as all other packets..
	unsigned char elementary_stream_priority_indicator:1;
	//the random_access_indicator is a 1-bit boolean field in a MPEG-2 adaptation_field that indicates the current packet and possibly other packets with the same packet id, contain some informaition to aid random access at this point.
	unsigned char random_access_indicator:1;
	//the discontinuity_indicator is a 1-bit boolean field in a MPEG-2 adaptation_field that signals the continuity state of a packet.
	//If discontinuity_indicator is equal to '1' or 'True', the discontinuity state for that packet is 'True.' When discontinuity state is set to '0' or 'False' or is not present, the discontinuity state for the packet is 'False.'
	unsigned char discontinuity_indicator:1;
} MPEGTS_ADAPTATION_FIELD; //2�ֽ�


typedef struct
{
	//byte 0-3
	unsigned char program_clock_reference_base_26_33;
	unsigned char program_clock_reference_base_18_25;
	unsigned char program_clock_reference_base_10_17;
	unsigned char program_clock_reference_base_2_9;
	//byte 4
	unsigned char program_clock_reference_base_1:1;
	unsigned char reserved:6;
	unsigned char program_clock_reference_extension_high1:1;
	//byte 5
	unsigned char program_clock_reference_extension_low8;
} MPEGTS_PCR;
#define MPEG_PCR_PROGRAM_CLOCK_REFERENCE_BASE _MPEG_PROGRAM_CLOCK_REFERENCE_BASE
#define MPEG_PCR_SET_PROGRAM_CLOCK_REFERENCE_BASE(_ptr, _n) _MPEG_PCR_SET_PROGRAM_CLOCK_REFERENCE_BASE(_ptr, _n)
#define MPEG_PCR_PROGRAM_CLOCK_REFERENCE_EXTENSION(_ptr) ((_ptr->program_clock_reference_extension_high1 << 8) | _ptr->program_clock_reference_extension_low8)
#define MPEG_PCR_SET_PROGRAM_CLOCK_REFERENCE_EXTENSION(_ptr, _n) ((_ptr)->program_clock_reference_extension_high1 = (_n >> 8) & 0x01, (_ptr)->program_clock_reference_extension_low8 = _n & 0xff)
#define MPEG_PCR_PROGRAM_CLOCK_RESERVED_CONST (0x3f)

typedef struct
{
	//byte 0
	unsigned char table_id;//������0x42,��ʾ�������ǵ�ǰ������Ϣ,Ҳ������0x46,��ʾ������������Ϣ(EPGʹ�ô˲���)
	//byte 1
	unsigned char section_length_high4:4;
	unsigned char reserved:1;//����λ,��ֹ�����ֳ�ͻ,һ����''0'',Ҳ�п�����''1''
	unsigned char reserved_future_used:2;//����δ��ʹ��
	unsigned char section_syntax_indicator:1;//���﷨��־,һ����''1''
	//byte 2
	unsigned char section_length_low8;//�Ķγ���,��λ��Bytes,��transport_stream_id��ʼ,��CRC_32����(����)
	//byte 3-4
	unsigned char transport_stream_id_high8;
	unsigned char transport_stream_id_low8;//��ǰ��������ID
	//byte 5
	unsigned char current_next_indicator:1;//��ǰδ����־,һ����''0'',��ʾ��ǰ����ʹ��.
	unsigned char version_number:5;//�İ汾����,������ݸ�������ֶε���1
	unsigned char reserved2:2;//����λ
	//byte 6
	unsigned char section_number;
	//byte 7
	unsigned char last_section_number;
	//byte 8
	unsigned char original_netword_id_high8;//ԭʼ����ID��
	//byte 9
	unsigned char original_netword_id_low8;//ԭʼ����ID��
	//byte 10
	unsigned char reserved_future_use;//����δ��ʹ��λ
} MPEGTS_SDT; //11bytes
#define MPEGTS_SDT_SECTION_LENGTH(_ptr) ((_ptr->section_length_high4 << 8) | _ptr->section_length_low8)
#define MPEGTS_SDT_TRANSPORT_STREAM_ID(_ptr) ((_ptr->transport_stream_id_high8 << 8) | _ptr->transport_stream_id_low8)
#define MPEGTS_SDT_ORIGINAL_NETWORK_ID(_ptr) ((_ptr->original_netword_id_high8 << 8) | _ptr->original_netword_id_low8)

typedef struct
{
	//byte 0
	unsigned char service_id_high8;
	//byte 1
	unsigned char service_id_low8;
	//byte 2
	unsigned char EIT_present_following_flag:1;
	unsigned char EIT_schedule_flag:1;
	unsigned char reserved_future_use:6;
	//byte 3
	unsigned char descriptor_loop_length_high4:4;
	unsigned char free_CA_mode:1;
	unsigned char running_status:3;
	//byte 4
	unsigned char descriptor_loop_length_low8;
} MPEGTS_SDT_ENTRY;
#define MPEGTS_SDT_ENTRY_SERVICE_ID(_ptr) ((_ptr->service_id_high8 << 8) | _ptr->service_id_low8)
#define MPEGTS_SDT_ENTRY_DESCRIPTOR_LOOP_LENGTH(_ptr) ((_ptr->descriptor_loop_length_high4 << 8) | _ptr->descriptor_loop_length_low8)



typedef struct
{
	//byte 0-2
	unsigned char start_code[3];
	//byte 3
	unsigned char stream_id;
	//byte 4
	unsigned char PES_packet_length_high8;
	//byte 5
	unsigned char PES_packet_length_low8;
	//byte 6
	unsigned char original_or_copy:1;
	unsigned char copyright:1;
	unsigned char data_alignment_indicator:1;
	unsigned char PES_priority:1;
	unsigned char PES_scrambling_control:2;
	unsigned char reserved:2;//10b
	//byte 7
	unsigned char PES_extension_flag:1;
	unsigned char PES_CRC_flag:1;
	unsigned char additional_copy_info_flag:1;
	unsigned char DSM_trick_mode_flag:1;
	unsigned char ES_rate_flag:1;
	unsigned char ESCR_flag:1;
	unsigned char PTS_DTS_flag:2;
	//byte 8
	unsigned char PES_header_data_length;
//	//byte 9
//	unsigned char PTS;
//	unsigned char DTS;
} MPEGTS_PES;
#define MPEGTS_PES_PES_PACKET_LENGTH(_ptr) ((_ptr->PES_packet_length_high8 << 8) | _ptr->PES_packet_length_low8)
#define MPEGTS_PES_SET_PES_PACKET_LENGTH(_ptr, _n) ((_ptr)->PES_packet_length_high8 = ((_n) >> 8) & 0xff, (_ptr)->PES_packet_length_low8 = (_n) & 0xff)
#define MPEGTS_PES_SET_START_CODE(_ptr) ((_ptr)->start_code[0] = 0, (_ptr)->start_code[1] = 0, (_ptr)->start_code[2] = 1)
#define MPEGTS_PES_RESERVED_CONST (2)



//���Կ���PTS/DTS�Ǵ���PES������ģ�������parameters�ǽ������Ƶͬ����ʾ����ֹ���������뻺�����������Ĺؼ���
//PTS��ʾ��ʾ��Ԫ������ϵͳĿ�������(STD: system target decoder)��ʱ�䣬
//DTS��ʾ����ȡ��Ԫȫ���ֽڴ�STD��ES���뻺�������ߵ�ʱ�̡�
//ÿ��I��P��B֡�İ�ͷ����һ��PTS��DTS����PTS��DTS��B֡����һ���ģ�������B֡��DTS��
//��I֡��P֡����ʾǰһ��Ҫ�洢����Ƶ���������������򻺴����У������ӳ٣��������򣩺�����ʾ��һ��Ҫ�ֱ����PTS��DTS��
typedef struct
{
	//byte 0
	unsigned char reserved:1;
	unsigned char value_32_30:3;
	unsigned char reserved2:4;
	//byte 1
	unsigned char value_29_22;
	//byte 2
	unsigned char reserved3:1;
	unsigned char value_21_15:7;
	//byte 3
	unsigned char value_14_07;
	//byte 4
	unsigned char reserved4:1;
	unsigned char value_06_00:7;
} MPEGTS_DTS_PTS;//5bytes
#define MPEGTS_DTS_PTS_VALUE _MPEGTS_DTS_PTS_VALUE
#define MPEGTS_DTS_PTS_SET_VALUE _MPEGTS_DTS_PTS_SET_VALUE
#define MPEGTS_DTS_PTS_RESERVED_CONST (1)
#define MPEGTS_DTS_PTS_RESERVED2_CONST (2)
#define MPEGTS_DTS_PTS_RESERVED3_CONST (1)
#define MPEGTS_DTS_PTS_RESERVED4_CONST (1)

typedef struct
{
	unsigned char crc32[4];
} MPEGTS_CRC32;


#pragma pack()

#define IS_MPEG_HEAD(_ptr) (_ptr->sync_byte == MPEG_SYNC_BYTE_CONST)
#define IS_MPEGTS_PES_HEAD(_ptr) (_ptr->start_code[0] == 0x00 && _ptr->start_code[1] == 0x00 && _ptr->start_code[2] == 0x01)

u_int64_t MPEGTS_NS2PTS_PCR(u_int64_t _ns);

void MPEGTS_PCR_print(MPEGTS_PCR* _pcr);
void MPEGTS_DATA_print(unsigned char* _buf, int _len);
void MPEGTS_ADAPTATION_FIELD_print(MPEGTS_ADAPTATION_FIELD* _field);
void MPEGTS_SDT_print(MPEGTS_SDT* _sdt);
void MPEGTS_SDT_ENTRY_print(MPEGTS_SDT_ENTRY* _entry);
void MPEGTS_PAT_print(MPEGTS_PAT* _pat);
void MPEGTS_PAT_ENTRY_print(MPEGTS_PAT_ENTRY* _entry);
void MPEGTS_PMT_print(MPEGTS_PMT* _pmt);
void MPEGTS_PMT_ENTRY_print(MPEGTS_PMT_ENTRY* _entry);
void MPEGTS_PES_print_print(MPEGTS_PES* _pes);
void MPEGTS_DTS_PTS_print(MPEGTS_DTS_PTS* _ptr, int _pts_or_dts);
void MPEGTS_dump(MPEGTS_PACKET* _packet);


void _MPEG_PCR_SET_PROGRAM_CLOCK_REFERENCE_BASE(MPEGTS_PCR* _ptr, u_int64_t _n);
u_int64_t _MPEG_PROGRAM_CLOCK_REFERENCE_BASE(MPEGTS_PCR* _ptr);
void _MPEGTS_DTS_PTS_SET_VALUE(MPEGTS_DTS_PTS* _ptr, u_int64_t _n);
u_int64_t _MPEGTS_DTS_PTS_VALUE(MPEGTS_DTS_PTS* _ptr);
#endif /* MPEGTS_H_ */
