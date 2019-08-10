/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : direct_io.h
  Version       : Initial Draft
  Author        : Law
  Created       : 2011/6/7
  Last Modified :
  Description   : direct io routines declarations
  Function List :
  History       :
  1.Date        : 2011/6/7
    Author      : Law
    Modification: Created file

******************************************************************************/

#ifndef __DIRECT_IO_H__
#define __DIRECT_IO_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct DIRECT_IO_HDC {
	int fID;

	int bufMax;
	char *buf;
	int writeBufOff, readBufOff;
	int readBufSeen;

}ST_DIRECT_IO_HDC, *LP_DIRECT_IO_HDC;

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

extern int direct_io_open(const char* path, const char* mode);
extern void direct_io_close(int fd);

extern int direct_io_read(int fd, void* buff, ssize_t size);
extern int direct_io_write(int fd, void* buff, ssize_t size);
extern int direct_io_fsync(int fd);

extern int direct_io_seek(int fd, off_t offset, int whence);
extern int direct_io_tell(int fd);

#define DIRECT_IO_BUFFER_SZ (1024 * 1024)


#ifdef __cplusplus
}
#endif
#endif /* __DIRECT_IO_H__ */

