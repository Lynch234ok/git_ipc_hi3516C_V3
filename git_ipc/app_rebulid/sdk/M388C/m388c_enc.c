
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "sdk_common.h"

#include "sync_ring_buffer.h"
#include "msg_broker.h"
#include "msg_format.h"
#include <signal.h>
#include <dirent.h>
#include <sys/prctl.h>
//#include "gb2312utf8.h"


//======================================
#define SRB_RCV_VENC_PIN     "venc_srb_"                //!< VMF_VideoEnc Output pin
#define SRB_RCV_CMD_FIFO     "/tmp/venc/c0/command.fifo" //!< communicate with rtsps, vrec, etc.
#define ACTUAL_STREAM "/media/conf/actual_stream"
#define SENSOR_TYPE_FILE    "/media/conf/sensor_type"
//======================================


# define VA_VENC_H264_REF_MODE (H264E_REFSLICE_FOR_1X)
# define VA_VENC_CH_BACKLOG_REF (1)
# define VA_AENC_CH_BACKLOG_REF (1)
# define VA_VENC_STREAM_BACKLOG_REF (4)


#define VA_VENC_JPEG_MIN_WIDTH (64)
#define VA_VENC_JPEG_MAX_WIDTH (8192)
#define VA_VENC_JPEG_MIN_HEIGHT (64)
#define VA_VENC_JPEG_MAX_HEIGHT (8192)

#define VA_VENC_OVERLAY_BACKLOG_REF (8)
#define VA_VENC_OVERLAY_CANVAS_STOCK_REF (VA_VENC_CH_BACKLOG_REF * VA_VENC_STREAM_BACKLOG_REF * VA_VENC_OVERLAY_BACKLOG_REF)
#define VA_VENC_OVERLAY_HANDLE_OFFSET (RGN_HANDLE_MAX - VA_VENC_OVERLAY_CANVAS_STOCK_REF) // 0 - 1024

#define __HI_VENC_CH(_vin, _stream) ((_vin)* VA_VENC_STREAM_BACKLOG_REF + (_stream))

typedef enum{
	eMSTRM_FIX_MODE_CELL = 0,
	eMSTRM_FIX_MODE_TABLE,
	eMSTRM_FIX_MODE_WALL,
}eMSTRM_FIX_MODE;

typedef enum{
	eMSTRM_SHOW_MODE_ORIGIN = 0,
	eMSTRM_SHOW_MODE_360,
	eMSTRM_SHOW_MODE_P180A,
	eMSTRM_SHOW_MODE_P180S,
	eMSTRM_SHOW_MODE_P180T,
	eMSTRM_SHOW_MODE_6R,
	eMSTRM_SHOW_MODE_4R,
	eMSTRM_SHOW_MODE_1P2R,
	eMSTRM_SHOW_MODE_1O3R,
	eMSTRM_SHOW_MODE_SPHERE,
	eMSTRM_SHOW_MODE_KIT_1R,
	eMSTRM_SHOW_MODE_360S,
	eMSTRM_SHOW_MODE_KIT_1RO,
}eMSTRM_SHOW_MODE;


#define HI_ENC_ATTR_MAGIC (0xf0f0f0f0)

typedef struct SDK_ENC_VIDEO_OVERLAY_ATTR {
	LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas;
	char name[32];
	int x, y;
	size_t width, height;
}stSDK_ENC_VIDEO_OVERLAY_ATTR, *lpSDK_ENC_VIDEO_OVERLAY_ATTR;

typedef struct SDK_ENC_VIDEO_OVERLAY_ATTR_SET {
	stSDK_ENC_VIDEO_OVERLAY_ATTR attr[VA_VENC_OVERLAY_BACKLOG_REF];
}stSDK_ENC_VIDEO_OVERLAY_ATTR_SET, *lpSDK_ENC_VIDEO_OVERLAY_ATTR_SET;

typedef struct SDK_ENC_ATTR {
	enSDK_ENC_BUF_DATA_TYPE enType[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];
	union{	
		ST_SDK_ENC_STREAM_H264_ATTR h264_attr[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];
		ST_SDK_ENC_STREAM_H265_ATTR h265_attr[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];
	};
	uint8_t frame_ref_counter[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];	
	ST_SDK_ENC_STREAM_G711A_ATTR g711_attr[VA_AENC_CH_BACKLOG_REF];
	
	stSDK_ENC_VIDEO_OVERLAY_ATTR_SET video_overlay_set[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];
	ST_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas_stock[VA_VENC_OVERLAY_CANVAS_STOCK_REF];

	bool loop_trigger;
	int ref_count;
	pthread_t loop_tid;
	pthread_mutex_t snapshot_mutex;
	srb_handle_t* srb_handle[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];
	srb_buffer_t srb_buf[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];
	int fix_mode[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];
	int show_mode[VA_VENC_CH_BACKLOG_REF][VA_VENC_STREAM_BACKLOG_REF];
	int actual_stream;
}stSDK_ENC_ATTR, *lpSDK_ENC_ATTR;

#define STREAM_H264_ISSET(__stream_h264) (strlen((__stream_h264)->name) > 0)
#define STREAM_H264_CLEAR(__stream_h264) do{ memset((__stream_h264)->name, 0, sizeof((__stream_h264)->name)); }while(0)


typedef struct SDK_ENC_M388C
{
	stSDK_ENC_API api;
	stSDK_ENC_ATTR attr;
}stSDK_ENC_M388C, *lpSDK_ENC_M388C;
static stSDK_ENC_M388C _sdk_enc;
lpSDK_ENC_API sdk_enc = NULL;


emSENSOR_MODEL gs_sensorModel;

/////test for H265 file
//static FILE *pFile[VENC_MAX_CHN_NUM];


