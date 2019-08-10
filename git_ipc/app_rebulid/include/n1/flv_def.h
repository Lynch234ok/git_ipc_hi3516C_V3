/**
 * FLV 相关类型定义。
 */

#include <NkUtils/types.h>
#include <NkUtils/macro.h>
#include <NkUtils/log.h>
#include <NkUtils/assert.h>

#ifndef NK_FLV_TYPES_H_
#define NK_FLV_TYPES_H_
NK_CPP_EXTERN_BEGIN


#define NK_AMF_DATA_TYPE_NUMBER			(0x00)
#define NK_AMF_DATA_TYPE_BOOL			(0x01)
#define NK_AMF_DATA_TYPE_STRING			(0x02)
#define NK_AMF_DATA_TYPE_OBJECT			(0x03)
#define NK_AMF_DATA_TYPE_NULL			(0x05)
#define NK_AMF_DATA_TYPE_UNDEFINED		(0x06)
#define NK_AMF_DATA_TYPE_REFERENCE		(0x07)
#define NK_AMF_DATA_TYPE_MIXEDARRAY		(0x08)
#define NK_AMF_DATA_TYPE_OBJECT_END		(0x09)
#define NK_AMF_DATA_TYPE_ARRAY			(0x0a)
#define NK_AMF_DATA_TYPE_DATE			(0x0b)
#define NK_AMF_DATA_TYPE_LONG_STRING	(0x0c)
#define NK_AMF_DATA_TYPE_UNSUPPORTED	(0x0d)


/**
 * FLV 标签长度数据定义。
 */
typedef NK_UInt32 NK_FLVTagPreSize;

/**
 * FLV 文件头数据结构。
 */
#pragma pack(1)
typedef union Nk_FLVFileHeadField
{
	NK_Byte spacing[9 + sizeof(NK_FLVTagPreSize)];

	struct {
		/**
		 * 始终是字符串 FLV。
		 */
		NK_Byte signature[3];

		/**
		 * 始终为 0。
		 */
		NK_Byte version;

		/**
		 * 当文件有视频的时候为 1，否则为 0。
		 */
		NK_UInt32 hasVideo :1;
		NK_UInt32 reserved :1;

		/**
		 * 当文件由音频的时候为 1，否则为 0。
		 */
		NK_UInt32 hasAudio :1;
		NK_UInt32 reserved2 :5;

		/**
		 * 数据偏移区，FLV 文件头大小相等，从文件读出来为大端模式。
		 */
		NK_UInt32 dataOffset;

		/**
		 * 必须为 0。
		 */
		NK_FLVTagPreSize zero;
	};

} NK_FLVFileHeadField;


/**
 * FLV Tag 的类型。
 */
typedef enum Nk_FLVTagType
{
	NK_FLV_TAG_TYPE_AUDIO = (8),
	NK_FLV_TAG_TYPE_VIDEO = (9),
	NK_FLV_TAG_TYPE_SCRIPT_DATA = (18),

} NK_FLVTagType;

/**
 * FLV Tag 过滤标识。
 */
typedef enum Nk_FLVTagFilter
{
	NK_FLV_TAG_FILTER_NO_PRE_PROCESS = (0),
	NK_FLV_TAG_FILTER_PRE_PROCESS = (1),

} NK_FLVTagFilter;

/**
 * FLV Tag 声音类型。
 */
typedef enum Nk_FLVTagSoundType
{
	NK_FLV_TAG_SOUND_TYPE_MONO = (0),
	NK_FLV_TAG_SOUND_TYPE_STEREO = (1),

} NK_FLVTagSoundType;

/**
 * FLV Tag 声音尺寸。
 */
typedef enum Nk_FLVTagSoundSize
{
	NK_FLV_TAG_SOUND_SIZE_8BITS = (0),
	NK_FLV_TAG_SOUND_SIZE_16BITS = (1),

} NK_FLVTagSoundSize;

/**
 * FLV Tag 声音采样率。
 */
typedef enum Nk_FLVTagSoundRate {

	NK_FLV_TAG_SOUND_RATE_5P5KHZ = (0),
	NK_FLV_TAG_SOUND_RATE_11KHZ = (1),
	NK_FLV_TAG_SOUND_RATE_22KHZ = (2),
	NK_FLV_TAG_SOUND_RATE_44KHZ = (3),

} NK_FLVTagSoundRate;


