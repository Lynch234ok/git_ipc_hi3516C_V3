
#include <unistd.h>
#include <getopt.h>

#include "version.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"
#include "sensor.h"
#include "demo.h"
#include "n1_protocol/n1_device.h"

extern 	NK_N1Device public_N1Device;


static void demo_signal_escape(int sig)
{
	printf("Hey!\n");
	fclose(stdin);
	exit(0);
}

static void demo_signal_init()
{
	signal(SIGINT, demo_signal_escape);
	signal(SIGQUIT, demo_signal_escape);
}

static void demo_signal_destroy()
{
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
}

static void demo_mediabuf_avenc_init()
{
	int i = 0, ii = 0, iii = 0;
	int stream_cnt = 2;

	MEDIABUF_init();
	SDK_ENC_init();

	sdk_enc->do_buffer_request = MEDIABUF_in_request;
	sdk_enc->do_buffer_append = MEDIABUF_in_append;
	sdk_enc->do_buffer_commit = MEDIABUF_in_commit;

	for(i = 0; i < 1; ++i)
	{
		// video input
		for(ii = 0; ii < stream_cnt; ++ii)
		{
			// video h264 encode
			char preferred_name[32] = {""};
			char alternate_name[32] = {""};

			if(stream_cnt > 3)
			{//just for multi channel IP camera
				sprintf(preferred_name, "ch%d_0.264", ii);
			}
			else
			{
				sprintf(preferred_name, "ch%d_%d.264", i, ii);
			}
			//fix me
			if(ii ==0)
			{
				//main stream
				sprintf(alternate_name, "720p.264");
			}
			else if(ii == 1)
			{
				//sub stream 1
				sprintf(alternate_name, "360p.264");
			}
			else
			{
				//sub stream 2
				sprintf(alternate_name, "qvga.264");
			}

			int const entry_available = 400;
			int const entry_key_available = 1;
			int user_available;
			switch(stream_cnt)
			{
				default:
				case 3:
				{
							if(ii == 0)
							{	
								user_available = 5;
							}
							else
							{
								user_available = 8;
							}
				}
				break;	
				case 2:
				{
					//HI3518E
					if(ii == 0)
					{	
						user_available = 5;
					}
					else
					{
						user_available = 8;
					}
				}
					break;
			}
			
			if(0 == MEDIABUF_new(0, preferred_name, alternate_name, entry_available, entry_key_available, user_available))
			{
				// success to new a mediabuf

				
				ST_SDK_ENC_STREAM_ATTR stream_attr;
		
				LP_SDK_ENC_STREAM_H264_ATTR stream_h264_attr = &stream_attr.H264_attr;				
				LP_SDK_ENC_STREAM_H265_ATTR stream_h265_attr = &stream_attr.H265_attr;
				
					
				int venc_id = 0;
				int const buf_id = MEDIABUF_lookup_byname(preferred_name);
		 //   	int stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR;
				int stream_rc_mode;

		//		strcpy(stream_h264_attr.name, preferred_name);

				// start video stream
				// FIXME:
				
//					switch(venc_ch.codecType){
//						default:
//						case kNSDK_CODEC_TYPE_H264:
					if(ii == 0)
					{
							stream_attr.enType = kSDK_ENC_BUF_DATA_H264;
							stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR;
							strcpy(stream_h264_attr->name, preferred_name);
							
							stream_h264_attr->width = 1280;
							stream_h264_attr->height = 720;
							
							printf("\n\n\nresolution: %dx%d\n\n\n", stream_h264_attr->width, stream_h264_attr->height);
							stream_h264_attr->fps = 25;
							stream_h264_attr->gop = 50;
							
							stream_h264_attr->profile = 1;
							stream_rc_mode = kSDK_ENC_H264_RC_MODE_VBR; 
//							break;
							stream_h264_attr->rc_mode = stream_rc_mode;
							stream_h264_attr->bps = 2000;
							stream_h264_attr->quality = 0;
							stream_h264_attr->buf_id = buf_id;
							
					
							SDK_ENC_create_stream(i, ii, &stream_attr);
							SDK_ENC_enable_stream(i, ii, true);
					}
					else if(ii == 1)
					{
									stream_attr.enType = kSDK_ENC_BUF_DATA_H264;
									stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR;
									strcpy(stream_h264_attr->name, preferred_name);
									
									stream_h264_attr->width = 640;
									stream_h264_attr->height = 360;
									
									printf("\n\n\nresolution: %dx%d\n\n\n", stream_h264_attr->width, stream_h264_attr->height);
									stream_h264_attr->fps = 25;
									stream_h264_attr->gop = 50;
									
									stream_h264_attr->profile = 1;
									stream_rc_mode = kSDK_ENC_H264_RC_MODE_VBR; 
		//							break;
									stream_h264_attr->rc_mode = stream_rc_mode;
									stream_h264_attr->bps = 2000;
									stream_h264_attr->quality = 0;
									stream_h264_attr->buf_id = buf_id;
									
							
									SDK_ENC_create_stream(i, ii, &stream_attr);
									SDK_ENC_enable_stream(i, ii, true);
					}

		//					break;
	//					case kNSDK_CODEC_TYPE_H265:							
							
//							break;	
//					}					
			}
		}
	}

		//sdk_enc->create_stream_g711a(0, 0);

}

static void demo_mediabuf_avenc_destroy()
{
	SDK_ENC_destroy();
	SDK_destroy_vin();//fix me
	MEDIABUF_destroy();
}