static inline uint64_t get_time_us()
{
	uint64_t time_us = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_us = tv.tv_sec;
	time_us <<= 32;
	time_us += tv.tv_usec;
	return time_us;
}

void send_msg(MsgContext *msg, const uint8_t *data, size_t data_size)
{
	msg->host_len  = strlen(msg->host) + 1;
	msg->cmd_len   = strlen(msg->cmd) + 1;
	msg->data_size = data_size;
	msg->data      = data;
	
	if (MsgBroker_SendMsg(SRB_RCV_CMD_FIFO, msg) != 0)
		printf("[srb_rcv] send command error, %s\n", SRB_RCV_CMD_FIFO);
}

void send_command(unsigned int cmd, int vin, int stream)//send command to MsgBroker
{
	printf("[srb_rcv] send command , %d\n", cmd);
	MsgContext msg_context;
	char hostname[32];
	
	snprintf(hostname, sizeof(hostname), "encoder%d", stream);
	//msg_context.host = "encoder0";
	msg_context.host = hostname;
	msg_context.host_len = strlen(msg_context.host) + 1;
	msg_context.data_size = 0;
	msg_context.has_response = 1;

	switch(cmd)
	{
		case 1:
			msg_context.cmd = "forceIntra"; //notify encoder to produce "keyframe"
			msg_context.cmd_len = 11;
			break;
		case 2:
			msg_context.cmd = "forceCI"; //notify encode to produce "CONF data"
			msg_context.cmd_len = 8;
			break;
		case 3:
			msg_context.cmd = "start"; //Start ecndoe
			msg_context.cmd_len = 6;
			break;
		case 4:
			msg_context.cmd = "stop"; //stop encode
			msg_context.cmd_len = 5;
			break;
		default:
			return;
	}

	if (MsgBroker_SendMsg(SRB_RCV_CMD_FIFO, &msg_context) != 0)
		printf("[srb_rcv] send command error, %s\n", SRB_RCV_CMD_FIFO);
}

void enc_video_set(int vin, int stream, int fps, int gop, int bps, int width, int height)
{
	msg_video_t video_msg;
	MsgContext msg_context;
	
	memset(&video_msg, 0, sizeof(msg_video_t));
	memset(&msg_context, 0, sizeof(MsgContext));
	
	char hostname[32];
	printf("stream:%d, fps:%d gop:%d bps:%d\n", stream, fps, gop, bps);
	snprintf(hostname, sizeof(hostname), "encoder%d", stream);
	msg_context.host = hostname;
	msg_context.host_len = strlen(msg_context.host) + 1;
	msg_context.cmd = "changeVideo";
			
	video_msg.type = 1; //! h264
	video_msg.fps = fps;
	video_msg.gop = gop;
	video_msg.bitrate = bps*1024;
	//video_msg.advanced_mode = 0;
	send_msg(&msg_context, (uint8_t*)&video_msg, sizeof(msg_video_t));

	msg_channel_t msg_channel;
	memset(&msg_channel, 0, sizeof(msg_channel_t));
	
	snprintf(hostname, sizeof(hostname), "encoder%d", stream);
	msg_context.host = hostname;
	msg_context.host_len = strlen(msg_context.host) + 1;
	msg_context.cmd = "setupChannel";
	
	msg_channel.channel = stream;
	//msg_channel.type = 1; //! h264
	msg_channel.w = width;
	msg_channel.h = height;
	msg_channel.fps = fps;
	msg_channel.gop = gop;
	
	printf(" wxh=%dx%d, fps=%d\n", width, height, fps);
	send_msg(&msg_context, (uint8_t*)&msg_channel, sizeof(msg_channel_t));
    if(gs_sensorModel == SENSOR_MODEL_SMARTSENS_SC2045) {
        if((width == 1920) && (height == 1080)) {
            system("export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/share/ipcam/mpp/lib/;/usr/share/ipcam/mpp/mstrm_msg_sender -C 0 -c 0 -a 2 -m 3 -p 0 -t 0 -z 0.67 -f 0.71 -u 0.65 -o 0.8 -e 1 -N 0 -J 66 -K 1.2");
        }
        else if((width == 1280) && (height == 720)) {
            system("export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/share/ipcam/mpp/lib/;/usr/share/ipcam/mpp/mstrm_msg_sender -C 0 -c 0 -a 2 -m 3 -p 0 -t 0 -z 0.67 -f 0.71 -u 0.65 -o 0.8 -e 1 -N 0 -J 47 -K 1.2");
        }
    }
	usleep(500000);
	
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

static int do_buffer_request(int buf_id, ssize_t data_sz, uint32_t keyflag)
{
	if(sdk_enc->do_buffer_request){
		return sdk_enc->do_buffer_request(buf_id, data_sz, keyflag);
	}
	return -1;
}
static int do_buffer_append(int buf_id, const void* item, ssize_t item_sz)
{
	if(sdk_enc->do_buffer_append){
		return sdk_enc->do_buffer_append(buf_id, item, item_sz);
	}
	return -1;
}
static int do_buffer_commit(int buf_id)
{
	if(sdk_enc->do_buffer_commit){
		return sdk_enc->do_buffer_commit(buf_id);
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

static int enc_lookup_stream_byname(const char* name, int* ret_vin, int* ret_stream)
{
	int i = 0, ii = 0;
	if(NULL != name && strlen(name)){
		for(i = 0; i < VA_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < VA_VENC_STREAM_BACKLOG_REF; ++ii){
				LP_SDK_ENC_STREAM_H264_ATTR h264_attr = &_sdk_enc.attr.h264_attr[i][ii];
				if(STREAM_H264_ISSET(h264_attr)
					&& 0 == strcasecmp(h264_attr->name, name)
					&& strlen(h264_attr->name) == strlen(name)){

					if(ret_vin){
						*ret_vin = i;
					}
					if(ret_stream){
						*ret_stream = ii;
					}
					return 0;
				}
			}
		}
	}
	return -1;
}

static int do_enc_limit(LP_SDK_ENC_STREAM_H264_ATTR streamAttr)
{
	if(eMSTRM_SHOW_MODE_KIT_1R == _sdk_enc.attr.show_mode[0][0] || 
		eMSTRM_SHOW_MODE_KIT_1RO== _sdk_enc.attr.show_mode[0][0] ||
		_sdk_enc.attr.actual_stream > 2){
		if(streamAttr->width >= 960 || streamAttr->height >= 720){
			streamAttr->width = 960;
			streamAttr->height = 720;
			streamAttr->bps = 1280;
			streamAttr->fps = 20;
		}
	}else if(streamAttr->width  > 1280 && streamAttr->height > 1280){
		streamAttr->fps = 15;
	}else{

	}
	return 0;
}

static int enc_create_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	char srb_name[32];
	if(vin < VA_VENC_CH_BACKLOG_REF && stream < _sdk_enc.attr.actual_stream
		&& NULL != h264_attr && STREAM_H264_ISSET(h264_attr)){
		
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
			
		if(0 == enc_lookup_stream_byname(h264_attr->name, NULL, NULL)){
			// the name is overlap
			printf("failed\n");
			return -1;
		}
		memcpy(streamH264Attr, h264_attr, sizeof(ST_SDK_ENC_STREAM_H264_ATTR));

		//init reader
		snprintf(srb_name, sizeof(srb_name), "%s%d", SRB_RCV_VENC_PIN, stream+1);
		_sdk_enc.attr.srb_handle[vin][stream] = SRB_InitReader(srb_name);

		//set parameters
		do_enc_limit(streamH264Attr);
		enc_video_set(vin, stream, streamH264Attr->fps, streamH264Attr->gop, streamH264Attr->bps, 
			streamH264Attr->width, streamH264Attr->height);

	}
	return 0;
}

static int enc_create_stream_h265(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr)
{
	return 0;
}

static int enc_release_stream_h264(int vin, int stream)
{
//	srb_buffer_t srb_buf;
	printf("%s:%d-%d\n", __FUNCTION__, vin, stream);
	if(vin < VA_VENC_CH_BACKLOG_REF && stream < _sdk_enc.attr.actual_stream){

		SRB_ReturnReaderBuff(_sdk_enc.attr.srb_handle[vin][stream], &_sdk_enc.attr.srb_buf[vin][stream]);
		SRB_Release(_sdk_enc.attr.srb_handle[vin][stream]);
		_sdk_enc.attr.srb_handle[vin][stream] = NULL;
		return 0;
	}
	return -1;
}

static int enc_release_stream_h265(int vin, int stream)
{
	return 0;
}

static int enc_set_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	if(vin < VA_VENC_CH_BACKLOG_REF && stream < _sdk_enc.attr.actual_stream
		&& NULL != h264_attr && STREAM_H264_ISSET(h264_attr)){
		
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
			
		memcpy(streamH264Attr, h264_attr, sizeof(ST_SDK_ENC_STREAM_H264_ATTR));

		//set parameters
		do_enc_limit(streamH264Attr);
		enc_video_set(vin, stream, streamH264Attr->fps, streamH264Attr->gop, streamH264Attr->bps, 
			streamH264Attr->width, streamH264Attr->height);

	}
	return 0;
}

