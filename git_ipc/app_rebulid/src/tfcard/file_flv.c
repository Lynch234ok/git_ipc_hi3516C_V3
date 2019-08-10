

//#include <NkEmbedded/file_flv.h> //<! 模块 FileFLV 定义文件。
#include "file_flv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NkUtils/assert.h>
#include <NkUtils/macro.h>
#include <NkUtils/bufio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "avc_aac_dcr.h"
//#include "NkEmbedded/avc_aac_dcr.h"


/// NK_FLVTagHeadField 数据结构转成网络序。
static inline NK_Void
TAG_HEAD_FIELD_HTON(NK_FLVTagHeadField *TagHeadField)
{
	/// 专程网络序。
	TagHeadField->dataSize = NK_HTON24(TagHeadField->dataSize);
	TagHeadField->timestamp = NK_HTON24(TagHeadField->timestamp);

	if (NK_FLV_TAG_TYPE_VIDEO == TagHeadField->type)
	{
		TagHeadField->Video.compositionTime = NK_HTON32(TagHeadField->Video.compositionTime);
	}
}
/// NK_FLVTagHeadField 数据结构转成本地序。
static inline NK_Void
TAG_HEAD_FIELD_NTOH(NK_FLVTagHeadField *TagHeadField)
{
	/// 专程网络序。
	TagHeadField->dataSize = NK_NTOH24(TagHeadField->dataSize);
	TagHeadField->timestamp = NK_NTOH24(TagHeadField->timestamp);

	if (NK_FLV_TAG_TYPE_VIDEO == TagHeadField->type)
	{
		TagHeadField->Video.compositionTime = NK_NTOH32(TagHeadField->Video.compositionTime);
	}
}





static void BufIO_PutAMFString(NK_BufIO *BufIO, NK_PChar str)
{
	NK_Size len = (NK_Size)(strlen(str));
	NK_BufIO_PutBE16(BufIO, len);
	NK_BufIO_PutBytes(BufIO, (NK_PByte)(str), len);
}

static int64_t dbl2int(double d){
	int e;
	if     ( !d) return 0;
	else if(d-d) return 0x7FF0000000000000LL + ((int64_t)(d<0)<<63) + (d!=d);
	d= frexp(d, &e);
	return (int64_t)(d<0)<<63 | (e+1022LL)<<52 | (int64_t)((fabs(d)-0.5)*(1LL<<53));
}

void BufIO_PutAMFDouble(NK_BufIO *BufIO, NK_DFloat dval)
{
	NK_BufIO_PutByte(BufIO, NK_AMF_DATA_TYPE_NUMBER);
	NK_BufIO_PutBE64(BufIO, dbl2int(dval));
}

void BufIO_PutAMFBoolean(NK_BufIO *BufIO, NK_Boolean b)
{
	NK_BufIO_PutByte(BufIO, NK_AMF_DATA_TYPE_BOOL);
	NK_BufIO_PutByte(BufIO, !!b);
}


/// FIXME 尽快去掉。
static void
bufio_put_scriptobject_double(NK_BufIO *BufIO, NK_PChar object_name, NK_DFloat object_data)
{
	BufIO_PutAMFString(BufIO, object_name);
	BufIO_PutAMFDouble(BufIO, object_data);
}

/// FIXME 尽快去掉。
static void
bufio_put_scriptobject_end(NK_BufIO *BufIO)
{
	NK_BufIO_PutBE24(BufIO, NK_AMF_DATA_TYPE_OBJECT_END); // always 9
}

/**
 * FileFLV 模块私有句柄，句柄访问模块内部的私有成员。\n
 * 内存在 FileFLV 模块创建时统一分配。\n
 * 位置在句柄数据结构 @ref NK_FileFLV 上位，\n
 * 这样有效避免 @ref NK_FileFLV 内存空间被错误释放。\n
 * 如下图：\n
 *
 *  | NK_PrivatedFileFLV
 * \|/
 *  +------------------------+
 *  |          |             |
 *  |          |             |
 *  +------------------------+
 *            /|\
 *             | NK_FileFLV
 *
 */
typedef struct Nk_PrivatedFileFLV
{

	NK_Allocator *Alloctr;    ///< 模块内内存分配器。

	NK_Boolean writer;

	FILE *fID;

	NK_FLVFileHeadField FileHeadField;    ///< FLV 文件头数据结构。

	/**
	 * 流媒体第一帧数据的时间戳。\n
	 * 所有的流媒体以这个时间戳作参考差。
	 */
	NK_UInt64 startTimestamp;

	/**
	 * 文件音视频属性。
	 */
	struct {
		struct {

			NK_FLVTagCodecID codec_id;
			NK_Size width, height;
			NK_Size framerate, bitrate;

		} Video;

		struct {

			NK_FLVTagSoundFormat sound_fmt;
			NK_FLVTagSoundType sound_type;
			NK_FLVTagSoundSize sound_size;
			NK_FLVTagSoundRate sound_rate;

		} Audio;

	} Attr;

	/**
	 * 上一时刻的可检索帧的时间戳。\n
	 */
	NK_UInt32 prevSeekableTimestamp;

	/// Meta TAG 信息。
	struct
	{
		NK_DFloat duration;
		NK_DFloat fileSize;
		NK_DFloat width;
		NK_DFloat height;
		NK_DFloat videoDataRate;
		NK_DFloat frameRate;
		NK_DFloat videoCodecID;
		NK_DFloat audioDataRate;
		NK_DFloat audioCodecID;
		NK_DFloat canSeekToEnd;

	} Meta;

	/**
	 * 写入 Video 视频媒体时涉及的上下文。
	 */
	struct
	{
		struct
		{
			NK_Boolean seqHeaderWritten; ///< 写入了 AVC Sequence 标识。
		} AVC;

//		NK_Size n_indexs;
//		NK_Size *indexs;

	} Video;

	/**
	 * 写入 Audio 音频媒体时涉及的上下文。
	 */
	struct
	{
		struct
		{
			NK_Byte adts[2];

		} AAC;

	} Audio;

	NK_Char magic[4];    ///< 模块幻数

} NK_PrivatedFileFLV;

