#include <stdio.h>

static void crc_byteCRC(unsigned char *crc, char ch)
{
    unsigned int genPoly = 0x107;
    int i = 0;
    *crc ^= ch;
    for(i = 0; i < 8; i++)
    if(*crc & 0x80)
        *crc = (*crc << 1) ^ genPoly;
    else
        *crc <<= 1;
    *crc &= 0xff;

}

unsigned char CRC_getByteCRC(const char *block, int blockLen)
{
    unsigned char crc = 0;
    int i = 0;
    for (i = 0; i < blockLen; i++)
		crc_byteCRC(&crc, block[i]);
    return crc;

}

