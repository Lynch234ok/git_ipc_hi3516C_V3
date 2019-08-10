
#ifndef FRANK_CRYPT_H_
#define FRANK_CRYPT_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int frank_encrypt(const char *in, int inLength, char *out, int outMax);
extern int frank_decrypt(const char *in, int inLength, char *out, int outMax);

#ifdef __cplusplus
}
#endif
#endif //FRANK_CRYPT_H_

