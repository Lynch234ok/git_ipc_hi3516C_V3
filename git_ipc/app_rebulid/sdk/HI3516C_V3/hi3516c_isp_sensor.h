
#ifndef __HI3516C_ISP_SENSOR_H__
#define __HI3516C_ISP_SENSOR_H__

#include <stdbool.h>
#include "hi_comm_isp.h"
#include "mpi_isp.h"
#include "hi_comm_sns.h"
#include "hi_isp_debug.h"




typedef int (*SENSOR_APTINA_AR0237_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_APTINA_AR0237_DO_I2CWR)(unsigned short addr, unsigned short data);

typedef int (*SENSOR_SMARTSENS_SC2235_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_SMARTSENS_SC2235_DO_I2CWR)(unsigned short addr, unsigned short data);

extern void APTINA_AR0237_init(SENSOR_APTINA_AR0237_DO_I2CRD do_i2c_read, SENSOR_APTINA_AR0237_DO_I2CWR do_i2c_write);//,ISP_AF_REGISTER_S *pAfRegister

extern void  SmartSens_SC2235_init(SENSOR_SMARTSENS_SC2235_DO_I2CRD do_i2c_read, SENSOR_SMARTSENS_SC2235_DO_I2CWR do_i2c_write);

//sensor support mirror  filp
extern int AR0237_set_mirror(bool mirror);
extern int AR0237_set_flip(bool flip);
extern int AR0237_get_sensor_name(char *sensor_name);
extern bool SENSOR_AR0237_probe();


extern int SC2235_set_mirror(bool mirror);
extern int SC2235_set_flip(bool flip);
extern int SC2235_get_sensor_name(char *sensor_name);
extern bool SENSOR_SC2235_probe();

extern int SC2232_set_mirror(bool mirror);
extern int SC2232_set_flip(bool flip);
extern int SC2232_get_sensor_name(char *sensor_name);
extern bool SENSOR_SC2232_probe();
extern int SmartSens_SC2232_init();


extern int IMX307_set_mirror(bool mirror);
extern int IMX307_set_flip(bool flip);
extern int IMX307_get_sensor_name(char *sensor_name);
extern bool IMX307_probe();
extern int IMX307_init();





#endif //__HI3518E_ISP_SENSOR_H__

