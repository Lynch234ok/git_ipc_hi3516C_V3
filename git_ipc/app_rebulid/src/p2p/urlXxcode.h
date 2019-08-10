#ifndef __URLXXCODE_H__
#define __URLXXCODE_H__

char *url_encode(char const *s, size_t len, size_t *new_length);
extern size_t url_decode(char *str, size_t len);

#endif // end of __URLXXCODE_H__