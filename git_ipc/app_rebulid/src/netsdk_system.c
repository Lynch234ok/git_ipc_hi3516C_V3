
#include <sys/time.h>
#include <bsp/rtc.h>
#include "netsdk.h"
#include "netsdk_util.h"
#include "netsdk_private.h"
#include "generic.h"
#include "sdk/sdk_api.h"
#include "app_overlay.h"
#include "ticker.h"
#include "app_debug.h"
#include "schedule_parse.h"
#include "app_gsensor.h"
#include "app_bluetooth.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// NetSDK general opertaions
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
/*
Afghanistan Standard Time			(GMT+04:30) Kabul
Alaskan Standard Time				(GMT-09:00) Alaska
Arab Standard Time				(GMT+03:00) Kuwait, Riyadh
Arabian Standard Time				(GMT+04:00) Abu Dhabi, Muscat
Arabic Standard Time				(GMT+03:00) Baghdad
Atlantic Standard Time				(GMT-04:00) Atlantic Time (Canada)
AUS Central Standard Time			(GMT+09:30) Darwin
AUS Eastern Standard Time			(GMT+10:00) Canberra, Melbourne, Sydney
Azerbaijan Standard Time			(GMT +04:00) Baku
Azores Standard Time				(GMT-01:00) Azores
Canada Central Standard Time		(GMT-06:00) Saskatchewan
Cape Verde Standard Time			(GMT-01:00) Cape Verde Islands
Caucasus Standard Time			(GMT+04:00) Yerevan
Cen. Australia Standard Time		(GMT+09:30) Adelaide
Central America Standard Time		(GMT-06:00) Central America
Central Asia Standard Time			(GMT+06:00) Astana, Dhaka
Central Brazilian Standard Time		(GMT -04:00) Manaus
Central Europe Standard Time		(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague
Central European Standard Time		(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb
Central Pacific Standard Time		(GMT+11:00) Magadan, Solomon Islands, New Caledonia
Central Standard Time				(GMT-06:00) Central Time (US and Canada)
Central Standard Time (Mexico)		(GMT-06:00) Guadalajara, Mexico City, Monterrey
China Standard Time				(GMT+08:00) Beijing, Chongqing, Hong Kong SAR, Urumqi
Dateline Standard Time				(GMT-12:00) International Date Line West
E. Africa Standard Time				(GMT+03:00) Nairobi
E. Australia Standard Time			(GMT+10:00) Brisbane
E. Europe Standard Time			(GMT+02:00) Minsk
E. South America Standard Time		(GMT-03:00) Brasilia
Eastern Standard Time				(GMT-05:00) Eastern Time (US and Canada)
Egypt Standard Time				(GMT+02:00) Cairo
Ekaterinburg Standard Time			(GMT+05:00) Ekaterinburg
Fiji Standard Time					(GMT+12:00) Fiji Islands, Kamchatka, Marshall Islands
FLE Standard Time					(GMT+02:00) Helsinki, Kiev, Riga, Sofia, Tallinn, Vilnius
Georgian Standard Time			(GMT +04:00) Tblisi
GMT Standard Time				(GMT) Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London
Greenland Standard Time			(GMT-03:00) Greenland
Greenwich Standard Time			(GMT) Casablanca, Monrovia
GTB Standard Time				(GMT+02:00) Athens, Bucharest, Istanbul
Hawaiian Standard Time				(GMT-10:00) Hawaii
India Standard Time				(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi
Iran Standard Time				(GMT+03:30) Tehran
Israel Standard Time				(GMT+02:00) Jerusalem
Korea Standard Time				(GMT+09:00) Seoul
Mid-Atlantic Standard Time			(GMT-02:00) Mid-Atlantic
Mountain Standard Time				(GMT-07:00) Mountain Time (US and Canada)
Mountain Standard Time (Mexico)		(GMT-07:00) Chihuahua, La Paz, Mazatlan
Myanmar Standard Time			(GMT+06:30) Yangon (Rangoon)
N. Central Asia Standard Time		(GMT+06:00) Almaty, Novosibirsk
Namibia Standard Time				(GMT +02:00) Windhoek
Nepal Standard Time				(GMT+05:45) Kathmandu
New Zealand Standard Time			(GMT+12:00) Auckland, Wellington
Newfoundland Standard Time		(GMT-03:30) Newfoundland and Labrador
North Asia East Standard Time		(GMT+08:00) Irkutsk, Ulaanbaatar
North Asia Standard Time			(GMT+07:00) Krasnoyarsk
Pacific SA Standard Time			(GMT-04:00) Santiago
Pacific Standard Time				(GMT-08:00) Pacific Time (US and Canada); Tijuana
Romance Standard Time			(GMT+01:00) Brussels, Copenhagen, Madrid, Paris
Russian Standard Time				(GMT+03:00) Moscow, St. Petersburg, Volgograd
SA Eastern Standard Time			(GMT-03:00) Buenos Aires, Georgetown
SA Pacific Standard Time			(GMT-05:00) Bogota, Lima, Quito
SA Western Standard Time			(GMT-04:00) Caracas, La Paz
Samoa Standard Time				(GMT-11:00) Midway Island, Samoa
SE Asia Standard Time				(GMT+07:00) Bangkok, Hanoi, Jakarta
Singapore Standard Time			(GMT+08:00) Kuala Lumpur, Singapore
South Africa Standard Time			(GMT+02:00) Harare, Pretoria
Sri Lanka Standard Time			(GMT+06:00) Sri Jayawardenepura
Taipei Standard Time				(GMT+08:00) Taipei
Tasmania Standard Time			(GMT+10:00) Hobart
Tokyo Standard Time				(GMT+09:00) Osaka, Sapporo, Tokyo
Tonga Standard Time				(GMT+13:00) Nuku'alofa
US Eastern Standard Time			(GMT-05:00) Indiana (East)
US Mountain Standard Time			(GMT-07:00) Arizona
Vladivostok Standard Time			(GMT+10:00) Vladivostok
W. Australia Standard Time			(GMT+08:00) Perth
W. Central Africa Standard Time		(GMT+01:00) West Central Africa
W. Europe Standard Time			(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna
West Asia Standard Time			(GMT+05:00) Islamabad, Karachi, Tashkent
West Pacific Standard Time			(GMT+10:00) Guam, Port Moresby
Yakutsk Standard Time				(GMT+09:00) Yakutsk

(GMT-12:00) International Date Line West
(GMT-11:00) Coordinated Universal Time-11
(GMT-11:00) Samoa
(GMT-10:00) Hawaii
(GMT-09:00) Alaska
(GMT-08:00) Baja California
(GMT-08:00) Pacific Time (US & Canada)
(GMT-07:00) Arizona
(GMT-07:00) Chihuahua, La Paz, Mazatlan - New
(GMT-07:00) Chihuahua, La Paz, Mazatlan - Old
(GMT-07:00) Mountain Time (US & Canada)
(GMT-06:00) Central America
(GMT-06:00) Central Time (US & Canada)
(GMT-06:00) Guadalajara, Mexico City, Monterrey - New
(GMT-06:00) Guadalajara, Mexico City, Monterrey - Old
(GMT-06:00) Saskatchewan
(GMT-05:00) Bogota, Lima, Quito
(GMT-05:00) Eastern Time (US & Canada)
(GMT-05:00) Indiana (East)
(GMT-04:30) Caracas
(GMT-04:00) Asuncion
(GMT-04:00) Atlantic Time (Canada)
(GMT-04:00) Cuiaba
(GMT-04:00) Georgetown, La Paz, Manaus, San Juan
(GMT-04:00) Santiago
(GMT-03:30) Newfoundland
(GMT-03:00) Brasilia
(GMT-03:00) Buenos Aires
(GMT-03:00) Cayenne, Fortaleza
(GMT-03:00) Greenland
(GMT-03:00) Montevideo
(GMT-02:00) Coordinated Universal Time-02
(GMT-02:00) Mid-Atlantic
(GMT-01:00) Azores
(GMT-01:00) Cape Verde Is.
(GMT) Casablanca
(GMT) Coordinated Universal Time
(GMT) Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London
(GMT) Monrovia, Reykjavik
(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna
(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague
(GMT+01:00) Brussels, Copenhagen, Madrid, Paris
(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb
(GMT+01:00) West Central Africa
(GMT+02:00) Amman
(GMT+02:00) Athens, Bucharest, Istanbul
(GMT+02:00) Beirut
(GMT+02:00) Cairo
(GMT+02:00) Damascus
(GMT+02:00) Harare, Pretoria
(GMT+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius
(GMT+02:00) Jerusalem
(GMT+02:00) Minsk
(GMT+02:00) Windhoek
(GMT+03:00) Baghdad
(GMT+03:00) Kuwait, Riyadh
(GMT+03:00) Moscow, St. Petersburg, Volgograd
(GMT+03:00) Nairobi
(GMT+03:30) Tehran
(GMT+04:00) Abu Dhabi, Muscat
(GMT+04:00) Baku
(GMT+04:00) Caucasus Standard Time
(GMT+04:00) Port Louis
(GMT+04:00) Tbilisi
(GMT+04:00) Yerevan
(GMT+04:30) Kabul
(GMT+05:00) Ekaterinburg
(GMT+05:00) Islamabad, Karachi
(GMT+05:00) Tashkent
(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi
(GMT+05:30) Sri Jayawardenepura
(GMT+05:45) Kathmandu
(GMT+06:00) Astana
(GMT+06:00) Dhaka
(GMT+06:00) Novosibirsk
(GMT+06:30) Yangon (Rangoon)
(GMT+07:00) Bangkok, Hanoi, Jakarta
(GMT+07:00) Krasnoyarsk
(GMT+08:00) Beijing, Chongqing, Hong Kong, Urumqi
(GMT+08:00) Irkutsk
(GMT+08:00) Kuala Lumpur, Singapore
(GMT+08:00) Perth
(GMT+08:00) Taipei
(GMT+08:00) Ulaanbaatar
(GMT+09:00) Osaka, Sapporo, Tokyo
(GMT+09:00) Seoul
(GMT+09:00) Yakutsk
(GMT+09:30) Adelaide
(GMT+09:30) Darwin
(GMT+10:00) Brisbane
(GMT+10:00) Canberra, Melbourne, Sydney
(GMT+10:00) Guam, Port Moresby
(GMT+10:00) Hobart
(GMT+10:00) Vladivostok
(GMT+11:00) Magadan, Solomon Is., New Caledonia
(GMT+12:00) Auckland, Wellington
(GMT+12:00) Coordinated Universal Time+12
(GMT+12:00) Fiji
(GMT+12:00) Petropavlovsk-Kamchatsky - Old
(GMT+13:00) Nuku'alofa

*/


