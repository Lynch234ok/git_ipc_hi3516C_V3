#ifndef __AF_INFO_H__
#define __AF_INFO_H__

int AFStatistics_Init(void);
int AFStatistics_Exit(void);
int AFStatistics_GetFV(unsigned long * Fv1, unsigned long * Fv2, int TimeOut);

#endif //__AF_INFO_H__
