
#include "sdk_def.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef SDK_VIN_H_
#define SDK_VIN_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef enum SDK_VIN_HW_SPEC {
	kSDK_VIN_HW_SPEC_IGNORE,
	kSDK_VIN_HW_SPEC_HI35XX_4D1X2_CH0_CH2,
	kSDK_VIN_HW_SPEC_HI35XX_4D1X2_CH2_CH0,
	//
}enSDK_VIN_HW_SPEC, *lpSDK_VIN_HW_SPEC;

typedef enum SDK_VIN_FOCUS_MEASURE_ALG {
	kSDK_VIN_FOCUS_MEASURE_ALG_AUTO = 0,
	kSDK_VIN_FOCUS_MEASURE_ALG_VOLLATH5,
	kSDK_VIN_FOCUS_MEASURE_ALG_SQUARED_GRADIENT,
	//
}enSDK_VIN_FOCUS_MEASURE_ALG, *lpSDK_VIN_FOCUS_MEASURE_ALG;

typedef struct SDK_VIN_CAPTURE_ATTR {
	uint32_t width, height;
	int fps;
}ST_SDK_VIN_CAPTURE_ATTR, *LP_SDK_VIN_CAPTURE_ATTR;

typedef struct SDK_VIN_COVER_ATTR {
	bool enable;
	float x, y, width, height;
	uint32_t color;
}ST_SDK_VIN_COVER_ATTR, *LP_SDK_VIN_COVER_ATTR;

typedef void (*fSDK_MD_DO_TRAP)(int vin, const char *ret_name);
typedef struct SDK_VIN_API {
	
	enSDK_VIN_HW_SPEC (*hw_spec)(void);

	int (*get_capture_attr)(int vin, LP_SDK_VIN_CAPTURE_ATTR capture_attr);

	int (*focus_measure)(int vin, enSDK_VIN_FOCUS_MEASURE_ALG alg);
	int (*capture)(int vin, FILE* bitmap_stream);

	// motion detection 
	int (*create_md)(int vin, size_t ref_hblock, size_t ref_vblock, int ref_frame);
	int (*release_md)(int vin);

	int (*start_md)(int vin);
	int (*stop_md)(int vin);

	void (*set_md_bitmap_mode)(int vin, bool flag);

	int (*set_md_bitmap_threshold)(int vin, float threshold);
	float (*get_md_bitmap_threshold)(int vin);
	int (*set_md_bitmap_one_mask)(int vin, int x, int y, bool mask_flag);
	bool (*get_md_bitmap_one_mask)(int vin, int x, int y);
	int (*set_md_bitmap_mask)(int vin, uint8_t *mask_bitflag);
	const uint8_t *(*get_md_bitmap_mask)(int vin, size_t *ref_hblock, size_t *ref_vblock);
	void (*clear_md_bitmap_mask)(int vin);
	int (*set_md_alarm_stop)(bool *md_alarm_stop);
	int (*get_md_alarm_state)(bool* md_alarm_state);
	int (*add_md_rect)(int vin, const char *name,
		float x_ratio, float y_ratio, float width_ratio, float height_ratio, float threshold);
	int (*del_md_rect)(int vin, const char *name);
	bool (*check_md_rect)(int vin, const char *name,
		float *x_ratio, float *y_ratio, float *width_ratio, float *height_ratio, float *threshold);
	void (*clear_md_rect)(int vin);
	int (*set_md_rect_threshold)(int vin, const char *name, float threshold);
	float (*get_md_rect_threshold)(int vin, const char *name);

	int (*set_md_trap)(int vin, fSDK_MD_DO_TRAP do_trap);

	int (*set_md_ref_frame)(int vin, int ref_frame);
	int (*get_md_ref_frame)(int vin);

	int (*set_md_ref_freq)(int vin, int freq);
	int (*get_md_ref_freq)(int vin);
	
	void (*dump_md)(int vin);

	int (*set_cover)(int vin, int id, LP_SDK_VIN_COVER_ATTR cover);
	int (*get_cover)(int vin, int id, LP_SDK_VIN_COVER_ATTR cover);
	
	int (*start)(void);
	int (*stop)(void);

}stSDK_VIN_API, *lpSDK_VIN_API;

// could be used after 'SDK_init_vin' success to call
extern lpSDK_VIN_API sdk_vin;

extern int SDK_init_vin(enSDK_VIN_HW_SPEC hw_spec);
extern int SDK_destroy_vin();


#ifdef __cplusplus
};
#endif
#endif //SDK_VIN_H_

