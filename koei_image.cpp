#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "koei_image.h"

#define BITS_PER_BYTE (8)

static rgb_t g_palette[16];

rgb_t get_palette(uint8_t index)
{
    return g_palette[index];
}

int read_palette(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        return (-1);
    }

    int index = 0;
    while (!feof(fp))
    {
        fread(&g_palette[index++], 1, 3, fp);
    }

    fclose(fp);

    return (0);
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

int read_image(uint8_t *buf, image_t *image, int width, int height, int align_length, int bpp, int left_to_right)
{
    if (!buf || !image)
    {
        return (-1);
    }

    image->width = width;
    image->height = height;
    image->buf = (uint8_t *)malloc(width * height);
    memset(image->buf, 0, width * height);

    buf_reader_t *reader = create_buffer_reader(buf, width * height * bpp / BITS_PER_BYTE, true);

    int position = 0;
    const int image_raw_size = (width * height * bpp) / BITS_PER_BYTE;
    for (int i = 0; i < image_raw_size / (align_length * bpp); i++)
    {
        uint8_t *pixels = (uint8_t *)malloc(align_length * bpp);

        read_bytes(reader, pixels, align_length * bpp);
        for (int j = 0; j < align_length * BITS_PER_BYTE; j++)
        {
            image->buf[position] = 0;
            for (int k = 0; k < bpp; k++)
            {
                if (left_to_right)
                {
                    image->buf[position] |= (bit_from_bytes(pixels + (align_length * k), j) << (bpp - k - 1));
                }
                else
                {
                    image->buf[position] |= (bit_from_bytes(pixels + (align_length * k), j) << (k));
                }
            }
            position++;
        }

        free(pixels);
    }

    destroy_buffer_reader(reader);

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
