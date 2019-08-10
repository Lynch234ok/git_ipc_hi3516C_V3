

#if !defined(APP_CLOUD_REC_H)
# define APP_CLOUD_REC_H
#ifdef __cplusplus
extern "C" {
#endif

#include "netsdk_def.h"

typedef enum{
	NK_CLOUD_REC_TYPE_TIME = (1 << 0),
	NK_CLOUD_REC_TYPE_MOTION = (1 << 1),
	NK_CLOUD_REC_TYPE_ALARM = (1 << 2),
	NK_CLOUD_REC_TYPE_MANU = (1 << 3),
	NK_CLOUD_REC_TYPE_ALL = (NK_CLOUD_REC_TYPE_TIME | NK_CLOUD_REC_TYPE_MOTION | NK_CLOUD_REC_TYPE_ALARM | NK_CLOUD_REC_TYPE_MANU),
}tNK_CLOUD_REC_TYPE;

int NK_CLOUD_REC_Update(LP_NSDK_NETWORK_OSSCLD osscld);
int NK_CLOUD_REC_Start(int chn, int stream, int rec_type);
int NK_CLOUD_REC_Stop(int chn, int stream);
int NK_CLOUD_REC_Trigger(int chn, int rec_type, int rec_duration);
int NK_CLOUD_REC_Init(char *esee_id, int id_len, int max_chn);
int NK_CLOUD_REC_Deinit();

#ifdef __cplusplus
};
#endif
#endif /* NK_TFER_RECORDER_H_ */
