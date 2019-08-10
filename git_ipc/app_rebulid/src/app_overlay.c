
#include "app_overlay.h"
//#include "conf.h"
#include "sysconf.h"
#include "netsdk.h"
#include "sdk/sdk_api.h"
#include "global_runtime.h"
#include "app_debug.h"
#include "securedat.h"
#include "model_conf.h"
#include "jalaali.h"
#include <sys/prctl.h>


#define kAPP_OVERLAY_CLOCK_NAME "clock"
#define kAPP_OVERLAY_TITLE_NAME "title"
#define kAPP_OVERLAY_ID_NAME "id"
#define kAPP_OVERLAY_PIC_NAME "pic"
#define kAPP_OVERLAY_CAUTION_NAME "caution"
#define kAPP_OVERLAY_table_NAME  "table"




static bool _overlay_chs = false;
static bool _update_overlay_table_flag;
static bool _overlay_table_change_flag;
static bool _overlay_table_create_flag = false;

static int app_overlay_get_osd_stream()
{
	ST_MODEL_CONF model_conf;
	if(NULL != MODEL_CONF_get(&model_conf)){
		if(MODEL_CONF_check_int_valid(model_conf.osd.osd_stream_num)){
			return model_conf.osd.osd_stream_num;
		}
	}
	return NETSDK_venc_get_channels();
}

