
#include "web_server.h"

#ifndef PIE_CAKE_H_
#define PIE_CAKE_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int PIECAKE_ptz(LP_HTTP_CONTEXT context);
extern int PIECAKE_login(LP_HTTP_CONTEXT context);

extern int PIECAKE_live_picture(LP_HTTP_CONTEXT context);
extern int PIECAKE_live_stream(LP_HTTP_CONTEXT context);

extern int PIECAKE_sdcard_media_search(LP_HTTP_CONTEXT context);
extern int PIECAKE_sdcard_media_playback(LP_HTTP_CONTEXT context);

#ifdef __cplusplus
};
#endif
#endif //PIE_CAKE_H_