/////////////////////////////////////////////////////////////////////////////////////////////////
// NetSDK general opertaions
/////////////////////////////////////////////////////////////////////////////////////////////////

const ST_NSDK_MAP_STR_DEC promptSoundType_map[] = {
	{"chinese", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_CHINESE},
	{"english", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_ENGLISH},
	{"german", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_GERMAN},
	{"korean", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_KOREAN},
	{"portuguese", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_PORTUGUESE},
	{"russian", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_RUSSIAN},
	{"spanish", kNSDK_SYSTEM_PROMPT_SOUND_TYPE_SPANISH}
};

static const ST_NSDK_MAP_STR_DEC pirTrigger_map[] = {
    {"fallingEdge", kNSDK_PIR_MANAGER_TRIGGER_FALLING_EDGE},
    {"risingEdge", kNSDK_PIR_MANAGER_TRIGGER_RISING_EDGE},
};

static inline int SYSTEM_ENTER_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	return pthread_rwlock_wrlock(&netsdk->system_sync);
}

static int SYSTEM_LEAVE_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	return pthread_rwlock_unlock(&netsdk->system_sync);
}


void NETSDK_conf_system_save()
{
	if(0 == SYSTEM_ENTER_CRITICAL()){
		APP_TRACE("NetSDK System Conf Save!!");
		NETSDK_conf_save(netsdk->jsonSystem, "system");
		SYSTEM_LEAVE_CRITICAL();
	}
}

void NETSDK_conf_system_save2()
{
	if(netsdk->system_conf_save){
		netsdk->system_conf_save(eNSDK_CONF_SAVE_JUST_SAVE, 2);
	}else{
		NETSDK_conf_system_save();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/*
/NetSDK/System/time/localTime
/NetSDK/System/time/timeZone
*/

static void system_remove_time_properties(LP_JSON_OBJECT time)
{
	int i, n;
	
	LP_JSON_OBJECT ntp = NETSDK_json_get_child(time, "ntp");
	NETSDK_json_remove_properties(time);
	if(NULL != ntp){
		NETSDK_json_remove_properties(ntp);
	}
	
	LP_JSON_OBJECT dstJSON = NETSDK_json_get_child(time, "DaylightSavingTime");
	if(dstJSON != NULL){
		NETSDK_json_remove_properties(dstJSON);
		
		LP_JSON_OBJECT _weekJSON, weekJSON = NETSDK_json_get_child(dstJSON, "Week");
		if(weekJSON != NULL){
			n = json_object_array_length(weekJSON);
			for(i = 0; i < n; i++){
				_weekJSON = json_object_array_get_idx(weekJSON, i);
				if(_weekJSON != NULL){
					NETSDK_json_remove_properties(_weekJSON);
				}
			}
		}
	}
}


static int system_time_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT timeRefJSON, LP_JSON_OBJECT timeDupJSON, LP_JSON_OBJECT formJSON,
	char *content, int contentMax)
{
	int nsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;
	char text[128] = {""};
	int gmt = GMT_GET();
	time_t utc = 0;
	struct tm local;
	struct timeval tv;
	fNSDK_SYSTEM_DST_CHG systemDSTChanged = NULL;
	// get local time


	//APP_TRACE("subURI = %s", subURI);
	if(HTTP_IS_GET(context)){
		RTC_sync_to_system();
		time(&utc);
		localtime_r(&utc, &local);

		if(!NSDK_PROPERTIES(subURI)){
			system_remove_time_properties(timeDupJSON);
		}
		if(NULL != timeDupJSON){
			if(NSDK_SUBURI_MATCH(prefix, subURI, "/LOCALTIME")){
				LP_JSON_OBJECT localTimeJSON = NULL;
				ISO8601(&local, true, text, sizeof(text));
				localTimeJSON = json_object_new_string(text);
				snprintf(content, contentMax, "%s", json_object_to_json_string(localTimeJSON));
				json_object_put(localTimeJSON);
				localTimeJSON = NULL;
				// ready
				nsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/TIMEZONE")){
				LP_JSON_OBJECT timeZoneJSON = NETSDK_json_get_child(timeDupJSON, "timeZone");
				snprintf(content, contentMax, "%s", json_object_to_json_string(timeZoneJSON));
				nsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/NTP")){
				LP_JSON_OBJECT ntpJSON = NETSDK_json_get_child(timeDupJSON, "ntp");
				snprintf(content, contentMax, "%s", json_object_to_json_string(ntpJSON));
				APP_TRACE("%s", content);
				nsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/CALENDARSTYLE")){
				LP_JSON_OBJECT calendarStyleJSON = NETSDK_json_get_child(timeDupJSON, "calendarStyle");
				snprintf(content, contentMax, "%s", json_object_to_json_string(calendarStyleJSON));
				APP_TRACE("%s", content);
				nsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/RTC")){
				rtc_time_t rtc_time;
				RTC_gettime(&rtc_time);
				local.tm_year = rtc_time.year - 1900;
				local.tm_mon = rtc_time.month - 1;
				local.tm_mday = rtc_time.date;
				local.tm_hour = rtc_time.hour;
				local.tm_min = rtc_time.minute;
				local.tm_sec = rtc_time.second;

				time_t timet = timegm(&local);
				snprintf(text, sizeof(text), "%d", timet);
				snprintf(content, contentMax, "%s", text);
				// ready
				nsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/DAYLIGHTSAVINGTIME")){
				//Daylight Saving Time
				LP_JSON_OBJECT dstJSON = NETSDK_json_get_child(timeDupJSON, "DaylightSavingTime");
				snprintf(content, contentMax, "%s", json_object_to_json_string(dstJSON));
				//APP_TRACE("%s", content);
				nsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}else{
				ISO8601(&local, true, text, sizeof(text));
				NETSDK_json_set_string(timeDupJSON, "localTime", text);
				snprintf(content, contentMax, "%s", json_object_to_json_string(timeDupJSON));
				nsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}
		}
	}else if(HTTP_IS_PUT(context)){	
		time(&utc);
		localtime_r(&utc, &local);
		if(NULL != timeRefJSON && NULL != formJSON){
			if(0 == SYSTEM_ENTER_CRITICAL()){
				if(NSDK_SUBURI_MATCH(prefix, subURI, "/LOCALTIME")){
					const char *localTimeText = strdupa(json_object_get_string(formJSON));
					const char *gmtText = NULL;

					//APP_TRACE("subURI = %s", subURI);
					
					if(strptime(localTimeText, "%FT%T", &local)){
						strftime(text, sizeof(text), "%FT%T", &local);
						gmtText = localTimeText + strlen(text);
						// covert to GMT format
						snprintf(text, sizeof(text), "GMT%s", gmtText);
						//APP_TRACE("GMT: %s", text);
						// parse time zone
						gmt = GMT_PARSE(text, gmt);
						if(0 == NETSDK_json_set_string2(timeRefJSON, "timeZone", GMT_STRING(gmt, text, sizeof(text)))){
							// set time zone
							GMT_SET(gmt);
							// set time
							utc = mktime(&local);
							tv.tv_sec = utc;
							tv.tv_usec = 0;
							settimeofday(&tv, NULL);
							RTC_settime(utc);
							// ok
							nsdkRet = kNSDK_INS_RET_OK;
						}
					}
				}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/TIMEZONE")){
					// parse the GMT

					//APP_TRACE("subURI = %s", subURI);
					gmt = GMT_PARSE(strdupa(json_object_get_string(formJSON)), gmt);
					GMT_STRING(gmt, text, sizeof(text));
					// check GMT changed
					NETSDK_json_set_string2(timeRefJSON, "timeZone", text);
					if(GMT_GET() != gmt){
						GMT_SET(gmt);
					}
					nsdkRet = kNSDK_INS_RET_OK;
				}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/CALENDARSTYLE")){
					//APP_TRACE("subURI = %s", subURI);
					LP_JSON_OBJECT formcalendarStyleJSON = NETSDK_json_get_child(formJSON, "calendarStyle");

					if(NULL != formcalendarStyleJSON){
						NETSDK_json_copy_child(formJSON, timeRefJSON, "calendarStyle");
					}
					nsdkRet = kNSDK_INS_RET_OK;
				}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/NTP")){
					//APP_TRACE("subURI = %s", subURI);
					LP_JSON_OBJECT ntpJSON = NETSDK_json_get_child(timeRefJSON, "ntp");
					if(NULL !=ntpJSON){
						LP_JSON_OBJECT ntpEnabledJSON = NETSDK_json_get_child(ntpJSON, "ntpEnabled");
						LP_JSON_OBJECT ntpServerDomainJSON = NETSDK_json_get_child(ntpJSON, "ntpServerDomain");
						
						LP_JSON_OBJECT formNtpEnabledJSON = NETSDK_json_get_child(formJSON, "ntpEnabled");
						LP_JSON_OBJECT formNtpServerDomainJSON = NETSDK_json_get_child(formJSON, "ntpServerDomain");
						
						if(NULL != ntpEnabledJSON && NULL != formNtpEnabledJSON){
							NETSDK_json_copy_child(formJSON, ntpJSON, "ntpEnabled");
						}
						if(NULL != ntpServerDomainJSON && NULL != formNtpServerDomainJSON){
							NETSDK_json_copy_child(formJSON, ntpJSON, "ntpServerDomain");
						}
					}
					nsdkRet = kNSDK_INS_RET_OK;
				}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/RTC")){
					const char *rtcTimeText = strdupa(json_object_get_string(formJSON));
					sscanf(rtcTimeText, "%d", &utc);
					APP_TRACE("set the utc is %d", utc);
					RTC_settime(utc);
					// ok
					nsdkRet = kNSDK_INS_RET_OK;

				}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/DAYLIGHTSAVINGTIME")){
					//Daylight Saving Time
					LP_JSON_OBJECT dstRefJSON = NETSDK_json_get_child(timeRefJSON, "DaylightSavingTime");
					
					if(dstRefJSON != NULL){
						LP_JSON_OBJECT weekJSON = NETSDK_json_get_child(formJSON, "Week");
						LP_JSON_OBJECT weekRefJSON = NETSDK_json_get_child(dstRefJSON, "Week");
						LP_JSON_OBJECT _weekJSON, _weekRefJSON;
						char type[8], typeRef[8];
						int i, j, n1, n2;
						
						if(weekJSON != NULL && weekRefJSON != NULL){
							n1 = json_object_array_length(weekJSON);
							n2 = json_object_array_length(weekRefJSON);
							for(i = 0; i < n2; i++){
								if((_weekRefJSON = json_object_array_get_idx(weekRefJSON, i)) != NULL){
									memset(type, 0, sizeof(type));
									NETSDK_json_get_string(_weekRefJSON, "Type", typeRef, sizeof(typeRef));
									for(j = 0; j < n1; j++){
										if((_weekJSON = json_object_array_get_idx(weekJSON, j)) != NULL){
											memset(type, 0, sizeof(type));
											NETSDK_json_get_string(_weekJSON, "Type", type, sizeof(type));
											if(strcmp(typeRef, type) == 0){
												NETSDK_json_copy_child(_weekJSON, _weekRefJSON, "Month");
												NETSDK_json_copy_child(_weekJSON, _weekRefJSON, "Week");
												NETSDK_json_copy_child(_weekJSON, _weekRefJSON, "Weekday");
												NETSDK_json_copy_child(_weekJSON, _weekRefJSON, "Hour");
												NETSDK_json_copy_child(_weekJSON, _weekRefJSON, "Minute");
												break;
											}
										}
									}
								}
							}
						}
						NETSDK_json_copy_child(formJSON, dstRefJSON, "Enabled");
                        NETSDK_json_copy_child(formJSON, dstRefJSON, "Country");
						NETSDK_json_copy_child(formJSON, dstRefJSON, "Offset");
						if(netsdk->systemDSTChanged != NULL){
							systemDSTChanged = netsdk->systemDSTChanged;
						}
					}
					nsdkRet = kNSDK_INS_RET_OK;
				}
				SYSTEM_LEAVE_CRITICAL();

				if(netsdk->systemChanged){
					APP_TRACE("netsdk->systemChanged %x",netsdk->systemChanged);
					ST_NSDK_SYSTEM_TIME systime;
					if(NETSDK_conf_system_get_time(&systime)){
						if(0 == netsdk->systemChanged(&systime)){
							nsdkRet = kNSDK_INS_RET_OK;
						}else{
							nsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
						}
					}
				}else{
					nsdkRet = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
				}
				if(systemDSTChanged != NULL){
					APP_TRACE("Set Daylight Saving Time!");
					ST_NSDK_SYSTEM_DST dstInfo = {0};
					NETSDK_conf_system_get_DST_info(&dstInfo);
					if(systemDSTChanged(&dstInfo) == 0){
						nsdkRet = kNSDK_INS_RET_OK;
					}
					else{
						nsdkRet = kNSDK_INS_RET_DEVICE_ERROR;
					}
				}
				if(kNSDK_INS_RET_OK == nsdkRet){
					NETSDK_conf_system_save2();
				}
			}
		}else{
			nsdkRet = kNSDK_INS_RET_INVALID_DOCUMENT;
		}
	}
	return nsdkRet;
}



	
static void system_remove_md_alarm_properties(LP_JSON_OBJECT md_alarm)
{

	if(NULL != md_alarm){
		NETSDK_json_remove_properties(md_alarm);
	}

}

