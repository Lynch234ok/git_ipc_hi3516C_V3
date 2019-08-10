/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : aenc_aac_adp.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2011/02/26
  Description   : 
  History       :
  1.Date        : 2011/02/26
    Author      : n00168968
    Modification: Created file

******************************************************************************/


#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "audio_aac_adp.h"

static HI_S32 AencCheckAACAttr(const AENC_ATTR_AAC_S *pstAACAttr)
{  
    if (pstAACAttr->enBitWidth != AUDIO_BIT_WIDTH_16)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", 
            __FUNCTION__, __LINE__, "invalid bitwidth for AAC encoder");
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }
    
    if (pstAACAttr->enSoundMode >= AUDIO_SOUND_MODE_BUTT)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", 
            __FUNCTION__, __LINE__, "invalid sound mode for AAC encoder");
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

	if ((pstAACAttr->enAACType == AAC_TYPE_EAACPLUS) && (pstAACAttr->enSoundMode != AUDIO_SOUND_MODE_STEREO))
	{
		printf("[Func]:%s [Line]:%d [Info]:%s\n", 
            __FUNCTION__, __LINE__, "invalid sound mode for AAC encoder");
        return HI_ERR_AENC_ILLEGAL_PARAM;
	}
   
    return HI_SUCCESS;
}

