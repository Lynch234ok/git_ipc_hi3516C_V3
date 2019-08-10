
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

#include "hi_type.h"
#include "hi_debug.h"
#include "hi_common.h"
#include "hi_comm_vb.h"
#include "hi_comm_sys.h"
#include "hi_comm_video.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_aenc.h"
#include "hi_comm_venc.h"
#include "hi_comm_aio.h"
#include "hi_comm_vpp.h"
#include "hi_comm_venc.h"
#include "hi_comm_avenc.h"
#include "hi_comm_md.h"

#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_ai.h"
#include "mpi_vb.h"
#include "mpi_aenc.h"
#include "mpi_vpp.h"
#include "mpi_venc.h"
#include "mpi_avenc.h"
#include "mpi_md.h"

#include "hifb.h"

#ifndef HI3515_H_
#define HI3515_H_
#ifdef __cplusplus
extern "C" {
#endif

const char *sdk_strerror(uint32_t const errno); // very import when "SOC_CHECK" used

#ifdef __cplusplus
};
#endif
#endif //HI3515_H_

