#include "generic.h"
#include "inifile.h"
#include "usrm.h"
#include "jsocket.h"
#include "ezxml.h"

#include "app_debug.h"
#include "cgi_user_1.h"
#include "securedat.h"
//
// XML template
//
//

static int user_list_xml(USRM_I_KNOW_U_t* i_m, ezxml_t xml_root, lpINI_PARSER ini_list)
{
	int i = 0;
	int ret = 0;
	ezxml_t list_node = NULL;
	ezxml_t user_node = NULL;
	char str_val[32] = {""};
	
	int const user_count = ini_list->read_int(ini_list, "OPTION", "user", 0);
	//APP_ASSERT(user_count > 0, "Why there is no user");

	// xml root node
	list_node = ezxml_add_child_d(xml_root, "user_list", 0); // user list node
	// attribute count
	sprintf(str_val, "%d", user_count);
	ezxml_set_attr_d(list_node, "count", str_val);
	// attribute backlog
	sprintf(str_val, "%d", USR_MANGER_USER_HOURSE_BACKLOG);
	ezxml_set_attr_d(list_node, "backlog", str_val);

	for(i = 0; NULL != i_m && i < user_count && i < USR_MANGER_USER_HOURSE_BACKLOG; ++i){
		char user_section[32] = {""};
		char user_node_name[32] = {""};
		char his_name[32] = {""};
		char *can_del_user, *can_edit_user, *can_set_pass;
		bool his_is_admin;
		char buf[1024] = {""};

		sprintf(user_section, "USER%d", i);
		sprintf(user_node_name, "user%d", i);
		//APP_TRACE("Reading section [%s]", user_section);
		
		user_node = ezxml_add_child_d(list_node, user_node_name, i); // add user child to list
		strncpy(his_name, ini_list->read_text(ini_list, user_section, "name", "", buf, sizeof(buf)), ARRAY_ITEM(his_name));
		his_is_admin = ini_list->read_bool(ini_list, user_section, "admin", false);
		ezxml_set_attr_d(user_node, "name", his_name);
		ezxml_set_attr_d(user_node, "admin", ini_list->read_bool(ini_list, user_section, "admin", false) ? "yes" : "no");
		ezxml_set_attr_d(user_node, "permit_live", ini_list->read_bool(ini_list, user_section, "permit_live", false) ? "yes" : "no");
		ezxml_set_attr_d(user_node, "permit_setting", ini_list->read_bool(ini_list, user_section, "permit_setting", false) ? "yes" : "no");
		ezxml_set_attr_d(user_node, "permit_playback", ini_list->read_bool(ini_list, user_section, "permit_playback", false) ? "yes" : "no");
		
		// 1. only admin and not user himself could be deleted
		can_del_user = "no";
		if(i_m->is_admin && !STR_CASE_THE_SAME(i_m->username, his_name)){
			can_del_user = "yes";
		}
		// 2. only admin to edit not admin, and not user himself
		can_edit_user = "no";

		if(i_m->is_admin && !his_is_admin && !STR_CASE_THE_SAME(i_m->username, his_name)){
			can_edit_user = "yes";
		}
		// 3. only user himself could set his own password
		can_set_pass = STR_CASE_THE_SAME(i_m->username, his_name) ? "yes" : "no";

		// 4. add attributes
		ezxml_set_attr_d(user_node, "del_user", can_del_user);
		ezxml_set_attr_d(user_node, "edit_user", can_edit_user);
		ezxml_set_attr_d(user_node, "set_pass", can_set_pass);

	}

	return 0;
}

static int cgi_user_http_response(LP_HTTP_CONTEXT session,const char * xml_text)
{
	int ret = 0;
	char response_buf[4096] = {""};
	const char* const http_version =strdupa(session->request_header->version);
	const char* const http_protocol =strdupa(session->request_header->protocol);
	LP_HTTP_HEAD_FIELD http_header = NULL;

	// response
	http_header =HTTP_UTIL_new_response_header(http_protocol,http_version,200,NULL);
	if(xml_text && strlen(xml_text) > 0){
		// with content
		http_header->add_tag_text(http_header, "Content-Type", http_get_file_mime("xml"),true);
		http_header->add_tag_int(http_header, "Content-Length", strlen(xml_text),true);
	}
	http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
	http_header->free(http_header);
	http_header = NULL;

	// make a whole tcp packet
	strncat(response_buf, xml_text, ARRAY_ITEM(response_buf));
	ret = jsock_send(session->sock, response_buf, strlen(response_buf));
	if(ret < 0){
		// FIXME:
	}
	return ret;
}



