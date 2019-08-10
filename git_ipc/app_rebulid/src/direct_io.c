
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "direct_io.h"
#include "generic.h"
#include "frank_trace.h"

#include <assert.h>

#include "app_debug.h"



//#include "../app/log.h"
#include <sys/time.h>



#define KDIO_ALIGNED_SIZE (4096)

//
// cache(aligned)                         cacheMax(aligned)
//   +------------------------+------------+
//   +                        +            +
//   +                        +            +
//   +                        +            +
//   +                        +            +
//   +                        +            +
//   +                        +            +
//   +------------------------+------------+
//                           cacheSeen
//




LP_DIRECT_IO_HDC direct_io_open(const char * path, int cacheMax, int mode)
{
	LP_DIRECT_IO_HDC dIO = NULL;
	if(0 != mode && 1 != mode ){
		return NULL;
		}
	printf("open***%d\n",__LINE__);
	if(cacheMax > 0){

		// cache align to a page size
		cacheMax += KDIO_ALIGNED_SIZE - 1;
		cacheMax /= KDIO_ALIGNED_SIZE;
		cacheMax *= KDIO_ALIGNED_SIZE;
		
		printf("open***%d\n",__LINE__);

		dIO = calloc(sizeof(ST_DIRECT_IO_HDC) + cacheMax + KDIO_ALIGNED_SIZE, 1); // with addtional page size for aligning
		if(NULL != dIO){
			unsigned int cacheAddress = 0;
			
			if(0 == mode){			
				dIO->fID = open(path, O_RDONLY | O_DIRECT);
			}else {
				dIO->fID = open(path, O_RDWR | O_DIRECT);
			}

			printf("open***%d\n",__LINE__);
			
			
			_ASSERT(dIO->fID > 0, "DirectIO Open File \"%s\" Failed!", path);

			dIO->cache = (typeof(dIO->cache))(dIO + 1);
			cacheAddress = (unsigned int)(dIO->cache);
			// algin cache
			cacheAddress += KDIO_ALIGNED_SIZE - 1;
			cacheAddress /= KDIO_ALIGNED_SIZE;
			cacheAddress *= KDIO_ALIGNED_SIZE;
			dIO->cache = (LP_DIRECT_IO_HDC)cacheAddress;

			_TRACE("Cache Offset = %p/%p", dIO->cache, (dIO + 1));

			// other element
			dIO->cacheMax = cacheMax;
			dIO->cacheSeen = 0;
			dIO->cacheOffset = 0;
			dIO->mode = mode;    //表示创建的cache是read or write 
			
			printf("open***%d\n",__LINE__);

			// read the cache from file at first time
			dIO->cacheSeen = read(dIO->fID, dIO->cache, dIO->cacheMax);   //从fid读到缓冲区上，数量cacheMax
			if(dIO->cacheSeen < 0){             //偏移量
				direct_io_close(dIO);
				dIO = NULL;
			}
		}
		printf("open***%d\n",__LINE__);
		return dIO;
	}
	return NULL;
}



void direct_io_close(void *fID)
{
	LP_DIRECT_IO_HDC dIO = (LP_DIRECT_IO_HDC)(fID);
	if(NULL != dIO){
		// close the file description
		close(dIO->fID);
		dIO->fID = -1;
		free(dIO);
		dIO = NULL;
	}
}

static int  read_from_cache(LP_DIRECT_IO_HDC dIO,void *buff,ssize_t size){
	if( size >  dIO->cacheSeen - dIO->cacheOffset ){
		size = dIO->cacheSeen - dIO->cacheOffset;
	}
	memcpy(buff, (dIO->cache + dIO->cacheOffset), size);
	dIO->cacheOffset += size;
	
	printf("%d***size=%d\n",__LINE__,size);
	
	return size;
}

static int update_from_file(LP_DIRECT_IO_HDC dIO){
	dIO->cacheSeen = read(dIO->fID, dIO->cache, dIO->cacheMax);   //dIO->cacheMax
	
	printf("%d***cacheSeen=%d\n",__LINE__,dIO->cacheSeen);//test
	
	if(dIO->cacheSeen >= 0){
		dIO->cacheOffset = 0;
		}
	return dIO->cacheSeen;
}


ssize_t direct_io_fread(void *fID, void* buff, ssize_t size){
	LP_DIRECT_IO_HDC dIO = (LP_DIRECT_IO_HDC) (fID);
	int read_total = size;
	int read_size = 0;
	int update_size = 0;


	APP_TRACE("fread**%d\n",size);

	if(NULL != dIO){
		do{
			//read first
			read_size = read_from_cache(dIO,(char *)buff + read_total - size ,size);
			size -= read_size;
			
			printf("fread***%d\n",__LINE__);
			
			if(size > 0 ){
				printf("fread***%d\n",__LINE__);
				//update cache from file
				update_size = update_from_file(dIO);
				printf("fread***%d\n",__LINE__);
				if(0 == update_size){
					//to the end of file
					break;
					}
				}
		}while(size > 0);
		// return read size
		printf("fread***%d\n",__LINE__);
		return read_total - size;

	}

	return -1;
}






static int write_to_cache (LP_DIRECT_IO_HDC dIO, void *buff , ssize_t size){
	APP_TRACE("write to cache  size = %ld",size);
	APP_TRACE("write to cache  cacheSeen = %ld",dIO->cacheSeen );
	APP_TRACE("write to cache  cacheMax = %ld",dIO->cacheMax);
	if((dIO->cacheOffset + size ) > dIO->cacheMax ){    //dIO->cacheMax
		size = dIO->cacheMax - dIO->cacheOffset;
		APP_TRACE("write to cache  size = %ld",size);		
	}
	
	APP_TRACE("write to cache  size = %ld",size);
	
	if(size > 0){
		memcpy((dIO->cache + dIO->cacheOffset) , buff, size );
		dIO->cacheOffset += size;
		APP_TRACE("write to cache**dIO->cacheOffset:%ld\n",dIO->cacheOffset);
		return size;
	}
	return 0;
}

