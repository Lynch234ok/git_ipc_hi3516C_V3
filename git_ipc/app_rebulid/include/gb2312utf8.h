/*
 * encode_convert_function.h
 *
 *  Created on: 2011-11-16
 *      Author: root
 */

#include <stdbool.h>
#include <stdint.h>

#ifndef ENCODE_CONVERT_FUNCTION_H_
#define ENCODE_CONVERT_FUNCTION_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int gb2312_to_utf8(bool zhT, char *textGB2312, char *result, int resultMax);
extern int utf8_to_gb2312(char* _utf8, char *result, int resultMax);
extern int gb2312_to_utf8_nprintf(char *utf8_buf, int max_sz, const char *fmt, ...);

//int gb2312_to_utf8(char* _gb2312, char* _utf8);
//int utf8_to_gb2312(char* _utf8, char* _gb2312);

#ifdef __cplusplus
}
#endif
#endif /* ENCODE_CONVERT_FUNCTION_H_ */
