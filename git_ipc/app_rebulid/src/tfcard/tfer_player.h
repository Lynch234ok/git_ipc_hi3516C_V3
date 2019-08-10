
#if 0
/**
 * TF 卡录像播放模块抽象。
 */

#include "NkEmbedded/tfer.h"
#include <NkUtils/json.h>

#if !defined(NK_TFER_PLAYER_H_)
# define NK_TFER_PLAYER_H_
NK_CPP_EXTERN_BEGIN


/**
 * 创建 TFerPlayer 模块句柄。
 */
extern NK_TFerPlayer *
NK_TFer_CreatePlayer(NK_Allocator *Alloctr, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx,
		NK_UTC1970 play_utc, NK_JSON *PlayFileList, NK_PVoid player_ctx);

/**
 * 销毁 TFerPlayer 模块句柄。
 */
extern NK_Int
NK_TFer_FreePlayer(NK_TFerPlayer **TFerPlayer_r);


NK_CPP_EXTERN_END
#endif /* NK_TFER_PLAYER_H_ */

#endif