/**
 * FLV Tag 声音格式。
 *
 * Formats 7, 8, 14, and 15 are reserved.
 * AAC is supported in Flash Player 9,0,115,0 and higher.
 *
 */
typedef enum Nk_FLVTagSoundFormat {

	NK_FLV_TAG_SOUND_FMT_LPCM_PE = (0), ///< Linear PCM, platform endian
	NK_FLV_TAG_SOUND_FMT_ADPCM = (1), ///< ADPCM
	NK_FLV_TAG_SOUND_FMT_MP3 = (2), ///< MP3
	NK_FLV_TAG_SOUND_FMT_PCM_LE = (3), ///< Linear PCM, little endian
	NK_FLV_TAG_SOUND_FMT_NELLY_16KHZ_MONO = (4), ///<  Nellymoser 16 kHz mono
	NK_FLV_TAG_SOUND_FMT_NELLY_8KHZ_MONO = (5), ///< Nellymoser 8 kHz mono
	NK_FLV_TAG_SOUND_FMT_NELLY = (6), ///< Nellymoser
	NK_FLV_TAG_SOUND_FMT_G711_ALAW = (7), ///< G.711 A-law logarithmic PCM
	NK_FLV_TAG_SOUND_FMT_G711_ULAW = (8), ///< G.711 mu-law logarithmic PCM
	NK_FLV_TAG_SOUND_FMT_RESERVED = (9), ///<reserved
	NK_FLV_TAG_SOUND_FMT_AAC = (10), ///< AAC
	NK_FLV_TAG_SOUND_FMT_SPEEDX = (11), ///< Speex
	NK_FLV_TAG_SOUND_FMT_MP3_8KHZ = (14), ///< MP3 8 kHz
	NK_FLV_TAG_SOUND_FMT_DEV_SPEC_SOUND = (15), ///< Device-specific sound

} NK_FLVTagSoundFormat;


typedef enum Nk_FLVTagCodecID
{
	NK_FLV_TAG_CODEC_ID_H263 = (2),
	NK_FLV_TAG_CODEC_ID_SCREEN = (3),
	NK_FLV_TAG_CODEC_ID_VP6 = (4),
	NK_FLV_TAG_CODEC_ID_VP6_ALPHA = (5),
	NK_FLV_TAG_CODEC_ID_SCREEN_V2 = (6),
	NK_FLV_TAG_CODEC_ID_AVC = (7),

} NK_FLVTagCodecID;


typedef union Nk_FLVTagHeadField
{
	NK_Byte spacing[11];

	struct {
		/**
		 * 枚举 @ref NK_FLVTagType。
		 */
		NK_FLVTagType type :5;

		/**
		 * 枚举 @ref NK_FLVTagFilter。
		 */
		NK_FLVTagFilter filter :1;
		NK_UInt32 reserved :2;
		NK_UInt32 dataSize :24;
		NK_UInt32 timestamp : 24;
		NK_UInt32 timestampEx: 8;
		NK_UInt32 streamID :24;

		union {
			union {
				NK_Byte spacing[2];

				struct {
					/**
					 * 枚举 @ref NK_FLVTagSoundType。
					 */
					NK_FLVTagSoundType soundType :1;

					/**
					 * 枚举 @ref NK_FLVTagSoundSize。
					 */
					NK_FLVTagSoundSize soundSize :1;

					/**
					 * 枚举 @ref NK_FLVTagSoundRate。
					 */
					NK_FLVTagSoundRate soundRate :2;

					/**
					 * 枚举 @ref NK_FLVTagSoundFormat。
					 */
					NK_FLVTagSoundFormat soundFormat :4;

					/**
					 *
					 */
#define NK_FLV_TAG_AUD_AAC_SEQ_HEAD (0)    //!< #define NK_FLV_TAG_AUD_AAC_SEQ_HEAD
#define NK_FLV_TAG_AUD_AAC_RAW (1)     //!< #define NK_FLV_TAG_AUD_AAC_RAW
					NK_UInt32 aacPacketType;
				};

			} Audio;

			union {
				NK_Byte spacing[5];

				struct {
					/**
					 * 枚举 @ref Nk_FLVTagCodecID。
					 */
					NK_FLVTagCodecID codecID :4;

					/**
					 *
					 */
#define NK_FLV_TAG_VID_FRAME_AVC_KEY (1)            //!< #define NK_FLV_TAG_VID_FRAME_AVC_KEY
#define NK_FLV_TAG_VID_FRAME_AVC_INNER (2)          //!< #define NK_FLV_TAG_VID_FRAME_AVC_INNER
#define NK_FLV_TAG_VID_FRAME_H263_INNER (3)         //!< #define NK_FLV_TAG_VID_FRAME_H263_INNER
#define NK_FLV_TAG_VID_FRAME_GENERATED_KEY_FRAME (4)    //!< #define NK_FLV_TAG_VID_FRAME_GENERATED_KEY_FRAME
#define NK_FLV_TAG_VID_FRAME_INFO_OR_COMMAND (5)    //!< #define NK_FLV_TAG_VID_FRAME_INFO_OR_COMMAND

					NK_UInt32 frameType :4;

					/**
					 *
					 */
#define NK_FLV_TAG_AVC_SEQ_HEAD (0)    //!< #define NK_FLV_TAG_AVC_SEQ_HEAD
#define NK_FLV_TAG_AVC_NALU (1)    //!< #define NK_FLV_TAG_AVC_NALU
#define NK_FLV_TAG_AVC_END_SEQ (2)    //!< #define NK_FLV_TAG_AVC_END_SEQ

					NK_UInt32 avcPacketType :8;

					/**
					 * 编码组内时间戳。
					 */
					NK_UInt32 compositionTime :24;
				};

			} Video;

			union {
				NK_Byte spacing[8];

			} Encryption;

			union {
				NK_Byte spacing[8];

			} FilterParams;

		};
	};

} NK_FLVTagHeadField;

