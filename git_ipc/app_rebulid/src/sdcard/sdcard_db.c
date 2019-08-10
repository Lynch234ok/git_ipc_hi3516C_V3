
#include "sdcard_db.h"
#include "frank_db.h"
#include "frank_trace.h"

enum {
	kSD_DB_MEDIA_COLUMN_ID = 0, // int
	kSD_DB_MEDIA_COLUMN_CHANNEL_ID, // int
	kSD_DB_MEDIA_COLUMN_BEGIN_UTC, // int
	kSD_DB_MEDIA_COLUMN_BEGIN_FILE_ID, // int
	kSD_DB_MEDIA_COLUMN_BEGIN_FILE_OFF, // int
	kSD_DB_MEDIA_COLUMN_END_UTC, // int
	kSD_DB_MEDIA_COLUMN_END_FILE_ID, // int
	kSD_DB_MEDIA_COLUMN_END_FILE_OFF, // int
	kSD_DB_MEDIA_COLUMN_TYPE, // string
};

static LP_FRANK_DB _lpSDCardDB = NULL;

static int sdcard_init_table(LP_FRANK_DB const frankDB)
{
	sqlite3 *sqliteDB = frankDB->sqliteDB;
	char *sql = NULL;
	// create media table
	// id(int) | beginUTC(int) | beginFileID(int) | beginFileOffset(int) | endUTC(int) | endFileID(int) | endFileOffset(int) | type(string) |
	
	sql = sqlite3_mprintf("CREATE TABLE MAIN.[media]("
		"[id] INTEGER PRIMARY KEY AUTOINCREMENT," // the table id
		"[channelID] INTEGER," // the session unique id
		"[beginUTC] INTEGER,"
		"[beginFileID] INTEGER,"
		"[beginFileOffset] INTEGER,"
		"[endUTC] INTEGER,"
		"[endFileID] INTEGER,"
		"[endFileOffset] INTEGER,"
		"[type] VARCHAR)");
	SQLITE3_API_CHECK(sqlite3_exec(sqliteDB, sql, NULL, NULL, NULL), sqliteDB);
	sqlite3_free(sql);
	sql = NULL;
	sql = sqlite3_mprintf("CREATE INDEX [mediaIndex] ON [media] ([id], [beginUTC], [endUTC])");
	SQLITE3_API_CHECK(sqlite3_exec(sqliteDB, sql, NULL, NULL, NULL), sqliteDB);
	sqlite3_free(sql);
	sql = NULL;
	
	// create gallery table
	sql = sqlite3_mprintf("CREATE TABLE MAIN.[%s]("
		"[id] INTEGER PRIMARY KEY AUTOINCREMENT," // the table id
		"[createUTC] INTEGER)", "gallery");
	SQLITE3_API_CHECK(sqlite3_exec(sqliteDB, sql, NULL, NULL, NULL), sqliteDB);
	sqlite3_free(sql);
	sql = NULL;
	// FIXME: dont forget to create index for gallery table
	return 0;
}

int SDCARD_db_create(const char *dbPath, const char *cachePath)
{
	if(!_lpSDCardDB){
		int const flashAligned = 512 * 1024;
		_lpSDCardDB = frank_db_create(dbPath, flashAligned, cachePath, sdcard_init_table, true);
		if(_lpSDCardDB){
			return 0;
		}
	}
	return -1;
}

int SDCARD_db_open(const char *dbPath, const char *cachePath)
{
	if(!_lpSDCardDB){
		int const flashAligned = 512 * 1024;
		_lpSDCardDB = frank_db_create(dbPath, flashAligned, cachePath, sdcard_init_table, false);
		if(_lpSDCardDB){
			return 0;
		}
	}
	return -1;
}

void SDCARD_db_close()
{
	if(_lpSDCardDB){
		//frank_db_flush(_lpSDCardDB);
		frank_db_release(_lpSDCardDB);
		_lpSDCardDB = NULL;
	}
}

