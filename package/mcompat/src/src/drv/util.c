#include "mcompat_config.h"


const unsigned short wCRCTalbeAbs[] =
{
    0x0000, 0xCC01, 0xD801, 0x1400,
    0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401,
    0x5000, 0x9C01, 0x8801, 0x4400,
};

unsigned short CRC16_2(unsigned char* pchMsg, unsigned short wDataLen)
{
    volatile unsigned short wCRC = 0xFFFF;
    unsigned short i;
    unsigned char chChar;

    for (i = 0; i < wDataLen; i++)
    {
        chChar = *pchMsg++;
        wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
        wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
    }

    return wCRC;
}


void print_data_hex(char *arg, unsigned char *buff, int len)
{
    int i = 0;

    printf("%s \n", arg);
    for(i = 0; i < len; i++)
    {
        printf("0x%02x ", buff[i]);
        if((i % 16) == 15)
        {
            printf("\n");
        }
    }

    if((len % 16) != 0)
    {
        printf("\n");
    }
}


