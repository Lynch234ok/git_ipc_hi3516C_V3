
#include "hi3518e.h"
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
    AI_VQE_CONFIG_S stAiVqeAttr;
    memset(&stAiVqeAttr, 0, sizeof(AI_VQE_CONFIG_S));
    if((_sdk_audio.attr.hw_spec == kSDK_AUDIO_HW_SPEC_1X) || 
        (_sdk_audio.attr.hw_spec == kSDK_AUDIO_HW_SPEC_5X)) {
        stAiVqeAttr.s32WorkSampleRate  = AUDIO_SAMPLE_RATE_8000;
        stAiVqeAttr.s32FrameSample     = AUDIO_PTNUMPERFRM;
        stAiVqeAttr.enWorkstate        = VQE_WORKSTATE_COMMON;

        stAiVqeAttr.bHpfOpen           = HI_TRUE;
        stAiVqeAttr.stHpfCfg.bUsrMode  = HI_FALSE;

        stAiVqeAttr.bAecOpen           = HI_FALSE;
        stAiVqeAttr.stAecCfg.bUsrMode  = HI_FALSE;
        stAiVqeAttr.stAecCfg.s8CngMode = 0;

        stAiVqeAttr.bAnrOpen           = HI_TRUE;
        stAiVqeAttr.stAnrCfg.bUsrMode  = HI_TRUE;
        stAiVqeAttr.stAnrCfg.s16NrIntensity = 10;
        stAiVqeAttr.stAnrCfg.s16NoiseDbThr = 50;
        stAiVqeAttr.stAnrCfg.s8SpProSwitch = 1;

        stAiVqeAttr.bAgcOpen           = HI_TRUE;
        stAiVqeAttr.stAgcCfg.bUsrMode  = HI_FALSE;

        stAiVqeAttr.bRnrOpen           = HI_FALSE;

        stAiVqeAttr.bEqOpen            = HI_FALSE;

        stAiVqeAttr.bHdrOpen           = HI_FALSE;
    }
    else if((_sdk_audio.attr.hw_spec == kSDK_AUDIO_HW_SPEC_2X) || 
        (_sdk_audio.attr.hw_spec == kSDK_AUDIO_HW_SPEC_3X)) {
        stAiVqeAttr.s32WorkSampleRate  = AUDIO_SAMPLE_RATE_8000;
        stAiVqeAttr.s32FrameSample     = AUDIO_PTNUMPERFRM;
        stAiVqeAttr.enWorkstate        = VQE_WORKSTATE_COMMON;

        stAiVqeAttr.bHpfOpen           = HI_TRUE;
        stAiVqeAttr.stHpfCfg.bUsrMode  = HI_FALSE;

        stAiVqeAttr.bAecOpen           = HI_FALSE;
        stAiVqeAttr.stAecCfg.bUsrMode  = HI_FALSE;
        stAiVqeAttr.stAecCfg.s8CngMode = 0;

        stAiVqeAttr.bAnrOpen           = HI_TRUE;
        stAiVqeAttr.stAnrCfg.bUsrMode  = HI_TRUE;
        stAiVqeAttr.stAnrCfg.s16NrIntensity = 10;
        stAiVqeAttr.stAnrCfg.s16NoiseDbThr = 50;
        stAiVqeAttr.stAnrCfg.s8SpProSwitch = 1;

        stAiVqeAttr.bAgcOpen           = HI_TRUE;
        stAiVqeAttr.stAgcCfg.bUsrMode  = HI_FALSE;

        stAiVqeAttr.bRnrOpen           = HI_FALSE;

        stAiVqeAttr.bEqOpen            = HI_TRUE;
        stAiVqeAttr.stEqCfg.s8GaindB[0]= -25;
        stAiVqeAttr.stEqCfg.s8GaindB[1]= -25;
        stAiVqeAttr.stEqCfg.s8GaindB[2]= -22;
        stAiVqeAttr.stEqCfg.s8GaindB[3]= -22;
        stAiVqeAttr.stEqCfg.s8GaindB[4]= -20;
        stAiVqeAttr.stEqCfg.s8GaindB[5]= -5;
        stAiVqeAttr.stEqCfg.s8GaindB[6]= 2;
        stAiVqeAttr.stEqCfg.s8GaindB[7]= 5;
        stAiVqeAttr.stEqCfg.s8GaindB[8]= 0;
        stAiVqeAttr.stEqCfg.s8GaindB[9]= -30;

        stAiVqeAttr.bHdrOpen           = HI_FALSE;
    }
    else {
        stAiVqeAttr.s32WorkSampleRate  = AUDIO_SAMPLE_RATE_8000;
        stAiVqeAttr.s32FrameSample     = AUDIO_PTNUMPERFRM;
        stAiVqeAttr.enWorkstate        = VQE_WORKSTATE_COMMON;

        stAiVqeAttr.bHpfOpen           = HI_TRUE;
        stAiVqeAttr.stHpfCfg.bUsrMode  = HI_FALSE;

        stAiVqeAttr.bAecOpen           = HI_FALSE;
        stAiVqeAttr.stAecCfg.bUsrMode  = HI_FALSE;
        stAiVqeAttr.stAecCfg.s8CngMode = 0;

        stAiVqeAttr.bAnrOpen           = HI_TRUE;
        stAiVqeAttr.stAnrCfg.bUsrMode  = HI_TRUE;
        stAiVqeAttr.stAnrCfg.s16NrIntensity = 10;
        stAiVqeAttr.stAnrCfg.s16NoiseDbThr = 50;
        stAiVqeAttr.stAnrCfg.s8SpProSwitch = 1;

        stAiVqeAttr.bAgcOpen           = HI_TRUE;
        stAiVqeAttr.stAgcCfg.bUsrMode  = HI_FALSE;

        stAiVqeAttr.bRnrOpen           = HI_FALSE;

        stAiVqeAttr.bEqOpen            = HI_TRUE;
        stAiVqeAttr.stEqCfg.s8GaindB[0]= -25;
        stAiVqeAttr.stEqCfg.s8GaindB[1]= -25;
        stAiVqeAttr.stEqCfg.s8GaindB[2]= -22;
        stAiVqeAttr.stEqCfg.s8GaindB[3]= -22;
        stAiVqeAttr.stEqCfg.s8GaindB[4]= -20;
        stAiVqeAttr.stEqCfg.s8GaindB[5]= -5;
        stAiVqeAttr.stEqCfg.s8GaindB[6]= 2;
        stAiVqeAttr.stEqCfg.s8GaindB[7]= 5;
        stAiVqeAttr.stEqCfg.s8GaindB[8]= 0;
        stAiVqeAttr.stEqCfg.s8GaindB[9]= -30;

        stAiVqeAttr.bHdrOpen           = HI_FALSE;
    }

    HI_MPI_AI_SetVqeAttr(HI_AIN_DEV, HI_AIN_CH, HI_AOUT_DEV, HI_AOUT_CH, &stAiVqeAttr);
    HI_MPI_AI_EnableVqe(HI_AIN_DEV, HI_AIN_CH);

}

