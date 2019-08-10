
#include "hikvisionv10.h"
#include "netsdk_util.h"
#include "app_debug.h"

/*
<StreamingStatus version=¡°1.0¡± xmlns=¡°http://www.hikvision.com/ver10/XMLSchema¡±>
<totalStreamingSessions> <!-- req, xs:integer --> </totalStreamingSessions>
<StreamingSessionStatusList/> <!-- dep, only if there are sessions -->
</StreamingStatus>
*/

int HIKVISIONv10_streaming_status(LP_HTTP_CONTEXT context)
{
	const char *text = 
		"<StreamingStatus xmlns=\"urn:psialliance-org\" version=\"1.0\">"
			"<totalStreamingSessions>0</totalStreamingSessions>"
			"<StreamingSessionStatusList xmlns=\"urn:psialliance-org\" version=\"1.0\"></StreamingSessionStatusList>"
		"</StreamingStatus>";

	HIKv10_response(context->sock, text);
	return 0;
}

static ezxml_t
hikv10_streaming_channel_transport_controlprotocol
(
	const char *streaming_transport
)
{
	ezxml_t control_protocol = ezxml_new("ControlProtocol");
	if(control_protocol){
		// insert streaming transport
		ezxml_insert(HIKv10_xs_string("streamingTransport", streaming_transport), control_protocol, 0);
		return control_protocol;
	}
	return NULL;
}

static ezxml_t
hikv10_streaming_channel_transport_controlprotocollist
(
	int n_protocols, ...
)
{
	ezxml_t control_protocol_list = ezxml_new("ControlProtocolList");
	if(NULL != control_protocol_list){
		if(n_protocols > 0){
			int node_off = 0;
			va_list argp;

			va_start(argp, n_protocols);
			while(n_protocols--){
				ezxml_t control_protocol = va_arg(argp, ezxml_t);
				ezxml_insert(control_protocol, control_protocol_list, node_off++);
			}
			va_end(argp);
		}
		return control_protocol_list;
	}
	return NULL;
}

static ezxml_t
hikv10_streaming_channel_transport_unicast
(
	bool enabled
)
{
	ezxml_t unicast = ezxml_new("Unicast");
	if(NULL != unicast){
		// insert enabled
		ezxml_insert(HIKv10_xs_boolean("enable", enabled), unicast, 0);
		return unicast;
	}
	return NULL;
}

static ezxml_t
hikv10_streaming_channel_transport_multicast
(
	bool enabled,
	const char *dest_ip,
	int dest_port
)
{
	ezxml_t multicast = ezxml_new("Unicast");
	if(NULL != multicast){
		int node_off = 0;
		// insert enabled
		ezxml_insert(HIKv10_xs_boolean("enable", enabled), multicast, node_off++);
		// insert dest ip addr
		ezxml_insert(HIKv10_xs_string("destIPAddress", dest_ip), multicast, node_off++);
		// insert dest port
		ezxml_insert(HIKv10_xs_integer("destPortNo", dest_port), multicast, node_off++);
		return multicast;
	}
	return NULL;
}

static ezxml_t
hikv10_streaming_channel_transport
(
	int rtsp_port_no,
	int max_packet_size,
	int source_port_no,
	ezxml_t control_protocol_list,
	ezxml_t unicast,
	ezxml_t multicase
)
{
	ezxml_t transport = ezxml_new("Transport");
	if(NULL != transport){
		int node_off = 0;
		// insert rtsp port number
		ezxml_insert(HIKv10_xs_integer("rtspPortNo", rtsp_port_no), transport, node_off++);
		// insert max packet size
		ezxml_insert(HIKv10_xs_integer("maxPacketSize", max_packet_size), transport, node_off++);
		// insert source port number
		ezxml_insert(HIKv10_xs_integer("sourcePortNo", 0), transport, node_off++);
		// insert control protocol list
		ezxml_insert(control_protocol_list, transport, node_off++);
		// insert unicast
		ezxml_insert(unicast, transport, node_off++);
		// insert multicase
		ezxml_insert(multicase, transport, node_off++);
		return transport;
	}
	return NULL;
}

static ezxml_t
hikv10_streaming_channel_video
(
	bool enabled,
	int video_input_channel_id,
	const char *video_codec_type,
	int video_resolution_width,
	int video_resolution_height,
	const char *video_quality_control_type,
	int constant_bitrate,
	int fixed_quality,
	int max_framerate,
	int keyframe_interval,
	int bpframe_interval,
	const char *snapshot_image_type
)
{
	ezxml_t video = ezxml_new("Video");
	if(NULL != video){
		int node_off = 0;
		ezxml_insert(HIKv10_xs_boolean("enabled", enabled), video, node_off++);
		ezxml_insert(HIKv10_xs_integer("videoInputChannelID", video_input_channel_id), video, node_off++);
		ezxml_insert(HIKv10_xs_string("videoCodecType", "H264"), video, node_off++);
		ezxml_insert(HIKv10_xs_string("videoScanType", "progressive"), video, node_off++);
		ezxml_insert(HIKv10_xs_integer("videoResolutionWidth", video_resolution_width), video, node_off++);
		ezxml_insert(HIKv10_xs_integer("videoResolutionHeight", video_resolution_height), video, node_off++);
		ezxml_insert(HIKv10_xs_string("videoQualityControlType", "CBR"), video, node_off++);
		ezxml_insert(HIKv10_xs_integer("constantBitRate", constant_bitrate), video, node_off++);
		ezxml_insert(HIKv10_xs_integer("maxFrameRate", max_framerate * 100), video, node_off++);
		ezxml_insert(HIKv10_xs_integer("keyFrameInterval", keyframe_interval), video, node_off++);
		ezxml_insert(HIKv10_xs_integer("BPFrameInterval", 0), video, node_off++);
		ezxml_insert(HIKv10_xs_string("snapShotImageType", "JPEG"), video, node_off++);
		return video;
	}
	return NULL;
}

