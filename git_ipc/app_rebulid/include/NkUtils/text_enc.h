/**
 * 文本编码相关方法。
 *
 */

#include <NkUtils/types.h>

#if !defined(NK_UTILS_TEXT_ENC_H_)
#define NK_UTILS_TEXT_ENC_H_
NK_CPP_EXTERN_BEGIN

/**
 * 文本编码由 GB2312 转换成 UTF-8。
 * GB2312 向 UTF-8 中文的转换，会存在繁简体的问题，
 * 通过对 @ref text_cht 标识设置可以决定 GB2312 编码转换至 UTF-8 的中文简体或繁体。
 *
 * @param[in]			text_gb2312				GB2312 文本。
 * @param[in]			text_cht				中文繁体标识。
 * @param[out]			text_utf8				转换成功后 UTF-8 文本缓冲。
 * @param[in]			text_utf8_max			UTF-8 文本缓冲大小。
 *
 * @return		转换成功后返回 UTF-8 文本长度，失败返回 -1。
 *
 */
NK_API NK_SSize
NK_TextEnc_Gb2312ToUtf8(NK_PByte text_gb2312, NK_Boolean text_cht, NK_PByte text_utf8, NK_Size text_utf8_max);

/**
 * 文本编码由 UTF-8 转换成 GB2312。
 *
 * @param[in]			text_utf8				UTF-8 文本。
 * @param[out]			text_gb2312				转换成功后 GB2312 文本缓冲。
 * @param[in]			text_gb2312_max			GB2312 文本缓冲大小。
 *
 * @return		转换成功后返回 GB2312 文本长度，失败返回 -1。
 */
NK_API NK_SSize
NK_TextEnc_Utf8ToGb2312(NK_PChar text_utf8, NK_PChar text_gb2312, NK_Size text_gb2312_max);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_TEXT_ENC_H_ */
