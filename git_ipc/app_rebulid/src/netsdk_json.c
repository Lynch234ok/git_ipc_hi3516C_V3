
#include "netsdk_json.h"
#include "app_debug.h"

static char* json_remove_opt(char* dst, const char* src, const char* str)
{
	char* i = strstr(src, str);
	char* src_tmp = src;
	if(strlen(str) < strlen(src) && i != NULL){		
		while(*src){
			if(src == i){
				src+=strlen(str)+2;
				//*dst = 0;
				dst-= 2;
			}
			*dst++ = *src++;
		}
		if(*(dst-2) == ','){
			*(dst-2) = ' ';
		}
		*dst = 0;
		return dst;
	}
	return NULL;
}

LP_JSON_OBJECT NETSDK_json_parse(const char *str)
{
	LP_JSON_OBJECT jso = json_tokener_parse(str);
	if((int)jso < 0){
		return NULL;
	}
	return jso;
}


LP_JSON_OBJECT NETSDK_json_dup(LP_JSON_OBJECT jso)
{
	LP_JSON_OBJECT orignJSON = json_object_get(jso);
	const char *text = json_object_to_json_string(orignJSON);
	LP_JSON_OBJECT newJSON = json_tokener_parse(text);
	// put the reference
	if((int)newJSON < 0){
		//APP_TRACE("\r\n%s", text);
		//APP_TRACE("JSON New = %d", (int)newJSON);
		newJSON = NULL;
	}
	json_object_put(orignJSON);
	orignJSON = NULL;
	return newJSON;
}

bool NETSDK_json_check_child(LP_JSON_OBJECT jso, const char *key)
{
	return NULL != json_object_object_get(jso, key);
}

LP_JSON_OBJECT NETSDK_json_get_child(LP_JSON_OBJECT jso, const char *keys)
{
	if(NULL != jso && NULL != keys){
        char key_dup[256] = {0};
		const char *key = NULL;
		char *token = NULL;
		LP_JSON_OBJECT obj = jso;
        int len = 0;

        len = snprintf(key_dup, sizeof(key_dup), "%s", keys);
        if(len != strlen(keys))
        {
            printf("%s(%d) snprintf failed!!! sprintf return len=%d != keys(%s) len=%d\n", __FUNCTION__, __LINE__, len, keys, strlen(keys));
            return NULL;
        }

		key = strtok_r(key_dup, "./", &token);
		obj = json_object_object_get(obj, key);
		
		while(NULL != (key = strtok_r(NULL, "./", &token))){
			obj = json_object_object_get(obj, key);
		}
		return obj;
	}
	return NULL;
}

int NETSDK_json_copy_child(LP_JSON_OBJECT src_jso, LP_JSON_OBJECT dst_jso, const char *key)
{
	if(src_jso && dst_jso){
		LP_JSON_OBJECT src_child = NETSDK_json_get_child(src_jso, key);
		LP_JSON_OBJECT dst_child = NETSDK_json_get_child(dst_jso, key);
		
		if(src_child && dst_child && src_child != dst_child){
			//APP_TRACE(json_object_to_json_string(src_jso));
			//APP_TRACE(json_object_to_json_string(dst_jso));
			if(json_object_get_type(src_child) == json_object_get_type(dst_child)){
				switch(json_object_get_type(src_child)){
					case json_type_boolean:
						NETSDK_json_set_boolean2(dst_jso, key, json_object_get_boolean(src_child));
						break;
					case json_type_double:
						NETSDK_json_set_dfloat2(dst_jso, key, json_object_get_double(src_child));
						break;
					case json_type_int:
						NETSDK_json_set_int2(dst_jso, key, json_object_get_int(src_child));
						break;
					case json_type_string:
						NETSDK_json_set_string2(dst_jso, key, json_object_get_string(src_child));
						break;
					case json_type_array:
					case json_type_object:
					default:
						return -1;
				}
				return 0;
			}
		}
	}
	//APP_TRACE("what the hell @ %s", key);
	return -1;
}



