#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "hi_comm_aio.h"
#include "mpi_ao.h"

#include "sound_queue.h"
#include <sys/prctl.h>
#include "base/cross_thread.h"

// audio in and out
#define HI_AIN_DEV  (0)
#define HI_AOUT_DEV (0)
#define HI_AOUT_CH  (0)

#define QUEUE_MAX_COUNT     1024


typedef struct QUEUE
{
    unsigned int qbSize, qbCount;
    unsigned int readPos, writePos;
    unsigned int dataInfoReadPos, dataInfoWritePos;
}stQUEUE, *lpQUEUE;

typedef struct DATA_INFO
{
    unsigned int dataLen;
    emSOUND_DATA_TYPE dataType;
}stDATA_INFO, *lpDATA_INFO;

typedef struct PTHREAD_CTRL
{
    pthread_mutex_t lock;
    pthread_cond_t notempty;
    pthread_cond_t notfull;
    pthread_t ptPid;
    bool ptRun;

}stPTHREAD_CTRL, *lpPTHREAD_CTRL;

static unsigned char *queueBuffer = NULL;  // buffer先暂定这样定义
static stDATA_INFO gs_dataInfo[QUEUE_MAX_COUNT];

static stQUEUE gs_queue;
static stPTHREAD_CTRL gs_ptCtrl;

