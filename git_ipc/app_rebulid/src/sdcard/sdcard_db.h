
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "sdcard_def.h"

#ifndef SDCARD_DB_H_
#define SDCARD_DB_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int SDCARD_db_create(const char *dbPath, const char *cachePath);
extern int SDCARD_db_open(const char *dbPath, const char *cachePath);

extern int SDCARD_db_flush();

extern void SDCARD_db_close();

extern int SDCARD_db_media_create_session(int channelID, int utc, int fileID, int fileOffset, const char *type);
extern int SDCARD_db_media_update_session(int sessionID, int utc, int fileID, int fileOffset);
extern int SDCARD_db_media_remove_session(int sessionID);

extern int SDCARD_db_media_free_file_id();
extern int SDCARD_db_media_oldest_file_id();

extern int SDCARD_db_media_oldest_session();

typedef struct SDCARD_MEDIA_SESSION {
	int sessionID;
	int channelID;
	int beginUTC, endUTC;
	int beginFileID, endFileID;
	int beginFileOffset, endFileOffset;
	char type[64];
}ST_SDCARD_MEDIA_SESSION, *LP_SDCARD_MEDIA_SESSION;

extern LP_SDCARD_MEDIA_SESSION SDCARD_db_media_get_session(int sessionID, LP_SDCARD_MEDIA_SESSION session);

extern int SDCARD_db_media_search_by_time(uint64_t channelIDs, int beginUTC, int endUTC, LP_SDCARD_MEDIA_SESSION buf, int bufMax);
extern int SDCARD_db_media_search_by_session(int sessionID, LP_SDCARD_MEDIA_SESSION result);

extern void SDCARD_db_dump_media();
extern void SDCARD_db_dump_gallery();


#ifdef __cplusplus
};
#endif
#endif //SDCARD_WRITER_H_

