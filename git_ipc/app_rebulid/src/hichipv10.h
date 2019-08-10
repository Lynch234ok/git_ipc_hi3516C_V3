

#include "web_server.h"

#ifndef HICHIPV10_H_
#define HICHIPV10_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int HICHIPV10_get_identify(LP_HTTP_CONTEXT context);
extern int HICHIPV10_get_video_displayattr(LP_HTTP_CONTEXT context);

extern int HICHIPV10_compat(LP_HTTP_CONTEXT context);

extern int HICHIP_live_stream(LP_HTTP_CONTEXT context);


#ifdef __cplusplus
};
#endif
#endif //HICHIPV10_H_

