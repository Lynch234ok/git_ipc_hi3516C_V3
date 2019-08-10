

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <NkUtils/assert.h>
#include <NkUtils/macro.h>
#include <NkEmbedded/thread.h>
#include <NkUtils/macro.h>

#include "hi_type.h"
#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vpss.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vda.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vda.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_ae.h"
#include <sound.h>
#include "p2p/p2pdevice.h"




// ���´���д���е����

typedef struct Nk_WAVFileHeadField {

    NK_Byte riff[4];			  //��Դ�����ļ���־
    NK_UInt32 size; 			  //���¸���ַ��ʼ���ļ���β���ֽ���
    NK_Byte wave_flag[4];		  //wave�ļ���ʶ
    NK_Byte fmt[4];			  //���θ�ʽ��ʶ
    NK_UInt32 fmt_len;			  //�����ֽ�(һ��Ϊ00000010H)
    NK_UInt16 tag;				  //��ʽ���ֵ࣬Ϊ1ʱ����ʾPCM���Ա���
    NK_UInt16 channels; 		  //ͨ������������Ϊ1��˫����Ϊ2
    NK_UInt32 samp_freq;		  //����Ƶ��
    NK_UInt32 byte_rate;		  //���ݴ����� (ÿ���ֽڣ�����Ƶ�ʡ�ÿ�������ֽ���)
    NK_UInt16 block_align;		  //������ֽ��� = channles * bit_samp / 8
    NK_UInt16 bit_samp; 		  //bits per sample (�ֳ�����λ��)

} NK_WAVFileHeadField;

typedef struct WaveStruct
{
    FILE *fp;				  //file pointer
    NK_WAVFileHeadField header;	  //header
    NK_Byte data_flag[4];		  //���ݱ�ʶ��
    NK_UInt32 length;			  //������������
    NK_UInt32 *pData;			  //data
} wave_t;


