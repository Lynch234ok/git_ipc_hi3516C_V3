/**
 *
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>
#include "flv_def.h"

#ifndef NK_FLV_FILE_H_
#define NK_FLV_FILE_H_
NK_CPP_EXTERN_BEGIN


/**
 * FlvFile ģ������
 */
typedef struct Nk_FileFLV {
#define THIS struct Nk_FileFLV *const

	/**
	 * ����ģ�顣
	 */
	NK_Object Object;

	/**
	 * ��ȡ FLV �ļ���С��
	 */
	NK_SSize
	(*size)(THIS);

    /**
     * ��ȡ�ļ�ͷ��
     */
    NK_Integer
    (*getFileHeadField)(THIS, NK_FLVFileHeadField *HeadField);

    /**
     * ������Ƶ���ԡ�
     *
     * @param[in]		width			��Ƶ��ȣ����� 0 ��ʾʹ��Ĭ�ϡ�
     * @param[in]		height			��Ƶ�߶ȣ����� 0 ��ʾʹ��Ĭ�ϡ�
     * @param[in]		framerate		��Ƶ֡�ʣ����� 0 ��ʾʹ��Ĭ�ϡ�
     * @param[in]		bitrate			���ʣ����� 0 ��ʾʹ��Ĭ�ϡ�
     *
     * @return		���óɹ����� 0�����򷵻� -1��
     */
    NK_Int
	(*setVideoAttr)(THIS, NK_FLVTagCodecID codec_id, NK_Size width, NK_Size height, NK_Size framerate, NK_Size bitrate);

    /**
     * ������Ƶ���ԡ�
     *
     * @return
     */
    NK_Int
	(*setAudioAttr)(THIS, NK_FLVTagSoundFormat sound_fmt, NK_Boolean stereo, NK_Int32 bitwidth, NK_UInt32 samplerate);



    /**
	 * д��һ�� TAG ���ݡ�
	 *
	 * @param[in]			ts_ms			��ǰ TAG ��������ʱ�������λ�����룩��
	 * @param[in,out]		TagHeadField	FLV �ļ��� TAG ͷ�����ݽṹ�������ģ�齫�������� dateSize��timestamp �� timestampEx ����������
	 * @param[in]			data			FLV �ļ��� TAG ���ݡ�
	 * @param[in]			len				FLV �ļ��� TAG ���ݳ��ȡ�
	 *
	 * @return				д�볤�ȡ�
	 *
	 */
    NK_SSize
	(*writeTag)(THIS, NK_UInt64 ts_ms, NK_FLVTagHeadField *TagHeadField, NK_PByte data, NK_Size len);

    /**
     * д��ͬһʱ�����һ�����߶�� AVC NALU ���ݡ�\n
     * �� @ref seekable ��ʶλ��Ч��ʱ�����ݱ���������б����������� PPS �� SPS NALU��\n
     * ���ڲ�������� @ref NK_FileFLV::writeTag ʵ��д�������
     *
     * @param[in]			ts_ms			��ǰ���ݵ�ʱ�������λ�����룩��
     * @param[in]			seekable		�ɼ�����ʶ�����ļ��ڲ���λ�йأ�һ���Ӧ�ؼ�֡���ݡ�
     * @param[in]			data			д���һ�����߶������ NALU ���ݡ�
     * @param[in]			len				���ݳ��ȡ�
     *
     *
     */
	NK_SSize
    (*writeH264)(THIS, NK_UInt64 ts_ms, NK_Boolean seekable, NK_PByte data, NK_Size len);

	/**
	 * д�� AAC Raw ���ݡ�\n
	 * ���ڲ�������� NK_FileFLV::writeTag ʵ��д�������
	 *
	 * @param[in]			timestamp		�����ݱ������ʱ��ʱ�������λ�����룩��
	 * @param[in]			stereo			�Ƿ�˫������ʶ��NK_True ��ʾ˫����������Ϊ��������
	 * @param[in]			bitwidth		��Ƶλ���ο� @ref NK_FLVTagHeadField::Audio::soundSize��
	 * @param[in]			sample			��Ƶ�����ʣ��ο� @ref NK_FLVTagHeadField::Audio::soundRate��
	 * @param[in]			raw				��Ƶ���ݣ���ҪЯ�� ADTS ��Ϣ��
	 * @param[in]			len				��Ƶ���ݵĳ��ȣ���λ���ֽڣ���
	 *
	 * @return				�ɹ�����д����Ƶ���ݵĳ��ȣ�ʧ�ܷ��� -1��
	 */
	NK_SSize
	(*writeAACRaw)(THIS, NK_UInt64 timestamp, NK_Boolean stereo, NK_Int32 bitwidth, NK_UInt32 sample, NK_PByte raw, NK_Size len);

	/**
	 *
	 * @param
	 * @param timestamp
	 * @param stereo
	 * @param bitwidth
	 * @param sample
	 * @param raw
	 * @param len
	 * @return
	 */
	NK_SSize
	(*writeG711a)(THIS, NK_UInt64 ts_ms, NK_PByte data, NK_Size len);

    /**
     * ��ȡһ�� TAG ���ݡ�
     */
    NK_SSize
    (*readTag)(THIS, NK_FLVTagHeadField *TagHeader, NK_PByte stack, NK_Size stack_len);


#undef THIS
} NK_FileFLV;


/**
 * ���� FlvFile ģ������\n
 * ������ʽ���ɾ��ֻ�߱�д���ܡ�
 */
extern NK_FileFLV *
NK_FileFLV_Create(NK_Allocator *Alloctr, NK_PChar file_path);

/**
 * �� FlvFile �ļ������ɾ����\n
 * ͨ���򿪷�ʽ�������֮�߱������ܡ�
 */
extern NK_FileFLV *
NK_FileFLV_Open(NK_Allocator *Alloctr, NK_PChar file_path);

/**
 * ���� FlvFile ģ������
 */
extern NK_Int
NK_FileFLV_Free(NK_FileFLV **FileFLV_r);


NK_CPP_EXTERN_END
#endif /* NK_FLV_FILE_H_ */