static int  update_to_file(LP_DIRECT_IO_HDC dIO){
	struct timeval tv_begin;
	struct timeval tv_end;
	struct timeval tv_ret;
	gettimeofday(&tv_begin, NULL);

	int write_size = 0;
	write_size = write(dIO->fID,dIO->cache,dIO->cacheMax);

	APP_TRACE("update_to_file***write size = %ld\n",write_size);
	
	gettimeofday(&tv_end, NULL);
	timersub(&tv_end, &tv_begin, &tv_ret);
	if(tv_ret.tv_sec > 1)
	{
		char buf[64];
		sprintf(buf, "HDD:slow writting, sec=%ld", tv_ret.tv_sec);
	//	XLOG_add(XLOG_TYPE_WARNING, buf, tv_end.tv_sec);   
		printf("%s",buf);
	}
	dIO->cacheOffset = 0;
	return write_size;

}

ssize_t direct_io_fwrite(void *fID, void* buff, ssize_t size)            // write 从buff写入到fid里面去，
{
// 将buff的数据搞到cache上面。再写入fid里面   fd
	LP_DIRECT_IO_HDC dIO = (LP_DIRECT_IO_HDC)(fID);
	APP_TRACE("direct_io_fwrite****");

	if(NULL != dIO){
		// FIXME:
		int write_total = size;
		int write_size = 0;
		int writen_size = 0;
		int update_size = 0;
		APP_TRACE("fwrite  **size=%ld\n**",size);
		
		while(size > 0){
			write_size  = write_to_cache(dIO,(char *)buff + writen_size,size);
			
			APP_TRACE("write_size =%ld  ****\n",write_size);
			
			size -=write_size;
			writen_size += write_size;
			
			APP_TRACE("writen_size =%d  ****\n",writen_size);

			if(size > 0 || dIO->cacheOffset == dIO->cacheMax){
				update_size = update_to_file(dIO);

				APP_TRACE("update_size =%d  ****\n",update_size);
				
				if( 0 == update_size ){
				break;
				}
			}
		}
		return write_total - size;
	}
	return -1;
}

int direct_io_fflush(void *fID)
{
	LP_DIRECT_IO_HDC dIO = (LP_DIRECT_IO_HDC)(fID);
	if(NULL != dIO){
		// write back to file
		if(lseek(dIO->fID, -1 * dIO->cacheSeen, SEEK_CUR) >= 0){
			if(dIO->cacheMax == write(dIO->fID, dIO->cache, dIO->cacheMax)){ // cacheMax is aligend
				return 0;
			}
		}
	}
	return -1;
}



static int sync_to_file(LP_DIRECT_IO_HDC dIO){
	int sync_size = (dIO->cacheOffset + 1 + 4095) & ~(4095);
	int write_size = 0;
	if(0 == dIO->cacheOffset){
		return 0;
	}
	write_size = write(dIO->fID, dIO->cache, sync_size);// sync form 0 - write_pos
	lseek(dIO->fID, -1 * write_size - 1, SEEK_CUR);// backup to original position
	return (sync_size == write_size) ? 0 : -1;
}



int direct_io_fseek(void *fID, off_t offset, int whence)
{
	LP_DIRECT_IO_HDC dIO = (LP_DIRECT_IO_HDC)(fID);
	if(NULL != dIO){
		// FIXME:
		if(SEEK_SET == whence){
			if(dIO->mode == 1){// write mode 
				sync_to_file(dIO);
				//--计算块索引
				int file_block_index = offset / dIO->cacheMax;
				
				//--计算缓冲中的索引
				dIO->cacheOffset = offset % dIO->cacheMax ;
				//--读入缓冲
				lseek(dIO->fID, file_block_index * dIO->cacheMax  , SEEK_SET);
				int read_ret = read(dIO->fID, dIO->cache, dIO->cacheMax );
				assert(read_ret != -1);
				lseek(dIO->fID, -1*read_ret, SEEK_CUR);
			}
			else if (dIO->mode == 0){
				int file_block_index_cur = lseek(dIO->fID, 0, SEEK_CUR) / dIO->cacheMax  -1;
				//--计算块索引
				int file_block_index = offset / dIO->cacheMax ;
				//--计算缓冲中的索引
				dIO->cacheOffset = offset % dIO->cacheMax ;
				if(file_block_index_cur != file_block_index ){
					//--读入缓冲
					lseek(dIO->fID, file_block_index * dIO->cacheMax , SEEK_SET);
					int read_ret = read(dIO->fID, dIO->cache, dIO->cacheMax );
					assert(read_ret != -1 );
				}
				
			}
		}
			return 0;
			
	}
	return -1;
}


int direct_io_ftell(void *fID){
	LP_DIRECT_IO_HDC dIO = (LP_DIRECT_IO_HDC)(fID);
	int offset = -1;
	if(NULL != dIO ){
		if(dIO->mode == 0){   //read mode
			int offset = lseek(dIO->fID, 0, SEEK_CUR);
			if(offset >= 0){
				offset -= dIO->cacheSeen;
				offset += dIO->cacheOffset;
				}
		}else if(dIO->mode == 1){ // write mode 
		offset = lseek(dIO->fID, 0, SEEK_CUR);
		if(offset >= 0){
			offset += dIO->cacheOffset;
		}		
		}
		return offset;			
	}
	return -1;
}

