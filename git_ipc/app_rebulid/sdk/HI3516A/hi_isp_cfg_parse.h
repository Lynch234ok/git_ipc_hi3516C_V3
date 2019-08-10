
#ifndef HI_ISP_CFG_PARSE_H_
#define HI_ISP_CFG_PARSE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern int HI_ISP_cfg_ini_parse_init(void);
extern int HI_ISP_cfg_ini_parse_destroy(void);
extern int HI_ISP_cfg_ini_load(const char *filepath, void *args, size_t size);
extern int HI_ISP_cfg_ini_save(const char *filePath, void *args, size_t size);

#ifdef __cplusplus
};
#endif
#endif //HI_ISP_CFG_PARSE_H_

