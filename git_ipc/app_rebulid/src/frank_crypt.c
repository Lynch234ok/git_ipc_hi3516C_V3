
#include "frank_crypt.h"
#include <ctype.h>
#include "base64.h"

#define kFRANK_CRYPT_MASK ("+_)(*&^%$#@!")

int frank_encrypt(const char *in, int inLength, char *out, int outMax)
{
	if(NULL != in && NULL != out){
		char *mask = strdupa(kFRANK_CRYPT_MASK);
		int const maskLength = strlen(mask);
		int const maskOffset = (rand() % maskLength) % 26;
		int const outMaskLength = inLength + 1;
		char *outMask = alloca(outMaskLength);
		int i = 0;
		if(outMax < 2 * inLength){
			return 0;
		}
//		APP_TRACE("enc0:\"%s\"", in);
		for(i = 0; i < inLength && i < outMax; ++i){
			int const ch = (int)(in[i] ^ mask[(i + maskOffset)  % maskLength]);
			if(isprint(ch)){
				outMask[i] = ch;
			}else{
				outMask[i] = in[i];
			}
		}
		outMask[i] = '\0';
//		APP_TRACE("enc1:\"%s\"", outMask);
		*out++ = (char)(maskOffset + 'a');
		base64_encode(outMask, out, strlen(outMask));
//		APP_TRACE("enc2:\"%s\"", out);
		return strlen(out);
	}
	return 0;
}

int frank_decrypt(const char *in, int inLength, char *out, int outMax)
{
	int i = 0;
	if(NULL != in && NULL != out){
		char *mask = strdupa(kFRANK_CRYPT_MASK);
		int const maskLength = strlen(mask);
		int const maskOffset = (int)(--inLength, *in++ - 'a');
		int inMaskLength = 2 * inLength;
		char *inMask = alloca(inMaskLength);

//		APP_TRACE("dec0=\"%s\" %d", in, strlen(in));
		i = base64_decode(in, inMask, strlen(in));
		inMask[i] = 0;
		inMaskLength = strlen(inMask);
//		APP_TRACE("dec1=\"%s\" %d", inMask, strlen(inMask));
		for(i = 0; i < inMaskLength && i < outMax; ++i){
			int const ch = (int)(inMask[i] ^ mask[(i + maskOffset) % maskLength]);
			if(isprint(ch)){
				out[i] = ch;
			}else{
				out[i] = inMask[i];
			}
		}
		out[i] = '\0';
//		APP_TRACE("dec2=\"%s\" %d", out, strlen(out));
		return i;
	}
	return 0;
}

