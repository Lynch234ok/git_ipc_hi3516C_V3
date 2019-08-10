#include <stdio.h>
#include <time.h>
#include <sys/statfs.h>
#include <sys/stat.h>
//#include <io.h>
#include <stdint.h>

#include <string.h>
//#include <iostream.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>






//#define max_file_size (67108864);
//const T_U32 max_file_size = 67108864UL;//64M
//#define preservation_dise_size (67108864);//预留空间 64M
//#define image_max_num   (200);


	const int32_t max_file_size = 67108864;
	const int32_t image_max_num = 200;
	const int64_t preservation_dise_size = 67108864;

	const  char sd_root_path[100] ="/mnt";         //固定值 sd 挂载目录    
	const char record_file_root_dir[100] = "/mnt/VIDEO";//固定值  ，在hour时间切换的时候使用
	const char image_file_root_dir[100] = "/mnt/IMAGE";//固定值，图像目录


	char  record_file_dir[100]  = "";   //录像的上层目录     /mnt/VIDEO/1             目录1     
	char  record_file_path[100] = "/mnt/VIDEO";//可进行初值设定,随着写入过程改变，如/mnt/VIDEO/1



	char file_name[50] = "";   //tmp

	char index_file_path[50] = "";//目录中的txt文件索引地址；




	int32_t time_change_flag = 0;  //时间hour改变，或者掉电等重启，或者日期重设，都需要新建目录，标志   1，需要重建；   0，不需要
	char current_time_hour[50];//获取当前系统时间，保持，精确到hour，要是不一样就更新，并新建目录
	//T_U32 record_file_num = 0;










//加一个函数，判断目录是否存在
int32_t is_exists(char *path){
	if(NULL == path){
		printf("The path is NULL!%d\n",__LINE__);
		return -1;
	}
	if(0 == access(path,0)){
		printf("The path is exists!%d\n",__LINE__);
		return 0;
	}
	else 
		printf("The path is not exists!%d\n",__LINE__);
		return -1;
} 

int32_t add_entry_to_txt_file(char *info_record_file, char *index_file_path)//将info_record_file文件信息写入
{
	if(NULL == index_file_path){
		printf("The index file path is NULL!%d\n",__LINE__);
		return -1;
	}
	FILE *index_fp;	
	
	if(NULL != (index_fp = fopen(index_file_path,"ab+"))){
		fwrite(info_record_file,512,1,index_fp);
		return 0;
	}
	return -1;
}



int32_t make_txt_index(char * record_file_dir)//建立一个索引文件.txt文件  在创建目录的时候调用一次
{   
	
	char index_file_name[30] = "IndexFileName";
	
	char dir_name[50];
	FILE *index_fp;
	int32_t m = 0,n = 0;
	

	n = m = strlen(record_file_dir);
	while('/' !=record_file_dir[m]){
		m-=1;
	}
	m = m+1;
	memset(dir_name,0,sizeof(dir_name));
	while( m <= n ){
		strcat(dir_name,record_file_dir[m]);
	}
	
	//       mnt/video/1/indexfilename1.txt      
	snprintf(index_file_path,"%s/%s%s.txt", record_file_dir,index_file_name,dir_name);

	index_fp = fopen(index_file_path,"wb+");
	if(NULL == index_fp){
		printf("Can't open the index-file!%d\n",__LINE__);
		return -1;
	}
	fclose(index_file_path);
	return 0;;
}




