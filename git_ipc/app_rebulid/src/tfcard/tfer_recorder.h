
#if 0
/**
 * TF ��¼��д��ģ�����
 */

#include "NkEmbedded/tfer.h"

#if !defined(NK_TFER_RECORDER_H_)
# define NK_TFER_RECORDER_H_
NK_CPP_EXTERN_BEGIN

/**
 * ���� TFerRecorder ģ������
 */
extern NK_TFerRecorder *
NK_TFer_CreateRecorder(NK_Allocator *Alloctr, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx,
		NK_PChar type, NK_PChar fs_path, NK_PVoid recorder_ctx);

/**
 * ���� TFerRecorder ģ������
 */
extern NK_Int
NK_TFer_FreeRecorder(NK_TFerRecorder **TFerRecorder_r);


NK_CPP_EXTERN_END
#endif /* NK_TFER_RECORDER_H_ */

#endif
