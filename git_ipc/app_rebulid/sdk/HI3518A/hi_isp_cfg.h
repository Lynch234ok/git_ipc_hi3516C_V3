#ifndef HI_ISP_CFG_H_
#define HI_ISP_CFG_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "hi_isp_cfg_def.h"

extern int HI_ISP_cfg_set_ae(int isDaylight, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_get_ae(int isDaylight, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_set_awb(int scene, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_get_awb(int scene, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_set_imp(int isDaylight, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_get_imp(int isDaylight, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_set_all(int isDaylight, int scene, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_get_all(int isDaylight, int scene, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_load(const char *filepath, LpIspCfgAttr ispCfgAttr);
extern int HI_ISP_cfg_set_imp_single(int isDaylight, LpIspCfgAttr ispCfgAttr);

#ifdef __cplusplus
};
#endif
#endif //HI_ISP_CFG_H_