/**
 * 通过模块公有句柄获取私有句柄。
 */
static inline NK_PrivatedFileFLV *
PRIVATED(NK_FileFLV *Public)
{
	/// 偏移到私有句柄。
	return (NK_PrivatedFileFLV *) Public - 1;
}



/**
 * THIS 指针定义，\n
 * 定义以下为模块 API 接口实现。
 */
#define THIS NK_FileFLV * const Public

/**
 * 获取模块名称。
 */
static const NK_PChar
Object_name(NK_Object *Obj)
{
	return "FileFLV";
}

/**
 * 调试打印模块。
 */
static NK_Void
Object_dump(NK_Object *Obj)
{
}

/**
 * 模块接口线程安全标识。
 */
static NK_Boolean
Object_threadsafe(NK_Object *Obj)
{
	return NK_False;
}

static NK_Void
Object_free(NK_Object *Obj)
{
	NK_FileFLV *Public = (NK_FileFLV *)(Obj);
	NK_FileFLV_Free(&Public);
}

/**
 * 获取当前文件大小。
 */
static NK_SSize FileFLV_size(THIS)
{
	return PRIVATED(Public)->Meta.fileSize;
}


static NK_Int
FileFLV_setVideoAttr(THIS, NK_FLVTagCodecID codec_id, NK_Size width, NK_Size height, NK_Size framerate, NK_Size bitrate)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);

	if (NK_FLV_TAG_CODEC_ID_AVC != codec_id) {
		NK_Log()->error("FileFLV: Video Codec ID Support AVC Only.");
		return -1;
	}

	Privated->Attr.Video.codec_id = codec_id;
	Privated->Attr.Video.width = (0 != width) ? width : 1280;
	Privated->Attr.Video.height = (0 != height) ? height : 720;
	Privated->Attr.Video.framerate = (0 != framerate) ? framerate : 30;
	Privated->Attr.Video.bitrate = (0 != bitrate) ? bitrate : 1000;

	return 0;
}

static NK_Int
FileFLV_setAudioAttr(THIS, NK_FLVTagSoundFormat sound_fmt, NK_Boolean stereo, NK_Int32 bitwidth, NK_UInt32 samplerate)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);

	if (NK_FLV_TAG_SOUND_FMT_G711_ALAW != sound_fmt
			&& NK_FLV_TAG_SOUND_FMT_G711_ULAW != sound_fmt) {

		NK_Log()->error("FileFLV: Sound Format Support G.711 Only.");
		return -1;
	}

	Privated->Attr.Audio.sound_fmt = sound_fmt;
	Privated->Attr.Audio.sound_type =  stereo ? NK_FLV_TAG_SOUND_TYPE_STEREO : NK_FLV_TAG_SOUND_TYPE_MONO;
	if (8 != bitwidth && 16 != bitwidth) {
		NK_Log()->warn("FileFLV: Sound Size Fixed to 16bits.");
		Privated->Attr.Audio.sound_size = NK_FLV_TAG_SOUND_SIZE_16BITS;
	} else if (8 == bitwidth) {
		Privated->Attr.Audio.sound_size = NK_FLV_TAG_SOUND_SIZE_8BITS;
	} else {
		Privated->Attr.Audio.sound_size = NK_FLV_TAG_SOUND_SIZE_16BITS;
	}

	if (samplerate >= 44000) {
		if (samplerate > 44000) {
			NK_Log()->warn("FileFLV: Sound Sample Rate Fixed to 44kHz.");
		}
		Privated->Attr.Audio.sound_rate = NK_FLV_TAG_SOUND_RATE_44KHZ;

	} else if (samplerate >= 22000) {
		if (samplerate > 22000) {
			NK_Log()->warn("FileFLV: Sound Sample Rate Fixed to 22kHz.");
		}
		Privated->Attr.Audio.sound_rate = NK_FLV_TAG_SOUND_RATE_22KHZ;

	} else if (samplerate >= 11000) {
		if (samplerate > 11000) {
			NK_Log()->warn("FileFLV: Sound Sample Rate Fixed to 11kHz.");
		}
		Privated->Attr.Audio.sound_rate = NK_FLV_TAG_SOUND_RATE_11KHZ;

	} else {
		if (samplerate > 5500) {
			NK_Log()->warn("FileFLV: Sound Sample Rate Fixed to 5.5kHz.");
		}
		Privated->Attr.Audio.sound_rate = NK_FLV_TAG_SOUND_RATE_5P5KHZ;
	}

	return 0;
}



