
/**
 * N1SDK for IP Camra of IP Camera Like Device Interfaces.
 * Source File Text Encoding UTF-8.
 *
 */

/**
 * Reference Headers.\n
 */
#include <NkUtils/types.h>
#if defined(_WIN32)
# include <Windows.h>
#else
#endif

/**
 * Recursion Includes Guide.
 */
#if !defined(NK_N1_IPCAM_H_)
#define NK_N1_IPCAM_H_
NK_CPP_EXTERN_BEGIN


/**
 * @macro
 *  Maximal Channel Number.\n
 */
#define NK_IPCAM_MAX_CH (1)

/**
 * @macro
 *  Maximal Stream Number.\n
 */
#define NK_IPCAM_MAX_STREAM (2)

/**
 * @macro
 *  The License File Maximal Size.\n
 */
#define NK_IPCAM_MAX_LICENSE_SZ (1024 * 4)

/**
 * @macro
 *  The Module Internal Buffering Minimal Heap Size.\n
 */
#define NK_IPCAM_MIN_HEAP_MEM (1024 * 512)


/**
 * @macro
 *  The Module Listen Port Range Lower Value.\n
 */
#define NK_IPCAM_PORT_LOWER (10000)

/**
 * @macro
 *  The Module Listen Port Range Upper Value.\n
 */
#define NK_IPCAM_PORT_UPPER (60000)

/**
 * @macro
 *  The Maximal Size of Every Picture.\n
 */
#define NK_IPCAM_MAX_JPEG_SZ (1024 * 128)

/**
 * @macro
 *  The Maximal Bytes of String Property.\n
 */
#define NK_IPCAM_MAX_PROP_STR_SZ (1024)

/**
 * @macro
 *  The Maximal H.264 GOP ( Frames per Group ) Support.\n
 */
#define NK_IPCAM_H264_MAX_GOP (180)

/**
 * @macro
 *  The Maximal HEVC GOP ( Frames per Group ) Support.\n
 */
#define NK_IPCAM_HEVC_MAX_GOP (180)

/**
 * @macro
 *  The Maximal Main Stream on Demand Connections.\n
 */
#define NK_IPCAM_MAIN_STREAM_MAX_CONN (4)

/**
 * @macro
 *  The Maximal Sub Stream on Demand Connections.\n
 */
#define NK_IPCAM_SUB_STREAM_MAX_CONN (8)



/**
 * @brief
 *  Module Event Handlers Set.\n
 *
 */