static int enc_set_stream_h265(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr)
{
	return 0;
}

static int enc_get_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	if(vin < VA_VENC_CH_BACKLOG_REF && stream < _sdk_enc.attr.actual_stream
		&& NULL != h264_attr){
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
		if(STREAM_H264_ISSET(streamH264Attr)){
			memcpy(h264_attr, streamH264Attr, sizeof(ST_SDK_ENC_STREAM_H264_ATTR));
			return 0;
		}
	}
	return -1;
}

static int enc_get_stream_h265(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr)
{
	return 0;
}

static int enc_create_stream_g711a(int ain, int vin_ref)
{
	return 0;
}

static int enc_release_stream_g711a(int ain)
{
	return 0;
}

static int enc_enable_stream_h264(int vin, int stream, bool flag)
{
	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
	if(vin < VA_VENC_CH_BACKLOG_REF && stream < _sdk_enc.attr.actual_stream
		&& NULL != streamH264Attr && STREAM_H264_ISSET(streamH264Attr)){
		if(flag){
			//start to receive pictures
			send_command(3, vin, stream);
		}else{
			//stop to receive pictures
			send_command(4, vin, stream);
		}
		return 0;
	}
	return -1;
}

static int enc_enable_stream_h265(int vin, int stream, bool flag)
{
	return 0;
}


static int enc_request_stream_h264_keyframe(int vin, int stream)
{
	return 0;
}

static int enc_request_stream_h265_keyframe(int vin, int stream)
{
	return 0;
}