static int readWaveHeader(FILE* fID, wave_t* pwavst)
{
	NK_Byte temp = 0;
	NK_Byte read_bytes = 0;
	char *channel_mappings[] = {NULL,"mono","stereo"};
	NK_UInt32 total_time = 0;
	struct PlayTime        //����ʱ��
	{
		NK_Byte hour;
		NK_Byte minute;
		NK_Byte second;
	} play_time;

	pwavst->fp=fID;                              /* open file */

	/* read heade information */
	if(4 != fread(&pwavst->header.riff, sizeof(NK_Byte), 4, pwavst->fp))           /* RIFF chunk */
	{
		printf("read riff error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->header.size, sizeof(NK_UInt32), 1, pwavst->fp))         /* SIZE : from here to file end */
	{
		printf("read size error!\n");
		return -1;
	}
	if(4 != fread(&pwavst->header.wave_flag, sizeof(NK_Byte), 4, pwavst->fp))      /* wave file flag */
	{
		printf("read wave_flag error!\n");
		return -1;
	}
	if(4 != fread(&pwavst->header.fmt, sizeof(NK_Byte), 4, pwavst->fp))             /* fmt chunk */
	{
		printf("read fmt error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->header.fmt_len, sizeof(NK_UInt32), 1, pwavst->fp))       /* fmt length */
	{
		printf("read fmt_len error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->header.tag, sizeof(NK_UInt16), 1, pwavst->fp))           /* tag : PCM or not */
	{
		printf("read tag error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->header.channels, sizeof(NK_UInt16), 1, pwavst->fp))      /* channels */
	{
		printf("read channels error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->header.samp_freq, sizeof(NK_UInt32), 1, pwavst->fp))      /* samp_freq */
	{
		printf("read samp_freq error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->header.byte_rate, sizeof(NK_UInt32), 1, pwavst->fp))      /* byte_rate : decode how many bytes per second */
	{                                                                       /* byte_rate = samp_freq * bit_samp */
		printf("read byte_rate error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->header.block_align, sizeof(NK_UInt16), 1, pwavst->fp))       /* quantize bytes for per samp point */
	{
		printf("read byte_samp error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->header.bit_samp, sizeof(NK_UInt16), 1, pwavst->fp))        /* quantize bits for per samp point */
	{                                                                        /* bit_samp = byte_samp * 8 */
		printf("read bit_samp error!\n");
		return -1;
	}

	/* jump to "data" for reading data */
	do
	{
		fread(&temp, sizeof(NK_Byte), 1, pwavst->fp);
	}
	while('d' != temp);
	pwavst->data_flag[0] = temp;
	if(3 != fread(&pwavst->data_flag[1], sizeof(NK_Byte), 3, pwavst->fp))                 /* data chunk */
	{
		printf("read header data error!\n");
		return -1;
	}
	if(1 != fread(&pwavst->length, sizeof(NK_UInt32), 1, pwavst->fp))                  /* data length */
	{
		printf("read length error!\n");
		return -1;
	}

	/* jduge data chunk flag */
	if(strncmp(pwavst->data_flag, "data", 4))
	{
		printf("error : cannot read data! data_flag:%s\n",pwavst->data_flag);
		return -1;
	}

	total_time = pwavst->length / pwavst->header.byte_rate;
	play_time.hour = (NK_Byte)(total_time / 3600);
	play_time.minute = (NK_Byte)((total_time / 60) % 60);
	play_time.second = (NK_Byte)(total_time % 60);
	/* printf file header information */

	printf("%s %ldHz %dbit, DataLen: %ld, Rate: %ld, Length: %2ld:%2ld:%2ld\n",
		   channel_mappings[pwavst->header.channels],             //����
		   pwavst->header.samp_freq,                              //����Ƶ��
		   pwavst->header.bit_samp,                               //ÿ�������������λ��
           pwavst->length,
		   pwavst->header.byte_rate,
		   play_time.hour,play_time.minute,play_time.second);


	return 0;
}

/**
 * ���������ļ�·����
 */
static NK_Char
_sound_file_path[128];

/**
 * �������ź�̨�̡߳�
 */
static NK_Thread *
_SoundThread = NK_Nil;


/**
 *
 * @param Thread
 * @param argc
 * @param argv
 * @return
 */


/**
 * ��̨�����̡߳�
 */
static NK_Void
play_sound(NK_Thread *const Thread, NK_Int argc, NK_PVoid argv[])
{
	HI_S32 s32Ret;
	AIO_ATTR_S stAttr;
	AUDIO_RESAMPLE_ATTR_S stReSampleAttr;
	wave_t wav;
	int nRet;
	AIO_ATTR_S hiAinAttr;
	FILE* fID = NULL;
#if !defined(M388C1G)
	HI_MPI_AI_GetPubAttr(0, &hiAinAttr);

	fID = fopen(_sound_file_path,"rb");
	if(NULL != fID) {
        BSP_Speaker_Enable(true);  // ��������

		NK_Byte pcm16BitBuf[16 * 1024];
		int const pcm16BitLength = hiAinAttr.u32PtNumPerFrm * 2;
		int readn = 0;

		readWaveHeader(fID, &wav);//��ȡwaveͷ��Ϣ

		stAttr.enBitwidth = AUDIO_BIT_WIDTH_16;//wav.header.bit_samp;////�����ֱ��� 8bits or 16bits
		stAttr.enSamplerate = wav.header.samp_freq;//������
		stAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
		stAttr.enWorkmode = AIO_MODE_I2S_MASTER;
		stAttr.u32EXFlag = 1;
		stAttr.u32FrmNum = 5;
		stAttr.u32PtNumPerFrm = hiAinAttr.u32PtNumPerFrm;
		stAttr.u32ChnCnt = 2;
		stAttr.u32ClkSel = 1;

		//readWaveHeader(fID,&wav);
		while (!Thread->terminate(Thread)
				&& (readn = fread(pcm16BitBuf, 1, pcm16BitLength, fID)) > 0) {
#if defined(VOICE_TALK)
			/*if(au_flag){
				fclose(fID);
				break;
			}*/
#endif
			AUDIO_FRAME_S audioFrame;
			NK_BZERO(&audioFrame, sizeof(audioFrame));
			audioFrame.pVirAddr[0] = pcm16BitBuf;
			audioFrame.u32Len = readn;
			audioFrame.enSoundmode = AUDIO_SOUND_MODE_MONO;
			audioFrame.enBitwidth = AUDIO_BIT_WIDTH_16;

			HI_MPI_AO_SendFrame(0, 0, &audioFrame, -1);
		}

		fclose(fID);
        sleep(1);
        BSP_Speaker_Enable(false);  // �ȴ�2���ر����ȣ���������û������ϼ��ر�����
	}

#endif
	/**
	 * �ڴ�
	 */
	_SoundThread = NK_Nil;
}


/**
 * ���� WAV �ļ���
 *
 */
NK_Int PlaySound(NK_PChar file_path, NK_Boolean block)
{
	/**
	 * �����Ϸ����жϡ�
	 */
	NK_EXPECT_VERBOSE_RETURN_VAL(NK_Nil != file_path, -1);
	NK_EXPECT_VERBOSE_RETURN_VAL(strlen(file_path) > 0, -1);
	NK_EXPECT_VERBOSE_RETURN_VAL(NK_Nil == _SoundThread, -1);

	/**
	 * ��¼��Ƶ�ļ�·����
	 */
	snprintf(_sound_file_path, sizeof(_sound_file_path),
			"%s", file_path);

	// Get Audio Attribute from Audio Input
	_SoundThread = NK_Thread_Create(NK_MemAlloc_OS(), NK_True, NK_True,
			play_sound, 0, NK_Nil);

	if (!_SoundThread) {
		return -1;
	}

	/**
	 * �����ȴ���������ֹͣ��
	 */
	if (block) {
		while (NK_Nil != _SoundThread) {
			usleep(10000);
		}
	}
	
	return 0;
}
