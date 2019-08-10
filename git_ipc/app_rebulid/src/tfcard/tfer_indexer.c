#if 0

#include "tfer_indexer.h"
#include <sys/stat.h>
#ifdef _WIN32
//#include <unistd.h> //linux
#include <io.h>  //windows
#include <process.h> //windows 
#else
#include <unistd.h> 
#endif
#include <fcntl.h>
#include <time.h>
#include <NkUtils/log.h>
#include <NkUtils/assert.h>
#include "generic.h"

/**
 * 获取目录中索引文件的路径。
 * @param record_path
 * @param path
 * @return
 */
static NK_PChar
indexerPath(NK_PChar record_path, NK_PChar path)
{
	sprintf(path, "%s/index.json", record_path);
	return path;
}

extern NK_Void
NK_TFer_LocalTime(NK_UTC1970 utc, NK_PChar str_date, NK_PChar str_time)
{
	struct tm SetTM;
#ifdef _WIN32
	localtime_s( &SetTM, (time_t *)(&utc));
#else
	localtime_r((time_t *)(&utc), &SetTM);
#endif
	sprintf(str_date, "%04d%02d%02d", SetTM.tm_year + 1900, SetTM.tm_mon + 1, SetTM.tm_mday);
	sprintf(str_time, "%02d%02d%02d", SetTM.tm_hour, SetTM.tm_min, SetTM.tm_sec);
}

extern NK_JSON *
NK_TFer_OpenIndexer(NK_Allocator *Alloctr, NK_PChar record_path)
{
	NK_Char indexer_path[128];
	//NK_Char jsonp[1024 * 128];
	NK_Char jsonp[1024 * 256];

	NK_SSize jsonp_len = 0;
	NK_Int fID = -1;
	NK_JSON *Indexer = NK_Nil;

	if (!Alloctr) {
		/**
		 * 默认采用 OS 的内存分配器。
		 */
		Alloctr = NK_MemAlloc_OS();
	}

	indexerPath(record_path, indexer_path);
#ifdef _WIN32
	fID = open(indexer_path, O_RDWR | O_CREAT, _S_IREAD | _S_IWRITE);
#else
	fID = open(indexer_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
#endif
	if (fID < 0) {
		NK_Log()->error("TFer: Open Indexer( Path = \"%s\" ) Failed.", indexer_path);
		return NK_Nil;
	}

	/**
	 * 读取文件到缓冲。
	 */
	jsonp_len = read(fID, jsonp, sizeof(jsonp));
	if (jsonp_len < 0) {
		NK_Log()->error("TFer: Read Indexer( Path = \"%s\" ) Failed.", indexer_path);
		close(fID);
		return NK_Nil;
	}

	if (jsonp_len == sizeof(jsonp)) {
		NK_Log()->warn("TFer: Read Indexer( Path = \"%s\" ) Too Large.", indexer_path);
	}

	close(fID);
	fID = -1;

	if (0 == jsonp_len || NK_Nil == (Indexer = NK_JSON_ParseText(Alloctr, jsonp))) {
		/**
		 * 未创建 Indexer 索引文件或者文件解析失败。
		 */
		Indexer = NK_JSON_CreateObject(Alloctr);
		NK_JSON_AddStringToObject(Indexer, "version", NK_TFER_VERSION);
		NK_JSON_AddItemToObject(Indexer, "Index", NK_JSON_CreateArray(Alloctr));

		/**
		 * 回写到文件。
		 */
		if (0 != NK_TFer_FlushIndexer(Indexer, record_path)) {
			Indexer->Object.free(&Indexer->Object);
			return NK_Nil;
		}
	}

	return Indexer;
}


NK_Int
NK_TFer_CloseIndexer(NK_JSON *Indexer)
{
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Indexer, -1);
	Indexer->Object.free(&Indexer->Object);
	return 0;
}


NK_Int
NK_TFer_FlushIndexer(NK_JSON *Indexer, NK_PChar record_path)
{
	NK_Char indexer_path[128];
	NK_Char indexer_tmp_path[128];
	NK_Char jsonp[1024 * 128];
	NK_SSize jsonp_len = 0;
	NK_SSize writen = 0;
	NK_Int fID = -1;

	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Indexer, -1);

	/**
	 * 生成 JSON 文本。
	 */
	jsonp_len = sizeof(jsonp);
	NK_EXPECT_VERBOSE_RETURN(0 == NK_JSON_ToText(Indexer, NK_False, jsonp, (NK_Size *)(&jsonp_len)), -1);

	/**
	 * 写入 JSON 文本到文件。
	 */
	indexerPath(record_path, indexer_path);
	sprintf(indexer_tmp_path,"%s.tmp",indexer_path);
	REMOVE_FILE(indexer_tmp_path);

#ifdef _WIN32
	fID = open(indexer_tmp_path, O_RDWR | O_CREAT, _S_IREAD | _S_IWRITE);
