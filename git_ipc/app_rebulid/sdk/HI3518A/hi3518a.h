
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
#include <assert.h>

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
#include "hi_comm_sns.h"
#include "hi_sns_ctrl.h"

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
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_af.h"

#if defined(HI3531) // only dual mmz in 3531
// audio in
# define HI_AIN_CH (3)
// mmz zone name
# define HI_MMZ_ZONE_NAME0 (NULL)
# define HI_MMZ_ZONE_NAME1 "ddr1"

#elif defined(HI3520A) | defined(HI3520D) | defined(HI3521)
// audio in
# define HI_AIN_CH (1)
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

# define kHI_REG_PERIPHCTRL14_ADDR (0x20050068UL)
# define kHI_REG_CONVER_T_ADDR (0x20060080UL)
# define kHI_REG_T_VALUE_ADDR (0x2006009c)

# define kHI_RTC_APB_BASE_ADDR (0x20060000UL)
#  define kHI_RTC_APB_SPI_CLK_DIV (kHI_RTC_APB_BASE_ADDR + 0x0000UL)
#  define kHI_RTC_APB_SPI_RW (kHI_RTC_APB_BASE_ADDR + 0x0004UL)
#  define kHI_RTC_APB_SPI_CONVER_T (kHI_RTC_APB_BASE_ADDR + 0x0080UL)
#  define kHI_RTC_APB_SPI_CRC_EN (kHI_RTC_APB_BASE_ADDR + 0x0084UL)
#  define kHI_RTC_APB_SPI_INT_MASK (kHI_RTC_APB_BASE_ADDR + 0x0088UL)
#  define kHI_RTC_APB_SPI_INT_CLEAR (kHI_RTC_APB_BASE_ADDR + 0x008cUL)
#  define kHI_RTC_APB_SPI_BUSY (kHI_RTC_APB_BASE_ADDR + 0x0090UL)
#  define kHI_RTC_APB_SPI_INT_RAW (kHI_RTC_APB_BASE_ADDR + 0x0094UL)
#  define kHI_RTC_APB_SPI_INT_TCAP (kHI_RTC_APB_BASE_ADDR + 0x0098UL)
#  define kHI_RTC_APB_SPI_T_VALUE (kHI_RTC_APB_BASE_ADDR + 0x009cUL)
#  define kHI_RTC_APB_SPI_FILTER_NUM (kHI_RTC_APB_BASE_ADDR + 0x00a0UL)



#endif
extern const char *sdk_strerror(HI_S32 const errno);

#ifdef __cplusplus
};
#endif
#endif //HI3521_H_

