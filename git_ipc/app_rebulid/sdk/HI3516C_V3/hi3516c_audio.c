
#include "hi3516c.h"
#include "sdk/sdk_sys.h"
#include "sdk/sdk_audio.h"
#include "sdk_trace.h"
#include "sdk_common.h"
#include "audio_aac_adp.h"

#define kSDK_VIN_MAGIC (0xf0f0f0f0)

#define HI_AIN_CH_BACKLOG_REF (1)

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
			return AUDIO_BIT_WIDTH_24;
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
		case AUDIO_BIT_WIDTH_24:
			return 24;
		default:
			;
	}
    return -1;
}

static int audio_set_ai_vqe()
{
    AI_TALKVQE_CONFIG_S stAiVqeTalkAttr;
    memset(&stAiVqeTalkAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));
    stAiVqeTalkAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_8000;
    stAiVqeTalkAttr.s32FrameSample       = AUDIO_PTNUMPERFRM;
    stAiVqeTalkAttr.enWorkstate          = VQE_WORKSTATE_COMMON;

    stAiVqeTalkAttr.stAnrCfg.bUsrMode  = HI_TRUE;
    stAiVqeTalkAttr.stAnrCfg.s16NrIntensity = 10;
    stAiVqeTalkAttr.stAnrCfg.s16NoiseDbThr = 50;
    stAiVqeTalkAttr.stAnrCfg.s8SpProSwitch = 1;

    stAiVqeTalkAttr.stHpfCfg.bUsrMode    = HI_TRUE;
    stAiVqeTalkAttr.stHpfCfg.enHpfFreq   = AUDIO_HPF_FREQ_150;

    /*stAiVqeTalkAttr.stEqCfg.s8GaindB[0]= -25;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[1]= -25;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[2]= -22;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[3]= -22;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[4]= -20;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[5]= -5;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[6]= 2;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[7]= 5;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[8]= 0;
    stAiVqeTalkAttr.stEqCfg.s8GaindB[9]= -30;
    */

    stAiVqeTalkAttr.u32OpenMask =   AI_TALKVQE_MASK_ANR | AI_TALKVQE_MASK_HPF | AI_TALKVQE_MASK_AGC;

    HI_MPI_AI_SetTalkVqeAttr(HI_AIN_DEV, HI_AIN_CH, HI_AOUT_DEV, HI_AOUT_CH, &stAiVqeTalkAttr);
    HI_MPI_AI_EnableVqe(HI_AIN_DEV, HI_AIN_CH);

}

static int audio_set_ao_vqe()
{
    AO_VQE_CONFIG_S stAoVqeAttr;
    memset(&stAoVqeAttr, 0, sizeof(AO_VQE_CONFIG_S));
    stAoVqeAttr.u32OpenMask          = AO_VQE_MASK_ANR | AO_VQE_MASK_HPF;
    stAoVqeAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_8000;
    stAoVqeAttr.s32FrameSample       = AUDIO_PTNUMPERFRM;
    stAoVqeAttr.enWorkstate          = VQE_WORKSTATE_COMMON;

    stAoVqeAttr.stHpfCfg.bUsrMode    = HI_FALSE;

    stAoVqeAttr.stAnrCfg.bUsrMode       = HI_TRUE;
    stAoVqeAttr.stAnrCfg.s16NrIntensity = 25;
    stAoVqeAttr.stAnrCfg.s16NoiseDbThr  = 60;
    stAoVqeAttr.stAnrCfg.s8SpProSwitch  = 0;

    //stAoVqeAttr.stAgcCfg.bUsrMode       = HI_FALSE;

    HI_MPI_AO_SetVqeAttr(HI_AOUT_DEV, HI_AOUT_CH, &stAoVqeAttr);//HI_AOUT_CH
    HI_MPI_AO_EnableVqe(HI_AOUT_DEV, HI_AOUT_CH);

}