//创建环境，跟目录下的录像文件夹video。图像文件夹，or  exe   database
int32_t ini_sd_dir_creat(char * sd_root_path){

	if(NULL == sd_root_path){
		strcpy( sd_root_path,"/mnt");	
	}
	char dir_tmp[100];


//创建           /mnt/VIDEO            目录	    初始化的时候，目录不存在就创建。
	
	memset(dir_tmp,0,sizeof(dir_tmp));
	sprintf(dir_tmp,"%s/%s",sd_root_path,"VIDEO");
	if(0 != is_exists(dir_tmp)){
		if(0 != mkdir(dir_tmp,S_IRWXU | S_IRWXG | S_IRWXO)){
			printf("Can't creat the direction!%d\n",__LINE__);
			return -1;
		}
	}
	


//创建            /mnt/IMAGE            图像目录
	memset(dir_tmp,0,sizeof(dir_tmp));
	sprintf(dir_tmp,"%s/%s",sd_root_path,"IMAGE");
	
	if(0 != is_exists(dir_tmp)){
		if(0 != mkdir(dir_tmp,S_IRWXU | S_IRWXG | S_IRWXO)){
			printf("Can't creat the direction!%d\n",__LINE__);
			return -1;
		}
	}
	return 0;
}

int64_t get_disk_size( char * sd_root_path)   //由文件目录名，获取系统磁盘大小record_root_path
{
	printf("%d\n",__LINE__);

	if(NULL == sd_root_path){
		printf("The recording path is NULL!%d\n",__LINE__);
		return -1;
	}
	struct statfs disk_statfs;
	printf("%d\n",__LINE__);
	memset(&disk_statfs,0,sizeof(disk_statfs));
//	while(-1 == statfs(sd_root_path,&disk_statfs)){
//		if(errno != EINTR){
//			printf("Other ERRO!%d\n",__LINE__);
//			return -1;
//			}
//	}
	if( statfs(sd_root_path,&disk_statfs) >= 0 ){
		printf("%d\n",__LINE__);
		return (int64_t )(disk_statfs.f_blocks) * ( int64_t )(disk_statfs.f_bsize);
	}
	printf("%d\n",__LINE__);
	return -1;
}




struct tm * get_record_time(char * pstr_data)//获取录像的数据流帧中的头时间     可以不需要  keep   删除
{
	struct tm* t_record_time;
	//strtok_r (record_buffer,'.',token);
	
	
	return t_record_time;
}



int32_t  is_new_creat_dir()//判断时间到hour是否改变，
{
	struct tm *local_time;
	time_t t_local;
	char new_time[50];

	t_local = time(NULL);
	local_time = localtime(&t_local);

	sprintf(new_time,"%s.%02d.%02d.%02d",1900+local_time->tm_year,
		local_time->tm_mon+1,local_time->tm_mday,local_time->tm_hour);
	if(0 == strcmp(new_time,current_time_hour)){
		printf("The hour of time is not changed!%d\n",__LINE__);
		return -1;
	}
	printf("The hour had been changed!Creat a new dir!%d\n",__LINE__);
	
	return 0;
}



 int32_t is_record_file(char *file_name)//从名字特征判断是否是录像文件，后缀，长度      keep         可以删除
{

	return 0;
}

 int64_t get_old_files_size( char * p_record_file_root )
{
//获取已存录像文件的大小，即空间使用占用大小, 参数  record_file_root_dir   /mnt/VIDEO中的文件
	int64_t total_size = 0;
	DIR *dirp = NULL;
	struct dirent *direntp = NULL;
	struct stat stat_buf;
	char str_file_path[50];
//	char *file_name_tmp = NULL;

	memset(str_file_path,0,sizeof(str_file_path));
	if(NULL == (dirp = opendir(p_record_file_root))){
		printf("Can't open the dir !%d\n",__LINE__ );
		return -1;
	}
	while(NULL != (direntp = readdir(dirp))){
		if(NULL == direntp->d_name ){
			continue;
			}
		
		if ( !( strcmp( direntp->d_name, "." ) ) || !( strcmp( direntp->d_name, ".." ) ) ) {
			continue;
		}
	//判断是否是录像文件    is_record_file()
	
//	    file_name_tmp = strdupa(direntp->d_name);
		if(0 != is_record_file(direntp->d_name)){
			continue;
		}
		memset(str_file_path,0,sizeof(str_file_path));
		sprintf(str_file_path,"%s/%s", p_record_file_root,direntp->d_name);
		
		if(-1 == lstat(str_file_path,&stat_buf)){
			memset(&stat_buf,0,sizeof(stat_buf));
			if(-1 == lstat(str_file_path,&stat_buf)){
				continue;
				}
			}
		if(!S_ISREG(stat_buf.st_mode)){
			continue;
			}
		total_size += stat_buf.st_size;
	}
	closedir(dirp);
	return total_size;
}