static int audio_set_ao_vqe()
{
    AO_VQE_CONFIG_S stAoVqeAttr;
    memset(&stAoVqeAttr, 0, sizeof(AO_VQE_CONFIG_S));
    stAoVqeAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_8000;
    stAoVqeAttr.s32FrameSample       = AUDIO_PTNUMPERFRM;
    stAoVqeAttr.enWorkstate          = VQE_WORKSTATE_COMMON;

    stAoVqeAttr.bHpfOpen             = HI_TRUE;
    stAoVqeAttr.stHpfCfg.bUsrMode    = HI_TRUE;
    stAoVqeAttr.stHpfCfg.enHpfFreq   = AUDIO_HPF_FREQ_150;

    stAoVqeAttr.bAnrOpen             = HI_TRUE;
    stAoVqeAttr.stAnrCfg.bUsrMode    = HI_FALSE;

    stAoVqeAttr.bAgcOpen = HI_FALSE;

    stAoVqeAttr.bEqOpen  = HI_FALSE;

    HI_MPI_AO_SetVqeAttr(HI_AOUT_DEV, HI_AOUT_CH, &stAoVqeAttr);
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

    HI_S32 fdAcodec = -1;
    unsigned int i2s_fs_sel = 0;
    //unsigned int mixer_mic_ctrl = 0;
    unsigned int gain_mic = 0;

    fdAcodec = open(ACODEC_FILE,O_RDWR);
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

	
	i2s_fs_sel = 0x18;


    if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel))
    {
        printf("%s: set acodec sample rate failed\n", __FUNCTION__);
        close(fdAcodec);
        return HI_FAILURE;
    }

    if(0){//if (HI_TRUE == bMicIn)
                /*do not select micin */
#if 0
        mixer_mic_ctrl = 1;
        if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl))
        {
            printf("%s: set acodec micin failed\n", __FUNCTION__);
            close(fdAcodec);
            return HI_FAILURE;
        }
#endif
        /* set volume plus (0~0x1f,default 0x0c) */
        gain_mic = 0x18;
        if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICL, &gain_mic))
        {
            printf("%s: set acodec micin volume failed\n", __FUNCTION__);
            close(fdAcodec);
            return HI_FAILURE;
        }

        if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICR, &gain_mic))
        {
            printf("%s: set acodec micin volume failed\n", __FUNCTION__);
            close(fdAcodec);
            return HI_FAILURE;
        }
	}

	close(fdAcodec);

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

	// audio loop

    // audio setting callback
    _sdk_audio.api.audio_setting(false);

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
		// clear
		memset(&_sdk_audio.attr, 0, sizeof(_sdk_audio.attr));
		// hardware spec
		_sdk_audio.attr.hw_spec = hw_spec;
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

