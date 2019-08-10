
#ifndef HI3521_H_
#define HI3521_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "hi_type.h"

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vpss.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vda.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vda.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"



#if defined(HI3531) // only dual mmz in 3531
// audio in
# define HI_AIN_DEV (3)
# define HI_AOUT_DEV (4)
# define HI_AOUT_CH (0)
// mmz zone name
# define HI_MMZ_ZONE_NAME0 (NULL)
# define HI_MMZ_ZONE_NAME1 "ddr1"

#elif defined(HI3520A) | defined(HI3520D) | defined(HI3521)
// audio in
# define HI_AIN_DEV (1)
# define HI_AOUT_DEV (2)
# define HI_AOUT_CH (0)
// mmz zone name
# define HI_MMZ_ZONE_NAME0 (NULL)
# define HI_MMZ_ZONE_NAME1 HI_MMZ_ZONE_NAME0

#elif defined(HI3518A) | defined(HI3518C) | defined(HI3516C)|defined(HI3518E)
// audio in
# define HI_AIN_DEV (0)
# define HI_AOUT_DEV (0)
# define HI_AOUT_CH (0)
// mmz zone name
# define HI_MMZ_ZONE_NAME0 (NULL)
# define HI_MMZ_ZONE_NAME1 HI_MMZ_ZONE_NAME0

# define kHI_REG_PERIPHCTRL14
# define HI_CONVER_T_REG_ADDR (0x20060080UL)
# define HI_T_VALUE_REG_ADDR (0x2006009cUL)

#endif

extern const char *sdk_strerror(HI_S32 const errno);
extern void hi3521_mmz_zone_assign(MOD_ID_E const module_id, HI_S32 const dev_id, HI_S32 const chn_id, const char* mmz_zone_name);


#ifdef __cplusplus
};
#endif
#endif //HI3521_H_