static int system_md_alarm_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
		LP_JSON_OBJECT mdRefJSON, LP_JSON_OBJECT mdDupJSON, LP_JSON_OBJECT formJSON,
		char *content, int contentMax)
	{
		int nsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
		const char *prefix = NULL;
		
		if(HTTP_IS_GET(context)){
			// check properties necessary when GET method
			if(!NSDK_PROPERTIES(subURI)){
				system_remove_md_alarm_properties(mdDupJSON);
			}
			
			if(NULL != mdDupJSON ){		
				snprintf(content, contentMax, "%s", json_object_to_json_string(mdDupJSON));
				APP_TRACE("%s", content);
				nsdkRet = kNSDK_INS_RET_CONTENT_READY;
			}

		}else if(HTTP_IS_PUT(context)){	
			if(NULL != mdRefJSON && NULL != formJSON){ 
				if(NSDK_SUBURI_MATCH(prefix, subURI, "/MOTIONWARNINGTONE")){//MotionWarningTone					
					//PP_TRACE("subURI = %s", subURI);					
					LP_JSON_OBJECT mdAlarmJSON = NETSDK_json_get_child(mdRefJSON, "MotionWarningTone");
					LP_JSON_OBJECT formMAlarmJSON = NETSDK_json_get_child(formJSON, "MotionWarningTone");
					 
					if(NULL != mdRefJSON && NULL != formMAlarmJSON){	
						NETSDK_json_copy_child(formJSON, mdRefJSON, "MotionWarningTone");
					}	

				}				

					nsdkRet = kNSDK_INS_RET_OK;
			}

			if(kNSDK_INS_RET_OK == nsdkRet){
				NETSDK_conf_system_save2();
			}
		}
		return nsdkRet;
}
	

static int system_device_info_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT infoRefJSON, LP_JSON_OBJECT infoDupJSON, LP_JSON_OBJECT formJSON, char *content, int contentMax)
{
	int nsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;

	//APP_TRACE("%s -- %s", context->request_header->uri, subURI);

	if(HTTP_IS_GET(context)){
		ST_NSDK_SYSTEM_DEVICE_INFO info;
		LP_JSON_OBJECT contentJSON = infoDupJSON;

		if(!NSDK_PROPERTIES(subURI)){
			NETSDK_json_remove_properties(infoDupJSON);
		}
		// fill the device info
		if(netsdk->systemDeviceInfo && 0 == netsdk->systemDeviceInfo(&info)){
			NETSDK_json_set_string(infoDupJSON, "model", info.model);
			NETSDK_json_set_string(infoDupJSON, "serialNumber", info.serialNumber);
			NETSDK_json_set_string(infoDupJSON, "macAddress", info.macAddress);
			NETSDK_json_set_string(infoDupJSON, "firmwareVersion", info.firmwareVersion);
			NETSDK_json_set_string(infoDupJSON, "firmwareReleaseDate", info.firmwareReleaseDate);
			NETSDK_json_set_string(infoDupJSON, "hardwareVersion", info.hardwareVersion);
			NETSDK_json_set_string(infoDupJSON, "productCode", info.productCode);
		}

		if(prefix = "/DEVICENAME", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "deviceName");
		}else if(prefix = "/DEVICEADDRESS", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "deviceAddress");
		}else if(prefix = "/MODEL", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "model");
		}else if(prefix = "/SERIALNUMBER", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "serialNumber");
		}else if(prefix = "/MACADDRESS", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "macAddress");
		}else if(prefix = "/FIRMWAREVERSION", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "firmwareVersion");
		}else if(prefix = "/FIRMWARERELEASEDATE", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "firmwareReleaseDate");
		}else if(prefix = "/HARDWAREVERSION", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "hardwareVersion");
		}else if(prefix = "/PRODUCTCODE", 0 == strncmp(prefix, subURI, strlen(prefix))){
			contentJSON = NETSDK_json_get_child(infoDupJSON, "productCode");
		}
		
		snprintf(content, contentMax, "%s", json_object_to_json_string(contentJSON));
		nsdkRet = kNSDK_INS_RET_CONTENT_READY;
	}else if(HTTP_IS_PUT(context)){
		if(NULL != infoRefJSON && NULL != formJSON){
			if(0 == SYSTEM_ENTER_CRITICAL()){
				if(prefix = "/DEVICENAME", 0 == strncmp(prefix, subURI, strlen(prefix))){
					NETSDK_json_set_string2(infoRefJSON, "deviceName", json_object_get_string(formJSON));
					nsdkRet = kNSDK_INS_RET_OK;
				}else if(prefix = "/DEVICEADDRESS", 0 == strncmp(prefix, subURI, strlen(prefix))){
					NETSDK_json_set_int2(infoRefJSON, "deviceAddress", json_object_get_int(formJSON));
					nsdkRet = kNSDK_INS_RET_OK;
				}else{
					NETSDK_json_copy_child(formJSON, infoRefJSON, "deviceName");
					NETSDK_json_copy_child(formJSON, infoRefJSON, "deviceAddress");
					nsdkRet = kNSDK_INS_RET_OK;
				}
				SYSTEM_LEAVE_CRITICAL();
			}
		}else{
			nsdkRet = kNSDK_INS_RET_INVALID_DOCUMENT;
		}
		NETSDK_conf_system_save2();
	}
	return nsdkRet;
}