typedef struct Nk_IPCamEvent {

	/**
	 * @brief
	 *
	 * @param event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * @param[in] chid
	 *  The Input Channel ID of This Media Stream.\n
	 *
	 * @param[in] width
	 *  The Width of Snapshot Image Size.\n
	 *  If 0 Size Indicates Width is Negligible.\n
	 *
	 * @param[in] height
	 *  The Height of Snapshot Image Size.\n
	 *  If 0 Size Indicates Height is Negligible.\n
	 *
	 * @param[in] options
	 *  Reserved, Always 0.\n
	 *
	 */
	NK_Void (*onSnapshot)
	(NK_PVoid event_ctx,
			NK_Int sessionid, NK_Int chid, NK_Size width, NK_Size height, NK_UInt32 options);


	/**
	 * @brief
	 *  Start Stream Event.\n
	 *  When the Media Stream ( Live or Playback ) Reqeusted, This Event Would Active.\n
	 *
	 * @param event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * @param[in] chid
	 *  The Input Channel ID of This Media Stream.\n
	 *
	 * @param[in] streamid
	 *  The Stream ID of This Media Stream.\n
	 *  If 0 Indicates the Main Stream Requested.\n
	 *  If 1 Indicates the Sub Stream Requested.\n
	 *
	 * @param[in] options
	 *  Reserved, Always 0.\n
	 *
	 */
	NK_Void (*onStartMedia)
	(NK_PVoid event_ctx,
			NK_Int sessionid, NK_Int chid, NK_Int streamid, NK_UInt32 options);


	/**
	 * @brief
	 *  Stop Media Event.\n
	 *  When the Media Stream ( Live or Playback ) Reqeusts Quit, This Event Would Active.\n
	 *
	 * @param event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * param[in] sessionid
	 *  The Session ID.\n
	 *  0 -- Indicates This Media Session is Live.\n
	 *  Others -- Indicates This Media Session is Playback, This Value is the Playback Session ID.\n
	 *
	 * @param[in] chid
	 *  The Input Channel ID of This Media Stream.\n
	 *
	 * @param[in] streamid
	 *  The Stream ID of This Media Stream.\n
	 *  If 0 Indicates the Main Stream Requested.\n
	 *  If 1 Indicates the Sub Stream Requested.\n
	 *
	 * @param[in] options
	 *  Reserved, Always 0.\n
	 *
	 */
	NK_Void (*onStopMedia)
	(NK_PVoid event_ctx,
			NK_Int sessionid, NK_Int chid, NK_Int streamid, NK_UInt32 options);


	/**
	 * @brief
	 * 	Get The Ether Configuration Event.\n
	 * 	When Module Wants the Configuration of Ether, It Would Active This Event.\n
	 *
	 * @param[in] event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * @param[in] wifi
	 * 	Wi-Fi Ether Flag.\n
	 *
	 * @param[out] hwaddr
	 * 	Ether Hardware Address, Format Like %02x:%02x:%02x:%02x:%02x:%02x.\n
	 *
	 * @param[out] dhcp
	 *  DHCP Actived Flag, if TRUE indicated Use DHCP Addressing on This Ether.\n
	 *  Otherwise Use Static IP Addressing on This Ether.\n
	 *
	 * @param[out] ipaddr
	 *  The IP Address of This Ether.\n
	 *
	 * @param[out] netmask
	 *  The IP Netmask of This Ether.\n
	 *
	 * @param[ont] gateway
	 *  The Gateway of This Ether.\n
	 *  If NOT Use Gateway on This Ether, Leave it "0.0.0.0" or "(null)".\n
	 *
	 * @param[out] essid
	 *  The Essid of Wi-Fi Connection on This Ether If Wi-Fi Supported, @ref wifi Indicates,\n
	 *  NOT Use When @ref wifi FALSE.\n
	 *
	 * @param[out] psk
	 *  The Verify Password of Wi-Fi Connection on This Ether If Wi-Fi Supported, @ref wifi Indicates,\n
	 *  NOT Use When @ref wifi FALSE.\n
	 *
	 * @param[in] options
	 *  The Optional Flag, Reserved.\n
	 *
	 */
	NK_Void (*onGetEtherCfg)
	(NK_PVoid event_ctx,
			NK_Boolean wifi,
			NK_Char hwaddr[32],
			NK_Boolean *dhcp,
			NK_Char ipaddr[32], NK_Char netmask[32], NK_Char gateway[32], NK_Char nameserver[32],
			NK_Char essid[128], NK_Char psk[128], NK_UInt32 options);

	/**
	 * @brief
	 * 	Setup The Ether Card Configuration Event.\n
	 * 	When Module Sets the Configuration of Ether, It Would Active This Event.\n
	 *
	 * @param[in] event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * @param[in] wifi
	 * 	Wi-Fi Ether Flag, if TRUE, Indicates This Setup is for Wi-Fi Ether.\n
	 *
	 * @param[in] dhcp
	 *  Check Event @ref NK_IPCamEvent::onGetEtherCfg.\n
	 *
	 * @param[in] ipaddr
	 *  Check Event @ref NK_IPCamEvent::onGetEtherCfg.\n
	 *
	 * @param[in] netmask
	 *  Check Event @ref NK_IPCamEvent::onGetEtherCfg.\n
	 *
	 * @param[in] gateway
	 *  Check Event @ref NK_IPCamEvent::onGetEtherCfg.\n
	 *
	 * @param[in] essid
	 *  Check Event @ref NK_IPCamEvent::onGetEtherCfg.\n
	 *
	 * @param[in] psk
	 * 	Check Event @ref NK_IPCamEvent::onGetEtherCfg.\n
	 *
	 * @param[in] options
	 *  The Optional Flag, Reserved.\n
	 *
	 */
	NK_Void (*onSetEtherCfg)
	(NK_PVoid event_ctx,
			NK_Boolean wifi,
			NK_Boolean dhcp,
			NK_Char ipaddr[32], NK_Char netmask[32], NK_Char gateway[32], NK_Char nameserver[32],
			NK_Char essid[128], NK_Char psk[128], NK_UInt32 options);

	/**
	 * @brief
	 *  Setup The Video Encoder Configuration Event.\n
	 *
	 * @param[in] event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * @@param[in] chid
	 *  The Preferred Channel ID.\n
	 *
	 * @param[in] chid2
	 *  The Altenative Channel ID.\n
	 *
	 * @param[out] codec
	 *  The Codec Type of Video Encoder.\n
	 *  0 -- H.264.\n
	 *  1 -- H.265.\n
	 *
	 * @param[out] width
	 *  The Width of This Video Image.\n
	 *
	 * @param[out] height
	 *  The Height of This Video Image.\n
	 *
	 * @param[out] fps
	 *  The Framerate ( Unit. FPS ) of This Video.\n
	 *
	 * @param[out] gop
	 *  The Group of Picture ( Unit. FPG), Consider as Key Frames Interleave.\n
	 *
	 * @param[out] bps_ctrl
	 *  The Bitrate Control Mode.\n
	 *  0 -- VBR ( Variable Bitrate ).\n
	 *  1 -- CBR ( Constant Bitrate ).\n
	 *
	 * @param[out] kbps
	 *  The Bitrate ( Unit. KBPS ) of This Stream.\n
	 *  If @ref bps_ctrl is VBR, Bitrate Indicates The Maximal Bitrate.\n
	 *  If @ref bps_ctrl is CBR, Bitrate Indicates The Average Bitrate.\n
	 *
	 */
	NK_Void (*onGetVEncCfg)
	(NK_PVoid event_ctx,
			NK_Int chid, NK_Int streamid,
			NK_Int *codec, NK_Size *width, NK_Size *height, NK_Size *fps, NK_Size *gop, NK_Size *bps_ctrl, NK_Size *kbps);

	/**
	 * @brief
	 *  Get the Video Encoder Configuration Event.\n
	 *  All the Parameters Check @ref onGetVEncCfg().\n
	 *
	 */
	NK_Void (*onSetVEncCfg)
	(NK_PVoid event_ctx,
			NK_Int chid, NK_Int streamid,
			NK_Int codec, NK_Size width, NK_Size height, NK_Size fps, NK_Size gop, NK_Size bps_ctrl, NK_Size kbps);


	/**
	 * @brief
	 *
	 * @param[in] event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * @param[in] name
	 *  The Name of Indicated Property.\n
	 *
	 * @param[in] chid
	 *  The Preferred Channel ID.\n
	 *
	 * @param[in] chid2
	 *  The Altenative Channel ID.\n
	 *
	 * @param[out] vnum
	 *  The Property Value, Type Number.\n
	 *  If Nil, Ignore It, This Property is NOT Number Type.\n
	 *
	 * @param[out] vstr
	 *  The Property Value, Type String.\n
	 *  If Nil, Ignore It, This Property is NOT String Type.\n
	 *
	 * @param[in] options
	 *  The Optional Flag, Reserved.\n
	 *
	 */
	NK_Void (*onGetProperty)
	(NK_PVoid event_ctx,
			NK_PChar name, NK_Int chid, NK_Int chid2, NK_Int *vnum, NK_PChar vstr, NK_UInt32 options);


	/**
	 * @brief
	 *
	 * @param[in] event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * @param[in] name
	 *  The Name of Indicated Property.\n
	 *
	 * @param[in] chid
	 *  The Preferred Channel ID.\n
	 *
	 * @param[in] chid2
	 *  The Altenative Channel ID.\n
	 *
	 * @param[in] vnum
	 *  The Property Value, Type Number.\n
	 *
	 * @param[in] vstr
	 *  The Property Value, Type String.\n
	 *  If Nil, Ignore It.\n
	 *
	 * @param[in] options
	 *  The Optional Flag, Reserved.\n
	 *
	 */
	NK_Void (*onSetProperty)
	(NK_PVoid event_ctx,
			NK_PChar name, NK_Int chid, NK_Int chid2, NK_Int vnum, NK_PChar vstr, NK_UInt32 options);


	/**
	 * @brief
	 *  Get the History Record on Storage.\n
	 *  Reserved.\n
	 *
	 * @param[in] event_ctx
	 *  The Event Context, Passed by @ref NK_IPCam_Init().\n
	 *
	 * @param[in] sessionid
	 *  The Session ID of This Record Search.\n
	 *  After Got the Record History, Module May Active a Playback Request with This ID.\n
	 *
	 * @param[in] chid
	 *  The Record History Channel ID.\n
	 *
	 */
	NK_Void (*onHistory)
	(NK_PVoid event_ctx,
			NK_Int sessionid, NK_Int chid);

} NK_IPCamEvent;


