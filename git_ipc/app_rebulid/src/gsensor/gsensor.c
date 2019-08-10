#if defined(GSENSOR)

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <app_debug.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <gsensor.h>
#include "adxl345.h"



static bool gsensor_inited = false;


int GSENSOR_init( void )
{
    if (gsensor_inited) {
        APP_TRACE("%s: GSensor already inited", __FUNCTION__);
        return 0;
    } else {
        if (0 != ADXL345_Init()) {
            APP_TRACE("%s: GSensor init failed!", __FUNCTION__);
            return -1;
        } else {
            gsensor_inited = true;
            return 0;
        }
    }
}

void GSENSOR_deinit( void )
{
    gsensor_inited = false;
    ADXL345_Deinit();
}

void GSENSOR_get_angles(double *p_angle_x, double *p_angle_y, double *p_angle_z)
{
    int32_t total_x = 0, total_y = 0, total_z = 0;
    int16_t x, y, z;
    int total_cnt = 20;
    int i;

    for(i = 0; i < total_cnt; i++) {
        ADXL345_Read_XYZ(&x, &y, &z);
        total_x += x;
        total_y += y;
        total_z += z;
        usleep(500);
    }

    x = (int16_t)(total_x/total_cnt);
    y = (int16_t)(total_y/total_cnt);
    z = (int16_t)(total_z/total_cnt);


    ADXL345_Get_Angles(x, y, z, p_angle_x, p_angle_y, p_angle_z);

    *p_angle_x = *p_angle_x / 10;
    *p_angle_y = *p_angle_y / 10;
    *p_angle_z = *p_angle_z / 10;

}

void GSENSOR_auto_correct(int8_t *p_cor_x,
                          int8_t *p_cor_y,
                          int8_t *p_cor_z)
{
    ADXL345_AUTO_Adjust(p_cor_x, p_cor_y, p_cor_z);
}

void GSENSOR_set_correct_xyz(int8_t cor_x,
                             int8_t cor_y,
                             int8_t cor_z)
{
    ADXL345_Write_CorrectXYZ(cor_x, cor_y, cor_z);
}

/*
@return true 初始化成功
        false 初始化失败
*/
bool GSENSOR_is_init()
{
    return gsensor_inited;

}

/*
 * test
 */

static double test_angle_x = 0, test_angle_y = 0, test_angle_z = 0;

static bool gs_test_thread_trigger = false;


bool is_ok(double a, double b)
{
    double diff;

    diff = a - b;

    if (diff < 0) {
        diff = -diff;
    }

    if (diff <= 20) {
        return true;
    } else {
        return false;
    }
}

bool is_angles_same(double cur_angle_x, double cur_angle_y, double cur_angle_z)
{
    if (!is_ok(test_angle_x, cur_angle_x)) {
        return false;
    }
    if (!is_ok(test_angle_y, cur_angle_y)) {
        return false;
    }
    if (!is_ok(test_angle_z, cur_angle_z)) {
        return false;
    }

    return true;
}

static void *gsensor_thr_test(void *arg)
{
    double cur_angle_x, cur_angle_y, cur_angle_z;
    int8_t cor_x, cor_y, cor_z;

    int sleep_us = 200000;
    int cnt_per_unit;
    int cnt = 0;


    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "gsensor_test");

    cnt_per_unit = (10 * 1000 * 1000) / sleep_us;


    if (0 != GSENSOR_init()) {
        APP_TRACE("%s: ADXL345_Init failed!", __FUNCTION__);
        return NULL;
    }

    while (gs_test_thread_trigger) {

        if (0 == (cnt % cnt_per_unit)) {
            GSENSOR_auto_correct(&cor_x, &cor_y, &cor_z);
            puts("");
            APP_TRACE("%s: Corrected. cor_x: %d, cor_y: %d, cor_z: %d\n",
                      __FUNCTION__,
                      cor_x, cor_y, cor_z);
            usleep(20000);
        }

        GSENSOR_get_angles(&cur_angle_x, &cur_angle_y, &cur_angle_z);
        if (!is_angles_same(cur_angle_x, cur_angle_y, cur_angle_z)) {
            test_angle_x = cur_angle_x;
            test_angle_y = cur_angle_y;
            test_angle_z = cur_angle_z;

            APP_TRACE("angle_x: %lf, angle_y: %lf, angle_z: %lf\n",
                      test_angle_x, test_angle_y, test_angle_z);
        }

        cnt++;

//        usleep(sleep_us);
    }

    GSENSOR_deinit();
    APP_TRACE("%s: GSensor test quit", __FUNCTION__);
    return NULL;
}

int GSENSOR_test_start( void )
{
    pthread_t pid;
    int ret;

    gs_test_thread_trigger = true;
    ret = pthread_create(&pid, NULL, gsensor_thr_test, NULL);
    if (0 != ret) {
        APP_TRACE("%s: Start GSensor test failed!", __FUNCTION__);
        gs_test_thread_trigger = false;
        return -1;
    } else {
        APP_TRACE("%s: Start GSensor test success", __FUNCTION__);
        return 0;
    }
}

void GSENSOR_test_stop( void )
{
    gs_test_thread_trigger = false;
}

#endif // defined(GSENSOR)