static NK_SSize FileFLV_readFileHeader(THIS)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);

	if (sizeof(Privated->FileHeadField.spacing) !=
			fread(&Privated->FileHeadField, 1, sizeof(Privated->FileHeadField.spacing), Privated->fID))
	{
		return -1;
	}

	Privated->FileHeadField.dataOffset = NK_NTOH32(Privated->FileHeadField.dataOffset);    ///< 大小端转换。
	Privated->FileHeadField.zero = NK_NTOH32(Privated->FileHeadField.zero);

	return 0;
}


/**
 * 读取 FLV TAG 的头部数据结构。
 *
 * @param[out]	Header	TAG 的头部数据结构。
 *
 * @return 成功返回 TAG 数据的长度，失败返回 -1。
 */
static NK_Int
FileFLV_readTagHeadField(THIS, NK_FLVTagHeadField *Header, NK_Size *data_len)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);
	NK_Size len = 0;

	if (sizeof(Header->spacing) != fread(Header->spacing, 1, sizeof(Header->spacing), Privated->fID))
	{
		return -1;
	}

	Header->dataSize = NK_NTOH24(Header->dataSize);
	Header->timestamp = NK_NTOH24(Header->timestamp);

	len = Header->dataSize;

	if (NK_FLV_TAG_TYPE_AUDIO == Header->type)
	{
		fread(&Header->Audio.spacing, 1, sizeof(Header->Audio.spacing), Privated->fID);
		len -= sizeof(Header->Audio.spacing);
	}
	else if (NK_FLV_TAG_TYPE_VIDEO == Header->type)
	{
		fread(&Header->Video.spacing, 1, sizeof(Header->Video.spacing), Privated->fID);
		len -= sizeof(Header->Video.spacing);

		Header->Video.compositionTime = NK_NTOH32(Header->Video.compositionTime);
	}
	else if (NK_FLV_TAG_TYPE_SCRIPT_DATA == Header->type)
	{

	}

	*data_len = len;
	return 0;
}

static int FileFLV_readPreviousTagSize(THIS, NK_FLVTagPreSize *size)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);

	if (sizeof(NK_FLVTagPreSize) != fread(size, 1, sizeof(NK_FLVTagPreSize), Privated->fID)) {
		return -1;
	}

	size[0] = NK_NTOH32(size[0]);
	return 0;
}

/**
 * 更新文件头信息。\n
 * 在写模式下面打开文件和关闭文件的时候调用。\n
 * 其他时候调用会影响文件写指针的位置状态。
 */
static NK_SSize FileFLV_putFileHeader(THIS)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);
	NK_Byte meta[1024];
	NK_FLVTagHeadField TagHeadField;
	NK_BufIO bufio;
	NK_SSize len = 0;

	if (!Privated->writer)
	{
		return 0;
	}

	/// 定位到文件开头位置。
	fseek(Privated->fID, 0, SEEK_SET);

	/// 更新文件头
	len = fwrite(&Privated->FileHeadField, 1, sizeof(Privated->FileHeadField.spacing), Privated->fID);
	if (sizeof(Privated->FileHeadField.spacing) != len)
	{
		NK_Log()->error("FLV Put File Header Failed.");
		return -1;
	}

	/// 制作 Meta Tag 包
	NK_BufIO_Reset(meta, sizeof(meta), &bufio);

	NK_BufIO_PutByte(&bufio, NK_FLV_SCRIPT_DAT_VAL_STR);
	BufIO_PutAMFString(&bufio, "onMetaData"); // 12 bytes

	// mixed array (hash) with size and string/type/data tuples
	NK_BufIO_PutByte(&bufio, NK_FLV_SCRIPT_DAT_VAL_ECMA_ARRAY);
	NK_BufIO_PutBE32(&bufio, 10); // the following array item count

	bufio_put_scriptobject_double(&bufio, "duration", Privated->Meta.duration); // fill in the guessed duration, it'll be corrected later if incorrect
	bufio_put_scriptobject_double(&bufio, "filesize", Privated->Meta.fileSize); // fill in the guessed duration, it'll be corrected later if incorrect
	bufio_put_scriptobject_double(&bufio, "videocodecid", Privated->Meta.videoCodecID);
	bufio_put_scriptobject_double(&bufio, "videodatarate", Privated->Meta.videoDataRate);
	bufio_put_scriptobject_double(&bufio, "framerate", Privated->Meta.frameRate);
	bufio_put_scriptobject_double(&bufio, "width", Privated->Meta.width);
	bufio_put_scriptobject_double(&bufio, "height", Privated->Meta.height);
	bufio_put_scriptobject_double(&bufio, "audiocodecid", Privated->Meta.audioCodecID);
	bufio_put_scriptobject_double(&bufio, "audiodatarate", Privated->Meta.audioDataRate);
	bufio_put_scriptobject_double(&bufio, "canSeekToEnd", Privated->Meta.canSeekToEnd);
	bufio_put_scriptobject_end(&bufio);

	NK_BZERO(&TagHeadField, sizeof(TagHeadField));
	TagHeadField.type = NK_FLV_TAG_TYPE_SCRIPT_DATA;
	TagHeadField.filter = NK_FLV_TAG_FILTER_NO_PRE_PROCESS;
	TagHeadField.reserved = 0;
	TagHeadField.dataSize = bufio.actlen;
	TagHeadField.timestamp = 0;
	TagHeadField.streamID = 0;

	if (Public->writeTag(Public, 0, &TagHeadField, bufio.mem, bufio.actlen) == bufio.actlen)
	{
		return 0;
	}

	return -1;
}