static emSOUND_PRIORITY gs_curPriority = emSOUND_PRIORITY_NULL;
static stSOUNG_QUEUE_API gs_sqApi;
static short ALawDecompressTable[] =
{
    (short)0xEA80, (short)0xEB80, (short)0xE880, (short)0xE980, (short)0xEE80, (short)0xEF80, (short)0xEC80, (short)0xED80,
    (short)0xE280, (short)0xE380, (short)0xE080, (short)0xE180, (short)0xE680, (short)0xE780, (short)0xE480, (short)0xE580,
    (short)0xF540, (short)0xF5C0, (short)0xF440, (short)0xF4C0, (short)0xF740, (short)0xF7C0, (short)0xF640, (short)0xF6C0,
    (short)0xF140, (short)0xF1C0, (short)0xF040, (short)0xF0C0, (short)0xF340, (short)0xF3C0, (short)0xF240, (short)0xF2C0,
    (short)0xAA00, (short)0xAE00, (short)0xA200, (short)0xA600, (short)0xBA00, (short)0xBE00, (short)0xB200, (short)0xB600,
    (short)0x8A00, (short)0x8E00, (short)0x8200, (short)0x8600, (short)0x9A00, (short)0x9E00, (short)0x9200, (short)0x9600,
    (short)0xD500, (short)0xD700, (short)0xD100, (short)0xD300, (short)0xDD00, (short)0xDF00, (short)0xD900, (short)0xDB00,
    (short)0xC500, (short)0xC700, (short)0xC100, (short)0xC300, (short)0xCD00, (short)0xCF00, (short)0xC900, (short)0xCB00,
    (short)0xFEA8, (short)0xFEB8, (short)0xFE88, (short)0xFE98, (short)0xFEE8, (short)0xFEF8, (short)0xFEC8, (short)0xFED8,
    (short)0xFE28, (short)0xFE38, (short)0xFE08, (short)0xFE18, (short)0xFE68, (short)0xFE78, (short)0xFE48, (short)0xFE58,
    (short)0xFFA8, (short)0xFFB8, (short)0xFF88, (short)0xFF98, (short)0xFFE8, (short)0xFFF8, (short)0xFFC8, (short)0xFFD8,
    (short)0xFF28, (short)0xFF38, (short)0xFF08, (short)0xFF18, (short)0xFF68, (short)0xFF78, (short)0xFF48, (short)0xFF58,
    (short)0xFAA0, (short)0xFAE0, (short)0xFA20, (short)0xFA60, (short)0xFBA0, (short)0xFBE0, (short)0xFB20, (short)0xFB60,
    (short)0xF8A0, (short)0xF8E0, (short)0xF820, (short)0xF860, (short)0xF9A0, (short)0xF9E0, (short)0xF920, (short)0xF960,
    (short)0xFD50, (short)0xFD70, (short)0xFD10, (short)0xFD30, (short)0xFDD0, (short)0xFDF0, (short)0xFD90, (short)0xFDB0,
    (short)0xFC50, (short)0xFC70, (short)0xFC10, (short)0xFC30, (short)0xFCD0, (short)0xFCF0, (short)0xFC90, (short)0xFCB0,
    (short)0x1580, (short)0x1480, (short)0x1780, (short)0x1680, (short)0x1180, (short)0x1080, (short)0x1380, (short)0x1280,
    (short)0x1D80, (short)0x1C80, (short)0x1F80, (short)0x1E80, (short)0x1980, (short)0x1880, (short)0x1B80, (short)0x1A80,
    (short)0x0AC0, (short)0x0A40, (short)0x0BC0, (short)0x0B40, (short)0x08C0, (short)0x0840, (short)0x09C0, (short)0x0940,
    (short)0x0EC0, (short)0x0E40, (short)0x0FC0, (short)0x0F40, (short)0x0CC0, (short)0x0C40, (short)0x0DC0, (short)0x0D40,
    (short)0x5600, (short)0x5200, (short)0x5E00, (short)0x5A00, (short)0x4600, (short)0x4200, (short)0x4E00, (short)0x4A00,
    (short)0x7600, (short)0x7200, (short)0x7E00, (short)0x7A00, (short)0x6600, (short)0x6200, (short)0x6E00, (short)0x6A00,
    (short)0x2B00, (short)0x2900, (short)0x2F00, (short)0x2D00, (short)0x2300, (short)0x2100, (short)0x2700, (short)0x2500,
    (short)0x3B00, (short)0x3900, (short)0x3F00, (short)0x3D00, (short)0x3300, (short)0x3100, (short)0x3700, (short)0x3500,
    (short)0x0158, (short)0x0148, (short)0x0178, (short)0x0168, (short)0x0118, (short)0x0108, (short)0x0138, (short)0x0128,
    (short)0x01D8, (short)0x01C8, (short)0x01F8, (short)0x01E8, (short)0x0198, (short)0x0188, (short)0x01B8, (short)0x01A8,
    (short)0x0058, (short)0x0048, (short)0x0078, (short)0x0068, (short)0x0018, (short)0x0008, (short)0x0038, (short)0x0028,
    (short)0x00D8, (short)0x00C8, (short)0x00F8, (short)0x00E8, (short)0x0098, (short)0x0088, (short)0x00B8, (short)0x00A8,
    (short)0x0560, (short)0x0520, (short)0x05E0, (short)0x05A0, (short)0x0460, (short)0x0420, (short)0x04E0, (short)0x04A0,
    (short)0x0760, (short)0x0720, (short)0x07E0, (short)0x07A0, (short)0x0660, (short)0x0620, (short)0x06E0, (short)0x06A0,
    (short)0x02B0, (short)0x0290, (short)0x02F0, (short)0x02D0, (short)0x0230, (short)0x0210, (short)0x0270, (short)0x0250,
    (short)0x03B0, (short)0x0390, (short)0x03F0, (short)0x03D0, (short)0x0330, (short)0x0310, (short)0x0370, (short)0x0350,
};

static void sound_clearQueue(void);
static void *sound_playPro(void *arg);

