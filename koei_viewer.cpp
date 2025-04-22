#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <map>
#include <string>
#include <stdbool.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "koei_image.h"
#include "ls11_decoder.h"

#define BITS_PER_BYTE (8)

const int ARG_WINDOW_WIDTH = 640;
const int ARG_WINDOW_HEIGHT = 400;

static int _SCALE = 1;

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

rapidjson::Document d;
uint32_t align_length_override;

static uint8_t *raw_buffer;
static size_t raw_buffer_size;

void put_pixel(int x, int y, rgb_t rgb)
{
    SDL_Rect rect = {(x)*_SCALE, (y)*_SCALE, _SCALE, _SCALE};
    SDL_FillRect(screenSurface, &rect,
                 SDL_MapRGB(screenSurface->format, rgb.r, rgb.g, rgb.b));
}

void redraw(rapidjson::Value &config)
{
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));

    bool is_ls11 = config["ls11"].GetBool();
    uint32_t default_width = config["default_width"].GetUint();
    uint32_t default_height = config["default_height"].GetUint();
    uint32_t bpp = config["bpp"].GetUint();
    uint32_t align_length = config["align_length"].GetUint();
    bool is_tiled = config["tiled"].GetBool();
    bool is_big_endian = config["big_endian"].GetBool();

    int x = 0;
    int y = 0;

    if (!is_ls11)
    {
        int image_data_size = (default_width * default_height * bpp / BITS_PER_BYTE);
        int image_count = raw_buffer_size / image_data_size;

        for (int index = 0; index < image_count; index++)
        {
            image_t image;
            read_image(raw_buffer + (image_data_size * index), &image,
                       default_width, default_height,
                       align_length_override,
                       bpp,
                       is_big_endian);

            if (is_tiled)
            {
                int src_index = 0;
                for (int i = 0; i < default_height / 16; i++)
                {
                    for (int j = 0; j < default_width / 16; j++)
                    {
                        for (int k = 0; k < 16; k++)
                        {
                            for (int l = 0; l < 16; l++)
                            {
                                put_pixel(x + j * 16 + l, y + i * 16 + k, index_to_rgb(image.buf[src_index++]));
                            }
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < default_height; i++)
                {
                    for (int j = 0; j < default_width; j++)
                    {
                        put_pixel(x + j, y + i, index_to_rgb(get_index_image(&image, i, j)));
                    }
                }
            }
            free_image(&image);

            x += default_width;
            if (x > ARG_WINDOW_WIDTH)
            {
                x = 0;
                y += default_height;
            }
        }
    }
    else
    {
        ls11_decode(raw_buffer, raw_buffer_size, [=, &x, &y](uint8_t *buf, uint32_t size) -> void
                    {
                        image_t image;
                        read_image(buf, &image,
                                   default_width, default_height,
                                   align_length_override,
                                   bpp,
                                   is_big_endian);

                        if (is_tiled)
                        {
                            int src_index = 0;
                            for (int i = 0; i < default_height / 16; i++)
                            {
                                for (int j = 0; j < default_width / 16; j++)
                                {
                                    for (int k = 0; k < 16; k++)
                                    {
                                        for (int l = 0; l < 16; l++)
                                        {
                                            put_pixel(x + j * 16 + l, y + i * 16 + k, index_to_rgb(image.buf[src_index++]));
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            for (int i = 0; i < default_height; i++)
                            {
                                for (int j = 0; j < default_width; j++)
                                {
                                    put_pixel(x + j, y + i, index_to_rgb(get_index_image(&image, i, j)));
                                }
                            }
                        }
                        free_image(&image);

                        x += default_width;
                        if (x > ARG_WINDOW_WIDTH)
                        {
                            x = 0;
                            y += default_height;
                        }
                    });
    }

    SDL_UpdateWindowSurface(window);
}

void read_json()
{
    long size = 0;
    FILE *fp = fopen("config.json", "r");
    if (fp)
    {
        fseek(fp, 0L, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        char *json = new char[size+16];        // allocate more mem
        memset(json, 0, size+16);
        fread(json, 1, size, fp);

        d.Parse(json);

        fclose(fp);
    }
}

void destroy_all()
{
    if (raw_buffer)
    {
        delete[] raw_buffer;
        raw_buffer = NULL;
        raw_buffer_size = 0;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <image> <palette>\n", argv[0]);
        printf("   ex) %s KAODATA.DAT SAM3KAO.RGB\n", argv[0]);

        return 0;
    }

    read_json();

    read_palette(argv[2]);

    FILE *fp = fopen(argv[1], "r");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        raw_buffer_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        raw_buffer = new uint8_t[raw_buffer_size];
        fread(raw_buffer, sizeof(uint8_t), raw_buffer_size, fp);

        fclose(fp);
    }

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

    if (!d.HasMember(base_filename.c_str()))
    {
        printf("Error: There is no preset information of %s!\n", argv[1]);
        return 0;
    }

    rapidjson::Value &config = d[base_filename.c_str()];
    align_length_override = config["align_length"].GetUint();

    do
    {
        redraw(config);

        switch (event.type)
        {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                destroy_all();

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
                align_length_override++;
                printf("Align length override: %d\n", align_length_override);
                break;
            case SDLK_COMMA:
                align_length_override--;
                printf("Align length override: %d\n", align_length_override);
                break;
            }
            break;

        case SDL_QUIT:
            destroy_all();

            SDL_DestroyWindow(window);
            SDL_Quit();
            return 0;
        }
    } while (SDL_WaitEvent(&event) >= 0);

    destroy_all();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