int WEB_CGI_user_list(LP_HTTP_CONTEXT http_session)
{
	int i = 0;
	int ret = 0;
	char query_string[2048] = {""};
    HTTP_STR_t password = {""};
	HTTP_STR_t username = {""};
	USRM_I_KNOW_U_t* i_m = NULL;
	ezxml_t user_xml = NULL;
	const char* xml_text = NULL;
	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	LP_HTTP_TAG pCurr=NULL;
	pHead=HTTP_UTIL_parse_query_as_para(http_session->request_header->query);
	if(NULL!=pHead){
		pCurr=pHead->paras;
		while(NULL!=pCurr){
			if(0 == strcmp("username",pCurr->name)){
				username=strdupa(pCurr->val);				
			}
			if(0 == strcmp("password",pCurr->name)){
				password=strdupa(pCurr->val);
			}
			pCurr=pCurr->next;
		}
	}
	// get current session username / password
	

	// xml root node
	user_xml = ezxml_new_d("user");
	ezxml_set_attr_d(user_xml, "ver", CGI_USER_VERSION);

	// user check in

	i_m = USRM_login(username,password);
	if(i_m){
		lpINI_PARSER user_ini = NULL;
		
		// attribute count
		ezxml_set_attr_d(user_xml, "you", i_m->username);
		// attribute 'add user' permit
		ezxml_set_attr_d(user_xml, "add_user", i_m->is_admin ? "yes" : "no");
		
		// open the ini file
		user_ini = OpenIniFile(USR_MANGER_TMP_FILE);
		//APP_ASSERT(NULL != user_ini, "File not existed? it's impossible");

		// put the user list to xml
		user_list_xml(i_m, user_xml, user_ini);

		// close the ini file
		CloseIniFile(user_ini);
		user_ini = NULL;

		// add return
		ezxml_set_attr_d(user_xml, "ret", "success");
		ezxml_set_attr_d(user_xml, "mesg", "check in success");

		USRM_logout(i_m);
		i_m = NULL;
	}else{
		// add return
		ezxml_set_attr_d(user_xml, "ret", "sorry");
		ezxml_set_attr_d(user_xml, "mesg", "check in falied");
	}
	// make the xml text
	xml_text = ezxml_toxml(user_xml);
	ezxml_free(user_xml);
	user_xml = NULL;
	// response
	cgi_user_http_response(http_session, xml_text);

	// free the xml text
	free(xml_text);
	xml_text = NULL;
	

	password = NULL;
	username = NULL;

	pHead->free(pHead);
	pCurr = NULL;
	pHead = NULL;

	return 0;
}




