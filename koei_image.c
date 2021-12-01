#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "koei_image.h"

static rgb_t g_palette[16];

void init_palette()
{
    g_palette[0].b = 0x00;
    g_palette[0].g = 0x1f;
    g_palette[0].r = 0x2f;
    g_palette[1].b = 0x7f;
    g_palette[1].g = 0x3f;
    g_palette[1].r = 0x1f;
    g_palette[2].b = 0x1f;
    g_palette[2].g = 0x3f;
    g_palette[2].r = 0xaf;
    g_palette[3].b = 0x4f;
    g_palette[3].g = 0x7f;
    g_palette[3].r = 0xbf;
    g_palette[4].b = 0x1f;
    g_palette[4].g = 0x6f;
    g_palette[4].r = 0x3f;
    g_palette[5].b = 0x8f;
    g_palette[5].g = 0x7f;
    g_palette[5].r = 0x3f;
    g_palette[6].b = 0x7f;
    g_palette[6].g = 0xaf;
    g_palette[6].r = 0xff;
    g_palette[7].b = 0xaf;
    g_palette[7].g = 0xcf;
    g_palette[7].r = 0xcf;

// SAM3
#if 0
    g_palette[0].b = 0x00;
    g_palette[0].g = 0x00;
    g_palette[0].r = 0x00;
    g_palette[1].b = 0xef;
    g_palette[1].g = 0x3f;
    g_palette[1].r = 0x00;
    g_palette[2].b = 0x00;
    g_palette[2].g = 0x4f;
    g_palette[2].r = 0xef;
    g_palette[3].b = 0xcf;
    g_palette[3].g = 0x4f;
    g_palette[3].r = 0xef;
    g_palette[4].b = 0x4f;
    g_palette[4].g = 0xaf;
    g_palette[4].r = 0x0f;
    g_palette[5].b = 0xef;
    g_palette[5].g = 0xbf;
    g_palette[5].r = 0x00;
    g_palette[6].b = 0x00;
    g_palette[6].g = 0xdf;
    g_palette[6].r = 0xef;
    g_palette[7].b = 0xef;
    g_palette[7].g = 0xef;
    g_palette[7].r = 0xef;
#endif

    g_palette[8].b = 0xdd;
    g_palette[8].g = 0xf0;
    g_palette[8].r = 0x10;
    g_palette[9].b = 0x0d;
    g_palette[9].g = 0xf4;
    g_palette[9].r = 0x15;
    g_palette[10].b = 0xed;
    g_palette[10].g = 0xf1;
    g_palette[10].r = 0x11;
    g_palette[11].b = 0xa0;
    g_palette[11].g = 0xb4;
    g_palette[11].r = 0x7d;
    g_palette[12].b = 0x5e;
    g_palette[12].g = 0x78;
    g_palette[12].r = 0x55;
    g_palette[13].b = 0x77;
    g_palette[13].g = 0x55;
    g_palette[13].r = 0x22;
    g_palette[14].b = 0x14;
    g_palette[14].g = 0x36;
    g_palette[14].r = 0x47;
    g_palette[15].b = 0x97;
    g_palette[15].g = 0x54;
    g_palette[15].r = 0x20;
}

rgb_t index_to_rgb(int index)
{
    if (index >= 0 && index < 16)
    {
        return g_palette[index];
    }

    return (rgb_t){0, 0, 0};
}

uint8_t bit_from_byte(uint8_t byte, int position)
{
    return (byte & (0x80 >> position)) >> (7 - position);
}

int read_image_8color(FILE *fp, image_t *image, int width, int height)
{
    uint8_t pixels[8] = {
        0,
    };

    if (!image)
    {
        return (-1);
    }

    image->width = width;
    image->height = height;
    image->buf = (uint8_t *)malloc(width * height);

    memset(image->buf, 0, width * height);

    int x = 0;
    for (int i = 0; i < (width / 8) * height; i++)
    {
        uint8_t b1 = 0;
        uint8_t b2 = 0;
        uint8_t b3 = 0;

        fread(&b1, 1, 1, fp);
        fread(&b2, 1, 1, fp);
        fread(&b3, 1, 1, fp);

        for (int j = 0; j < 8; j++)
        {
            pixels[j] = (bit_from_byte(b1, j) << 0) |
                        (bit_from_byte(b2, j) << 1) |
                        (bit_from_byte(b3, j) << 2);

            image->buf[x++] = pixels[j];
        }
    }

    return (0);
}

int read_image_16color(FILE *fp, image_t *image, int width, int height)
{
    uint8_t pixels[8] = {
        0,
    };

    if (!image)
    {
        return (-1);
    }

    image->width = width;
    image->height = height;
    image->buf = (uint8_t *)malloc(width * height);

    memset(image->buf, 0, width * height);

    fseek(fp, 0x990, SEEK_SET);

    int x = 0;
    for (int i = 0; i < (width / 8) * height; i++)
    {
        uint8_t b1 = 0;
        uint8_t b2 = 0;
        uint8_t b3 = 0;
        uint8_t b4 = 0;

        fread(&b1, 1, 1, fp);
        fread(&b2, 1, 1, fp);
        fread(&b3, 1, 1, fp);
        fread(&b4, 1, 1, fp);

        for (int j = 0; j < 8; j++)
        {
            pixels[j] = (bit_from_byte(b1, j) << 0) |
                        (bit_from_byte(b2, (j + 1) % 8) << 1) |
                        (bit_from_byte(b3, (j + 2) % 8) << 2) |
                        (bit_from_byte(b4, (j + 3) % 8) << 3);

            image->buf[x++] = pixels[j];
        }
    }

    return (0);
}

void free_image(image_t *image)
{
    if (image && image->buf)
    {
        free(image->buf);
        image->buf = NULL;
    }
}

int get_index_image(image_t *image, int row, int col)
{
    if (!image)
    {
        return (0);
    }

    return image->buf[image->width * row + col];
}
