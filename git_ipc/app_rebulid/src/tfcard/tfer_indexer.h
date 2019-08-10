#if 0
/**
 * TF ��¼��д��ģ�����
 */



#include "NkEmbedded/tfer.h"
#include <NkUtils/json.h>


#if !defined(NK_TFER_INDEXER_H_)
# define NK_TFER_INDEXER_H_
NK_CPP_EXTERN_BEGIN


extern NK_Void
NK_TFer_LocalTime(NK_UTC1970 utc, NK_PChar str_date, NK_PChar str_time);


/**
 * ��¼��·�������������\n
 * �����·������û�������ļ�������һ���������ݽṹ��
 *
 * @param[in]		Alloctr			�ڴ��������
 * @param[in]		record_path		¼�����ڵ��ļ�ϵͳĿ¼��
 *
 * @return		���������
 */
extern NK_JSON *
NK_TFer_OpenIndexer(NK_Allocator *Alloctr, NK_PChar record_path);

/**
 * �ر������������
 *
 * @param[in]		Indexer			�����������
 *
 * @return		�ɹ����� 0��ʧ�ܷ��� -1��
 */
extern NK_Int
NK_TFer_CloseIndexer(NK_JSON *Indexer);

/**
 * ���������浽ָ��Ŀ¼�¡�
 *
 */
extern NK_Int
NK_TFer_FlushIndexer(NK_JSON *Indexer, NK_PChar record_path);

/**
 * ����һ����Ϣ��¼����������
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
