/******************************************************************************

  Copyright (C), 2013-, GuangZhou JUAN Electronics Co., Ltd.
  All rights reserverd.

 ******************************************************************************
  File Name    : mpeg.h
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/06/26
  Last Modified : 2013/06/26
  Description   : mpeg2 packet and unpacket  utils , reference to ISO/IEC-13818-1 2007;ISO/IEC-13818-2									
 
  History       : 
  1.Date        : 2013/06/26
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/

#ifndef __MPEG2_PS_H__
#define __MPEG2_PS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

/****************************************************************
* configure mirco
****************************************************************/
#define MPEG_PS_SPLIT_MODE
#define MPEG_PS_SPLIT_SIZE			(60000)
#define MPEG_PES_PACKETS_PER_PS		(5)
#define MPEG_PS_SYSTEM_HEADER_PERIOD (100)
#define MPEG_MUXER_DEFAULT_VIDEO_SIZE	(512*1024)
#define MPEG_MUXER_DEFAULT_AUDIO_SIZE	(8000)

//#define MPEG_FAST_MODE
/*************************************************************************
* const micro relative to rtsp, must not modified
**************************************************************************/
#define MPEG_PS_START_CODE			(0x000001BA)
#define MPEG_PS_SYSTEM_START_CODE	(0x000001BB)
#define MPEG_PS_END_CODE			(0x000001B9)

#define MPEG_PES_START_CODE			(0x000001)
// PES STREAM ID
#define MPEG_PROGRAM_STREAM_MAP		(0xBC)
#define MPEG_PRIVATE_SREAM1			(0xBD)
#define MPEG_PADDING_STREAM			(0xBE)
#define MPEG_PRIVATE_SREAM2			(0xBF)
#define MPEG_AUDIO_STREAM_PERFIX	(0xC0)
#define MPEG_VIDEO_STREAM_PERFIX	(0xE0)
#define MPEG_ECM_STREAM				(0xF0)
#define MPEG_EMM_STREAM				(0xF1)
#define MPEG_DSM_CC_STREAM			(0xF2)
#define MPEG_ISOIEC_13522_STREAM	(0xF3)
#define MPEG_PROGRAM_STREAM_DIR		(0xFF)
//
#define MPEG_DEFAULT_VIDEO_STREAM	MPEG_VIDEO_STREAM_PERFIX
#define MPEG_DEFAULT_AUDIO_STREAM	MPEG_AUDIO_STREAM_PERFIX

#pragma pack(1)

/*********************************************************************
* mpeg program stream
* | pack header | pack 1 | pack header | pack 2 | ... | pack header | pack n |
* pack header
* | pack start code | '01' | SCR | program mux rate | | pack stuffing length | pack struffing byte | system header | PES packet 1 | PES packet 2 | ... | PES packet n |
* SCR (system clock reference): 42 bits(6 bytes)
* | always set to 01 (2b) | SCR base bits32...30 (3b) | marker (1b) | SCR base 29..15(15b) | marker (1b) | SCR base14...0(15b) | marker | SCR extension (9b) | marker (1b) |
**********************************************************************/
typedef struct mpeg_ps_header
{
	//byte1~byte4 start code
	uint8_t pack_start_code[4];  //'0x000001BA'
	// byte5 ~ byte10 system clock reference,
	//in 27MHz clock cycles, SCR = 300*base + extension
	// SCR-base = ((system_clock_frequency * t(i))/300)%2^33
	// SCR-extension = ((system_clock_frequency * t(i))/1)%300
	//byte5 
	uint8_t scr_base_28_2:2;
	uint8_t marker_bit1:1;
	uint8_t scr_base_30_3:3;
	uint8_t fix_bit:2;    //'01'
	// byte6
	uint8_t scr_base_20_8;
	// byte7
	uint8_t scr_base_13_2:2;
	uint8_t marker_bit2:1;
	uint8_t scr_base_15_5:5;
	// byte8
	uint8_t scr_base_5_8;
	// byte9
	uint8_t scr_extension_7_2:2;
	uint8_t marker_bit3:1;
	uint8_t scr_base_0_5:5; //system_clock_reference_base 33bit

	// byte10
	uint8_t marker_bit4:1;
	uint8_t scr_extension_0_7:7; //system_clock_reference_extension 9bit
	// byte11~byte13  program mux rate,valid bits:22, unit: 50bytes/sec
	// byte11
	uint8_t program_mux_rate_14_8;
	// byte 12
	uint8_t program_mux_rate_6_8;
	// byte 13
	uint8_t marker_bit6:1;
	uint8_t marker_bit5:1;
	uint8_t program_mux_rate_0_6:6;
	// byte 14
	uint8_t pack_stuffing_length:3;
	uint8_t reserved:5;
	// for ( i = 0;i< pack_stuffing_length;i++){
	//	stuffing_byte(0xff)
	//}
	// if ( nextbits = system_header_start_code){
	// 	system_header
	//}
}MpegPsHeader_t; // 14bytes
#define MPEGPS_SET_MARKER_BIT(pps)	\
	{\
		pps->marker_bit1=1;\
		pps->marker_bit2=1;\
		pps->marker_bit3=1;\
		pps->marker_bit4=1;\
		pps->marker_bit5=1;\
		pps->marker_bit6=1;\
		pps->fix_bit = 1;\
	}
