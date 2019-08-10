

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#ifndef DIRECT_IO_H_
#define DIRECT_IO_H_
#ifdef __cplusplus
extern "C" {
#endif


typedef struct DIRECT_IO_HDC {
	int fID;

	void *cache;
	int cacheMax;
	int cacheSeen;
	int cacheOffset;
	int mode;     // 0 表示read   ；  1 表示write；

}ST_DIRECT_IO_HDC, *LP_DIRECT_IO_HDC;


extern LP_DIRECT_IO_HDC direct_io_open(const char* path, int cacheMax,int  mode);
extern void direct_io_close(void *fID);

extern ssize_t direct_io_fread(void *fID, void* buff, ssize_t size);
extern ssize_t direct_io_fwrite(void *fID, void* buff, ssize_t size);

extern int direct_io_fflush(void *fID);

extern int direct_io_fseek(void *fID, off_t offset, int whence);
extern int direct_io_ftell(void *fID);


#ifdef __cplusplus
}
#endif
#endif //DIRECT_IO_H_

