
#include "nk_stream_rw.h"
#include "app_debug.h"

static int stdio_seek(lpNK_STREAM_RW context, int offset, int whence)
{
	int seek_where = SEEK_SET;
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}

	switch (whence) {
		case NK_STREAM_RW_SEEK_SET:
			seek_where = SEEK_SET;
			break;
		case NK_STREAM_RW_SEEK_CUR:
			seek_where = SEEK_CUR;
			break;
		case NK_STREAM_RW_SEEK_END:
			seek_where = SEEK_END;
			break;
		default:
			APP_TRACE("Unknown value for whence");
			return(-1);
	}

	if ( fseek(context->hidden.stdio.fp, offset, seek_where) == 0 ) {
		return(ftell(context->hidden.stdio.fp));
	} else {
		APP_TRACE("fseek error");
		return(-1);
	}
}

static int stdio_size(lpNK_STREAM_RW context)
{
	int offset = 0;
	int size = 0;
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}

	offset = ftell(context->hidden.stdio.fp);

	if ( fseek(context->hidden.stdio.fp, offset, SEEK_END) == 0 ) {
		size = ftell(context->hidden.stdio.fp);
	} else {
		APP_TRACE("get size fseek end error");
		return -1;
	}

	if ( fseek(context->hidden.stdio.fp, offset, SEEK_SET) != 0){
		APP_TRACE("get size fseek reset error");
		return -2;
	}

	return size;
}

static int stdio_read(lpNK_STREAM_RW context, void *ptr, int size, int maxnum)
{
	size_t nread;
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}

	nread = fread(ptr, size, maxnum, context->hidden.stdio.fp); 
	if ( nread == 0 && ferror(context->hidden.stdio.fp) ) {
		APP_TRACE("fread error");
	}
	return(nread);
}

static int stdio_write(lpNK_STREAM_RW context, const void *ptr, int size, int num)
{
	size_t nwrote;
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}

	nwrote = fwrite(ptr, size, num, context->hidden.stdio.fp);
	if ( nwrote == 0 && ferror(context->hidden.stdio.fp) ) {
		APP_TRACE("fwrite error");
	}
	return(nwrote);
}

static int stdio_close(lpNK_STREAM_RW context)
{
	if ( context ) {
		if ( context->hidden.stdio.autoclose ) {
			/* WARNING:  Check the return value here! */
			fclose(context->hidden.stdio.fp);
			context->hidden.stdio.fp = NULL;
		}
	}
	return(0);
}

unsigned int  stdio_get_handle (lpNK_STREAM_RW context)
{
	if ( context ) {
		return (unsigned int) context->hidden.stdio.fp;
	}
	return(0);
}

static int mem_seek(lpNK_STREAM_RW context, int offset, int whence)
{
	char *newpos = NULL;
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}

	switch (whence) {
		case NK_STREAM_RW_SEEK_SET:
			newpos = context->hidden.mem.base+offset;
			break;
		case NK_STREAM_RW_SEEK_CUR:
			newpos = context->hidden.mem.here+offset;
			break;
		case NK_STREAM_RW_SEEK_END:
			newpos = context->hidden.mem.stop+offset;
			break;
		default:
			APP_TRACE("Unknown value for whence");
			return(-1);
	}
	if ( newpos < context->hidden.mem.base ) {
		newpos = context->hidden.mem.base;
	}
	if ( newpos > context->hidden.mem.stop ) {
		newpos = context->hidden.mem.stop;
	}
	context->hidden.mem.here = newpos;
	return(context->hidden.mem.here-context->hidden.mem.base);
}

static int mem_size(lpNK_STREAM_RW context)
{
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}
	return context->hidden.mem.here - context->hidden.mem.base;
}

static int mem_read(lpNK_STREAM_RW context, void *ptr, int size, int maxnum)
{
	size_t total_bytes = 0;
	size_t mem_available = 0;
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}

	total_bytes = (maxnum * size);
	if ( (maxnum <= 0) || (size <= 0) || ((total_bytes / maxnum) != (size_t) size) ) {
		return 0;
	}

	mem_available = (context->hidden.mem.stop - context->hidden.mem.here);
	if (total_bytes > mem_available) {
		total_bytes = mem_available;
	}

	memcpy(ptr, context->hidden.mem.here, total_bytes);
	context->hidden.mem.here += total_bytes;

	return (total_bytes / size);
}

static int mem_write(lpNK_STREAM_RW context, const void *ptr, int size, int num)
{
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}

	if ( (context->hidden.mem.here + (num*size)) > context->hidden.mem.stop ) {
		num = (context->hidden.mem.stop-context->hidden.mem.here)/size;
	}
	memcpy(context->hidden.mem.here, ptr, num*size);
	context->hidden.mem.here += num*size;
	return(num);
}

static int mem_close(lpNK_STREAM_RW context)
{
	if(!context){
		APP_TRACE("context is null!");
		return -1;
	}

	if ( context->hidden.mem.autofree ){
		NK_STREAM_RW_FREE(context->hidden.mem.base);
		context->hidden.mem.base = NULL;
	}

	return(0);
}

unsigned int  mem_get_handle (lpNK_STREAM_RW context)
{
	if ( context ) {
		return (unsigned int) context->hidden.mem.base;
	}
	return(0);
}


int NK_STREAM_RWFromFile(lpNK_STREAM_RW rwops, const char *file, const char *mode)
{
	FILE *fp = NULL;

	if ( !file || !*file || !mode || !*mode ) {
		APP_TRACE("NK_STREAM_RWFromFile args error");
		return -1;
	}

	fp = fopen(file, mode);

	if ( fp == NULL ) {
		APP_TRACE("fopen error");
		return -1;
	}

	return NK_STREAM_RWFromFP(rwops, fp, 1);
}

int NK_STREAM_RWFromFP(lpNK_STREAM_RW rwops, FILE *fp, int autoclose)
{
	if ( rwops != NULL ) {
		rwops->seek = stdio_seek;
		rwops->size = stdio_size;
		rwops->read = stdio_read;
		rwops->write = stdio_write;
		rwops->close = stdio_close;
		rwops->get_handle = stdio_get_handle;
		rwops->hidden.stdio.fp = fp;
		rwops->hidden.stdio.autoclose = autoclose;
		return 0;
	}
	return -1;
}

int NK_STREAM_RWFromMem(lpNK_STREAM_RW rwops, void *mem, int size, int autofree)
{
	if ( rwops != NULL ) {
		rwops->seek = mem_seek;
		rwops->size = mem_size;
		rwops->read = mem_read;
		rwops->write = mem_write;
		rwops->close = mem_close;
		rwops->get_handle = mem_get_handle;
		rwops->hidden.mem.base = (char *)mem;
		rwops->hidden.mem.here = rwops->hidden.mem.base;
		rwops->hidden.mem.stop = rwops->hidden.mem.base+size;
		rwops->hidden.mem.autofree = autofree;
		return 0;
	}
	return -1;
}