#define MPEGPS_GET_SCR_BASE(pps)	\
	(pps->scr_base_30_3 << 30) | \
	(pps->scr_base_28_2 << 28) | \
	(pps->scr_base_20_8 << 20) | \
	(pps->scr_base_15_5 << 15) | \
	(pps->scr_base_13_2 << 13) | \
	(pps->scr_base_5_8  << 5 ) | \
	(pps->scr_base_0_5)
#define MPEGPS_SET_SCR_BASE(pps,scr) \
	pps->scr_base_0_5 = (scr) & 0x1f;\
	pps->scr_base_5_8 = ((scr) >> 5) & 0xff;\
	pps->scr_base_13_2 = ((scr) >> 13) & 0x03;\
	pps->scr_base_15_5 = ((scr) >> 15) & 0x1f;\
	pps->scr_base_20_8 = ((scr) >> 20) & 0xff;\
	pps->scr_base_28_2 = ((scr) >> 28) & 0x03;\
	pps->scr_base_30_3 = ((scr) >> 30) & 0x07;
#define MPEGPS_SET_SCR_EXT(pps,ext) \
	pps->scr_extension_0_7 = (ext) & 0x7f;\
	pps->scr_extension_7_2 = ((ext) >> 7) & 0x03;
#define MPEGPS_GET_SCR_EXT(pps) \
	(pps->scr_extension_7_2 << 7) | (pps->scr_extension_0_7)
#define MPEGPS_SET_MUX_RATE(pps,val) \
		pps->program_mux_rate_0_6 = (val) & 0x3f;\
		pps->program_mux_rate_6_8 = ((val) >> 6) & 0xff;\
		pps->program_mux_rate_14_8 = ((val) >> 14) & 0xff;
#define MPEGPS_GET_MUX_RATE(pps) \
	((pps->program_mux_rate_14_8 << 14) | (pps->program_mux_rate_6_8 << 6) | pps->program_mux_rate_0_6)

typedef struct mpeg_ps_system_header{
	// byte1~byte4  system header start code, must be 0x000001BB
	uint8_t system_start_code[4];
	// byte5~byte6 bytes in system header,from byte6
	uint16_t header_length;
	// byte7~byte9 : rate bound , is equal and bigger than the maximum the 
	//program mux rate in the program header,it use to the decoder the decoder capibility
	// byte 7
	uint8_t rate_bound_15_7:7;
	uint8_t marker_bit1:1;
	//byte8
	uint8_t rate_bound_7_8;
	//byte9
	uint8_t marker_bit2:1;
	uint8_t rate_bound_0_7:7;
	// byte10
	uint8_t csps_flag:1;
	uint8_t fixed_flag:1; // fixed or variable bitrate
	uint8_t audio_bound:6;//max number of audio streams in this program stream
	// byte11
	uint8_t video_bound:5;// max number of video streams in this stream
	uint8_t marker_bit3:1;
	uint8_t system_video_lock_flag:1;
	uint8_t system_audio_lock_flag:1;
	//byte 12
	uint8_t reserved:6;
	uint8_t pack_rate_restriction_flag:1;
	// while(nextbits() = == '1'){
	// 	mpeg_stream_specs
	// }
}MpegPsSystemHeader_t;
#define MPEG_SYS_SET_MARKERBIT(psys) \
	psys->marker_bit1 = 1;\
	psys->marker_bit2 = 1;\
	psys->marker_bit3 = 1;
#define MPEG_SYS_SET_RATE_BOUND(psys,val) \
	psys->rate_bound_0_7 = (val) & 0x7f;\
	psys->rate_bound_7_8 = ((val) >> 7) & 0xff;\
	psys->rate_bound_15_7 = ((val) >> 15) & 0xff;
#define MPEG_SYS_GET_RATE_BOUND(psys) \
	((psys->rate_bound_15_7 << 15) | (psys->rate_bound_7_8 << 7) | psys->rate_bound_0_7)
	