/*
static T_S32 remove_oldest_file( char *pstr_record_dir_path )
{
	//循环，查找到最老的那个文件，通过文件属性状态函数lstat， 中的st_time参数，
	//距离1700年的秒数，越小说明文件越老，循环替换过程,释放一些空间
	
	DIR *pdir = NULL;
	struct dirent *pdirent = NULL;
	struct stat stat_buf;
	time_t older_file_time  = 0x7FFFFFFF;
	long nfile_cnt = 0;

	char str_dir_path[40];
	char str_file_path_tmp[40];
	char str_cmp_path_tmp[40];

	bzero(str_dir_path,sizeof(str_dir_path));

	
	strcpy(str_dir_path,pstr_record_dir_path);

	if(NULL == (pdir = opendir(str_dir_path))){
		printf("Can't open the dir! %d\n",__LINE__);
		return -1;
	}
	while(NULL != (pdirent = readdir(pdir))){
		if(NULL == pdirent->d_name ){
			continue;
			}
		if(pdirent->d_name == "." || pdirent->d_name == ".."){
			continue;
		}
		if(!is_record_file(pdirent->d_name)){
			continue;
			}
		bzero(str_file_path_tmp,sizeof(str_file_path_tmp));
		sprintf(str_file_path_tmp,"%s/%s",str_dir_path,pdirent->d_name);
		if(-1 == lstat(str_file_path_tmp,&stat_buf)){
			printf("Can't get the stat !%d\n ",__LINE__);
			return -1;
		}
		if( older_file_time >= stat_buf.st_mtime){
			memcmp(str_cmp_path_tmp,str_file_path_tmp);
			older_file_time = stat_buf.st_mtime;
			}
	}

	if(0 != remove(str_cmp_path_tmp){
		printf("Remove erro!%d\n",__LINE__);
		return -1;
		}
	printf("DELL the old file :%s  ** %d\n",str_cmp_path_tmp,__LINE__);
	return 0;
}

*/


static int32_t sd_file_remove( char * dir_path)// 先把目录下的文件删除，再删除目录
{	
	if(NULL == dir_path){
		printf("The dir path is NULL!%d\n",__LINE__);
		return -1;
	}

	DIR *p_dir = NULL;
	struct dirent *p_dirent = NULL;

	char file_path_tmp[100];

	if(NULL == (p_dir = opendir(dir_path))){
		printf("Can't open the dir!%d\n",__LINE__);
		return -1;
	}

	while(NULL != (p_dirent = readdir(p_dir))){
		if(!p_dirent->d_name ){
			continue;
		}
		if(p_dirent->d_name == "." || p_dirent->d_name == ".."){
			continue;
		}
		
		memset(file_path_tmp,0,sizeof(file_path_tmp));
		sprintf(file_path_tmp,"%s/%s",dir_path,p_dirent->d_name);
		if(0 != remove(file_path_tmp)){
			printf("Can't dele the file:%s**%d\n",file_path_tmp,__LINE__);
			return -1;
		}			
	}
	closedir(p_dir);
	return 0;
}





