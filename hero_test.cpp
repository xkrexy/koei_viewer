#include <SDL2/SDL.h>

#include "koei_image.h"
#include "buf_reader.h"
#include <string.h>

#define BITS_PER_BYTE (8)

const int ARG_WINDOW_WIDTH = 1280;
const int ARG_WINDOW_HEIGHT = 800;

static int _SCALE = 1;

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

uint8_t *buf;
buf_reader_t *reader;
uint8_t *zhaoyun;

int step_enabled = 0;

void put_pixel(int x, int y, rgb_t rgb)
{
    SDL_Rect rect = {(x)*_SCALE, (y)*_SCALE, _SCALE, _SCALE};
    SDL_FillRect(screenSurface, &rect,
                 SDL_MapRGB(screenSurface->format, rgb.r, rgb.g, rgb.b));
}

void hline(int y, rgb_t rgb)
{
    SDL_Rect rect = {0, (y)*_SCALE, ARG_WINDOW_WIDTH, _SCALE};
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

rgb_t white = {0xff, 0xff, 0xff};
rgb_t black = {0, 0, 0};
rgb_t blue = {0, 0, 0xff};

// 01100001
// 11010111

void draw_zhaoyun()
{
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

            uint8_t index = 0;
            // palette index:
            for (int p = 0; p < 16; p++)
            {
                rgb_t cc = get_palette(p);
                if (cc.r == c.r && cc.g == c.g && cc.b == c.b)
                {
                    index = p;
                    break;
                }
            }

            index >>= 1;

            int gap = (j / 8);

            if (index & 0x01)
            {
                put_pixel(9 * 8 + 1 + j + gap, i, white);
            }
            else
            {
                put_pixel(9 * 8 + 1 + j + gap, i, black);
            }
        }
    }
}

