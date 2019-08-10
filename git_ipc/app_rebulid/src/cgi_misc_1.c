
#include <md5sum.h>
#include "generic.h"
#include "sdk/sdk_api.h"
#include "smtp.h"
#include "ezxml.h"
#include "app_debug.h"
#include "cgi_bin.h"
#include "netsdk.h"

#include "sensor.h"

 int WEB_CGI_moo(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char moo_header[1024];
	const char moo_content[] =
	{
		"          (__) "CRLF
		"          (oo)  "CRLF
		"    /------\\/  "CRLF
		"   /  |   ||    "CRLF
		" *   /\\---/\\    "CRLF
		"     ~~    ~~   "CRLF
		"....\"Have you mooed today?\"..."CRLF
	};
	LP_HTTP_HEAD_FIELD http_header = NULL;

	http_header = HTTP_UTIL_new_response_header("HTTP","1.1" , 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "text/plain",true);
	http_header->add_tag_int(http_header, "Content-Length", strlen(moo_content),true);
	http_header->to_text(http_header, moo_header, sizeof(moo_header));
	http_header->free(http_header);
	http_header = NULL;

	ret = send(session->sock, moo_header, strlen(moo_header), 0);
	if(strlen(moo_header) == ret){
		ret = send(session->sock, moo_content, strlen(moo_content), 0);
		if(strlen(moo_content) == ret){
			return 0;
		}
	}
	return 0;
}