/**
 * @brief
 *  Get Version of SDK Module.\n
 *
 * @return
 *  The Version in Text ( Read-Only ).\n
 */
extern NK_PChar
NK_IPCam_Version();


/**
 * @brief
 *  Module Initializes.\n
 *
 * @param[in] license
 *  The User License Address.\n
 *
 * @param[in] mem_size
 *  The Size of Heap Memory Size for Module Internal Operation.\n
 *  Minimal Size is 1MBytes, If Pass a Size Less than 1MBytes, Function would Return Failed.\n
 *
 * @param[in] port
 *  The Communication Listen Port.\n
 *
 * @param[in] Event
 *  The Event Callback Handlers.\n
 *
 * @param[in] event_ctx
 *  The Event Context.\n
 *  This Value Would Pass to Every Events Handlers Once They Active.\n
 *
 * @retval NK_True
 *  Success.\n
 *
 * @retval NK_False
 *  Failed.\n
 *
 */
extern NK_Boolean
NK_IPCam_Init(NK_PByte license,
		NK_Size mem_size,
		NK_UInt16 port,
		NK_IPCamEvent *Event, NK_PVoid event_ctx);


/**
 * @brief
 *  Module Exits.\n
 *
 * @retval NK_True
 *  Success.\n
 *
 * @retval NK_False
 *  Failed.\n
 */
