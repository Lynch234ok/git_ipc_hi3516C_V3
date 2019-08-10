
#ifndef __CGI_USER_1_H__
#define __CGI_USER_1_H__

#include "http_server.h"
#define CGI_USER_VERSION "1.0"

extern int WEB_CGI_user_list(LP_HTTP_CONTEXT http_session);

//
// ?content=
// <user>
//  <add_user name="" password="" admin="" premit_live="" premit_setting="" premit_playback="" />
// </user>
//

extern int WEB_CGI_add_user(LP_HTTP_CONTEXT session);

//
// ?content=
// <user>
//  <del_user name="" />
// </user>
//

extern int WEB_CGI_del_user(LP_HTTP_CONTEXT http_session);

//
// ?content=
// <user>
//  <edit_user name="" admin="" premit_live="" premit_setting="" premit_playback="" />
// </user>
//

extern int WEB_CGI_edit_user(LP_HTTP_CONTEXT session);

//
// ?content=
// <user>
//  <set_pass old_pass="" new_pass="" />
// </user>
//
extern int WEB_CGI_user_set_password(LP_HTTP_CONTEXT http_session);

extern int WEB_CGI_user_reset(LP_HTTP_CONTEXT http_session);
extern int WEB_CGI_user_get_rand(LP_HTTP_CONTEXT http_session);


#endif //__CGI_USER_1_H__