#else
	fID = open(indexer_tmp_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

#endif
	if (fID < 0) {
		NK_Log()->error("TFer: Open Indexer tmp( Path = \"%s\" ) Failed.", indexer_tmp_path);
		printf("TFer: Open Indexer tmp( Path = \"%s\" ) Failed.", indexer_tmp_path);
		return -1;
	}

	writen = write(fID, jsonp, jsonp_len);
	if (writen <= 0) {
		NK_Log()->error("TFer: Write Indexer tmp( Path = \"%s\", Length = %u ) Failed.",
				indexer_tmp_path, jsonp_len);
		printf("TFer: Write Indexer tmp( Path = \"%s\", Length = %u ) Failed.",
				indexer_tmp_path, jsonp_len);
		close(fID);
		fID = -1;
		REMOVE_FILE(indexer_tmp_path);

		return -1;
	}

	close(fID);
	fID = -1;

	//recover the indexer file
	if(0 != rename(indexer_tmp_path,indexer_path)){
		NK_Log()->error("TFer: Sync Indexer ( Path = \"%s\" ) Failed", indexer_path);
		printf("TFer: Sync Indexer ( Path = \"%s\" ) Failed\n", indexer_path);
		//return -1;
	}
	REMOVE_FILE(indexer_tmp_path);
	
	return 0;
}


NK_Int
NK_TFer_AddInfoToIndexer(NK_JSON *Indexer, NK_JSON *Info)
{
	NK_JSON *VarIndexArray = NK_Nil;
	NK_JSON *VarInfo = NK_Nil;
	NK_UTC1970 old_begin_utc = 0;
	NK_UTC1970 new_begin_utc = 0;
	NK_Size index_len = 0;

	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Indexer, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Info, -1);

	/**
	 * 读取 Index 标签。
	 */
	VarIndexArray = NK_JSON_IndexOfName(Indexer, "Index");
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != VarIndexArray, -1);

	index_len = NK_JSON_ArraySize(VarIndexArray);
	if (0 == index_len) {
		/**
		 * 加入一条记录。
		 */
		NK_JSON_AddItemToArray(VarIndexArray, Info);

	} else {

		/**
		 * 尝试去更新最后一条记录。
		 */
		VarInfo = NK_JSON_IndexOf(VarIndexArray, index_len - 1);
		old_begin_utc = (NK_UTC1970)(NK_JSON_GetNumber(NK_JSON_IndexOfName(VarInfo, "begin_utc"), 0));
		new_begin_utc = (NK_UTC1970)(NK_JSON_GetNumber(NK_JSON_IndexOfName(Info, "begin_utc"), 0));

		if (old_begin_utc == new_begin_utc) {
			/**
			 * 更新上次最后一条记录。
			 */
			NK_JSON_SetItemInArray(VarIndexArray, index_len - 1, Info);
		} else {
			/**
			 * 追加一条新记录。
			 */
			NK_JSON_AddItemToArray(VarIndexArray, Info);
		}

	}

	return 0;
}

NK_Int
NK_TFer_History(NK_JSON *Indexer, NK_UTC1970 utc, NK_PChar type,
		NK_TFerHistory *History, NK_Size *history_count, NK_JSON **FileList)
{
	NK_Size count = 0;
	NK_JSON *VarIndexArray = NK_Nil;
	NK_Size index_array_len = 0;
	NK_JSON *VarInfo = NK_Nil;
	NK_JSON *VarEndUTC = NK_Nil;
	NK_UTC1970 end_utc = 0;
	NK_JSON *VarFileArray = NK_Nil;
	NK_Size file_array_len = 0;
	NK_Int i = 0, ii = 0;
	NK_JSON *VarFileList = NK_Nil;

	if (!type) {
		type = "default";
	}

	if (NK_Nil != FileList) {
		VarFileList = NK_JSON_CreateArray(NK_JSON_Allocator(Indexer));
	}

	VarIndexArray = NK_JSON_IndexOfName(Indexer, "Index");
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != VarIndexArray, -1);

	index_array_len = NK_JSON_ArraySize(VarIndexArray);
	for (i = 0; i < index_array_len; ++i) {

		/**
		 * 遍历每一个 Info。
		 */
		VarInfo = NK_JSON_IndexOf(VarIndexArray, i);
		if (!VarInfo) {
			break;
		}

		VarEndUTC = NK_JSON_IndexOfName(VarInfo, "end_utc");
		end_utc = NK_JSON_GetNumber(VarEndUTC, 0);

		/**
		 * 找出 UTC 时间正方向上的录像信息。
		 */
		if (end_utc >= utc) {

			/**
			 * 记录到历史列表。
			 */
			if (NK_Nil != History && NK_Nil != history_count && count < *history_count) {
				History[count].end_utc = end_utc;
				History[count].begin_utc = NK_JSON_GetNumber(NK_JSON_IndexOfName(VarInfo, "begin_utc"), end_utc);
				strcpy(History[count].begin_time, NK_JSON_GetString(NK_JSON_IndexOfName(VarInfo, "begin_time"), ""));
				strcpy(History[count].end_time, NK_JSON_GetString(NK_JSON_IndexOfName(VarInfo, "end_time"), ""));
				strcpy(History[count].type, NK_JSON_GetString(NK_JSON_IndexOfName(VarInfo, "type"), ""));
				++count;
			}

			if (NK_Nil != VarFileList) {

				/**
				 * 获取文件列表。
				 */
				VarFileArray = NK_JSON_IndexOfName(VarInfo, "File");
				file_array_len = NK_JSON_ArraySize(VarFileArray);

				for (ii = 0; ii < file_array_len; ++ii) {
					NK_JSON_AddItemToArray(VarFileList,
							NK_JSON_Duplicate(NK_JSON_Allocator(Indexer), NK_JSON_IndexOf(VarFileArray, ii)));
				}
			}
		}
	}

	if (NK_Nil != history_count) {
		*history_count = count;
	}

	if (NK_Nil != FileList) {
		*FileList = VarFileList;
	}

	return 0;
}

#endif
