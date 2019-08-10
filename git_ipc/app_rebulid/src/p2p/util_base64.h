#ifndef __BASE64___H__
#define __BASE64___H__


int util_BASE64_encode(const void *lpSoure, int cbSource, void *lpDest, int cbDest);
int util_BASE64_decode(const void *lpSoure, int cbSource, void *lpDest, int cbDest);


#endif
