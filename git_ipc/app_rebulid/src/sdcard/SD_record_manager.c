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
//#define preservation_dise_size (67108864);//Ԥ���ռ� 64M
//#define image_max_num   (200);


	const int32_t max_file_size = 67108864;
	const int32_t image_max_num = 200;
	const int64_t preservation_dise_size = 67108864;

	const  char sd_root_path[100] ="/mnt";         //�̶�ֵ sd ����Ŀ¼    
	const char record_file_root_dir[100] = "/mnt/VIDEO";//�̶�ֵ  ����hourʱ���л���ʱ��ʹ��
	const char image_file_root_dir[100] = "/mnt/IMAGE";//�̶�ֵ��ͼ��Ŀ¼


	char  record_file_dir[100]  = "";   //¼����ϲ�Ŀ¼     /mnt/VIDEO/1             Ŀ¼1     
	char  record_file_path[100] = "/mnt/VIDEO";//�ɽ��г�ֵ�趨,����д����̸ı䣬��/mnt/VIDEO/1



	char file_name[50] = "";   //tmp

	char index_file_path[50] = "";//Ŀ¼�е�txt�ļ�������ַ��




	int32_t time_change_flag = 0;  //ʱ��hour�ı䣬���ߵ���������������������裬����Ҫ�½�Ŀ¼����־   1����Ҫ�ؽ���   0������Ҫ
	char current_time_hour[50];//��ȡ��ǰϵͳʱ�䣬���֣���ȷ��hour��Ҫ�ǲ�һ���͸��£����½�Ŀ¼
	//T_U32 record_file_num = 0;










//��һ���������ж�Ŀ¼�Ƿ����
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

int32_t add_entry_to_txt_file(char *info_record_file, char *index_file_path)//��info_record_file�ļ���Ϣд��
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



int32_t make_txt_index(char * record_file_dir)//����һ�������ļ�.txt�ļ�  �ڴ���Ŀ¼��ʱ�����һ��
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




//������������Ŀ¼�µ�¼���ļ���video��ͼ���ļ��У�or  exe   database
int32_t ini_sd_dir_creat(char * sd_root_path){

	if(NULL == sd_root_path){
		strcpy( sd_root_path,"/mnt");	
	}
	char dir_tmp[100];


//����           /mnt/VIDEO            Ŀ¼	    ��ʼ����ʱ��Ŀ¼�����ھʹ�����
	
	memset(dir_tmp,0,sizeof(dir_tmp));
	sprintf(dir_tmp,"%s/%s",sd_root_path,"VIDEO");
	if(0 != is_exists(dir_tmp)){
		if(0 != mkdir(dir_tmp,S_IRWXU | S_IRWXG | S_IRWXO)){
			printf("Can't creat the direction!%d\n",__LINE__);
			return -1;
		}
	}
	


//����            /mnt/IMAGE            ͼ��Ŀ¼
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

int64_t get_disk_size( char * sd_root_path)   //���ļ�Ŀ¼������ȡϵͳ���̴�Сrecord_root_path
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




struct tm * get_record_time(char * pstr_data)//��ȡ¼���������֡�е�ͷʱ��     ���Բ���Ҫ  keep   ɾ��
{
	struct tm* t_record_time;
	//strtok_r (record_buffer,'.',token);
	
	
	return t_record_time;
}



int32_t  is_new_creat_dir()//�ж�ʱ�䵽hour�Ƿ�ı䣬
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



 int32_t is_record_file(char *file_name)//�����������ж��Ƿ���¼���ļ�����׺������      keep         ����ɾ��
{

	return 0;
}

 int64_t get_old_files_size( char * p_record_file_root )
{
//��ȡ�Ѵ�¼���ļ��Ĵ�С�����ռ�ʹ��ռ�ô�С, ����  record_file_root_dir   /mnt/VIDEO�е��ļ�
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
	//�ж��Ƿ���¼���ļ�    is_record_file()
	
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
	//ѭ�������ҵ����ϵ��Ǹ��ļ���ͨ���ļ�����״̬����lstat�� �е�st_time������
	//����1700���������ԽС˵���ļ�Խ�ϣ�ѭ���滻����,�ͷ�һЩ�ռ�
	
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


static int32_t sd_file_remove( char * dir_path)// �Ȱ�Ŀ¼�µ��ļ�ɾ������ɾ��Ŀ¼
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





int32_t remove_oldest_file( char *  record_file_root_dir)   // /mnt/VIDEO  ��������ļ��У�ɾ��  ¼���ļ�      
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

 
int32_t remove_oldest_image_file(char *image_file_root_dir)   //ɾ�����ϵ�ͼ���ļ�   �ɺϲ�   1-image   strtok_r()   �����txt�����Ļ�Ҫ����
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
  int64_t get_disk_free_size(char * sd_root_path , int32_t *bavail, int32_t *bsize ) //��initmanager�е���
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

int64_t get_disk_free_size(char * sd_root_path , int32_t *bavail, int32_t *bsize ) //��initmanager�е���
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
//�����ļ�·�������ļ�����ӵ�ԭ��·�����棬�����ļ���Ĳ���.���Ļ����Ե���ϵͳ����
//���Դ����ļ��У�����Ϊ���֣�2013.10.20��һ��һ���ļ�������.
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
//����һ���ļ�Ŀ¼��hour    ��mnt/VIDEO        /0    Ŀ¼Ϊ0~2^32       T_U32   record_file_root_dir  ���õ���ȫ�ֱ��������Բ���Ҫ����
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


int32_t make_new_file_path(char * file_name)//���ļ��������������ļ�·��
{
	
	if(NULL == file_name){
		printf("The file name is NULL!%d",__LINE__);
		return -1;
	}

	strcat(record_file_path,file_name);
	return 0;
}


/*

 void make_file_name()        //��GetRecordFile��ȡ�ļ������б�����,����һ���������ļ��� ����get_record_time
{ //get_record_time

///��ʽ  "%s%4d%02d%02d%02d%02d%02d%s"      1900 + tnow->tm_year, tnow->tm_mon + 1, tnow->tm_mday, tnow->tm_hour, 
///		 tnow->tm_min, tnow->tm_sec, FILE_SUFFIZ
	//�ڵ�һ�ε�ʱ����ã���ȡ�������е�һ֡��ͷ��ʱ��
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
char *  make_file_name( char *record_file_dir)//�����ļ���0~~~2^64   ¼���ļ���       
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



char *  make_image_file_name(char *image_file_root_dir)//����ͼ���ļ���     0-image   1-image   �����ͼ���־     �������Ժϲ������������־
 {         
 //int32_t is_record_flag 
 //���������ƣ��ļ�����Ŀ¼�»���һ��������txt�ļ���ɾ���ļ���ʱ��ҲӦ��ɾ�����ڴ����ļ���ʱ��Ҫ��ӽ�ȥ��


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

  int32_t manage_record_file( )//��InitManager�б����ã�����RunManagerCyc�е��á�
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
static char * thread_begin( char * user )//��·¼��     ���û�  ����¼��д�룬��������һ���߳�ȥ����д�������
{

//	ManageCycRecordFile(  , );  //��д��֮ǰ���пռ��ʣ�ദ��  

	manage_record_file();
	return NULL;
}

*/

FILE * get_record_file(char * file_name)       // ����¼���ļ������ر��򿪵�fd�ļ�       �����ⲿ�ӿ�
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



