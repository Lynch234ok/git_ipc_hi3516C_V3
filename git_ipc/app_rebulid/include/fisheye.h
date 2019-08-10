#if !defined(NK_FISHEYE_H)
#define NK_FISHEYE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "netsdk.h"
typedef struct FISHEYE_config_t
{
    bool type;          // true表示set int，false 表示set float
    char lensName[16];
	EM_NSDK_IMAGE_FISHEYE_FIX_MODE fixMode;
    struct
    {
        int CenterCoordinateX;
        int CenterCoordinateY;
        int Radius;
        int AngleX;
        int AngleY;
        int AngleZ;
    }param[2];

    struct
    {
        float CenterCoordinateX;
        float CenterCoordinateY;
        float Radius;
        float AngleX;
        float AngleY;
        float AngleZ;
    }param2[2];
    pthread_mutex_t mutex;
}stFISHEYE_config, *lpFISHEYE_config;

typedef struct FISHEYE_lensParam {
    float angle;
    float height;
}ST_FISHEYE_LENS_PARAM, *LP_FISHEYE_LENS_PARAM;

extern NK_Int FISHEYE_config_init();
extern NK_Int FISHEYR_config_destroy();
extern NK_Int FISHEYE_config_get(lpFISHEYE_config fisheye_config);

/* 参数二，true表示set int，false 表示set float */
extern NK_Int FISHEYE_config_set(lpFISHEYE_config fisheye_config, bool type);
extern NK_Int FISHEYE_lens_param_len_get();
extern LP_FISHEYE_LENS_PARAM FISHEYE_lens_param_get();
extern EM_NSDK_IMAGE_FISHEYE_FIX_MODE FISHEYE_get_fix_mode(void);
#ifdef __cplusplus
};
#endif
#endif /*NK_FISHEYE_H*/