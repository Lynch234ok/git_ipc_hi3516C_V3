#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <secure_chip.h>
#include <md5sum.h>
#include <app_debug.h>
#include "securedat.h"


typedef struct {
    uint32_t RandSeed[4];
    uint32_t RevMark;
    uint32_t RevSize;
    uint32_t MdlMark[2];
    uint32_t ODMMark[2];
    uint8_t UserMac[8];
    int8_t SNumber[32];
    int8_t UserKey[32];
    int8_t UserUid[32];
    int8_t UserDid[32];
    uint32_t Crc32;
} stSCHIP_EEPROM_DATA, *lpSCHIP_EEPROM_DATA;


static bool gs_sdata_read_success = false;

static char gs_schip_md5[33];
static unsigned int gs_sdata_revmark = 0;
static char gs_sdata_sn[33];
static char gs_sdata_uid[33];


// Reads SN, key, UID and DID from secure chip.
// Returns true on success. false if failed.
static bool secure_chip_read_from_chip(void)
{
    uint8_t eeprom_data_buf[256];
    int eeprom_bytes_read;
    size_t md5_buf_len;
    int n;
    lpSCHIP_EEPROM_DATA eeprom_data_struct;
    bool data_ok = true;
    char *cal_md5_buf = NULL;
    int data_ver = 0x00000100;

    if(UC_SNgetDecipherData((char *)eeprom_data_buf, &eeprom_bytes_read) != 0)
    {
        APP_TRACE("Cannot read from secure chip.");
        return false;
    }

    eeprom_data_struct = (lpSCHIP_EEPROM_DATA)eeprom_data_buf;

    if (0x00000200 == eeprom_data_struct->RevMark || 
        (0x00000100 == eeprom_data_struct->RevMark && eeprom_data_struct->RevSize == 160)){
        APP_TRACE("data is ver2.");
        data_ver = 0x00000200;
    }

    // 内容长度为 0 就认为没有这个字段
    if (0 == eeprom_data_struct->SNumber[0]) {
        APP_TRACE("%s:%d no SN in secure chip!", __FUNCTION__, __LINE__);
        data_ok = false;
    }

    if (data_ver == 0x00000200 && 0 == eeprom_data_struct->UserUid[0]) {
        APP_TRACE("%s:%d no UID in secure chip!", __FUNCTION__, __LINE__);
        data_ok = false;
    }

    if (!data_ok) {
        return false;
    }

    // 计算加密片 md5
    cal_md5_buf = md5sum_buffer(eeprom_data_buf, sizeof(eeprom_data_buf));
    if (NULL == cal_md5_buf) {
        APP_TRACE("%s:%d Failed to calculate md5!!", __FUNCTION__, __LINE__);
        return false;
    }
    md5sum_to_upper(cal_md5_buf, (int)strlen(cal_md5_buf));

    md5_buf_len = sizeof(gs_schip_md5);
    n = snprintf(gs_schip_md5, md5_buf_len, "%s", cal_md5_buf);
    if (n >= (int)md5_buf_len || n < 0) {
        APP_TRACE("%s:%d snprintf for md5 error!! ret: %d", __FUNCTION__, __LINE__, n);
        return false;
    }
    gs_schip_md5[32] = '\0';

    gs_sdata_revmark = eeprom_data_struct->RevMark;

    memcpy(gs_sdata_sn, eeprom_data_struct->SNumber, sizeof(eeprom_data_struct->SNumber));
    if(data_ver == 0x00000200){
        memcpy(gs_sdata_uid, eeprom_data_struct->UserUid, sizeof(eeprom_data_struct->UserUid));
    }
    else{
        memset(gs_sdata_uid, 0, sizeof(gs_sdata_uid));
    }
    gs_sdata_sn[32] = '\0';
    gs_sdata_uid[32] = '\0';

    return true;
}

// During initialization, reads SN, key, UID and DID from secure chip and writes to CONF_FILENAME.
// Falls back to read from CONF_FILENAME if the secure chip is broken.
// Returns 0 on success. -1 if failed.
int SECURE_CHIP_init( void )
{
    // if secure chip already init success, not init it again
    if (gs_sdata_read_success) {
        return 0;
    }

    if(secure_chip_read_from_chip() == true) {
        gs_sdata_read_success = true;
        APP_TRACE("Secure chip init success!");
        return 0;

    } else {
        gs_sdata_read_success = false;
        APP_TRACE("Secure chip init fail!");
        return -1;
    }
}

// Is Secure Chip read and write to conf succss
bool SECURE_CHIP_is_init_success( void )
{
    return gs_sdata_read_success;
}

// Get Secure Chip RevMark
// Returns 0 on success. not 0 if failed.
int SECURE_CHIP_get_revmark(int *out_revmark)
{
    if (!gs_sdata_read_success) {
        APP_TRACE("%s: Secure Chip was not read success!", __FUNCTION__);
        return -1;
    }

    *out_revmark = gs_sdata_revmark;

    return 0;
}

// Get Secure Data. Format of mac is like: 00:00:00:00:00:00
// Returns 0 on success. not 0 if failed.
int SECURE_CHIP_get_data(emSECURE_CHIP_DATA chip_data, char *out_buf, size_t out_buf_len)
{
    int n;
    char *data_str = NULL;

    if (!gs_sdata_read_success) {
        APP_TRACE("%s: Secure Chip was not read success!", __FUNCTION__);
        return -1;
    }

    if (NULL == out_buf) {
        APP_TRACE("%s: Out buffer can't be NULL!", __FUNCTION__);
        return -1;
    }

    switch (chip_data) {
        case SECURE_CHIP_DATA_SN:
            data_str = gs_sdata_sn;
            break;

        case SECURE_CHIP_DATA_UID:
            data_str = gs_sdata_uid;
            break;

        case SECURE_CHIP_MD5:
            data_str = gs_schip_md5;
            break;

        default:
            data_str = NULL;
    }

    if (NULL == data_str) {
        APP_TRACE("%s:%d Can't read such data! data enum: %d", __FUNCTION__, __LINE__, (int)chip_data);
        return -1;
    }

    n = snprintf(out_buf, out_buf_len, "%s", data_str);
    if (n >= (int)out_buf_len || n < 0) {
        APP_TRACE("%s:%d snprintf for read data error! ret: %d", __FUNCTION__, __LINE__, n);
        return -1;
    }
    return 0;
}