static int enc_video_proc(int vin, int stream)
{
	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &(_sdk_enc.attr.h264_attr[vin][stream]);
	stSDK_ENC_BUF_ATTR attr;

//	srb_buffer_t srb_buf;
	unsigned int* values;
	unsigned int nal_type = 0;
	bool is_keyframe = false;
#if 1
	if(STREAM_H264_ISSET(streamH264Attr)){
	/* Receive video data */
		//if(0 == SRB_ReturnReceiveReaderBuff(_sdk_enc.attr.srb_handle[vin][stream], &_sdk_enc.attr.srb_buf[vin][stream])){
		if(0 == SRB_QueryReaderBuff(_sdk_enc.attr.srb_handle[vin][stream], &_sdk_enc.attr.srb_buf[vin][stream])){

			values = (unsigned int*) _sdk_enc.attr.srb_buf[vin][stream].buffer;
			nal_type = _sdk_enc.attr.srb_buf[vin][stream].buffer[260] & 0x1f;

			if (values[0] == 0x34363248) //FOURCC_H264;
			{
				//printf("[srb_rcv] get video data nal type: %d size: %d %d\n", nal_type, values[3], i++);

				is_keyframe = (nal_type == 7) ? true:false;
				// buffering strream
				attr.magic = kSDK_ENC_BUF_DATA_MAGIC;
				attr.type = kSDK_ENC_BUF_DATA_H264;
				attr.timestamp_us = values[1]*1000000 + values[2];
				attr.time_us = get_time_us();
				attr.data_sz = values[3];
				attr.h264.keyframe = is_keyframe;
				attr.h264.ref_counter = 1;
				attr.h264.fps = streamH264Attr->fps;
				attr.h264.width = streamH264Attr->width;
				attr.h264.height = streamH264Attr->height;
				//printf("timestamp=%lld  size=%d  Type=%c\n", attr.timestamp_us, attr.data_sz, attr.h264.keyframe?'I':'P');
				if(0 == do_buffer_request(streamH264Attr->buf_id, attr.data_sz + sizeof(attr), attr.h264.keyframe)){	
					// buffer in the attribute
					do_buffer_append(streamH264Attr->buf_id, &attr, sizeof(attr));
					// buffer in the payload				
					do_buffer_append(streamH264Attr->buf_id, _sdk_enc.attr.srb_buf[vin][stream].buffer + 256, attr.data_sz);
					//commit one frame	
					do_buffer_commit(streamH264Attr->buf_id);
				}
				return 0;
			}
		}
	}
#endif
	return -1;
}

static int enc_audio_proc(int ain)
{
	return -1;
}


static void* enc_loop(void* arg)
{
	int i = 0, ii = 0;
	prctl(PR_SET_NAME, "enc_loop");
	while(_sdk_enc.attr.loop_trigger){
		// audio stream
		_sdk_enc.attr.ref_count = 1;
		for(i = 0; i < VA_AENC_CH_BACKLOG_REF; ++i){
			enc_audio_proc(i);
		}
		// main stream first
		for(i = 0; i < VA_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < _sdk_enc.attr.actual_stream; ++ii){
				while(0 == enc_video_proc(i, ii));
			}
		}
		_sdk_enc.attr.ref_count = 0;
		//usleep(30000);
		usleep(2000);
	}
	pthread_exit(NULL);
}

static int enc_start()
{
	if(!_sdk_enc.attr.loop_tid){
		int ret = 0;
		_sdk_enc.attr.loop_trigger = true;
		ret = pthread_create(&_sdk_enc.attr.loop_tid, NULL, enc_loop, NULL);
		SOC_ASSERT(0 == ret, "AV encode do loop create thread failed!");
		return 0;
	}
	return -1;
}

static int enc_stop()
{
	if(_sdk_enc.attr.loop_tid){
		_sdk_enc.attr.loop_trigger = false;
		pthread_join(_sdk_enc.attr.loop_tid, NULL);
		_sdk_enc.attr.loop_tid = (pthread_t)NULL;
		return 0;
	}
	return -1;
}

static int enc_snapshot(int vin, enSDK_ENC_SNAPSHOT_QUALITY quality, ssize_t width, ssize_t height, FILE* stream)
{
	return 0;
}


static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS enc_create_overlay_canvas(size_t width, size_t height)
{
	return NULL;
}

static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS enc_load_overlay_canvas(const char *bmp24_path)
{
	return NULL;
}


static void enc_release_overlay_canvas(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas)
{
	
}

static int enc_create_overlay(int vin, int stream, const char* overlay_name,
		float x, float y, LP_SDK_ENC_VIDEO_OVERLAY_CANVAS const canvas)
{
	return -1;
}

static int enc_release_overlay(int vin, int stream, const char* overlay_name)
{
	return -1;
}

static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS enc_get_overlay_canvas(int vin, int stream, const char* overlay_name)
{
	return NULL;
}

static int enc_show_overlay(int vin, int stream, const char* overlayName, bool showFlag)
{
	return -1;
}

static int enc_update_overlay(int vin, int stream, const char* overlay_name)
{
	return -1;
}

