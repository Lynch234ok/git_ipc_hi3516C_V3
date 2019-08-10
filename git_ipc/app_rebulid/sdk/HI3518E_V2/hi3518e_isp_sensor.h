
#ifndef __HI3518E_ISP_SENSOR_H__
#define __HI3518E_ISP_SENSOR_H__

#include <stdbool.h>
#include "hi_comm_isp.h"
#include "mpi_isp.h"
#include "hi_comm_sns.h"
#include "hi_isp_debug.h"



typedef int (*SENSOR_DO_I2CRD)(unsigned short addr, unsigned short* ret_data);
typedef int (*SENSOR_DO_I2CWR)(unsigned short addr, unsigned short data);


//AR0130
extern int APTINA_AR0130_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int AR0130_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_AR0130_probe();
extern int AR0130_get_sensor_name(char *sensor_name);


//AR0230
extern int APTINA_AR0230_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);//,ISP_AF_REGISTER_S *pAfRegister
extern int AR0230_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_AR0230_probe();
extern int AR0230_get_sensor_name(char *sensor_name);

//IMX122
extern int SONY_IMX122_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int IMX122_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_IMX122_probe();
extern int IMX122_get_sensor_name(char *sensor_name);

//SC2035
extern int  SmartSens_SC2035_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC2035_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC2035_probe();
extern int SC2035_get_sensor_name(char *sensor_name);

//AR0237
extern int APTINA_AR0237_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);//,ISP_AF_REGISTER_S *pAfRegister
extern int AR0237_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_AR0237_probe();
extern int AR0237_get_sensor_name(char *sensor_name);

//SC2045
extern int  SmartSens_SC2045_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC2045_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC2045_probe();
extern int SC2045_get_sensor_name(char *sensor_name);

//SC1035
extern int  SmartSens_SC1035_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC1035_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC1035_probe();
extern int SC1035_get_sensor_name(char *sensor_name);

//SC1045
extern int  SmartSens_SC1045_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC1045_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC1045_probe();
extern int SC1045_get_sensor_name(char *sensor_name);

//SC1135
extern int  SmartSens_SC1135_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC1135_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC1135_probe();
extern int SC1135_get_sensor_name(char *sensor_name);

//SC1145
extern int  SmartSens_SC1145_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC1145_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC1145_probe();
extern int SC1145_set_mirror(bool mirror);//sensor support mirror  filp
extern int SC1145_set_flip(bool flip);//sensor support mirror  filp
extern int SC1145_get_sensor_name(char *sensor_name);

//SC3035
extern int  SmartSens_SC3035_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC3035_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC3035_probe();
extern int SC3035_get_sensor_name(char *sensor_name);

extern int  SmartSens_PS5270_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int PS5270_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_PS5270_probe();
extern int PS5270_get_sensor_name(char *sensor_name);

//SC2135
extern int  SmartSens_SC2135_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC2135_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC2135_probe();
extern int SC2135_set_mirror(bool mirror);//sensor support mirror  filp
extern int SC2135_set_flip(bool flip);//sensor support mirror  filp
extern int SC2135_get_sensor_name(char *sensor_name);

//IMX225
extern int SONY_IMX225_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int IMX225_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_IMX225_probe();
extern int IMX225_get_sensor_name(char *sensor_name);

//IMX291
extern int SONY_IMX291_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int IMX291_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_IMX291_probe();
extern int IMX291_get_sensor_name(char *sensor_name);

//PS5230
extern int PrimeSensor_PS5230_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int PS5230_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_PS5230_probe();
extern int PS5230_get_sensor_name(char *sensor_name);

//SC1235
extern void SmartSens_SC1235_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC1235_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC1235_probe();
extern int SC1235_get_sensor_name(char *sensor_name);

//SC2235
extern int SmartSens_SC2235_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write);
extern int SC2235_get_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern bool SENSOR_SC2235_probe();
extern int SC2235_get_sensor_name(char *sensor_name);
extern int SC2235_set_mirror(bool mirror);//sensor support mirror  filp
extern int SC2235_set_flip(bool flip);//sensor support mirror  filp

//sc2232
extern int SC2232_set_mirror(bool mirror);
extern int SC2232_set_flip(bool flip);
extern int SC2232_get_sensor_name(char *sensor_name);
extern bool SENSOR_SC2232_probe();
extern int SmartSens_SC2232_init();
extern int SC2232_get_resolution(uint32_t *ret_width, uint32_t *ret_height);



#endif //__HI3518E_ISP_SENSOR_H__

