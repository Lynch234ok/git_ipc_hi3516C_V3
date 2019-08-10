
#include <NkUtils/types.h>

#if !defined(NK_N1_LICENSE_H_)
# define NK_N1_LICENSE_H_
NK_CPP_EXTERN_BEGIN


/**
 * @macro
 *  用户凭证版本号。
 */
#define NK_LICENSE_VERSION  (3)

/**
 * @macro
 */
#define NK_LICENSE_MAX_SIZE (1024 * 16)

/**
 * @brief
 *  用户许可数据结构。
 */
typedef struct Nk_N1License {

	/**
	 * 对应 SDK 版本号。
	 */
	NK_UInt32 version;

	/**
	 * 生成日期。
	 */
	NK_Char date[128];

	/**
	 * 客户名称。
	 */
	NK_Char user[128];

	/// 设备过滤规则。
	struct {

		/// 过滤关键字。
		NK_Char keyword[128];

		/// 输入过滤有效。
		NK_Boolean income;

		/// 输出过滤有效。
		NK_Boolean outcome;

	} DiscoveryFilter;

	/**
	 * 对应标签 AllowRJ45，\n
	 * 缺省 True
	 */
	NK_Boolean allowRJ45;

	/**
	 * 对应标签 AllowWiFiStation，\n
	 * 缺省 True。
	 */
	NK_Boolean allowWiFiStation;

	/**
	 * 对应标签 AllowWiFiAP，\n
	 * 缺省 False。
	 */
	NK_Boolean allowWiFiAP;

	/**
	 * 对应标签 AllowWiFiRepeater，\n
	 * 缺省 False。
	 */
	NK_Boolean allowWiFiRepeater;

	/**
	 * 对应标签 HasHinet，\n
	 * 缺省 True。
	 */
	NK_Boolean hasHinet;

	/**
	 * 对应标签 HasRTSP，\n
	 * 缺省 False。
	 */
	NK_Boolean hasRTSP;

	/**
	 * 读应标签 HasHikvision，\n
	 * 缺省 False。
	 */
	NK_Boolean hasHikvision;

	/**
	 * 读应标签 HasCorsee，\n
	 * 缺省 False。
	 */
	NK_Boolean hasCorsee;

	/**
	 * 只用于调试标识。
	 */
	NK_Boolean debugonly;

} NK_N1License;

/**
 * @macro
 *  打印证书内容。
 */
#define NK_N1_LICENSE_DUMP(__Lic) \
	do {\
		NK_TermTable Table;\
		NK_TermTbl_BeginDraw(&Table, "License Maker Options", 64, 4);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Version", "%u", (__Lic)->version);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Date", "%s", (__Lic)->date);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "User", "%s", (__Lic)->user);\
		NK_TermTbl_PutText(&Table, NK_True, "Discovery Filter");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Keyword", "%s", (__Lic)->DiscoveryFilter.keyword);\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Income", "%s", (__Lic)->DiscoveryFilter.income ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Outcome", "%s", (__Lic)->DiscoveryFilter.outcome ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Allow RJ-45", "%s", (__Lic)->allowRJ45 ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Allow Wi-Fi Station", "%s", (__Lic)->allowWiFiStation ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Allow Wi-Fi AP", "%s", (__Lic)->allowWiFiAP ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Allow Wi-Fi Repeater", "%s", (__Lic)->allowWiFiRepeater ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "With Hinet", "%s", (__Lic)->hasHinet ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "With RTSP", "%s", (__Lic)->hasRTSP ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "With Hikvision", "%s", (__Lic)->hasHikvision ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "With Corsee", "%s", (__Lic)->hasCorsee ? "Yes" : "No");\
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Debug Only", "%s", (__Lic)->debugonly ? "Yes" : "No");\
		NK_TermTbl_EndDraw(&Table);\
	} while (0)

/**
 * @brief
 *  解析来自明文文本的证书。
 */
extern NK_Int
NK_N1License_ParseText(NK_PChar text, NK_N1License *License);

/**
 * @brief
 *  生成证书的明文文本。
 */
extern NK_SSize
NK_N1License_ProduceText(NK_N1License *License, NK_PChar text, NK_Size text_max);

/**
 * @brief
 *  解析 N1 用户许可证。
 *
 * @param[in] license
 *  传入用户证书的内存地址。
 *
 * @param[out] License
 *  用户证书数据结构。
 *
 * @retval -1
 *  解析证书失败，可能证书不合法。
 *
 * @retval 0
 *  解析证书成功，从 @ref License 获取证书内容。
 *
 */
extern NK_Int
NK_N1License_Parse(NK_PByte license, NK_N1License *License);

/**
 * @brief
 *  制作 N1 用户许可证。
 */
extern NK_SSize
NK_N1License_Produce(NK_N1License *License, NK_PByte license, NK_Size license_max);


NK_CPP_EXTERN_END
#endif /* NK_N1_LICENSE_H_ */