static int sound_initQueue(lpSOUNG_QUEUE_API sqApi, lpPTHREAD_CTRL ptCtrl, lpQUEUE queue, unsigned int size)
{
    queueBuffer = (unsigned char *)malloc(sizeof(unsigned char) * size);
    if(queueBuffer == NULL)
    {
        printf("SOUND_Err:create mem fail!\n");
        return -1;
    }
    memset(queueBuffer, 0, sizeof(unsigned char) * size);

    pthread_mutex_init(&ptCtrl->lock, NULL);
    pthread_cond_init(&ptCtrl->notempty, NULL);
    pthread_cond_init(&ptCtrl->notfull, NULL);
    ptCtrl->ptRun = false;
    queue->qbSize = size;
    queue->qbCount = 0;
    queue->readPos = 0;
    queue->writePos = 0;
    queue->dataInfoReadPos = 0;
    queue->dataInfoWritePos = 0;

    memset(&gs_sqApi, 0, sizeof(stSOUNG_QUEUE_API));
    gs_sqApi.playSound = sqApi->playSound;
    gs_sqApi.playG711A = sqApi->playG711A;
    gs_sqApi.isAoBufFree = sqApi->isAoBufFree;
    gs_sqApi.speakerEnable = sqApi->speakerEnable;

    return 0;

}

int SOUND_initQueue(lpSOUNG_QUEUE_API sqApi, unsigned int qbSize)
{
    int ret;

    sound_initQueue(sqApi, &gs_ptCtrl, &gs_queue, qbSize);

    gs_ptCtrl.ptRun = true;
    ret = JA_THREAD_init0(&gs_ptCtrl.ptPid, sound_playPro, NULL, NULL, 0, NULL, 131072, 0);
    if(ret != 0)
    {
        printf("SOUND_Err:pthread create fail!\n");
        return -1;
    }

    return 0;

}

void * SOUND_QueueisInit()
{
	if(gs_ptCtrl.ptRun){
		return (void *)&gs_queue;
	}
	return NULL;
}

void sound_writeQueue(const unsigned char *data, unsigned int size, emSOUND_DATA_TYPE type)
{
    if(size >= gs_queue.qbSize) {
        printf("%s:%d Audio packet is too large! size: %u, max: %u\n",
               __FUNCTION__, __LINE__, size, gs_queue.qbSize);
    } else if (NULL == data) {
        printf("%s:%d write data can't be NULL!\n",
               __FUNCTION__, __LINE__);
    } else {
        unsigned int len;
        // get the right write position

        // if reach end of buffer, write start at begin of buffer
        if (gs_queue.writePos >= gs_queue.readPos
            && (gs_queue.writePos + size) <= gs_queue.qbSize) {
            // |--------r+++++++++++++++w--------| write at right

//            printf("%s:%d gs_queue.writePos: %u, +size: %u\n",
//                   __FUNCTION__, __LINE__, gs_queue.writePos, gs_queue.writePos + size);
            ;
        } else {
            if (gs_queue.writePos >= gs_queue.readPos) {
                // |--------r+++++++++++++++w--------| write at left
                gs_queue.writePos = 0;
            } else {
                // |++++++++w---------------r++++++++|
                ;
            }

            while (1) {

                if ((gs_queue.writePos + size) < gs_queue.readPos) {

//                    printf("%s:%d gs_queue.writePos: %u, +size: %u\n",
//                           __FUNCTION__, __LINE__, gs_queue.writePos, gs_queue.writePos + size);
                    break;
                } else {
                    if (gs_queue.qbCount <= 0) {
                        gs_queue.qbCount = 0;
                        gs_queue.readPos = 0;

                        gs_queue.writePos = 0;

//                        printf("%s:%d gs_queue.writePos: %u, +size: %u\n",
//                               __FUNCTION__, __LINE__, gs_queue.writePos, gs_queue.writePos + size);
                        break;

                    } else {
//                    printf("SOUND_Mes:queue wait for not full\n");
//                    pthread_cond_wait(&gs_ptCtrl.notfull, &gs_ptCtrl.lock);

                        // if buffer is full, discard old audio packet

                        printf("%s:%d Buffer is full, discard old audio packet!\n",
                               __FUNCTION__, __LINE__);

                        // get the right read position
                        if(gs_queue.dataInfoReadPos >= QUEUE_MAX_COUNT) {
                            gs_queue.dataInfoReadPos = 0;
                        }
                        len = gs_dataInfo[gs_queue.dataInfoReadPos].dataLen;
                        if (gs_queue.readPos + len > gs_queue.qbSize) {
                            gs_queue.readPos = 0;
                        }

                        // decrease queue
                        gs_queue.readPos += len;
                        gs_queue.dataInfoReadPos++;

                        gs_queue.qbCount--;
                    }
                }
            }
        }

        if (gs_queue.dataInfoWritePos >= QUEUE_MAX_COUNT) {
            gs_queue.dataInfoWritePos = 0;
        }


        // put data
        memcpy(queueBuffer + gs_queue.writePos, data, size);
        gs_dataInfo[gs_queue.dataInfoWritePos].dataLen = size;
        gs_dataInfo[gs_queue.dataInfoWritePos].dataType= type;

        // increase queue
        gs_queue.writePos += size;
        gs_queue.dataInfoWritePos++;

        gs_queue.qbCount++;

    }

    pthread_cond_signal(&gs_ptCtrl.notempty);
}

