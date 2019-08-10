#ifndef __NK_STREAM_RW_H__
#define __NK_STREAM_RW_H__
	
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
typedef struct NK_STREAM_RW stNK_STREAM_RW, *lpNK_STREAM_RW;

struct NK_STREAM_RW {
	int (*size)(lpNK_STREAM_RW context);

	int (*seek)(lpNK_STREAM_RW context, int offset, int whence);

	int (*read)(lpNK_STREAM_RW context, void *ptr, int size, int maxnum);

	int (*write)(lpNK_STREAM_RW context, const void *ptr, int size, int num);

	int (*close)(lpNK_STREAM_RW context);

	unsigned int (*get_handle)(lpNK_STREAM_RW context);

	union {
	    struct {
			int autoclose;
	 		FILE *fp;
	    } stdio;
	    struct {
			int autofree;
			char *base;
	 		char *here;
			char *stop;
	    } mem;
	} hidden;
};

#define NK_STREAM_RW_ALLOC		calloc
#define NK_STREAM_RW_FREE		free
#define NK_STREAM_RW_SEEK_SET	(0)
#define NK_STREAM_RW_SEEK_CUR	(1)
#define NK_STREAM_RW_SEEK_END	(2)

int NK_STREAM_RWFromFile(lpNK_STREAM_RW rwops, const char *file, const char *mode);
int NK_STREAM_RWFromFP(lpNK_STREAM_RW rwops, FILE *fp, int autoclose);
int NK_STREAM_RWFromMem(lpNK_STREAM_RW rwops, void *mem, int size, int autofree);

#define NK_STREAM_RWget_handle(ctx)				((ctx && (ctx)->get_handle) ? (ctx)->get_handle(ctx) : -1)
#define NK_STREAM_RWseek(ctx, offset, whence)		((ctx && (ctx)->seek) ? (ctx)->seek(ctx, offset, whence) : -1)
#define NK_STREAM_RWtell(ctx)						((ctx && (ctx)->seek) ? (ctx)->seek(ctx, 0, RW_SEEK_CUR) : -1)
#define NK_STREAM_RWsize(ctx)					((ctx && (ctx)->size) ? (ctx)->size(ctx) : -1)
#define NK_STREAM_RWread(ctx, ptr, size, n)			((ctx && (ctx)->read) ? (ctx)->read(ctx, ptr, size, n) : -1)
#define NK_STREAM_RWwrite(ctx, ptr, size, n)			((ctx && (ctx)->write) ? (ctx)->write(ctx, ptr, size, n) : -1)
#define NK_STREAM_RWclose(ctx)					((ctx && (ctx)->close) ? (ctx)->close(ctx) : -1)

#ifdef __cplusplus
}
#endif
#endif /* __NK_STREAM_RW_H__ */