static int system_operations_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI, char *content, int contentMax)
{
	int nsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;

	//APP_TRACE("%s -- %s", context->request_header->uri, subURI);
	if(HTTP_IS_GET(context)){
		if(NSDK_SUBURI_MATCH(prefix, subURI, "/REMOTEUPGRADEINFO")){
			//get remote upgrade info
			APP_TRACE("SUBURI:%s", subURI);
			if(NSDK_SUBURI_MATCH(prefix, subURI, "/STATUS")){
				//get remote upgrade status
				if(netsdk->remoteUpgradeStatus){
					char status[64];
					int rate;
					netsdk->remoteUpgradeStatus(status, &rate);
					snprintf(content, contentMax, "%s--%d", status, rate);
					nsdkRet = kNSDK_INS_RET_CONTENT_READY;
				}
			}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/ERROR")){
				//get remote upgrade error string
				if(netsdk->remoteUpgradeError){
					netsdk->remoteUpgradeError(content);
					nsdkRet = kNSDK_INS_RET_CONTENT_READY;
				}
			}
		}
	}else if(HTTP_IS_PUT(context)){
		if(NSDK_SUBURI_MATCH(prefix, subURI, "/REBOOT")){
			//reboot
			if(netsdk->systemOperation){
				netsdk->systemOperation(eNSDK_SYSTEM_OPERATION_REBOOT);
				nsdkRet = kNSDK_INS_RET_OK;
			}
		}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/DEFAULT")){
			//default factory
			if(netsdk->systemOperation){
				netsdk->systemOperation(eNSDK_SYSTEM_OPERATION_DEFAULT);
				nsdkRet = kNSDK_INS_RET_OK;
			}
		}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/REMOTEUPGRADE")){
			//remote upgrade
			if(netsdk->systemOperation){
				netsdk->systemOperation(eNSDK_SYSTEM_OPERATION_REMOTE_UPGRADE);
				nsdkRet = kNSDK_INS_RET_OK;
			}
		}
		
	}
	return nsdkRet;
}


static void system_remove_properties(LP_JSON_OBJECT sys)
{
	LP_JSON_OBJECT time = NETSDK_json_get_child(sys, "time");
	LP_JSON_OBJECT deviceInfo = NETSDK_json_get_child(sys, "deviceInfo");
	if(NULL != time){
		NETSDK_json_remove_properties(time);
		LP_JSON_OBJECT ntp = NETSDK_json_get_child(time, "ntp");
		if(NULL != ntp){
			NETSDK_json_remove_properties(ntp);
		}
	}
	if(NULL != deviceInfo){
		NETSDK_json_remove_properties(deviceInfo);
	}
}

static int system_module_gsensor_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT formJSON, char *content, int contentMax)
{
    LP_JSON_OBJECT tmpJson = NULL;
    LP_JSON_OBJECT gsensorJson = NULL;
    int nsdkRet = kNSDK_INS_RET_DEVICE_NOT_SUPPORT;
    const char *prefix = NULL;
    int ret = kNSDK_INS_RET_DEVICE_NOT_SUPPORT;
    bool enable = false;

#ifdef GSENSOR
    if(HTTP_IS_GET(context)) {
        gsensorJson = json_object_new_object();

        if(!NSDK_PROPERTIES(subURI)) {
            ret = APP_GSENSOR_get_to_json(gsensorJson, false);
        }
        else {
            ret = APP_GSENSOR_get_to_json(gsensorJson, true);
        }
        if(emAPP_GSENSOR_ERR_CODE_OK == ret) {
            snprintf(content, contentMax, "%s", json_object_to_json_string(gsensorJson));
            nsdkRet = kNSDK_INS_RET_CONTENT_READY;
        }
        else if(emAPP_GSENSOR_ERR_CODE_NOT_SUPPORT == ret) {
            nsdkRet = kNSDK_INS_RET_DEVICE_NOT_SUPPORT;
        }
    }
    else if(HTTP_IS_PUT(context)) {
        if(NULL != formJSON) {
            if(!NSDK_PROPERTIES(subURI)) {
                if(NSDK_SUBURI_MATCH(prefix, subURI, "/CALIBRATION")) {
                    tmpJson = json_object_object_get(formJSON, "enabled");
                    if(NULL != tmpJson) {
                        if(json_type_boolean == json_object_get_type(tmpJson)) {
                            ret = APP_GSENSOR_calibration(json_object_get_boolean(tmpJson));
                        }
                    }
                }
                else {
                    ret = APP_GSENSOR_set_for_json(formJSON);
                }

                if(emAPP_GSENSOR_ERR_CODE_OK == ret) {
                    nsdkRet = kNSDK_INS_RET_OK;
                }
                else if(emAPP_GSENSOR_ERR_CODE_NOT_SUPPORT == ret) {
                    nsdkRet = kNSDK_INS_RET_DEVICE_NOT_SUPPORT;
                }
            }
        }
    }
#else
    return kNSDK_INS_RET_DEVICE_NOT_SUPPORT;
#endif

    return nsdkRet;

}

static int system_module_bluetooth_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
        LP_JSON_OBJECT formJSON, char *content, int contentMax)
{
    LP_JSON_OBJECT tmpJson = NULL;
    LP_JSON_OBJECT bluetoothJson = NULL;
    int nsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
    char *prefix = NULL;
    int ret = kNSDK_INS_RET_INVALID_OPERATION;
    char strStatus[16] = {0};

#ifdef BLUETOOTH
    if(HTTP_IS_GET(context)) {

        if(NSDK_SUBURI_MATCH(prefix, subURI, "/STATUS")) {
            memset(strStatus, 0, sizeof(strStatus));
            APP_BLUETOOTH_status(strStatus);
            tmpJson = json_object_new_string_len(strStatus, strlen(strStatus));
            snprintf(content, contentMax, "%s", json_object_to_json_string(tmpJson));
            nsdkRet = kNSDK_INS_RET_CONTENT_READY;
        }
        else {
            bluetoothJson = json_object_new_object();
            if(NULL != bluetoothJson) {
                if(!NSDK_PROPERTIES(subURI)) {
                    ret = APP_BLUETOOTH_get_to_json(bluetoothJson, false);
                }
                else {
                    ret = APP_BLUETOOTH_get_to_json(bluetoothJson, true);
                }

                if(emAPP_BLUETOOTH_ERR_CODE_OK == ret) {
                    snprintf(content, contentMax, "%s", json_object_to_json_string(bluetoothJson));
                    nsdkRet = kNSDK_INS_RET_CONTENT_READY;
                }
                else if(emAPP_BLUETOOTH_ERR_CODE_NOT_SUPPORT == ret) {
                    nsdkRet = kNSDK_INS_RET_DEVICE_NOT_SUPPORT;
                }
            }
        }
    }
#else
    return kNSDK_INS_RET_DEVICE_NOT_SUPPORT;
#endif

    if(NULL != bluetoothJson) {
        json_object_put(bluetoothJson);
    }

    return nsdkRet;

}

static int system_module_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	LP_JSON_OBJECT formJSON, char *content, int contentMax)
{
    int nsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
    const char *prefix = NULL;

    if(NSDK_SUBURI_MATCH(prefix, subURI, "/GSENSOR")){
        nsdkRet = system_module_gsensor_instance(context, subURI, formJSON, content, contentMax);
    }else if(NSDK_SUBURI_MATCH(prefix, subURI, "/BLUETOOTH")){
        nsdkRet = system_module_bluetooth_instance(context, subURI, formJSON, content, contentMax);
    }

    return nsdkRet;

}

