/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : direct_io.c
  Version       : Initial Draft
  Author        : Law
  Created       : 2011/6/4
  Last Modified :
  Description   : direct i/o file read / wirte routinues
  Function List :
  History       :
  1.Date        : 2011/6/4
    Author      : Law
    Modification: Created file

******************************************************************************/

#if 1


#define _GNU_SOURCE
#define __USE_XOPEN2K

#include "direct_io.h"
#include <sys/types.h>
#include <assert.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include "generic.h"
#include "frank_trace.h"


/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/


#define DIRECT_IO_WRITE_FILED ('W')
#define DIRECT_IO_READ_FILED ('R')

//#define TEST_DIO
#define DEBUG_DIO
#ifdef DEBUG_DIO
#define DIO_TRACE(fmt...) \
	do{printf("\033[1;DIRECTIO->[%s]:%d ", __FUNCTION__, __LINE__);printf(fmt);printf("\033[m\r\n");}while(0)
#else
#define DIO_TRACE(fmt...)
#endif


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

// aligned buffer address to a page boundard
typedef char ALIGNED_U8 __attribute__((aligned(4096)));

typedef struct _DirectIOBuf{
	unsigned char write_buf[DIRECT_IO_BUFFER_SZ]; // 1st position is very important
	unsigned char read_buf[DIRECT_IO_BUFFER_SZ]; // 2nd position is very important
}DirectIOBuf __attribute__((aligned(4096)));

static DirectIOBuf s_rw_buf;

static int read_fd = -1;
static int write_fd = -1;

static char write_file[128] = {""};
//static ALIGNED_U8 write_buffer[DIRECT_IO_BUFFER_SZ];
static int write_pos = 0;

static char read_file[128] = {""};
//static ALIGNED_U8 read_buffer[DIRECT_IO_BUFFER_SZ];
static int read_buffsize = 0;
static int read_pos = 0;

//ALIGNED_U8 tmp_buf [DIRECT_IO_BUFFER_SZ];

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/



/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


static int read_from_file(int fd, void* buff, ssize_t size)
{
	int read_size = 0;
	read_size = read(fd, buff, size);
	return read_size;
}

static int read_from_buffer(void* buff, ssize_t size)
{
	if(!(read_pos + size < read_buffsize)){
		size = read_buffsize - read_pos;
	}
	//DIO_TRACE("copy %p size = %d", buff, size);
	memcpy(buff, &(s_rw_buf.read_buf[read_pos]), size);
	read_pos += size;
	return size;
}

static int update_from_file()
{
	//DIO_TRACE("update from file %d", lseek(read_fd, 0, SEEK_CUR));
	read_buffsize = read(read_fd, s_rw_buf.read_buf, DIRECT_IO_BUFFER_SZ);
	//fsync(read_fd);
	if(read_buffsize >= 0){
		read_pos = 0;
	}
	return read_buffsize;
}

static int sync_from_file()
{
	int sync_size = 0;
	lseek(read_fd, -1 * read_buffsize - 1, SEEK_CUR);
	sync_size = read(read_fd, s_rw_buf.read_buf, DIRECT_IO_BUFFER_SZ); // sync form 0 - write_pos
	return sync_size;
}

static int write_to_buffer(void* buff, ssize_t size)
{
	if(write_pos + size > DIRECT_IO_BUFFER_SZ){
		size = DIRECT_IO_BUFFER_SZ - write_pos;
	}
	//DIO_TRACE("copy %p size = %d", buff, size);
	if(size > 0){
		memcpy(&(s_rw_buf.write_buf[write_pos]), buff, size);
		write_pos += size;
		return size;
	}
	// no bytes write to buffer
	return 0;
}

//extern int hdd_writting;
//extern time_t hdd_begin_write;
static int update_to_file()
{
	struct timeval tv_begin;
	struct timeval tv_end;
	struct timeval tv_ret;
	gettimeofday(&tv_begin, NULL);
//	hdd_writting = 1;
//	hdd_begin_write = time(NULL);
	int write_size = 0;
//	DIO_TRACE("update @ %s offset=%d", write_file, lseek(write_fd, 0, SEEK_CUR));
	write_size = write(write_fd, s_rw_buf.write_buf, DIRECT_IO_BUFFER_SZ);
	gettimeofday(&tv_end, NULL);
	timersub(&tv_end, &tv_begin, &tv_ret);
	if(tv_ret.tv_sec > 1)
	{
		char buf[64];
		sprintf(buf, "HDD:slow writting, sec=%ld", tv_ret.tv_sec);
		XLOG_add(XLOG_TYPE_WARNING, buf, tv_end.tv_sec);
	}
//	if(XLOG_ready_for_work() == 1 && write_size != DIRECT_IO_BUFFER_SZ)
//	{
//		char buf[64];
//		sprintf(buf, "HDD:Invalid size %d", write_size);
//		XLOG_add(XLOG_TYPE_WARNING, buf, time(NULL));
//	}
	write_pos = 0;
//	hdd_writting = 0;
	return write_size;
}