const char *NETSDK_json_get_string(LP_JSON_OBJECT jso, const char *key, char *result, int result_max)
{
	LP_JSON_OBJECT keyJSON = json_object_object_get(jso, key);
	if(NULL != keyJSON){
		const char *str = json_object_get_string(keyJSON);
		if(NULL != str){
			snprintf(result, result_max, "%s", str);
			return result;
		}
	}
	return NULL;
}

int NETSDK_json_get_int(LP_JSON_OBJECT jso, const char *key)
{
	return json_object_get_int(json_object_object_get(jso, key));
}

float NETSDK_json_get_float(LP_JSON_OBJECT jso, const char *key)
{
	return (float)(json_object_get_double(json_object_object_get(jso, key)));
}

double NETSDK_json_get_dfloat(LP_JSON_OBJECT jso, const char *key)
{
	return json_object_get_double(json_object_object_get(jso, key));
}

bool NETSDK_json_get_boolean(LP_JSON_OBJECT jso, const char *key)
{
	return json_object_get_boolean(json_object_object_get(jso, key)) ? true : false;
}

void NETSDK_json_set_string(LP_JSON_OBJECT jso, const char *key, const char *val)
{
	if(key){
		LP_JSON_OBJECT jso_val = json_object_new_string(val ? val: "");
		json_object_object_add(jso, key, jso_val);
	}
}

void NETSDK_json_set_int(LP_JSON_OBJECT jso, const char *key, int val)
{
	if(key){
		LP_JSON_OBJECT jso_val = json_object_new_int(val);
		json_object_object_add(jso, key, jso_val);
	}
}

void NETSDK_json_set_float(LP_JSON_OBJECT jso, const char *key, float val)
{
	if(key){
		LP_JSON_OBJECT jso_val = json_object_new_double((double)val);
		json_object_object_add(jso, key, jso_val);
	}
}

void NETSDK_json_set_dfloat(LP_JSON_OBJECT jso, const char *key, double val)
{
	if(key){
		LP_JSON_OBJECT jso_val = json_object_new_double(val);
		json_object_object_add(jso, key, jso_val);
	}
}

void NETSDK_json_set_boolean(LP_JSON_OBJECT jso, const char *key, bool val)
{
	if(key){
		LP_JSON_OBJECT jso_val = json_object_new_boolean(val);
		json_object_object_add(jso, key, jso_val);
	}
}

void NETSDK_json_set_array(LP_JSON_OBJECT dst_jso, LP_JSON_OBJECT src_jso, char *val)
{
	if(NULL == src_jso){
		if(val){
			json_object_array_add(dst_jso, val);
		}
	}else{
		//clear array
		LP_JSON_OBJECT jsonOption = NETSDK_json_get_child(dst_jso, "opt");
		LP_JSON_OBJECT jsonOptionList = NULL, dstOptionJson = NULL;
		int n_jsonOptionList, i;
		if(NULL != jsonOption){
			n_jsonOptionList = json_object_array_length(jsonOption);
			
			for(i = 0; i < n_jsonOptionList; ++i){
				jsonOptionList = NETSDK_json_get_child(dst_jso, "opt");
				jsonOption = json_object_array_get_idx(jsonOptionList, 0);
				const char *const keyOpt = json_object_get_string(jsonOption);
				char jsonstr[512] = {0};
				if(NULL != json_remove_opt(jsonstr, json_object_to_json_string(jsonOptionList), keyOpt)){
					json_object_object_del(dst_jso, "opt");
					LP_JSON_OBJECT newJsonOpt =  NETSDK_json_parse(jsonstr);
					if(newJsonOpt){
						json_object_object_add(dst_jso, "opt", newJsonOpt);
					}					
				}
			}
			char *jsonstr;
			jsonstr = strdup(json_object_to_json_string(src_jso));
			LP_JSON_OBJECT newJsonOpt =  NETSDK_json_parse(jsonstr);

			n_jsonOptionList = json_object_array_length(newJsonOpt);
			for(i = 0; i < n_jsonOptionList; i++){
				jsonOption = json_object_array_get_idx(newJsonOpt, i);
				dstOptionJson = NETSDK_json_get_child(dst_jso, "opt");
				json_object_array_add(dstOptionJson, jsonOption);
			}
		}
	}
}