int NETSDK_system_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI, char *content, int contentMax)
{
	int netsdkRet = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;

	// create json handle
	LP_JSON_OBJECT systemJSON = json_object_get(netsdk->jsonSystem);
	LP_JSON_OBJECT subRefJSON = NULL;
	LP_JSON_OBJECT subDupJSON = NULL;
	LP_JSON_OBJECT formJSON = NULL;
	bool sysTime = false, sysDeviecInfo = false, sysOperation = false, sysMdAlarm = false;
    bool sysModule = false;

	if(NSDK_SUBURI_MATCH(prefix, subURI, "/TIME")){
		sysTime = true;
		subRefJSON = NETSDK_json_get_child(systemJSON, "time");
	}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/DEVICEINFO")){
		sysDeviecInfo = true;
		subRefJSON = NETSDK_json_get_child(systemJSON, "deviceInfo");
	}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/OPERATION")){
		sysOperation = true;
    }else if(NSDK_SUBURI_MATCH(prefix, subURI, "/MODULE")) {
        sysModule = true;
	}else if(NSDK_SUBURI_MATCH(prefix, subURI, "/MDALARM")){
		sysMdAlarm = true;
		subRefJSON = NETSDK_json_get_child(systemJSON, "MdAlarm");
	}

	APP_TRACE("subURI = %s", subURI);

	// duplicated the reference JSON
	if(NULL != subRefJSON){
		if(HTTP_IS_GET(context)){
			subDupJSON = NETSDK_json_dup(subRefJSON);
		}
	}
	
	// pars form JSON for content
	if(NULL != context->request_content && 0 != context->request_content_len){
		formJSON = NETSDK_json_parse(context->request_content);
		APP_TRACE("Form JSON:\r\n%s", json_object_to_json_string(formJSON));
	}
	
	if(sysTime){
		netsdkRet = system_time_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, contentMax);
	}else if(sysDeviecInfo){
		netsdkRet = system_device_info_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, contentMax);
	}else if(sysOperation){
		netsdkRet = system_operations_instance(context, subURI, content, contentMax);
    }else if(sysModule) {
        netsdkRet = system_module_instance(context, subURI, formJSON, content, contentMax);
	}else if(sysMdAlarm){
		netsdkRet = system_md_alarm_instance(context, subURI, subRefJSON, subDupJSON, formJSON, content, contentMax);
	}

	// put JSON reference
	if(formJSON){
		json_object_put(formJSON);
		formJSON = NULL;
	}
	if(NULL != subDupJSON){
		json_object_put(subDupJSON);
		subDupJSON = NULL;
	}
	json_object_put(systemJSON);
	systemJSON = NULL;
	return netsdkRet;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////////////////////////////

static LP_NSDK_SYSTEM_TIME netsdk_conf_system_time(bool setFlag, LP_NSDK_SYSTEM_TIME sys_time)
{
	char text[128] = {""};
	char *str = NULL;

	const ST_NSDK_MAP_STR_DEC calendar_style_map[] = {
		{"general", kNSDK_CALENDARSTYLE_GENERAL},
		{"jalaali", kNSDK_CALENDARSTYLE_JALAALI},
	};

    if(NULL == sys_time)
    {
        return NULL;
    }

	if(0 == SYSTEM_ENTER_CRITICAL()){
        LP_JSON_OBJECT systemJSON = json_object_get(netsdk->jsonSystem);
    	if(NULL != systemJSON)
    	{
    		LP_JSON_OBJECT time = NETSDK_json_get_child(systemJSON, "time");
    		
    		if(setFlag){
    			char temp[64];
    			char sign = (sys_time->greenwichMeanTime >= 0) ? '+' : '-';
    			int offHour = abs(sys_time->greenwichMeanTime ) / 100;
    			int offMinutes = abs(sys_time->greenwichMeanTime ) % 100;

    			// time zone
    			if (offHour == 0 && offMinutes == 0)
    				snprintf(temp, sizeof(temp), "GMT");
    			else
    				snprintf(temp, sizeof(temp), "GMT%c%02d:%02d", sign, offHour, offMinutes);
    			NETSDK_json_set_string2(time, "timeZone", temp);
    			// calendar style
    			str = NETSDK_MAP_DEC2STR(calendar_style_map, sys_time->calendarStyle, "general");
    			NETSDK_json_set_string2(time, "calendarStyle", str);
    			// ntp
    			LP_JSON_OBJECT ntpJSON = NETSDK_json_get_child(time, "ntp");
    			NETSDK_json_set_boolean2(ntpJSON, "ntpEnabled", sys_time->ntpEnabled);
    			NETSDK_json_set_string2(ntpJSON, "ntpServerDomain", sys_time->ntpServerDomain);
    			NETSDK_json_set_string2(ntpJSON, "ntpServerBackupOne", sys_time->ntpServerBackupOne);
    			NETSDK_json_set_string2(ntpJSON, "ntpServerBackupTwo", sys_time->ntpServerBackupTwo);
    			// save
    				
    		}else{
    			char sign = '+';
    			int offHour = 0, offMinutes = 0;
    			// get time data here

    			// time zone
    			NETSDK_json_get_string(time, "timeZone", text, sizeof(text));

    			if(strlen(text) == 3 && 0 == strcasecmp("GMT", text)){
    				sys_time->greenwichMeanTime = 0;
    			}else{
    				sscanf(text, "GMT%c%d:%d", &sign, &offHour, &offMinutes);
    				sys_time->greenwichMeanTime = 100 * offHour + offMinutes;
    				if('-' == sign){
    					sys_time->greenwichMeanTime *= -1;
    				}
    			}
    			NETSDK_json_get_string(time, "calendarStyle", text, sizeof(text));
    			sys_time->calendarStyle = NETSDK_MAP_STR2DEC(calendar_style_map, text, kNSDK_CALENDARSTYLE_GENERAL);
    			LP_JSON_OBJECT ntpJSON = NETSDK_json_get_child(time, "ntp");
    			sys_time->ntpEnabled = NETSDK_json_get_boolean(ntpJSON, "ntpEnabled");
    			NETSDK_json_get_string(ntpJSON, "ntpServerDomain", sys_time->ntpServerDomain, sizeof(text));
    			NETSDK_json_get_string(ntpJSON, "ntpServerBackupOne", sys_time->ntpServerBackupOne, sizeof(text));
    			NETSDK_json_get_string(ntpJSON, "ntpServerBackupTwo", sys_time->ntpServerBackupTwo, sizeof(text));
    		}

    		if(setFlag){
    			NETSDK_conf_system_save2();
    		}
    		json_object_put(systemJSON);
    		systemJSON = NULL;
		}
        SYSTEM_LEAVE_CRITICAL();
        return sys_time;
	}
	return NULL;
}


LP_NSDK_SYSTEM_TIME NETSDK_conf_system_get_time(LP_NSDK_SYSTEM_TIME sys_time)
{
	return netsdk_conf_system_time(false, sys_time);
}

LP_NSDK_SYSTEM_TIME NETSDK_conf_system_set_time(LP_NSDK_SYSTEM_TIME sys_time)
{
	return netsdk_conf_system_time(true, sys_time);
}

void NETSDK_conf_system_operation(EM_NSDK_SYSTEM_OPERATION operation)
{
	if(netsdk->systemOperation){
		netsdk->systemOperation(operation);
	}
}

static LP_NSDK_SYSTEM_DEVICE_INFO netsdk_conf_system_device_info(bool setFlag, LP_NSDK_SYSTEM_DEVICE_INFO device_info)
{
    if(NULL == device_info)
    {
        return NULL;
    }

    if(0 == SYSTEM_ENTER_CRITICAL()){
    	LP_JSON_OBJECT systemJSON = json_object_get(netsdk->jsonSystem);
    	if(NULL != systemJSON){
    		LP_JSON_OBJECT deviceInfoJSON = NETSDK_json_get_child(systemJSON, "deviceInfo");
    		if(NULL != deviceInfoJSON){
				if(setFlag){
					NETSDK_json_set_string2(deviceInfoJSON, "deviceName", device_info->deviceName);
					NETSDK_json_set_int2(deviceInfoJSON, "deviceAddress", device_info->deviceAddress);
				}else{
					// get device infomation
					NETSDK_json_get_string(deviceInfoJSON, "deviceName", device_info->deviceName, sizeof(device_info->deviceName));
					device_info->deviceAddress = NETSDK_json_get_int(deviceInfoJSON, "deviceAddress");
					//APP_TRACE("deviceName=%s deviceAddress=%d", device_info->deviceName, device_info->deviceAddress);
					
				}
				if(setFlag){
					NETSDK_conf_system_save2();
				}
    		}
    		json_object_put(systemJSON);
    		systemJSON = NULL;
    	}
        SYSTEM_LEAVE_CRITICAL();
        return device_info;
    }   
	return NULL;
}


LP_NSDK_SYSTEM_DEVICE_INFO NETSDK_conf_system_get_device_info(LP_NSDK_SYSTEM_DEVICE_INFO device_info)
{
	if(netsdk->systemDeviceInfo){
		netsdk->systemDeviceInfo(device_info);
	}
	return netsdk_conf_system_device_info(false, device_info);
}

LP_NSDK_SYSTEM_DEVICE_INFO NETSDK_conf_system_set_device_info(LP_NSDK_SYSTEM_DEVICE_INFO device_info)
{
	return netsdk_conf_system_device_info(true, device_info);
}