/**
 * 写入一个 TAG。
 */
static NK_SSize FileFLV_writeTag(THIS, NK_UInt64 ts_ms, NK_FLVTagHeadField *TagHeadField, NK_PByte data, NK_Size len)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);
	NK_FLVTagPreSize pre_size = 0;
	NK_FLVTagHeadField Hdr;
	NK_Size header_len = sizeof(Hdr.spacing);
	NK_SSize writen = 0;
	NK_DFloat file_size = 0;
	NK_UInt32 full_timestamp = 0;

	/// 读取模式下不能写入。
	if (!Privated->writer)
	{
		return -1;
	}

	/// 记录首个 TAG 数据的时间戳作为参考值。
	if (!Privated->startTimestamp)
	{
		Privated->startTimestamp = ts_ms;
	}

	/// 计算每个 TAG 的相对时间戳。
	if (0 != ts_ms)
	{
		/// 不允许小于参考时间戳。
		if (ts_ms < Privated->startTimestamp)
		{
			ts_ms = Privated->startTimestamp;
		}
		/// 换算相对时间戳。
		ts_ms = ts_ms - Privated->startTimestamp;
	}

	/// 修正传入 TAG 头部数据结构。
	TagHeadField->timestamp = ((NK_UInt32)(ts_ms) & 0x00ffffff); ///< 时间戳低 24 位是典型时间戳。
	TagHeadField->timestampEx = (NK_UInt8)((NK_UInt32)(ts_ms) >> 24); ///< 时间戳高 8 位为扩展位。

	/// 根据不同的类型判断 TAG 的头部结构长度。
	switch(TagHeadField->type)
	{
	case NK_FLV_TAG_TYPE_AUDIO:

		TagHeadField->dataSize = sizeof(TagHeadField->Audio.spacing) + len;
		header_len = sizeof(TagHeadField->spacing) + sizeof(TagHeadField->Audio.spacing);

		/// 更新文件头相关参数
		Privated->FileHeadField.hasAudio = 1;
		Privated->Meta.audioCodecID = TagHeadField->Audio.soundFormat;
		break;

	case NK_FLV_TAG_TYPE_VIDEO:

		TagHeadField->dataSize = sizeof(TagHeadField->Video.spacing) + len;
		header_len = sizeof(TagHeadField->spacing) + sizeof(TagHeadField->Video.spacing);

		/// 更新文件头相关参数
		Privated->FileHeadField.hasVideo = 1;
		Privated->Meta.videoCodecID = TagHeadField->Video.codecID;
		break;

	case NK_FLV_TAG_TYPE_SCRIPT_DATA:

		TagHeadField->dataSize = len;
		header_len = sizeof(TagHeadField->spacing);
		break;

	default:
		NK_Log()->error("FLV Tag Type(%d) NOT Support.", TagHeadField->type);
		return -1;

	}

	/// 重新换算卷对时间戳。
	full_timestamp = (TagHeadField->timestampEx << 24) + TagHeadField->timestamp;

	/// 修正视频组内部时间戳。
	if (NK_FLV_TAG_TYPE_VIDEO == TagHeadField->type)
	{
		TagHeadField->Video.compositionTime = 0;
		if (NK_FLV_TAG_CODEC_ID_AVC == TagHeadField->Video.codecID)
		{
			if (NK_FLV_TAG_VID_FRAME_AVC_KEY == TagHeadField->Video.frameType)
			{
				/// 遇到可检索帧更新视频组参考时间戳。
				Privated->prevSeekableTimestamp = full_timestamp;
			}
			/// 更新当前帧组内相对时间戳。
			TagHeadField->Video.compositionTime = full_timestamp - Privated->prevSeekableTimestamp;
		}
	}
	/// 强制为 0。
//	TagHeadField->Video.compositionTime = 0;

	/// 更新 Meta 文件时长。
	if ((full_timestamp / 1000) > (typeof(full_timestamp))(Privated->Meta.duration))
	{
		Privated->Meta.duration = (typeof(Privated->Meta.duration))(full_timestamp / 1000);
	}

	memcpy(&Hdr, TagHeadField, sizeof(Hdr));
	TAG_HEAD_FIELD_HTON(&Hdr);

	writen = fwrite(&Hdr, 1, header_len, Privated->fID);
	if (header_len != writen)
	{
		NK_Log()->error("Write FLV File %d/%d Error.", writen, header_len);
		return -1;
	}

	writen = fwrite(data, 1, len, Privated->fID);
	if (len != writen)
	{
		NK_Log()->error("Write FLV File %d/%d Error.", writen, len);
		return -1;
	}

	pre_size = header_len + len;
	pre_size = NK_HTON32(pre_size);

	writen = fwrite(&pre_size, 1, sizeof(pre_size), Privated->fID);
	if (sizeof(pre_size) != writen)
	{
		NK_Log()->error("Write FLV File %d/%d Error.", writen, sizeof(pre_size));
		return -1;
	}

	/// 更新记录中的文件大小。
	file_size = (NK_DFloat)(ftell(Privated->fID));
	if (file_size > Privated->Meta.fileSize)
	{
		Privated->Meta.fileSize = file_size;
	}