static ezxml_t
hikv10_streaming_channel_audio
(
	bool enabled,
	int audio_input_channel_id,
	const char *audio_compression_type
)
{
	ezxml_t audio = ezxml_new("Audio");
	if(NULL != audio){
		int node_off = 0;
		// insert enabled
		ezxml_insert(HIKv10_xs_boolean("enabled", enabled), audio, node_off++);
		// insert audioInputChannelID
		ezxml_insert(HIKv10_xs_integer("audioInputChannelID", audio_input_channel_id), audio, node_off++);
		// insert audioCompressionType
		ezxml_insert(HIKv10_xs_string("audioCompressionType", audio_compression_type), audio, node_off++);
		
		return audio;
	}
	return NULL;
}



static ezxml_t
hikv10_streaming_channel
(
	int id,
	const char *channel_name,
	bool enabled,
	ezxml_t transport,
	ezxml_t video,
	ezxml_t audio
)
{
	ezxml_t streaming_channel = ezxml_new("StreamingChannel");
	if(NULL != streaming_channel){
		int node_off = 0;
		HIKv10_set_namespace(streaming_channel);
		HIKv10_set_version(streaming_channel);
		// insert id
		ezxml_insert(HIKv10_xs_integer("id", id), streaming_channel, node_off++);
		// insert channel name
		ezxml_insert(HIKv10_xs_string("channelName", channel_name), streaming_channel, node_off++);
		// insert enabled
		ezxml_insert(HIKv10_xs_boolean("enabled", enabled), streaming_channel, node_off++);
		// insert  transport
		ezxml_insert(transport, streaming_channel, node_off++);
		// insert  video
		ezxml_insert(video, streaming_channel, node_off++);
		// insert  audio
		ezxml_insert(audio, streaming_channel, node_off++);
		return streaming_channel;
	}
	return NULL;
}

static ezxml_t
hikv10_streaming_channel_list
(
	int n_channels, ...
)
{
	ezxml_t streaming_channel_list = ezxml_new("StreamingChannel");
	if(NULL != streaming_channel_list){
		HIKv10_set_namespace(streaming_channel_list);
		HIKv10_set_version(streaming_channel_list);
		
		if(n_channels > 0){
			int node_off = 0;
			va_list argp;

			va_start(argp, n_channels);
			while(n_channels--){
				ezxml_t stream_channel = va_arg(argp, ezxml_t);
				ezxml_insert(stream_channel, streaming_channel_list, node_off++);
			}
			va_end(argp);
		}
		return streaming_channel_list;
	}
	return NULL;
}

int HIKVISIONv10_streaming_channels(LP_HTTP_CONTEXT context)
{
	ezxml_t stream_channel_list = hikv10_streaming_channel_list(0);
	if(stream_channel_list){
		int count = 3;
		int node_off = 0;
		const char *xml_text = NULL;

		while(count--){
			
			ezxml_t protocol_rtsp = hikv10_streaming_channel_transport_controlprotocol("RTSP");
			ezxml_t protocol_rtmp = hikv10_streaming_channel_transport_controlprotocol("RTMP");
			ezxml_t protocol_flv = hikv10_streaming_channel_transport_controlprotocol("FLV");
			
	 		ezxml_t transport_protocol_list = hikv10_streaming_channel_transport_controlprotocollist(3,
				protocol_rtsp, protocol_rtmp, protocol_flv);
			ezxml_t transport_unicast = hikv10_streaming_channel_transport_unicast(true);
			ezxml_t transport_multicast = hikv10_streaming_channel_transport_multicast(false, "0.0.0.0", 0);
			ezxml_t transport = 	hikv10_streaming_channel_transport(80, 1000, 0,
					transport_protocol_list, transport_unicast, transport_multicast);	

			ezxml_t video = hikv10_streaming_channel_video(true, 1,
				"H264", 1280, 720, "CBR", 3000, 40, 30, 60, 0, "JPEG");
			ezxml_t audio = 	hikv10_streaming_channel_audio(false, 11, "G711ulaw");

			ezxml_t stream_channel = hikv10_streaming_channel(1, "Camera", true,
				transport, video, audio);

			ezxml_insert(stream_channel, stream_channel_list, node_off++);
		}

		xml_text = ezxml_toxml(stream_channel_list);
		ezxml_free(stream_channel_list);
		stream_channel_list = NULL;
		
		APP_TRACE(xml_text);

		HIKv10_response(context->sock, xml_text);
		free(xml_text);
		xml_text = NULL;

	}
	return 0;
}

int HIKVISIONv10_streaming_channels_id(LP_HTTP_CONTEXT context)
{
	return 0;
}

int HIKVISIONv10_streaming_channels_id_capabilities(LP_HTTP_CONTEXT context)
{
	return 0;
}

int HIKVISIONv10_streaming_channels_id_status(LP_HTTP_CONTEXT context)
{
	return 0;
}


//
int HIKVISIONv10_streaming_channels_id_picture(LP_HTTP_CONTEXT context)
{

///Streaming/channels/ID/picture
	return 0;
}

int HIKVISIONv10_streaming_channels_id_requestkeyframe(LP_HTTP_CONTEXT context)
{
	return 0;
}



