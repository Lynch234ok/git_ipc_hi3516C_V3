#ifndef MSG_PUSH_H
#define MSG_PUSH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


extern int Esee_msg_send(char *eseeId, char * msg, char *type, int64_t ts_s);


#ifdef __cplusplus
};
#endif

#endif//MSG_PUSH_H