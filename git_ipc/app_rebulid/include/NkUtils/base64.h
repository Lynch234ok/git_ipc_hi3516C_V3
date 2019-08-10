
/**
 * base64.h
 * Created: Nov 11, 2015
 * Author: frank
 */


#include <NkUtils/types.h>

#if !defined(NK_UTILS_BASE64_H_)
# define NK_UTILS_BASE64_H_
NK_CPP_EXTERN_BEGIN

/**
 * BASE64 ±àÂë¡£
 *
 */
extern NK_Int
NK_Base64_Encode(NK_PByte origin, NK_Size len, NK_PChar enc, NK_Size *enc_len);

/**
 * BASE64 ½âÂë¡£
 *
 */
extern NK_Int
NK_Base64_Decode(NK_PChar enc, NK_Size enc_len, NK_PByte origin, NK_Size *origin_len);

NK_CPP_EXTERN_END
#endif /* NK_UTILS_BASE64_H_ */