int WEB_CGI_add_user(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	HTTP_STR_t val_username = {""};
	HTTP_STR_t val_password = {""};
	HTTP_STR_t val_content = {""};
		
	
	USRM_I_KNOW_U_t* i_m = NULL;
	bool check_in = false;
	bool add_success = false;
	ezxml_t output_xml = NULL;
	const char* xml_text = NULL;
	    
	char query_string[2048] = {""};
		
	// get current session username / password

	const char* const query_str_enc = strdupa(session->request_header->query);
	HTTP_UTIL_url_decode(query_str_enc,strlen(query_str_enc),query_string,2048);
	
	LP_HTTP_QUERY_PARA_LIST para_list = NULL;
	para_list = HTTP_UTIL_parse_query_as_para(query_string);
	if(NULL == para_list){
		printf("the list is NULL!\n");
		return -1;
	}
	LP_HTTP_TAG   p_paras = para_list->paras;
	while(NULL != p_paras){
		if(0 == strcmp("username",p_paras->name)){
			val_username = strdupa(p_paras->val);
		}
		if(0 == strcmp("password",p_paras->name)){
			val_password = strdupa(p_paras->val);
		}
		if(0 == strcmp("content",p_paras->name)){
			val_content = strdupa(p_paras->val);
		}
		p_paras = p_paras->next;
	}
	
		
	
		// user check in

	i_m = USRM_login(strdupa(val_username),strdupa(val_password));
	if(i_m){
		ezxml_t input_xml = NULL;
		ezxml_t add_node = NULL;
	
		APP_TRACE("Login success! Query string = \"%s\"", query_string);
			
	   // check in success
		check_in = true;
	
		input_xml = ezxml_parse_str(val_content,strlen(val_content));
		if(input_xml){
			USRM_HOW_ABOUT_t how_about = USRM_GREAT;
			ezxml_t add_node = ezxml_child(input_xml, "add_user");
			if(add_node){
				const char* attr_name = ezxml_attr(add_node, "name");
				const char* attr_password = ezxml_attr(add_node, "password");
				const char* attr_admin = ezxml_attr(add_node, "admin");
				const char* attr_permit_live = ezxml_attr(add_node, "permit_live");
				const char* attr_permit_setting = ezxml_attr(add_node, "permit_setting");
				const char* attr_permit_playback = ezxml_attr(add_node, "permit_playback");
				APP_TRACE("%s", attr_admin);
				APP_TRACE("%s", attr_permit_setting);
				APP_TRACE("%s", attr_permit_playback);
				
				bool const is_admin = attr_admin ? (STR_CASE_THE_SAME(attr_admin, "yes")) : false;
				uint32_t permit_flag = 0; // clear flag
	
				if(attr_permit_live ? (STR_CASE_THE_SAME(attr_permit_live, "yes")) : false){
					permit_flag |= USRM_PERMIT_LIVE;
				}
				if(attr_permit_setting ? (STR_CASE_THE_SAME(attr_permit_setting, "yes")) : false){
					permit_flag |= USRM_PERMIT_SETTING;
				}
				if(attr_permit_playback ? (STR_CASE_THE_SAME(attr_permit_playback, "yes")) : false){
					permit_flag |= USRM_PERMIT_PLAYBACK;
				}

				how_about = i_m->add_user(i_m, attr_name, attr_password, is_admin, permit_flag);
				if(USRM_GREAT == how_about){
					add_success = true;
					APP_TRACE("Add user \"%s\" success!", attr_name);
					USRM_store();
				}else{
					// FIXME:
						
					}
			}
	
			ezxml_free(input_xml);
			input_xml = NULL;
		}
	
		// check out
		USRM_logout(i_m);
		i_m = NULL;
	}	
	// make the xml content
	output_xml = ezxml_new_d("user");
	ezxml_set_attr_d(output_xml, "ver", CGI_USER_VERSION);
	ezxml_set_attr_d(output_xml, "you", strdupa(val_username));
	ezxml_set_attr_d(output_xml, "ret", "success");
	if(!STR_CASE_THE_SAME(ezxml_attr(output_xml, "ret"), "success")){
		ezxml_set_attr_d(output_xml, "mesg", "");
	}
	xml_text = ezxml_toxml(output_xml);
	ezxml_free(output_xml);
	output_xml = NULL;
	// response
	cgi_user_http_response(session, xml_text);	
	free(output_xml);
	output_xml = NULL;	


	val_username = NULL;
	val_password = NULL;
	val_content = NULL;


	para_list->free(para_list);
	para_list = NULL;
	
	return 0;
}        