//	NK_Log()->info("Write FLV Data Len: %d Timestamp: %u.", raw_len, NK_NTOH24(Hdr.timestamp));

	return len;
}

static NK_Int
MakeVideoTagHeadField(THIS, NK_FLVTagHeadField *HeadField, NK_Size data_size)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);

	if (0 == Privated->Attr.Video.codec_id) {
		NK_Log()->error("FileFLV: Call 'NK_FileFLV::setVideoAttr' First.");
		return -1;
	}

	NK_BZERO(HeadField, sizeof(NK_FLVTagHeadField));
	HeadField->filter = NK_FLV_TAG_FILTER_NO_PRE_PROCESS;
	HeadField->type = NK_FLV_TAG_TYPE_VIDEO;
	HeadField->reserved = 0;
	HeadField->streamID = 0;
	HeadField->timestamp = 0; ///< 待定。
	HeadField->dataSize = sizeof(HeadField->Video.spacing) + data_size;
	HeadField->Video.codecID = Privated->Attr.Video.codec_id;
	HeadField->Video.frameType = 0; ///< 待定。
	HeadField->Video.avcPacketType = 0; ///< 待定。
	HeadField->Video.compositionTime = 0;

	return 0;
}

static NK_Int
MakeAudioTagHeadField(THIS, NK_FLVTagHeadField *HeadField, NK_Size data_size)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);

	if (0 == Privated->Attr.Audio.sound_fmt) {
		NK_Log()->error("FileFLV: Call 'NK_FileFLV::setAudioAttr' First.");
		return -1;
	}

	HeadField->filter = NK_FLV_TAG_FILTER_NO_PRE_PROCESS;
	HeadField->type = NK_FLV_TAG_TYPE_AUDIO;
	HeadField->reserved = 0;
	HeadField->streamID = 0;
	HeadField->timestamp = 0; ///< 待定。
	HeadField->dataSize = sizeof(HeadField->Audio.spacing) + data_size;
	HeadField->Audio.soundFormat = Privated->Attr.Audio.sound_fmt;
	HeadField->Audio.soundRate = Privated->Attr.Audio.sound_rate;
	HeadField->Audio.soundSize = Privated->Attr.Audio.sound_size;
	HeadField->Audio.soundType = Privated->Attr.Audio.sound_type;
	HeadField->Audio.aacPacketType = 0; ///< 待定。

	return 0;
}

static NK_SSize FileFLV_writeH264(THIS, NK_UInt64 ts_ms, NK_Boolean seekable, NK_PByte nalus, NK_Size nalus_len)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);
	NK_FLVTagHeadField TagHeadField;
	NK_SSize writen = 0;

	/**
	 * 遇到关键帧必须先分离 Decoder Configurate Record。
	 */
	if (seekable) {

		static NK_Byte dcr[64];
		static NK_Size dcr_len = sizeof(dcr);

		/**
		 * 生成 DCR 并追加到文件中。
		 */
		if (0 == NK_DCR_ExtractAVC(nalus, nalus_len, dcr, &dcr_len)) {

			//NK_HEX_DUMP(dcr, dcr_len);
			MakeVideoTagHeadField(Public, &TagHeadField, dcr_len);
			TagHeadField.timestamp = ts_ms;
			TagHeadField.Video.frameType = NK_FLV_TAG_VID_FRAME_AVC_KEY;
			TagHeadField.Video.avcPacketType = NK_FLV_TAG_AVC_SEQ_HEAD;
			TagHeadField.Video.compositionTime = 0;
			TagHeadField.type = NK_FLV_TAG_TYPE_VIDEO;

			/**
			 * 调用模块接口写入数据。
			 */
			writen = FileFLV_writeTag(Public, ts_ms, &TagHeadField, dcr, dcr_len);
			if(-1 == writen){
				printf("FileFLV_writeTag error\n");
				return -1;
			}
		}
	}

	MakeVideoTagHeadField(Public, &TagHeadField, nalus_len);
	TagHeadField.timestamp = ts_ms;
	TagHeadField.Video.frameType = seekable ? NK_FLV_TAG_VID_FRAME_AVC_KEY : NK_FLV_TAG_VID_FRAME_AVC_INNER;
//printf("TagHeadField.Video.frameType=%d\n", TagHeadField.Video.frameType & 0xf);
	TagHeadField.Video.avcPacketType = NK_FLV_TAG_AVC_NALU;
	TagHeadField.Video.compositionTime = 0;
	TagHeadField.type = NK_FLV_TAG_TYPE_VIDEO;

	/// 写入前先记录当前帧的偏移位置。
