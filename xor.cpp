#include <stdio.h>
#include <stdint.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

int one_count(uint8_t value)
{
    int count = 0;
    for (int i = 0; i < 8; i++)
    {
        if (((value >> i) & 0x01) == 0x01)
        {
            count++;
        }
    }

    return count;
}

void output(uint8_t byte)
{
    uint8_t upper_half = (byte & 0xf0) >> 4;

    printf("0x%02X: " BYTE_TO_BINARY_PATTERN " : 0x%02X (" BYTE_TO_BINARY_PATTERN ")\n",
           byte,
           BYTE_TO_BINARY(byte),
           upper_half,
           BYTE_TO_BINARY(upper_half));
}

int main(int argc, char *argv[])
{
    {
        uint8_t data[] = {0x61, 0xA9, 0x55, 0x74};
        for (int i = 0; i < sizeof(data); i++)
        {
            output(data[i]);
        }
    }

    printf("========\n");

    {
        uint8_t data[] = {0x61, 0xD7, 0xCB, 0x7F};
        for (int i = 0; i < sizeof(data); i++)
        {
            output(data[i]);
        }
    }

    printf("========\n");

    {
        uint8_t data[] = {0x61, 0x5F, 0x9F, 0x76};
        for (int i = 0; i < sizeof(data); i++)
        {
            output(data[i]);
        }
    }

    printf("========\n");

    {
        uint8_t data[] = {0x62, 0x54, 0xAA, 0x29, 0x0C, 0x73};
        for (int i = 0; i < sizeof(data); i++)
        {
            output(data[i]);
        }
    }

    printf("========\n");

    {
        uint8_t data[] = {0xA1, 0xEE, 0xA4, 0xDF, 0xA2, 0xEF, 0xA1, 0xF7, 0x7E};
        for (int i = 0; i < sizeof(data); i++)
        {
            output(data[i]);
        }
    }

    return 0;
}
// 61 A9 55 74