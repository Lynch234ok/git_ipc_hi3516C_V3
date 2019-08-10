
#include "hi3521.h"
#include "sdk/sdk_sys.h"
#include "sdk/sdk_audio.h"
#include "sdk_trace.h"
#include "sdk_common.h"

#define kSDK_VIN_MAGIC (0xf0f0f0f0)

#if defined(HI3518A)|defined(HI3518C)|defined(HI3516C)|defined(HI3518E)
# define HI_AIN_CH_BACKLOG_REF (1)
#else
# define HI_AIN_CH_BACKLOG_REF (16)
#endif


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

static AUDIO_SAMPLE_RATE_E audio_samplerate_sdk2hisdk(enSDK_AUDIO_SAMPLE_RATE sample_rate)
{
	switch(sample_rate){
		case kSDK_AUDIO_SAMPLE_RATE_8000:
			return AUDIO_SAMPLE_RATE_8000;
		case kSDK_AUDIO_SAMPLE_RATE_12000:
			return AUDIO_SAMPLE_RATE_12000;
		case kSDK_AUDIO_SAMPLE_RATE_11025:
			return AUDIO_SAMPLE_RATE_11025;
		case kSDK_AUDIO_SAMPLE_RATE_16000:
			return AUDIO_SAMPLE_RATE_16000;
		case kSDK_AUDIO_SAMPLE_RATE_22050:
			return AUDIO_SAMPLE_RATE_22050;
		case kSDK_AUDIO_SAMPLE_RATE_24000:
			return AUDIO_SAMPLE_RATE_24000;
		case kSDK_AUDIO_SAMPLE_RATE_32000:
			return AUDIO_SAMPLE_RATE_32000;
		case kSDK_AUDIO_SAMPLE_RATE_44100:
			return AUDIO_SAMPLE_RATE_44100;
		case kSDK_AUDIO_SAMPLE_RATE_48000:
			return AUDIO_SAMPLE_RATE_48000;
		default:
			;
	}
    return AUDIO_SAMPLE_RATE_BUTT;
}

static enSDK_AUDIO_SAMPLE_RATE audio_samplerate_hisdk2sdk(AUDIO_SAMPLE_RATE_E sample_rate)
{
	switch(sample_rate){
	case AUDIO_SAMPLE_RATE_8000:
		return kSDK_AUDIO_SAMPLE_RATE_8000;
	case AUDIO_SAMPLE_RATE_12000:
		return kSDK_AUDIO_SAMPLE_RATE_12000;
	case AUDIO_SAMPLE_RATE_11025:
		return kSDK_AUDIO_SAMPLE_RATE_11025;
	case AUDIO_SAMPLE_RATE_16000:
		return kSDK_AUDIO_SAMPLE_RATE_16000;
	case AUDIO_SAMPLE_RATE_22050:
		return kSDK_AUDIO_SAMPLE_RATE_22050;
	case AUDIO_SAMPLE_RATE_24000:
		return kSDK_AUDIO_SAMPLE_RATE_24000;
	case AUDIO_SAMPLE_RATE_32000:
		return kSDK_AUDIO_SAMPLE_RATE_32000;
	case AUDIO_SAMPLE_RATE_44100:
		return kSDK_AUDIO_SAMPLE_RATE_44100;
	case AUDIO_SAMPLE_RATE_48000:
		return kSDK_AUDIO_SAMPLE_RATE_48000;
	default:
		;
	}
	return kSDK_AUDIO_SAMPLE_RATE_AUTO;
}


static AUDIO_SAMPLE_RATE_E audio_samplewidth_sdk2hisdk(int sample_rate)
{
	switch(sample_rate){
		case 8:
			return AUDIO_BIT_WIDTH_8;
		case 16:
			return AUDIO_BIT_WIDTH_16;
		case 32:
			return AUDIO_BIT_WIDTH_32;
	}
    return AUDIO_BIT_WIDTH_BUTT;
}

static int audio_samplewidth_hisdk2sdk(AUDIO_SAMPLE_RATE_E sample_rate)
{
	switch(sample_rate){
		case AUDIO_BIT_WIDTH_8:
			return 8;
		case AUDIO_BIT_WIDTH_16:
			return 16;
		case AUDIO_BIT_WIDTH_32:
			return 32;
		default:
			;
	}
    return -1;
}