//	if (keyFrame)
//	{
//		Privated->Video.indexs[Privated->Video.n_indexs] = ftell(Privated->fID);
//	}

	/// 调用模块接口写入数据。
	writen = Public->writeTag(Public, ts_ms, &TagHeadField, nalus, nalus_len);

	/// 确认写入成功后登记索引。
//	if (keyFrame && writen > 0)
//	{
//		/// 增加索引
//		++Privated->Video.n_indexs;
//		if (0 == (Privated->Video.n_indexs % 1024))
//		{
//			Privated->Video.n_indexs = realloc(Privated->Video.n_indexs,
//					sizeof(Privated->Video.indexs[0])
//							* (Privated->Video.n_indexs / 1024 + 1)
//							* 1024);
//		}
//	}

	return writen;
}

#define NK_ADTS_LEN() (7)

/**
 * 检查是否为 ADTS 判断宏。
 */
static inline NK_Boolean
NK_IS_ADTS(NK_PByte buf)
{
	return (0xff == buf[0] && 0xf0 == (buf[1] & 0xf0)) ? NK_True : NK_False;
}

/**
 * 写入 AAC Raw 数据。
 */
static NK_SSize
FileFLV_writeAACRaw(THIS, NK_UInt64 timestamp, NK_Boolean stereo, NK_Int32 bitwidth, NK_UInt32 sample, NK_PByte raw, NK_Size len)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);
	NK_FLVTagHeadField TagHeadField;
	NK_SSize writen = 0;

	if (!NK_IS_ADTS(raw))
	{
		NK_Log()->error("AAC Raw Data without ADTS.");
		return -1;
	}

	NK_BZERO(&TagHeadField, sizeof(TagHeadField));
	TagHeadField.filter = NK_FLV_TAG_FILTER_NO_PRE_PROCESS;
	TagHeadField.type = NK_FLV_TAG_TYPE_AUDIO;
	TagHeadField.dataSize = 0; ///< NK_FileFLV::writeTag 接口调用后会修正。
	TagHeadField.reserved = 0;
	TagHeadField.timestamp = 0; ///< NK_FileFLV::writeTag 接口调用后会修正。
	TagHeadField.timestampEx = 0; ///< NK_FileFLV::writeTag 接口调用后会修正。
	TagHeadField.streamID = 0;
	TagHeadField.Audio.soundFormat = NK_FLV_TAG_SOUND_FMT_AAC;
	TagHeadField.Audio.soundRate = sample; ///< FIXME
	TagHeadField.Audio.soundSize = bitwidth; ///< FIXME
 	TagHeadField.Audio.soundType = stereo ? NK_FLV_TAG_SOUND_TYPE_STEREO : NK_FLV_TAG_SOUND_TYPE_MONO;
	TagHeadField.Audio.aacPacketType = NK_FLV_TAG_AUD_AAC_SEQ_HEAD;

	/// 判断是否需要发送 AAC Sequence Header.
	/// FIXME
	if (0 != memcpy(Privated->Audio.AAC.adts, raw, NK_ADTS_LEN()))
	{
		NK_Byte aac_dcr[16];
		NK_SSize aac_dcr_len = 0;

		/// 更新 ADTS。
		memcpy(Privated->Audio.AAC.adts, raw, NK_ADTS_LEN());
		/// 制作新的 AAC DCR。
		aac_dcr_len = NK_DCR_EncodeAAC(raw, aac_dcr, sizeof(aac_dcr));
		if (2 != aac_dcr_len)
		{
			/// TODO

		}

		/// 写入 TAG 数据。
		writen = Public->writeTag(Public, timestamp, &TagHeadField, aac_dcr, aac_dcr_len);
		if (writen < 0)
		{
			/// TODO 错误退出?
		}
	}

	/// 去掉 ADTS。
	raw += NK_ADTS_LEN();
	len -= NK_ADTS_LEN();

	/// 更新 TAG 数据结构头。
	TagHeadField.Audio.aacPacketType = NK_FLV_TAG_AUD_AAC_RAW;

	/// 写入 TAG 数据。
	writen = Public->writeTag(Public, timestamp, &TagHeadField, raw, len);

	return writen;
}

/**
 * 写入 G.711A 数据。
 */
static NK_SSize
FileFLV_writeG711a(THIS, NK_UInt64 ts_ms, NK_PByte data, NK_Size len)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);
	NK_FLVTagHeadField TagHeadField;
	NK_SSize writen = 0;

	NK_BZERO(&TagHeadField, sizeof(TagHeadField));
	MakeAudioTagHeadField(Public, &TagHeadField, len);
	TagHeadField.Audio.aacPacketType = 0; ///< 这个参数在 G.711 中用不到。

	TagHeadField.type = NK_FLV_TAG_TYPE_AUDIO;
	/// 写入 TAG 数据。
	writen = Public->writeTag(Public, ts_ms, &TagHeadField, data, len);
	return writen;
}


/**
 * 读取一个 TAG。
 */
