#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "koei_image.h"

#define BITS_PER_BYTE (8)

static rgb_t g_palette[16];

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

int calc_image_count(const char *filename, int width, int height, int bpp)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        return (0);
    }

    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    fclose(fp);

    const int image_raw_size = (width * height * bpp) / BITS_PER_BYTE;

    const int image_count = (file_size / image_raw_size);
    if ((file_size % image_raw_size) != 0)
    {
        printf("Warning: File size is not divided by ImageSize! (%ld, %d)\n", file_size, image_raw_size);
    }

    return image_count;
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

int read_image(FILE *fp, image_t *image, int width, int height, int align_length, int bpp, int left_to_right)
{
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
    for (int i = 0; i < image_raw_size / (align_length * bpp); i++)
    {
        uint8_t *pixels = (uint8_t *)malloc(align_length * bpp);

        fread(pixels, 1, align_length * bpp, fp);
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
