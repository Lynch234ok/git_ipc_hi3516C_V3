/**
 * TF ¿¨Â¼ÏñÐ´ÈëÄ£¿é³éÏó¡£
 */

#include <tfcard.h>

#if !defined(NK_TFCARD_RECORDER_H_)
# define NK_TFCARD_RECORDER_H_
#ifdef __cplusplus
extern "C" {
#endif


extern NK_Int TFcard_Record_init(NK_PChar record_type,fTFcardOnGetFreeSpace getFreeSpace,NK_PChar mountPath,NK_Size maxBufferSizeKb);
extern NK_Int TFcard_Record_write_frame(lpRecord_Frame_Head frameHead,NK_PByte data);
extern NK_Int TFcard_Record_stop();


#ifdef __cplusplus
};
#endif
#endif /* NK_TFCARD_RECORDER_H_ */