int SOUND_writeQueue_lock()
{
	return pthread_mutex_lock(&gs_ptCtrl.lock);
}

int SOUND_writeQueue_unlock()
{
	return pthread_mutex_unlock(&gs_ptCtrl.lock);
}

int SOUND_writeQueue(const unsigned char *data, unsigned int size, emSOUND_DATA_TYPE type, emSOUND_PRIORITY pri)
{
	if(SOUND_writeQueue_lock() == 0){
		SOUND_writeQueue2(data, size, type, pri);
		SOUND_writeQueue_unlock();
		return 0;
	}

	return -1;
}

int SOUND_writeQueue2(const unsigned char *data, unsigned int size, emSOUND_DATA_TYPE type, emSOUND_PRIORITY pri)
{
    /* 如果当前是处于emSOUND_PRIORITY_ZERO优先级，
        当参数pri为其它优先级时，则直接退出 */
    if((pri != emSOUND_PRIORITY_ZERO)
            && (gs_curPriority == emSOUND_PRIORITY_ZERO)) {
        printf("SOUND_Mes:Very sorry!Current in the highest priority\n");
        pthread_mutex_unlock(&gs_ptCtrl.lock);
        return -1;
    }

    /* 如果优先级为emSOUND_PRIORITY_ZERO，则清空非优先级为emSOUND_PRIORITY_ZERO的队列 */
    if((pri == emSOUND_PRIORITY_ZERO)
            && (gs_curPriority != emSOUND_PRIORITY_ZERO)) {
        sound_clearQueue();
    }
    gs_curPriority = pri;

    sound_writeQueue(data, size, type);

    /* 非emSOUND_PRIORITY_ZERO优先级，每次写完数据都会释放优先级 */
    if(gs_curPriority != emSOUND_PRIORITY_ZERO) {
        gs_curPriority = emSOUND_PRIORITY_NULL;
    }

    return 0;

}

static unsigned char *sound_readQueue(lpDATA_INFO dataInfo)
{
    unsigned char *tmpData = NULL;
    unsigned int len;


    // if there is no data in queue, return NULL
    if (gs_queue.qbCount <= 0) {
        return NULL;
    }

    // get the right read position
    if(gs_queue.dataInfoReadPos >= QUEUE_MAX_COUNT) {
        gs_queue.dataInfoReadPos = 0;
    }
    len = gs_dataInfo[gs_queue.dataInfoReadPos].dataLen;
    if (gs_queue.readPos + len > gs_queue.qbSize) {
        gs_queue.readPos = 0;
    }

//    printf("%s:%d reading gs_queue.readPos: %u, +len: %u\n",
//           __FUNCTION__, __LINE__,gs_queue.readPos, gs_queue.readPos + len);

    // get data
    dataInfo->dataLen = len;
    dataInfo->dataType = gs_dataInfo[gs_queue.dataInfoReadPos].dataType;
    tmpData = queueBuffer + gs_queue.readPos;

    // decrease queue
    gs_queue.readPos += len;
    gs_queue.dataInfoReadPos++;

    gs_queue.qbCount--;

    pthread_cond_signal(&gs_ptCtrl.notfull);
    return tmpData;
}

