#ifndef _SECURE_CHIP_H_
#define _SECURE_CHIP_H_

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {

    SECURE_CHIP_MD5,
    SECURE_CHIP_DATA_SN,
    SECURE_CHIP_DATA_UID,

} emSECURE_CHIP_DATA;

// During initialization, reads SN, key, UID and DID from secure chip and writes to CONF_FILENAME.
// Falls back to read from CONF_FILENAME if the secure chip is broken.
// Returns 0 on success. -1 if failed.
extern int SECURE_CHIP_init( void );

// Is Secure Chip read and write to conf succss
extern bool SECURE_CHIP_is_init_success( void );

// Get Secure Chip RevMark
// Returns 0 on success. not 0 if failed.
extern int SECURE_CHIP_get_revmark(int *out_revmark);

// Get Secure Data.
// Returns 0 on success. not 0 if failed.
extern int SECURE_CHIP_get_data(emSECURE_CHIP_DATA chip_data, char *out_buf, size_t out_buf_len);

#ifdef __cplusplus
}
#endif

#endif // _SECURE_CHIP_H_