void NETSDK_json_set_array2(LP_JSON_OBJECT dst_jso, LP_JSON_OBJECT src_jso, char *key)
{
	if(dst_jso && src_jso && key){
		LP_JSON_OBJECT srcOption;
		srcOption = NETSDK_json_get_child(src_jso, key);
		if(srcOption){
			json_object_object_del(dst_jso, key);
			char *jsonstr;
			jsonstr = strdup(json_object_to_json_string(srcOption));
			LP_JSON_OBJECT newJsonOpt =  NETSDK_json_parse(jsonstr);
			if(newJsonOpt){
				json_object_object_add(dst_jso, key, newJsonOpt);
			}
		}
	}
}

static bool netsdk_json_check_property_mode(LP_JSON_OBJECT jso, const char *key)
{
	char keyProperty[64] = {""};
	LP_JSON_OBJECT jsonKeyProperty = NULL;
	snprintf(keyProperty, sizeof(keyProperty), "%sProperty", key);

	jsonKeyProperty = NETSDK_json_get_child(jso, keyProperty);
	if(jsonKeyProperty){
		NETSDK_json_get_string(jsonKeyProperty, "mode", keyProperty, sizeof(keyProperty));
		if(0 == strncmp(keyProperty, "ro", 2)){
			return false;
		}
	}
	return true;
}

static bool netsdk_json_check_property_option(LP_JSON_OBJECT jso, const char *key, const void *val_ref)
{
	int i = 0;
	char keyProperty[64] = {""};
	LP_JSON_OBJECT jsonKeyProperty = NULL;
	snprintf(keyProperty, sizeof(keyProperty), "%sProperty", key);

	jsonKeyProperty = NETSDK_json_get_child(jso, keyProperty);
	if(jsonKeyProperty){
		LP_JSON_OBJECT jsonOptionList = NETSDK_json_get_child(jsonKeyProperty, "opt");
		if(NULL != jsonOptionList){
			int const n_jsonOptionList = json_object_array_length(jsonOptionList);
			for(i = 0; i < n_jsonOptionList; ++i){
				LP_JSON_OBJECT jsonOption = json_object_array_get_idx(jsonOptionList, i);
				switch(json_object_get_type(jsonOption)){
					case json_type_boolean:
						{
							bool const keyVal = *(bool *)(val_ref);
							bool const keyOpt = json_object_get_boolean(jsonOption) ? true : false;
							if(keyOpt == keyVal){
								//APP_TRACE("Valid value @ %s:%s", key, keyVal ? "true" : "fasle");
								return true;
							}else{
								//APP_TRACE("Invalid value @ %s:%s", key, keyVal ? "true" : "fasle");
							}
							break;
						}
					case json_type_double:
						{
							double const keyVal = *(double *)(val_ref);
							double const keyOpt = json_object_get_double(jsonOption);
							if(keyOpt == keyVal){
								//APP_TRACE("Valid value @ %s:%f", key, (float)keyVal);
								return true;
							}else{
								//APP_TRACE("Invalid value @ %s:%f", key, (float)keyVal);
							}
							break;
						}
					case json_type_int:
						{
							int const keyVal = *(int *)(val_ref);
							int const keyOpt = json_object_get_int(jsonOption);
							if(keyOpt == keyVal){
								//APP_TRACE("Valid value @ %s:%d", key, keyVal);
								return true;
							}else{
								//APP_TRACE("Invalid value @ %s:%d", key, keyVal);
							}
							break;
						}
					case json_type_string:
						{
							const char *const keyVal = *(char **)(val_ref);
							const char *const keyOpt = json_object_get_string(jsonOption);
							if(strlen(keyOpt) == strlen(keyVal) && 0 == strcmp(keyOpt, keyVal)){
								//APP_TRACE("Valid value @ %s:%s", key, keyVal);
								return true;
							}else{
								//APP_TRACE("Invalid value @ %s:%s/%s", key, keyVal, keyOpt);
							}
							break;
						}
					default:
						return false;
				}
			}
			// value not in valid key option
			return false;
		}
	}
	return true;
}


