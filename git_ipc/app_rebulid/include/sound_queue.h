#ifndef SOUND_THREAD_H_
#define SOUND_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    emSOUND_DATA_TYPE_P2P_G711A = 0,
    emSOUND_DATA_TYPE_WAV_FILE,
    emSOUND_DATA_TYPE_SPX,
    emSOUND_DATA_TYPE_NULL
}emSOUND_DATA_TYPE;

/* 如果设置了 emSOUND_PRIORITY_ZERO优先级，需要调用清除优先级函数 */
typedef enum
{
    emSOUND_PRIORITY_ZERO = 0,
    emSOUND_PRIORITY_FIRST,
    emSOUND_PRIORITY_NULL
}emSOUND_PRIORITY;

typedef struct SOUNG_QUEUE_API
{
    void (*playSound)(const void *data, unsigned int dataLen);
    void (*playG711A)(const unsigned char *g711aData, const short *table, int g711aBytes);
    int (*isAoBufFree)(void);
    void (*speakerEnable)(bool enable);
}stSOUNG_QUEUE_API, *lpSOUNG_QUEUE_API;

extern int SOUND_initQueue(lpSOUNG_QUEUE_API sqApi, unsigned int qbSize);

extern void * SOUND_QueueisInit();

extern int SOUND_writeQueue_lock();

extern int SOUND_writeQueue_unlock();

extern int SOUND_writeQueue2(const unsigned char *data, unsigned int size, emSOUND_DATA_TYPE type, emSOUND_PRIORITY pri);  // lock outside

extern int SOUND_writeQueue(const unsigned char *data, unsigned int size, emSOUND_DATA_TYPE type, emSOUND_PRIORITY pri);  //lock inside

extern void SOUND_releasePriority(void);

extern void SOUND_releaseQueue(void);

#ifdef __cplusplus
}
#endif

#endif // SOUND_THREAD_H_

