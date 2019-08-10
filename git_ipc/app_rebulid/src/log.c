#include "log.h"
#include "generic.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <dirent.h>
#include <ftw.h>

//static int log_fd = -1;

#define TF_CARD_FOLDER 		"/media/tf"
#define LOG_FOLDER 			"/media/tf/log"
#define LOG_FILE_MAXSIZE 	512
#define LOG_FOLDER_SIZE		64*1024*1024

static unsigned int totalsize = 0;

/*
*获取目录所占空间大小
*/
static int check_size(const char *fpath, const struct stat *sb, int typeflag) 
{ 
     totalsize += sb->st_size;
     return 0;
}

static int get_OneFileName(int max, unsigned char *file_path)
{
	DIR * dirptr;
	struct dirent * ptr;
	unsigned int file_number=0;
	unsigned int pre_number = 0;
	int i=0;
	char *tmp = NULL;
	if((dirptr = opendir(file_path)) == NULL)
	{
		return -1;
	}

	while((ptr = readdir(dirptr)) != NULL)
	{
		if(ptr->d_name[0] == '.')
		{
			continue;
		}

		if(ptr->d_name){
			tmp = strchr(ptr->d_name, '.');
			if(tmp){
				memset(tmp, 0, 1);
			}
			file_number = atoi(ptr->d_name);
		}

		if((file_number > pre_number) && (pre_number != 0) && (max == 0)){
		    file_number = pre_number;
		}
		if((file_number < pre_number) && (pre_number != 0) && (max == 1)){
		    file_number = pre_number;
		}
			pre_number = file_number;
		}
		if(dirptr){
		closedir(dirptr);
		dirptr = NULL;
	}
	
	return file_number;
}

//回收最小文件名开头的日志文件
static int recylceOneFile(unsigned char *file_path)
{
	char cmd_text[80]={0};
	unsigned int name = get_OneFileName(0, file_path);
	if(name == 0){
		name =1;
	}
		
	sprintf(cmd_text, "rm -rf %s/%d.log", file_path, name);
	printf("cmd_text=%s\n", cmd_text);
	system(cmd_text);
	return 0;
}

static bool is_path_mounted(const char *path)
{
	const char *lineRet = NULL;
	const char *subStrRet = NULL;
	const char *proc_mounts = "/proc/mounts";
	FILE *fd_proc_mounts = NULL;
	char line[512];
	int line_n = sizeof(line);

	if (NULL == path) {
		printf("%s: mount dir can't be NULL\n", __FUNCTION__);
		return false;
	}

	fd_proc_mounts = fopen(proc_mounts, "r");
	if (NULL == fd_proc_mounts) {
		printf("%s: Failed to open file: %s\n", __FUNCTION__, proc_mounts);
		return false;
	}

	// 逐行查找 path
	while (1) {
		lineRet = fgets(line, line_n, fd_proc_mounts);
		if (NULL != lineRet) {
			subStrRet = strstr(lineRet, path);
			if (NULL != subStrRet) {
                printf("%s: found at: %s\n", __FUNCTION__, lineRet);
                break;
			}
		} else {
			break;
		}
	}

	fclose(fd_proc_mounts);

	// 找到 dir 说明已挂载，否则未挂载
	if (NULL == subStrRet) {
		return false;
	} else {
		return true;
	}
}

void onFlush(unsigned char *bytes, unsigned int len)
{
	char cmd_text[25]={0};
	int i = 0;
	static unsigned int ii = 0;
	unsigned int mm = 0;
	int logfd = -1;
	char filename[128] = {0};

    // 此接口暂时放置较全面的打印信息
    printf("%s:%d Flushing Log.\n", __FUNCTION__, __LINE__);
	if(len == 0)
	{
		len = strlen(bytes);
	}
	totalsize = 0;
	// SD挂载状态检测，挂载了才写入日志否则打印信息
	if(!is_path_mounted(TF_CARD_FOLDER)) {
        printf("%s:%d TF_CARD_FOLDER:%s is not mounted!! Do not flush.\n", __FUNCTION__, __LINE__, TF_CARD_FOLDER);
		return;
	}
    printf("%s:%d TF_CARD_FOLDER:%s mounted.\n", __FUNCTION__, __LINE__, TF_CARD_FOLDER);

    //LOG目录检测
	if (-1 == access(LOG_FOLDER, F_OK))	{
		sprintf(cmd_text, "mkdir %s", LOG_FOLDER);
        printf("%s", cmd_text);
		if (0 != system(cmd_text)) {
            printf("%s:%d LOG_FOLDER:%s Failed to create!!\n", __FUNCTION__, __LINE__, LOG_FOLDER);
            return;
        }
	}


	//LOG目录大于16M进行清理操作
	if (ftw(LOG_FOLDER, &check_size, 1)) {
        printf("%s:%d LOG_FOLDER:%s get size failed!!\n", __FUNCTION__, __LINE__, LOG_FOLDER);
        return;
    }
	//清理开始的LOG文件,目录超过16M
	if(totalsize > LOG_FOLDER_SIZE)
	{
		printf("recylceOneFile\n");
		recylceOneFile(LOG_FOLDER);
	}

	//找到要开始写的文件,找出最大的文件名
	mm = get_OneFileName(1, LOG_FOLDER);
	if(mm == 0){
		mm= 1;
	}
	sprintf(filename, "%s/%d.log", LOG_FOLDER, mm);	
	//日志文件大于512K bytes重新开一个文件
	if((get_file_size(filename) / 1024) < LOG_FILE_MAXSIZE){
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "%s/%d.log", LOG_FOLDER, mm);
		i = 0;
		while(logfd < 0)
		{
			if(i > 50)
			{
				break;
			}
			i++;
			usleep(5000);
			logfd = open(filename, O_CREAT|O_RDWR |O_APPEND);
		}
	}
	else
	{
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "%s/%d.log", LOG_FOLDER, mm+1);
		i=0;
		while(logfd < 0)
		{
			if(i > 50)
			{
				break;
			}
			i++;
			usleep(5000);
			logfd = open(filename, O_CREAT|O_WRONLY);
		}
	}
	
	if (logfd > 0) {
		//printf("line=%d\n", __LINE__);		
		write(logfd, bytes, len);
		close(logfd);
	}
	//写入LOG 文件	
	if(logfd){
		close(logfd);
		logfd = 0;
	}

    printf("%s:%d Flush Log Finished.\n", __FUNCTION__, __LINE__);
}


#if 0
int main(int argc, char *argv[])
{
	char *mychar = "this very good!!!";
	char *mychar2 = "bad badbad badbad badbad badbad badbad bad";
	
	while(1){
		sleep(2);
		onFlush(mychar, strlen(mychar));
		onFlush(mychar2, strlen(mychar2));
	}
	
	return 0;
}
#endif
