#include <stdio.h>
#include <stdint.h>

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

char *shortString(uint16_t word)
{
    static char _str[17];
    for (int i = 0; i < 16; i++)
    {
        _str[15 - i] = '0' + ((word >> i) & 0x01);
    }

    _str[16] = 0;
    return _str;
}

char *bitString(uint8_t byte)
{
    static char _str[9];
    for (int i = 0; i < 8; i++)
    {
        _str[7 - i] = '0' + ((byte >> i) & 0x01);
    }

    _str[8] = 0;
    return _str;
}

char *nibbleString(uint8_t byte)
{
    static char _str[5];
    for (int i = 0; i < 4; i++)
    {
        _str[3 - i] = '0' + ((byte >> i) & 0x01);
    }

    _str[4] = 0;
    return _str;
}

void Testing(uint32_t offset, uint8_t *data, size_t size)
{
    int sum = 0;
    int upper_half_sum = 0;
    int lower_half_sum = 0;

    printf("[0x%04X %s]\n", offset, shortString(offset));
    for (size_t i = 0; i < size; i++)
    {
        uint8_t upper_half = (data[i] & 0xf0) >> 4;
        uint8_t lower_half = (data[i] & 0x0f) >> 0;

        printf("0x%02X: %s\n",
               data[i],
               bitString(data[i]));

        if (i >= 1 && i < size - 1)
        {
            sum ^= upper_half;
            sum ^= lower_half;
            upper_half_sum += upper_half;
            lower_half_sum += lower_half;
        }
    }

    printf("[Target Value: 0x%01X %s]\n", data[size - 1] & 0x0f, nibbleString(data[size - 1] & 0x0f));

    printf("Sum: 0x%04X (%d) %s\n", sum, sum, bitString(sum & 0xff));
    printf("Upper Half Sum: 0x%04X (%d) %s\n", upper_half_sum, upper_half_sum, bitString(upper_half_sum & 0xff));
    printf("Lower Half Sum: 0x%04X (%d) %s\n", lower_half_sum, lower_half_sum, bitString(lower_half_sum & 0xff));

    printf("\n");
}

int main(int argc, char *argv[])
{
    {
        uint8_t data[] = {0x61, 0xA9, 0x55, 0x74};
        Testing(0x29, data, sizeof(data));
    }

    {
        uint8_t data[] = {0x61, 0xD7, 0xCB, 0x7F};
        Testing(0x162, data, sizeof(data));
    }

    {
        uint8_t data[] = {0x61, 0xDF, 0xD5, 0x7F};
        Testing(0x27a, data, sizeof(data));
    }

    {
        uint8_t data[] = {0x61, 0x5F, 0x9F, 0x76};
        Testing(0x40, data, sizeof(data));
    }

    {
        uint8_t data[] = {0xA4, 0xFF, 0x76};
        Testing(0x4B, data, sizeof(data));
    }

    {
        uint8_t data[] = {0xA1, 0xFB, 0xA0, 0xF7, 0x70};
        Testing(0x55, data, sizeof(data));
    }

    return 0;
}
// 61 A9 55 74