#define NK_FLV_SCRIPT_DAT_VAL_NUM (0) ///< Number
#define NK_FLV_SCRIPT_DAT_VAL_BOOL (1) ///< Boolean
#define NK_FLV_SCRIPT_DAT_VAL_STR (2) ///< String
#define NK_FLV_SCRIPT_DAT_VAL_OBJ (3) ///< Object
#define NK_FLV_SCRIPT_DAT_VAL_MOVIE_CLIP (4) ///< MovieClip (reserved, not supported)
#define NK_FLV_SCRIPT_DAT_VAL_NULL (5) ///< Null
#define NK_FLV_SCRIPT_DAT_VAL_UNDEF (6) ///< Undefined
#define NK_FLV_SCRIPT_DAT_VAL_REF (7) ///< Reference
#define NK_FLV_SCRIPT_DAT_VAL_ECMA_ARRAY (8) ///< ECMA array
#define NK_FLV_SCRIPT_DAT_VAL_OBJ_END_MAKER (9) ///< Object end marker
#define NK_FLV_SCRIPT_DAT_VAL_STR_ARRAY (10) ///< Strict array
#define NK_FLV_SCRIPT_DAT_VAL_DATE (11) ///< Date
#define NK_FLV_SCRIPT_DAT_VAL_LONG_STR (12) ///< Long string

typedef NK_UInt32 NK_FlvScriptDataValue;

#pragma pack()

#define PRINT_ITEM_INTEGER(__center_off, __description, __val) \
		do { printf("%*s : %u\r\n", (__center_off), __description, (NK_UInt32)(__val)); } while(0)

#define PRINT_ITEM_NUMBER(__center_off, __description, __val) \
		do { printf("%*s : #%08x, %u\r\n", (__center_off), __description, (NK_UInt32)(__val), (NK_UInt32)(__val)); } while(0)

#define PRINT_ITEM_STRING(__center_off, __description, __val) \
		do { printf("%*s : %s\r\n", (__center_off), __description, __val); } while(0)

#define PRINT_ITEM_MAPPING(__center_off, __description, __val, __mapping) \
		do { \
			printf("%*s : %u - %s\r\n",\
					(__center_off),\
					__description,\
					(NK_UInt32)(__val),\
					(__val) < (sizeof(__mapping) / sizeof(__mapping[0])) ? __mapping[__val] : NK_Nil);\
		} while(0)

