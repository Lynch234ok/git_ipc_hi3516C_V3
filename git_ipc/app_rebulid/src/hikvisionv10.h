
#include "http_server.h"
#include "ezxml.h"

extern ezxml_t HIKv10_xs_string(const char *name, const char *str);
extern ezxml_t HIKv10_xs_integer(const char *name, int content);
extern ezxml_t HIKv10_xs_boolean(const char *name, bool content);

extern int HIKv10_set_namespace(ezxml_t xml);
extern int HIKv10_set_version(ezxml_t xml);


extern int HIKv10_response(int sock, HTTP_CSTR_t content);





extern int HIKVISIONv10_video_inputs(LP_HTTP_CONTEXT context);


extern int HIKVISIONv10_streaming_channels(LP_HTTP_CONTEXT context);
extern int HIKVISIONv10_streaming_channels_id(LP_HTTP_CONTEXT context);
extern int HIKVISIONv10_streaming_channels_id_capabilities(LP_HTTP_CONTEXT context);
extern int HIKVISIONv10_streaming_channels_id_status(LP_HTTP_CONTEXT context);
extern int HIKVISIONv10_streaming_channels_id_picture(LP_HTTP_CONTEXT context);
extern int HIKVISIONv10_streaming_channels_id_requestkeyframe(LP_HTTP_CONTEXT context);

extern int HIKVISIOv10_init();
extern void HIKVISIONV10_destroy();


