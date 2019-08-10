#if 0
/**
 * TF 卡录像写入模块抽象。
 */



#include "NkEmbedded/tfer.h"
#include <NkUtils/json.h>


#if !defined(NK_TFER_INDEXER_H_)
# define NK_TFER_INDEXER_H_
NK_CPP_EXTERN_BEGIN


extern NK_Void
NK_TFer_LocalTime(NK_UTC1970 utc, NK_PChar str_date, NK_PChar str_time);


/**
 * 打开录像路径下面的索引，\n
 * 如果该路径下面没有索引文件，创建一个索引数据结构。
 *
 * @param[in]		Alloctr			内存分配器。
 * @param[in]		record_path		录像所在的文件系统目录。
 *
 * @return		索引句柄。
 */
extern NK_JSON *
NK_TFer_OpenIndexer(NK_Allocator *Alloctr, NK_PChar record_path);

/**
 * 关闭索引器句柄。
 *
 * @param[in]		Indexer			索引器句柄。
 *
 * @return		成功返回 0，失败返回 -1。
 */
extern NK_Int
NK_TFer_CloseIndexer(NK_JSON *Indexer);

/**
 * 索引器保存到指定目录下。
 *
 */
extern NK_Int
NK_TFer_FlushIndexer(NK_JSON *Indexer, NK_PChar record_path);

/**
 * 加入一条消息记录到索引器。
 *
 */
extern NK_Int
NK_TFer_AddInfoToIndexer(NK_JSON *Indexer, NK_JSON *Info);

/**
 *
 * @param record_path
 * @param utc
 * @return
 */
extern NK_Int
NK_TFer_History(NK_JSON *Indexer, NK_UTC1970 utc, NK_PChar type,
		NK_TFerHistory *History, NK_Size *history_count, NK_JSON **FileArray);


NK_CPP_EXTERN_END
#endif /* NK_TFER_INDEXER_H_ */

#endif
