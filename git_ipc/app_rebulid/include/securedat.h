
#ifndef __SECUREDAT_H__
#define __SECUREDAT_H__
#ifdef __cplusplus
extern "C" {
#endif

#define UC_SN_MAX_LEN (32)
#define UC_SD_BLK_SIZ (256)

enum{
	UC_ENCRYP_CHIP_TYPE_NONE = 0,
	UC_ENCRYP_CHIP_TYPE_AT88,
	UC_ENCRYP_CHIP_TYPE_LIC2,
	UC_ENCRYP_CHIP_TYPE_24C02,
	UC_ENCRYP_CHIP_TYPE_CNT,
};

extern int UC_SNumberInit(unsigned int Sgn1, unsigned int Sgn2);
extern int UC_SNumberSet(const char *sn, int len);
extern int UC_SNumberGet(char* sn);
extern int UC_SNumberChk(void);
extern int UC_SNAuthenChk(void);
extern int UC_SNumberExit(void);
extern int UC_SNGetODMNumber(unsigned int *odm1, unsigned int *odm2);
extern int UC_SNgetData(char *ret_data, int *len);
extern int UC_SNgetDecipherData(char *ret_data, int *len);

#ifdef __cplusplus
};
#endif
#endif //__SECUREDAT_H__

