#include <stdio.h>
#include <time.h>
#include "netsdk_def.h"

bool schedule_messagePush(ST_NSDK_SYSTEM_SETTING sinfo)
{
	unsigned int cur_weekday=0;
	unsigned int cur_sec=0;

	time_t cur_time;
	struct tm *p;
	time(&cur_time);
	p=localtime(&cur_time); /*取得当地时间*/
	
	cur_weekday = 1 << p->tm_wday;
	cur_sec = p->tm_hour * 3600 + p->tm_min * 60 + p->tm_sec;

	int i;
	for (i = 0
			; i < sizeof(sinfo.AlarmNotification.Schedule)
				/ sizeof(sinfo.AlarmNotification.Schedule[0]);
			++i)
	{
		if(sinfo.AlarmNotification.Schedule[i].enabled)
		{
			int beginSec, endSec;
			beginSec = sinfo.AlarmNotification.Schedule[i].BeginTime.hour * 3600
					+ sinfo.AlarmNotification.Schedule[i].BeginTime.min * 60
					+ sinfo.AlarmNotification.Schedule[i].BeginTime.sec;
			endSec = sinfo.AlarmNotification.Schedule[i].EndTime.hour * 3600
					+ sinfo.AlarmNotification.Schedule[i].EndTime.min * 60
					+ sinfo.AlarmNotification.Schedule[i].EndTime.sec;

			if((sinfo.AlarmNotification.Schedule[i].weekday) & cur_weekday)
			{
				if(cur_sec >= beginSec && cur_sec <= endSec){
					return true;
				}
			}
		}
	}

	return false;
}

bool TFcard_motion_record_isenabled()
{
	ST_NSDK_MD_CH sinfo={0};
	
	NETSDK_conf_md_ch_get(1,&sinfo);
	return (sinfo.enabled == true?true:false);
}

bool TFcard_motion_record_isenabled_new(void)
{
	ST_NSDK_SYSTEM_SETTING sysInfo;

	NETSDK_conf_system_get_setting_info(&sysInfo);
	return (sysInfo.MotionRecordEnabled == true? true:false);
}