static int enc_eptz_ctrl(int vin, int stream, int cmd, int param)
{
	float speed = param;	
	MsgContext msg_context;
	msg_fec_t fec_msg;
	char hostname[32];
	static int old_cmd = EPTZ_CMD_STOP;
	
	memset(&msg_context, 0, sizeof(MsgContext));
	memset(&fec_msg, 0, sizeof(msg_fec_t));

	snprintf(hostname, sizeof(hostname), "source%d", vin);
	msg_context.host = hostname;

	
	msg_context.cmd = "setupFC";
	
	/*  send msg_fec_t structure */
	fec_msg.lens = 2;
	fec_msg.channel = stream;
	fec_msg.mode = -1;
	
	if (-1 != stream) {
		//fec_msg.pan = ((pan >= (-180)) && (pan <=180)) ? pan: 0;
		//fec_msg.tilt =  ((tilt >= -135) && (tilt <=135)) ? tilt: 0;
		//fec_msg.zoom =((zoom >= 0.01) && (zoom <=100.0)) ? zoom: (1.0) ;
		/*if(focal != 0)
			fec_msg.focal = ((focal >= 0.1) && (focal <=1.5)) ? focal: (0.65) ;*/
		//fec_msg.focal = 0.65;
		
		fec_msg.cell_idx = 0;
		speed = 2;

		fec_msg.step = -1;

		if(EPTZ_CMD_AUTOPAN == cmd && EPTZ_CMD_AUTOPAN == old_cmd){
			old_cmd = EPTZ_CMD_STOP;
		}else{
			old_cmd = cmd;
		}
		switch(old_cmd){
			case EPTZ_CMD_UP:
				fec_msg.pan_step = 0;
				fec_msg.tilt_step = -speed;
				fec_msg.zoom_step = 0;
				break;
			case EPTZ_CMD_DOWN:
				fec_msg.pan_step = 0;
				fec_msg.tilt_step = speed;
				fec_msg.zoom_step = 0;
				break;
			case EPTZ_CMD_LEFT:
				fec_msg.pan_step = -speed;
				fec_msg.tilt_step = 0;
				fec_msg.zoom_step = 0;
				break;
			case EPTZ_CMD_RIGHT:
				fec_msg.pan_step = speed;
				fec_msg.tilt_step = 0;
				fec_msg.zoom_step = 0;
				break;
			case EPTZ_CMD_FOCUS_FAR:
				break;
			case EPTZ_CMD_FOCUS_NEAR:
				break;
			case EPTZ_CMD_ZOOM_IN:
				fec_msg.pan_step = 0;
				fec_msg.tilt_step = 0;
				fec_msg.zoom_step = -speed/4;
				break;
			case EPTZ_CMD_ZOOM_OUT:
				fec_msg.pan_step = 0;
				fec_msg.tilt_step = 0;
				fec_msg.zoom_step = speed/4;
				break;
			case EPTZ_CMD_AUTOPAN:
				fec_msg.pan_step = -speed;
				fec_msg.tilt_step = 0;
				//fec_msg.tilt =  40;
				fec_msg.zoom_step = 0;
				break;
			default:
				fec_msg.pan_step = 0;
				fec_msg.tilt_step = 0;
				fec_msg.zoom_step = 0;
				fec_msg.step = 1;
				break;
		}
		
	}
	fec_msg.app_type = -1;
	
	printf("[fec] ch=%d, mode=%d, focal=%.2f, cell_idx=%d step=%d ptz_unit= (%2.2f, %2.2f, %2.2f), app_type=%d \n", 
	fec_msg.channel, fec_msg.mode, fec_msg.focal, fec_msg.cell_idx, fec_msg.step, fec_msg.pan_step, fec_msg.tilt_step, fec_msg.zoom_step, fec_msg.app_type);

	send_msg(&msg_context, (uint8_t*)&fec_msg, sizeof(msg_fec_t));
	return 0;
}

static eMSTRM_SHOW_MODE showMode2mstrmMode(emSDK_ENC_SHOW_MODE showMode)
{
	eMSTRM_SHOW_MODE retMode = eMSTRM_SHOW_MODE_ORIGIN;

	switch(showMode){
		default:
		case eSDK_ENC_SHOW_MODE_WALL_ORIGIN:
		case eSDK_ENC_SHOW_MODE_CELL_ORIGIN:
		case eSDK_ENC_SHOW_MODE_TABLE_ORIGIN:
			retMode = eMSTRM_SHOW_MODE_ORIGIN;
			break;
		case eSDK_ENC_SHOW_MODE_CELL_180:
		case eSDK_ENC_SHOW_MODE_WALL_180:
			retMode = eMSTRM_SHOW_MODE_P180S;
			break;
		case eSDK_ENC_SHOW_MODE_TABLE_360:
		case eSDK_ENC_SHOW_MODE_CELL_360:
			retMode = eMSTRM_SHOW_MODE_360;
			break;
		case eSDK_ENC_SHOW_MODE_CELL_WALL_SPLIT:
		case eSDK_ENC_SHOW_MODE_WALL_WALL_SPLIT:
			retMode = eMSTRM_SHOW_MODE_1P2R;
			break;
		case eSDK_ENC_SHOW_MODE_TABLE_SPLIT:
		case eSDK_ENC_SHOW_MODE_CELL_SPLIT:
		case eSDK_ENC_SHOW_MODE_WALL_SPLIT:
			retMode = eMSTRM_SHOW_MODE_1O3R;
			break;
		case eSDK_ENC_SHOW_MODE_CELL_4R:
		case eSDK_ENC_SHOW_MODE_TABLE_4R:
		case eSDK_ENC_SHOW_MODE_WALL_4R:
			retMode = eMSTRM_SHOW_MODE_4R;
			break;
		case eSDK_ENC_SHOW_MODE_TABLE_VR:
			retMode = eMSTRM_SHOW_MODE_SPHERE;
			break;
		case eSDK_ENC_SHOW_MODE_CELL_KITR:
		case eSDK_ENC_SHOW_MODE_TABLE_KITR:
		case eSDK_ENC_SHOW_MODE_WALL_KITR:
			retMode = eMSTRM_SHOW_MODE_KIT_1R;
			break;
		case eSDK_ENC_SHOW_MODE_CELL_KITO:
		case eSDK_ENC_SHOW_MODE_TABLE_KITO:
		case eSDK_ENC_SHOW_MODE_WALL_KITO:	
			retMode = eMSTRM_SHOW_MODE_KIT_1RO;
			break;
	}

	return retMode;
}

static eMSTRM_FIX_MODE fixMode2mstrmMode(emSDK_ENC_FIX_MODE fixMode)
{
	eMSTRM_FIX_MODE retMode = eMSTRM_FIX_MODE_CELL;
	
	switch(fixMode){
		default:
		case eSDK_ENC_FIX_MODE_CELL:
			retMode = eMSTRM_FIX_MODE_CELL;
			break;
		case eSDK_ENC_FIX_MODE_TABLE:
			retMode = eMSTRM_FIX_MODE_TABLE;
			break;
		case eSDK_ENC_FIX_MODE_WALL:
			retMode = eMSTRM_FIX_MODE_WALL;
			break;
	}

	return retMode;
}

