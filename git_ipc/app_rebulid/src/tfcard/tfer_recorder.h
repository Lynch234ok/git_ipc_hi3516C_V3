
#if 0
/**
 * TF 卡录像写入模块抽象。
 */

#include "NkEmbedded/tfer.h"

#if !defined(NK_TFER_RECORDER_H_)
# define NK_TFER_RECORDER_H_
NK_CPP_EXTERN_BEGIN

/**
 * 创建 TFerRecorder 模块句柄。
 */
extern NK_TFerRecorder *
NK_TFer_CreateRecorder(NK_Allocator *Alloctr, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx,
		NK_PChar type, NK_PChar fs_path, NK_PVoid recorder_ctx);

/**
 * 销毁 TFerRecorder 模块句柄。
 */
extern NK_Int
NK_TFer_FreeRecorder(NK_TFerRecorder **TFerRecorder_r);


NK_CPP_EXTERN_END
#endif /* NK_TFER_RECORDER_H_ */

#endif