int SDCARD_db_media_create_session(int channelID, int utc, int fileID, int fileOffset, const char *type)
{
	int sessionID = -1;
	int sqliteRet = 0;
	sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
	
	if(frank_db_wrlock(_lpSDCardDB)){
		char *sql = sqlite3_mprintf("INSERT INTO [media]"
			"("
			"channelID,"
			"beginUTC,beginFileID,beginFileOffset,"
			"endUTC, endFileID, endFileOffset,"
			"type"
			")"
			"VALUES("
			"%d,"
			"%d, %d, %d,"
			"%d, %d, %d,"
			"'%s'"
			");",
			channelID,
			utc, fileID, fileOffset,
			utc, fileID, fileOffset,
			type);
		_TRACE("SQL: %s", sql);
		sqliteRet = sqlite3_exec(sqliteDB, sql, NULL, NULL, NULL);
		SQLITE3_API_CHECK(sqliteRet, sqliteDB);
		sqlite3_free(sql);
		sql = NULL;
		
		// get the id as sessionID
		sql = sqlite3_mprintf("SELECT id FROM [media] ORDER BY id DESC LIMIT 1"); // get the latest id as session ID
		sqlite3_stmt *stmt = NULL;
		_TRACE("SQL: %s", sql);
		SQLITE3_API_CHECK(sqlite3_prepare(sqliteDB, sql, -1, &stmt, 0), sqliteDB);
		sqlite3_free(sql);
		sql = NULL;

		if(SQLITE_ROW == sqlite3_step(stmt)){
			sessionID = sqlite3_column_int(stmt, 0); // use the next file ID
			_TRACE("Create Session ID(%d)", sessionID);
		}
		sqlite3_finalize(stmt);
		stmt = NULL;
		
		frank_db_unlock(_lpSDCardDB);
	}
	return sessionID;
}

int SDCARD_db_media_update_session(int sessionID, int utc, int fileID, int fileOffset)
{
	int sqliteRet = 0;
	sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
	
	if(frank_db_wrlock(_lpSDCardDB)){
		char *sql = sqlite3_mprintf("UPDATE [media] SET "
			"endUTC=%d, endFileID=%d, endFileOffset=%d WHERE id=%d",
			utc, fileID, fileOffset,
			sessionID);
		_TRACE("SQL: %s", sql);
		sqliteRet = sqlite3_exec(sqliteDB, sql, NULL, NULL, NULL);
		SQLITE3_API_CHECK(sqliteRet, sqliteDB);
		sqlite3_free(sql);
		sql = NULL;
		frank_db_unlock(_lpSDCardDB);
	}

	if(SQLITE_OK != sqliteRet){
		return -1;
	}
	return 0;
}

int SDCARD_db_media_remove_session(int sessionID)
{
	int sqliteRet = 0;
	sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
	
	if(frank_db_wrlock(_lpSDCardDB)){
		char *sql = sqlite3_mprintf("DELETE FROM [media] WHERE id=%d", sessionID);
		//_TRACE("INSERT SQL: %s", sql);
		sqliteRet = sqlite3_exec(sqliteDB, sql, NULL, NULL, NULL);
		SQLITE3_API_CHECK(sqliteRet, sqliteDB);
		sqlite3_free(sql);
		sql = NULL;
		frank_db_unlock(_lpSDCardDB);
	}

	if(SQLITE_OK != sqliteRet){
		return -1;
	}
	return 0;
}

