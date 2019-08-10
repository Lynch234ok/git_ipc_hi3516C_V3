#ifndef MM_UTF8_GBK_MEM_H
#define MM_UTF8_GBK_MEM_H
#ifdef _cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

//utf8转unicode
int utf82unicode(char *utf8char, int* unicode, int *len, int leaveLen);

//utf8转gbk
int utf82gbk_(char *strutf8, char *strgbk );

//gbk转utf8
int gbk2utf8_(char *strgbk, char *strutf8);

#ifdef _cplusplus
}
#endif
#endif