int32_t remove_oldest_file( char *  record_file_root_dir)   // /mnt/VIDEO  里面查找文件夹，删除  录像文件      
{	

	if(NULL ==  record_file_root_dir){
		printf("The record file dir root is NULL !%d\n",__LINE__);
		return -1;
	}

	DIR * p_dir = NULL;
	struct dirent *p_dirent =NULL;
	int32_t t_smaller_file_name = 0 ;
	int32_t t_back_file_name = 0;
	int32_t t_smaller = 0;
		

	char  record_file_path_tmp[100];


	if(NULL == (p_dir = opendir(record_file_root_dir))){
		printf("Can't open the dir !%d\n",__LINE__);
		return -1;
	}

	while(  NULL != (p_dirent = readdir(p_dir))){
		if(NULL == p_dirent->d_name ){
			continue ;
		}
		
		if(p_dirent->d_name == "." || p_dirent->d_name == ".."){
			continue;
		}

		t_back_file_name = atoi(p_dirent->d_name);

		if(t_back_file_name <= t_smaller_file_name){
			t_smaller = t_back_file_name;
			memset(record_file_path_tmp,0,sizeof(record_file_path_tmp));
			sprintf(record_file_path_tmp,"%s/%s",record_file_root_dir,p_dirent->d_name);
		}
	}
	
	if(0 != sd_file_remove(record_file_path_tmp)){
		printf("Can't remove the dir!%d\n",__LINE__);
		return -1;
	}
	
	return 0;
}

 
int32_t remove_oldest_image_file(char *image_file_root_dir)   //删除最老的图像文件   可合并   1-image   strtok_r()   有添加txt索引的话要过滤
{

	if(NULL ==  image_file_root_dir){
		printf("The image file dir root is NULL !%d\n",__LINE__);
		return -1;
	}

	DIR * p_dir = NULL;
	struct dirent *p_dirent =NULL;
	int32_t t_smaller_file_name = 0 ;
	int32_t t_back_file_name = 0;

	char *token = NULL;
	char *image_file_num = NULL;
		

	char  image_file_path_tmp[100];


	if(NULL == (p_dir = opendir(record_file_root_dir))){
		printf("Can't open the dir !%d\n",__LINE__);
		return -1;
	}

	while(  NULL != (p_dirent = readdir(p_dir))){
		if(NULL == p_dirent->d_name ){
			continue ;
		}
		
		if(p_dirent->d_name == "." || p_dirent->d_name == ".."){
			continue;
		}

		image_file_num = strtok_r(p_dirent->d_name,"-",&token);
		
		t_back_file_name = atoi(image_file_num);

		if(t_back_file_name <= t_smaller_file_name){
			t_smaller_file_name = t_back_file_name;
			memset(image_file_path_tmp,0,sizeof(image_file_path_tmp ));
			sprintf( image_file_path_tmp,"%s/%s",record_file_root_dir,p_dirent->d_name);
		}
	}
	
	if(0 != remove( image_file_path_tmp)){
		printf("Can't remove the dir!%d\n",__LINE__);
		return -1;
	}
	
	return 0;
}


/*
  int64_t get_disk_free_size(char * sd_root_path , int32_t *bavail, int32_t *bsize ) //在initmanager中调用
{	
	struct statfs disk_statfs;
	while(-1 == statfs(sd_root_path,&disk_statfs))){
		if(  strerror(errno) != EINTR){
			printf("Statfs erro!%d\n",__LINE__);
			return -1;
		}
	}
	*bavail = disk_statfs.f_bavail;
	*bsize = disk_statfs.f_bsize;
	
	return ( int64_t )(disk_statfs.f_bavail) * ( int64_t)(disk_statfs.f_bsize)/1000000;
}
*/

////////////////////////////////

int64_t get_disk_free_size(char * sd_root_path , int32_t *bavail, int32_t *bsize ) //在initmanager中调用
{   
	struct statfs disk_statfs;
	if(statfs(sd_root_path,&disk_statfs) >= 0 ){
		
		*bavail = disk_statfs.f_bavail;
		*bsize = disk_statfs.f_bsize;
		
		return ( int64_t )(disk_statfs.f_bavail) * ( int64_t)(disk_statfs.f_bsize)/1000000;
	}

	return -1;
  }




  