typedef struct mpeg_stream_specs
{
	// byte1
	uint8_t stream_id;//0xB8 meaning audio & 0xB9 meaning video
	// byte 2
	uint8_t p_std_buffer_size_bound_8_5:5;// if bound scale is '0',the this unit is 128bytes,else 1024bytes
	uint8_t p_std_buffer_bound_scale:1;//it should be '0' for audio,and '1' for video
	uint8_t fix_bits:2;	//must be '11'
	// byte 3
	uint8_t p_std_buffer_size_bound_0_8;
}MpegStreamSpecs_t;
#define MPEG_STREAM_SET_SIZEBOUND(p_media,val) \
	p_media->p_std_buffer_size_bound_0_8 = (val) & 0xff;\
	p_media->p_std_buffer_size_bound_8_5 = (val >> 8) & 0x1f;
#define MPEG_STREAM_GET_SIZEBOUND(p_media) \
	((p_media->p_std_buffer_size_bound_8_5 << 8) | p_media->p_std_buffer_size_bound_0_8);

typedef struct mpeg_ps_map
{
	// byte1~byte3 , start code
	uint8_t pack_start_code[3]; // must be 0x000001
	// byte4 , map stream id
	uint8_t map_stream_id; // must be 0xBC
	// byte5~byte6 , program stream map length
	uint16_t map_length; //total bytes in the program stream map from byte7 
	// byte7
	uint8_t program_steam_map_version:5;
	uint8_t reserved1:2;
	uint8_t current_next_indicator:1;// indicate this map applicable or not,if not,next table would become valid
	//byte8
	uint8_t marker_bit:1;
	uint8_t reserved2:7;
	//byte 9~byte10
	uint16_t program_stream_info_length;// descriptor length from next byte
	//
	//for(i=0;i<N;i++){
	//	descriptor()
	//}
	uint16_t elementary_stream_map_length; // total ES info map length
	// for (i=0;i<N1;i++){
	//	stream_type
	//	elementary_stream_id
	//	elementary_stream_info_length
	//	for(i=0;i<N2;i++){
	//		descriptor()
	//	}
	//}	
	//uint32_t CRC_32;
}MpegPsMap_t;

typedef struct _mpeg_es_map_info
{
	//byte1
	uint8_t stream_type;
	//byte2
	uint8_t es_id;
	//byte3~byte4
	uint16_t es_info_len;
	//	for(i=0;i<N2;i++){
	//		descriptor()
	//	}
}MpegEsMapInfo_t;

typedef struct mpeg_pes_header
{
	// byte1 ~ byte3 start code, must be 0x000001
	uint8_t pack_start_code_prefix[3];
	// bype 4 ,pes stream id
	uint8_t stream_id;
	// byte 5 & byte 6, PES packet length from byte 7 ,if this is zero
	uint16_t pes_packet_length;
	
	//if	(stream_id != program_stream_map 
	// && stream_id != padding_stream 
	// && stream_id != private_stream_2 
	// && stream_id != ECM 
	// && stream_id != EMM 
	// && stream_id != program_stream_directory 
	// && stream_id != DSMCC_stream 
	// && stream_id != ITU-T Rec. H.222.1 type E stream)
	
	// byte 7
	uint8_t original_or_copy:1;
	uint8_t copyright:1;
	uint8_t data_alignment_indicator:1;
	uint8_t PES_priority:1;
	uint8_t PES_scrambling_control:2;
	uint8_t fix_bit:2; //'10'
	// byte 8
	uint8_t PES_extension_flag:1;
	uint8_t PES_CRC_flag:1;
	uint8_t additional_copy_info_flag:1;
	uint8_t DSM_trick_mode_flag:1;
	uint8_t ES_rate_flag:1;
	uint8_t ESCR_flag:1;
	uint8_t PTS_DTS_flags:2;
	// byte 9
	uint8_t PES_header_data_length;

}MpegPesHeader_t;	//9bytes

// if PTS_DTS_flags == '10',only using pts, if '11' ,use all of pts and dts
typedef struct _pes_pts_dts
{	
	//byte1
	uint8_t marker_bit1:1;
	uint8_t pts_dts_30_3:3;
	uint8_t fix_bits:4;//PTS: '0010'; DTS:'0011'
	//byte2
	uint8_t pts_dts_22_8;
	//byte3
	uint8_t marker_bit2:1;
	uint8_t pts_dts_15_7:7;
	//byte4
	uint8_t pts_dts_7_8;
	//byte5
	uint8_t marker_bit3:1;
	uint8_t pts_dts_0_7:7;
}MpegPesPtsDts_t;