static inline NK_Size NK_FLV_TAG_HEADER_SIZE(NK_FLVTagHeadField *Header)
{
	NK_Size size = 0;

	switch (Header->type)
	{
	case NK_FLV_TAG_TYPE_AUDIO:
		size = sizeof(Header->spacing) + sizeof(Header->Audio.spacing);
		break;

	case NK_FLV_TAG_TYPE_VIDEO:
		size = sizeof(Header->spacing) + sizeof(Header->Video.spacing);
		break;

	case NK_FLV_TAG_TYPE_SCRIPT_DATA:
		/// FIXME
		size = sizeof(Header->spacing);
		break;

	default:
		break;
	}

	return size;
}

static inline NK_Size NK_FLV_TAG_SIZE(NK_FLVTagHeadField *Header)
{
	return sizeof(Header->spacing) + Header->dataSize;
}


#define NK_FLV_HEAD_FIELD_DUMP(__Header) \
	do {\
		NK_Char signature[4];\
		NK_TermTable Table;\
		NK_BZERO(signature, sizeof(signature));\
		memcpy(signature, (__Header)->signature, sizeof((__Header)->signature));\
		NK_CHECK_POINT();\
		NK_TermTbl_BeginDraw(&Table, "FLV File Head Field", 64, 4);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Signature", "%s", signature);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Version", "%d", (__Header)->version);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Reserved", "%d", (__Header)->reserved);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Has Audio", "%s", (__Header)->hasAudio ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Reserved", "%d", (__Header)->reserved2);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Has Video", "%s", (__Header)->hasVideo ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Data Offset", "%d", (__Header)->dataOffset);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Zero", "%d", (__Header)->zero);\
		NK_TermTbl_EndDraw(&Table);\
	} while(0)