static int sync_to_file()
{
	int sync_size = (write_pos + 1 + 4095) & ~(4095);
	int write_size = 0;
	if(0 == write_pos){
		return 0;
	}
	write_size = write(write_fd, s_rw_buf.write_buf, sync_size); // sync form 0 - write_pos
	lseek(write_fd, -1 * write_size - 1, SEEK_CUR); // backup to original position
	return (sync_size == write_size) ? 0 : -1;

//	//当前缓冲不为空，写入当前缓冲
//	if(write_pos > 0)
//	{
//		//--读出当前缓冲
//		//--更改缓冲
//		//--写入缓冲
//		int tmp_buf_len = 0;
////		unsigned char* tmp_buf = calloc(1, DIRECT_IO_BUFFER_SZ);
//		tmp_buf_len = read(write_fd, tmp_buf, DIRECT_IO_BUFFER_SZ);
//		if(tmp_buf_len == 0)
//		{
//			write(write_fd, s_rw_buf.write_buf, DIRECT_IO_BUFFER_SZ);
//		}
//		else if(tmp_buf_len > 0 )
//		{
//			memcpy(tmp_buf, s_rw_buf.write_buf, write_pos);
//			lseek(write_fd, -1 * tmp_buf_len, SEEK_CUR);
//			write(write_fd, tmp_buf, DIRECT_IO_BUFFER_SZ);
//		}
//		else
//		{
//			perror("@@b");
//			assert(0);
//		}
//		free(tmp_buf);
//	}
	return 0;
}

static int file_to_memory(char *memBuf, int size, int fID)
{
	//DIO_TRACE("update from file %d", lseek(read_fd, 0, SEEK_CUR));
	read_buffsize = read(fID, memBuf, size);
	//fsync(read_fd);
	if(read_buffsize >= 0){
		read_pos = 0;
	}
	return read_buffsize;
}


LP_DIRECT_IO_HDC direct_io_open2(const char* path, int bufSize, const char* mode)
{
	int ret = 0;
	int fd = 0;
	LP_DIRECT_IO_HDC dIO = calloc(sizeof(ST_DIRECT_IO_HDC), 1);

	// align to 4k
	bufSize += 4 * 1024 - 1;
	bufSize /= 4 * 1024;
	bufSize *= 4 * 1024;
	
	if(strstr(mode, "r")){
		// read only
		dIO->fID = open(path, O_RDONLY | O_DIRECT);
		_ASSERT(dIO->fID > 0, "DirectIO Open File \"%s\" Failed!", path);

		// setup environment
		dIO->readBufMax = bufSize;
		dIO->readBuf = calloc(dIO->readBufMax, 1);
		dIO->readBufOff = 0;
		dIO->readBufSeen = 0;
		//dIO->writeBuf = NULL; // because of read - only
		//dIO->writeBufMax = 0;
		//dIO->writeBufOff = 0;

		// read file data to memory buffer
		dIO->readBufSeen = read(dIO->fID, dIO->readBuf, dIO->readBufMax);
		_ASSERT(dIO->readBufSeen >= 0, "DirectIO Read File \"%s\" Failed!", path);
		
	}else if(strstr(mode, "w")){
		int sync_size = 0;

		dIO->fID = open(path, O_RDWR | O_DIRECT);
		_ASSERT(dIO->fID > 0, "DirectIO Open File \"%s\" Failed!", path);

		// setup context
		dIO->readBufMax = bufSize;
		dIO->readBuf = calloc(dIO->readBufMax, 1);
		dIO->readBufOff = 0;
		dIO->readBufSeen = 0;
		dIO->writeBufMax = bufSize;
		dIO->writeBuf = calloc(dIO->writeBufMax, 1);
		dIO->writeBufOff = 0;

		// with original data
		ret = read(dIO->fID, dIO->writeBuf, dIO->writeBufMax);
		lseek(dIO->fID, 0, SEEK_SET); // reset to orignal position
		
	}else{
		errno = EINVAL;
		free(dIO);
		dIO = NULL;
	}
	return dIO;
}