static NK_SSize FileFLV_readTag(THIS, NK_FLVTagHeadField *TagHeadField, NK_PByte stack, NK_Size stack_len)
{
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);
	NK_FLVTagPreSize pre_size = 0;
	NK_SSize data_len = 0;
	NK_SSize read_len = 0;

	/**
	 * 参数合法性检查。
	 */
	NK_EXPECT_VERBOSE_RETURN_VAL(NK_Nil != stack, -1);
	NK_EXPECT_VERBOSE_RETURN_VAL(stack_len > 0, -1);

	/**
	 * 写入模式下不能读取。
	 */
	if (Privated->writer) {
		NK_Log()->error("FileFLV: File Read Only.");
		return -1;
	}

	/**
	 * 读取标签头域。
	 */
	if (0 != FileFLV_readTagHeadField(Public, TagHeadField, &data_len)) {
		NK_Log()->error("FileFLV: Read Tag Head Field.");
		return -1;
	}

	/**
	 * 判断栈区大小。
	 */
	if (stack_len < data_len) {
		NK_Log()->error("FileFLV: Stack Size( Data = %u, Stack = %u ) NOT Enougth.",
				data_len, stack_len);

		/**
		 * 跳过文件这个区域。
		 */
		fseek(Privated->fID, data_len, SEEK_CUR);
	}

	read_len = fread(stack, 1, data_len, Privated->fID);
	if (read_len != data_len) {
		if(feof(Privated->fID)){
			return -2;
		}
		NK_Log()->error("FileFLV: Read Data Failed.");
		return -1;
	}

	if (0 != FileFLV_readPreviousTagSize(Public, &pre_size)) {
		NK_Log()->error("FileFLV: Read Data Failed.");
		return -1;
	}

	/**
	 * 判断 Previous Tag Size 是否正确。
	 */
	if (pre_size != NK_FLV_TAG_SIZE(TagHeadField)) {
		NK_Log()->info("Previous Tag Size %d/%d Error.", NK_FLV_TAG_SIZE(TagHeadField), pre_size);
		/// FIXME 回到上一次正确的位置？
		return -1;
	}

//	NK_Log()->info("Previous Tag Size: %d", pre_size);
//	NK_Log()->info("Raw Data Length: %d", len);

	return data_len;
}

/**
 * THIS 指针定义解除，\n
 * 定义以上为模块 API 接口实现。
 */
#undef THIS

static NK_FileFLV *
FileFLV_Create(NK_Allocator *Alloctr, const NK_PChar filePath)
{
	NK_PrivatedFileFLV *Privated = NK_Nil;
	NK_FileFLV *Public = NK_Nil;

	if (!Alloctr) {
		Alloctr = NK_MemAlloc_OS();
	}

	// 初始化句柄。
	// 公有句柄的内存空间仅靠私有句柄的高位。
	// 有效防止模块公有句柄在外部被意外释放。
	Privated = Alloctr->alloc(Alloctr, sizeof(NK_PrivatedFileFLV) + sizeof(NK_FileFLV));
	Public = (NK_FileFLV *) (Privated + 1);

	/// 初始化私有成员。
	Privated->Alloctr = Alloctr;
	NK_BZERO(&Privated->FileHeadField, sizeof(Privated->FileHeadField));
	Privated->fID = NK_Nil;
	snprintf(Privated->magic, sizeof(Privated->magic), "%s", "OBJ");

	/// 初始化公有成员。
	Public->Object.name			= Object_name;
	Public->Object.dump			= Object_dump;
	Public->Object.threadsafe	= Object_threadsafe;
	Public->Object.free			= Object_free;
	Public->size				= FileFLV_size;
	Public->getFileHeadField 	= NK_Nil;
	Public->setVideoAttr 		= FileFLV_setVideoAttr;
	Public->setAudioAttr 		= FileFLV_setAudioAttr;
	Public->writeTag			= FileFLV_writeTag;
	Public->writeH264			= FileFLV_writeH264;
	Public->writeAACRaw			= FileFLV_writeAACRaw;
	Public->writeG711a			= FileFLV_writeG711a;
	Public->readTag				= FileFLV_readTag;

	/// 返回公有句柄。
	return Public;
}

NK_FileFLV *
NK_FileFLV_Create(NK_Allocator *Alloctr, const NK_PChar filePath)
{
	NK_FileFLV *Public = FileFLV_Create(Alloctr, filePath);
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);

	if (!Public)
	{
		goto NK_FileFLV_Create_err1;
	}

	/// 其他初始化操作。
	Privated->writer = NK_True;
	Privated->FileHeadField.signature[0] = 'F';
	Privated->FileHeadField.signature[1] = 'L';
	Privated->FileHeadField.signature[2] = 'V';
	Privated->FileHeadField.version = 0;
	Privated->FileHeadField.hasAudio = 0;
	Privated->FileHeadField.hasVideo = 0;
	Privated->FileHeadField.reserved = 0;
	Privated->FileHeadField.reserved2 = 0;
	Privated->FileHeadField.dataOffset = sizeof(Privated->FileHeadField.spacing)
			- sizeof(Privated->FileHeadField.zero);
	Privated->FileHeadField.dataOffset = NK_HTON32(Privated->FileHeadField.dataOffset);

	/// FIXME 随便写一个数值。
	Privated->Meta.duration = 0;
	Privated->Meta.width = 1280;
	Privated->Meta.height = 720;
	Privated->Meta.videoDataRate = 4000;
	Privated->Meta.frameRate = 12;
	Privated->Meta.videoCodecID = 7;
	Privated->Meta.audioDataRate = 96;
	Privated->Meta.audioCodecID = 2;
	Privated->Meta.canSeekToEnd = 1;

	/// 初始化视频索引
