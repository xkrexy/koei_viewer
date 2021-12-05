#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <map>
#include <string>
#include <stdbool.h>

#include "koei_image.h"

struct CONFIG
{
    bool is_ls11;
    uint32_t width;
    uint32_t height;
    uint32_t align_length;
    uint32_t bpp;
    bool is_tiled;
    bool is_big_endian;

    CONFIG(
        bool _is_ls11, uint32_t _width, uint32_t _height,
        uint32_t _align_length, uint32_t _bpp,
        bool _is_tiled, bool _is_big_endian)
        : is_ls11(_is_ls11), width(_width), height(_height), align_length(_align_length), bpp(_bpp), is_big_endian(_is_big_endian)
    {
    }

    CONFIG()
        : is_ls11(false), width(0), height(0), align_length(1), bpp(3), is_big_endian(true)
    {
    }
};

std::map<std::string, CONFIG> configs;

const int ARG_WINDOW_WIDTH = 640;
const int ARG_WINDOW_HEIGHT = 400;

static int _SCALE = 1;

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

void put_pixel(int x, int y, rgb_t rgb)
{
    SDL_Rect rect = {(x)*_SCALE, (y)*_SCALE, _SCALE, _SCALE};
    SDL_FillRect(screenSurface, &rect,
                 SDL_MapRGB(screenSurface->format, rgb.r, rgb.g, rgb.b));
}

void redraw(const char *filename, const char *palette, CONFIG *config)
{
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL,
                 SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));

    int x = 0;
    int y = 0;
    int image_count = calc_image_count(filename, config->width, config->height, config->bpp);

    read_palette(palette);

    FILE *fp = fopen(filename, "r");

    for (int index = 0; index < image_count; index++)
    {
        image_t image;
        read_image(fp, &image, config->width, config->height, config->align_length, config->bpp, config->is_big_endian);

#ifdef HERO_TILED
        int src_index = 0;
        for (int i = 0; i < height / 16; i++)
        {
            for (int j = 0; j < width / 16; j++)
            {
                for (int k = 0; k < 16; k++)
                {
                    for (int l = 0; l < 16; l++)
                    {
                        put_pixel(j * 16 + l, i * 16 + k, index_to_rgb(image.buf[src_index++]));
                    }
                }
            }
        }
#else
        for (int i = 0; i < config->height; i++)
        {
            for (int j = 0; j < config->width; j++)
            {
                put_pixel(x + j, y + i, index_to_rgb(get_index_image(&image, i, j)));
            }
        }
#endif
        x += config->width;
        if (x > ARG_WINDOW_WIDTH)
        {
            x = 0;
            y += config->height;
        }

        free_image(&image);
    }

    fclose(fp);

    SDL_UpdateWindowSurface(window);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <image> <palette>\n", argv[0]);
        printf("   ex) %s KAODATA.DAT SAM3KAO.RGB\n", argv[0]);

        return 0;
    }

    configs.insert(std::pair<std::string, CONFIG>("KAODATA.DAT", CONFIG(false, 64, 80, 1, 3, false, false)));
    configs.insert(std::pair<std::string, CONFIG>("KAODATAP.S4", CONFIG(false, 64, 80, 1, 3, false, false)));
    configs.insert(std::pair<std::string, CONFIG>("KAODATA2.S4", CONFIG(false, 64, 80, 1, 3, false, false)));
    configs.insert(std::pair<std::string, CONFIG>("HEXBCHP.R3", CONFIG(true, 16, 3584, 32, 4, false, true)));
    configs.insert(std::pair<std::string, CONFIG>("HEXZCHP.R3", CONFIG(true, 16, 1280, 32, 4, false, true)));
    configs.insert(std::pair<std::string, CONFIG>("HEXICHR.R3", CONFIG(true, 96, 96, 32, 4, true, true)));
    configs.insert(std::pair<std::string, CONFIG>("HEXBCHR.R3", CONFIG(true, 64, 64, 32, 4, true, true)));
    configs.insert(std::pair<std::string, CONFIG>("HEXZCHR.R3", CONFIG(true, 32, 64, 32, 4, true, true)));
    configs.insert(std::pair<std::string, CONFIG>("SMAPBGPL.R3", CONFIG(true, 16, 3392, 32, 4, false, true)));
    configs.insert(std::pair<std::string, CONFIG>("MMAPBGPL.R3", CONFIG(true, 16, 4080, 32, 4, false, true)));
    configs.insert(std::pair<std::string, CONFIG>("SSCCHR2.R3", CONFIG(true, 32, 160, 160, 4, false, true)));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, ARG_WINDOW_WIDTH * _SCALE,
                              ARG_WINDOW_HEIGHT * _SCALE, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    std::string path = argv[1];
    std::string base_filename = path.substr(path.find_last_of("/\\") + 1);

    if (configs.find(base_filename) == configs.end())
    {
        printf("Error: There is no preset information of %s!\n", argv[1]);
        return 0;
    }

    CONFIG config = configs[base_filename];

    printf("config.width: %d\n", config.width);
    printf("config.height: %d\n", config.height);
    printf("config.align_length: %d\n", config.align_length);
    printf("config.bpp: %d\n", config.bpp);
    printf("config.is_big_endian: %d\n", config.is_big_endian);

    do
    {
        redraw(argv[1], argv[2], &config);

        switch (event.type)
        {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            case SDLK_EQUALS:
                _SCALE++;
                break;
            case SDLK_MINUS:
                _SCALE--;
                break;
            case SDLK_PERIOD:
                config.align_length++;
                printf("Align length: %d\n", config.align_length);
                break;
            case SDLK_COMMA:
                config.align_length--;
                printf("Align length: %d\n", config.align_length);
                break;
            }
            break;

        case SDL_QUIT:
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 0;
        }
    } while (SDL_WaitEvent(&event) >= 0);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