static bool netsdk_json_check_property_range(LP_JSON_OBJECT jso, const char *key, const void *val_ref)
{
	int i = 0;
	char keyProperty[64] = {""};
	LP_JSON_OBJECT jsonKeyProperty = NULL;
	LP_JSON_OBJECT jsonKey = NETSDK_json_get_child(jso, key);
	snprintf(keyProperty, sizeof(keyProperty), "%sProperty", key);

	jsonKeyProperty = NETSDK_json_get_child(jso, keyProperty);
	if(NULL != jsonKeyProperty && NULL != jsonKey){
		LP_JSON_OBJECT jsonRangeMin = NETSDK_json_get_child(jsonKeyProperty, "min");
		LP_JSON_OBJECT jsonRangeMax = NETSDK_json_get_child(jsonKeyProperty, "max");
		double range = 0.0;

		if(NULL != jsonRangeMin || NULL != jsonRangeMax){

			switch(json_object_get_type(jsonKey)){
				case json_type_double:
					range = *(double *)val_ref;
					break;
				case json_type_int:
					range = (double)(*(int *)val_ref);
					break;
				case json_type_string:
					range = (double)(strlen(*(char **)val_ref));
					break;
				default:
					// range property only for doube, integer, and string, to the others all pass
					return true;
			}

			if(NULL != jsonRangeMin){
				double rangeMin = json_object_get_double(jsonRangeMin);
				if(range < rangeMin){
					return false; // out of range
				}
			}
			if(NULL != jsonRangeMax){
				double rangeMax = json_object_get_double(jsonRangeMax);
				if(range > rangeMax){
					return false; // out of range
				}
			}
		}
	}
	return true;
}


int NETSDK_json_set_string2(LP_JSON_OBJECT jso, const char *key, const char *val)
{
	if(key){
		if(!netsdk_json_check_property_mode(jso, key)){
			return -1;
		}
		if(!netsdk_json_check_property_option(jso, key, &val)){
			return -1;
		}
		if(!netsdk_json_check_property_range(jso, key, &val)){
			return -1;
		}
		NETSDK_json_set_string(jso, key, val);
		return 0;
	}
	return -1;
}

void NETSDK_json_set_int2(LP_JSON_OBJECT jso, const char *key, int val)
{
	if(key){
		if(!netsdk_json_check_property_mode(jso, key)){
			return;
		}
		if(!netsdk_json_check_property_option(jso, key, &val)){
			return;
		}
		if(!netsdk_json_check_property_range(jso, key, &val)){
			return;
		}
		NETSDK_json_set_int(jso, key, val);
	}
}

void NETSDK_json_set_float2(LP_JSON_OBJECT jso, const char *key, float val)
{
	if(key){
		double dfloat = (double)val;
		if(!netsdk_json_check_property_mode(jso, key)){
			return;
		}
		if(!netsdk_json_check_property_option(jso, key, &dfloat)){
			return;
		}
		if(!netsdk_json_check_property_range(jso, key, &dfloat)){
			return;
		}
		NETSDK_json_set_float(jso, key, dfloat);
	}
}

void NETSDK_json_set_dfloat2(LP_JSON_OBJECT jso, const char *key, double val)
{
	if(key){
		if(!netsdk_json_check_property_mode(jso, key)){
			return;
		}
		if(!netsdk_json_check_property_option(jso, key, &val)){
			return;
		}
		if(!netsdk_json_check_property_range(jso, key, &val)){
			return;
		}
		NETSDK_json_set_dfloat(jso, key, val);
	}
}

void NETSDK_json_set_boolean2(LP_JSON_OBJECT jso, const char *key, bool val)
{
	if(key){
		if(!netsdk_json_check_property_mode(jso, key)){
			return;
		}
		if(!netsdk_json_check_property_option(jso, key, &val)){
			return;
		}
		NETSDK_json_set_boolean(jso, key, val);
	}
}