static int audio_init_ain(enSDK_AUDIO_SAMPLE_RATE sample_rate, int sample_width)
{
	AIO_ATTR_S aio_attr;
	// config aio dev attr
    memset(&aio_attr, 0, sizeof(aio_attr));
	//aio_attr.enWorkmode = AIO_MODE_I2S_SLAVE;
	aio_attr.enWorkmode = AIO_MODE_I2S_MASTER;
	aio_attr.enBitwidth = audio_samplewidth_sdk2hisdk(sample_width);
	if(AUDIO_BIT_WIDTH_BUTT == aio_attr.enBitwidth){
		aio_attr.enBitwidth = AUDIO_BIT_WIDTH_16;
	}
	aio_attr.enSamplerate = audio_samplerate_sdk2hisdk(sample_rate);
	if(AUDIO_SAMPLE_RATE_BUTT == aio_attr.enSamplerate){
		aio_attr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
	}
	aio_attr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	aio_attr.u32EXFlag = 1;
	aio_attr.u32FrmNum = 5;
	aio_attr.u32PtNumPerFrm = 480;
	aio_attr.u32ClkSel = 1;

/*
	aio_attr.u32ChnCnt = 2;
	SOC_CHECK(HI_MPI_AO_SetPubAttr(HI_AOUT_DEV, &aio_attr));
	SOC_CHECK(HI_MPI_AO_Enable(HI_AOUT_DEV));
	SOC_CHECK(HI_MPI_AO_EnableChn(HI_AOUT_DEV, HI_AOUT_CH));
*/
	// enable ain device
	aio_attr.u32ChnCnt = 2;
	SOC_CHECK(HI_MPI_AI_SetPubAttr(HI_AIN_DEV, &aio_attr));
	SOC_CHECK(HI_MPI_AI_Enable(HI_AIN_DEV));

	// audio loop

	return 0;
}

static int audio_destroy_ain()
{
	SOC_CHECK(HI_MPI_AI_Disable(HI_AIN_DEV));
	return 0;

}

static int audio_get_ain_ch_attr(int ch, enSDK_AUDIO_SAMPLE_RATE *sample_rate, int *sample_width, int *packet_size)
{
	AIO_ATTR_S aio_attr;
	if(HI_SUCCESS == HI_MPI_AI_GetPubAttr(HI_AIN_DEV, &aio_attr)){
		if(sample_rate){
			*sample_rate = audio_samplerate_hisdk2sdk(aio_attr.enSamplerate);
		}
		if(sample_width){
			*sample_width = audio_samplewidth_hisdk2sdk(aio_attr.enBitwidth);
		}
		if(packet_size){
			*packet_size = (int)aio_attr.u32PtNumPerFrm;
		}
		return 0;
	}
	return -1;
}

static int audio_create_ain_ch(int ch)
{
	if(1){
		SOC_CHECK(HI_MPI_AI_EnableChn(HI_AIN_DEV, ch));
		//SOC_CHECK(HI_MPI_AI_EnableAnr(HI_AIN_DEV, ch));
		return 0;
	}
	return -1;
}

static int audio_release_ain_ch(int ch)
{
	if(1){
		//SOC_CHECK(HI_MPI_AI_DisableAnr(HI_AIN_DEV, ch));
		SOC_CHECK(HI_MPI_AI_DisableChn(HI_AIN_DEV, ch));
		return 0;
	}
	return -1;
}

static int audio_set_aout_loop(int ain)
{
	if(HI_SUCCESS == ){
		return 0;
	}
	return -1;
}

static int audio_set_aout_play(int aout)
{
	SOC_INFO("HI3518 not support audio play mode");
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

int SDK_init_audio(enSDK_AUDIO_HW_SPEC hw_spec)
{
//	int i = 0;
	if(NULL == sdk_audio){
		// hardware spec
		_sdk_audio.attr.hw_spec = hw_spec;
		// clear
		memset(&_sdk_audio.attr, 0, sizeof(_sdk_audio.attr));
		// init the handle pointer
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

