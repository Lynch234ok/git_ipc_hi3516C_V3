
#include "sdk/sdk_sys.h"
#include "sdk/sdk_audio.h"
#include "sdk_trace.h"
#include "sdk_common.h"

#define kSDK_VIN_MAGIC (0xf0f0f0f0)


# define HI_AIN_CH_BACKLOG_REF (1)


typedef struct SDK_AUDIO_AIN_CH_ATTR {

}stSDK_AUDIO_AIN_CH_ATTR, *lpSDK_AUDIO_AIN_CH_ATTR;

typedef struct SDK_AUDIO_ATTR {
	enSDK_AUDIO_HW_SPEC hw_spec;
	enSDK_AUDIO_SAMPLE_RATE ain_sample_rate;
	uint32_t ain_sample_width;
	
}stSDK_AUDIO_ATTR, *lpSDK_AUDIO_ATTR;

typedef struct SDK_AUDIO_HI3521 {
	stSDK_AUDIO_API api;
	stSDK_AUDIO_ATTR attr;
}stSDK_AUDIO_HI3521, *lpSDK_AUDIO_HI3521;

static stSDK_AUDIO_HI3521 _sdk_audio;
lpSDK_AUDIO_API sdk_audio = NULL;


static int audio_init_ain(enSDK_AUDIO_SAMPLE_RATE sample_rate, int sample_width, fSDK_AUDIO_SETTING api)
{
	return 0;
}

static int audio_destroy_ain()
{
	return 0;
}

static int audio_get_ain_ch_attr(int ch, enSDK_AUDIO_SAMPLE_RATE *sample_rate, int *sample_width, int *packet_size)
{
	return -1;
}

static int audio_create_ain_ch(int ch)
{
	return -1;
}

static int audio_release_ain_ch(int ch)
{
	return -1;
}

static int audio_set_aout_loop(int ain)
{
	return -1;
}

static int audio_set_aout_play(int aout)
{
	return -1;
}

static void audio_playSound(const void *data, unsigned int dataLen)
{

}

static void audio_playG711A(const unsigned char *g711aData, const short *table, int g711aBytes)
{

}

static int audio_isAoBufFree(void)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

static stSDK_AUDIO_HI3521 _sdk_audio = {
	.api = {
		.init_ain = audio_init_ain,
		.destroy_ain = audio_destroy_ain,
		
		.create_ain_ch = audio_create_ain_ch,
		.release_ain_ch = audio_release_ain_ch,
		.get_ain_ch_attr = audio_get_ain_ch_attr,

		.set_aout_loop = audio_set_aout_loop,
		.set_aout_play = audio_set_aout_play,

        .playSound = audio_playSound,
        .playG711A = audio_playG711A,
        .isAoBufFree = audio_isAoBufFree,
	},
};

int SDK_init_audio(enSDK_AUDIO_HW_SPEC hw_spec, fSDK_AUDIO_SETTING setting_api)
{
//	int i = 0;
	if(NULL == sdk_audio){
		// hardware spec
		_sdk_audio.attr.hw_spec = hw_spec;
		// clear
		memset(&_sdk_audio.attr, 0, sizeof(_sdk_audio.attr));
		// init the handle pointer
		_sdk_audio.api.audio_setting = setting_api;
		sdk_audio = (lpSDK_AUDIO_API)(&_sdk_audio);
		return 0;
	}
	return -1;
}

int SDK_destroy_audio()
{
//	int i = 0;
	if(sdk_audio){
		// destroy audio input
		sdk_audio->destroy_ain();
		// clear the handle pointer
		sdk_audio = NULL;
		return 0;
	}
	return -1;
}

