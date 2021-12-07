#include <SDL2/SDL.h>

#include "koei_image.h"
#include "buf_reader.h"
#include <string.h>

#define BITS_PER_BYTE (8)

const int ARG_WINDOW_WIDTH = 640;
const int ARG_WINDOW_HEIGHT = 400;

static int _SCALE = 1;

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

uint8_t *buf;
buf_reader_t *reader;
uint8_t *zhaoyun;

void put_pixel(int x, int y, rgb_t rgb)
{
    SDL_Rect rect = {(x)*_SCALE, (y)*_SCALE, _SCALE, _SCALE};
    SDL_FillRect(screenSurface, &rect,
                 SDL_MapRGB(screenSurface->format, rgb.r, rgb.g, rgb.b));
}

int header = 27;
int step = 0;

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

void redraw()
{
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));

    rgb_t white;
    white.r = 0xff;
    white.g = 0xff;
    white.b = 0xff;

    buf_seek(reader, header);

    int _step = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            uint8_t line = read_uint8(reader);
            printf("%d: %02x (%d) / " BYTE_TO_BINARY_PATTERN "\n", _step, line, line, BYTE_TO_BINARY(line));

            for (int b = 0; b < BITS_PER_BYTE; b++)
            {
                if ((line >> (7 - b)) & 0x01)
                {
                    if (i % 2 == 0)
                    {
                        put_pixel(i * 8 + b, j, white);
                    }
                    else
                    {
                        put_pixel(i * 8 + b, 79 - j, white);
                    }
                }
            }

            _step++;
            if (_step >= step)
            {
                goto end;
            }
        }
    }
end:

    // Zhao Yun
    for (int i = 0; i < 80; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            int base_index = (64 * 3) * i + (j * 3);
            uint8_t b = zhaoyun[base_index + 0];
            uint8_t g = zhaoyun[base_index + 1];
            uint8_t r = zhaoyun[base_index + 2];
            rgb_t c;
            c.r = r;
            c.g = g;
            c.b = b;
            put_pixel(64 + j, i, c);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void destroy_all()
{
    destroy_buffer_reader(reader);
    delete[] buf;

    delete[] zhaoyun;
}

int main(int argc, char *argv[])
{
    FILE *fp = fopen(argv[1], "r");
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buf = new uint8_t[filesize];
    size_t ret = fread(buf, 1, filesize, fp);
    fclose(fp);

    reader = create_buffer_reader(buf, filesize, false);

    read_palette("HERO.PAL");

    FILE *fpz = fopen("zhaoyun.tga", "r");
    if (fpz)
    {
        fseek(fpz, 18, SEEK_SET);
        zhaoyun = new uint8_t[64 * 80 * 3];
        fread(zhaoyun, 1, 64 * 80 * 3, fpz);
        fclose(fpz);
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

    do
    {
        redraw();

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
                step++;
                printf("Step: %d\n", step);
                break;
            case SDLK_COMMA:
                step--;
                printf("Step: %d\n", step);
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
