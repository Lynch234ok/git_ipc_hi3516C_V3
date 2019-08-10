#ifndef _TF_CARD_H
#define _TF_CARD_H
#include <tfcard.h>
#ifdef __cplusplus
extern "C" {
#endif

extern NK_Void TFCARD_move_record_start(NK_PChar record_type);
extern NK_Int TFCARD_init();
extern NK_Void TFCARD_destroy();

#ifdef __cplusplus
};
#endif
#endif