/*
static T_S32 make_record_path( T_pSTR pstrRecPath )
//创建文件路径，将文件名添加到原本路径后面，创建文件后的操作.简便的话可以调用系统函数
//可以创建文件夹，以日为划分，2013.10.20，一天一个文件夹那种.
{	
	struct tm * record_time ;
	char str_record_date[20];
	DIR *pdir = NULL;
	struct dirent *pdirent = NULL;
	struct stat stat_buf = NULL; 

	char dir_path[40] ;
	
	
	record_time = get_record_time(record_buffer);
	bzero(str_record_date,sizeof(str_record_date));
	
	sprintf(str_record_date,"%4d.%02d%.02d%",1900+record_time->tm_year,
		record_time->tm_mon+1,record_time->tm_mday);

	if(NULL == (pdir = opendir(record_file_path))){
		printf("Open the recording dir erro!%d\n ",__LINE__);
		return -1;	
	}
	while(NULL != (pdirent = readdir(pdir))){
		if(0 == strcmp(str_record_date ,pdirent->d_name ) ){
			bzero(dir_path,sizeof(dir_path));
			sprintf(dir_path,"%s/%s",record_file_path,pdirent->d_name);
			
			
			if(NULL == (stat_buf = lstat(dir_path))){
				continue;
				}
			if(S_ISDIR(stat_buf.st_mode)){
				printf("The dir exists,no need to creat!%d\n",__LINE__);
				return 0;
				}
			}
		}
		bzero(dir_path,sizeof(dir_path));
		sprintf(dir_path,"%s/%s",record_file_path,pdirent->d_name);
		
		if ( mkdir( dir_path, S_IRWXU | S_IRWXG | S_IRWXO ) != 0 ){
			printf("Can't creat the dir!%d\n",__LINE__)
			return -1;
			}
		bzero(record_file_path,sizeof(record_file_path);
		strcpy(record_file_path,dir_path);	
	return 0;
}
*/
 int32_t make_new_record_dir( char * record_file_root_dir)
//创建一个文件目录，hour    、mnt/VIDEO        /0    目录为0~2^32       T_U32   record_file_root_dir  调用的是全局变量，可以不需要参数
{

	if(NULL == record_file_root_dir){
		printf("The record_file_root_dir is NULL !%d\n",__LINE__);
		return -1;
	}

	DIR *p_dir = NULL;
	struct dirent *p_dirent = NULL;
	struct stat stat_buf ;
	char dir_path[100];

    uint32_t record_dir_num =0;
	uint32_t dir_num = 0;
	uint32_t bigger_dir_num = 0;
	
	memset(&stat_buf,0,sizeof(stat_buf));


	if(NULL == (p_dir =opendir(record_file_root_dir)) ){
		printf("Can't open the record_file_root_dir:%s**%d !\n",record_file_root_dir,__LINE__);
		return -1;
	}

	while(NULL != (p_dirent =readdir(p_dir))){
		if(NULL == p_dirent->d_name ){
			continue;
		}
		if(p_dirent->d_name == "." || p_dirent->d_name == ".."){
			continue;
		}
		memset(dir_path,0,sizeof(dir_path));
		sprintf(dir_path,"%s/%s",record_file_root_dir,p_dirent->d_name);

		if(0 != lstat(dir_path,&stat_buf)){
			printf("Can't get the dir stat!%d\n",__LINE__);
			return -1;
		}

		if(!S_ISDIR(stat_buf.st_mode)){
			continue;
		}

		dir_num = atoi(p_dirent->d_name);
		if(dir_num >= bigger_dir_num){
			bigger_dir_num = dir_num;
		}
	}
	
	record_dir_num = bigger_dir_num + 1;
	memset(dir_path,0,sizeof(dir_path));
	sprintf(dir_path,"%s/%d",record_file_root_dir,record_dir_num);
	
	if(0 != mkdir( dir_path, S_IRWXU | S_IRWXG | S_IRWXO )){
		printf("Can't creat the dir!%d\n",__LINE__);
		return -1;
	}
	strcpy(record_file_path,dir_path);
	return 0;
}


int32_t make_new_file_path(char * file_name)//在文件名创建后，生成文件路径
{
	
	if(NULL == file_name){
		printf("The file name is NULL!%d",__LINE__);
		return -1;
	}

	strcat(record_file_path,file_name);
	return 0;
}