static void* usr_mode_stream_changed(void *arg)
{
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, "usr_mode_stream_changed");
	int *mode = (int *)arg;
	printf("%s:%d\n", __FUNCTION__, *mode);
	if(NULL != mode &&  (*mode == eMSTRM_SHOW_MODE_KIT_1R || *mode == eMSTRM_SHOW_MODE_KIT_1RO)){
		FILE *fid = fopen(ACTUAL_STREAM, "w+b");
		char buf[16]= "4";
		if(NULL != fid){
			fwrite(buf, 1, strlen(buf), fid);
			fclose(fid);
			fid = NULL;
		}else{
			printf("open %s failed\n", ACTUAL_STREAM);
		}
	}else{
		printf("start to remove actual stream\n");
		if(0 == unlink(ACTUAL_STREAM)){
			if(0 == remove(ACTUAL_STREAM)){
 			}
		}
	}

	sleep(3);
	printf("usr_mode_stream_changed\n");
	system("reboot");
	return 0;
}

static emSENSOR_MODEL va_isp_api_set_sensor_model()
{
    char type[8];
    FILE *file = NULL;
    file = fopen(SENSOR_TYPE_FILE, "rb");
    if(file == NULL) {
        printf("Err:read sensor type file fail!\n");
        return -1;
    }
    fread(type, sizeof(type), 1, file);
    fclose(file);
    if(!strncmp(type, "SC2045", 6)) {
        return SENSOR_MODEL_SMARTSENS_SC2045;

    }
    else if(!strncmp(type, "AR0330", 6)) {
        return SENSOR_MODEL_APTINA_AR0330;

    }
    else {
        return SENSOR_MODEL_SMARTSENS_SC2045;

    }

}

static int enc_usr_mode(int vin, int stream, int fix_mode, int show_mode)
{
	MsgContext msg_context;
	msg_fec_t fec_msg;
	char hostname[32];
	int stream_tmp = 0;//stream;

	memset(&msg_context, 0, sizeof(MsgContext));
	memset(&fec_msg, 0, sizeof(msg_fec_t));

	snprintf(hostname, sizeof(hostname), "source%d", vin);
	msg_context.host = hostname;

	
	msg_context.cmd = "setupFC";

    if(gs_sensorModel == SENSOR_MODEL_SMARTSENS_SC2045) {
        // SC2045目前不需要发送修改模式的命令
    }
    else if(gs_sensorModel == SENSOR_MODEL_APTINA_AR0330) {
        /*  send msg_fec_t structure */
        fec_msg.lens = 2;
        fec_msg.channel = 0;
        fec_msg.mode = (int)showMode2mstrmMode((emSDK_ENC_SHOW_MODE)show_mode);
        fec_msg.app_type = (int)fixMode2mstrmMode((emSDK_ENC_FIX_MODE)fix_mode);

        // +++ fac_conf.ini浼氳缃繖閮ㄥ垎鍙傛暟
        fec_msg.pan = PARAM_UNSED;
        fec_msg.tilt = PARAM_UNSED;
        fec_msg.zoom = PARAM_UNSED;
        fec_msg.focal = 0;

        if((_sdk_enc.attr.show_mode[vin][stream_tmp] >= eMSTRM_SHOW_MODE_KIT_1R && 
            fec_msg.mode < eMSTRM_SHOW_MODE_KIT_1R) ||
            (_sdk_enc.attr.show_mode[vin][stream_tmp] < eMSTRM_SHOW_MODE_KIT_1R && 
            fec_msg.mode >= eMSTRM_SHOW_MODE_KIT_1R)){
            pthread_t pid = NULL;
            _sdk_enc.attr.show_mode[vin][stream_tmp] = fec_msg.mode;
            pthread_create(&pid, NULL, usr_mode_stream_changed, (void *)&_sdk_enc.attr.show_mode[vin][stream_tmp]);
        }

        _sdk_enc.attr.fix_mode[vin][stream_tmp] = fec_msg.app_type;
        _sdk_enc.attr.show_mode[vin][stream_tmp] = fec_msg.mode;
        fec_msg.mode %= eMSTRM_SHOW_MODE_KIT_1RO;
        printf("fix mode:%d, show mode:%d\n", fix_mode, show_mode);
        printf("app_type:%d, mode:%d\n", fec_msg.app_type, fec_msg.mode);
        send_msg(&msg_context, (uint8_t*)&fec_msg, sizeof(msg_fec_t));
    }

	//sub stream
	fec_msg.channel = 1;
	fec_msg.mode = eMSTRM_SHOW_MODE_KIT_1R;
	//fec_msg.mode = (int)showMode2mstrmMode((emSDK_ENC_SHOW_MODE)show_mode);
	fec_msg.app_type = (int)fixMode2mstrmMode((emSDK_ENC_FIX_MODE)fix_mode);
	send_msg(&msg_context, (uint8_t*)&fec_msg, sizeof(msg_fec_t));

	fec_msg.channel = 2;
	send_msg(&msg_context, (uint8_t*)&fec_msg, sizeof(msg_fec_t));

	fec_msg.channel = 3;
	send_msg(&msg_context, (uint8_t*)&fec_msg, sizeof(msg_fec_t));

	usleep(100000);

	return 0;
}