int NETSDK_json_remove_property_option(LP_JSON_OBJECT jso, const char *key,  const void *val_ref)
{	
	int i = 0;
	char keyProperty[64] = {""};
	LP_JSON_OBJECT jsonKeyProperty = NULL;
	snprintf(keyProperty, sizeof(keyProperty), "%sProperty", key);

	jsonKeyProperty = NETSDK_json_get_child(jso, keyProperty);
	if(jsonKeyProperty){
		LP_JSON_OBJECT jsonOptionList = NETSDK_json_get_child(jsonKeyProperty, "opt");
		if(NULL != jsonOptionList){
			int const n_jsonOptionList = json_object_array_length(jsonOptionList);

			for(i = 0; i < n_jsonOptionList; ++i){
				LP_JSON_OBJECT jsonOption = json_object_array_get_idx(jsonOptionList, i);
				switch(json_object_get_type(jsonOption)){
					case json_type_boolean:
						{
							bool const keyVal = *(bool *)(val_ref);
							bool const keyOpt = json_object_get_boolean(jsonOption) ? true : false;
							if(keyOpt == keyVal){
								//APP_TRACE("Valid value @ %s:%s", key, keyVal ? "true" : "fasle");
							}else{
								//APP_TRACE("Invalid value @ %s:%s", key, keyVal ? "true" : "fasle");
							}
							break;
						}
					case json_type_double:
						{
							double const keyVal = *(double *)(val_ref);
							double const keyOpt = json_object_get_double(jsonOption);
							if(keyOpt == keyVal){
								//APP_TRACE("Valid value @ %s:%f", key, (float)keyVal);
							}else{
								//APP_TRACE("Invalid value @ %s:%f", key, (float)keyVal);
							}
							break;
						}
					case json_type_int:
						{
							int const keyVal = *(int *)(val_ref);
							int const keyOpt = json_object_get_int(jsonOption);
							if(keyOpt == keyVal){
								//APP_TRACE("Valid value @ %s:%d", key, keyVal);
							}else{
								//APP_TRACE("Invalid value @ %s:%d", key, keyVal);
							}
							break;
						}
					case json_type_string:
						{
							const char *const keyOpt = json_object_get_string(jsonOption);
							if(strlen(keyOpt) == strlen(val_ref) && 0 == strcmp(keyOpt, val_ref)){
								char jsonstr[512] = {0};
								if(NULL != json_remove_opt(jsonstr, json_object_to_json_string(jsonOptionList), val_ref)){
									json_object_object_del(jsonKeyProperty, "opt");
									LP_JSON_OBJECT newJsonOpt =  NETSDK_json_parse(jsonstr);
									if(newJsonOpt){
										json_object_object_add(jsonKeyProperty, "opt", newJsonOpt);
										return 0;
									}
									
								}							
							}else{
								//APP_TRACE("Invalid value @ %s:%s/%s", key, val_ref, keyOpt);
							}
							break;
						}
					default:
						break;
				}
			}
			// value not in valid key option
		}
	}
	return -1;
}


void NETSDK_json_remove_properties(LP_JSON_OBJECT channel)
{
	struct lh_entry *entry = json_object_get_object(channel)->head;
	while(entry){
		const char *const key = (const char *const)entry->k;
		struct lh_entry *entry_next = entry->next;

		if(memmem(key, strlen(key) + 1, "Property\0", strlen("Property\0"))){
			//APP_TRACE("key: %s", key);
			json_object_object_del(channel, key);
		}
		entry = entry_next;
	 }
}

LP_JSON_OBJECT NETSDK_json_load(const char *filename)
{
	LP_JSON_OBJECT json = json_object_from_file(filename);
	if((int)json < 0){
		return NULL;
	}
	return json;
}

int NETSDK_json_save(LP_JSON_OBJECT jso, const char *conf_path)
{
	json_object_to_file(conf_path, jso);
	return 0;
}