/*

 void make_file_name()        //在GetRecordFile获取文件函数中被调用,生成一个完整的文件名 调用get_record_time
{ //get_record_time

///格式  "%s%4d%02d%02d%02d%02d%02d%s"      1900 + tnow->tm_year, tnow->tm_mon + 1, tnow->tm_mday, tnow->tm_hour, 
///		 tnow->tm_min, tnow->tm_sec, FILE_SUFFIZ
	//在第一次的时候调用，获取缓冲区中第一帧的头，时间
	char record_file_name[40];
	char record_file_path_complete[100];
	FILE record_fd;

	struct tm *t_time = get_record_time(record_buffer);

	bzero(record_file_name,sizeof(record_file_name));
	bzero(record_file_path_complete,sizeof(record_file_path_complete));
	
	sprintf(record_file_name,"%s.%4d.%02d.%02d:%02d:%02d",1900+t_time->tm_year,t_time->tm_mon+1,
		t_time->tm_mday,t_time->tm_hour,t_time->tm_min,t_time->sec);
	sprintf(record_file_path_complete,"%s/%s",record_file_path,record_file_name);

	
	if(NULL == (record_fd = fopen(record_file_path_complete,"wb+")){
		printf("Can't open the file %d\n",__LINE__);
		return NULL;
	}
	fclose(record_fd);

	return record_file_path_complete;
}

*/
char *  make_file_name( char *record_file_dir)//创建文件名0~~~2^64   录像文件名       
{
	if(NULL ==  record_file_dir){
		printf("The record_file_root_dir is NULL !%d\n",__LINE__);
		return -1;
	}

	DIR *p_dir = NULL;
	struct dirent *p_dirent = NULL;
	struct stat stat_buf;
	char file_path[100];
	FILE *record_file_fd = NULL;

    uint32_t record_file_num =0;
	uint32_t file_num = 0;
	uint32_t bigger_file_num = 0;

	memset(&stat_buf,0,sizeof(stat_buf));
	if(NULL == (p_dir =opendir( record_file_dir)) ){
		printf("Can't open the record_file_root_dir:%s**%d !\n", record_file_dir,__LINE__);
		return -1;
	}

	while(NULL != (p_dirent =readdir(p_dir))){
		if(NULL == p_dirent->d_name ){
			continue;
		}
		if(p_dirent->d_name == "." || p_dirent->d_name == ".."){
			continue;
		}
		memset(file_path,0,sizeof(file_path));
		sprintf(file_path,"%s/%s", record_file_dir,p_dirent->d_name);

		if(0 != lstat(file_path,&stat_buf)){
			printf("Can't get the dir stat!%d\n",__LINE__);
			return -1;
		}

		if(!S_ISDIR(stat_buf.st_mode)){
			continue;
		}

		file_num = atoi(p_dirent->d_name);
		if(file_num >= bigger_file_num){
			bigger_file_num = file_num;
		}
	}
	
	record_file_num  = bigger_file_num + 1;
	memset(file_path,0,sizeof(file_path));
	sprintf(file_path,"%s/%d", record_file_dir,record_file_num);
	

	if(NULL == ( record_file_fd = fopen(file_path,"wb+"))){
		printf("Can't open the file %d\n",__LINE__);
		return -1;
	}
	closedir(p_dir);
	fclose(record_file_fd);
	return file_path;
}