int direct_io_open(const char* path, const char* mode)
{
	int ret = 0;
	int fd = 0;
	if(strstr(mode, "r")){
		if(read_fd > 0){
			errno = EPERM;
			return -EPERM;
		}
		read_fd = open(path, O_RDONLY | O_DIRECT);
		fd = DIRECT_IO_READ_FILED;
		//assert(read_fd > 0);

		// really open here and read a part of file
		ret = update_from_file();
		//assert(ret == DIRECT_IO_BUFFER_SZ);
		strcpy(read_file, path);
	}else if(strstr(mode, "w")){
		int sync_size = 0;
		if(write_fd > 0){
			errno = EPERM;
			return -EPERM;
		}
		write_fd = open(path, O_RDWR | O_DIRECT);
		fd = DIRECT_IO_WRITE_FILED;
		DIO_TRACE("open file \"%s\"", path);
		//assert(write_fd > 0);

		// with original data
		sync_size = read_from_file(write_fd, s_rw_buf.write_buf, DIRECT_IO_BUFFER_SZ);
		lseek(write_fd, 0, SEEK_SET);
		strcpy(write_file, path);
		// reset fd pos
		write_pos = 0;
	}else{
		errno = EINVAL;
		return -EINVAL;
	}
	return fd;
}

void direct_io_close(int fd)
{
	if(DIRECT_IO_READ_FILED == fd && read_fd){
		close(read_fd);
		read_fd = -1;
		strcpy(read_file, "null");
	}else if(DIRECT_IO_WRITE_FILED == fd){
		sync_to_file(); // sync buffer to file
		close(write_fd);
		write_fd = -1;
		strcpy(write_file, "null");
	}
}

int direct_io_read(int fd, void* buff, ssize_t size)
{
	int read_total = size;
	int read_size = 0;
	int update_size = 0;
	do{
		// read buffer first
		read_size = read_from_buffer((char*)buff + read_total - size, size);
		size -= read_size;
		if(size > 0){
			// update buffer from file
			update_size = update_from_file();
			if(0 == update_size){
				// to the end of file
				break;
			}
		}
	}while(size > 0);
	// return read size
	//DIO_TRACE("buffer(%d),file(%d),tell(%d)", read_pos, lseek(read_fd, 0, SEEK_CUR), direct_io_tell(fd));
	return read_total - size;
}

int direct_io_write(int fd, void* buff, ssize_t size)
{
	int write_total = size;
	int write_size = 0;
	int writen_size = 0;
	int update_size = 0;

	while(size > 0){
		write_size = write_to_buffer((char*)buff + writen_size, size);
		size -= write_size;
		writen_size += write_size;
		//DIO_TRACE("write_pos = %d", write_pos);
		if(size > 0 || DIRECT_IO_BUFFER_SZ == write_pos){
			update_size = update_to_file();
			if(0 == update_size){
				break;
			}
		}
	}
	return write_total - size;
}

int direct_io_fsync(int fd)
{
	if(DIRECT_IO_READ_FILED == fd){
		return sync_from_file();
	}else if(DIRECT_IO_WRITE_FILED == fd){
		return sync_to_file();
	}
	return -1;
}