int WEB_CGI_del_user(LP_HTTP_CONTEXT http_session)
{
	HTTP_STR_t username={""},password={""},content={""};
	USRM_I_KNOW_U_t* i_m = NULL;
	bool checkin_success = false;
	bool del_success = false;
	ezxml_t output_xml = NULL;
	char* response_xml = NULL;
	
	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	LP_HTTP_TAG pCurr=NULL;
	pHead=HTTP_UTIL_parse_query_as_para(http_session->request_header->query);
	APP_TRACE("Login success! http_session->request_header->query = \"%s\"", http_session->request_header->query);
	if(NULL!=pHead){
		pCurr=pHead->paras;
		while(NULL!=pCurr){
			if(0==strcmp("username",pCurr->name)){
				username=strdupa(pCurr->val);
			}
			if(0==strcmp("password",pCurr->name)){
				password=strdupa(pCurr->val);
			}
			if(0==strcmp("content",pCurr->name)){
				content=strdupa(pCurr->val);
			}
			pCurr=pCurr->next;
		}
	}
	// get current session username / password
	// user check in

	i_m = USRM_login(username, password);
	if(i_m){
		ezxml_t input_xml = NULL;
		checkin_success = true;

		input_xml = ezxml_parse_str(content, strlen(content));
		if(input_xml){
			USRM_HOW_ABOUT_t how_about = USRM_GREAT;
			ezxml_t add_node = ezxml_child(input_xml, "del_user");
			if(add_node){
				const char* attr_name = ezxml_attr(add_node, "name");

				how_about = i_m->del_user(i_m, attr_name);
				if(USRM_GREAT == how_about){
					del_success = true;
					APP_TRACE("Delete user \"%s\" success!", attr_name);
					USRM_store();
				}else{
					// FIXME:

					
				}
			}

			ezxml_free(input_xml);
			input_xml = NULL;
		}

		// check out
		USRM_logout(i_m);
		i_m = NULL;
	}

	// make the xml content
	output_xml = ezxml_new_d("user");
	ezxml_set_attr_d(output_xml, "ver", CGI_USER_VERSION);
	ezxml_set_attr_d(output_xml, "you", username);
	ezxml_set_attr_d(output_xml, "ret", "success");
	if(!STR_CASE_THE_SAME(ezxml_attr(output_xml, "ret"), "success")){
		ezxml_set_attr_d(output_xml, "mesg", "");
	}
	response_xml = ezxml_toxml(output_xml);
	ezxml_free(output_xml);
	output_xml = NULL;

	// response
	cgi_user_http_response(http_session, response_xml);
	
	free(output_xml);
	output_xml = NULL;


	username = NULL;
	password = NULL;
	content = NULL;

	pHead->free(pHead);
	pCurr = NULL;
	pHead = NULL;
	
	return 0;
}



int WEB_CGI_edit_user(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	HTTP_STR_t username={""},password={""},content={""};
	USRM_I_KNOW_U_t* i_m = NULL;
	bool check_in = false;
	bool add_success = false;
	ezxml_t output_xml = NULL;
	const char* xml_text = NULL;

	char query_string[2048] = {""};

	// get current session username / password
	const char* const query_str_enc = strdupa(session->request_header->query);
	HTTP_UTIL_url_decode(query_str_enc,strlen(query_str_enc),query_string,2048);
	
	
	LP_HTTP_QUERY_PARA_LIST para_list = NULL;
	para_list = HTTP_UTIL_parse_query_as_para(query_string);
	if(NULL == para_list){
		printf("the list is NULL!\n");
		return -1;
	}
	LP_HTTP_TAG	 p_paras = para_list->paras;
	while(NULL != p_paras){
		if(0 == strcmp("username",p_paras->name)){
			username = strdupa(p_paras->val);
		}
		if(0 == strcmp("password",p_paras->name)){
			password = strdupa(p_paras->val);
		}
		if(0 == strcmp("content",p_paras->name)){
			content = strdupa(p_paras->val);
		}
		p_paras = p_paras->next;
	}
	// user check in


	// user check in
	i_m = USRM_login(username, password);
	if(i_m){
		ezxml_t input_xml = NULL;
		ezxml_t add_node = NULL;

		APP_TRACE("Login success! Query string = \"%s\"", query_string);
		
		// check in success
		check_in = true;

		input_xml = ezxml_parse_str(content, strlen(content));
		if(input_xml){
			USRM_HOW_ABOUT_t how_about = USRM_GREAT;
			ezxml_t edit_node = ezxml_child(input_xml, "edit_user");
			if(edit_node){
				const char* attr_name = ezxml_attr(edit_node, "name");
				const char* attr_admin = ezxml_attr(edit_node, "admin");
				const char* attr_permit_live = ezxml_attr(edit_node, "permit_live");
				const char* attr_permit_setting = ezxml_attr(edit_node, "permit_setting");
				const char* attr_permit_playback = ezxml_attr(edit_node, "permit_playback");
				APP_TRACE("%s", attr_admin);
				APP_TRACE("%s", attr_permit_setting);
				APP_TRACE("%s", attr_permit_playback);
				
				bool const is_admin = attr_admin ? (STR_CASE_THE_SAME(attr_admin, "yes")) : false;
				uint32_t permit_flag = 0; // clear flag

				if(attr_permit_live ? (STR_CASE_THE_SAME(attr_permit_live, "yes")) : false){
					permit_flag |= USRM_PERMIT_LIVE;
				}
				if(attr_permit_setting ? (STR_CASE_THE_SAME(attr_permit_setting, "yes")) : false){
					permit_flag |= USRM_PERMIT_SETTING;
				}
				if(attr_permit_playback ? (STR_CASE_THE_SAME(attr_permit_playback, "yes")) : false){
					permit_flag |= USRM_PERMIT_PLAYBACK;
				}
				APP_TRACE("flag:%x", permit_flag);
				how_about = i_m->edit_user(i_m, attr_name, is_admin, permit_flag);
				if(USRM_GREAT == how_about){
					add_success = true;
					APP_TRACE("Edit user \"%s\" success!", attr_name);
					USRM_store();
				}else{
					// FIXME:

					
				}
			}

			ezxml_free(input_xml);
			input_xml = NULL;
		}

		// check out
		USRM_logout(i_m);
		i_m = NULL;
	}

	// make the xml content
	output_xml = ezxml_new_d("user");
	ezxml_set_attr_d(output_xml, "ver", CGI_USER_VERSION);
	ezxml_set_attr_d(output_xml, "you", strdupa(username));
	ezxml_set_attr_d(output_xml, "ret", "success");
	if(!STR_CASE_THE_SAME(ezxml_attr(output_xml, "ret"), "success")){
		ezxml_set_attr_d(output_xml, "mesg", "");
	}
	xml_text = ezxml_toxml(output_xml);
	ezxml_free(output_xml);
	output_xml = NULL;

	// response
	cgi_user_http_response(session, xml_text);
	
	free(output_xml);
	output_xml = NULL;

	username = NULL;
	password = NULL;
	content = NULL;
	para_list->free(para_list);
	p_paras = NULL;
	para_list = NULL;
	
	return 0;
}



