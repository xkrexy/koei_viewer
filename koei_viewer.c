#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include "koei_image.h"

const int ARG_WINDOW_WIDTH = 640;
const int ARG_WINDOW_HEIGHT = 400;
const int ARG_SCALE = 3;

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

void put_pixel(int x, int y, rgb_t rgb)
{
    SDL_Rect rect = {(x)*ARG_SCALE, (y)*ARG_SCALE, ARG_SCALE, ARG_SCALE};
    SDL_FillRect(screenSurface, &rect,
                 SDL_MapRGB(screenSurface->format, rgb.r, rgb.g, rgb.b));
}

void redraw(const char *filename, const char *palette, int width, int height, int align_length, int bpp, int left_to_right)
{
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL,
                 SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));

    int x = 0;
    int y = 0;
    int image_count = calc_image_count(filename, width, height, bpp);

    read_palette(palette);

    FILE *fp = fopen(filename, "r");

    for (int index = 0; index < image_count; index++)
    {
        image_t image;
        read_image(fp, &image, width, height, align_length, bpp, left_to_right);

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                put_pixel(x + j, y + i, index_to_rgb(get_index_image(&image, i, j)));
            }
        }

        x += width;
        if (x > ARG_WINDOW_WIDTH)
        {
            x = 0;
            y += height;
        }

        free_image(&image);
    }

    fclose(fp);

    SDL_UpdateWindowSurface(window);
}

int main(int argc, char *argv[])
{
    if (argc < 8)
    {
        printf("Usage: %s <image> <palette> <width> <height> <align length> <bpp> <left to right>\n", argv[0]);
        printf("   ex) %s KAODATA.DAT SAM3KAO.RGB 64 80 1 3 0\n", argv[0]);
        printf("\n");
        printf("[Align length / bpp / left to right examples]\n");
        printf("  - RPGMK\n");
        printf("    align_length: width * height / 8\n");
        printf("    bpp: 4\n");
        printf("    left_to_right: 1\n");
        printf("  - Sam3 Kao / Sam4 Kao\n");
        printf("    align_length: 1\n");
        printf("    bpp: 3\n");
        printf("    left_to_right: 0\n");
        printf("  - Horizon2(DH2) Kao\n");
        printf("    align_length: 1\n");
        printf("    bpp: 3\n");
        printf("    left_to_right: 1\n");
        printf("  - Hero HEXBCHP.000\n");
        printf("    align_length: 32\n");
        printf("    bpp: 4\n");
        printf("\n");

        return 0;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, ARG_WINDOW_WIDTH * ARG_SCALE,
                              ARG_WINDOW_HEIGHT * ARG_SCALE, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    redraw(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));

    while (SDL_WaitEvent(&event) >= 0)
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
                break;
            }
        }
        break;

        case SDL_QUIT:
        {
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 0;
        }
        break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
