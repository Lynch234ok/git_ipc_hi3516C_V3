#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int BASE64_encode(const void *lpSoure, int cbSource, void *lpDest, int cbDest)   
{   
    int i, j;   
    const unsigned char *lpSourceByte = (const unsigned char *)lpSoure;   
    unsigned char *lpDestByte = (unsigned char *)lpDest;   
    unsigned char byte[3];   
    static const char key[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";   
       
    if (cbDest < (cbSource + 2)/3 * 4){
		printf("ERRER:No enough room to fill the encoded data\n");
        return -1;   
    }   
       
    memset(lpDest, 0, cbDest);   
       
    for (i = 0, j = 0; i < cbSource; i += 3, j += 4){   
        byte[0] = lpSourceByte[i + 0];   
        lpDestByte[j + 0] = key[(byte[0] & 0xfc) >> 2];   
        if (i + 1 < cbSource) {   
            byte[1] = lpSourceByte[i + 1];   
            lpDestByte[j + 1] = key[((byte[0] & 0x03) << 4) | ((byte[1] & 0xf0) >> 4)];   
        } else {   
            byte[1] = 0;   
            lpDestByte[j + 1] = key[((byte[0] & 0x03) << 4) | ((byte[1] & 0xf0) >> 4)];   
            lpDestByte[j + 2] = '=';   
            lpDestByte[j + 3] = '=';   
            break;   
        }   
           
        if (i + 2 < cbSource) {   
            byte[2] = lpSourceByte[i + 2];   
            lpDestByte[j + 2] = key[((byte[1] & 0x0f) << 2) | ((byte[2] & 0xc0) >> 6)];   
            lpDestByte[j + 3] = key[(byte[2] & 0x3f)];   
        } else {   
            byte[2] = 0;   
            lpDestByte[j + 2] = key[((byte[1] & 0x0f) << 2) | ((byte[2] & 0xc0) >> 6)];   
            lpDestByte[j + 3] = '=';   
        }   
    }   
    return (cbSource + 2)/3 * 4;   
}   
   
   
static unsigned char DecodeByte(char ch)   
{   
	unsigned int i;
    static const char key[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";   
    for (i = 0; i < strlen(key); i ++){   
        if (key[i] == ch) return i;   
        if (ch == '=') return '\x80';   
    }   
    return '\x40';   
}   
   
int BASE64_decode(const void *lpSoure, int cbSource, void *lpDest, int cbDest)   
{   
    int i, j;   
    const unsigned char *lpSourceByte = (const unsigned char *)lpSoure;   
    unsigned char *lpDestByte = (unsigned char *)lpDest;   
    unsigned char byte[4];   
   
    if (cbDest < cbSource * 3 / 4){   
        // No enough room to fill the encoded data   
        return -1;   
    }   
   
    memset(lpDest, 0, cbDest);   
   
    for (i = 0, j = 0; i < cbSource; ){   
        // ???¨²?a4??¡Á??¨²??¨º? 00ABCDEF 00GHIJKL 00MNOPQR 00STUVWX   
        if (lpSourceByte[i + 0] <= ' '){   
            i ++;   
            continue;   
        }   
        byte[0] = DecodeByte(lpSourceByte[i + 0]);   
        if (byte[0] & 0xC0) break;   
        byte[1] = DecodeByte(lpSourceByte[i + 1]);   
        if (byte[1] & 0xC0) break;   
        lpDestByte[j++] = (unsigned char)(byte[0] << 2) | (unsigned char)((byte[1] & 0x30) >> 4); // ABCDEF-GH   
   
        byte[2] = DecodeByte(lpSourceByte[i + 2]);   
        if (byte[2] & 0xC0) break;   
        lpDestByte[j++] = (unsigned char)(byte[1] << 4) | (unsigned char)((byte[2] & 0x3C) >> 2); // IJKL-MNOP   
   
        byte[3] = DecodeByte(lpSourceByte[i + 3]);   
        if (byte[3] & 0xC0) break;   
        lpDestByte[j++] = (unsigned char)(byte[2] << 6) | (unsigned char)byte[3]; // QRSTUVWX   
        i += 4;   
    }   
   
    return j;   
}   
   
int QuotedPrintableDecode(char * lpSource)   
{   
    char * p = lpSource;   
    char * q = lpSource;   
   
    while (*p){   
        char ch = *p;   
        if (ch == '='){   
            int nValue = 0;   
            sscanf(p, "=%02X", &nValue);   
            *q = (char)nValue;   
            p += 3;   
        } else {   
            *q = ch;   
            p ++;   
        }   
        q ++;   
    }   
    *q = '\0';   
    return q - lpSource;   
}   
