
#ifndef _TFCARD_RECORD1_H_
#define _TFCARD_RECORD1_H_

#ifdef __cplusplus
extern "C"{
#endif
/////////////////////////////////////////////////////////////////////

 typedef enum tagTFRECSTATUS_
 {
	EN_TFRECORD_RUNING = (0x1010),
	EN_TFRECORD_ERRWRITE,
	EN_TFRECORD_START,
	EN_TFRECORD_STOP,
 }enTFRECSTATUS;


 HTFRECORD TFRECORD_Create(pstTFRECPARAM pstRecParam, pstTFRECEVENT pstRecEvent);
 NK_Int TFRECORD_Destroy(HTFRECORD hRecord);
 NK_Int TFRECORD_SetParam(HTFRECORD hRecord, enTFRECOPT emType, NK_PVoid lParam);
 NK_Int TFRECORD_RecordFile_Get(HTFRECORD hRecord, NK_PChar recordFile);

////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif

#endif










