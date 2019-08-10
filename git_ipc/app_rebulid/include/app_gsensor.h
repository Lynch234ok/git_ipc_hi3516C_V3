#if defined(GSENSOR)

#ifndef APP_GSENSOR
#define APP_GSENSOR

#include "http_server.h"

typedef struct {
    double angle_x;
    double angle_y;
    double angle_z;
}stGsensor_angles, *lpGsensor_angles;

typedef struct gsensorAngles {
    struct {
        double min_angle_range;      // -90度
        double max_angle_range;      // 90度
        double angle_x;
        double angle_y;
        double angle_z;
    }param;
	int version;           // 版本号0x01000000
	int reverse;
}ST_GSENSOR_ANGLES, *LP_GSENSOR_ANGLES;

typedef struct gsensorFrame {
	int frameType;         // 帧类型gsensor是2,  0是单镜头,1是双镜头
	int dataSize;
}ST_GSENSOR_FRAME, *LP_GSENSOR_FRAME;

typedef enum {
    emAPP_GSENSOR_ERR_CODE_OK = 0,
    emAPP_GSENSOR_ERR_CODE_ERROR = -1,
    emAPP_GSENSOR_ERR_CODE_NOT_SUPPORT = -2
}emAPP_GSENSOR_ERR_CODE;

extern int APP_GSENSOR_get_angles(lpGsensor_angles angles);

extern int APP_GSENSOR_start(void);

extern void APP_GSENSOR_stop(void);

extern int APP_GSENSOR_web_cgi_get_angles(LP_HTTP_CONTEXT session);

extern int APP_GSENSOR_get_to_json(struct json_object *gsensorJson, bool properties_flag);

extern int APP_GSENSOR_set_for_json(struct json_object *gsensorJson);

extern int APP_GSENSOR_calibration(bool enable);

extern int APP_GSENSOR_get_angles_frame(void *frame, unsigned int *frame_size);

extern bool APP_GSENSOR_is_support();

#endif // APP_GSENSOR

#endif // defined(GSENSOR)

