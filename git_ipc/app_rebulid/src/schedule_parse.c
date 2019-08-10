#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "schedule_parse.h"

int schedule_parse_time(char *text, unsigned int *hour, unsigned int *minute, unsigned int *second)
{
	unsigned int hh, mm, ss;

	if(sscanf(text, "%d:%d:%d", &hh, &mm, &ss) != 3)
	{
        return -1;
	}

	// ÅÐ¶ÏºÏ·¨ÐÔ
	if((hh >= 0 && hh <= 23)
	                && (mm >= 0 && mm <= 59)
	                && (ss >= 0 && ss <= 59))
	{
        *hour = hh;
        *minute = mm;
        *second = ss;
        return 0;
	}

	return -1;
}

void schedule_time_to_string(unsigned int hour, unsigned int minute, unsigned int second, char *stack, unsigned int  stackLen)
{
	snprintf(stack, stackLen
			, "%02d:%02d:%02d"
			, (int)(hour)
			, (int)(minute)
			, (int)(second));
}

int schedule_parse_weekday(const char *text, unsigned int *weekday)
{

    char *txt = strdup(text);
    char *token = NULL;
    char *chr = NULL;

    *weekday = 0;
    chr = strtok_r(txt, ",", &token);
    while(NULL != chr)
    {
        int day = atoi(chr);
        if(day >= 0 && day < 7)
        {
			*weekday |= (1 << day);
        }
        chr = strtok_r(NULL, ",", &token);
    }
	free(txt);
	
    return 0;
}

void schedule_weekday_to_string(int weekday, char *stack, int stackLen)
{
    int i;
    strcpy(stack, "");
    for(i = 0; i < 7; ++i)
    {
        if(weekday & (1 << i))
        {
            if(!strlen(stack))
            {
                snprintf(stack + strlen(stack), stackLen - strlen(stack), "%d", i);
            }
            else
            {
				snprintf(stack + strlen(stack), stackLen - strlen(stack), ",%d", i);
            }
        }
    }
}