//	Privated->Video.n_indexs = 0;
//	Privated->Video.indexs = calloc(sizeof(Privated->Video.indexs[0])
//			* (Privated->Video.n_indexs / 1024 + 1)
//			* 1024, 1);

	/// 打开文件描述符。
	Privated->fID = fopen(filePath, "wb");
	if (!Privated->fID) {
		char cmd[60]={0};
		//NK_Log()->error("Flv File: Create Flv File(\"%s\") Failed.", filePath);
		printf("Flv File: Create Flv File(\"%s\") Failed.\n", filePath);
		sprintf(cmd, "rm %s -rf", filePath);
		system(cmd);
		goto NK_FileFLV_Create_err2;
	}

	/// 更新文件头。
	if (0 != FileFLV_putFileHeader(Public))
	{
		//NK_Log()->error("Flv File: Put Flv File(\"%s\") Header Failed.", filePath);
		printf("Flv File: Put Flv File(\"%s\") Header Failed.\n", filePath);
		char cmd[60]={0};
		sprintf(cmd, "rm %s -rf", filePath);
		system(cmd);
		goto NK_FileFLV_Create_err2;
	}

	/// 返回公有句柄。
	return Public;

NK_FileFLV_Create_err2:

	NK_FileFLV_Free(&Public);
	return NK_Nil;
NK_FileFLV_Create_err1:

	return NK_Nil;
}

extern NK_FileFLV *
NK_FileFLV_Open(NK_Allocator *Alloctr, const NK_PChar filePath)
{
	NK_FileFLV *Public = FileFLV_Create(Alloctr, filePath);
	NK_PrivatedFileFLV *Privated = PRIVATED(Public);

	if (!Public)
	{
		goto NK_FileFLV_Open_err1;
	}

	/// 其他初始化操作。
	Privated->writer = NK_False;

	NK_BZERO(&Privated->FileHeadField, sizeof(Privated->FileHeadField));
	Privated->fID = fopen(filePath, "rb");
	if (!Privated->fID)
	{
		NK_Log()->error("Flv File: Open Flv File(\"%s\") Failed.", filePath);
		goto NK_FileFLV_Open_err2;
	}

	if (FileFLV_readFileHeader(Public) < 0)
	{
		NK_Log()->error("Flv File: Read Flv File(\"%s\") Header Failed.", filePath);
		goto NK_FileFLV_Open_err2;
	}

	NK_FLV_HEAD_FIELD_DUMP(&Privated->FileHeadField);

	return Public;

NK_FileFLV_Open_err2:

	NK_FileFLV_Free(&Public);

NK_FileFLV_Open_err1:

	return NK_Nil;
}

/**
 * 销毁 FileFLV 模块句柄。
 */
NK_Int
NK_FileFLV_Free(NK_FileFLV **FileFLV_r)
{
	NK_FileFLV *Public = NK_Nil;
	NK_PrivatedFileFLV *Privated = NK_Nil;
//	NK_FLVTagHeadField TagHeadField;
//	int i = 0;

	/// 无效句柄。
	NK_ASSERT(NK_Nil != FileFLV_r && NK_Nil != FileFLV_r[0]);

	/// 获取私有句柄。
	Public = FileFLV_r[0];
	Privated = PRIVATED(Public);
	FileFLV_r[0] = NK_Nil;    ///< 操作前先清空句柄防止上层用户在释放过程中意外调用。

//	/// 记录文件索引值在最后。
//	NK_BZERO(&TagHeadField, sizeof(TagHeadField));
//	TagHeadField.type = NK_FLV_TAG_TYPE_VIDEO;
//	TagHeadField.filter = NK_FLV_TAG_FILTER_NO_PRE_PROCESS;
//	TagHeadField.Video.frameType = NK_FLV_TAG_VID_FRAME_INFO_OR_COMMAND;
//	TagHeadField.reserved = 0;
//	TagHeadField.dataSize = 0;
//	TagHeadField.timestamp = 0;
//	TagHeadField.streamID = 0;
//	TagHeadField.Video.codecID = 0;
//
//	Public->writeTag(Public, &TagHeadField,
//			Privated->Video.indexs,
//			Privated->Video.n_indexs * sizeof(Privated->Video.indexs[0]));

	/// FIXME
//	if (NULL != Privated->Video.indexs)
//	{
//		free(Privated->Video.indexs);
//		Privated->Video.indexs = NULL;
//		Privated->Video.n_indexs = 0;
//	}

	/// 释放模块内部资源。
	if (NK_Nil != Privated->fID)
	{
		/// 更新一下文件头部数据结构和 Meta 数据结构。
		FileFLV_putFileHeader(Public);

		/// 关闭文件。
		fclose(Privated->fID);
		Privated->fID = NK_Nil;
	}

	/// 释放模块句柄。
	Privated->Alloctr->freep(Privated->Alloctr, Privated);
}