#define MPEG_SET_PTS_DTS(pts,val) \
	pts->pts_dts_0_7 = (val) & 0x7f;\
	pts->pts_dts_7_8 = ((val) >> 7) & 0xff;\
	pts->pts_dts_15_7 = ((val) >> 15) & 0x7f;\
	pts->pts_dts_22_8 = ((val) >> 22) & 0xff;\
	pts->pts_dts_30_3 = ((val) >> 30) & 0x07;\
	pts->fix_bits = 0x02;\
	pts->marker_bit1 = 1;\
	pts->marker_bit2 = 1;\
	pts->marker_bit3 = 1;
//
typedef struct _mpeg_video_stream_descriptor
{
	// byte1
	uint8_t descriptor_tag;
	//byte2
	uint8_t descriptor_length;
	//byte3
	uint8_t still_picture_flag:1;
	uint8_t constrained_param_flag:1;
	uint8_t mpeg1_only_flag:1;
	uint8_t frame_rate_code:4;
	uint8_t multiple_frame_rate_flag:1;
	//if(mpeg1_only_flag=='0'){
	//	Mpeg1OnlyInfo_t
	//}
}MpegVideoDescriptor_t;

typedef struct _mpeg_avc_video_descriptor
{
	// byte1
	uint8_t descriptor_tag;
	//byte2
	uint8_t descriptor_length;
	//byte3
	uint8_t profile_idc;
	//byte4
	uint8_t avc_compatible_flags:5;
	uint8_t constraint_set2_flag:1;
	uint8_t constraint_set1_flag:1;
	uint8_t constraint_set0_flag:1;
	//byte5
	uint8_t level_idc;
	//byte6
	uint8_t reserved:6;
	uint8_t avc_24hour_picture_flag:1;
	uint8_t avc_still_preset:1;
}MpegAvcVideoDescriptor_t;

typedef struct _mpeg_avc_timing_hrd_descriptor
{
	// byte1
	uint8_t descriptor_tag;
	//byte2
	uint8_t descriptor_length;
	//byte3
	uint8_t picture_and_timing_info_present:1;
	uint8_t reserved1:6;
	uint8_t hrd_management_valid_flag:1;
	//byte4
	//if (picture_and_timing_info_present){
		uint8_t reserved2:7;
		uint8_t freq_90khz_flag:1;
	//	if(freq_90khz_flag){
	//		N:32
	//		K:32
	//	}
	//byte5
		uint32_t num_units_in_tick;
	//}
	uint8_t reserved3:5;
	uint8_t picture_to_display_conversion_flag:1;
	uint8_t temporal_poc_flag:1;
	uint8_t fixed_frame_rate_flag:1;
}MpegAvcTimingHrdDescriptor_t;


typedef struct _mpeg1_only_info
{
	uint8_t profile_level_indication;
	uint8_t reserved:5;
	uint8_t frame_rate_ext_flag:1;
	uint8_t chroma_format:2;
}Mpeg1OnlyInfo_t;

typedef struct _audio_descriptor
{
	// byte1
	uint8_t descriptor_tag;
	//byte2
	uint8_t descriptor_length;
	//byte3
	uint8_t reserved:3;
	uint8_t variable_rate_audio_indicator:1;
	uint8_t layer:2;
	uint8_t id:1;
	uint8_t free_format_flag:1;
}MpegAudioDescriptor_t;

#pragma pack()

typedef struct _timestamp_info
{
	int flag;
	uint32_t timestamp;
	uint32_t size;
}TimestampInfo_t;

typedef struct _timestamp_node
{
	TimestampInfo_t data;
	struct _timestamp_node *next;
	struct _timestamp_node *tail;
}TimestampNode_t,TimestampList_t;

typedef struct _mpeg_muxer
{
	// circle buffer for audio and video
	void *m_videobuf;
	void *m_audiobuf;
	// list about timestamp
	TimestampList_t *m_videots;
	TimestampList_t *m_audiots;
	//
	uint8_t *m_muxbuf;//use to containt m_muxisze
	uint32_t m_pos;//current write postion in m_muxbuf
	uint32_t m_size;// allocated buffer size for m_muxbuf
	//
	uint32_t m_seq;
	//timestamp
	uint32_t m_scr;
	int m_audio_bound;
	int m_video_bound;
	int m_mux_rate;
}MpegMuxer_t;


/*****************************************************************
* public interfaces 
*****************************************************************/
extern int MPEG_muxer_video(uint8_t *src,uint32_t in_size,
	uint32_t ts,uint8_t type,int isidr,
	uint8_t *out,uint32_t out_size,
	uint32_t *ret_size);
extern int MPEG_muxer_audio(uint8_t * src,uint32_t in_size,
	uint32_t timestamp/*ms*/,uint8_t type,
	uint8_t *out,uint32_t *ret_size);


#ifdef __cplusplus
}
#endif
#endif