static inline NK_Void NK_FLV_TAG_DUMP(NK_FLVTagHeadField *Tag)
{
	static NK_PChar filter_map[32];
	static NK_PChar tag_type_map[32];
		
	static NK_PChar sound_type_map[32];		
	static NK_PChar sound_size_map[32];		
	static NK_PChar sound_rate_map[32];		
	static NK_PChar aac_packet_type_map[32];
	
	static NK_PChar sound_format_map[32];
	static NK_PChar avc_packet_type_map[32];
	static NK_PChar frame_type_map[32];
	static NK_PChar codec_id_map[32];

	filter_map[NK_FLV_TAG_FILTER_NO_PRE_PROCESS] =
			"No Pre-Processing Required";
	filter_map[NK_FLV_TAG_FILTER_PRE_PROCESS] =
			"Pre-Processing (Such as Decryption)";

	tag_type_map[NK_FLV_TAG_TYPE_AUDIO] = "Audio";
	tag_type_map[NK_FLV_TAG_TYPE_VIDEO] = "Video";
	tag_type_map[NK_FLV_TAG_TYPE_SCRIPT_DATA] = "Script Data";

	printf("\r\n");
	PRINT_ITEM_INTEGER(16, "Reserved", Tag->reserved);
	PRINT_ITEM_MAPPING(16, "Filter", Tag->filter, filter_map);
	PRINT_ITEM_MAPPING(16, "Tag Type", Tag->type, tag_type_map);
	PRINT_ITEM_NUMBER(16, "Data Size", Tag->dataSize);
	PRINT_ITEM_NUMBER(16, "Timestamp", Tag->timestamp);
	PRINT_ITEM_INTEGER(16, "Stream ID", Tag->streamID);

	if (NK_FLV_TAG_TYPE_AUDIO == Tag->type)
	{

		sound_format_map[NK_FLV_TAG_SOUND_FMT_LPCM_PE] =
				"Linear PCM, Platform Endian";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_ADPCM] = "ADPCM";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_MP3] = "MP3";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_PCM_LE] =
				"Linear PCM, Little Endian";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_NELLY_16KHZ_MONO] =
				"Nellymoser 16 kHz Mono";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_NELLY_8KHZ_MONO] =
				"Nellymoser 8 kHz Mono";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_NELLY] = "Nellymoser";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_G711_ALAW] =
				"G.711 A-law Logarithmic PCM";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_G711_ULAW] =
				"G.711 mu-law Logarithmic PCM";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_RESERVED] = "Reserved";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_AAC] = "AAC";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_SPEEDX] = "Speex";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_MP3_8KHZ] = "MP3 8 kHz";
		sound_format_map[NK_FLV_TAG_SOUND_FMT_DEV_SPEC_SOUND] =
				"Device-specific Sound";


		sound_type_map[NK_FLV_TAG_SOUND_TYPE_MONO] = "Mono Sound";
		sound_type_map[NK_FLV_TAG_SOUND_TYPE_STEREO] = "Stereo Sound";


		sound_size_map[NK_FLV_TAG_SOUND_SIZE_8BITS] = "8-bit";
		sound_size_map[NK_FLV_TAG_SOUND_SIZE_16BITS] = "16-bit";


		sound_rate_map[NK_FLV_TAG_SOUND_RATE_5P5KHZ] = "5.5 kHz";
		sound_rate_map[NK_FLV_TAG_SOUND_RATE_11KHZ] = "11 kHz";
		sound_rate_map[NK_FLV_TAG_SOUND_RATE_22KHZ] = "22 kHz";
		sound_rate_map[NK_FLV_TAG_SOUND_RATE_44KHZ] = "44 kHz";


		aac_packet_type_map[NK_FLV_TAG_AUD_AAC_SEQ_HEAD] =
				"AAC Sequence Header";
		aac_packet_type_map[NK_FLV_TAG_AUD_AAC_RAW] = "AAC Raw";

		PRINT_ITEM_MAPPING(16, "Sound Format", Tag->Audio.soundFormat,
				sound_format_map);
		PRINT_ITEM_MAPPING(16, "Sound Rate", Tag->Audio.soundRate,
				sound_rate_map);
		PRINT_ITEM_MAPPING(16, "Sound Size", Tag->Audio.soundSize,
				sound_size_map);
		PRINT_ITEM_MAPPING(16, "Sound Type", Tag->Audio.soundType,
				sound_type_map);
		PRINT_ITEM_MAPPING(16, "AAC Packet Type", Tag->Audio.aacPacketType,
				aac_packet_type_map);

	}
	else if (NK_FLV_TAG_TYPE_VIDEO == Tag->type)
	{

		frame_type_map[NK_FLV_TAG_VID_FRAME_AVC_KEY] =
				"Key Frame (for AVC, a Seekable Frame)";
		frame_type_map[NK_FLV_TAG_VID_FRAME_AVC_INNER] =
				"Inter Frame (for AVC, a Non-Seekable Frame)";
		frame_type_map[NK_FLV_TAG_VID_FRAME_H263_INNER] =
				"Disposable Inter Frame (H.263 Only)";
		frame_type_map[NK_FLV_TAG_VID_FRAME_GENERATED_KEY_FRAME] =
				"Generated Key Frame (Reserved for Server Use Only)";
		frame_type_map[NK_FLV_TAG_VID_FRAME_INFO_OR_COMMAND] =
				"Video Info / Command Frame";


		codec_id_map[NK_FLV_TAG_CODEC_ID_H263] = "Sorenson H.263";
		codec_id_map[NK_FLV_TAG_CODEC_ID_SCREEN] = "Screen Video";
		codec_id_map[NK_FLV_TAG_CODEC_ID_VP6] = "On2 VP6";
		codec_id_map[NK_FLV_TAG_CODEC_ID_VP6_ALPHA] =
				"On2 VP6 with Alpha Channel";
		codec_id_map[NK_FLV_TAG_CODEC_ID_SCREEN_V2] =
				"Screen Video Version 2";
		codec_id_map[NK_FLV_TAG_CODEC_ID_AVC] = "AVC";

		avc_packet_type_map[NK_FLV_TAG_AVC_SEQ_HEAD] = "AVC Sequence Header";
		avc_packet_type_map[NK_FLV_TAG_AVC_NALU] = "AVC NALU";
		avc_packet_type_map[NK_FLV_TAG_AVC_END_SEQ] =
				"AVC End of Sequence (Lower Level NALU Sequence Ender is NOT Required or Supported)";

		PRINT_ITEM_MAPPING(16, "Frame Type", Tag->Video.frameType,
				frame_type_map);
		PRINT_ITEM_MAPPING(16, "Code ID", Tag->Video.codecID, codec_id_map);
		PRINT_ITEM_MAPPING(16, "AVC Packet Type", Tag->Video.avcPacketType,
				avc_packet_type_map);
		PRINT_ITEM_INTEGER(16, "Composition Time", Tag->Video.compositionTime);

	}
	else if (NK_FLV_TAG_TYPE_SCRIPT_DATA == Tag->type)
	{
		/// FIXME
	}

	printf("\r\n");

}

NK_CPP_EXTERN_END
#endif /* NK_FLV_TYPES_H_ */
