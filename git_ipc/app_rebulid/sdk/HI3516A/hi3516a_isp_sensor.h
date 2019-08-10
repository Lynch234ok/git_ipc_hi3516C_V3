
#ifndef __HI3516A_ISP_SENSOR_H__
#define __HI3516A_ISP_SENSOR_H__

#include <stdbool.h>
#include "hi_comm_isp.h"
#include "mpi_isp.h"
#include "hi_comm_sns.h"
#include "hi_isp_debug.h"


typedef int (*SENSOR_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_DO_I2CWR)(unsigned short addr, unsigned short data);

//IMX178
extern HI_S32 SONY_IMX178_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int IMX178_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_IMX178_probe();
extern int IMX178_get_sensor_name(char *sensor_name);

//IMX185
extern HI_S32 SONY_IMX185_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int IMX185_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_IMX185_probe();
extern int IMX185_get_sensor_name(char *sensor_name);


//AR0330
extern HI_S32 APTINA_AR0330_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int AR0330_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_AR0330_probe();
extern int AR0330_get_sensor_name(char *sensor_name);


//OV4689
extern HI_S32 OV_OV4689_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int OV4689_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_OV4689_probe();
extern int OV4689_get_sensor_name(char *sensor_name);


//OV5658
extern HI_S32 OV5658_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int OV5658_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_OV5658_probe();
extern int OV5658_get_sensor_name(char *sensor_name);

//IMX34220
extern HI_S32 MN34220_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int MN34220_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_MN34220_probe();
extern int MN34220_get_sensor_name(char *sensor_name);


extern HI_S32 SONY_IMX326_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int IMX326_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_IMX326_probe();
extern int IMX326_get_sensor_name(char *sensor_name);


//OS05A
extern HI_S32 OS05A_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int OS05A_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_OS05A_probe();
extern int OS05A_get_sensor_name(char *sensor_name);

#endif //__HI3516A_ISP_SENSOR_H__

