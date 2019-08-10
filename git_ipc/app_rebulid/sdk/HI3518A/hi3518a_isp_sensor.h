
#ifndef __HI3518A_ISP_SENSOR_H__
#define __HI3518A_ISP_SENSOR_H__

#include "hi_comm_isp.h"
#include "mpi_isp.h"
#include "hi_comm_sns.h"
#include "hi_isp_debug.h"

typedef int (*SENSOR_APTINA_AR0130_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_APTINA_AR0130_DO_I2CWR)(unsigned short addr, unsigned short data);
typedef int (*SENSOR_OV9712_DO_I2CRD)(unsigned char addr, unsigned char* ret_data);
typedef int (*SENSOR_OV9712_DO_I2CWR)(unsigned char addr, unsigned char data);
typedef int (*SENSOR_SOIH22_DO_I2CRD)(unsigned char addr, unsigned char* ret_data);
typedef int (*SENSOR_SOIH22_DO_I2CWR)(unsigned char addr, unsigned char data);
typedef int (*SENSOR_SONY_IMX122_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_SONY_IMX122_DO_I2CWR)(unsigned short addr, unsigned short data);
typedef int (*SENSOR_APTINA_AR0330_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_APTINA_AR0330_DO_I2CWR)(unsigned short addr, unsigned short data);
typedef int (*SENSOR_GC1004_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_GC1004_DO_I2CWR)(unsigned short addr, unsigned short data);
typedef int (*SENSOR_APTINA_AR0141_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_APTINA_AR0141_DO_I2CWR)(unsigned short addr, unsigned short data);
typedef int (*SENSOR_SC1035_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_SC1035_DO_I2CWR)(unsigned short addr, unsigned short data);
typedef int (*SENSOR_OV2710_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_OV2710_DO_I2CWR)(unsigned short addr, unsigned short data);

typedef int (*SENSOR_SOIH42_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_SOIH42_DO_I2CWR)(unsigned short addr, unsigned short data);

typedef int (*SENSOR_SC1045_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_SC1045_DO_I2CWR)(unsigned short addr, unsigned short data);

typedef int (*SENSOR_BG0701_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_BG0701_DO_I2CWR)(unsigned short addr, unsigned short data);

typedef int (*SENSOR_IMX225_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_IMX225_DO_I2CWR)(unsigned short addr, unsigned short data);

typedef int (*SENSOR_SC1135_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_SC1135_DO_I2CWR)(unsigned short addr, unsigned short data);

extern void APTINA_AR0130_init(SENSOR_APTINA_AR0130_DO_I2CRD do_i2c_read, SENSOR_APTINA_AR0130_DO_I2CWR do_i2c_write,ISP_AF_REGISTER_S *pAfRegister);
extern void OV9712PLUS_init(SENSOR_OV9712_DO_I2CRD do_i2c_read, SENSOR_OV9712_DO_I2CWR do_i2c_write);
extern void SOIH22_init(SENSOR_OV9712_DO_I2CRD do_i2c_read, SENSOR_OV9712_DO_I2CWR do_i2c_write);
extern void SONY_IMX122_init(SENSOR_SONY_IMX122_DO_I2CRD do_i2c_read,SENSOR_SONY_IMX122_DO_I2CWR do_i2c_write);
extern void APTINA_AR0330_init(SENSOR_APTINA_AR0330_DO_I2CRD do_i2c_read, SENSOR_APTINA_AR0330_DO_I2CWR do_i2c_write);
extern void OV9712_init(SENSOR_OV9712_DO_I2CRD do_i2c_read, SENSOR_OV9712_DO_I2CWR do_i2c_write);
extern void GC1004_init(SENSOR_GC1004_DO_I2CRD do_i2c_read, SENSOR_GC1004_DO_I2CWR do_i2c_write);

extern void APTINA_AR0141_init(SENSOR_APTINA_AR0141_DO_I2CRD do_i2c_read,SENSOR_APTINA_AR0141_DO_I2CWR do_i2c_write);

extern void SC1035_init(SENSOR_SC1035_DO_I2CRD do_i2c_read, SENSOR_SC1035_DO_I2CWR do_i2c_write);
extern void OV2710_init(SENSOR_OV2710_DO_I2CRD do_i2c_read, SENSOR_OV2710_DO_I2CWR do_i2c_write);

extern void SOIH42_init(SENSOR_SOIH42_DO_I2CRD do_i2c_read, SENSOR_SOIH42_DO_I2CWR do_i2c_write);

extern void SC1045_init(SENSOR_SC1045_DO_I2CRD do_i2c_read, SENSOR_SC1045_DO_I2CWR do_i2c_write);

extern void BG0701_init(SENSOR_BG0701_DO_I2CRD do_i2c_read, SENSOR_BG0701_DO_I2CWR do_i2c_write);
extern void SONY_IMX225_init(SENSOR_IMX225_DO_I2CRD  do_i2c_read,SENSOR_IMX225_DO_I2CWR do_i2c_write);

extern void SC1135_init(SENSOR_SC1135_DO_I2CRD do_i2c_read, SENSOR_SC1135_DO_I2CWR do_i2c_write);

#endif //__HI3518A_ISP_SENSOR_H__