int WEB_CGI_user_set_password(LP_HTTP_CONTEXT http_session)
{
	int ret = 0;
	HTTP_STR_t username={""},password={""},content={""};
	USRM_I_KNOW_U_t* i_m = NULL;
	bool checkin_success = false;
	bool set_success = false;
	ezxml_t output_xml = NULL;
	char* response_xml = NULL;
	
	char query_string[2048] = {""};
	
	// get current session username / password
	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	LP_HTTP_TAG pCurr=NULL;
	pHead=HTTP_UTIL_parse_query_as_para(http_session->request_header->query);
	APP_TRACE("Login success! http_session->request_header->query = \"%s\"", http_session->request_header->query);
	if(NULL!=pHead){
		pCurr=pHead->paras;
		while(NULL!=pCurr){
			if(0==strcmp("username",pCurr->name)){
				username=strdupa(pCurr->val);
			}
			if(0==strcmp("password",pCurr->name)){
				password=strdupa(pCurr->val);
			}
			if(0==strcmp("content",pCurr->name)){
				content=strdupa(pCurr->val);
			}
			pCurr=pCurr->next;
		}
	}
	// user check in

	i_m = USRM_login(username, password);
	if(i_m){
		ezxml_t input_xml = NULL;
		checkin_success = true;
		APP_TRACE("Login success! Query string = \"%s\"", query_string);

		input_xml = ezxml_parse_str(content, strlen(content));
		if(input_xml){
			USRM_HOW_ABOUT_t how_about = USRM_GREAT;
			ezxml_t add_node = ezxml_child(input_xml, "set_pass");
			if(add_node){
				const char* attr_old_pass = ezxml_attr(add_node, "old_pass");
				const char* attr_new_pass = ezxml_attr(add_node, "new_pass");

				how_about = i_m->set_password(i_m, attr_old_pass, attr_new_pass);
				if(USRM_GREAT == how_about){
					set_success = true;
					APP_TRACE("Set user \"%s\" password success!", strdupa(username));
					USRM_store();
				}else{
					// FIXME:
				}
			}

			ezxml_free(input_xml);
			input_xml = NULL;
		}

		// check out
		USRM_logout(i_m);
		i_m = NULL;
	}

	// make the xml content
	output_xml = ezxml_new_d("user");
	ezxml_set_attr_d(output_xml, "ver", CGI_USER_VERSION);
	ezxml_set_attr_d(output_xml, "you", strdupa(username));
	ezxml_set_attr_d(output_xml, "ret", "success");
	if(!STR_CASE_THE_SAME(ezxml_attr(output_xml, "ret"), "success")){
		ezxml_set_attr_d(output_xml, "mesg", "");
	}
	response_xml = ezxml_toxml(output_xml);
	ezxml_free(output_xml);
	output_xml = NULL;

	// response
	cgi_user_http_response(http_session, response_xml);
	
	free(output_xml);
	output_xml = NULL;


	username = NULL;	
	password = NULL;
	content = NULL;
	pHead->free(pHead);
	pCurr = NULL;
	pHead = NULL;
	
	return 0;
}


