/**
 * NK Utils 库修订版本定义。
 */

#include <NkUtils/types.h>

#if !defined(NK_CIPHER_REVISION_H_)
# define NK_CIPHER_REVISION_H_
NK_CPP_EXTERN_BEGIN

/**
 * 修订版本号。
 */
#define NK_CIPHER_REVISION	(5U)

/**
 * @brief
 *  获取库版本号。\n
 * @details
 *  在库该方法实现内部直接返回宏定义的值，\n
 *  由于库发布时二进制已经确认，因此可以过在链接改库时判断此方法返回结果是否等于判断头文件是否与二进制库文件匹配。\n
 *  通过次方法可以有效防止由于头文件与二进制文件不匹配所带来的致命问题。
 *
 * @return
 * 	返回库的版本号，与宏义一致。\n
 */
extern NK_UInt32
NK_Cipher_LibraryRevision();


NK_CPP_EXTERN_END
#endif /* NK_CIPHER_REVISION_H_ */