/* 此函数暂时没设置使用权 */
void SOUND_releasePriority(void)
{
    gs_curPriority = emSOUND_PRIORITY_NULL;

}

void SOUND_releaseQueue(void)
{
    if(gs_ptCtrl.ptPid) {
        gs_ptCtrl.ptRun = false;
        pthread_join(gs_ptCtrl.ptPid, NULL);
        gs_ptCtrl.ptPid = (pthread_t)NULL;
    }

    free(queueBuffer);
    queueBuffer = NULL;
    pthread_mutex_destroy(&gs_ptCtrl.lock);
    pthread_cond_destroy(&gs_ptCtrl.notfull);
    pthread_cond_destroy(&gs_ptCtrl.notempty);

    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);

}

static void sound_clearQueue(void)
{
    int i = 0;

    memset(queueBuffer, 0, sizeof(unsigned char) * gs_queue.qbSize);
    for(i = 0; i < QUEUE_MAX_COUNT; i++)
    {
        gs_dataInfo[i].dataLen = 0;
        gs_dataInfo[i].dataType = emSOUND_DATA_TYPE_NULL;
    }
    gs_queue.writePos = 0;
    gs_queue.readPos = 0;

}

static void *sound_playPro(void *arg)
{
    unsigned char *data;
    stDATA_INFO dataInfo;
    int isBufFree = 0;
    bool playing = false;

    const int wait_us = 500000;
    const unsigned int sleep_time_us = 20000;
    int remain_us = wait_us;

    uint8_t *play_buf;
    const ssize_t play_buf_sz = 4096;

    play_buf = malloc(play_buf_sz);
    if (NULL == play_buf) {
        printf("%s:%d Failed to malloc play buffer!\n", __FUNCTION__, __LINE__);
        pthread_exit(NULL);
    }
	prctl(PR_SET_NAME, "sound_playPro");

    while(gs_ptCtrl.ptRun) {

        pthread_mutex_lock(&gs_ptCtrl.lock);

        data = sound_readQueue(&dataInfo);
        if(data == NULL) {
            pthread_mutex_unlock(&gs_ptCtrl.lock);
            if(playing) {
                if(gs_sqApi.isAoBufFree != NULL) {
                    isBufFree = gs_sqApi.isAoBufFree();
                    if(isBufFree <= 0) {
                        if (remain_us <= 0) {
                            if(gs_sqApi.speakerEnable != NULL) {
                                gs_sqApi.speakerEnable(false);
                                playing = false;
                            }
                        } else {
                            remain_us -= sleep_time_us;
                        }
                    } else {
                        remain_us = wait_us;
                    }
                }
            }
            usleep(sleep_time_us);

        } else {

            if (dataInfo.dataLen > play_buf_sz) {
                printf("%s:%d Audio packet too large, ignore it! size: %u, buf size: %lu\n",
                       __FUNCTION__, __LINE__, dataInfo.dataLen, play_buf_sz);
                pthread_mutex_unlock(&gs_ptCtrl.lock);

            } else {

                memcpy(play_buf, data, dataInfo.dataLen);
                pthread_mutex_unlock(&gs_ptCtrl.lock);

                playing = true;
                if(gs_sqApi.speakerEnable != NULL) {
                    gs_sqApi.speakerEnable(true);
                }

                if(dataInfo.dataType == emSOUND_DATA_TYPE_WAV_FILE) {
                    if(gs_sqApi.playSound != NULL) {
                        gs_sqApi.playSound(play_buf, dataInfo.dataLen);
                    }
                }
                else if(dataInfo.dataType == emSOUND_DATA_TYPE_P2P_G711A) {
                    if(gs_sqApi.playG711A != NULL) {
                        gs_sqApi.playG711A(play_buf, ALawDecompressTable, dataInfo.dataLen);
                    }
                }
            }
        }
    }

    free(play_buf);
    pthread_exit(NULL);
}

