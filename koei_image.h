#ifndef __KOEI_IMAGE_H__
#define __KOEI_IMAGE_H__

#include <stdint.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} __attribute__((packed)) rgb_t;

typedef struct
{
    int width;
    int height;
    uint8_t *buf;
} __attribute__((packed)) image_t;

void init_palette();
rgb_t index_to_rgb(int index);
int read_image_8color(FILE *fp, image_t *image, int width, int height);
int read_image_16color(FILE *fp, image_t *image, int width, int height);
void free_image(image_t *image);
int get_index_image(image_t *image, int row, int col);

#endif
