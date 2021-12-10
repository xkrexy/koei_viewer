#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    {
        int a = 0x61;
        int b = 0xA9;
        int c = 0x55;
        int d = 0x74;
        printf("%02X\n", a + b + c);
        printf("%02X\n", a ^ b ^ c);
    }

    {
        int a = 0xA1;
        int b = 0xEE;
        int c = 0xA4;
        int d = 0xDF;
        int e = 0xA2;
        int f = 0xEF;
        int g = 0xA1;
        int h = 0xF7;
        int i = 0x7E;
        printf("%02X\n", a + b + c + d + e + f + g + h);
        printf("%02X\n", a ^ b ^ c ^ d ^ e ^ f ^ g ^ h);
    }

    return 0;
}
// 61 A9 55 74