static LP_NSDK_SYSTEM_DEVICE_INFO netsdk_conf_system_setting(bool setFlag, LP_NSDK_SYSTEM_SETTING info)
{
	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}

    if(NULL == info)
    {
        return NULL;
    }

    if(0 == SYSTEM_ENTER_CRITICAL()){

    	LP_JSON_OBJECT systemJSON = json_object_get(netsdk->jsonSystem);
    	LP_JSON_OBJECT promptSound = NETSDK_json_get_child(systemJSON, "PromptSounds");
    	LP_JSON_OBJECT Schedule= NETSDK_json_get_child(systemJSON, "MessagePushSchedule");
    	LP_JSON_OBJECT recordSchedule = NETSDK_json_get_child(systemJSON,"TFcard_recordSchedule");
        LP_JSON_OBJECT mdAlarm = NETSDK_json_get_child(systemJSON,"MdAlarm");
        LP_JSON_OBJECT capabilitySet = NETSDK_json_get_child(systemJSON,"CapabilitySet");
        LP_JSON_OBJECT pirManagerJSON = NETSDK_json_get_child(systemJSON,"pirManager");
    	
    	char text[128];
    	char customflag[32];
    	char *str = NULL;
    	
    	if(NULL != systemJSON){
			if(setFlag){
				NETSDK_json_set_boolean2(systemJSON, "MessagePushEnabled", info->messagePushEnabled);
                NETSDK_json_set_boolean2(systemJSON, "TimeRecordEnabled", info->timeRecordEnabled);
				NETSDK_json_set_boolean2(systemJSON, "MotionRecordEnabled", info->MotionRecordEnabled);

				if(Schedule){
					int i;
					for (i = 0
							; i < sizeof(info->AlarmNotification.Schedule)
								/ sizeof(info->AlarmNotification.Schedule[0]);
							++i)
					{
						LP_JSON_OBJECT Scheduletime = json_object_array_get_idx(Schedule, i);
						if(info->AlarmNotification.Schedule[i].enabled)
						{
							char beginTime[64]={0};
							char endTime[64]={0};
							char weekday[64]={0};							
							schedule_time_to_string(info->AlarmNotification.Schedule[i].BeginTime.hour
									, info->AlarmNotification.Schedule[i].BeginTime.min
									, info->AlarmNotification.Schedule[i].BeginTime.sec
									, beginTime, sizeof(beginTime));
					
							schedule_time_to_string(info->AlarmNotification.Schedule[i].EndTime.hour
									, info->AlarmNotification.Schedule[i].EndTime.min
									, info->AlarmNotification.Schedule[i].EndTime.sec
									, endTime, sizeof(endTime));
							schedule_weekday_to_string(info->AlarmNotification.Schedule[i].weekday
									, weekday
									, sizeof(weekday));

							if(!Scheduletime){
								Scheduletime = json_object_new_object();
								NETSDK_json_set_string2(Scheduletime, "Weekday", weekday);
								NETSDK_json_set_string2(Scheduletime, "BeginTime", beginTime);
								NETSDK_json_set_string2(Scheduletime, "EndTime", endTime);
								//NETSDK_json_set_boolean2(Scheduletime, "Enabled", true);
								json_object_array_put_idx(Schedule, i, Scheduletime);
							}
							else{
								NETSDK_json_set_string2(Scheduletime, "Weekday", weekday);
								NETSDK_json_set_string2(Scheduletime, "BeginTime", beginTime);
								NETSDK_json_set_string2(Scheduletime, "EndTime", endTime);
								//NETSDK_json_set_boolean2(Scheduletime, "Enabled", true);
							}
						}
						else
						{
							if(Scheduletime)
							{
								//NETSDK_json_set_boolean2(Scheduletime, "Enabled", false);
								//删除
								NETSDK_json_set_string2(Scheduletime, "Weekday", NULL);
								NETSDK_json_set_string2(Scheduletime, "BeginTime", NULL);
								NETSDK_json_set_string2(Scheduletime, "EndTime", NULL);
							}
						}
					}

				}
				if(promptSound){
					NETSDK_json_set_boolean2(promptSound, "Enabled", info->promptSound.enabled);
					str = NETSDK_MAP_DEC2STR(promptSoundType_map, info->promptSound.soundType, "chinese");
					NETSDK_json_set_string2(promptSound, "Type", str);
				}

				//Tfcard_recordSchedule
				if(NULL != recordSchedule){
					int i;
					for(i = 0
							; i < sizeof(info->TFcard_Record.Schedule)
								/ sizeof(info->TFcard_Record.Schedule[0]);
							i++)
					{
						LP_JSON_OBJECT recordScheduleTime = json_object_array_get_idx(recordSchedule,i);

						if(info->TFcard_Record.Schedule[i].enabled){
							char beginTime[64] = {0};
							char endTime[64] = {0};
							char weekday[64] = {0};

							schedule_time_to_string(info->TFcard_Record.Schedule[i].BeginTime.hour
									, info->TFcard_Record.Schedule[i].BeginTime.min
									, info->TFcard_Record.Schedule[i].BeginTime.sec
									, beginTime, sizeof(beginTime));
							schedule_time_to_string(info->TFcard_Record.Schedule[i].EndTime.hour
									, info->TFcard_Record.Schedule[i].EndTime.min
									, info->TFcard_Record.Schedule[i].EndTime.sec
									, endTime, sizeof(endTime));
							schedule_weekday_to_string(info->TFcard_Record.Schedule[i].weekday
									, weekday
									, sizeof(weekday));

							if(!recordScheduleTime){
								recordScheduleTime = json_object_new_object();
								NETSDK_json_set_string2(recordScheduleTime, "Weekday", weekday);
								NETSDK_json_set_string2(recordScheduleTime, "BeginTime", beginTime);
								NETSDK_json_set_string2(recordScheduleTime, "EndTime", endTime);
								//NETSDK_json_set_boolean2(Scheduletime, "Enabled", true);
								json_object_array_put_idx(recordSchedule, i, recordScheduleTime);
							}
							else{
								NETSDK_json_set_string2(recordScheduleTime, "Weekday", weekday);
								NETSDK_json_set_string2(recordScheduleTime, "BeginTime", beginTime);
								NETSDK_json_set_string2(recordScheduleTime, "EndTime", endTime);
								//NETSDK_json_set_boolean2(Scheduletime, "Enabled", true);
							}
							printf("\n***enable record_schedule, i = %d***\n",i);
						}else{
							if(recordScheduleTime){
								//NETSDK_json_set_boolean2(Scheduletime, "Enabled", false);
								NETSDK_json_set_string2(recordScheduleTime, "Weekday", NULL);
								NETSDK_json_set_string2(recordScheduleTime, "BeginTime", NULL);
								NETSDK_json_set_string2(recordScheduleTime, "EndTime", NULL);
								printf("\n***clear record_schedule, i = %d***\n",i);
							}
						}
					}
				}
                if(NETSDK_json_check_child(systemJSON, "area") == true) {
                    NETSDK_json_set_string(systemJSON, "area", info->area);
                }
                if(NULL != mdAlarm) {
                    NETSDK_json_set_boolean2(mdAlarm, "MotionWarningTone", info->mdAlarm.MotionWarningTone);
                    NETSDK_json_set_string(mdAlarm, "WarningToneType", info->mdAlarm.WarningToneType);
                }

                if(NULL != capabilitySet)
                {
                    NETSDK_json_set_int2(capabilitySet, "version", info->capabilitySet.version);
                    NETSDK_json_set_int2(capabilitySet, "maxChannel", info->capabilitySet.maxChannel);
                    NETSDK_json_set_string2(capabilitySet, "model", info->capabilitySet.model);
                    NETSDK_json_set_boolean2(capabilitySet, "powerBattery", info->capabilitySet.powerBattery);
                    NETSDK_json_set_boolean2(capabilitySet, "audioInput", info->capabilitySet.audioInput);
                    NETSDK_json_set_boolean2(capabilitySet, "audioOutput", info->capabilitySet.audioOutput);
                    NETSDK_json_set_boolean2(capabilitySet, "bluetooth", info->capabilitySet.bluetooth);
                    NETSDK_json_set_int2(capabilitySet, "lightControl", info->capabilitySet.lightControl);
                    NETSDK_json_set_int2(capabilitySet, "bulbControl", info->capabilitySet.bulbControl);
                    NETSDK_json_set_boolean2(capabilitySet, "ptz", info->capabilitySet.ptz);
                    NETSDK_json_set_boolean2(capabilitySet, "sdCard", info->capabilitySet.sdCard);
                    NETSDK_json_set_boolean2(capabilitySet, "lte", info->capabilitySet.lte);
                    NETSDK_json_set_boolean2(capabilitySet, "wifi", info->capabilitySet.wifi);
                    NETSDK_json_set_boolean2(capabilitySet, "rj45", info->capabilitySet.rj45);
                    NETSDK_json_set_boolean2(capabilitySet, "rtc", info->capabilitySet.rtc);
                    NETSDK_json_set_int2(capabilitySet, "fisheye", info->capabilitySet.fisheye);
                    NETSDK_json_set_boolean2(capabilitySet, "wifiStationCanSet", info->capabilitySet.wifiStationCanSet);
                    NETSDK_json_set_boolean2(capabilitySet, "pir", info->capabilitySet.pir);
                }

                pirManagerJSON = NETSDK_json_get_child(systemJSON, "pirManager");
                if(NULL != pirManagerJSON)
                {
                    str = NETSDK_MAP_DEC2STR(pirTrigger_map, info->pirManager.pirTrigger, "fallingEdge");
                    NETSDK_json_set_string2(pirManagerJSON, "trigger", str);
                }

			}else{
				// get device infomation
				info->messagePushEnabled = NETSDK_json_get_boolean(systemJSON, "MessagePushEnabled");	
                info->timeRecordEnabled = NETSDK_json_get_boolean(systemJSON, "TimeRecordEnabled");
				info->MotionRecordEnabled = NETSDK_json_get_boolean(systemJSON, "MotionRecordEnabled");
				int i;
				for(i = 0
						; i < sizeof(info->AlarmNotification.Schedule)
							/ sizeof(info->AlarmNotification.Schedule[0])
						; ++i)
				{
					info->AlarmNotification.Schedule[i].enabled = false;
				}
				for (i = 0
						; i < json_object_array_length(Schedule) && i < sizeof(info->AlarmNotification.Schedule)
							/ sizeof(info->AlarmNotification.Schedule[0]);
						++i)
				{
					LP_JSON_OBJECT Scheduletime = json_object_array_get_idx(Schedule, i);
					if(Scheduletime)
					{
						char beginTime[12]={0};
						char endTime[12]={0};
						char weekday[18]={0};
						int beginSec=0, endSec=0;
						//info->AlarmNotification.Schedule[i].enabled = NETSDK_json_get_boolean(Scheduletime, "Enabled");
						//if(false == info->AlarmNotification.Schedule[i].enabled){
						//if(NETSDK_json_get_boolean(Scheduletime, "Enabled")){
						//	printf("file=%s, line=%d\n", __FILE__, __LINE__);
						//	continue;
						//}
						NETSDK_json_get_string(Scheduletime, "Weekday", weekday, sizeof(weekday));
						NETSDK_json_get_string(Scheduletime, "BeginTime", beginTime, sizeof(beginTime));
						NETSDK_json_get_string(Scheduletime, "EndTime", endTime, sizeof(endTime));
						//printf("BeginTime:%s\n", beginTime);
						//printf("EndTime:%s\n", endTime);
						//printf("weekday:%s\n", weekday);
						//printf("\n\n");

						if(NULL != beginTime && NULL != endTime && NULL != weekday)
						{
							if(0 != schedule_parse_time(beginTime
									, &info->AlarmNotification.Schedule[i].BeginTime.hour
									, &info->AlarmNotification.Schedule[i].BeginTime.min
									, &info->AlarmNotification.Schedule[i].BeginTime.sec))
							{
								//APP_TRACE("Parse Begin Time Failed");
								continue;
							}
							if(0 != schedule_parse_time(endTime
									, &info->AlarmNotification.Schedule[i].EndTime.hour
									, &info->AlarmNotification.Schedule[i].EndTime.min
									, &info->AlarmNotification.Schedule[i].EndTime.sec))
							{
								//APP_TRACE("Parse End Time Failed");
								continue;
							}
							if(0 != schedule_parse_weekday(weekday
									, &(info->AlarmNotification.Schedule[i].weekday)))
							{
								//APP_TRACE("Parse Weekday Failed");
								continue;
							}
							info->AlarmNotification.Schedule[i].weekday &= 0x7f;
							beginSec = info->AlarmNotification.Schedule[i].BeginTime.hour * 3600
									+ info->AlarmNotification.Schedule[i].BeginTime.min * 60
									+ info->AlarmNotification.Schedule[i].BeginTime.sec;
							endSec = info->AlarmNotification.Schedule[i].EndTime.hour * 3600
									+ info->AlarmNotification.Schedule[i].EndTime.min * 60
									+ info->AlarmNotification.Schedule[i].EndTime.sec;
							// 判断星期以及时间的合法性
							if(0 != info->AlarmNotification.Schedule[i].weekday
									&& beginSec < endSec)
							{
								info->AlarmNotification.Schedule[i].enabled = true;
							}
							else
							{
								info->AlarmNotification.Schedule[i].enabled = false;
							}
						}
					}
				}
				if(promptSound){
					info->promptSound.enabled = NETSDK_json_get_boolean(promptSound, "Enabled");
					NETSDK_json_get_string(promptSound, "Type", text, sizeof(text));
					info->promptSound.soundType = NETSDK_MAP_STR2DEC(promptSoundType_map, text, kNSDK_SYSTEM_PROMPT_SOUND_TYPE_CHINESE);
					snprintf(info->promptSound.soundTypeStr, sizeof(info->promptSound.soundTypeStr), "%s", text);
					LP_JSON_OBJECT soundOpt = NETSDK_json_get_child(promptSound, "TypeProperty");
					if(soundOpt) {
						if(NETSDK_json_get_string(soundOpt, "opt", text, sizeof(text)) != NULL) {
							snprintf(info->promptSound.soundTypeOpt, sizeof(info->promptSound.soundTypeOpt), "%s", text);
						}
					}
				}

				//TFcard_recordSchedule
				int j = 0;
				for (j = 0
						; j < sizeof(info->TFcard_Record.Schedule)
							/ sizeof(info->TFcard_Record.Schedule[0]);
						++j)
				{
					info->TFcard_Record.Schedule[j].enabled = false;
				}
				if(NULL != recordSchedule)
				{
					for (j = 0
							; j < json_object_array_length(recordSchedule) && j < sizeof(info->TFcard_Record.Schedule)
								/ sizeof(info->TFcard_Record.Schedule[0]);
							++j)
					{
						LP_JSON_OBJECT recordScheduleTime = json_object_array_get_idx(recordSchedule, j);
						if(recordScheduleTime)
						{
							char beginTime[12]={0};
							char endTime[12]={0};
							char weekday[18]={0};
							int beginSec=0, endSec=0;

							NETSDK_json_get_string(recordScheduleTime, "Weekday", weekday, sizeof(weekday));
							NETSDK_json_get_string(recordScheduleTime, "BeginTime", beginTime, sizeof(beginTime));
							NETSDK_json_get_string(recordScheduleTime, "EndTime", endTime, sizeof(endTime));
							//printf("BeginTime:%s\n", beginTime);
							//printf("EndTime:%s\n", endTime);
							//printf("weekday:%s\n", weekday);
							//printf("\n\n");

							if(NULL != beginTime && NULL != endTime && NULL != weekday)
							{
								if(0 != schedule_parse_time(beginTime
										, &info->TFcard_Record.Schedule[j].BeginTime.hour
										, &info->TFcard_Record.Schedule[j].BeginTime.min
										, &info->TFcard_Record.Schedule[j].BeginTime.sec))
								{
									//APP_TRACE("Parse RecordScheduleTime Begin Time Failed");
									continue;
								}
								if(0 != schedule_parse_time(endTime
										, &info->TFcard_Record.Schedule[j].EndTime.hour
										, &info->TFcard_Record.Schedule[j].EndTime.min
										, &info->TFcard_Record.Schedule[j].EndTime.sec))
								{
									//APP_TRACE("Parse RecordScheduleTime End Time Failed");
									continue;
								}
								if(0 != schedule_parse_weekday(weekday
										, &(info->TFcard_Record.Schedule[j].weekday)))
								{
									//APP_TRACE("Parse RecordScheduleTime Weekday Failed");
									continue;
								}
								info->TFcard_Record.Schedule[j].weekday &= 0x7f;
								beginSec = info->TFcard_Record.Schedule[j].BeginTime.hour * 3600
										+ info->TFcard_Record.Schedule[j].BeginTime.min * 60
										+ info->TFcard_Record.Schedule[j].BeginTime.sec;
								endSec = info->TFcard_Record.Schedule[j].EndTime.hour * 3600
										+ info->TFcard_Record.Schedule[j].EndTime.min * 60
										+ info->TFcard_Record.Schedule[j].EndTime.sec;

								if(0 != info->TFcard_Record.Schedule[j].weekday
										&& beginSec < endSec)
								{
									info->TFcard_Record.Schedule[j].enabled = true;
									//printf("TFcard_Record.Schedule[%d].enabled = true",j);
								}
								else
								{
									info->TFcard_Record.Schedule[j].enabled = false;
									//printf("TFcard_Record.Schedule[%d].enabled = false",j);
								}
							}
						}
					}
				}
                if(NETSDK_json_get_string(systemJSON, "area", text, sizeof(text)) != NULL) {
                    snprintf(info->area, sizeof(info->area), "%s", text);
                }
                if(NULL != mdAlarm) {
                    info->mdAlarm.MotionWarningTone = NETSDK_json_get_boolean(mdAlarm, "MotionWarningTone");

                    NETSDK_json_get_string(mdAlarm, "WarningToneType", customflag, sizeof(customflag));
                    snprintf(info->mdAlarm.WarningToneType, sizeof(info->mdAlarm.WarningToneType), "%s", customflag);
                }

                if(NULL != capabilitySet)
                {
                    char strModel[64] = {0};
                    info->capabilitySet.version = NETSDK_json_get_int(capabilitySet, "version");
                    info->capabilitySet.maxChannel = NETSDK_json_get_int(capabilitySet, "maxChannel");
                    if(NULL != NETSDK_json_get_string(capabilitySet, "model", strModel, sizeof(strModel)))
                    {
                        snprintf(info->capabilitySet.model, sizeof(info->capabilitySet.model), "%s", strModel);
                    }
                    else
                    {
                        snprintf(info->capabilitySet.model, sizeof(info->capabilitySet.model), "");
                    }
                    info->capabilitySet.powerBattery = NETSDK_json_get_boolean(capabilitySet, "powerBattery");
                    info->capabilitySet.audioInput = NETSDK_json_get_boolean(capabilitySet, "audioInput");
                    info->capabilitySet.audioOutput = NETSDK_json_get_boolean(capabilitySet, "audioOutput");
                    info->capabilitySet.bluetooth = NETSDK_json_get_boolean(capabilitySet, "bluetooth");
                    info->capabilitySet.lightControl = NETSDK_json_get_int(capabilitySet, "lightControl");
                    info->capabilitySet.bulbControl = NETSDK_json_get_int(capabilitySet, "bulbControl");
                    info->capabilitySet.ptz = NETSDK_json_get_boolean(capabilitySet, "ptz");
                    info->capabilitySet.sdCard = NETSDK_json_get_boolean(capabilitySet, "sdCard");
                    info->capabilitySet.lte = NETSDK_json_get_boolean(capabilitySet, "lte");
                    info->capabilitySet.wifi = NETSDK_json_get_boolean(capabilitySet, "wifi");
                    info->capabilitySet.rj45 = NETSDK_json_get_boolean(capabilitySet, "rj45");
                    info->capabilitySet.rtc = NETSDK_json_get_boolean(capabilitySet, "rtc");
                    info->capabilitySet.fisheye = NETSDK_json_get_int(capabilitySet, "fisheye");
                    info->capabilitySet.wifiStationCanSet = NETSDK_json_get_boolean(capabilitySet, "wifiStationCanSet");
                    info->capabilitySet.pir = NETSDK_json_get_boolean(capabilitySet, "pir");
                }

                pirManagerJSON = NETSDK_json_get_child(systemJSON, "pirManager");
                if(NULL != pirManagerJSON)
                {
                    memset(text, 0, sizeof(text));
                    NETSDK_json_get_string(pirManagerJSON, "trigger", text, sizeof(text));
                    info->pirManager.pirTrigger = NETSDK_MAP_STR2DEC(pirTrigger_map, text, kNSDK_PIR_MANAGER_TRIGGER_FALLING_EDGE);
                }

			}
			if(setFlag){
				NETSDK_conf_system_save2();
			}
		}	
		json_object_put(systemJSON);
		systemJSON = NULL;

        SYSTEM_LEAVE_CRITICAL();
		return info;
    }
	return NULL;
}