static int app_overlay_resolution_to_ratio(int venc_width, int venc_height)
{
	ST_MODEL_CONF model_conf;
	if(NULL != MODEL_CONF_get(&model_conf)){
		if(MODEL_CONF_check_int_valid(model_conf.osd.osd_radio)){
			return model_conf.osd.osd_radio;
		}
	}

	if((2048 * 1536) < (venc_width * venc_height))
	{
		return 5;
	}
	else if((1280 * 960) < (venc_width * venc_height))
	{
		return 3;
	}
	else if((960 * 576) < (venc_width * venc_height))
	{
		return 2;
	}
	else if((720 * 576) < (venc_width * venc_height))
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

static void app_overlay_resolution_get(int vin_ex, bool freeResolution, int *venc_width, int *venc_height, int *ratio)
{
	if(0 <= vin_ex && NULL != venc_width && NULL != venc_height){
		ST_NSDK_VENC_CH venc_ch_ex;
		NETSDK_conf_venc_ch_get(vin_ex, &venc_ch_ex);
		if(freeResolution){
			*venc_width = venc_ch_ex.resolutionWidth;
			*venc_height = venc_ch_ex.resolutionHeight;
		}else{
			*venc_width = (venc_ch_ex.resolution >> 16) & 0xffff;
			*venc_height = (venc_ch_ex.resolutionHeight >> 0) & 0xffff;
		}
		if(NULL != ratio){
			*ratio = app_overlay_resolution_to_ratio(*venc_width, *venc_height);
		}
	}
}

static int app_overlay_show(int vin, const char *overlay_name, bool show_flag)
{
	int i = 0;
	SYSCONF_t *sysconf = SYSCONF_dup();
	int const stream_cnt = app_overlay_get_osd_stream(); // FIXME:
	
	for(i = 0; i < stream_cnt; ++i){
		sdk_enc->show_overlay(vin, i, overlay_name, show_flag);
	}
	return 0;
}

static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_title_sub = NULL;
static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_title_main = NULL;

static char app_overlay_font_main = 32;
static char app_overlay_font_sub = 16;

static int app_overlay_set_title(int vin, const char *title)
{
	int i = 0;
	if(NULL != title && strlen(title)>0){
		bool chs_title = false;
		int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini
		ST_NSDK_VENC_CH venc_ch;
		int const title_len = strlen(title);
		// check chinese simplistic
		if(title_len > 0){
			for(i = 0; i < strlen(title) - 1; ++i){
				char *chr = &title[i];
				if(*chr >= 0xa0){
					char const qu_code = *chr; // 87
					if(qu_code >= 0xb0 && qu_code <= 0xf7){
						chs_title = true;
						break;
					}
				}
			}
		}

		if(chs_title){
			_overlay_chs = true;
		}else{
			_overlay_chs = false;
		}

		if(NETSDK_conf_venc_ch_get((vin + 1) * 100 + 1, &venc_ch)){
			for(i = 0; i < stream_cnt; ++i){
				//update overlay in sdk
				if(0 == sdk_enc->update_overlay_bytext(0, i, title)){
					continue;
				}
				
				int venc_width = 0, venc_height = 0;
				int ratio = 0;
				app_overlay_resolution_get((vin + 1) * 100 + 1 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
				if((venc_width <= 720 || venc_height < 576) && (i > 0)){
					if(_canvas_title_sub){
						_canvas_title_sub->erase_rect(_canvas_title_sub, 0, 0, 0, 0);
						OVERLAY_put_text(_canvas_title_sub, 0, 0, app_overlay_font_sub, title, 0, ratio);
					}
				}else{
					if(_canvas_title_main){
						_canvas_title_main->erase_rect(_canvas_title_main, 0, 0, 0, 0);
						OVERLAY_put_text(_canvas_title_main, 0, 0, app_overlay_font_main, title, 0, ratio);
					}
				}
				sdk_enc->update_overlay(vin, i, kAPP_OVERLAY_TITLE_NAME);
			}
		}
	}
	return 0;
}


int APP_OVERLAY_create_title(int vin)
{
#ifdef	DEL_OSD
	//fix me: 需要解决16D OSD不显示时会出错的问题(p系列不显示OSD)
	if(g_authorized){
		return 0;
	}
#endif

	int i = 0;
	int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini
	ST_NSDK_VENC_CH venc_ch;

	if(NETSDK_conf_venc_ch_get((vin + 1) * 100 + 1, &venc_ch)){
		for(i = 0; i < stream_cnt; ++i){
			if(NULL == sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_TITLE_NAME)){
				LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas_use = NULL;
				int venc_width = 0, venc_height = 0;
				int ratio = 0;
				app_overlay_resolution_get((vin + 1) * 100 + 1 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
				
				if((venc_width <= 720 || venc_height < 576) && (i > 0)){	
					if(!_canvas_title_sub){
						_canvas_title_sub = sdk_enc->create_overlay_canvas(app_overlay_font_sub * 16 * ratio, app_overlay_font_sub * ratio);
					}
					canvas_use = _canvas_title_sub;
				}else{
					if(!_canvas_title_main){
						_canvas_title_main = sdk_enc->create_overlay_canvas(app_overlay_font_main * 16 * ratio, app_overlay_font_main * ratio);
					}
					canvas_use = _canvas_title_main;
				}

				if(g_authorized){
					sdk_enc->create_overlay(vin, i, kAPP_OVERLAY_TITLE_NAME, venc_ch.channelNameOverlay.o.regionX / 100.0, venc_ch.channelNameOverlay.o.regionY / 100.0, canvas_use);
				}else{
					sdk_enc->create_overlay(vin, i, kAPP_OVERLAY_TITLE_NAME, 0.2, 0.48, canvas_use);
				}
				sdk_enc->update_overlay(vin, i, kAPP_OVERLAY_TITLE_NAME);
				
			}
		}

		if(g_authorized){
			app_overlay_set_title(vin, venc_ch.channelName);
			app_overlay_show(vin, kAPP_OVERLAY_TITLE_NAME, venc_ch.channelNameOverlay.o.enabled);
		}else{
			app_overlay_set_title(vin, "(Unauthorized)产品未被授权");
			app_overlay_show(vin, kAPP_OVERLAY_TITLE_NAME, true);
		}
	}
	return 0;
		
}

void APP_OVERLAY_release_title(int vin)
{
#ifdef	DEL_OSD
		//fix me: 需要解决16D OSD不显示时会出错的问题(p系列不显示OSD)
		if(g_authorized){
			return 0;
		}
#endif

	int i = 0;
	int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini

	if(sdk_enc){
		for(i = 0; i < stream_cnt; ++i){
			if(NULL != sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_TITLE_NAME)){
				sdk_enc->release_overlay(vin, i, kAPP_OVERLAY_TITLE_NAME);
			}
		}
		if(_canvas_title_sub){
			sdk_enc->release_overlay_canvas(_canvas_title_sub);
			_canvas_title_sub = NULL;
		}
		if(_canvas_title_main){
			sdk_enc->release_overlay_canvas(_canvas_title_main);
			_canvas_title_main = NULL;
		}
	}
}

void APP_OVERLAY_reload_title(int vin)
{
	APP_OVERLAY_release_title(vin);
	APP_OVERLAY_create_title(vin);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_clock_sub = NULL;
static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_clock_main = NULL;


static void app_overlay_update_clock(time_t t) // per second
{
	int i = 0, ii = 0;
	int const stream_cnt = app_overlay_get_osd_stream();
	
	struct tm tm_now = {0};
	size_t text_width = 0;
	
	ST_NSDK_VENC_CH venc_ch;
	ST_NSDK_SYSTEM_TIME system_time;
	char text_buf[128] = {""};

	// FIXME: dot fix the id number
	if(NETSDK_conf_venc_ch_get(101, &venc_ch) && NETSDK_conf_system_get_time(&system_time)){
		char* weekday_map_chs[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六",};
		char* weekday_map_eng[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT",};
		char* weekday_map_jalaali[] = {"Yekshanbeh", "Doshanbeh", "Seshanbeh", "Chaharshanbeh", "Panjshanbeh", "Jom'eh", "Shanbeh"};
		
		int year = 0, month = 0, mday = 0, hour = 0, min = 0, sec = 0;
		char date_buf[64] = {""}, time_buf[64] = {""};
		const char *meridiem = "";
		const char* weekday = "";

		// all the clock overlay is reference to these 2 canvas
		// erase the canvas firstly
		if(_canvas_clock_main){
			_canvas_clock_main->erase_rect(_canvas_clock_main, 0, 0, 0, 0);
		}
		if(_canvas_clock_sub){
			_canvas_clock_sub->erase_rect(_canvas_clock_sub, 0, 0, 0, 0);
		}

		// get system time
		localtime_r(&t, &tm_now);
		
		// the date
		year = tm_now.tm_year + 1900;
		month = tm_now.tm_mon + 1;
		mday = tm_now.tm_mday;
		hour = tm_now.tm_hour;
		min = tm_now.tm_min;
		sec = tm_now.tm_sec;

		if(_overlay_chs){
			switch(venc_ch.datetimeOverlay.dateFormat){
				default:
				case kNSDK_DATETIME_FMT_SLASH_YYYYMMDD:
				case kNSDK_DATETIME_FMT_DASH_YYYYMMDD:
					snprintf(date_buf, sizeof(date_buf), "%04d年%02d月%02d日", year, month, mday);
					break;

				case kNSDK_DATETIME_FMT_SLASH_MMDDYYYY:
				case kNSDK_DATETIME_FMT_DASH_MMDDYYYY:
					snprintf(date_buf, sizeof(date_buf), "%02d月%02d日%04d年", month, mday, year);
					break;

				case kNSDK_DATETIME_FMT_SLASH_DDMMYYYY:
				case kNSDK_DATETIME_FMT_DASH_DDMMYYYY:
					snprintf(date_buf, sizeof(date_buf), "%02d日%02d月%04d年", mday, month, year);
					break;
			}

			if(hour < 6){
				meridiem = "凌晨";
			}else if(hour < 12){
				meridiem = "上午";
			}else if(hour < 18){
				meridiem = "下午";
			}else if(hour < 20){
				meridiem = "傍晚";
			}else{
				meridiem = "晚上";
			}

			switch(venc_ch.datetimeOverlay.timeFormat){
				default:
				case 24:
					snprintf(time_buf, sizeof(time_buf), "%d:%02d:%02d", hour, min, sec);
					break;
				case 12:
					snprintf(time_buf, sizeof(time_buf), "%s%d:%02d:%02d", meridiem, hour < 12 ? hour : hour - 12, min, sec);
					break;
			}
			weekday = weekday_map_chs[tm_now.tm_wday];
		}else{
			if(kNSDK_CALENDARSTYLE_JALAALI == system_time.calendarStyle){
				solar_to_jalaali(&year, &month, &mday);
			}
			switch(venc_ch.datetimeOverlay.dateFormat){
				default:
				case kNSDK_DATETIME_FMT_SLASH_YYYYMMDD:
					snprintf(date_buf, sizeof(date_buf), "%04d/%02d/%02d", year, month, mday);
					break;
				case kNSDK_DATETIME_FMT_DASH_YYYYMMDD:
					snprintf(date_buf, sizeof(date_buf), "%04d-%02d-%02d", year, month, mday);
					break;
				case kNSDK_DATETIME_FMT_SLASH_MMDDYYYY:
					snprintf(date_buf, sizeof(date_buf), "%02d/%02d/%04d", month, mday, year);
					break;
				case kNSDK_DATETIME_FMT_DASH_MMDDYYYY:
					snprintf(date_buf, sizeof(date_buf), "%02d-%02d-%04d", month, mday, year);
					break;
				case kNSDK_DATETIME_FMT_SLASH_DDMMYYYY:
					snprintf(date_buf, sizeof(date_buf), "%02d/%02d/%04d", mday, month, year);
					break;
				case kNSDK_DATETIME_FMT_DASH_DDMMYYYY:
					snprintf(date_buf, sizeof(date_buf), "%02d-%02d-%04d", mday, month, year);
					break;
			}

			if(hour < 12){
				meridiem = "AM";
			}else{
				meridiem = "PM";
			}

			switch(venc_ch.datetimeOverlay.timeFormat){
				default:
				case 24:
					snprintf(time_buf, sizeof(time_buf), "%d:%02d:%02d", hour, min, sec);
					break;
				case 12:
					snprintf(time_buf, sizeof(time_buf), "%d:%02d:%02d%s", hour < 12 ? hour : hour - 12, min, sec, meridiem);
					break;
			}
			if(kNSDK_CALENDARSTYLE_JALAALI == system_time.calendarStyle){
				weekday = weekday_map_jalaali[tm_now.tm_wday];
			}else{
				weekday = weekday_map_eng[tm_now.tm_wday];
			}
		}
		if(!venc_ch.datetimeOverlay.displayWeek){
			weekday = "";
		}
		snprintf(text_buf, sizeof(text_buf), "%s %s %s", date_buf, time_buf, weekday);
	}
	
	for(i = 0; i < 1; ++i){
		if(NETSDK_conf_venc_ch_get((i + 1) * 100 + 1, &venc_ch)){
			for(ii = 0; ii < stream_cnt; ++ii){
				int venc_width = 0, venc_height = 0;
				int ratio = 0;
				app_overlay_resolution_get((i + 1) * 100 + 1 + ii, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
				if((venc_width <= 720 || venc_height < 576) && (ii > 0)){
					if(_canvas_clock_sub){
						_canvas_clock_sub->erase_rect(_canvas_clock_sub, 0, 0, 0, 0);
						text_width = OVERLAY_put_text(_canvas_clock_sub, 0, 0, app_overlay_font_sub, text_buf, 0, ratio);
					}
				}else{
					if(_canvas_clock_main){
						_canvas_clock_main->erase_rect(_canvas_clock_main, 0, 0, 0, 0);
						text_width = OVERLAY_put_text(_canvas_clock_main, 0, 0, app_overlay_font_main, text_buf, 0, ratio);
					}
				}
				sdk_enc->update_overlay(i, ii, kAPP_OVERLAY_CLOCK_NAME);
			}
		}
	}
	app_overlay_show(0, kAPP_OVERLAY_CLOCK_NAME, venc_ch.datetimeOverlay.o.enabled);
}

#include "ticker.h"
static void ticker_update_overlay_clock()
{
	//LP_TICKER_TIME tickIn = TICKER_time_in();
	app_overlay_update_clock(time(NULL));
	//APP_TRACE("Cost %dms", TICKER_time_out(tickIn));
}

int APP_OVERLAY_create_clock(int vin)
{
#ifdef	DEL_OSD
		return 0;
#endif

	int i = 0;
	int const stream_cnt = app_overlay_get_osd_stream();
	ST_NSDK_VENC_CH venc_ch;
	
	if(NETSDK_conf_venc_ch_get((vin + 1) * 100 + 1, &venc_ch)){ // FIXME: dont fix the id number
		float overlay_x = venc_ch.datetimeOverlay.o.regionX / 100.0;
		float overlay_y = venc_ch.datetimeOverlay.o.regionY / 100.0;

		for(i = 0; i < stream_cnt; ++i){
			if(NULL == sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_CLOCK_NAME)){
				LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas_use = NULL;
				int venc_width = 0, venc_height = 0;
				int ratio = 0;
				app_overlay_resolution_get((vin + 1) * 100 + 1 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
				if((venc_width <= 720 || venc_height < 576) && i > 0){
					if(!_canvas_clock_sub){
						_canvas_clock_sub = sdk_enc->create_overlay_canvas(app_overlay_font_sub * 18 * ratio, app_overlay_font_sub * ratio);
					}
					canvas_use = _canvas_clock_sub;
				}else{
					if(!_canvas_clock_main){
						_canvas_clock_main = sdk_enc->create_overlay_canvas(app_overlay_font_main * 18 * ratio, app_overlay_font_main * ratio);
					}
					canvas_use = _canvas_clock_main;
				}
					
				sdk_enc->create_overlay(vin, i, kAPP_OVERLAY_CLOCK_NAME, overlay_x, overlay_y, canvas_use);
				sdk_enc->update_overlay(vin, i, kAPP_OVERLAY_CLOCK_NAME);
			}
		}
	}

	app_overlay_show(vin, kAPP_OVERLAY_CLOCK_NAME, venc_ch.datetimeOverlay.o.enabled);
	TICKER_add_task(ticker_update_overlay_clock, 1, true);
	return 0;
	
}

void APP_OVERLAY_release_clock(int vin)
{
#ifdef	DEL_OSD
		return 0;
#endif

	int i = 0;
	int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini
	if(sdk_enc){
		TICKER_del_task(ticker_update_overlay_clock);
		usleep(150000); // wait for the last drawing
		for(i = 0; i < stream_cnt; ++i){
			if(NULL != sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_CLOCK_NAME)){
				sdk_enc->release_overlay(vin, i, kAPP_OVERLAY_CLOCK_NAME);
			}
		}

		if(_canvas_clock_sub){
			sdk_enc->release_overlay_canvas(_canvas_clock_sub);
			_canvas_clock_sub = NULL;
		}
		if(_canvas_clock_main){
			sdk_enc->release_overlay_canvas(_canvas_clock_main);
			_canvas_clock_main = NULL;
		}
	}
}

void APP_OVERLAY_reload_clock(int vin)
{
	APP_OVERLAY_release_clock(vin);
	APP_OVERLAY_create_clock(vin);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_id_sub = NULL;
static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_id_main = NULL;

static void app_overlay_update_id()
{
	int i = 0;
	int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini
	//ST_SYSCONF2_VENC_OVERLAY venc_overlay;
	ST_NSDK_VENC_CH venc_ch;
	
	//if(SYSCONF2_VENC_get_overlay_byname(0, 0, "id", &venc_overlay)){
	
	char id_text[64] = {""};
	if(strlen(g_esee_id) > 0){		
		snprintf(id_text, sizeof(id_text), "ID:%s", g_esee_id);
	}else{
		snprintf(id_text, sizeof(id_text), "");
	}

	//APP_TRACE("Update ESEE ID: %s", g_esee_id);
	if(NETSDK_conf_venc_ch_get(101, &venc_ch)){
		for(i = 0; i < stream_cnt; ++i){
			int venc_width = 0, venc_height = 0;
			int ratio = 0;
			app_overlay_resolution_get(101 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
			if((venc_width <= 720 || venc_height < 576) && (i > 0)){
				if(NULL != _canvas_id_sub){
					_canvas_id_sub->erase_rect(_canvas_id_sub, 0, 0, 0, 0);
					OVERLAY_put_text(_canvas_id_sub, 0, 0, app_overlay_font_sub, id_text, 0, ratio);
				}
			}else{
				if(NULL != _canvas_id_main){
					_canvas_id_main->erase_rect(_canvas_id_main, 0, 0, 0, 0);
					OVERLAY_put_text(_canvas_id_main, 0, 0, app_overlay_font_main, id_text, 0, ratio);
				}
			}
			sdk_enc->update_overlay(0, i, kAPP_OVERLAY_ID_NAME);
		}
	}
	
	app_overlay_show(0, kAPP_OVERLAY_ID_NAME, venc_ch.deviceIDOverlay.o.enabled);
}

#include "esee_client.h"

static void ticker_update_overlay_id()
{
#if 0
	ESEE_CLIENT_INFO_t ret_info;
	memset(&ret_info, 0, sizeof(ret_info));
	if(0 == 	ESEE_CLIENT_get_info(&ret_info)
		&& strlen(ret_info.id) > 0){
		strcpy(g_esee_id, ret_info.id);
		app_overlay_update_id();
	}
#else
	char sn_str[32] = {0};
	if(0 == strlen(g_esee_id)){
		if(0 == UC_SNumberGet(sn_str)) {
			if(strlen(sn_str)> 10){
				if(sn_str[strlen(sn_str) - 10] == '0'){
					memcpy(g_esee_id, &sn_str[strlen(sn_str) - 10+1], 9);
				}else{
					memcpy(g_esee_id, &sn_str[strlen(sn_str) - 10], 10);
				}
			}
		}
	}
	app_overlay_update_id();
#endif
}

int APP_OVERLAY_create_id(int vin)
{
#ifdef	DEL_OSD
		return 0;
#endif

	int i = 0;
	int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini
	//ST_SYSCONF2_VENC_OVERLAY venc_overlay;
	ST_NSDK_VENC_CH venc_ch;
	//LP_SYSCONF2_VENC_OVERLAY const para = SYSCONF2_VENC_get_overlay_byname(vin, 0, "id", &venc_overlay);

	if(NETSDK_conf_venc_ch_get((vin + 1) * 100 + 1, &venc_ch)){
		float overlay_x = venc_ch.deviceIDOverlay.o.regionX / 100.0;
		float overlay_y = venc_ch.deviceIDOverlay.o.regionY / 100.0;
		for(i = 0; i < stream_cnt; ++i){
			if(NULL == sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_ID_NAME)){
				LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas_use = NULL;
				int venc_width = 0, venc_height = 0;
				int ratio = 0;
				app_overlay_resolution_get((vin + 1) * 100 + 1 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);

				if((venc_width <= 720 || venc_height < 576) && (i > 0)){
					if(!_canvas_id_sub){
						_canvas_id_sub = sdk_enc->create_overlay_canvas(app_overlay_font_sub * 16 * ratio, app_overlay_font_sub * ratio);
					}
					canvas_use = _canvas_id_sub;
				}else{
					if(!_canvas_id_main){
						_canvas_id_main = sdk_enc->create_overlay_canvas(app_overlay_font_main * 16 * ratio, app_overlay_font_main * ratio);
					}
					canvas_use = _canvas_id_main;
				}
	
				sdk_enc->create_overlay(vin, i, kAPP_OVERLAY_ID_NAME, overlay_x, overlay_y, canvas_use);
				sdk_enc->update_overlay(vin, i, kAPP_OVERLAY_ID_NAME);
			}
		}
	}

	app_overlay_show(0, kAPP_OVERLAY_ID_NAME, venc_ch.deviceIDOverlay.o.enabled);

	TICKER_add_task(ticker_update_overlay_id, 5, true);
	return 0;
}


void APP_OVERLAY_release_id(int vin)
{
#ifdef	DEL_OSD
		return 0;
#endif

	int i = 0;
	int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini
	if(sdk_enc){
		TICKER_del_task(ticker_update_overlay_id);
		usleep(150000); // wait for the last drawing
		for(i = 0; i < stream_cnt; ++i){
			if(NULL != sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_ID_NAME)){
				sdk_enc->release_overlay(vin, i, kAPP_OVERLAY_ID_NAME);
			}
		}
		if(_canvas_id_sub){
			sdk_enc->release_overlay_canvas(_canvas_id_sub);
			_canvas_id_sub = NULL;
		}
		if(_canvas_id_main){
			sdk_enc->release_overlay_canvas(_canvas_id_main);
			_canvas_id_main = NULL;
		}
	}
}

void APP_OVERLAY_reload_id(int vin)
{
	APP_OVERLAY_release_id(vin);
	APP_OVERLAY_create_id(vin);
}


static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_pic = NULL;

int APP_OVERLAY_create_pic(int vin)
{
	//ST_SYSCONF2_VENC_OVERLAY pic_overlay;
	int i, stream_cnt = 1; // FIXME: should be got from ini
	ST_NSDK_VENC_CH venc_ch;
	float overlay_x, overlay_y;
	char cmd[256] = {0};
	// create title overlay
	if(!_canvas_pic){
		//_canvas_pic = sdk_enc->load_overlay_canvas("/root/nfs/git_ipc/gm_ipc/app_rebulid/bin/resource/error2.bmp");
		snprintf(cmd, sizeof(cmd), "%s/ja_tools/mismatchsoc.bmp", IPCAM_ENV_HOME_DIR);
		_canvas_pic = sdk_enc->load_overlay_canvas(cmd);
	}

	for(i = 0; i < stream_cnt; ++i){
		if(NULL == sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_PIC_NAME)){
			if(NETSDK_conf_venc_ch_get((vin + 1) * 100 + 1 + i, &venc_ch)){		
				int venc_width = 0, venc_height = 0;
				if(venc_ch.freeResolution){
					venc_width = venc_ch.resolutionWidth;
					venc_height = venc_ch.resolutionHeight;
				}else{
					venc_width = (venc_ch.resolution >> 16) & 0xffff;;
					venc_height = (venc_ch.resolutionHeight >> 0) & 0xffff;;
				}
				overlay_x = (float)((venc_width - 128)/2)/venc_width;
				overlay_y = (float)((venc_height - 128)/2)/venc_height;
				sdk_enc->create_overlay(vin, i, kAPP_OVERLAY_PIC_NAME, overlay_x, overlay_y, _canvas_pic);
				sdk_enc->update_overlay(vin, i, kAPP_OVERLAY_PIC_NAME);

			}
		}
	}

	app_overlay_show(vin, kAPP_OVERLAY_PIC_NAME, true);
	return 0;
		
}

int APP_OVERLAY_release_pic(int vin)
{
	if(sdk_enc){
		sdk_enc->release_overlay(vin, 0, kAPP_OVERLAY_PIC_NAME);
		sdk_enc->release_overlay_canvas(_canvas_pic);
		_canvas_pic = NULL;
	}
	return -0;
}

static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_caution_sub = NULL;
static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_caution_main = NULL;

static int app_overlay_set_caution(int vin, const char *title)
{
	if(NULL != title && strlen(title)>0){
		int i = 0;
		ST_NSDK_VENC_CH venc_ch;
		int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini

		if(NETSDK_conf_venc_ch_get((vin + 1) * 100 + 1 + i, &venc_ch)){
			for(i = 0; i < stream_cnt; ++i){
				int venc_width = 0, venc_height = 0;
				int ratio = 0;
				app_overlay_resolution_get((vin + 1) * 100 + 1 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
				if((venc_width <= 720 || venc_height < 576) && (i > 0)){
					if(NULL != _canvas_caution_sub){
						_canvas_caution_sub->erase_rect(_canvas_caution_sub, 0, 0, 0, 0);
						OVERLAY_put_text(_canvas_caution_sub, 0, 0, app_overlay_font_sub, title, 0, ratio);
					}
				}else{
					if(NULL != _canvas_caution_main){
						_canvas_caution_main->erase_rect(_canvas_caution_main, 0, 0, 0, 0);
						OVERLAY_put_text(_canvas_caution_main, 0, 0, app_overlay_font_main, title, 0, ratio);
					}
				}
				sdk_enc->update_overlay(vin, i, kAPP_OVERLAY_CAUTION_NAME);
			}
		}
	}
	return 0;
}


int APP_OVERLAY_create_caution(int vin)
{
	int i = 0;
	int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini
	ST_NSDK_VENC_CH venc_ch;
	int str_len = 0;
	float overlay_x;
	char caution_str[64];

	//use different language 
	if(_overlay_chs){
		strncpy(caution_str,"固件升级中，请勿断电!", sizeof(caution_str)); 
	}else{
		strncpy(caution_str,"Firmware upgrading!", sizeof(caution_str)); 
	}
	str_len = strlen(caution_str);

	if(NETSDK_conf_venc_ch_get((vin + 1) * 100 + 1 + i, &venc_ch)){
		for(i = 0; i < stream_cnt; ++i){
			if(NULL == sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_CAUTION_NAME)){
				LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas_use = NULL;
				int venc_width = 0, venc_height = 0;
				int ratio = 0;
				app_overlay_resolution_get((vin + 1) * 100 + 1 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
				if((venc_width <= 720 || venc_height < 576) && (i > 0)){
					if(!_canvas_caution_sub){
						_canvas_caution_sub = sdk_enc->create_overlay_canvas(app_overlay_font_sub * 16 * ratio, app_overlay_font_sub * ratio);
					}
					canvas_use = _canvas_caution_sub;
				}else{
					if(!_canvas_caution_main){
						_canvas_caution_main = sdk_enc->create_overlay_canvas(app_overlay_font_main * 16 * ratio, app_overlay_font_main * ratio);
					}
					canvas_use = _canvas_caution_main;
				}
				overlay_x = 0.48 - (str_len * 4.0 / venc_width);
				sdk_enc->create_overlay(vin, i, kAPP_OVERLAY_CAUTION_NAME, overlay_x, 0.28, canvas_use);
				sdk_enc->update_overlay(vin, i, kAPP_OVERLAY_CAUTION_NAME);
			}
		}

		app_overlay_set_caution(vin, caution_str);
		app_overlay_show(vin, kAPP_OVERLAY_CAUTION_NAME, true);

	}
	return 0;
		
}

void APP_OVERLAY_release_caution(int vin)
{
	int i = 0;
	int stream_cnt = app_overlay_get_osd_stream(); // FIXME: should be got from ini
	if(sdk_enc){
		for(i = 0; i < stream_cnt; ++i){
			if(NULL != sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_CAUTION_NAME)){
				sdk_enc->release_overlay(vin, i, kAPP_OVERLAY_CAUTION_NAME);
			}
		}
		if(_canvas_caution_sub){
			sdk_enc->release_overlay_canvas(_canvas_caution_sub);
			_canvas_caution_sub = NULL;
		}
		if(_canvas_caution_main){
			sdk_enc->release_overlay_canvas(_canvas_caution_main);
			_canvas_caution_main = NULL;
		}
	}
}


//#define TEXT_LENGTH 128
//#define TEXT_LENGTH 320
#define TEXT_LENGTH 512

//#define TABLE_LINE 4
#define TABLE_LINE  12 //16
#define TABLE_LINE_LENGTH   24   //32
static char table_array[TABLE_LINE][TABLE_LINE_LENGTH] = {}; 


static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_table_sub = NULL;
static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS _canvas_table_main = NULL;
static char table_text[TEXT_LENGTH] = {""};

static bool table_osd_update[TABLE_LINE];


static void app_overlay_update_table(int osd_line,int osd_cow) 
{
	int i = 0;
	int stream_cnt = NETSDK_venc_get_channels(); // FIXME: should be got from ini
	ST_NSDK_VENC_CH venc_ch;
		
	if(NETSDK_conf_venc_ch_get(101, &venc_ch)){
		for(i = 0; i < stream_cnt; ++i){
			int venc_width = 0, venc_height = 0;
			int ratio = 0;
			app_overlay_resolution_get(101 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
			if((venc_width <= 720 || venc_height < 576) && (i > 0)){
				if(NULL != _canvas_table_sub){
					_canvas_table_sub->erase_rect(_canvas_table_sub, osd_cow* 8 * ratio, osd_line* 16 * ratio,  TABLE_LINE_LENGTH * 8 * ratio, app_overlay_font_sub* ratio);
					OVERLAY_put_text2(_canvas_table_sub, osd_cow* 8 * ratio, osd_line* 16 * ratio, app_overlay_font_sub, table_array[osd_line], 0, ratio,TABLE_LINE_LENGTH);
				}
			}else{
				if(NULL != _canvas_table_main){
					_canvas_table_main->erase_rect(_canvas_table_main, osd_cow* 8 * ratio, osd_line* 16 * ratio,  TABLE_LINE_LENGTH * 8 * ratio, app_overlay_font_main* ratio);
					OVERLAY_put_text2(_canvas_table_main, osd_cow* 8 * ratio, osd_line* 16 * ratio, app_overlay_font_main, table_array[osd_line], 0, ratio,TABLE_LINE_LENGTH);
				}
			}
			sdk_enc->update_overlay(0, i, kAPP_OVERLAY_table_NAME);
		}
	}
	
	app_overlay_show(0, kAPP_OVERLAY_table_NAME, true);

}

static int  app_overlay_clear_table() 
{
	int i = 0;
	int stream_cnt = NETSDK_venc_get_channels(); 
	ST_NSDK_VENC_CH venc_ch;

	
	if(sdk_enc){
		if(NETSDK_conf_venc_ch_get(101, &venc_ch)){
			for(i = 0; i < stream_cnt; ++i){
				int venc_width = 0, venc_height = 0,ratio = 0;
				
				app_overlay_resolution_get(101 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
				if((venc_width <= 720 || venc_height < 576) && (i > 0)){
					if(NULL != _canvas_table_sub){
						_canvas_table_sub->erase_rect(_canvas_table_sub,0, 0, 0, 0);
					}
				}else{
					if(NULL != _canvas_table_main){
						_canvas_table_main->erase_rect(_canvas_table_main, 0, 0, 0, 0);
					}
				}
				sdk_enc->update_overlay(0, i, kAPP_OVERLAY_table_NAME);
			}
		}
		app_overlay_show(0, kAPP_OVERLAY_table_NAME, true);
		
		return 0;
	}
	return -1;

}



int APP_OVERLAY_create_table(int vin)
{	
	int i = 0;
	int stream_cnt = NETSDK_venc_get_channels(); 

	float regionX = 60.0;
	float regionY = 25.0;
	
	ST_NSDK_VENC_CH venc_ch;
	
	if(NETSDK_conf_venc_ch_get((vin + 1) * 100 + 1, &venc_ch)){ // FIXME: dont fix the id number
		float overlay_x = regionX / 100.0;
		float overlay_y = regionY / 100.0;

		for(i = 0; i < stream_cnt; ++i){			
			if(NULL == sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_table_NAME)){
				LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas_use = NULL;
				
				int venc_width = 0, venc_height = 0;
				int ratio = 0;
				app_overlay_resolution_get((vin + 1) * 100 + 1 + i, venc_ch.freeResolution, &venc_width, &venc_height, &ratio);
				if((venc_width <= 720 || venc_height < 576) && i > 0){
					if(!_canvas_table_sub){
						_canvas_table_sub = sdk_enc->create_overlay_canvas(app_overlay_font_sub * 12 * ratio, app_overlay_font_sub * ratio * 12);
					}
					_canvas_table_sub->erase_rect(_canvas_table_sub, 0, 0, 0, 0);
					canvas_use = _canvas_table_sub;
				}else{
					if(!_canvas_table_main){
						_canvas_table_main = sdk_enc->create_overlay_canvas(app_overlay_font_main * 12 * ratio, app_overlay_font_main * ratio * 12);
					}
					_canvas_table_main->erase_rect(_canvas_table_main, 0, 0, 0, 0);
					canvas_use = _canvas_table_main;
				}
					
				sdk_enc->create_overlay(vin, i, kAPP_OVERLAY_table_NAME, overlay_x, overlay_y, canvas_use);
				sdk_enc->update_overlay(vin, i, kAPP_OVERLAY_table_NAME);
			}
		}
		
	}

	app_overlay_update_table(0,0);

    _overlay_table_create_flag = true;

}


int APP_OVERLAY_release_table(int vin)
{
	int i = 0;
	int stream_cnt = NETSDK_venc_get_channels(); // FIXME: should be got from ini

    if(false == _overlay_table_create_flag)
    {
        return -1;
    }
    _overlay_table_create_flag = false;

	_update_overlay_table_flag = false;
	_overlay_table_change_flag = false;
	if(sdk_enc){
		for(i = 0; i < stream_cnt; ++i){
			if(NULL != sdk_enc->get_overlay_canvas(vin, i, kAPP_OVERLAY_table_NAME)){
				sdk_enc->release_overlay(vin, i, kAPP_OVERLAY_table_NAME);
			}
		}
		sdk_enc->release_overlay_canvas(_canvas_table_sub);
		_canvas_table_sub = NULL;
		sdk_enc->release_overlay_canvas(_canvas_table_main);
		_canvas_table_main = NULL;
	}

    return 0;

}


static void ticker_update_overlay_table(void *arg)
{
	time_t interleave = (time_t *)arg;
	
	prctl(PR_SET_NAME, "update_overlay_table");
	int i = 0;
	
	while(_update_overlay_table_flag){
		if(_overlay_table_change_flag){	
			
			for(i = 0; i < TABLE_LINE ;i++){
				if(true == table_osd_update[i]){
					app_overlay_update_table(i, 0);
					table_osd_update[i] = false;
				}
			}
			
			_overlay_table_change_flag = false;
		}	
		usleep(interleave*1000);
	}
	pthread_exit(NULL);
}

static int TICKER_table_task(time_t interl, bool immediate)
{
	pthread_t table_thread_id;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  
	
	_update_overlay_table_flag = true;
	_overlay_table_change_flag = false;
	
	if(immediate)
	{
		app_overlay_update_table(0,0);
	}
	_canvas_table_sub->erase_rect(_canvas_table_sub, 0, 0, 0, 0);
	_canvas_table_main->erase_rect(_canvas_table_main, 0, 0, 0, 0);
	
	
	int err = pthread_create(&table_thread_id, &attr, ticker_update_overlay_table, (void *)interl);
	
	if (err != 0) {
		printf("create update_overlay_table  thread error: %s \n", strerror(err));
	}
	pthread_attr_destroy(&attr);
	return 0;
}

/**
*添加行信息\n
*逐行添加调用\n
*/
int  APP_OVERLAY_table_info_add(int  line, int cow,char* font_info)
{
	//memset(table_array[line - 1],0,TABLE_LINE_LENGTH);
	//memcpy(table_array[line - 1],font_info,TABLE_LINE_LENGTH);
	//snprintf(table_array[line - 1],TABLE_LINE_LENGTH,"%s",font_info);	
	//snprintf(table_array[line - 1] + cow - 1 ,TABLE_LINE_LENGTH - cow + 1 ,"%s",font_info);	

	//贝斯得需求要求协议行列都从 0 开始计算
	if((cow + strlen(font_info)) > TABLE_LINE_LENGTH){
		memcpy(table_array[line] +  cow, font_info, TABLE_LINE_LENGTH - cow);
	}else{
		memcpy(table_array[line] +  cow, font_info, strlen(font_info));
	}

	table_osd_update[line] = true;

}


/**
*添加行信息后更新桌面信息显示\n
*/
int  APP_OVERLAY_table_info_update()
{
	int i = 0;	
//	memset(table_text,0,TEXT_LENGTH);
	
	for(i = 0; i < TABLE_LINE; i++){
		memcpy((table_text + i *TABLE_LINE_LENGTH), table_array[i], TABLE_LINE_LENGTH);
	}
	int end_cnt = 0;	
	int end_cnt2 = 0;
	
	for(i = 0; i < TEXT_LENGTH; i++){
		if(*(table_text + i) == '\0'){
			end_cnt ++;
		}
	}
	
	_overlay_table_change_flag = true;

}


/**
*清空桌面信息显示\n
*/
int  APP_OVERLAY_table_info_clear()
{
	memset(table_text,0,sizeof(table_text));
	memset(table_array,0,sizeof(table_array));
	memset(table_osd_update,0,sizeof(table_osd_update));
	
	app_overlay_clear_table();

	return 0;

}


int APP_OVERLAY_init()
{
	// load the font
	char file_path_asc[128] = {""}, file_path_hzk[128] = {""};


	
	OVERLAY_init();
/*
	if((!strcmp(SOC_MODEL, "HI3518A"))||(!strcmp(SOC_MODEL, "HI3518C"))||(!strcmp(SOC_MODEL, "HI3516C"))){
		APP_TRACE("1");
		app_overlay_font_main = kOVERLAY_FONT_SIZE_32;
		app_overlay_font_sub = kOVERLAY_FONT_SIZE_16;
		snprintf(file_path_asc, sizeof(file_path_asc), "%s/%s%d", getenv("FONTDIR"), "asc", 16);
		snprintf(file_path_hzk, sizeof(file_path_hzk), "%s/%s%d", getenv("FONTDIR"), "hzk", 16);
	    OVERLAY_load_font(kOVERLAY_FONT_SIZE_16, file_path_asc, file_path_hzk);
	    snprintf(file_path_asc, sizeof(file_path_asc), "%s/%s%d", getenv("FONTDIR"), "asc", 32);
		snprintf(file_path_hzk, sizeof(file_path_hzk), "%s/%s%d", getenv("FONTDIR"), "hzk", 32);
	    OVERLAY_load_font(kOVERLAY_FONT_SIZE_32, file_path_asc, file_path_hzk);
	}else if(!strcmp(SOC_MODEL, "HI3518E")){
		APP_TRACE("2");
*/
		app_overlay_font_main = kOVERLAY_FONT_SIZE_16;
		app_overlay_font_sub = kOVERLAY_FONT_SIZE_16;
		snprintf(file_path_asc, sizeof(file_path_asc), "%s/%s%d", getenv("FONTDIR"), "asc", 16);
		snprintf(file_path_hzk, sizeof(file_path_hzk), "%s/%s%d", getenv("FONTDIR"), "hzk", 16);
	    OVERLAY_load_font(kOVERLAY_FONT_SIZE_16, file_path_asc, file_path_hzk);
/*
	}else{
		app_overlay_font_main = kOVERLAY_FONT_SIZE_32;
		app_overlay_font_sub = kOVERLAY_FONT_SIZE_16;
		snprintf(file_path_asc, sizeof(file_path_asc), "%s/%s%d", getenv("FONTDIR"), "asc", 16);
		snprintf(file_path_hzk, sizeof(file_path_hzk), "%s/%s%d", getenv("FONTDIR"), "hzk", 16);
		OVERLAY_load_font(kOVERLAY_FONT_SIZE_16, file_path_asc, file_path_hzk);
		snprintf(file_path_asc, sizeof(file_path_asc), "%s/%s%d", getenv("FONTDIR"), "asc", 32);
		snprintf(file_path_hzk, sizeof(file_path_hzk), "%s/%s%d", getenv("FONTDIR"), "hzk", 32);
		OVERLAY_load_font(kOVERLAY_FONT_SIZE_32, file_path_asc, file_path_hzk);
	}
*/
	// create title
	APP_OVERLAY_create_title(0);
	// create clock
	APP_OVERLAY_create_clock(0);
	// create id
	APP_OVERLAY_create_id(0);
	// create picture
	//APP_OVERLAY_create_pic(0);
	// create caution
	//APP_OVERLAY_create_caution(0);
	ST_NSDK_PTZ_CFG stPtz;
	memset(&stPtz, 0, sizeof(ST_NSDK_PTZ_CFG));
	NETSDK_conf_ptz_ch_get(&stPtz);

	if(0 == strcmp(stPtz.stPtzExternalConfig.strptzCustomTpye,"BEISIDE")){//璐濇柉寰桹SD妗岄潰鍔熻兘
		memset(table_array,0,sizeof(table_array));
#if defined(BSD_OSD)
		APP_OVERLAY_create_table(0);

		//TICKER_add_task(ticker_update_overlay_table, 1, FALSE);
		TICKER_table_task(100, FALSE);//set overlay table update 0ms
#endif
	}

	return 0;
}

void APP_OVERLAY_destroy()
{
	// release all the overlay
	//app_overaly_release_pic(0);
	APP_OVERLAY_release_clock(0);
	APP_OVERLAY_release_id(0);
	APP_OVERLAY_release_title(0);
	
#if defined(BSD_OSD)
		APP_OVERLAY_release_table(0);
#endif

    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);

}