LP_SDCARD_MEDIA_SESSION SDCARD_db_media_get_session(int sessionID, LP_SDCARD_MEDIA_SESSION session)
{
	if(_lpSDCardDB){
		// find where is the latest session
		sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
		if(frank_db_rdlock(_lpSDCardDB)){
			char* sql = sqlite3_mprintf("SELECT * FROM [media] WHERE id=%d", sessionID); // FIXME: you'd better use embedded
			sqlite3_stmt *stmt = NULL;
			_TRACE("SQL: %s", sql);
			SQLITE3_API_CHECK(sqlite3_prepare(sqliteDB, sql, -1, &stmt, 0), sqliteDB);
			sqlite3_free(sql);
			if(SQLITE_ROW == sqlite3_step(stmt)){
				int column = 0;
				session->sessionID = sqlite3_column_int(stmt, column++);
				session->channelID = sqlite3_column_int(stmt, column++);
				session->beginUTC = sqlite3_column_int(stmt, column++);
				session->beginFileID = sqlite3_column_int(stmt, column++);
				session->beginFileOffset = sqlite3_column_int(stmt, column++);
				session->endUTC = sqlite3_column_int(stmt, column++);
				session->endFileID = sqlite3_column_int(stmt, column++);
				session->endFileOffset = sqlite3_column_int(stmt, column++);
				snprintf(session->type, sizeof(session->type), "%s", sqlite3_column_text(stmt, column++));
				//_TRACE("Column %d", column);
			}			
			sqlite3_finalize(stmt);
			stmt = NULL;
			frank_db_unlock(_lpSDCardDB);

			return session;
		}
	}
	return NULL;
}

int SDCARD_db_media_search_by_time(uint64_t channelIDs, int beginUTC, int endUTC, LP_SDCARD_MEDIA_SESSION buf, int bufMax)
{
	int i = 0;
	if(_lpSDCardDB){
		sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;

		if(frank_db_try_rdlock(_lpSDCardDB)){
			int sessionMax = bufMax / sizeof(ST_SDCARD_MEDIA_SESSION);
			int sessionCnt = 0;
			char sqlWhereChannelID[128], sqlWhereUTC[128];

			char* sql = NULL;//sqlite3_mprintf("SELECT * FROM [media] WHERE ((beginUTC>=%d AND beginUTC<=%d)  OR (endUTC>=%d AND endUTC<=%d))", beginUTC, endUTC, beginUTC, endUTC);
			//char* sql = sqlite3_mprintf("SELECT * FROM [media] WHERE channelID=1");
			sqlite3_stmt *stmt = NULL;

			// create sql channel ID condition
			strcpy(sqlWhereChannelID, "channelID IN (1)"); // FIXME:
			// create sql UTC condition
			if(beginUTC >= 0 && endUTC >= 0){
				snprintf(sqlWhereUTC, sizeof(sqlWhereUTC), "(beginUTC<=%d AND endUTC>=%d)", endUTC, beginUTC);
 			}else if(beginUTC >= 0){
				snprintf(sqlWhereUTC, sizeof(sqlWhereUTC), "endUTC>%d", beginUTC);
			}else{
				snprintf(sqlWhereUTC, sizeof(sqlWhereUTC), "1", endUTC, beginUTC);
			}

			sql = sqlite3_mprintf("SELECT * FROM [media] WHERE %s AND %s ORDER BY id DESC", sqlWhereChannelID, sqlWhereUTC);
			_TRACE("SQL: %s", sql);
			SQLITE3_API_CHECK(sqlite3_prepare(sqliteDB, sql, -1, &stmt, 0), sqliteDB);
			sqlite3_free(sql);
			sql = NULL;

			_TRACE("Session Capacity = %d", sessionMax);
			
			while(SQLITE_ROW == sqlite3_step(stmt)){
				if(sessionCnt < sessionMax){
					LP_SDCARD_MEDIA_SESSION const session = &buf[sessionCnt++];

					//_TRACE("Column Count = %d", sqlite3_column_count(stmt));

					session->sessionID = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_ID);
					session->channelID = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_CHANNEL_ID);
					session->beginUTC = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_UTC);
					session->beginFileID = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_FILE_ID);
					session->beginFileOffset = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_FILE_OFF);
					session->endUTC = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_UTC);
					session->endFileID = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_FILE_ID);
					session->endFileOffset = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_FILE_OFF);
					strncpy(session->type, (char *)sqlite3_column_text(stmt, kSD_DB_MEDIA_COLUMN_TYPE), sizeof(session->type) - 1);

					//_TRACE("id=%d beginUTC=%d endUTC=%d type=%s", session->sessionID, session->beginUTC, session->endUTC, session->type);
				}else{
					break;
				}
			}
			sqlite3_finalize(stmt);
			stmt = NULL;
			frank_db_unlock(_lpSDCardDB);

			return sessionCnt;
		}
	}
	return -1;
}

