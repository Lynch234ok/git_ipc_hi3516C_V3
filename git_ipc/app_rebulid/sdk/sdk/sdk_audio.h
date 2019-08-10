
#include "sdk_def.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef SDK_AUDIO_H_
#define SDK_AUDIO_H_
#ifdef __cplusplus
extern "C" {
#endif

/* 四位，千、百位表示芯片型号，十、个位表示板子类型和麦类型 */
typedef enum SDK_AUDIO_HW_SPEC {
	kSDK_AUDIO_HW_SPEC_IGNORE = 0,
    kSDK_AUDIO_HW_SPEC_1X = 100,      // Hi3518Ev200+38板+硅麦
    kSDK_AUDIO_HW_SPEC_2X = 101,      // Hi3518Ev200+38板+模拟麦
    kSDK_AUDIO_HW_SPEC_3X = 102,      // Hi3518Ev200+长条形板+模拟麦
    kSDK_AUDIO_HW_SPEC_4X = 200,      // Hi3516Dv100+38板+硅麦
    kSDK_AUDIO_HW_SPEC_5X = 300,      // Hi3518Ev200+P2_720单sensor板+普通麦
	//
}enSDK_AUDIO_HW_SPEC;


typedef enum SDK_AUDIO_SAMPLE_RATE {
	kSDK_AUDIO_SAMPLE_RATE_AUTO = 0,
	kSDK_AUDIO_SAMPLE_RATE_8000,
	kSDK_AUDIO_SAMPLE_RATE_12000, 
	kSDK_AUDIO_SAMPLE_RATE_11025,
	kSDK_AUDIO_SAMPLE_RATE_16000,
	kSDK_AUDIO_SAMPLE_RATE_22050,
	kSDK_AUDIO_SAMPLE_RATE_24000,
	kSDK_AUDIO_SAMPLE_RATE_32000,
	kSDK_AUDIO_SAMPLE_RATE_44100,
	kSDK_AUDIO_SAMPLE_RATE_48000,
}enSDK_AUDIO_SAMPLE_RATE;

typedef void (*fSDK_AUDIO_SETTING)(bool enabled);

typedef struct SDK_AUDIO_API {

	int (*init_ain)(enSDK_AUDIO_SAMPLE_RATE sample_rate, int sample_width, int type);
	int (*destroy_ain)(void);
	
	int (*create_ain_ch)(int ch);
	int (*release_ain_ch)(int ch);
	int (*get_ain_ch_attr)(int ch, enSDK_AUDIO_SAMPLE_RATE *sample_rate, int *sample_width, int *packet_size);
    int (*audio_set_ain_ch_attr_ptnum)(int encType);

	int (*set_aout_loop)(int ain);
	int (*set_aout_play)(int aout);
    void (*playSound)(const void *data, unsigned int dataLen);
    void (*playG711A)(const unsigned char *g711aData, const short *table, int g711aBytes);
    int (*isAoBufFree)(void);
	fSDK_AUDIO_SETTING audio_setting;
}stSDK_AUDIO_API, *lpSDK_AUDIO_API;

// could be used after 'SDK_init_ain' success to call
extern lpSDK_AUDIO_API sdk_audio;

extern int SDK_init_audio(enSDK_AUDIO_HW_SPEC hw_spec, fSDK_AUDIO_SETTING setting_api);
extern int SDK_destroy_audio();

#ifdef __cplusplus
};
#endif
#endif //SDK_AUDIO_H_

