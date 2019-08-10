/**
 * NK Utils ��İ汾���塣
 */

#include <NkUtils/types.h>

#if !defined(NK_UTILS_VERSION_H_)
# define NK_UTILS_VERSION_H_
NK_CPP_EXTERN_BEGIN

/**
 * ���汾�š�
 */
#define NK_UTILS_VER_MAJ	(1U)
/**
 * �ΰ汾�š�
 */
#define NK_UTILS_VER_MIN	(4U)
/**
 * �޶��汾�š�
 */
#define NK_UTILS_VER_REV	(6U)


/**
 * ��ȡ NK Utils �汾�š�\n
 * �˽ӿ������жϿ�������ͷ�ļ��Ƿ�һ�¡�\n
 * ������ֽӿڻ�ȡ��ͷ�ļ��汾���岻һ�µ�ʱ����ƥ����ȷ��ͷ�ļ�����ļ���
 *
 * @param[out]			major			��Ӧ @ref NK_N1_VER_MAJ��
 * @param[out]			minor			��Ӧ @ref NK_N1_VER_MIN��
 * @param[out]			revision		��Ӧ @ref NK_N1_VER_REV��
 *
 */
extern NK_Void
NK_Utils_LibraryVersion(NK_Int *major, NK_Int *minor, NK_Int *revision);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_VERSION_H_ */




