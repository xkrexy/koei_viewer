#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "koei_image.h"

#define BITS_PER_BYTE (8)

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

    // Horizon KAO (8-Colors)
    g_palette[0].r = 0x00;
    g_palette[0].g = 0x00;
    g_palette[0].b = 0x00;
    g_palette[1].r = 0x00;
    g_palette[1].g = 0xA0;
    g_palette[1].b = 0x60;
    g_palette[2].r = 0xD0;
    g_palette[2].g = 0x40;
    g_palette[2].b = 0x00;
    g_palette[3].r = 0xF0;
    g_palette[3].g = 0xA0;
    g_palette[3].b = 0x60;
    g_palette[4].r = 0x00;
    g_palette[4].g = 0x40;
    g_palette[4].b = 0xD0;
    g_palette[5].r = 0x00;
    g_palette[5].g = 0xA0;
    g_palette[5].b = 0xF0;
    g_palette[6].r = 0xD0;
    g_palette[6].g = 0x60;
    g_palette[6].b = 0xA0;
    g_palette[7].r = 0xF0;
    g_palette[7].g = 0xE0;
    g_palette[7].b = 0xD0;
}

rgb_t index_to_rgb(int index)
{
    if (index >= 0 && index < 16)
    {
        return g_palette[index];
    }

    return (rgb_t){0, 0, 0};
}

uint8_t bit_from_bytes(uint8_t *bytes, int position)
{
    int byte_index = position / BITS_PER_BYTE;
    int bit_position = position % BITS_PER_BYTE;
    return (bytes[byte_index] & (0x80 >> bit_position)) >> (7 - bit_position);
}

static int read_image(FILE *fp, image_t *image, int width, int height, int align_length, int bpp) {
    if (!image)
    {
        return (-1);
    }

    image->width = width;
    image->height = height;
    image->buf = (uint8_t *)malloc(width * height);
    memset(image->buf, 0, width * height);

    int position = 0;
    const int image_raw_size = (width * height * bpp) / BITS_PER_BYTE;
    for (int i = 0; i < image_raw_size / (align_length * bpp); i++) {
        uint8_t *pixels = (uint8_t *)malloc(align_length * bpp);

        fread(pixels, 1, align_length * bpp, fp);
        for (int j = 0; j < align_length * BITS_PER_BYTE; j++) {
            image->buf[position] = 0;
            for (int k = 0; k < bpp; k++) {
#ifdef __LEFT_TO_RIGHT
                image->buf[position] |= (bit_from_bytes(pixels + (align_length * k), j) << (k));
#else
                image->buf[position] |= (bit_from_bytes(pixels + (align_length * k), j) << (bpp - k - 1));
#endif
            }
            position++;
        }

        free(pixels);
    }

    return (0);
}

int read_image_8color(FILE *fp, image_t *image, int width, int height)
{
    read_image(fp, image, width, height, 1, 3);
}

int read_image_16color(FILE *fp, image_t *image, int width, int height)
{
    read_image(fp, image, width, height, 32, 4);
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