extern NK_Boolean
NK_IPCam_Exit();


/**
 * @brief
 *  Add an H.264 Video Frame to Module.\n
 *
 * @param[in] sessionid
 *  The Session ID of This Picture,\n
 *  0 - Live Session.\n
 *  Others - Playback Session.\n
 *
 * @param[in] chid
 *  Physical Input Capture Channel ID,\n
 *  Reserved to Typical IP Camera, Always be 0.\n
 *
 * @param[in] streamid
 *  The Stream ID of This Frame.\n
 *  0 -- Main Stream.\n
 *  1 -- Sub Stream.\n
 *
 * @param[in] ts_us
 *  Relative Timestamp (Unit: Microsecond).\n
 *
 * @param[in] data
 *  The Data Memory Address.\n
 *
 * @param[in] len
 *  The Data Size.\n
 *
 * @param[in] options
 *  Optional Flag, Reserved.\n
 *
 * @retval NK_True
 *  Success.\n
 *
 * @retval NK_False
 *  Failed.\n
 */
extern NK_Boolean
NK_IPCam_AddPic(NK_Int sessionid, NK_Int chid, NK_Int streamid, NK_UInt64 ts_us, NK_PVoid data, NK_Size len, NK_UInt32 options);


/**
 * @macro
 *  Quick Live Calling for @ref NK_IPCam_AddPic().\n
 */
#define NK_IPCAM_ADD_LIVE_PIC(__chid, __data, __len) NK_IPCam_AddPic(0, (__chid), 0, 0, (__data), (__len), 0)


/**
 * @brief
 *  Add an H.264 Video Frame to Module.\n
 *
 * @param[in] sessionid
 *  The Session ID of This Frame,\n
 *  0 - Live Session.\n
 *  Others - Playback Session.\n
 *
 * @param[in] chid
 *  Reserved, Always be 0.
 *
 * @param[in] streamid
 *  The Stream ID of This Frame.\n
 *  0 -- Main Stream.\n
 *  1 -- Sub Stream.\n
 *
 * @param[in] ts_us
 *  Relative Timestamp (Unit: Microsecond).\n
 *
 * @param[in] data
 *  The Data Memory Address.\n
 *
 * @param[in] len
 *  The Data Size.\n
 *
 * @param[in] options
 *  Optional Flag, Reserved.\n
 *
 * @retval NK_True
 *  Success.\n
 *
 * @retval NK_False
 *  Failed.\n
 *
 */
extern NK_Boolean
NK_IPCam_AddH264(NK_Int sessionid, NK_Int chid, NK_Int streamid, NK_UInt64 ts_us, NK_PVoid data, NK_Size len, NK_UInt32 options);


