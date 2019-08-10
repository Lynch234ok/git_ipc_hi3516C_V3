
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SDK_SYS_H_
#define SDK_SYS_H_
#ifdef __cplusplus
extern "C" {
#endif

#define SDK_HI3518A_V100 (0x3518a100)
#define SDK_HI3518C_V100 (0x3518c100)
#define SDK_HI3518E_V100 (0x3518e100)
#define SDK_HI3516C_V100 (0x3516c100)
#define SDK_HI3516A_V100 (0x3516a100)
#define SDK_HI3518E_V200 (0x3518e200)
#define SDK_HI3516C_V200 (0x3516c200)
#define SDK_M388C1G    		(0x388c2)
#define SDK_HI3516E_V100 (0x3516e100)


typedef struct SDK_SYS_API {

	int (*read_reg)(uint32_t reg_addr, uint32_t *val32);
	int (*write_reg)(uint32_t reg_addr, uint32_t val32);
	int (*read_mask_reg)(uint32_t reg_addr, uint32_t mask32, uint32_t *val32);
	int (*write_mask_reg)(uint32_t reg_addr, uint32_t mask32, uint32_t val32);

	float (*temperature)();
	uint32_t (*get_chip_id)(uint32_t *chip_id);
	
}stSDK_SYS_API, *lpSDK_SYS_API;

extern lpSDK_SYS_API sdk_sys;
extern int SDK_init_sys(const char* solution);
extern int SDK_destroy_sys();

#ifdef __cplusplus
};
#endif
#endif //SDK_SYS_H_

