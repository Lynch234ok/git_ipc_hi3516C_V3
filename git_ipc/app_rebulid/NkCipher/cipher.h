

#include <NkUtils/types.h>

#if !defined(NK_CIPHER_CIPHER_H_)
#define NK_CIPHER_CIPHER_H_
NK_CPP_EXTERN_BEGIN

/**
 * @brief
 *  提取 EEPROM 数据。
 * @details
 *
 * @param eeprom [in]
 *  EEPROM 数据的内存起始地址。
 * @param eepromsz [in]
 *  EEPROM 数据大小。
 * @param mask0 [in]
 *  掩码 0。
 * @param mask1 [in]
 * @param stack [out]
 *  返回栈区内存起始地址。
 * @param stacklen [in]
 *  栈区大小。
 *
 * @return
 *  返回从 EEPROM 提取的字段。
 */
NK_API NK_PChar
NK_Cipher_ExtractE2PROM(NK_PByte eeprom, NK_Size eepromsz, NK_UInt32 mask0, NK_UInt32 mask1, NK_PChar stack, NK_Size stacklen);

/**
 * @macro
 *  方法别名。
 */
#define NK_Cipher_ExtractEEPROM NK_Cipher_ExtractE2PROM

NK_CPP_EXTERN_END
#endif /* NK_CIPHER_CIPHER_H_ */

