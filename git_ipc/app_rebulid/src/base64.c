
#include "base64.h"

int base64_encode(const void *in_str,void *out_str, ssize_t length) 
{
	// BASE64
#define BASE64_PAD '='
	const char const __base64_table[] =
	{
		'A','B','C','D','E','F','G','H','I','J','K','L','M', 
		'N','O','P','Q','R','S','T','U','V','W','X','Y','Z', 
		'a','b','c','d','e','f','g','h','i','j','k','l','m', 
		'n','o','p','q','r','s','t','u','v','w','x','y','z',
		'0','1','2','3','4','5','6','7','8','9','+','/','\0', 
	}; 

	const unsigned char *current = in_str;

	int i = 0; 
	unsigned char *result = out_str;

	while (length > 2){
		// keep going until we have less than 24 bits
		result[i++] = __base64_table[current[0] >> 2];
		result[i++] = __base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		result[i++] = __base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
		result[i++] = __base64_table[current[2] & 0x3f];

		current += 3;
		length -= 3; // we just handle 3 octets of data
	}

	// now deal with the tail end of things
	if (length != 0){
		result[i++] = __base64_table[current[0] >> 2];

		if (length > 1){
			result[i++] = __base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
			result[i++] = __base64_table[(current[1] & 0x0f) << 2]; 
			result[i++] = BASE64_PAD;
		}else{
			result[i++] = __base64_table[(current[0] & 0x03) << 4];
			result[i++] = BASE64_PAD;
			result[i++] = BASE64_PAD;
		}
	}

	result[i] = '\0';
	return i; 
}


static char _base64_decode(char c)
{
    if(c <= 'Z' && c >= 'A')
        return (c - 'A');
    else if(c <= 'z' && c >= 'a')
        return (c - 'a' + 26);
    else if(c <= '9' && c >= '0')
        return (c - '0' + 52);
    else
    {
        switch(c)
        {
        case '+':
            return 62;
            break;
        case '/':
            return 63;
            break;
        case '=':
            return 0;
            break;
        }
    }
	return 0;
}

int base64_decode(const void* in_str, void* out_str, ssize_t length)
{
    if(length%4)
        return -1;
    char chunk[4];
    int parsenum=0;
	uint8_t *current = (uint8_t*)in_str;
	uint8_t *result = out_str;

    while(length>0){
        chunk[0] = _base64_decode(current[0]);
        chunk[1] = _base64_decode(current[1]);
        chunk[2] = _base64_decode(current[2]);
        chunk[3] = _base64_decode(current[3]);

        *result++ = (chunk[0] << 2) | (chunk[1] >> 4);
        *result++ = (chunk[1] << 4) | (chunk[2] >> 2);
        *result++ = (chunk[2] << 6) | (chunk[3]);

        current+=4;
        length-=4;
        parsenum+=3;
    }

	*result = '\0';
    return parsenum;
}