#include "acodec.h"
#define ACODEC_FILE "/dev/acodec"
#define CODE_TYPE_G711A (1<<1)
#define CODE_TYPE_G711U (1<<2)
#define CODE_TYPE_AAC (1<<3)
static int audio_init_ain(enSDK_AUDIO_SAMPLE_RATE sample_rate, int sample_width, int type)
{
	AIO_ATTR_S aio_attr;
    HI_U32 aiPtNumPerFrm;

    HI_S32 fdAcodec = -1;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0)
    {
        printf("%s: can't open acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        return HI_FAILURE;
    }

    if(ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL))
    {
        printf("Reset audio codec error\n");
        close(fdAcodec);
        return HI_FAILURE;
    }

    close(fdAcodec);

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
	//printf("%s-%d sample_width = %d-%d\r\n", __FUNCTION__, __LINE__, sample_width, aio_attr.enBitwidth);
	//printf("%s-%d sample_rate = %d-%d\r\n", __FUNCTION__, __LINE__, sample_rate, aio_attr.enSamplerate);
	aio_attr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	aio_attr.u32EXFlag = 0;
	aio_attr.u32FrmNum = 30;
    if(type == CODE_TYPE_AAC){
        printf("init under AAC mode, init the PtNumPerFrm:%d\n", AUDIO_AAC_PTNUMPERFRM);
        aiPtNumPerFrm = AUDIO_AAC_PTNUMPERFRM;
    }else if(type == CODE_TYPE_G711A){
        printf("init under G711A mdoe, init the PtNumPerFrm:%d\n", AUDIO_PTNUMPERFRM);
        aiPtNumPerFrm = AUDIO_PTNUMPERFRM;
    }
	aio_attr.u32ClkSel = 0;

	// enable aout device
    /* register aac encoder */
    SOC_CHECK(HI_MPI_AENC_AacInit());
	aio_attr.u32ChnCnt = 1;
    aio_attr.u32PtNumPerFrm = AUDIO_PTNUMPERFRM;
	SOC_CHECK(HI_MPI_AO_SetPubAttr(HI_AOUT_DEV, &aio_attr));
	SOC_CHECK(HI_MPI_AO_Enable(HI_AOUT_DEV));
	SOC_CHECK(HI_MPI_AO_EnableChn(HI_AOUT_DEV, HI_AOUT_CH));
    audio_set_ao_vqe();

	// enable ain device
	aio_attr.u32ChnCnt = 1;
    aio_attr.u32PtNumPerFrm = aiPtNumPerFrm;
	SOC_CHECK(HI_MPI_AI_SetPubAttr(HI_AIN_DEV, &aio_attr));
	SOC_CHECK(HI_MPI_AI_Enable(HI_AIN_DEV));
	SOC_CHECK(HI_MPI_AI_EnableChn(HI_AIN_DEV, HI_AIN_CH));
    audio_set_ai_vqe();

    _sdk_audio.api.audio_setting(false);



	return 0;
}

static int audio_destroy_ain()
{
    SOC_CHECK(HI_MPI_AI_DisableChn(HI_AIN_DEV, HI_AIN_CH));
    SOC_CHECK(HI_MPI_AI_Disable(HI_AIN_DEV));

    SOC_CHECK(HI_MPI_AO_DisableChn(HI_AOUT_DEV, HI_AOUT_CH));
    SOC_CHECK(HI_MPI_AO_Disable(HI_AOUT_DEV));
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

static int audio_set_ain_ch_attr_ptnum(int encType)
{
    AIO_ATTR_S aio_attr;
    HI_MPI_AI_GetPubAttr(HI_AIN_DEV, &aio_attr);
    SOC_CHECK(HI_MPI_AI_DisableChn(HI_AIN_DEV, HI_AIN_CH));
    SOC_CHECK(HI_MPI_AI_Disable(HI_AIN_DEV));
    if(encType == CODE_TYPE_G711A) {
        aio_attr.u32PtNumPerFrm = AUDIO_PTNUMPERFRM;
    }
    else if(encType == CODE_TYPE_AAC) {
        aio_attr.u32PtNumPerFrm = AUDIO_AAC_PTNUMPERFRM;
    }
    SOC_CHECK(HI_MPI_AI_SetPubAttr(HI_AIN_DEV, &aio_attr));
    SOC_CHECK(HI_MPI_AI_Enable(HI_AIN_DEV));
    SOC_CHECK(HI_MPI_AI_EnableChn(HI_AIN_DEV, HI_AIN_CH));
    audio_set_ai_vqe();

    return aio_attr.u32PtNumPerFrm;

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
	return -1;
}

static int audio_set_aout_play(int aout)
{
	SOC_INFO("HI3518 not support audio play mode");
	return -1;
}

static void audio_playSound(const void *data, unsigned int dataLen)
{
    AUDIO_FRAME_S audioFrame;
    int milliSec = -1;    // ×èÈû
    memset(&audioFrame, 0, sizeof(audioFrame));
    audioFrame.pVirAddr[0] = data;
    audioFrame.u32Len = dataLen;
    audioFrame.enSoundmode = AUDIO_SOUND_MODE_MONO;
    audioFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
    HI_MPI_AO_SendFrame(HI_AOUT_DEV, HI_AOUT_CH, &audioFrame, milliSec);

}

static void audio_playG711A(const unsigned char *g711aData, const short *table, int g711aBytes)
{
#define SOUND_PLAY_G711A_FRAME_SIZE (320)
    unsigned char code;
    short amp[SOUND_PLAY_G711A_FRAME_SIZE];
    int j = 0, data_left = g711aBytes, data_size = SOUND_PLAY_G711A_FRAME_SIZE;
	unsigned char *data_play = g711aData;

    while(data_left > 0) {
		data_size = (data_left >= SOUND_PLAY_G711A_FRAME_SIZE ? SOUND_PLAY_G711A_FRAME_SIZE : data_left);
        for(j = 0; j < data_size; j++) {
            code = *(data_play + j);
            amp[j] = table[code];
        }
        audio_playSound((void *)amp, data_size * sizeof(short));
        data_play += data_size;
		data_left -= data_size;
    }
}

static int audio_isAoBufFree(void)
{
    AO_CHN_STATE_S pstStatus;
    HI_MPI_AO_QueryChnStat(HI_AOUT_DEV, HI_AOUT_CH, &pstStatus);

    return pstStatus.u32ChnBusyNum;

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
		.audio_set_ain_ch_attr_ptnum = audio_set_ain_ch_attr_ptnum,

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