int Demo_init()
{
	NK_N1Device *N1Device;

	demo_signal_init();

	SDK_init_sys(PRODUCT_CLASS);
	
	printf("%s\r\n", PRODUCT_CLASS);
	SENSOR_init(SOC_MODEL);


	SDK_init_audio(kSDK_AUDIO_HW_SPEC_IGNORE);
	sdk_audio->init_ain(kSDK_AUDIO_SAMPLE_RATE_8000, 16);
	sdk_audio->create_ain_ch(0);
	//sdk_audio->set_aout_loop(0);

	// sdk init video input
	SDK_init_vin(kSDK_VIN_HW_SPEC_IGNORE);

	
	demo_mediabuf_avenc_init();
	

	sdk_enc->start();
	sdk_vin->start();

	//N1协议初始化
	if(NULL != (N1Device = n1_Device_main("N101234567890123456666")));
	{
		printf("N1Device init complete\n");
		return 0;
	}
	
}

void Demo_destroy()
{
	printf("at Demo_destroy...");
	
	demo_mediabuf_avenc_destroy();
	sdk_audio->release_ain_ch(0);
	sdk_audio->destroy_ain();
	SDK_destroy_audio();
	//LVIEW_destroy();
	SENSOR_destroy();
	SDK_destroy_sys();

	demo_signal_destroy();

	/**
	 * 销毁 N1 设备环境。
	 */
	NK_N1Device_Destroy(&public_N1Device);
	//SDK_destroy_vin();//fix me :call alert
	printf("Demo_destroy done");
}

int Demo_exec()
{
	usleep(100000);
	return 0;
	char inbuf[128];
#define _IAPP_EXEC_POLL_INPUT() \
	(memset(inbuf, 0, sizeof(inbuf)), fgets(inbuf, sizeof(inbuf) - 1, stdin))
#define _IAPP_EXEC_MATCH_INPUT(cmd) \
	(0 == strncasecmp(inbuf, (cmd), strlen(cmd)))

	if(_IAPP_EXEC_POLL_INPUT()){
		if(_IAPP_EXEC_MATCH_INPUT("quit")){
			return -1;
		}
		
		
		return 0;
	}
	printf("stdin error\n");
	exit(1);
	return -1;
}

#if 0
char* UCODEC_get_id()
{
	char* id_buf[64];
	memset(id_buf, 0, sizeof(id_buf));

	if(!UCODE_check(UCODE_SN_MTD, -1)){
		UCODE_read(UCODE_SN_MTD, -1, id_buf);
	}
	return id_buf;
}
#endif

static void demo_process_arg(int argc, char *argv[])
{
	int i = 0, c = 0;
	struct option long_opt[] = {
		{
			.name = 	"webdir",
			.has_arg = 1,
			.flag = NULL,
			.val = 'w',
		},
		{
			.name = 	"fontdir",
			.has_arg = 1,
			.flag = NULL,
			.val = 'f',
		},
		{
			.name = 	"flashmap",
			.has_arg = 1,
			.flag = NULL,
			.val = 'm',
		},
		{
			.name = 	"sysconf",
			.has_arg = 1,
			.flag = NULL,
			.val = 's',
		},
		{
			.name = 	"defnetsdk",
			.has_arg = 1,
			.flag = NULL,
			.val = 'd',
		},
		{
			.name = 	"netsdk",
			.has_arg = 1,
			.flag = NULL,
			.val = 'n',
		},
		{
			.name = 	"ispcfg",
			.has_arg = 1,
			.flag = NULL,
			.val = 'i',
		},
		// end
		{
			.name = 	NULL,
			.has_arg = 0,
			.flag = NULL,
			.val = 0,
		},
	};
	char short_opt[(sizeof(long_opt) / sizeof(long_opt[0])) * 2] = {""};
	
	// make the short option string
	for(i = 0; i < sizeof(long_opt) / sizeof(long_opt[0]); ++i){
		struct option *l_opt = long_opt + i;
		if(!l_opt->name){
			break;
		}

		strncat(short_opt, (char*)(&l_opt->val), 1);
		if(1 == l_opt->has_arg){
			strcat(short_opt, ":");
		}
	}

	// set the default environment
	//setenv("WEBDIR", Demo_ENV_WEB_DIR, 1);
	//setenv("FONTDIR", Demo_ENV_FONT_DIR, 1);
	//setenv("FLASHMAP", Demo_ENV_FLASH_MAP, 1);
	//setenv("SYSCONF", Demo_ENV_SYSCONF, 1);
	//setenv("DEFNETSDK", Demo_ENV_DEFNETSDK_DIR, 1);
	//setenv("NETSDK", Demo_ENV_NETSDK_DIR, 1);
	//setenv("ISPCFG", Demo_ENV_ISPCFG_DIR, 1);

	// read the options
	while((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1){
		switch(c){
		case 'w':
			setenv("WEBDIR", optarg, 1);
			break;
		case 'f':
			setenv("FONTDIR", optarg, 1);
			break;
		case 'm':
			setenv("FLASHMAP", optarg, 1);
			break;
		case 's':
			setenv("SYSCONF", optarg, 1);
			break;
		case 'd':
			setenv("DEFNETSDK", optarg, 1);
			break;
		case 'n':
			setenv("NETSDK", optarg, 1);
			break;
		case 'i':
			setenv("ISPCFG", optarg, 1);
			break;
		default:
			;
		}
	}

	//system("printenv");exit(1);
	
}

int Demo_main(int argc, char** argv)
{	
	printf("enter main application\r\n");
	Demo_init();
	//printf("demo init complete\n");
	atexit(Demo_destroy);
	//printf("demo destroy complete\n");
	while(0 == Demo_exec());
	Demo_destroy();

	exit(0);
}