static void *enc_upgrade_env_prepare(void *arg)
{
	FILE *fp1, *fp2;
		
	fp1 = popen("killall -9 ash", "r");
	if(fp1){
		pclose(fp1);
	}

	fp2 = popen("killall -9 mstrm_venc_2stream", "r");
	if(fp2){
		pclose(fp2);
	}

	fp2 = popen("killall -9 mstrm_venc", "r");
	if(fp2){
		pclose(fp2);
	}

	if(arg){
		ssize_t length = *((ssize_t *)arg);
		return (void*)MemBroker_GetMemory(length, 0);
	}else{
		return NULL;
	}	
}

static int enc_update_overlay_by_text(int vin, int stream, const char* text)
{
	msg_osd_t osd_msg;
	MsgContext msg_context;
	char hostname[32];
	
	memset(&osd_msg, 0, sizeof(msg_osd_t));	
	memset(&msg_context, 0, sizeof(MsgContext));

	snprintf(hostname, sizeof(hostname), "encoder%d", stream);
	msg_context.host = hostname;
	msg_context.host_len = strlen(msg_context.host) + 1;
	msg_context.cmd = "changeOSD";	

	char channelNameutf8[64];
	//fix me: use gb2312_to_utf8.c in src
	/*if(gb2312_to_utf8(0, text, channelNameutf8, sizeof(channelNameutf8))){
		printf("UTF8-\n");
		int i;
		for(i=0;i< strlen(channelNameutf8); i++){
			printf("%02x-", channelNameutf8[i]);
		}
		printf("\n");
		for(i=0;i< strlen(text); i++){
			printf("%02x-", text[i]);
		}
		printf("\n");
		snprintf(osd_msg.str, 64, "%s", channelNameutf8);
	}else{
		snprintf(osd_msg.str, 64, "%s", text);
	}*/
	printf("channelname:%s\n", text);
	snprintf(osd_msg.str, 64, "%s", text);			
	//send_msg(&msg_context, (uint8_t*)&osd_msg, sizeof(msg_osd_t));
	return 0;
}

static int enc_resolution(int width, int height)
{	
	return -1;
}

static stSDK_ENC_M388C _sdk_enc =
{
	// init the interfaces
	.api = {
		// h264 stream
		.create_stream_h264 = enc_create_stream_h264,
		.release_stream_h264 = enc_release_stream_h264,
		.enable_stream_h264 = enc_enable_stream_h264,
		.set_stream_h264 = enc_set_stream_h264,
		.get_stream_h264 = enc_get_stream_h264,
		.request_stream_h264_keyframe = enc_request_stream_h264_keyframe,

		//h265 stream
		.create_stream_h265 = enc_create_stream_h265,
		.release_stream_h265 = enc_release_stream_h265,
		.enable_stream_h265 = enc_enable_stream_h265,
		.set_stream_h265 = enc_set_stream_h265,
		.get_stream_h265 = enc_get_stream_h265,
		.request_stream_h265_keyframe = enc_request_stream_h265_keyframe,
		
		.create_stream_g711a = enc_create_stream_g711a,
		.release_stream_g711a = enc_release_stream_g711a,

		// snapshot a picture
		.snapshot = enc_snapshot,
		// overlay
		.create_overlay_canvas = enc_create_overlay_canvas,
		.load_overlay_canvas = enc_load_overlay_canvas,
		.release_overlay_canvas = enc_release_overlay_canvas,
		.create_overlay = enc_create_overlay,
		.release_overlay = enc_release_overlay,
		.get_overlay_canvas = enc_get_overlay_canvas,
		.show_overlay = enc_show_overlay,
		.update_overlay = enc_update_overlay,

		// encode start / stop
		.start = enc_start,
		.stop = enc_stop,

		//fish eye
		.eptz_ctrl = enc_eptz_ctrl,
		.enc_mode = enc_usr_mode,
		.upgrade_env_prepare = enc_upgrade_env_prepare,
		.update_overlay_bytext = enc_update_overlay_by_text,


		//switch resolution
		.enc_resolution = enc_resolution,
	},
};

static int get_actual_stream()
{
	int ret;
	if(-1 != access(ACTUAL_STREAM, F_OK)){
		_sdk_enc.attr.show_mode[0][0] = eMSTRM_SHOW_MODE_KIT_1RO;
		ret = 4;
	}else{
		_sdk_enc.attr.show_mode[0][0] = eMSTRM_SHOW_MODE_ORIGIN;
		ret = 2;
	}
	printf("stream_count = %d\n", ret);
	return ret;
}

int SDK_ENC_init()
{
	int i = 0, ii = 0;
	// only 'sdk_enc' pointer is NULL could be init
	if(NULL == sdk_enc){
		// set handler pointer
		sdk_enc = (lpSDK_ENC_API)(&_sdk_enc);
		
		// clear the buffering callback
		sdk_enc->do_buffer_request = NULL;
		sdk_enc->do_buffer_append = NULL;
		sdk_enc->do_buffer_commit = NULL;

		//get actual stream by file
		_sdk_enc.attr.actual_stream = get_actual_stream();
		// init the internal attribute value
		// clear the stream attrubutes
		// clear the frame counter
		
		for(i = 0; i < VA_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < VA_VENC_STREAM_BACKLOG_REF; ++ii){
				LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[i][ii];				
				LP_SDK_ENC_STREAM_H265_ATTR const streamH265Attr = &_sdk_enc.attr.h265_attr[i][ii];
				
				uint8_t *const frame_ref_counter = &_sdk_enc.attr.frame_ref_counter[i][ii];

				STREAM_H264_CLEAR(streamH264Attr);
				STREAM_H264_CLEAR(streamH265Attr);
				*frame_ref_counter = 0;
			}
		}
		
		// init the snapshot mutex
		pthread_mutex_init(&_sdk_enc.attr.snapshot_mutex, NULL);
		// start
		//sdk_enc->start();
		// success to init
		gs_sensorModel = va_isp_api_set_sensor_model();
		return 0;
	}
	return -1;
}
int SDK_ENC_wdr_destroy()
{	
	if(sdk_enc){
		int i = 0, ii = 0;
	   // release the video encode
		for(i = 0; i < VA_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < _sdk_enc.attr.actual_stream; ++ii){	// destroy sub stream firstly
				switch(_sdk_enc.attr.enType[i][ii]){
				default:
				case kSDK_ENC_BUF_DATA_H264:					
					sdk_enc->release_stream_h264(i, ii);
					break;
				case kSDK_ENC_BUF_DATA_H265:
					sdk_enc->release_stream_h265(i, ii);
					break;	
			    }				
		    }
		}
		return 0;
	}
	return -1;

}