int WEB_CGI_user_reset(LP_HTTP_CONTEXT http_session)
{
	int i = 0;
	int ret = 0;
	char query_string[2048] = {""};
    HTTP_STR_t password = {""};
	HTTP_STR_t username = {""};
	USRM_I_KNOW_U_t* i_m = NULL;
	ezxml_t user_xml = NULL;
	const char* xml_text = NULL;
	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	LP_HTTP_TAG pCurr=NULL;
	pHead=HTTP_UTIL_parse_query_as_para(http_session->request_header->query);
	if(NULL!=pHead){
		pCurr=pHead->paras;
		while(NULL!=pCurr){
			if(0 == strcmp("username",pCurr->name)){
				username=strdupa(pCurr->val);				
			}
			if(0 == strcmp("password",pCurr->name)){
				password=strdupa(pCurr->val);
			}
			pCurr=pCurr->next;
		}
	}
	// get current session username / password
	

	// xml root node
	user_xml = ezxml_new_d("user");
	ezxml_set_attr_d(user_xml, "ver", CGI_USER_VERSION);

	// user check in
	APP_TRACE("%s--%s", username, password);
	i_m = USRM_login(username,password);
	if(i_m){
		if(i_m->reset_user){
			i_m->reset_user();
			ezxml_set_attr_d(user_xml, "ret", "success");
			ezxml_set_attr_d(user_xml, "mesg", "");
		}else{
			//permit limited
			ezxml_set_attr_d(user_xml, "ret", "failed");
			ezxml_set_attr_d(user_xml, "mesg", "permit limited");
		}

		USRM_logout(i_m);
		i_m = NULL;
	}else{
		// add return
		ezxml_set_attr_d(user_xml, "ret", "failed");
		ezxml_set_attr_d(user_xml, "mesg", "check in falied");
	}
	// make the xml text
	xml_text = ezxml_toxml(user_xml);
	ezxml_free(user_xml);
	user_xml = NULL;
	// response
	cgi_user_http_response(http_session, xml_text);

	// free the xml text
	free(xml_text);
	xml_text = NULL;
	

	password = NULL;
	username = NULL;

	pHead->free(pHead);
	pCurr = NULL;
	pHead = NULL;
	exit(0);
	return 0;
}

extern char rand_num[3];

int WEB_CGI_user_get_rand(LP_HTTP_CONTEXT http_session)
{
	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	LP_HTTP_TAG pCurr=NULL;
	HTTP_STR_t sn={""};
	char ucode[32];
	if(0 == UC_SNumberGet(ucode)) {
		pHead=HTTP_UTIL_parse_query_as_para(http_session->request_header->query);
		if(NULL!=pHead){
			pCurr=pHead->paras;
			while(NULL!=pCurr){
				if(0 == strcmp("sn",pCurr->name)){
					sn=strdupa(pCurr->val);				
				}
				pCurr=pCurr->next;
			}
			if(!strcmp(ucode, sn)){
				cgi_user_http_response(http_session, rand_num);
			}else{
				cgi_user_http_response(http_session, "sn_error");
			}
		}
	}else{
		cgi_user_http_response(http_session, "no_sn");
	}
	sn = NULL;	
	pHead->free(pHead);
	return 0;
}

