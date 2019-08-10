
#include "n1_device_2waytalk.h"
#include <NkEmbedded/http_websocket.h>
#include <NkUtils/json.h>
#include <NkUtils/macro.h>
#include <n1/n1_device_twowaytalk.h>
#include "sound_queue.h"
#include "mutex_f.h"
#include "global_runtime.h"

static NK_Boolean
TwoWayTalkOnCalled(NK_PVoid ctx, NK_Int chid)
{
#if defined(VOICE_TALK)
	printf("FUNC:TwoWayTalkOnCalled...... chnid:%d\n", chid);
	return GLOBAL_enter_twowaytalk() == 0 ? NK_True : NK_False;
#endif
	return NK_False;
}

static NK_Void
TwoWayTalkOnRecv(NK_PVoid ctx, NK_Int chid, NK_N1MediaPackage *Package, NK_PByte data, NK_Size len)
{
#if defined(VOICE_TALK)
	SOUND_writeQueue((unsigned char *)data, len, emSOUND_DATA_TYPE_P2P_G711A, emSOUND_PRIORITY_ZERO);
#endif
	return;
}


static NK_Void
TwoWayTalkOnHungUp(NK_PVoid ctx, NK_Int chid)
{

#if defined(VOICE_TALK)
    printf("N1 TwoWayTalk Hangup......%p\n", ctx);
	GLOBAL_leave_twowaytalk();
    SOUND_releasePriority();
#endif
	return;
}

NK_N1Error
onLiveConnected(NK_N1LiveSession *Session, NK_PVoid ctx)
{
	return NK_N1_ERR_UNDEF;
}

NK_N1Error
onLiveDisconnected(NK_N1LiveSession *Session, NK_PVoid ctx)
{
	return NK_N1_ERR_UNDEF;
}

NK_SSize
onLiveReadFrame(NK_N1LiveSession *Session, NK_PVoid ctx, NK_N1DataPayload *payload_type, NK_UInt32 *ts_ms, NK_PByte *data_r)
{
	return 0;
}

NK_N1Error
onLanSetup(NK_PVoid ctx, NK_Boolean is_set, NK_N1LanSetup *LanSetup)
{
	return NK_N1_ERR_UNDEF;
}

NK_Int
NK_N1Device_TwoWayTalk_init()
{
	NK_N1Device N1Device;
	NK_BZERO(&N1Device, sizeof(N1Device));
	/** N1Device_Init 初始化使用外部缓存，默认需要注册以下回调接口 */
	N1Device.EventSet.onLiveConnected = onLiveConnected;
	N1Device.EventSet.onLiveDisconnected = onLiveDisconnected;
	N1Device.EventSet.onLiveReadFrame = onLiveReadFrame;
	N1Device.EventSet.onLanSetup = onLanSetup;
	/** 初始化N1设备，配合双向语音的使用 */
	if (0 != N1Device_Init("JUAN", 0, &N1Device))
	{
		exit(EXIT_FAILURE);
	}

	printf("-%s-\n",__FUNCTION__);
	NK_N1DeviceEventTwoWayTalk Event;
        NK_BZERO(&Event, sizeof(Event));
	Event.onCalled = TwoWayTalkOnCalled;	
	Event.onRecv  = TwoWayTalkOnRecv;
	Event.onHungUp = TwoWayTalkOnHungUp;
	
	NK_N1Device_TwoWayTalk(&Event);

}




