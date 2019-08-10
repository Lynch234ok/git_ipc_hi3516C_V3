
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sqlite3.h"
#include "rwlock.h"
#include "frank_trace.h"

#ifndef FRANK_DB_H_
#define FRANK_DB_H_
#ifdef __cplusplus
extern "C" {
#endif

#define SQLITE3_API_CHECK(__exp, __db) \
	do{\
		int ret = (__exp);\
		if(SQLITE_OK != ret){ _TRACE("SQLITE3 error(%d) %s\r\n", ret, sqlite3_errmsg(__db)); } \
	}while(0)

typedef struct FRANK_DB {
	
	sqlite3 *sqliteDB;
	LP_RW_LOCK rwLock;
	char storagePath[256], cachePath[256];
	int alignedSize;
	
	int (*initTable)(struct FRANK_DB *const db);
}ST_FRANK_DB, *LP_FRANK_DB;

typedef int (*fFRANK_DB_INIT_TABLE)(LP_FRANK_DB const db);

extern LP_FRANK_DB frank_db_create(const char *storagePath, int aligendSize, const char *cachePath, fFRANK_DB_INIT_TABLE initTable, bool autoRebuild);
extern void frank_db_release(LP_FRANK_DB db);

extern bool frank_db_rdlock(LP_FRANK_DB const _this);
extern bool frank_db_wrlock(LP_FRANK_DB const _this);

extern bool frank_db_try_rdlock(LP_FRANK_DB const _this);
extern bool frank_db_try_wrlock(LP_FRANK_DB const _this);

extern bool frank_db_unlock(LP_FRANK_DB const _this);

extern int frank_db_vacuum(LP_FRANK_DB const _this);
extern int frank_db_flush(LP_FRANK_DB const _this);

#ifdef __cplusplus
}
#endif
#endif //FRANK_DB_H_

