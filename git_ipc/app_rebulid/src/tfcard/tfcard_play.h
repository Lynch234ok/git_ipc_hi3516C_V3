/**
 * TF ¿¨Â¼ÏñĞ´ÈëÄ£¿é³éÏó¡£
 */

#include "tfcard.h"

#if !defined(NK_TFER_PLAY_H_)
# define NK_TFER_PLAY_H_
#ifdef __cplusplus
extern "C" {
#endif

#define NK_REC_TIMER	(1<<0)
#define NK_REC_MOTION	(1<<1)
#define NK_REC_ALARM	(1<<2)
#define NK_REC_MANUAL	(1<<3)

extern NK_Int TFCARD_get_history(NK_UTC1970 beginUtc, NK_UTC1970 endUtc,
                                 NK_PChar type,
                                 lpTFCARD_History_List historyList,
                                 NK_Int startIndex,
                                 NK_Int *historyCnt,
                                 NK_PChar mountPath);
extern NK_Int TFcard_Play_read_frame(lpRecord_Frame_Head frameHead, NK_PByte data, NK_Size dataMaxSize);
extern NK_Int TFCARD_Play_start(NK_UTC1970 beginUtc, NK_PChar playType, NK_PChar mountPath);
extern NK_Int TFCARD_Play_stop();

extern int REC_PLAY_get_history(time_t beginTs, time_t endTs,
                                char recType, int startIndex,
                                lpTFCARD_History_List recList, size_t *pRecListSize);
extern void * REC_PLAY_start(time_t beginTs, time_t endTs, char recType, int openType);
extern int REC_PLAY_stop(void * const ctx);
extern ssize_t REC_PLAY_read_frame(void * const ctx,
                                   lpRecord_Frame_Head pFrameHead,
                                   uint8_t *outBuf, size_t outBufSize);
extern int REC_PLAY_get_date_list(char ***p_list);

#ifdef __cplusplus
};
#endif
#endif /* NK_TFER_RECORDER_H_ */