LP_NSDK_SYSTEM_SETTING NETSDK_conf_system_get_setting_info(LP_NSDK_SYSTEM_SETTING info)
{
	return netsdk_conf_system_setting(false, info);
}

LP_NSDK_SYSTEM_SETTING NETSDK_conf_system_set_setting_info(LP_NSDK_SYSTEM_SETTING info)
{
	return netsdk_conf_system_setting(true, info);
}

static LP_NSDK_SYSTEM_DST netsdk_conf_system_daylight_saving_time_info(bool setFlag, LP_NSDK_SYSTEM_DST info)
{
	int i, n;
	LP_JSON_OBJECT dstJSON = NULL, weekJSON = NULL, tmpWeekJSON = NULL;

    if(NULL == info)
    {
        return NULL;
    }

    if(0 == SYSTEM_ENTER_CRITICAL()){
        LP_JSON_OBJECT systemJSON = json_object_get(netsdk->jsonSystem);
    	if(NULL != systemJSON){
    		dstJSON = NETSDK_json_get_child(systemJSON, "time.DaylightSavingTime");
    		if(NULL != dstJSON){
    			if(setFlag){
    				int j;
    				char tmpType[8];
    				
    				NETSDK_json_set_boolean2(dstJSON, "Enabled", info->enable);
                    NETSDK_json_set_string2(dstJSON, "Country", info->country);
    				NETSDK_json_set_int2(dstJSON, "Offset", info->offset);
    				if((weekJSON = NETSDK_json_get_child(dstJSON, "Week")) != NULL){
    					n = json_object_array_length(weekJSON);
    					for(j = 0; j < n; j++){
    						if((tmpWeekJSON = json_object_array_get_idx(weekJSON, j)) != NULL){
    							memset(tmpType, 0, sizeof(tmpType));
    							NETSDK_json_get_string(tmpWeekJSON, "Type", tmpType, sizeof(tmpType));
    							for(i = 0; i < sizeof(info->week) / sizeof(info->week[0]); i++){
    								if(strcmp(tmpType, info->week[i].type) == 0){
    									NETSDK_json_set_int2(tmpWeekJSON, "Month", info->week[i].month);
    									NETSDK_json_set_int2(tmpWeekJSON, "Week", info->week[i].week);
    									NETSDK_json_set_int2(tmpWeekJSON, "Weekday", info->week[i].weekday);
    									NETSDK_json_set_int2(tmpWeekJSON, "Hour", info->week[i].hour);
    									NETSDK_json_set_int2(tmpWeekJSON, "Minute", info->week[i].minute);
    									break;
    								}
    							}
    						}
    					}
    				}
    			}
    			else{
    				info->enable = NETSDK_json_get_boolean(dstJSON, "Enabled");
                    NETSDK_json_get_string(dstJSON, "Country", info->country, sizeof(info->country));
    				info->offset = NETSDK_json_get_int(dstJSON, "Offset");
    				if((weekJSON = NETSDK_json_get_child(dstJSON, "Week")) != NULL){
    					n = json_object_array_length(weekJSON);
    					for(i = 0; i < n && i < sizeof(info->week) / sizeof(info->week[0]); i++){
    						if((tmpWeekJSON = json_object_array_get_idx(weekJSON, i)) != NULL){
    							NETSDK_json_get_string(tmpWeekJSON, "Type", info->week[i].type, sizeof(info->week[i].type));
    							info->week[i].month = NETSDK_json_get_int(tmpWeekJSON, "Month");
    							info->week[i].week = NETSDK_json_get_int(tmpWeekJSON, "Week");
    							info->week[i].weekday = NETSDK_json_get_int(tmpWeekJSON, "Weekday");
    							info->week[i].hour = NETSDK_json_get_int(tmpWeekJSON, "Hour");
    							info->week[i].minute = NETSDK_json_get_int(tmpWeekJSON, "Minute");
    						}
    					}
    				}
    			}
    			if(setFlag){
    				NETSDK_conf_system_save2();
    			}
            }	
        }
		json_object_put(systemJSON);
		systemJSON = NULL;
		
        SYSTEM_LEAVE_CRITICAL();
		return info;
	}
	
	return NULL;
}

