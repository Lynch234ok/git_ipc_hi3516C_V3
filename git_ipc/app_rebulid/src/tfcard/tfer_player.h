
#if 0
/**
 * TF ��¼�񲥷�ģ�����
 */

#include "NkEmbedded/tfer.h"
#include <NkUtils/json.h>

#if !defined(NK_TFER_PLAYER_H_)
# define NK_TFER_PLAYER_H_
NK_CPP_EXTERN_BEGIN


/**
 * ���� TFerPlayer ģ������
 */
extern NK_TFerPlayer *
NK_TFer_CreatePlayer(NK_Allocator *Alloctr, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx,
		NK_UTC1970 play_utc, NK_JSON *PlayFileList, NK_PVoid player_ctx);

/**
 * ���� TFerPlayer ģ������
 */
extern NK_Int
NK_TFer_FreePlayer(NK_TFerPlayer **TFerPlayer_r);


NK_CPP_EXTERN_END
#endif /* NK_TFER_PLAYER_H_ */

#endif