int SDK_ENC_destroy()
{
	if(sdk_enc){
		int i = 0, ii = 0;
		// destroy the snapshot mutex
		pthread_mutex_destroy(&_sdk_enc.attr.snapshot_mutex);
		// stop encode firstly
		sdk_enc->stop();
		// release the canvas stock
		for(i = 0; i < VA_VENC_OVERLAY_CANVAS_STOCK_REF; ++i){
			sdk_enc->release_overlay_canvas(_sdk_enc.attr.canvas_stock + i);
		}
		// release the audio encode
		for(i = 0; i < VA_AENC_CH_BACKLOG_REF; ++i){
			sdk_enc->release_stream_g711a(i);
		}
		// release the video encode
		for(i = 0; i < VA_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < _sdk_enc.attr.actual_stream; ++ii){
				switch(_sdk_enc.attr.enType[i][ii]){
					default:
					case kSDK_ENC_BUF_DATA_H264:					
						sdk_enc->release_stream_h264(i, ii);
						break;
					case kSDK_ENC_BUF_DATA_H265:
						sdk_enc->release_stream_h265(i, ii);
						break;	
				}	
			}
		}
		// clear handler pointer
		sdk_enc = NULL;
		// success to destroy
		return 0;
	}
	return -1;
}


int SDK_ENC_create_stream(int vin, int stream, LP_SDK_ENC_STREAM_ATTR stream_attr)
{	
	int ret;
	_sdk_enc.attr.enType[vin][stream] = stream_attr->enType;

	switch(stream_attr->enType){
		default:
		case kSDK_ENC_BUF_DATA_H264:
			ret = sdk_enc->create_stream_h264(vin, stream,&stream_attr->H264_attr);
			break;
		case kSDK_ENC_BUF_DATA_H265:
			ret = sdk_enc->create_stream_h265(vin, stream,&stream_attr->H265_attr);
			break;	
	}
	return ret;
}



int SDK_ENC_release_stream(int vin, int stream)
{
	int ret;
	switch(_sdk_enc.attr.enType[vin][stream]){
		default:
		case kSDK_ENC_BUF_DATA_H264:
			ret = sdk_enc->release_stream_h264(vin,stream);
			break;
		case kSDK_ENC_BUF_DATA_H265:
			ret = sdk_enc->release_stream_h265(vin,stream);
			break;	
	}
	return ret;
}



int SDK_ENC_set_stream(int vin, int stream,LP_SDK_ENC_STREAM_ATTR stream_attr)
{
	int ret;
	switch(stream_attr->enType){
		default:
		case kSDK_ENC_BUF_DATA_H264:
			ret = sdk_enc->set_stream_h264(vin, stream,&stream_attr->H264_attr);
			break;
		case kSDK_ENC_BUF_DATA_H265:
			ret = sdk_enc->set_stream_h265(vin, stream,&stream_attr->H265_attr);
			break;	
	}
	

	return ret;
}


int SDK_ENC_get_stream(int vin, int stream, LP_SDK_ENC_STREAM_ATTR stream_attr)
{
	int ret;
	switch(_sdk_enc.attr.enType[vin][stream]){
		default:
		case kSDK_ENC_BUF_DATA_H264:
			ret = sdk_enc->get_stream_h264(vin, stream,&stream_attr->H264_attr);
			stream_attr->enType = kSDK_ENC_BUF_DATA_H264;		
			break;
		case kSDK_ENC_BUF_DATA_H265:
			ret = sdk_enc->get_stream_h265(vin, stream,&stream_attr->H265_attr);			
			stream_attr->enType = kSDK_ENC_BUF_DATA_H265;
			break;	
	}

	return ret;
}

int SDK_ENC_enable_stream(int vin, int stream, bool flag)
{
	int ret;
	switch(_sdk_enc.attr.enType[vin][stream]){
		default:
		case kSDK_ENC_BUF_DATA_H264:
			ret = sdk_enc->enable_stream_h264(vin, stream, flag);
			break;
		case kSDK_ENC_BUF_DATA_H265:
			ret = sdk_enc->enable_stream_h265(vin, stream, flag);
			break;	
	}
	return ret;
}

int	SDK_ENC_request_stream_keyframe(int vin, int stream)
{	
	switch(_sdk_enc.attr.enType[vin][stream]){
		default:
		case kSDK_ENC_BUF_DATA_H264:
			sdk_enc->request_stream_h264_keyframe(vin, stream);
			break;
		case kSDK_ENC_BUF_DATA_H265:
			sdk_enc->request_stream_h265_keyframe(vin, stream);
			break;	
	}
	return 0;
}

enSDK_ENC_BUF_DATA_TYPE  SDK_ENC_request_venc_type(int vin, int stream)
{
	enSDK_ENC_BUF_DATA_TYPE enc_type = kSDK_ENC_BUF_DATA_H264;

	return enc_type;
}

int SDK_ENC_eptz_ctrl(int nChn, int enCmd, unsigned char u8Param)
{
	return sdk_enc->eptz_ctrl(0, nChn, enCmd, u8Param);	
}