char *  make_image_file_name(char *image_file_root_dir)//创建图像文件名     0-image   1-image   后面的图像标志     函数可以合并，添加启动标志
 {         
 //int32_t is_record_flag 
 //以数量控制，文件量，目录下还有一个索引的txt文件，删除文件的时候也应该删掉，在创建文件的时候要添加进去。


	if(NULL == image_file_root_dir ){
			printf("The record_file_root_dir is NULL !%d\n",__LINE__);
			return -1;
		}
	
		DIR *p_dir = NULL;
		struct dirent *p_dirent = NULL;
		struct stat stat_buf ;
		char file_path[100];
		FILE *record_file_fd = NULL;
		uint32_t total_num = 0;
	
		uint32_t record_file_num =0;
		uint32_t file_num = 0;
		uint32_t bigger_file_num = 0;
	
		memset(&stat_buf,0,sizeof(stat_buf));
		if(NULL == (p_dir =opendir( image_file_root_dir ))){
			printf("Can't open the record_file_root_dir:%s**%d !\n",image_file_root_dir ,__LINE__);
			return -1;
		}
	
		while(NULL != (p_dirent =readdir(p_dir))){
			if(NULL == p_dirent->d_name ){
				continue;
			}
			if(p_dirent->d_name == "." || p_dirent->d_name == ".."){
				continue;
			}
   			memset(file_path,0,sizeof(file_path));
			sprintf(file_path,"%s/%s",image_file_root_dir ,p_dirent->d_name);
			
			if(0 != lstat(file_path,&stat_buf)){
				printf("Can't get the dir stat!%d\n",__LINE__);
				return -1;
			}
	
			if(!S_ISDIR(stat_buf.st_mode)){
				continue;
			}
	
			file_num = (uint32_t)atoi(p_dirent->d_name);
			if(file_num >= bigger_file_num){
				bigger_file_num = file_num;
			}
			total_num +=1;
		}
		
		if(total_num >= (uint32_t)image_max_num){
			if(0 != remove_oldest_image_file(image_file_root_dir ))
				{	
					printf("remove the image file erro!%d\n",__LINE__);
					return -1;
				}
		}
		
		record_file_num  =  bigger_file_num + 1;
		memset(file_path,0,sizeof(file_path));
		sprintf(file_path,"%s/%d-image",image_file_root_dir ,record_file_num);
		
	
		if(NULL == ( record_file_fd = fopen(file_path,"wb+"))){
			printf("Can't open the file %d\n",__LINE__);
			return -1;
		}
		fclose(record_file_fd);
		closedir(p_dir);
		return file_path;

}

  int32_t manage_record_file( )//在InitManager中被调用，还有RunManagerCyc中调用。
{
	//
	//GetDiskFreeSize( g_RecPath, &avial, &bsize );
	int64_t  sd_disk_free_size = 0;
	int64_t need_disk_size = max_file_size;
	int32_t avial,bsize;
	int32_t ret = 0;
	
	get_disk_free_size(record_file_path,&avial,&bsize);
	sd_disk_free_size = ( int64_t)( uint32_t)(avial) * (int64_t)( uint32_t)(bsize);
	if(sd_disk_free_size < 0){
		printf("get disk  %s free size error!%d \n ",record_file_path,__LINE__);
		return -1;
	}
	while((sd_disk_free_size - preservation_dise_size) < need_disk_size ){
		ret = remove_oldest_file(record_file_path);
		if( 0 != ret ){
			printf("sd_record_manager: remove_oldest_file error!%d\n",__LINE__);
			return -1;
			}
		get_disk_free_size(record_file_path,&avial,&bsize);
		sd_disk_free_size = ( int64_t)(uint32_t)(avial) * (int64_t)(uint32_t)(bsize);
		if(sd_disk_free_size < 0){
		printf("get disk  %s free size error!%d \n ",record_file_path,__LINE__);
		return -1;
		}
		sprintf("Free space :%lld,Preservation_dise_size:%lld,Need size:%lld ***%d\n",
			sd_disk_free_size,preservation_dise_size,need_disk_size);
	}
	return 0;
}



/*
static char * thread_begin( char * user )//多路录像     多用户  对于录像写入，单独开辟一个线程去进行写入操作。
{

//	ManageCycRecordFile(  , );  //对写入之前进行空间的剩余处理  

	manage_record_file();
	return NULL;
}

*/

FILE * get_record_file(char * file_name)       // 调用录像文件，返回被打开的fd文件       属于外部接口
{

//MakeFileName() 
	FILE *fp ;
	char *pstr_file_name_path = NULL;

	pstr_file_name_path = make_file_name(record_file_dir);
	if(NULL == pstr_file_name_path){
	//	free(pstr_file_name);
		return -1;
	}

	fp = fopen(pstr_file_name_path,"wb+");
	
	if(NULL != file_name)
		strcpy(file_name,pstr_file_name_path);
	
	return fp;
}