int WEB_CGI_whoami(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char response_msg[1024];
	char content_buf[4096];
	LP_HTTP_HEAD_FIELD http_header = NULL;

	char* const http_version = strdupa(session->request_header->version);
	char* const http_protocol = strdupa(session->request_header->protocol);

	// reponse the request line
	strncpy(content_buf, "<table border=3 "
		"width=\"80%\" "
		"height=\"60\" "
		"align=\"center\" "
		"bordercolor=#336699 >", ARRAY_ITEM(content_buf));

	strncat(content_buf, "<tr><td align=\"center\" colspan=\"2\">", ARRAY_ITEM(content_buf));
	ret = snprintf(content_buf + strlen(content_buf), ARRAY_ITEM(content_buf) - strlen(content_buf),
		"%s %s %s",
		strdupa(session->request_header->method),strdupa(session->request_header->uri),strdupa(session->request_header->version));
	    strncat(content_buf, "</td></tr>", ARRAY_ITEM(content_buf));

		LP_HTTP_TAG p_tags = NULL;
		if(NULL != session->request_header->tags){
			p_tags = session->request_header->tags;
			while(NULL != p_tags){
				strncat(content_buf, "<tr>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "<td align=\"center\" width=\"25%\">", ARRAY_ITEM(content_buf));
				strncat(content_buf, p_tags->name, ARRAY_ITEM(content_buf));
				strncat(content_buf, "</td>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "<td align=\"center\">", ARRAY_ITEM(content_buf));
				strncat(content_buf,p_tags->val, ARRAY_ITEM(content_buf));
				strncat(content_buf, "</td>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "</tr>", ARRAY_ITEM(content_buf));
				p_tags = p_tags->next;
			}
		}
	strncat(content_buf, "</table>", ARRAY_ITEM(content_buf));


	http_header = HTTP_UTIL_new_response_header(http_protocol,http_version, 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "text/html; charset=UTF-8",true);
	http_header->add_tag_int(http_header, "Content-Length", strlen(content_buf),true);
	http_header->to_text(http_header, response_msg, sizeof(response_msg));
	http_header->free(http_header);
	http_header = NULL;

	ret = send(session->sock, response_msg, strlen(response_msg), 0);
	if(ret < 0){
		// FIXME:
	}
	APP_TRACE("%s", content_buf);
	
	ret = send(session->sock, content_buf, strlen(content_buf), 0);
	if(ret < 0){
		// FIXME:
	}

	do
	{
		char query_string[4096];
		ret = snprintf(query_string, sizeof(query_string),
			"target=lawishere@yeah.net&subject=Who are you?&content=%s&snapshot=yes&vin=0&size=320x180",
			content_buf);
		//http_fool_style_request("127.0.0.1", 80, "GET", "/email", query_string, "1.1", NULL, 0);
	}while(0);
	return 0;
}


int WEB_CGI_shell(LP_HTTP_CONTEXT session){
	if(NULL == session->request_header->query){
		return -1;
	}
	const char* const query_string = strdupa(session->request_header->query);
	char* const shell_cmd = alloca(strlen(query_string));
	
    if(HTTP_UTIL_url_decode(query_string, strlen(query_string), shell_cmd, strlen(query_string)) > 0){
		int ret = 0;
		FILE* fid = popen(shell_cmd, "r");
		if(fid > 0){
			int fid_no = fileno(fid);
			fd_set read_fds;
			struct timeval read_timeo;
		
			char response_header[1024] = {""};
			char response_content[4096] = {""};
			
			FD_ZERO(&read_fds);
			FD_SET(fid_no, &read_fds);
			read_timeo.tv_sec = session->keep_alive / 2;
			read_timeo.tv_usec = 0;
			
			ret = select(fid_no + 1, &read_fds, NULL, NULL, &read_timeo);
			if(ret <= 0){
				// do something
			}else{
				const char* const http_version = strdupa(session->request_header->version);
			    const char* const http_protocol = strdupa(session->request_header->protocol);
			    LP_HTTP_HEAD_FIELD  http_header = NULL;
				while(fgets(response_content + strlen(response_content),
					ARRAY_ITEM(response_content) - strlen(response_content), fid) != NULL);
                http_header = HTTP_UTIL_new_response_header( http_protocol,http_version,200,NULL);
				http_header->add_tag_text(http_header, "Content-Type", "text/plain",true);
				http_header->add_tag_int(http_header, "Content-Length", strlen(response_content),true);
				http_header->to_text(http_header, response_header, ARRAY_ITEM(response_header));
			    http_header->free(http_header);
				http_header = NULL;

				// response http header
				ret = send(session->sock, response_header, strlen(response_header), 0);
				if(strlen(response_header) == ret){
					// response content type
					ret = send(session->sock, response_content, strlen(response_content), 0);
				}
			}
			pclose(fid);
			fid = NULL;
			APP_TRACE("Shell \"%s\"", shell_cmd);
			free(shell_cmd);
			return 0;
		}
	}
	return -1;	
}

static int WEB_CGI_capture_a_jpeg(int vin, int width, int height, const char* file_name)
{
	FILE* fid = NULL;
	int file_size = -1;
	fid = fopen(file_name, "w+b");
	if(NULL != fid){
		if(0 == sdk_enc->snapshot(vin, kSDK_ENC_SNAPSHOT_QUALITY_HIGH, width, height, fid)){
			GET_FILE_SIZE(file_name, file_size);
		}
		fclose(fid);
	}
	return file_size;
}
int WEB_CGI_snapshot(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char jpeg_name[128];
	LP_HTTP_TAG pSize=NULL, pDownload=NULL,pCurr=NULL;
	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	int width = 640, height = 360;
	bool is_download = false;
	ssize_t file_size = 0;
	const char *querySize = NULL, *queryDownload = NULL;
	pHead= HTTP_UTIL_parse_query_as_para(session->request_header->query);

	querySize = pHead->read(pHead, "size");
	if(NULL != querySize){
		sscanf(querySize, "%dx%d", &width, &height);
	}
	APP_TRACE("SIZE:%d:%d", width, height);
	queryDownload = pHead->read(pHead, "download");
	if(NULL != queryDownload){
		if(0 == strcasecmp("yes", queryDownload) || 0 == strcasecmp("1", queryDownload)){
			is_download = true;
		}
	}

	// generate a jpeg file name
	ret = snprintf(jpeg_name, ARRAY_ITEM(jpeg_name), "/tmp/%d%d.jpg", rand(), rand());
	file_size = WEB_CGI_capture_a_jpeg(0, width, height, jpeg_name);
	if(file_size > 0){
		const char* const http_version = session->request_header->version;
		LP_HTTP_HEAD_FIELD http_header = NULL;
		char response_buf[1024];
		http_header = HTTP_UTIL_new_response_header("HTTP", http_version, 200, NULL);
		http_header->add_tag_text(http_header, "Content-Type", HTTP_UTIL_file_mime("jpeg"),true);
		if(is_download){
			char disposition[128];
			struct tm tm_now;
			time_t t = time(NULL);
			ST_NSDK_NETWORK_INTERFACE lan0;
			NETSDK_conf_interface_get(1, &lan0);
		
			localtime_r(&t, &tm_now);
			sprintf(disposition, "attachment; filename=\"%s %04d-%02d-%02d %02d-%02d-%02d.jpg", 
				lan0.lan.staticIP, 
				tm_now.tm_year + 1900,
				tm_now.tm_mon + 1,
				tm_now.tm_mday,
				tm_now.tm_hour,
				tm_now.tm_min,
				tm_now.tm_sec);
			http_header->add_tag_text(http_header, "Content-Disposition", disposition, true);
		}
		http_header->add_tag_int(http_header, "Content-Length", file_size,true);
		http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
		http_header->free(http_header);
		http_header = NULL;
		ret = send(session->sock, response_buf, strlen(response_buf), 0);
		if(ret > 0){
			FILE* fid = fopen(jpeg_name, "rb");
			if(NULL != fid){
				fseek(fid, 0, SEEK_SET);
				while((ret = fread(response_buf, 1, ARRAY_ITEM(response_buf), fid)) > 0){
					ret = send(session->sock, response_buf, ret, 0);
					if(ret < 0){
						break;
					}
				}
				fclose(fid);
				APP_TRACE("SEND JPG END");
				fid = NULL;
			}
		}
	}
	unlink(jpeg_name);
	remove(jpeg_name);

	pHead->free(pHead);
	pHead = NULL;
	pSize = NULL;
	pDownload = NULL;
	pCurr = NULL;

	
	return 0;
}

static int WEB_CGI_svg_thermometer(int range_low, int range_high, float temperature, void *buf, size_t stack_len)
{
	int buf_size = 0;
	int const svg_width = 600;
	int const svg_height = 800;
	int const datum_x = 100;
	int const datum_y = 500;
	int const mometer_width = 50;
	int const mometer_height = 400;
	char svg_buf[4096] = {""};
	
	int temperature_zero = (int)((float)mometer_height * (float)(0 - range_low) / (float)(range_high - range_low));
	int temperature_value = (int)((float)mometer_height * (float)(temperature - range_low) / (float)(range_high - range_low));

	char svg_base[] =
		"<svg xmlns='http://www.w3.org/2000/svg' version='1.1'>"
			"<defs>"
				"<filter id='f1' x='0' y='0'>"
					"<feOffset result='offOut' in='SourceAlpha' dx='5' dy='5'/>"
					"<feGaussianBlur result='blurOut' in='offOut' stdDeviation='5'/>"
					"<feBlend in='SourceGraphic' in2='blurOut' mode='normal'/>"
				"</filter>"
			"</defs>"
			"<path id='cycle' fill='red' stroke='black' stroke-width='5' filter='url(#f1)' />"
			"<path id='rect' fill='none' stroke='black' stroke-width='5' />"
			"<path id='scale0' stroke='black' stroke-width='2' />"
			"<text id='label0' x='118' y='422' fill='black' font-size='20px'>0</text>"
			"<path id='scale' fill='none' stroke='black' stroke-width='2'></path>"
			"<text id='label' x='155' y='312' fill='black' font-size='24px' filter='url(#f1)'>0 dC</text>"
		"</svg>";

	// root node
	ezxml_t xml_svg = ezxml_parse_str(svg_base, sizeof(svg_base));
	if(NULL != xml_svg){
		char *xml_text = NULL;
		ezxml_t xml_svg_cycle = ezxml_child(xml_svg, "path");
		ezxml_t xml_svg_rect = ezxml_next(xml_svg_cycle);
		ezxml_t xml_svg_scale0 = ezxml_next(xml_svg_rect);
		ezxml_t xml_svg_scale = ezxml_next(xml_svg_scale0);
		
		ezxml_t xml_svg_label0 = ezxml_child(xml_svg, "text");
		ezxml_t xml_svg_label = ezxml_next(xml_svg_label0);

		// xml svg
		snprintf(svg_buf, sizeof(svg_buf), "%dpx", svg_width);
		ezxml_set_attr_d(xml_svg, "width", svg_buf);
		snprintf(svg_buf, sizeof(svg_buf), "%dpx", svg_height);
		ezxml_set_attr_d(xml_svg, "height", svg_buf);

		// xml svg cycle
		snprintf(svg_buf, sizeof(svg_buf), "M%d,%d v-%d h%d v%d a%d,%d 1 1,1 -%d,0 z",
			datum_x, datum_y, temperature_value,
			mometer_width,
			temperature_value,
			mometer_width, mometer_width,
			mometer_width);
		ezxml_set_attr_d(xml_svg_cycle, "d", svg_buf);

		// xml svg rect
		snprintf(svg_buf, sizeof(svg_buf), "M%d,%d v-%d a%d,%d 1 1,1 %d,0 v%d",
			datum_x, datum_y - temperature_value,
			mometer_height - temperature_value,
			mometer_width / 2, mometer_width / 2, mometer_width,
			mometer_height - temperature_value);
		ezxml_set_attr_d(xml_svg_rect, "d", svg_buf);

		// xml svg scale0
		snprintf(svg_buf, sizeof(svg_buf), "M%d,%d h%d ",
			datum_x, datum_y - temperature_zero, mometer_width / 3);
		ezxml_set_attr_d(xml_svg_scale0, "d", svg_buf);

		// xml svg label0
		snprintf(svg_buf, sizeof(svg_buf), "%d", datum_x + mometer_width / 3 + 2);

		
		do{
			// xml svg scale
			int i = 0;
			int scale_distance = mometer_height - temperature_zero;
			int scale_10step = (float)(mometer_height * 10) / (float)(range_high - range_low);
			int scale_items = scale_distance / scale_10step;

			strcpy(svg_buf, "");
			for(i = 0; i < scale_items; ++i){
				int const off_x = datum_x;
				int const off_y = datum_y - temperature_zero - i * scale_10step;
				if(off_y < datum_y){
					snprintf(svg_buf + strlen(svg_buf), sizeof(svg_buf) - strlen(svg_buf),
						"M%d,%d h%d ", off_x, off_y, mometer_width / 4);
				}
			}
			ezxml_set_attr_d(xml_svg_scale, "d", svg_buf);

			// xml svg label0
			snprintf(svg_buf, sizeof(svg_buf), "%d", datum_x + mometer_width + 5);
			ezxml_set_attr_d(xml_svg_label, "x", svg_buf);
			snprintf(svg_buf, sizeof(svg_buf), "%d", datum_y - temperature_value);
			ezxml_set_attr_d(xml_svg_label, "y", svg_buf);
			snprintf(svg_buf, sizeof(svg_buf), "%.3f dC", temperature);
			ezxml_set_txt_d(xml_svg_label, svg_buf);
			
		}while(0);

		xml_text = ezxml_toxml(xml_svg);
		ezxml_free(xml_svg);
		xml_svg = NULL;

		//APP_TRACE("svg: %s", xml_text);
		buf_size = snprintf(buf, stack_len, "%s", xml_text);
		
		free(xml_text);
		xml_text = NULL;

		return buf_size;
	}

	return -1;
}



int WEB_CGI_cpu_temperature(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char response_buf[1024] = {""};
	char response_content[4096] = {""};
	int content_len = 0;
	LP_HTTP_HEAD_FIELD http_header = NULL;
	float rand_val = rand() % 1000;
	float rand_suffix = (rand_val / 1000.0) * 0.05 * (rand() % 2 ? 1 : -1);
	float soc_temperature = sdk_sys->temperature() + rand_suffix;
	content_len = WEB_CGI_svg_thermometer(-40, 140, soc_temperature, response_content, sizeof(response_content));
	http_header = HTTP_UTIL_new_response_header(session->request_header->protocol,session->request_header->version, 200, NULL);

	
	http_header->add_tag_text(http_header, "Content-Type", "image/svg+xml",true);
	http_header->add_tag_int(http_header, "Content-Length", content_len,true);
	http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
	http_header->free(http_header);
	http_header = NULL;

	ret = send(session->sock, response_buf, strlen(response_buf), 0);
	if(strlen(response_buf) == ret){
		ret = send(session->sock, response_content, strlen(response_content), 0);
		if(strlen(response_content) == ret){
			return 0;
		}
	}
	return -1;
}


int WEB_CGI_mjpeg_html(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char response_buf[1024];
	char response_content[1024];

	strncpy(response_content,
		"<body>"CRLF
		"<img src=\"/mjpeg?size=640x360&fps=15\" />"CRLF
		"</body>", ARRAY_ITEM(response_content));
	
	ret = snprintf(response_buf, ARRAY_ITEM(response_buf),
		"HTTP/%s 200 OK"CRLF
		"Content-Type: text/html"CRLF
		"Content-Length: %d"CRLF
		CRLF
		"%s",
	    strdupa(session->request_header->version),
		strlen(response_content),
		response_content);

	ret = send(session->sock, response_buf, strlen(response_buf), 0);
	return 0;
}



int WEB_CGI_mjpeg(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char response_buf[1024];
	LP_HTTP_QUERY_PARA_LIST  p_para = NULL;	
	
	int width = 640, height = 360;
	int n_fps = 15, n_f_time = 0;;
	ssize_t file_size = 0;
	p_para = HTTP_UTIL_parse_query_as_para(session->request_header->query);	 
	if(NULL != p_para){
	HTTP_CSTR_t val = strdupa(p_para->read(p_para,"size"));
	if(NULL != val ){
		const char* const str_size = strdupa(val);
		if(2 != sscanf(str_size,"%dx%d", &width, &height)){
			width = 640;
			height = 320;
		}
	}
    HTTP_CSTR_t val2 = strdupa(p_para->read(p_para,"fps"));
	if(NULL != val2){
		n_fps= atoi(strdupa(val2));
		if(n_fps > 30){
			n_fps = 15;
		}
	}
		}

	n_f_time = (1000000 + n_fps - 1) / n_fps;

	ret = snprintf(response_buf, ARRAY_ITEM(response_buf),
		"HTTP/%s 200 OK"CRLF
		"Content-Type: multipart/x-mixed-replace;boundary=ipcamera"CRLF
		CRLF,
	    strdupa(session->request_header->version));

	ret = send(session->sock, response_buf, strlen(response_buf), 0);
	if(ret > 0){
		while(*session->trigger)
		{
			FILE* fid = NULL;
			char jpeg_name[128];
			ret = snprintf(jpeg_name, ARRAY_ITEM(jpeg_name), "/tmp/%d%d.jpg", rand(), rand());
			file_size = WEB_CGI_capture_a_jpeg(0, width, height, jpeg_name);
			if(file_size > 0){
				ret = snprintf(response_buf, ARRAY_ITEM(response_buf),
					"--ipcamera"CRLF
					"Content-Type: image/jpeg"CRLF
					"Content-Length: %d"CRLF
					CRLF, file_size);

				ret = send(session->sock, response_buf, strlen(response_buf), 0);
				if(ret > 0){
					fid = fopen(jpeg_name, "rb");
					if(NULL != fid){
						while((ret = fread(response_buf, 1, ARRAY_ITEM(response_buf), fid)) > 0){
							ret = send(session->sock, response_buf, ret, 0);
							if(ret < 0){
								break;
							}
						}
					}
				}
			}

			unlink(jpeg_name);
			remove(jpeg_name);

			if(ret < 0){
				break;
			}
			usleep(n_f_time); 
		}

		if(ret > 0){
			strcpy(response_buf, "0"CRLF);
			ret = send(session->sock, response_buf, strlen(response_buf), 0);
			return 0;
		}
	}

	p_para->free(p_para);
	p_para = NULL;

	return -1;
}


// server
// port
// usermail
// password
// target
// subject
// content
// snapshot : yes / no
// vin :
// size :






int WEB_CGI_sdk_reg_rw(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char response_header[1024] = {""};
	char response_content[1024] = {""};
	LP_HTTP_HEAD_FIELD http_header = NULL;
    HTTP_STR_t val_addr = strdupa("0x00000000");
    
	
	uint32_t reg_addr = 0x200f0018;
	uint32_t reg_val = 0;

    LP_HTTP_QUERY_PARA_LIST  para_list=NULL;
    para_list = HTTP_UTIL_parse_query_as_para(session->request_header->query);
	if(NULL!=para_list){
	val_addr = para_list->read(para_list,"addr");
	
  }
	// read reg
	sdk_sys->read_reg(reg_addr, &reg_val);
	ret = snprintf(response_content, ARRAY_ITEM(response_content),
		"0x%08X", reg_val);
	
    http_header = HTTP_UTIL_new_response_header(session->request_header->protocol,session->request_header->version,200,NULL);
	
	http_header->add_tag_text(http_header, "Content-Type", "text/plain",true);
	http_header->add_tag_int(http_header, "Content-Length", strlen(response_content),true);
	http_header->to_text(http_header, response_header, ARRAY_ITEM(response_header));
    http_header->free(http_header);
	http_header = NULL;
	
	para_list->free(para_list);
	para_list = NULL;
	val_addr = NULL;

	ret = send(session->sock, response_header, strlen(response_header), 0);
	ret = send(session->sock, response_content, strlen(response_content), 0);

	return 0;
}

int WEB_CGI_focus_measure(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char buf[1024] = {""};
	LP_HTTP_HEAD_FIELD http_header = NULL;
  
    HTTP_STR_t  p_vin = NULL;
	HTTP_STR_t  p_algorithm = strdupa("auto");
	HTTP_STR_t  p_alg = NULL;
    LP_HTTP_QUERY_PARA_LIST  para_list = NULL;
	enSDK_VIN_FOCUS_MEASURE_ALG focus_alg = kSDK_VIN_FOCUS_MEASURE_ALG_SQUARED_GRADIENT;
	int focus_measure = 0;
	char str_measure[32] = {""};

	// read the channel
    para_list = HTTP_UTIL_parse_query_as_para(session->request_header->query);
	if(NULL == para_list){
		printf("ERROR:The query is NULL!\n");	
		return -1;
	}
	if(NULL != para_list){
		 p_vin = para_list->read(para_list,"vin");
         p_algorithm = para_list->read(para_list,"algorithm");
		 p_alg = para_list->read(para_list,"alg");
         if(NULL == p_vin){	
         	printf("Please input the parameter : vin!\n");
			return -1;
		 }
         if(NULL == p_algorithm && NULL ==p_alg){
		 	printf("Please input the parameter : algorithm!\n");
			return -1;
		 }
	 
		 if(0 == p_algorithm || 0 == p_alg){
		 	if( strcasecmp(p_algorithm,"vollath5")){
				focus_alg = kSDK_VIN_FOCUS_MEASURE_ALG_VOLLATH5;
			}else{
			    focus_alg = kSDK_VIN_FOCUS_MEASURE_ALG_SQUARED_GRADIENT;
			}	
		 }	
	}
	
	// read the algorithm
	if(NULL != sdk_vin){
	    focus_measure = sdk_vin->focus_measure(atoi(strdupa(p_vin)),focus_alg);
	}
	sprintf(str_measure, "%d", focus_measure);
	
	http_header = HTTP_UTIL_new_response_header(session->request_header->protocol,session->request_header->version,200,NULL);
	http_header->add_tag_text(http_header, "Content-Type", "text/plain",true);
	http_header->add_tag_int(http_header, "Content-Length", strlen(str_measure),true);
	http_header->to_text(http_header, buf, ARRAY_ITEM(buf));
    http_header->free(http_header);	
	http_header = NULL;

	para_list->free(para_list);
	para_list = NULL;
	free(p_vin);
	p_vin = NULL;
	free(p_alg);
	p_alg = NULL;
	free(p_algorithm);
	p_algorithm = NULL;
	

	strcat(buf, str_measure);
	ret = send(session->sock, buf, strlen(buf), 0);
	return 0;
}

int WEB_CGI_get_dana_id_QRCode(LP_HTTP_CONTEXT session)
{
	int ret = 0;
	char file_name[128] = "/tmp/dana_id.bmp";
	char response_buf[1024];
	FILE* fid = fopen(file_name, "rb");
	ssize_t file_size = 0;
	LP_HTTP_HEAD_FIELD http_header = NULL;
	const char* const http_version = session->request_header->version;
	if(fid){
		GET_FILE_SIZE(file_name, file_size);
		if(file_size > 0){
			http_header = HTTP_UTIL_new_response_header("HTTP", http_version, 200, NULL);
			http_header->add_tag_text(http_header, "Content-Type", HTTP_UTIL_file_mime("bmp"),true);
			http_header->add_tag_int(http_header, "Content-Length", file_size,true);
			http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
			http_header->free(http_header);
			http_header = NULL;
			ret = send(session->sock, response_buf, strlen(response_buf), 0);
			if(ret > 0){
				fseek(fid, 0, SEEK_SET);
				while((ret = fread(response_buf, 1, ARRAY_ITEM(response_buf), fid)) > 0){
					ret = send(session->sock, response_buf, ret, 0);
					if(ret < 0){
						break;
					}
				}
			}
		}
		fclose(fid);
		fid = NULL;
	}
	else{//file not exist
		http_header = HTTP_UTIL_new_response_header("HTTP", http_version, 404, NULL);
		http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
		http_header->free(http_header);
		http_header = NULL;
		ret = send(session->sock, response_buf, strlen(response_buf), 0);
	}

	APP_TRACE("GET dana QRCode");
	return 0;
}

#if defined(DANA_P2P)
#define WEB_CGI_DANALE_CONF "/media/conf/danale.conf"
int web_cgi_get_dana_uid(LP_HTTP_CONTEXT session)
{
	ssize_t ret = 0;
	char *file_name = WEB_CGI_DANALE_CONF;
	char response_buf[1024];
	FILE* fid = NULL;
	ssize_t file_size = 0;
	LP_HTTP_HEAD_FIELD http_header = NULL;
	const char* const http_version = session->request_header->version;
	int res_code = 500;
	size_t response_len = 0;
	struct timeval tv;


	fid = fopen(file_name, "rb");
	if (NULL == fid) {
		APP_TRACE("%s: Failed to open file: %s!", __FUNCTION__, file_name);
		res_code = 404;
		goto FUNC_ERR_RES_RETURN;
	}

	GET_FILE_SIZE(file_name, file_size);
	if (file_size < 0) {
		APP_TRACE("%s: Failed to get size of file: %s!", __FUNCTION__, file_name);
		res_code = 500;
		goto FUNC_ERR_RES_RETURN;
	} if (0 == file_size) {
        APP_TRACE("%s: file %s shouldn't be empty!", __FUNCTION__, file_name);
        res_code = 500;
        goto FUNC_ERR_RES_RETURN;
    }

	http_header = HTTP_UTIL_new_response_header("HTTP", http_version, 200, NULL);
	if (NULL == http_header) {
		APP_TRACE("%s: HTTP_UTIL_new_response_header failed!", __FUNCTION__);
		goto FUNC_FAIL_RETURN;
	}

	http_header->add_tag_text(http_header, "Content-Type", "application/octet-stream", true);
	http_header->add_tag_int(http_header, "Content-Length", file_size, true);
	http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
	http_header->free(http_header);

	tv.tv_sec = 5;
	tv.tv_usec = 0;
	ret = setsockopt(session->sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	if (ret < 0) {
		APP_TRACE("%s: Failed to set send timeout! errno: %d", __FUNCTION__, errno);
	}

	response_len = strlen(response_buf);
	ret = send(session->sock, response_buf, strlen(response_buf), 0);
	if(ret == response_len){
		fseek(fid, 0, SEEK_SET);
		while((ret = fread(response_buf, 1, ARRAY_ITEM(response_buf), fid)) > 0){
			ret = send(session->sock, response_buf, ret, 0);
			if(ret < 0){
				APP_TRACE("%s: Failed to send file! ret: %d", __FUNCTION__, ret);
				goto FUNC_FAIL_RETURN;
			}
		}
	} else {
		APP_TRACE("%s: Failed to send header! ret: %zd, res len: %zu",
				  __FUNCTION__, ret, response_len);
	}

	return 0;

FUNC_RETURN:

	if (NULL != fid) {
		fclose(fid);
	}

	return 0;


FUNC_FAIL_RETURN:

	if (NULL != fid) {
		fclose(fid);
	}

	return -1;


FUNC_ERR_RES_RETURN:

	http_header = HTTP_UTIL_new_response_header("HTTP", http_version, res_code, NULL);
	if (NULL == http_header) {
		APP_TRACE("%s: HTTP_UTIL_new_response_header failed!", __FUNCTION__);
		goto FUNC_FAIL_RETURN;
	}
	http_header->add_tag_int(http_header, "Content-Length", 0, true);
	http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
	http_header->free(http_header);
	response_len = strlen(response_buf);
	ret = send(session->sock, response_buf, response_len, 0);
	if (ret != response_len) {
		APP_TRACE("%s: Failed to send response! ret: %zd, res len: %zu",
				  __FUNCTION__, ret, response_len);
		goto FUNC_FAIL_RETURN;
	}

	goto FUNC_RETURN;
}


int web_cgi_post_dana_uid(LP_HTTP_CONTEXT session)
{
	ssize_t ret = 0;
	size_t fwrite_ret;
	char *file_name = WEB_CGI_DANALE_CONF;
	char *file_name_tmp = WEB_CGI_DANALE_CONF".tmp";
	FILE* fid = NULL;

	int res_code = 500;
	LP_HTTP_HEAD_FIELD res_header = NULL;
	const char *http_version = NULL;
	const char *md5 = NULL;
	const char *cal_md5 = NULL;
    LP_HTTP_QUERY_PARA_LIST pHead = NULL;

	size_t response_buf_len = 1024;
	char response_buf[response_buf_len];
	size_t response_len = 0;



    // http version
    http_version = session->request_header->version;
    if (NULL == http_version) {
        APP_TRACE("%s: Failed to get http version of request! Use 1.1 to response", __FUNCTION__);
        http_version = "1.1";
    }


    pHead = HTTP_UTIL_parse_query_as_para(session->request_header->query);
    if (NULL == pHead) {
        APP_TRACE("%s: HTTP_UTIL_parse_query_as_para failed!", __FUNCTION__);
        goto FUNC_FAIL_RETURN;
    }

    md5 = pHead->read(pHead, "md5");
    if(NULL == md5){
        APP_TRACE("%s: can't read md5 parameter from url!", __FUNCTION__);
        res_code = 400;
        goto FUNC_ERR_RES_RETURN;
    }

    if (0 == session->request_content_len) {
        APP_TRACE("%s: Can't receive empty file!", __FUNCTION__);
        res_code = 400;
		goto FUNC_ERR_RES_RETURN;
    }

    if (NULL == session->request_content) {
        APP_TRACE("%s: request_content can't be NULL!", __FUNCTION__);
        res_code = 500;
        goto FUNC_ERR_RES_RETURN;
    }

	fid = fopen(file_name_tmp, "wb");
	if (NULL == fid) {
		APP_TRACE("%s: Failed to open file: %s!", __FUNCTION__, file_name_tmp);
		res_code = 500;
		goto FUNC_ERR_RES_RETURN;
	}

    fwrite_ret = fwrite(session->request_content, 1, session->request_content_len, fid);
    if (fwrite_ret != session->request_content_len) {
        APP_TRACE("%s: fwrite failed! want write: %zu, fwrite ret: %zu",
                  __FUNCTION__, session->request_content_len, fwrite_ret);
        res_code = 500;
        goto FUNC_ERR_RES_RETURN;
    }



	fclose(fid);
	fid = NULL;

	// md5
	cal_md5 = md5sum_file(file_name_tmp);
    if (NULL == cal_md5) {
        APP_TRACE("%s: md5sum_file failed for file %s!", __FUNCTION__, file_name_tmp);
        res_code = 500;
        goto FUNC_ERR_RES_RETURN;
    }

    if (0 != strcasecmp(md5, cal_md5)) {
		APP_TRACE("%s: md5 mismatched. md5 in request: %s, cal md5: %s",
				  __FUNCTION__,
				  md5, cal_md5);
        REMOVE_FILE(file_name_tmp);
		res_code = 400;
		goto FUNC_ERR_RES_RETURN;
	}

	if (0 != rename(file_name_tmp, file_name)) {
		APP_TRACE("%s: Failed rename %s to %s",
				  __FUNCTION__,
				  file_name_tmp, file_name);
		res_code = 500;
		goto FUNC_ERR_RES_RETURN;
	}


	res_header = HTTP_UTIL_new_response_header("HTTP", http_version, 200, NULL);
    if (NULL == res_header) {
		APP_TRACE("%s: HTTP_UTIL_new_response_header failed!", __FUNCTION__);
		goto FUNC_FAIL_RETURN;
	}
    res_header->add_tag_int(res_header, "Content-Length", 0, true);
	res_header->to_text(res_header, response_buf, ARRAY_ITEM(response_buf));
	res_header->free(res_header);
	response_len = strlen(response_buf);
	ret = send(session->sock, response_buf, strlen(response_buf), 0);
	if(ret != response_len){
		APP_TRACE("%s: Failed to send header! ret: %zd, res len: %zu",
				  __FUNCTION__, ret, response_len);
		goto FUNC_FAIL_RETURN;
	}


FUNC_RETURN:

	if (NULL != fid) {
		fclose(fid);
	}

    if (NULL != pHead) {
        pHead->free(pHead);
    }

	return 0;


FUNC_FAIL_RETURN:

	if (NULL != fid) {
		fclose(fid);
	}

    if (NULL != pHead) {
        pHead->free(pHead);
    }

	return -1;


FUNC_ERR_RES_RETURN:

	res_header = HTTP_UTIL_new_response_header("HTTP", http_version, res_code, NULL);
	if (NULL == res_header) {
		APP_TRACE("%s: HTTP_UTIL_new_response_header failed!", __FUNCTION__);
		goto FUNC_FAIL_RETURN;
	}
	res_header->add_tag_int(res_header, "Content-Length", 0, true);
	res_header->to_text(res_header, response_buf, ARRAY_ITEM(response_buf));
	res_header->free(res_header);
	response_len = strlen(response_buf);
	ret = send(session->sock, response_buf, response_len, 0);
	if (ret != response_len) {
		APP_TRACE("%s: Failed to send response! ret: %zd, res len: %zu",
				  __FUNCTION__, ret, response_len);
		goto FUNC_FAIL_RETURN;
	}

	goto FUNC_RETURN;
}

int WEB_CGI_dana_uid(LP_HTTP_CONTEXT session)
{
    ssize_t ret;
    LP_HTTP_HEAD_FIELD req_header = NULL;
    HTTP_STR_t method = NULL;
    LP_HTTP_HEAD_FIELD res_header = NULL;
    HTTP_STR_t http_version = NULL;
    size_t response_buf_len = 1024;
    char response_buf[response_buf_len];
    size_t response_len = 0;


    req_header = session->request_header;
    if(NULL == req_header){
        APP_TRACE("%s: Failed to get url request header!", __FUNCTION__);
        return -1;
    }

    // http version
    http_version = session->request_header->version;
    if (NULL == http_version) {
        APP_TRACE("%s: Failed to get http version of request! Use 1.1 to response", __FUNCTION__);
        http_version = "1.1";
    }

    method = req_header->method;
    if (NULL == method) {
        APP_TRACE("%s: Failed to get request method!", __FUNCTION__);
        return -1;
    }

    if (0 == strcasecmp(method, "GET")) {
        return web_cgi_get_dana_uid(session);
    } else if (0 == strcasecmp(method, "POST")) {
        return web_cgi_post_dana_uid(session);
    } else {
        res_header = HTTP_UTIL_new_response_header("HTTP", http_version, 400, NULL);
        if (NULL == res_header) {
            APP_TRACE("%s: HTTP_UTIL_new_response_header failed!", __FUNCTION__);
            return -1;
        }
        res_header->add_tag_int(res_header, "Content-Length", 0, true);
        res_header->to_text(res_header, response_buf, ARRAY_ITEM(response_buf));
        res_header->free(res_header);
        response_len = strlen(response_buf);
        ret = send(session->sock, response_buf, response_len, 0);
        if (ret != response_len) {
            APP_TRACE("%s: Failed to send response! ret: %zd, res len: %zu",
                      __FUNCTION__, ret, response_len);
            return -1;
        }
        return 0;
    }
}
#endif

#define BUFFER_SIZE			1024
static void WEB_CGI_isp_cfg_send_file(LP_HTTP_CONTEXT session, FILE *pf, int file_len)
{
	char send_buf[BUFFER_SIZE] = {0};
	LP_HTTP_HEAD_FIELD http_header = HTTP_UTIL_new_response_header("HTTP", session->request_header->version, 200, NULL);

	http_header->add_tag_text(http_header, "Content-Type", "text/html", true);
	http_header->add_tag_int(http_header, "Content-Length", file_len, true);

	bzero(send_buf, sizeof(send_buf));
	http_header->to_text(http_header, send_buf, sizeof(send_buf));

	http_header->free(http_header);
	http_header = NULL;

	send(session->sock, send_buf, strlen(send_buf), 0);

	do
	{
		bzero(send_buf, sizeof(send_buf));
		fread(send_buf, BUFFER_SIZE < file_len ? sizeof(send_buf) : file_len, 1, pf);
		send(session->sock, send_buf, BUFFER_SIZE < file_len ? sizeof(send_buf) : file_len, 0);
		file_len -= BUFFER_SIZE;
	} while(0 < file_len);
}

static void WEB_CGI_isp_cfg_send_status(LP_HTTP_CONTEXT session, int http_status)
{
	char send_buf[BUFFER_SIZE] = {0};
	LP_HTTP_HEAD_FIELD http_header = HTTP_UTIL_new_response_header("HTTP", session->request_header->version, http_status, NULL);

	http_header->add_tag_text(http_header, "Content-Type", "text/html", true);
	http_header->add_tag_int(http_header, "Content-Length", 0, true);

	bzero(send_buf, sizeof(send_buf));
	http_header->to_text(http_header, send_buf, sizeof(send_buf));

	http_header->free(http_header);
	http_header = NULL;

	send(session->sock, send_buf, strlen(send_buf), 0);
}

int WEB_CGI_isp_cfg(LP_HTTP_CONTEXT session)
{
	int content_len = 0;

	FILE *pf = NULL;

	if(HTTP_IS_GET(session))
	{
		do
		{
			if(NULL != (pf = fopen(ISP_CFG_TMP_INI, "rt")))
	 		{
				fseek(pf, 0L, SEEK_END);
				content_len = ftell(pf);
				fseek(pf, 0L, SEEK_SET);

				if(100 * BUFFER_SIZE >= content_len)
				{
					WEB_CGI_isp_cfg_send_file(session, pf, content_len);
					break;
				}
	 		}
			WEB_CGI_isp_cfg_send_status(session, 404);
		} while(0);
	}
	else if(HTTP_IS_POST(session) || HTTP_IS_PUT(session))
	{
		do
		{
			if(0 < session->request_content_len \
				&& 100 * BUFFER_SIZE >= session->request_content_len \
				&& NULL != (pf = fopen(ISP_CFG_TMP_INI, "wt+")) \
				&& 1 == fwrite(session->request_content, session->request_content_len, 1, pf))
			{
				// TODO: flush file
				fflush(pf);

				if(0 == SENSOR_cfg_load(ISP_CFG_TMP_INI))
				{
					WEB_CGI_isp_cfg_send_status(session, 200);
					break;
				}
			}
			WEB_CGI_isp_cfg_send_status(session, 400);
		} while(0);
	}
	else
	{
		WEB_CGI_isp_cfg_send_status(session, 405);
	}

	// close file
	if(NULL != pf)
	{
		fclose(pf);
		pf = NULL;
	}

	return 0;
}


extern int HI_SDK_ISP_get_sensor_defect_pixel_table(void);
extern int HI_SDK_ISP_set_sensor_defect_pixel_table(void);

int WEB_CGI_cal_defect_pixel(LP_HTTP_CONTEXT session)
{
	int content_len = 0;

	FILE *pf = NULL;

	if(HTTP_IS_GET(session))
	{
		
	}
	else if(HTTP_IS_POST(session) || HTTP_IS_PUT(session))
	{	REMOVE_FILE("/media/conf/defect_pixel_table");
		//HI_SDK_ISP_get_sensor_defect_pixel_table();
		//HI_SDK_ISP_set_sensor_defect_pixel_table();
		WEB_CGI_isp_cfg_send_status(session, 200);
	}
	else
	{
		WEB_CGI_isp_cfg_send_status(session, 405);
	}

	// close file
	if(NULL != pf)
	{
		fclose(pf);
		pf = NULL;
	}

	return 0;
}

