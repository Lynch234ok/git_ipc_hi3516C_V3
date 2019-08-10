#ifndef __SHA1_H__
#define __SHA1_H__

#define SHA1_MAC_LEN 20     

typedef struct {   
        unsigned long state[5];   
        unsigned long count[2];   
        unsigned char buffer[64];   
} SHA1_CTX;   
   
void SHA1Reset(SHA1_CTX *context);   
void SHA1Input(SHA1_CTX *context, unsigned char *data, unsigned long len);   
void SHA1Result(SHA1_CTX *context, unsigned char *digest);//20   
void SHA1Transform(unsigned long *state, unsigned char *buffer); //5  64   
   

#endif