/**
 * @brief
 *  Add an H.265 Video Frame to Module.\n
 *
 * @param[in] sessionid
 *  The Session ID of This Frame,\n
 *  0 - Live Session.\n
 *
 * @param[in] chid
 *  Reserved, Always be 0.\n
 *
 * @param[in] streamid
 *  The Stream ID of This Frame.\n
 *  0 -- Main Stream.\n
 *  1 -- Sub Stream.\n
 *
 * @param[in] ts_us
 *  Relative Timestamp (Unit: Microsecond).\n
 *
 * @param[in] data
 *  The Data Memory Address.\n
 *
 * @param[in] len
 *  The Data Size.\n
 *
 * @param[in] options
 *  Optional Flag, Reserved.\n
 *
 * @retval NK_True
 *  Success.\n
 *
 * @retval NK_False
 *  Failed.\n
 *
 */
extern NK_Boolean
NK_IPCam_AddHEVC(NK_Int sessionid, NK_Int chid, NK_Int streamid, NK_UInt64 ts_us, NK_PVoid data, NK_Size len, NK_UInt32 options);

/**
 * @brief
 *  Alias to @ref NK_IPCam_AddHEVC().\n
 */
#define NK_IPCam_AddH265(__sessionid, __chid, __streamid, __ts_us, __data, __len, __options) \
	NK_IPCam_AddHEVC(__sessionid, __chid, __streamid, __ts_us, __data, __len, __options)


/**
 * @brief
 *  Add a G.711 Audio Frame to Module.\n
 *
 * @param[in] chid
 *  Reserved, Always be 0.\n
 *
 * @param[in] ts_us
 *  Relative Timestamp (Unit: Microsecond).\n
 *
 * @param[in] sample_rate
 *  The Audio Input Sample Rate.\n
 *  Should be One of Options in 8000, 11025, 16000, 22000, 24000.\n
 *
 * @param[in] sample_bitwidth
 *  The Audio Input Sample Bit Width.\n
 *  Should be One of Options in 8, 16, 32.\n
 *
 * @param[in] data
 *  The Data Memory Address.\n
 *
 * @param[in] len
 *  The Data Size.\n
 *
 * @param[in] options
 *  Optional Flag, Reserved.\n
 *
 * @retval NK_True
 *  Success.\n
 *
 * @retval NK_False
 *  Failed.\n
 *
 */
extern NK_Boolean
NK_IPCam_AddG711(NK_Int sessionid, NK_Int chid, NK_UInt64 ts_us, NK_Int sample_rate, NK_Int sample_bitwidth, NK_PVoid data, NK_Size len);


/**
 * @brief
 *  Add a AAC Audio Frame to Module.\n
 */
NK_Boolean
NK_IPCam_AddAAC(NK_Int sessionid, NK_Int chid, NK_UInt64 ts_us, NK_Int sample_rate, NK_Int sample_bitwidth, NK_PVoid data, NK_Size len);


/**
 * @brief
 *  Set String Type Property.\n
 *
 * @param[in] name
 *  The Property Name.\n
 *
 * @param[in] chid
 *  The Preferred Channel ID.\n
 *
 * @param[in] chid2
 *  The Altenative Channel ID.\n
 *
 * @param[in] vnum
 *  Number Type Value.\n
 *
 * @param[in] vstr
 *  String Type Value, Pass Nil If Needlessness.\n
 *
 * @param[in] options
 *  Optional Flag, Reserved.\n
 *
 * @retval NK_True
 *  Success.\n
 *
 * @retval NK_False
 *  Failed.\n
 *
 */
extern NK_Boolean
NK_IPCam_AddProperty(NK_PChar name, NK_Int chid, NK_Int chid2, NK_Int vnum, NK_PChar vstr, NK_UInt32 options);


/**
 * @macro
 *  Quick Number Type Property Calling for @ref NK_IPCam_AddProperty().\n
 */
#define NK_IPCAM_ADD_PROP_NUM(__name, __chid, __chid2, __vnum) NK_IPCam_AddProperty((__name), (__chid), (__chid2), (__vnum), NK_Nil, 0)


/**
 * @macro
 *  Quick String Type Property Calling for @ref NK_IPCam_AddProperty().\n
 */
#define NK_IPCAM_ADD_PROP_STR(__name, __chid, __chid2, __vstr) NK_IPCam_AddProperty((__name), (__chid), (__chid2), 0, (__vstr), 0)

/**
 * @brief
 *  Dump All the Properties.\n
 */
extern NK_Void
NK_IPCam_ListProperty();


NK_CPP_EXTERN_END
#endif /* NK_N1_IPCAM_H_ */


