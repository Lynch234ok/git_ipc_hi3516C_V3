#ifndef __PACKBITS_H__
#define __PACKBITS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int PACKBITS_encode(unsigned char *src, unsigned char *dst, unsigned int n);
extern unsigned int PACKBITS_decode(unsigned char *outp, unsigned char *inp,
			unsigned int outlen, unsigned int inlen);

#ifdef __cplusplus
}
#endif
#endif

