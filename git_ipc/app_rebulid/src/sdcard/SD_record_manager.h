#include <stdio.h>

#ifndef SD_RECORD_MANAGER_H_
#define SD_RECORD_MANAGER_H_


#ifdef __cplusplus
extern "C" {
#endif




extern  FILE    get_record_file(T_pSTR file_name);
extern  int32_t ini_sd_dir_creat(char * sd_root_path);
extern  int64_t get_disk_size( char * sd_root_path);
extern  int32_t is_new_creat_dir();
extern  int64_t get_old_files_size( char * p_record_file_root );
extern  int32_t remove_oldest_file( char *  record_file_root_dir) ;
extern  int32_t remove_oldest_image_file(char *image_file_root_dir) ;
extern  int64_t get_disk_free_size(char * sd_root_path , int32_t *bavail, int32_t *bsize );
extern  int32_t manage_record_file( );






#ifdef __cplusplus
};
#endif
#endif 