int SDCARD_db_media_search_by_session(int sessionID, LP_SDCARD_MEDIA_SESSION result)
{
	int i = 0;
	if(_lpSDCardDB){
		sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
		int found = 0;

		if(frank_db_try_rdlock(_lpSDCardDB)){
			char* sql = NULL;
			sqlite3_stmt *stmt = NULL;

			sql = sqlite3_mprintf("SELECT * FROM [media] WHERE id=%d", sessionID);
			_TRACE("SQL: %s", sql);
			SQLITE3_API_CHECK(sqlite3_prepare(sqliteDB, sql, -1, &stmt, 0), sqliteDB);
			sqlite3_free(sql);
			sql = NULL;

			if(SQLITE_ROW == sqlite3_step(stmt)){
				result->sessionID = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_ID);
				result->channelID = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_CHANNEL_ID);
				result->beginUTC = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_UTC);
				result->beginFileID = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_FILE_ID);
				result->beginFileOffset = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_FILE_OFF);
				result->endUTC = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_UTC);
				result->endFileID = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_FILE_ID);
				result->endFileOffset = sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_FILE_OFF);
				strncpy(result->type, (char *)sqlite3_column_text(stmt, kSD_DB_MEDIA_COLUMN_TYPE), sizeof(result->type) - 1);

				_TRACE("id=%d beginUTC=%d endUTC=%d type=%s", result->sessionID, result->beginUTC, result->endUTC, result->type);

				++found;
			}
			sqlite3_finalize(stmt);
			stmt = NULL;
			frank_db_unlock(_lpSDCardDB);

			return found;
		}
	}
	return -1;
}


int SDCARD_db_media_oldest_session()
{
	int sessionID = -1;
	if(_lpSDCardDB){
		// find where is the latest session
		sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
		if(frank_db_rdlock(_lpSDCardDB)){
			char* sql = sqlite3_mprintf("SELECT id FROM [media] ORDER BY id ASC LIMIT 1");
			sqlite3_stmt *stmt = NULL;
			_TRACE("SQL: %s", sql);
			SQLITE3_API_CHECK(sqlite3_prepare(sqliteDB, sql, -1, &stmt, 0), sqliteDB);
			sqlite3_free(sql);
			if(SQLITE_ROW == sqlite3_step(stmt)){
				sessionID = sqlite3_column_int(stmt, 0);
				_TRACE("Get Oldest Session ID %d", sessionID);
			}
			sqlite3_finalize(stmt);
			stmt = NULL;
			frank_db_unlock(_lpSDCardDB);
		}
	}
	return sessionID;
}


int SDCARD_db_media_latest_session_id(int sessionID)
{
	return -1;
}




int SDCARD_db_flush()
{
	if(_lpSDCardDB){
		return frank_db_flush(_lpSDCardDB);
	}
	return -1;
}

