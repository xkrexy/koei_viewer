#include <SDL2/SDL.h>

#include "koei_image.h"
#include "buf_reader.h"
#include <string.h>

#define BITS_PER_BYTE (8)

const int ARG_WINDOW_WIDTH = 1280;
const int ARG_WINDOW_HEIGHT = 800;

static int _SCALE = 8;

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

uint8_t build[10000] = {
    0,
};
int build_pos = 0;

void draw_1byte(int *row, int *col, uint8_t value)
{
    build[build_pos++] = value;
    for (int i = 0; i < BITS_PER_BYTE; i++)
    {
        rgb_t color = black;
        if ((value >> (7 - i)) & 0x01)
        {
            color = white;
        }
        put_pixel((*col) * 9 + i, (*row), color);
    }
    if ((*col) % 2 == 0)
    {
        *row = (*row) + 1;
        if ((*row) > 79)
        {
            (*col) = (*col) + 1;
            (*row) = 79;
        }
    }
    else
    {
        (*row) = (*row) - 1;
        if ((*row) < 0)
        {
            (*col) = (*col) + 1;
            (*row) = 0;
        }
    }
}

static int _SAVE_POS = 0;
void save_pos()
{
    _SAVE_POS = buf_get_seek_pos(reader);
}
void restore_pos()
{
    buf_seek(reader, _SAVE_POS);
}

void redraw()
{
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0x00, 0x00));

    buf_seek(reader, header);

    uint8_t swapped = 0x54;

    draw_zhaoyun();

    int _step = 0;
    int row = 0;
    int col = 0;
    build_pos = 0;

    while (col < 8)
    {
        uint8_t value = read_uint8(reader);
        save_pos();

        printf("[%03d] Byte: " BYTE_TO_BINARY_PATTERN "\n", _step, BYTE_TO_BINARY(value));

        if (value == 0b00100001) // 00100001 - 00000100 - 10000100 - 10100101
        {
            // 일단 위에 4바이트를 읽어서 그대로 복사
            uint8_t pattern[4] = {
                build[build_pos - 4],
                build[build_pos - 3],
                build[build_pos - 2],
                build[build_pos - 1]};

            // Read dummy
            read_uint8(reader);
            read_uint8(reader);
            read_uint8(reader);

            draw_1byte(&row, &col, pattern[0]);
            draw_1byte(&row, &col, pattern[1]);
            draw_1byte(&row, &col, pattern[2]);
            draw_1byte(&row, &col, pattern[3]);
        }
        else if (value == 0b01100010) // 01100010 : 일단 위에 2바이트를 4번 복사
        {
            uint8_t pattern[2] = {
                build[build_pos - 2],
                build[build_pos - 1]};

            // Read dummy
            read_uint8(reader); // 00101001

            // 반복 (일단 5번)
            for (int i = 0; i < 5; i++)
            {
                draw_1byte(&row, &col, pattern[0]);
                draw_1byte(&row, &col, pattern[1]);
            }
        }
        else if (value == 0b10100100) // 아래거 한줄을 복사: 10100100 - 11111111 - 01110110
        {
            uint8_t body[2];
            read_bytes(reader, body, 2);

            // 마지막 패턴이 01110110 가 아니면 일반 픽셀로 취급
            if (body[1] != 0b01110110)
            {
                draw_1byte(&row, &col, value);
                restore_pos();
                continue;
            }

            // 반복 (일단 6번)
            for (int i = 0; i < 6; i++)
            {
                draw_1byte(&row, &col, body[0]);
            }
        }
        else if (value == 0b01110000) // 다음거 하나를 1번 복사
        {
            uint8_t body[1];
            read_bytes(reader, body, 1);

            // 반복 (일단 1번)
            for (int i = 0; i < 1; i++)
            {
                draw_1byte(&row, &col, body[0]);
            }
        }
        else if (value == 0b10100000) // 다음거 하나를 2번 복사
        {
            uint8_t body[1];
            read_bytes(reader, body, 1);

            // 반복 (일단 2번)
            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, body[0]);
            }
        }
        else if (value == 0b10100001) // 다음거 하나를 3번 복사
        {
            uint8_t body[1];
            read_bytes(reader, body, 1);

            // 반복 (일단 3번)
            for (int i = 0; i < 3; i++)
            {
                draw_1byte(&row, &col, body[0]);
            }
        }
        // else if (value == 0b10100010) // 다음거 하나를 4번 복사
        // {
        //     uint8_t body[1];
        //     read_bytes(reader, body, 1);

        //     // 반복 (일단 4번)
        //     for (int i = 0; i < 4; i++)
        //     {
        //         draw_1byte(&row, &col, body[0]);
        //     }
        // }
        else if (value == 0b01100001) // 01100001 - 2 bytes - 01110100
        {
            uint8_t pattern[2];
            read_bytes(reader, pattern, 2);

            // Read dummy
            read_uint8(reader); // 01110100

            // 반복 (일단 2번)
            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern[0]);
                draw_1byte(&row, &col, pattern[1]);
            }
        }
        else
        {
            draw_1byte(&row, &col, value);
        }
        _step++;
        if (_step >= step)
        {
            if (step_enabled)
            {
                hline(row, blue);
                goto end;
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
                              SDL_WINDOWPOS_UNDEFINED, ARG_WINDOW_WIDTH,
                              ARG_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
