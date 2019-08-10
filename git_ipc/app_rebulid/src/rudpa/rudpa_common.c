#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "rudpa_common.h"

#define MAX_RUDPA_LOG_LEN  2000

uint32_t rudpa_log( char *log_file, char *mode, char *log_fmt, ...)
{
	FILE *fp = fopen(log_file, mode);
	int32_t ret = -1;
	if(fp){
		char *log_buf = (char *)calloc(MAX_RUDPA_LOG_LEN, 1);
		if(log_buf){
			va_list ap;
			va_start(ap, log_fmt);
			vsnprintf(log_buf,MAX_RUDPA_LOG_LEN, log_fmt, ap);
			va_end(ap);
			ret = fwrite(log_buf, 1, strlen(log_buf), fp);
			if(ret <= 0){
				printf("fwrite \"%s\" failed\n", log_buf);			
			}
			free(log_buf);
		}		
		fclose(fp);
	}else{
		printf("open \"log_path\" failed\n", log_file);
	}
}


//#define TEST_RUDPA_LOG
#ifdef TEST_RUDPA_LOG
int main(void)
{
	rudpa_log( "/tmp/rudpa.port", "w+","%d", 56789);
	
	rudpa_log( "/tmp/rudpa.error", "a+","%s", "hello");
	rudpa_log( "/tmp/rudpa.error", "a+","%s", "  world");
	return 0;
}
#endif