void SDCARD_db_dump_media()
{
	if(_lpSDCardDB){
		sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
		if(frank_db_rdlock(_lpSDCardDB)){
			sqlite3_stmt *stmt = NULL;
			_TRACE("|%12s"
				"|%12s" // channelID
				"|%24s|%12s|%12s"
				"|%24s|%12s|%12s"
				"|%12s|",
				"ID",
				"channelID",
				"Begin UTC", "File ID", "File Offset",
				"End UTC", "File ID", "File Offset",
				"Type");
			SQLITE3_API_CHECK(sqlite3_prepare(sqliteDB, "SELECT * FROM [media]", -1, &stmt, 0), sqliteDB);
			
			while(SQLITE_ROW == sqlite3_step(stmt)){
				time_t const beginUTC = (time_t)(sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_UTC));
				time_t const endUTC = (time_t)(sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_UTC));
				char textBeginUTC[64] = {""}, textEndUTC[64] = {""};

				strftime(textBeginUTC, sizeof(textBeginUTC), "%G/%m/%d-%T GMT", gmtime(&beginUTC));
				strftime(textEndUTC, sizeof(textEndUTC), "%G/%m/%d-%T GMT", gmtime(&endUTC));
				
				_TRACE("|%12d"
					"|%12d"
					"|%24s|%12d|%12d"
					"|%24s|%12d|%12d"
					"|%12s"
					"|",
					sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_ID),
					sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_CHANNEL_ID),
					textBeginUTC,
					sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_FILE_ID),
					sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_BEGIN_FILE_OFF),
					textEndUTC,
					sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_FILE_ID),
					sqlite3_column_int(stmt, kSD_DB_MEDIA_COLUMN_END_FILE_OFF),
					sqlite3_column_text(stmt, kSD_DB_MEDIA_COLUMN_TYPE));
			}
			sqlite3_finalize(stmt);
			stmt = NULL;
			frank_db_unlock(_lpSDCardDB);
		}
	}
}

void SDCARD_db_dump_gallery()
{
	if(_lpSDCardDB){
		if(frank_db_wrlock(_lpSDCardDB)){
			frank_db_unlock(_lpSDCardDB);
		}
	}
}

int SDCARD_db_media_free_file_id()
{
	int id = -1;
	if(_lpSDCardDB){
		// find where is the latest session
		sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
		if(frank_db_rdlock(_lpSDCardDB)){
			char* sql = sqlite3_mprintf("SELECT endFileID FROM [media] ORDER BY endFileID DESC LIMIT 1");
			sqlite3_stmt *stmt = NULL;
			_TRACE("SQL: %s", sql);
			SQLITE3_API_CHECK(sqlite3_prepare(sqliteDB, sql, -1, &stmt, 0), sqliteDB);
			sqlite3_free(sql);
			if(SQLITE_ROW == sqlite3_step(stmt)){
				id = sqlite3_column_int(stmt, 0) + 1; // use the next file ID
				_TRACE("Database Indicate Free FileID = %d", id);
			}else{
				// nothing to found
				id = 0;
			}
			sqlite3_finalize(stmt);
			stmt = NULL;
			frank_db_unlock(_lpSDCardDB);
		}
	}
	_TRACE("DB free file ID = %d", id);
	return id;
}

int SDCARD_db_media_oldest_file_id()
{
	int id = -1;
	if(_lpSDCardDB){
		// find where is the latest session
		sqlite3 *sqliteDB = _lpSDCardDB->sqliteDB;
		if(frank_db_rdlock(_lpSDCardDB)){
			char* sql = sqlite3_mprintf("SELECT id FROM [media] ORDER BY id ASC LIMIT 1");
			sqlite3_stmt *stmt = NULL;
			_TRACE("SQL: %s", sql);
			SQLITE3_API_CHECK(sqlite3_prepare(sqliteDB, sql, -1, &stmt, 0), sqliteDB);
			sqlite3_free(sql);
			if(SQLITE_ROW == sqlite3_step(stmt)){
				id = sqlite3_column_int(stmt, 0) + 1; // use the next file ID
				_TRACE("Database Indicate Free FileID = %d", id);
			}else{
				// nothing to found
				id = 0;
			}
			sqlite3_finalize(stmt);
			stmt = NULL;
			frank_db_unlock(_lpSDCardDB);
		}
	}
	_TRACE("DB free file ID = %d", id);
	return id;
}


