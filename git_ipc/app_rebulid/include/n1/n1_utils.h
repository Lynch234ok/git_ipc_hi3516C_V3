/*
 *  N1 协议数处理抽象层。
 *
 */

#include <NkUtils/n1_def.h>

#ifndef NK_UTILS_N1_UTILS_H_
#define NK_UTILS_N1_UTILS_H_
NK_CPP_EXTERN_BEGIN

/**
 * 打印 N1 协议图标。
 *
 *           nnnn
 *          nnnn                    nn     nn nnnn    11
 *         nnnn  nnnnnnn          nnn      nnn   nn  111
 *        nnnn nnnnnnnnnnn      nnnn       nn    nn   11
 *     nnnnnnnn       nnnn    nnnnn        nn    nn   11
 *   nnnnnnn         nnnn      nnn         nn    nn  1111
 *     nnnn         nnnn      nnn
 *    nnnn         nnnn      nnn
 *   nnnn         nnnn      nnn
 *  nnnn         nnnnnnnnnnnnn
 *
 */
#define NK_N1_LOGO_PATTERN(__ver_maj, __ver_min, __ver_rev, __rel_yyyy, __rel_mm, __rel_dd) \
	do{\
		printf("\r\n");\
		printf("\033[40;30m");\
		printf("                                                                               \r\n");\
		printf("            \033[41;31mnnnn\033[40;30mn                                                           \r\n");\
		printf("           \033[41;31mnnnn\033[40;30mn                  \033[41;31mnn\033[40;30mn    \033[47;37mnn\033[40;30mn\033[47;37mnnnn\033[40;30mn   \033[47;37m11\033[40;30m1                      \r\n");\
		printf("          \033[41;31mnnnn\033[40;30mn \033[41;31mnnnnnn\033[40;30mn         \033[41;31mnnn\033[40;30mn     \033[47;37mnnn\033[40;30mn  \033[47;37mnn\033[40;30mn \033[47;37m111\033[40;30m1                      \r\n");\
		printf("         \033[41;31mnnnn\033[40;30mn\033[41;31mnnnnnnnnnn\033[40;30mn     \033[41;31mnnnn\033[40;30mn      \033[47;37mnn\033[40;30mn   \033[47;37mnn\033[40;30mn  \033[47;37m11\033[40;30m1                      \r\n");\
		printf("      \033[41;31mnnnnnnnn\033[40;30mn     \033[41;31mnnnn\033[40;30mn   \033[41;31mnnnnn\033[40;30mn       \033[47;37mnn\033[40;30mn   \033[47;37mnn\033[40;30mn  \033[47;37m11\033[40;30m1                      \r\n");\
		printf("    \033[41;31mnnnnnnn\033[40;30mn       \033[41;31mnnnn\033[40;30mn     \033[41;31mnnn\033[40;30mn        \033[47;37mnn\033[40;30mn   \033[47;37mnn\033[40;30mn \033[47;37m1111\033[40;30m1                     \r\n");\
		printf("      \033[41;31mnnnn\033[40;30mn       \033[41;31mnnnn\033[40;30mn     \033[41;31mnnn\033[40;30mn                                            \r\n");\
		printf("     \033[41;31mnnnn\033[40;30mn       \033[41;31mnnnn\033[40;30mn     \033[41;31mnnn\033[40;30mn          \033[40;37mVersion \033[4m%d.%d.%d\033[24m                    \r\n", __ver_maj, __ver_min, __ver_rev);\
		printf("    \033[41;31mnnnn\033[40;30mn       \033[41;31mnnnn\033[40;30mn     \033[41;31mnnn\033[40;30mn           \033[40;37mDate \033[4m%04u/%02u/%02u\033[24m                    \r\n", __rel_yyyy, __rel_mm, __rel_dd);\
		printf("   \033[41;31mnnnn\033[40;30mn       \033[41;31mnnnnnnnnnnnnn\033[40;30mn                                               \r\n");\
		printf("                                                                                           \r\n");\
		printf("\033[0m");\
		printf("\r\n");\
	} while(0)


/**
 * 生成 SDP 帧。\n
 * 在 Live 会话中必须先发送此帧，客户端才能有效解析媒体数据。\n
 *
 * @param Session
 * @return
 */
extern int
NK_N1Utils_MakeSDPFrame(NK_N1LiveSession *Session,
		NK_PByte sdp, NK_Size stack_len);

/**
 *
 *
 * @param Session
 * @param ts_ms
 * @param[in,out]		DataFrame		数据
 * @return
 */
extern int
NK_N1Utils_MakeDataFrame(NK_N1LiveSession *Session, NK_N1DataPayload payload_type, NK_UInt32 ts_ms, NK_N1DataFrame *DataFrame);


/**
 * 通过 ESSID 检测对应热点是否为 NVR。
 */
extern NK_Boolean
NK_N1Utils_IsHotSpotOfNVR(NK_PChar essid);



/**
 * 通过 NVR 无线热点 ESSID 获取热点密码。\n
 * 内部会调用 @ref NK_N1Utils_IsHotSpotOfNVR() 方法。
 *
 * @param[in]		essid						NVR 无线热点 ESSID 名称。
 * @param[out]		psk							NVR 无线热点密码。
 * @param[in,out]	psk_size					传入密码栈区大小，传出无线热点密码长度。
 *
 * @return		获取热点密码成功返回 0，失败返回 -1。
 */
extern NK_Int
NK_N1Utils_CrackWiFiNVRPSK(NK_PChar essid, NK_PChar psk, NK_Size *psk_size);

/**
 * 更正 @ref LanSetup 数据结构内容。
 *
 */
extern NK_Int
NK_N1Utils_CorrectLanSetup(NK_N1LanSetup *LanSetup);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_N1_UTILS_H_ */