HI_S32 AencAACCheckConfig(AACENC_CONFIG *pconfig)
{
    if(NULL == pconfig)
    {
    	printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "pconfig is null");
        return HI_ERR_AENC_NULL_PTR;
    }

    if(pconfig->coderFormat != AACLC && pconfig->coderFormat!= EAAC && pconfig->coderFormat != EAACPLUS)
    {
        printf("aacenc coderFormat(%d) invalid\n",pconfig->coderFormat);
    }


    if(pconfig->quality != AU_QualityExcellent && pconfig->quality!= AU_QualityHigh && pconfig->quality != AU_QualityMedium && pconfig->quality != AU_QualityLow) 
    {
        printf("aacenc quality(%d) invalid\n",pconfig->quality);
    }

    if(pconfig->bitsPerSample != 16) 
    {
        printf("aacenc bitsPerSample(%d) should be 16\n",pconfig->bitsPerSample);
    }

    if(pconfig->coderFormat == AACLC)
    {
        if(pconfig->nChannelsOut != pconfig->nChannelsIn) 
        {
    	    printf("AACLC nChannelsOut(%d) in not equal to nChannelsIn(%d)\n", pconfig->nChannelsOut, pconfig->nChannelsIn);
    	    return HI_ERR_AENC_ILLEGAL_PARAM;
        }
    	
        if(pconfig->sampleRate == 32000)
        {
    	    if(pconfig->bitRate < 24000 ||  pconfig->bitRate > 256000)
    	    {
    		    printf("AACLC 32000 Hz bitRate(%d) should be 24000 ~ 256000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else if(pconfig->sampleRate == 44100)
        {
    	    if(pconfig->bitRate < 32000 ||  pconfig->bitRate > 320000)
    	    {
    		    printf("AACLC 44100 Hz bitRate(%d) should be 32000 ~ 320000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else if(pconfig->sampleRate == 48000)
        {
    	    if(pconfig->bitRate < 32000 ||  pconfig->bitRate > 320000)
    	    {
    		    printf("AACLC 48000 Hz bitRate(%d) should be 48000 ~ 320000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else if(pconfig->sampleRate == 16000)
        {
    	    if(pconfig->bitRate < 16000 ||  pconfig->bitRate > 96000)
    	    {
    		    printf("AACLC 16000 Hz bitRate(%d) should be 16000 ~ 96000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else if(pconfig->sampleRate == 8000)
        {
    	    if(pconfig->bitRate < 8000 ||  pconfig->bitRate > 96000)
    	    {
    		    printf("AACLC 8000 Hz bitRate(%d) should be 8000 ~ 96000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else if(pconfig->sampleRate == 24000)
        {
    	    if(pconfig->bitRate < 20000 ||  pconfig->bitRate > 128000)
    	    {
    		    printf("AACLC 24000 Hz bitRate(%d) should be 20000 ~ 128000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else if(pconfig->sampleRate == 22050)
        {
    	    if(pconfig->bitRate < 16000 ||  pconfig->bitRate > 128000)
    	    {
    		    printf("AACLC 22025 Hz bitRate(%d) should be 16000 ~ 128000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else
        {
    	    printf("AACLC invalid samplerate(%d) should be 8000~48000\n",pconfig->sampleRate);
    	    return HI_ERR_AENC_ILLEGAL_PARAM;
        }
    }   
    else if(pconfig->coderFormat == EAAC)
    {
        if(pconfig->nChannelsOut != pconfig->nChannelsIn) 
        {
    	    printf("EAAC nChannelsOut(%d) is not equal to nChannelsIn(%d)\n", pconfig->nChannelsOut, pconfig->nChannelsIn);
    	    return HI_ERR_AENC_ILLEGAL_PARAM;
        }
    	
        if(pconfig->sampleRate == 32000)
        {
    	    if(pconfig->bitRate < 18000 ||  pconfig->bitRate > 23000)
    	    {
    		    printf("EAAC 32000 Hz bitRate(%d) should be 18000 ~ 23000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else if(pconfig->sampleRate == 44100)
        {
    	    if(pconfig->bitRate < 24000 ||  pconfig->bitRate > 51000)
    	    {
    		    printf("EAAC 44100 Hz bitRate(%d) should be 24000 ~ 51000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else if(pconfig->sampleRate == 48000)
        {
    	    if(pconfig->bitRate < 24000 || pconfig->bitRate > 51000)
    	    {
    		    printf("EAAC 48000 Hz bitRate(%d) should be 24000 ~ 51000\n",pconfig->bitRate);
    		    return HI_ERR_AENC_ILLEGAL_PARAM;
    	    }
        }
        else
        {
    	    printf("EAAC invalid samplerate(%d) should be 32000/44100/48000\n",pconfig->sampleRate);
    	    return HI_ERR_AENC_ILLEGAL_PARAM;
        }
    }  
    else if(pconfig->coderFormat == EAACPLUS)
    {
        if(pconfig->nChannelsOut != 2 || pconfig->nChannelsIn != 2) 
        {
            printf("EAACPLUS nChannelsOut(%d) and nChannelsIn(%d) should be 2\n",pconfig->nChannelsOut, pconfig->nChannelsIn);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }
        	
        if(pconfig->sampleRate == 32000)
        {
        	if(pconfig->bitRate < 10000 ||  pconfig->bitRate > 17000)
        	{
        	    printf("EAACPLUS 32000 Hz bitRate(%d) should  be 10000 ~ 17000\n",pconfig->bitRate);
        	    return HI_ERR_AENC_ILLEGAL_PARAM;
        	}
        }
        else if(pconfig->sampleRate == 44100)
        {
        	if(pconfig->bitRate < 12000 ||  pconfig->bitRate > 35000)
        	{
        	    printf("EAACPLUS 44100 Hz bitRate(%d) should  be 12000 ~ 35000\n",pconfig->bitRate);
        	    return HI_ERR_AENC_ILLEGAL_PARAM;
        	}
        }
        else if(pconfig->sampleRate == 48000)
        {
        	if(pconfig->bitRate < 12000 ||  pconfig->bitRate > 35000)
        	{
        	    printf("EAACPLUS 48000 Hz bitRate(%d) should  be 12000 ~ 35000\n",pconfig->bitRate);
        	    return HI_ERR_AENC_ILLEGAL_PARAM;
        	}
        }
        else
        {
        	printf("EAACPLUS invalid samplerate(%d) should be 32000/44100/48000\n",pconfig->sampleRate);
        	return HI_ERR_AENC_ILLEGAL_PARAM;
        }
        	
    }

    return 0;
}

HI_S32 OpenAACEncoder(HI_VOID *pEncoderAttr, HI_VOID **ppEncoder)
{
    AENC_AAC_ENCODER_S *pstEncoder = NULL;
    AENC_ATTR_AAC_S *pstAttr = NULL;
    HI_S32 s32Ret;
    AACENC_CONFIG  config;

    HI_ASSERT(pEncoderAttr != NULL); 
    HI_ASSERT(ppEncoder != NULL);

    /* 检查编码器属性 */
    pstAttr = (AENC_ATTR_AAC_S *)pEncoderAttr;
    s32Ret = AencCheckAACAttr(pstAttr);
    if (s32Ret)
    {
    	printf("[Func]:%s [Line]:%d s32Ret:0x%x.#########\n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }

    /* 为编码器状态空间分配内存 */
    pstEncoder = (AENC_AAC_ENCODER_S *)malloc(sizeof(AENC_AAC_ENCODER_S));
    if(NULL == pstEncoder)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "no memory");
        return HI_ERR_AENC_NOMEM;
    }
    memset(pstEncoder, 0, sizeof(AENC_AAC_ENCODER_S));
    *ppEncoder = (HI_VOID *)pstEncoder;        

    /* 为编码器设定默认配置 */
    s32Ret = AACInitDefaultConfig(&config);
    if (s32Ret)
    {
    	printf("[Func]:%s [Line]:%d s32Ret:0x%x.#########\n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }
    
    config.coderFormat   = (AuEncoderFormat)pstAttr->enAACType;
    config.bitRate       = pstAttr->enBitRate;
    config.bitsPerSample = 8*(1<<(pstAttr->enBitWidth)); 
    config.sampleRate    = pstAttr->enSmpRate;
    config.bandWidth     = config.sampleRate/2;     

	if (AUDIO_SOUND_MODE_MONO == pstAttr->enSoundMode && AAC_TYPE_EAACPLUS != pstAttr->enAACType)
	{
		config.nChannelsIn   = 1;
		config.nChannelsOut  = 1; 
	}
	else
	{
		config.nChannelsIn   = 2;
		config.nChannelsOut  = 2; 
	}
    
    config.quality       = AU_QualityHigh;

    s32Ret = AencAACCheckConfig(&config);
    if (s32Ret)
    {
    	printf("[Func]:%s [Line]:%d #########\n", __FUNCTION__, __LINE__);
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    /* 创建编码器 */
    s32Ret = AACEncoderOpen(&pstEncoder->pstAACState, &config);
    if (s32Ret)
    {
    	printf("[Func]:%s [Line]:%d s32Ret:0x%x.#########\n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }

    memcpy(&pstEncoder->stAACAttr, pstAttr, sizeof(AENC_ATTR_AAC_S));
        
    return HI_SUCCESS;
}

HI_S32 EncodeAACFrm(HI_VOID *pEncoder, const AUDIO_FRAME_S *pstData,
                    HI_U8 *pu8Outbuf,HI_U32 *pu32OutLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    AENC_AAC_ENCODER_S *pstEncoder = NULL;
    HI_U32 u32PtNums;
    HI_S32 i;     
    HI_S16 aData[AACENC_BLOCKSIZE*2*MAX_CHANNELS];
    HI_S16 s16Len = 0;

    HI_U32 u32WaterLine;    

    HI_ASSERT(pEncoder != NULL);
    HI_ASSERT(pstData != NULL);
    HI_ASSERT(pu8Outbuf != NULL); 
    HI_ASSERT(pu32OutLen != NULL);

    pstEncoder = (AENC_AAC_ENCODER_S *)pEncoder;
   
    if (AUDIO_SOUND_MODE_STEREO == pstEncoder->stAACAttr.enSoundMode)
    {
		/* 检查数据是否为立体声模式 */
		if(AUDIO_SOUND_MODE_STEREO != pstData->enSoundmode)
		{
			printf("[Func]:%s [Line]:%d [Info]:%s\n", 
            	__FUNCTION__, __LINE__, "AAC encode receive a frame which not match its Soundmode");
			return HI_ERR_AENC_ILLEGAL_PARAM;
		}
    }

    /*水线大小，等于各协议要求的帧长*/
    if (pstEncoder->stAACAttr.enAACType == AAC_TYPE_AACLC)
    {
        u32WaterLine = AACLC_SAMPLES_PER_FRAME;
    }
    else if (pstEncoder->stAACAttr.enAACType == AAC_TYPE_EAAC
        || pstEncoder->stAACAttr.enAACType == AAC_TYPE_EAACPLUS)
    {
        u32WaterLine = AACPLUS_SAMPLES_PER_FRAME;
    }
    else
	{
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "invalid AAC coder type");
        return HI_ERR_AENC_ILLEGAL_PARAM;;
	}
    /* 计算采样点数目(单声道的) */
    u32PtNums = pstData->u32Len/(pstData->enBitwidth + 1);

    /*如果传入数据的帧长大于协议要求帧长，非法，不接收，否则可能造成buffer爆掉*/
    if (u32PtNums != u32WaterLine)
    {
        printf("[Func]:%s [Line]:%d [Info]:invalid u32PtNums%d for AAC_TYPE_AACLC\n", 
            __FUNCTION__, __LINE__, u32PtNums);
        return HI_ERR_AENC_ILLEGAL_PARAM;;
    }

    /* AAC encoder need interleaved data,here change LLLRRR to LRLRLR. 
       AACLC will encode 1024*2 point, and AACplus encode 2048*2 point*/
    if (AUDIO_SOUND_MODE_STEREO == pstEncoder->stAACAttr.enSoundMode)
    {
		s16Len = u32WaterLine;
        for (i = s16Len-1; i >= 0 ; i--)
        {                
            aData[2*i] = *((HI_S16 *)pstData->pVirAddr[0] + i);
            aData[2*i+1] = *((HI_S16 *)pstData->pVirAddr[1] + i);
        }
 
    }
    else/* if inbuf is momo, copy left to right */
    {
        HI_S16 *temp = (HI_S16 *)pstData->pVirAddr[0];

		s16Len = u32WaterLine;
        for (i = s16Len-1; i >= 0 ; i--)
        {        
        	aData[i] = *(temp + i);
        }

    }
  
    s32Ret = AACEncoderFrame(pstEncoder->pstAACState, aData, pu8Outbuf, (HI_S32*)pu32OutLen);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "AAC encode failed");
    }
    return s32Ret;
}

HI_S32 CloseAACEncoder(HI_VOID *pEncoder)
{
    AENC_AAC_ENCODER_S *pstEncoder = NULL;

    HI_ASSERT(pEncoder != NULL);
    pstEncoder = (AENC_AAC_ENCODER_S *)pEncoder;

    AACEncoderClose(pstEncoder->pstAACState); 

    free(pstEncoder);
    return HI_SUCCESS;
}

HI_S32 OpenAACDecoder(HI_VOID *pDecoderAttr, HI_VOID **ppDecoder)
{
    ADEC_AAC_DECODER_S *pstDecoder = NULL;
    ADEC_ATTR_AAC_S *pstAttr = NULL;
    
    HI_ASSERT(pDecoderAttr != NULL); 
    HI_ASSERT(ppDecoder != NULL);

    pstAttr = (ADEC_ATTR_AAC_S *)pDecoderAttr;

    /* 为编码器状态空间分配内存 */
    pstDecoder = (ADEC_AAC_DECODER_S *)malloc(sizeof(ADEC_AAC_DECODER_S));
    if(NULL == pstDecoder)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "no memory");
        return HI_ERR_ADEC_NOMEM;
    }
    memset(pstDecoder, 0, sizeof(ADEC_AAC_DECODER_S));
    *ppDecoder = (HI_VOID *)pstDecoder;

    /* 创建编码器 */
    pstDecoder->pstAACState = AACInitDecoder();
    if (!pstDecoder->pstAACState)
    {
        
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "AACInitDecoder failed");
        return HI_FAILURE;
    }

    memcpy(&pstDecoder->stAACAttr, pstAttr, sizeof(ADEC_ATTR_AAC_S));
        
    return HI_SUCCESS;
}

HI_S32 DecodeAACFrm(HI_VOID *pDecoder, HI_U8 **pu8Inbuf,HI_S32 *ps32LeftByte,
                        HI_U16 *pu16Outbuf,HI_U32 *pu32OutLen,HI_U32 *pu32Chns)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ADEC_AAC_DECODER_S *pstDecoder = NULL;
    HI_S32 s32Samples, s32FrmLen, s32SampleBytes;
    AACFrameInfo aacFrameInfo;
    
    HI_ASSERT(pDecoder != NULL);
    HI_ASSERT(pu8Inbuf != NULL);
    HI_ASSERT(ps32LeftByte != NULL); 
    HI_ASSERT(pu16Outbuf != NULL);  
    HI_ASSERT(pu32OutLen != NULL);  
    HI_ASSERT(pu32Chns != NULL);

    *pu32Chns = 1;/*voice encoder only one channle */

    pstDecoder = (ADEC_AAC_DECODER_S *)pDecoder;
    
    s32FrmLen = AACDecodeFindSyncHeader(pstDecoder->pstAACState, pu8Inbuf, ps32LeftByte);            
    if (s32FrmLen < 0)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "AAC decoder can't find sync header");
        return HI_ERR_ADEC_BUF_LACK;
    }
    
    /*Notes: pInbuf will updated*/ 
    s32Ret = AACDecodeFrame(pstDecoder->pstAACState, pu8Inbuf, ps32LeftByte, (HI_S16 *)pu16Outbuf);
    #if 0
    if (s32Ret)
    {
        //printf("------------[Func]:%s [Line]:%d  [Ret]:%x  [Inbuf]:0x%x  [LeftByte]:%d->%d [Info]:%s\n", 
        //    __FUNCTION__, __LINE__, s32Ret, pu8Inbuf,s32LefteByte, *ps32LeftByte, "AAC decode failed");
        return s32Ret;
    }  
    else
    {
        //printf("------------[Func]:%s [Line]:%d  [Ret]:%x  [Inbuf]:0x%x  [LeftByte]:%d->%d [Info]:%s\n", 
        //    __FUNCTION__, __LINE__, s32Ret, pu8Inbuf,s32LefteByte, *ps32LeftByte, "AAC decode failed");
        
    }
    #endif
    AACGetLastFrameInfo(pstDecoder->pstAACState, &aacFrameInfo);
    
    /* samples per frame of one sound track*/
    s32Samples = aacFrameInfo.outputSamps/aacFrameInfo.nChans;
    HI_ASSERT(s32Samples==AACLC_SAMPLES_PER_FRAME||s32Samples==AACPLUS_SAMPLES_PER_FRAME);
    
    s32SampleBytes = s32Samples * sizeof(HI_U16);
    *pu32Chns = aacFrameInfo.nChans;
    *pu32OutLen = s32SampleBytes;
    
    /* NOTICE: our audio frame format is same as AAC decoder L/L/L/... R/R/R/...*/    
#if 0       
    /*change data format of AAC encoder to our AUDIO_FRAME format */
    if (MAX_AUDIO_FRAME_LEN != s32SampleBytes)
    {
        /* after decoder, 1024 left samples , then 1024 right samples*/
        memmove((HI_U8*)pOutbuf + MAX_AUDIO_FRAME_LEN, 
            (HI_U8*)pOutbuf + s32SampleBytes, s32SampleBytes);
    }
#endif  
    
    return s32Ret;
}

HI_S32 GetAACFrmInfo(HI_VOID *pDecoder, HI_VOID *pInfo)
{
    ADEC_AAC_DECODER_S *pstDecoder = NULL;
    AACFrameInfo aacFrameInfo;
    AAC_FRAME_INFO_S *pstAacFrm = NULL;
    
    HI_ASSERT(pDecoder != NULL);
    HI_ASSERT(pInfo != NULL);
    
    pstDecoder = (ADEC_AAC_DECODER_S *)pDecoder;
    pstAacFrm = (AAC_FRAME_INFO_S *)pInfo;
            
    AACGetLastFrameInfo(pstDecoder->pstAACState, &aacFrameInfo);

    pstAacFrm->s32Samplerate = aacFrameInfo.sampRateOut;
    pstAacFrm->s32BitRate = aacFrameInfo.bitRate;
    pstAacFrm->s32Profile = aacFrameInfo.profile;
    pstAacFrm->s32TnsUsed = aacFrameInfo.tnsUsed;
    pstAacFrm->s32PnsUsed = aacFrameInfo.pnsUsed;

    return HI_SUCCESS;
}


HI_S32 CloseAACDecoder(HI_VOID *pDecoder)
{
    ADEC_AAC_DECODER_S *pstDecoder = NULL;

    HI_ASSERT(pDecoder != NULL);
    pstDecoder = (ADEC_AAC_DECODER_S *)pDecoder;

    AACFreeDecoder(pstDecoder->pstAACState); 

    free(pstDecoder);
    return HI_SUCCESS;
}

HI_S32 ResetAACDecoder(HI_VOID *pDecoder)
{
    ADEC_AAC_DECODER_S *pstDecoder = NULL;

    HI_ASSERT(pDecoder != NULL);
    pstDecoder = (ADEC_AAC_DECODER_S *)pDecoder;

    AACFreeDecoder(pstDecoder->pstAACState); 

    /* 创建编码器 */
    pstDecoder->pstAACState = AACInitDecoder();
    if (!pstDecoder->pstAACState)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "AACResetDecoder failed");
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}



HI_S32 HI_MPI_AENC_AacInit(HI_VOID)
{
    HI_S32 s32Handle, s32Ret;
    AENC_ENCODER_S stAac;
    
    stAac.enType = PT_AAC;
    sprintf(stAac.aszName, "Aac");
    stAac.u32MaxFrmLen = MAX_AAC_MAINBUF_SIZE;
    stAac.pfnOpenEncoder = OpenAACEncoder;
    stAac.pfnEncodeFrm = EncodeAACFrm;
    stAac.pfnCloseEncoder = CloseAACEncoder;
    s32Ret = HI_MPI_AENC_RegeisterEncoder(&s32Handle, &stAac);
    if (s32Ret)
    {
        return s32Ret;
    }

    stAac.enType = PT_AACLC;
    sprintf(stAac.aszName, "Aaclc");
    stAac.u32MaxFrmLen = MAX_AAC_MAINBUF_SIZE;
    stAac.pfnOpenEncoder = OpenAACEncoder;
    stAac.pfnEncodeFrm = EncodeAACFrm;
    stAac.pfnCloseEncoder = CloseAACEncoder;
    s32Ret = HI_MPI_AENC_RegeisterEncoder(&s32Handle, &stAac);
    if (s32Ret)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ADEC_AacInit(HI_VOID)
{
    HI_S32 s32Handle, s32Ret;

    ADEC_DECODER_S stAac;
    
    stAac.enType = PT_AAC;
    sprintf(stAac.aszName, "Aac");
    stAac.pfnOpenDecoder = OpenAACDecoder;
    stAac.pfnDecodeFrm = DecodeAACFrm;
    stAac.pfnGetFrmInfo = GetAACFrmInfo;
    stAac.pfnCloseDecoder = CloseAACDecoder;
    stAac.pfnResetDecoder = ResetAACDecoder;
    s32Ret = HI_MPI_ADEC_RegeisterDecoder(&s32Handle, &stAac);
    if (s32Ret)
    {
        return s32Ret;
    }

    stAac.enType = PT_AACLC;
    sprintf(stAac.aszName, "Aaclc");
    stAac.pfnOpenDecoder = OpenAACDecoder;
    stAac.pfnDecodeFrm = DecodeAACFrm;
    stAac.pfnGetFrmInfo = GetAACFrmInfo;
    stAac.pfnCloseDecoder = CloseAACDecoder;
    stAac.pfnResetDecoder = ResetAACDecoder;
    s32Ret = HI_MPI_ADEC_RegeisterDecoder(&s32Handle, &stAac);
    if (s32Ret)
    {
        return s32Ret;
    }
    
    return HI_SUCCESS;
}

