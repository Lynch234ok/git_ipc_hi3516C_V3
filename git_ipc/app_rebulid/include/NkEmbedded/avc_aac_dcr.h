/**
 * AVC Decoder Configuration Record ���¼�� DCR�� ���ɽӿڡ�
 */

#include <NkUtils/types.h>

#ifndef JAM_AVC_AAC_DCR_H_
#define JAM_AVC_AAC_DCR_H_
NK_CPP_EXTERN_BEGIN

/**
 * ͨ�������������� SPS �� PPS �� NALU ���� AVC DCR��
 *
 * @param[in]		n_sps				SPS NALU ���������ɴ˲������� @ref sps �� @ref sps_len �����ĸ�����
 * @param[in]		sps					SPS NALU �������ݼ��ϡ�
 * @param[in]		sps_len				SPS NALU �ĳ��ȼ��ϡ�
 * @param[in]		n_pps				PPS NALU ���������ɴ˲������� @ref pps �� @ref pps_len �����ĸ�����
 * @param[in]		pps					PPS NALU �������ݼ��ϡ�
 * @param[in]		pps_len				PPS NALU �ĳ��ȼ��ϡ�
 * @param[in]		dcr					���� DCR ���ݡ�
 * @param[in,out]	dcr_len				����ջ�������С������ DCR ���ȡ�
 *
 * @return			�����ɹ����� AVC DCR �ĳ��ȣ�ʧ�ܷ��� -1��
 *
 */
extern NK_Int
NK_DCR_EncodeAVC(NK_Size n_sps, NK_PByte *sps, NK_Size *sps_len,
		NK_Size n_pps, NK_PByte *pps, NK_Size *pps_len,
		NK_PByte dcr, NK_Size *dcr_len);

/**
 *
 * �ڶ�� NALU �г���� DCR��\n
 * NALU �б���������������������� SPS �� PPS ���ݡ�\n
 * �ڲ����� @ref NK_DCR_EncodeAVC()��
 *
 * @param[in]		nalus				��� NALU ���������ݿ顣
 * @param[in]		len					NALU ���ݿ鳤�ȡ�
 * @param[in]		dcr					���� DCR ���ݡ�
 * @param[in,out]	dcr_len				����ջ�������С������ DCR ���ȡ�
 *
 * @return		�ɹ����� 0������ @ref dcr �� @ref dcr_len �л�ȡ DCR ���ݣ�ʧ�ܷ��� -1��
 */
extern NK_Int
NK_DCR_ExtractAVC(NK_PByte nalus, NK_Size len, NK_PByte dcr, NK_Size *dcr_len);


/**
 * ͨ�������������� ADTS ���� AAC DCR��
 *
 * @param[in]		adts				ADTS ���������ݡ�
 * @param[out]		stack				�������ݵ��ڴ�ռ䡣
 * @param[in]		stack_len			�������ݵ��ڴ�ռ��С��������� 2 ���ֽڡ�
 * @return
 */
extern NK_SSize
NK_DCR_EncodeAAC(NK_PByte adts, NK_PByte stack, NK_Size stack_len);

/**
 * ADTS �ĳ��ȡ�
 */
#define JAM_ADTS_LEN() (7)

/**
 * ����Ƿ�Ϊ ADTS �жϺꡣ
 */
static inline NK_Boolean
JAM_IS_ADTS(NK_PByte buf)
{
	return (0xff == buf[0] && 0xf0 == (buf[1] & 0xf0)) ? NK_True : NK_False;
}


NK_CPP_EXTERN_END
#endif /* JAM_AVC_AAC_DCR_H_ */
