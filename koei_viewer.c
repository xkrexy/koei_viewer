#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include "koei_image.h"

const int ARG_WINDOW_WIDTH = 640;
const int ARG_WINDOW_HEIGHT = 400;
const int ARG_SCALE = 1;

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

void put_pixel(int x, int y, rgb_t rgb)
{
    SDL_Rect rect = {(x)*ARG_SCALE, (y)*ARG_SCALE, ARG_SCALE, ARG_SCALE};
    SDL_FillRect(screenSurface, &rect,
                 SDL_MapRGB(screenSurface->format, rgb.r, rgb.g, rgb.b));
}

void redraw(const char *filename, int width, int height, int color_count)
{
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL,
                 SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));

    int x = 0;
    int y = 0;
    int image_count = 1; //calc_image_count(filename, width, height);

    FILE *fp = fopen(filename, "r");

    init_palette();

    for (int index = 0; index < image_count; index++)
    {
        image_t image;
        if (color_count == 8)
        {
            read_image_8color(fp, &image, width, height);
        }
        else
        {
            read_image_16color(fp, &image, width, height);
        }

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                put_pixel(x + j, y + i,
                          index_to_rgb(get_index_image(&image, i, j)));
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
    if (argc < 5)
    {
        printf("Usage: %s <image> <width> <height> <color>\n", argv[0]);
        return 0;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 0;
    }

    window =
        SDL_CreateWindow(argv[0], SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, ARG_WINDOW_WIDTH * ARG_SCALE,
                         ARG_WINDOW_HEIGHT * ARG_SCALE, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    redraw(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

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
