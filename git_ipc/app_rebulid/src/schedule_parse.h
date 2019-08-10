
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SCHEDULE_H_
#define SCHEDULE_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int schedule_parse_time(char *text, unsigned int *hour, unsigned int *minute, unsigned int *second);
extern int schedule_parse_weekday(const char *text, unsigned int *weekday);
extern void schedule_time_to_string(unsigned int hour, unsigned int minute, unsigned int second, char *stack, unsigned int  stackLen);
extern void schedule_weekday_to_string(int weekday, char *stack, int stackLen);

#ifdef __cplusplus
};
#endif
#endif //SDCARD_WRITER_H_

