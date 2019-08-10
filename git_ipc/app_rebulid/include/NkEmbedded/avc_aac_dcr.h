/**
 * AVC Decoder Configuration Record （下简称 DCR） 生成接口。
 */

#include <NkUtils/types.h>

#ifndef JAM_AVC_AAC_DCR_H_
#define JAM_AVC_AAC_DCR_H_
NK_CPP_EXTERN_BEGIN

/**
 * 通过编码器产生的 SPS 和 PPS 的 NALU 制作 AVC DCR。
 *
 * @param[in]		n_sps				SPS NALU 的数量，由此参数决定 @ref sps 和 @ref sps_len 参数的个数。
 * @param[in]		sps					SPS NALU 数据内容集合。
 * @param[in]		sps_len				SPS NALU 的长度集合。
 * @param[in]		n_pps				PPS NALU 的数量，由此参数决定 @ref pps 和 @ref pps_len 参数的个数。
 * @param[in]		pps					PPS NALU 数据内容集合。
 * @param[in]		pps_len				PPS NALU 的长度集合。
 * @param[in]		dcr					返回 DCR 数据。
 * @param[in,out]	dcr_len				传入栈区缓冲大小，返回 DCR 长度。
 *
 * @return			制作成功返回 AVC DCR 的长度，失败返回 -1。
 *
 */
extern NK_Int
NK_DCR_EncodeAVC(NK_Size n_sps, NK_PByte *sps, NK_Size *sps_len,
		NK_Size n_pps, NK_PByte *pps, NK_Size *pps_len,
		NK_PByte dcr, NK_Size *dcr_len);

/**
 *
 * 在多个 NALU 中抽离出 DCR，\n
 * NALU 中必须包含编码器产生的所有 SPS 和 PPS 数据。\n
 * 内部调用 @ref NK_DCR_EncodeAVC()。
 *
 * @param[in]		nalus				多个 NALU 的连续数据块。
 * @param[in]		len					NALU 数据块长度。
 * @param[in]		dcr					返回 DCR 数据。
 * @param[in,out]	dcr_len				传入栈区缓冲大小，返回 DCR 长度。
 *
 * @return		成功返回 0，并从 @ref dcr 和 @ref dcr_len 中获取 DCR 数据，失败返回 -1。
 */
extern NK_Int
NK_DCR_ExtractAVC(NK_PByte nalus, NK_Size len, NK_PByte dcr, NK_Size *dcr_len);


/**
 * 通过编码器产生的 ADTS 制作 AAC DCR。
 *
 * @param[in]		adts				ADTS 的数据内容。
 * @param[out]		stack				返回数据的内存空间。
 * @param[in]		stack_len			返回数据的内存空间大小，必须大于 2 个字节。
 * @return
 */
extern NK_SSize
NK_DCR_EncodeAAC(NK_PByte adts, NK_PByte stack, NK_Size stack_len);

/**
 * ADTS 的长度。
 */
#define JAM_ADTS_LEN() (7)

/**
 * 检查是否为 ADTS 判断宏。
 */
static inline NK_Boolean
JAM_IS_ADTS(NK_PByte buf)
{
	return (0xff == buf[0] && 0xf0 == (buf[1] & 0xf0)) ? NK_True : NK_False;
}


NK_CPP_EXTERN_END
#endif /* JAM_AVC_AAC_DCR_H_ */
