#ifdef REBOOT_ONTIME
#ifndef _NK_REBOOT_ONTIME_H
#define _NK_REBOOT_ONTIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include <NkUtils/types.h>
	/*
	 * function : �����豸��ʱ�������� \n
	 * attention ; ��ɹ����ô˽ӿ�֮ǰ��Ҫ�����������ļ��ж�ʱ�����Ŀ���enableΪtrue��
	 *
	 * @param[in]		hourNum		�豸����ʱ���,Ҫ��Ϊ����,��λΪСʱ,��ΧΪ0 ~ 23,��������Χ,��Ĭ������Ϊ 2
	 *
	 * @return		д��ɹ�������д�����ݵĳ��ȣ�д��ʧ�ܷ��� -1��
	 */
typedef void *(*fDestroySystem)(void);
extern NK_Int NK_REBOOT_ONTIME_init(NK_Int hourNum, fDestroySystem OnDestroy);
extern NK_Int NK_REBOOT_ONTIME_destroy();
extern NK_Boolean NK_REBOOT_ONTIME_is_flag_exist();


#ifdef __cplusplus
};
#endif
#endif /*NK_TFER_H*/
#endif