LP_NSDK_SYSTEM_DST NETSDK_conf_system_get_DST_info(LP_NSDK_SYSTEM_DST info)
{
	return netsdk_conf_system_daylight_saving_time_info(false, info);
}

LP_NSDK_SYSTEM_DST NETSDK_conf_system_set_DST_info(LP_NSDK_SYSTEM_DST info)
{
	return netsdk_conf_system_daylight_saving_time_info(true, info);
}

static LP_NSDK_SYSTEM_REC_MANAGER netsdk_conf_system_record_info(bool setFlag, LP_NSDK_SYSTEM_REC_MANAGER recManager)
{
	LP_JSON_OBJECT recManagerJSON = NULL;

    if(NULL == recManager)
    {
        return NULL;
    }

    if(0 == SYSTEM_ENTER_CRITICAL()){
        LP_JSON_OBJECT systemJSON = json_object_get(netsdk->jsonSystem);
    	if(NULL != systemJSON){
    		recManagerJSON = NETSDK_json_get_child(systemJSON, "RecordManager");
    		if(NULL != recManagerJSON){
				if(setFlag){
					NETSDK_json_set_string2(recManagerJSON, "Mode", recManager->recMode);
					NETSDK_json_set_boolean2(recManagerJSON, "UseIOAlarm", recManager->useIOAlarm);
				}
				else{
					NETSDK_json_get_string(recManagerJSON, "Mode", recManager->recMode, sizeof(recManager->recMode));
					//APP_TRACE("Mode (%s)", recManager->recMode);
					recManager->useIOAlarm = NETSDK_json_get_boolean(recManagerJSON, "UseIOAlarm");
				}
				if(setFlag){
					NETSDK_conf_system_save2();
				}
    		}
    		json_object_put(systemJSON);
    		systemJSON = NULL;
    	}
        SYSTEM_LEAVE_CRITICAL();
        return recManager;
    }
	return NULL;
}

LP_NSDK_SYSTEM_REC_MANAGER NETSDK_conf_system_get_record_info(LP_NSDK_SYSTEM_REC_MANAGER recManager)
{
	return netsdk_conf_system_record_info(false, recManager);
}

LP_NSDK_SYSTEM_REC_MANAGER NETSDK_conf_system_set_record_info(LP_NSDK_SYSTEM_REC_MANAGER recManager)
{
	return netsdk_conf_system_record_info(true, recManager);
}