int direct_io_seek(int fd, off_t offset, int whence)
{
	if(SEEK_SET == whence){
		// only supported
//		int file_locate = 0;
//		int file_size = 0;

		if(DIRECT_IO_WRITE_FILED == fd){

			//同步缓冲
			sync_to_file();
			//移动指针
			//--计算文件总大小
//			int file_size = lseek(write_fd, 0, SEEK_END);
//			assert(offset <= file_size);
			//--计算块索引
			int file_block_index = offset / DIRECT_IO_BUFFER_SZ;
			//--计算缓冲中的索引
			write_pos = offset % DIRECT_IO_BUFFER_SZ;
			//--读入缓冲
			lseek(write_fd, file_block_index * DIRECT_IO_BUFFER_SZ, SEEK_SET);
			int read_ret = read(write_fd, s_rw_buf.write_buf, DIRECT_IO_BUFFER_SZ);
			//assert(read_ret != -1);
			lseek(write_fd, -1 * read_ret, SEEK_CUR);

//			int sync_size = 0;
//			// backup current location
//			file_locate = lseek(write_fd, 0, SEEK_CUR);
//			// get the max file size
//			file_size = lseek(write_fd, 0, SEEK_END);
//			//DIO_TRACE("size = %d", file_size);
//			if(!(offset < file_size)){
//				// reset pointer
//				lseek(write_fd, file_locate, SEEK_SET);
//				errno = ESPIPE;
//				return -ESPIPE;
//			}
//			//update_to_file();
//			sync_to_file();
//			lseek(write_fd, offset, SEEK_SET);
//			// very important
//			sync_size = read_from_file(fd, s_rw_buf.write_buf, DIRECT_IO_BUFFER_SZ);
//			lseek(write_fd, -1 * sync_size - 1, SEEK_CUR);
//			write_pos = 0;
		}else if(DIRECT_IO_READ_FILED == fd){

			//移动指针
			//--计算文件总大小
			int file_block_index_cur = lseek(read_fd, 0, SEEK_CUR) / DIRECT_IO_BUFFER_SZ - 1;

//			int file_size = lseek(read_fd, 0, SEEK_END);
//			assert(offset <= file_size);
			//--计算块索引
			int file_block_index = offset / DIRECT_IO_BUFFER_SZ;
			//--计算缓冲中的索引
			read_pos = offset % DIRECT_IO_BUFFER_SZ;
			if(file_block_index != file_block_index_cur)
			{
				//--读入缓冲
				lseek(read_fd, file_block_index * DIRECT_IO_BUFFER_SZ, SEEK_SET);
				int read_ret = read(read_fd, s_rw_buf.read_buf, DIRECT_IO_BUFFER_SZ);
				//assert(read_ret != -1);
//				DIO_TRACE("read from file,pid=%d,file_block_index=%d,file_block_index_cur=%d"
//						, getpid(), file_block_index, file_block_index_cur);
			}


//			// backup current location
//			file_locate = lseek(read_fd, 0, SEEK_CUR);
//			// get the max file size
//			file_size = lseek(read_fd, 0, SEEK_END);
//			//DIO_TRACE("size = %d", file_size);
//			if(!(offset < file_size)){
//				// reset pointer
//				lseek(read_fd, file_locate, SEEK_SET);
//				errno = ESPIPE;
//				return -ESPIPE;
//			}
//			// seek to object position and update buffer
//			lseek(read_fd, offset, SEEK_SET);
//			update_from_file();
		}else{
			errno = EPERM;
			return -EPERM;
		}
		return 0;
	}
	errno = EPERM;
	return -EPERM;
}

int direct_io_tell(int fd)
{
	int tell = -1;
	if(DIRECT_IO_READ_FILED == fd){
		tell = lseek(read_fd, 0, SEEK_CUR);
		//assert(tell >= 0);
		tell -= read_buffsize;
		tell += read_pos;
	}else if(DIRECT_IO_WRITE_FILED == fd){
		tell = lseek(write_fd, 0, SEEK_CUR);
		//assert(tell >= 0);
		tell += write_pos;
	}
	return tell;
}


#ifdef TEST_DIO
#define TEST_COPY_SIZE (1024 * 1024)
int main(int argc, char** argv)
{
	int copy_size = 0;
	unsigned char* buffer = NULL;
	int read_size = 0;
	int nor_fd = 0;
	int dio_fd = 0;
	nor_fd = direct_io_open("test_directio.bin", "wb");
	dio_fd = direct_io_open(argv[1], "rb");
	do
	{
		copy_size = rand() % 1000 + 1024;
		buffer = calloc(1, copy_size);
		read_size = direct_io_read(dio_fd, buffer, copy_size);
		if(read_size > 0){
			direct_io_write(nor_fd, buffer, read_size);
		}
		free(buffer);
	}while(read_size > 0);

	copy_size = rand() % 1000 + 1024;
	buffer = calloc(1, copy_size);
	lseek(read_fd, 1024, SEEK_SET);
	read(read_fd, buffer, copy_size);
	direct_io_seek(nor_fd, 1024, SEEK_SET);
	direct_io_write(nor_fd, buffer, copy_size);
	free(buffer);

	direct_io_close(dio_fd);
	direct_io_close(nor_fd);
	return 0;
}
#endif /* TEST_DIO */

#endif