void redraw()
{
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0x00, 0x00));

    buf_seek(reader, header);

    uint8_t swapped = 0x54;

    draw_zhaoyun();

    int _step = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            uint8_t line = read_uint8(reader);
            printf("%d: %02x (%d) / " BYTE_TO_BINARY_PATTERN "\n", _step, line, line, BYTE_TO_BINARY(line));

            int y = j;

            if (bit_from_bytes(&swapped, i))
            {
                y = 80 - j - 1;
            }

            // 10100100 이면 뒤에 1바이트를 읽어 5번 반복(임시)
            // if (line == 0xa4)
            // {
            //     uint8_t line1 = read_uint8(reader);
            //     uint8_t line2 = read_uint8(reader); // 임시
            //     int xxx = 5;
            //     for (int count = 0; count < xxx; count++)
            //     {
            //         for (int b = 0; b < BITS_PER_BYTE; b++)
            //         {
            //             if ((line1 >> (7 - b)) & 0x01)
            //             {
            //                 put_pixel(i * 9 + b, y, white);
            //             }
            //             else
            //             {
            //                 put_pixel(i * 9 + b, y, black);
            //             }
            //         }
            //         y++;
            //     }
            //     j += xxx - 1;
            //     continue;
            // }

            // 0x21이면 기존 2개를 읽어 일단 6번 반복
            if (line == 0x21)
            {
                buf_seek(reader, buf_get_seek_pos(reader) - 3);
                uint8_t line1 = read_uint8(reader);
                uint8_t line2 = read_uint8(reader);
                uint8_t line3 = read_uint8(reader); // 33
                uint8_t line4 = read_uint8(reader); // 33
                uint8_t line5 = read_uint8(reader); // 33
                uint8_t line6 = read_uint8(reader); // 33
                int xxx = 10;
                for (int count = 0; count < xxx; count++)
                {
                    for (int b = 0; b < BITS_PER_BYTE; b++)
                    {
                        if ((line1 >> (7 - b)) & 0x01)
                        {
                            put_pixel(i * 9 + b, y, white);
                        }
                        else
                        {
                            put_pixel(i * 9 + b, y, black);
                        }
                    }
                    y++;
                    for (int b = 0; b < BITS_PER_BYTE; b++)
                    {
                        if ((line2 >> (7 - b)) & 0x01)
                        {
                            put_pixel(i * 9 + b, y, white);
                        }
                        else
                        {
                            put_pixel(i * 9 + b, y, black);
                        }
                    }
                    y++;
                }
                j += xxx + 1;

                continue;
            }
            // 10100011 이면 뒤에 1바이트를 읽어 4번 반복(임시)
            if (line == 0xa3)
            {
                uint8_t line1 = read_uint8(reader);
                int xxx = 4;
                for (int count = 0; count < xxx; count++)
                {
                    for (int b = 0; b < BITS_PER_BYTE; b++)
                    {
                        if ((line1 >> (7 - b)) & 0x01)
                        {
                            put_pixel(i * 9 + b, y, white);
                        }
                        else
                        {
                            put_pixel(i * 9 + b, y, black);
                        }
                    }
                    y++;
                }
                j += xxx - 1;
                continue;
            }
            if (line == 0x62)
            {
                uint8_t line1 = read_uint8(reader);
                uint8_t line2 = read_uint8(reader);
                uint8_t line3 = read_uint8(reader); // 임시

                printf("  S - " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(line));
                printf("    - " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(line1));
                printf("    - " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(line2));
                printf("  E - " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(line3));

                //int xxx = (line & 0x0f) + 1;
                int xxx = 3;
                for (int count = 0; count < xxx; count++)
                {
                    for (int b = 0; b < BITS_PER_BYTE; b++)
                    {
                        if ((line1 >> (7 - b)) & 0x01)
                        {
                            put_pixel(i * 9 + b, y, white);
                        }
                        else
                        {
                            put_pixel(i * 9 + b, y, black);
                        }
                    }
                    y++;
                    for (int b = 0; b < BITS_PER_BYTE; b++)
                    {
                        if ((line2 >> (7 - b)) & 0x01)
                        {
                            put_pixel(i * 9 + b, y, white);
                        }
                        else
                        {
                            put_pixel(i * 9 + b, y, black);
                        }
                    }
                    y++;
                }
                j += xxx + 1;

                continue;
            }

            // 01100010 이어도 뒤에 2바이트 읽음

            // 01100001 이면 뒤에 2바이트를 읽어 2번 출력(임시)
            // 가 아니라 0110 이면 우측4비트 + 1번 반복
            //if ((line & 0x60) == 0x60)
            if (line == 0x61)
            {
                uint8_t line1 = read_uint8(reader);
                uint8_t line2 = read_uint8(reader);
                uint8_t line3 = read_uint8(reader); // 임시

                printf("  S - " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(line));
                printf("    - " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(line1));
                printf("    - " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(line2));
                printf("  E - " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(line3));

                //int xxx = (line & 0x0f) + 1;
                int xxx = 2;
                for (int count = 0; count < xxx; count++)
                {
                    for (int b = 0; b < BITS_PER_BYTE; b++)
                    {
                        if ((line1 >> (7 - b)) & 0x01)
                        {
                            put_pixel(i * 9 + b, y, white);
                        }
                        else
                        {
                            put_pixel(i * 9 + b, y, black);
                        }
                    }
                    y++;
                    for (int b = 0; b < BITS_PER_BYTE; b++)
                    {
                        if ((line2 >> (7 - b)) & 0x01)
                        {
                            put_pixel(i * 9 + b, y, white);
                        }
                        else
                        {
                            put_pixel(i * 9 + b, y, black);
                        }
                    }
                    y++;
                }
                j += xxx + 1;

                continue;
            }

            for (int b = 0; b < BITS_PER_BYTE; b++)
            {
                if ((line >> (7 - b)) & 0x01)
                {
                    put_pixel(i * 9 + b, y, white);
                }
                else
                {
                    put_pixel(i * 9 + b, y, black);
                }
            }

            _step++;
            if (_step >= step)
            {
                if (step_enabled)
                {
                    hline(y + 1, blue);
                    goto end;
                }
            }
        }
    }
end:
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

    if (argc >= 3 && argv[2][0] == '1')
    {
        step_enabled = 1;
    }

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
