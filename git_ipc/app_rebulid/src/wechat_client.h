
#include "web_server.h"

#ifndef WECHAT_CLIENT_H_
#define WECHAT_CLIENT_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int WECHAT_CLIENT_put_image(void);

extern int WECHAT_CLIENT_init(char serverIP[128], int serverPort);
extern void WECHAT_CLIENT_destroy();

#ifdef __cplusplus
};
#endif
#endif //WECHAT_CLIENT_H_

