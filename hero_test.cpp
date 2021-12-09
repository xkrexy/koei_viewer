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
int step = 390;

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

int g_inst_count[256] = {
    0,
};
void inc_inst(int num)
{
    g_inst_count[num]++;
}

int inst_count(int num)
{
    return g_inst_count[num];
}

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

            gap = 0;

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
        put_pixel((*col) * 8 + i, (*row), color);
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

uint8_t _read_uint8()
{
    int32_t pos = buf_get_seek_pos(reader);
    uint8_t value = read_uint8(reader);
    printf("[%03d] Byte: " BYTE_TO_BINARY_PATTERN " (%02X)\n", pos, BYTE_TO_BINARY(value), value);
    return value;
}

uint8_t _read_uint8_relative(int32_t pos)
{
    uint8_t value = reader->buf[reader->seek_pos + pos - 1];
    return value;
}

void redraw()
{
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xff, 0x00, 0x00));

    buf_seek(reader, header);
    memset(g_inst_count, 0, 256 * 4);

    uint8_t swapped = 0x54;

    draw_zhaoyun();

    int _step = 0;
    int row = 0;
    int col = 0;
    build_pos = 0;

    while (col < 8)
    {
        uint8_t value = _read_uint8();

        // A1 XX A4 XX A2 XX A1 XX 7E
        if (value == 0b10100001 &&                   // A1
            _read_uint8_relative(2) == 0b10100100 && // A4
            _read_uint8_relative(4) == 0b10100010 && // A2
            _read_uint8_relative(6) == 0b10100001 && // A1
            _read_uint8_relative(8) == 0b01111110    // 7E
        )
        {
            uint8_t pattern0 = _read_uint8_relative(1);
            uint8_t pattern1 = _read_uint8_relative(3);
            uint8_t pattern2 = _read_uint8_relative(5);
            uint8_t pattern3 = _read_uint8_relative(7);
            buf_rseek(reader, 8);

            for (int i = 0; i < 3; i++)
            {
                draw_1byte(&row, &col, pattern0);
            }
            for (int i = 0; i < 6; i++)
            {
                draw_1byte(&row, &col, pattern1);
            }
            for (int i = 0; i < 4; i++)
            {
                draw_1byte(&row, &col, pattern2);
            }
            for (int i = 0; i < 3; i++)
            {
                draw_1byte(&row, &col, pattern3);
            }
            // 00110110 무시됨
        }
        else if (value == 0b01100010 &&                   // 62
                 _read_uint8_relative(3) == 0b01100001 && // 61
                 _read_uint8_relative(6) == 0b01110100    // 74
        )
        {
            uint8_t pattern0 = _read_uint8_relative(1);
            uint8_t pattern1 = _read_uint8_relative(2);
            uint8_t pattern2 = _read_uint8_relative(4);
            uint8_t pattern3 = _read_uint8_relative(5);
            buf_rseek(reader, 6);

            for (int i = 0; i < 3; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }

            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern2);
                draw_1byte(&row, &col, pattern3);
            }
        }
        else if (value == 0b10100011 &&                   // A3
                 _read_uint8_relative(2) == 0b01100001 && // 61
                 _read_uint8_relative(5) == 0b01111111    // 7F
        )
        {
            uint8_t pattern0 = _read_uint8_relative(1);
            uint8_t pattern1 = _read_uint8_relative(3);
            uint8_t pattern2 = _read_uint8_relative(4);
            buf_rseek(reader, 5);

            // 패턴0을 5회 반복
            for (int i = 0; i < 5; i++)
            {
                draw_1byte(&row, &col, pattern0);
            }

            // 패턴 1/2 를 2회 반복
            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern1);
                draw_1byte(&row, &col, pattern2);
            }
        }
        else if (value == 0b10100000 &&                   // A0
                 _read_uint8_relative(1) == 0b11111000 && // F8
                 _read_uint8_relative(2) == 0b01110010)   // 72
        {
            buf_rseek(reader, 2);

            // 2번씩?
            draw_1byte(&row, &col, 0xf8);
            draw_1byte(&row, &col, 0xf8);
        }
        else if (value == 0b10100000 &&                   // A0
                 _read_uint8_relative(2) == 0b10100000 && // A0
                 _read_uint8_relative(4) == 0b01110100)   // 74
        {
            uint8_t pattern0 = _read_uint8_relative(1);
            uint8_t pattern1 = _read_uint8_relative(3);
            buf_rseek(reader, 4);

            // 2번씩?
            draw_1byte(&row, &col, pattern0);
            draw_1byte(&row, &col, pattern0);
            draw_1byte(&row, &col, pattern1);
            draw_1byte(&row, &col, pattern1);
        }
        else if (value == 0b10100000 &&                   // A0
                 _read_uint8_relative(1) == 0b01111111 && // 7F
                 _read_uint8_relative(2) == 0b01110100)   // 74
        {
            // 1바이트를 읽어서 2번 반복
            uint8_t pattern = _read_uint8();
            draw_1byte(&row, &col, pattern);
            draw_1byte(&row, &col, pattern);

            _read_uint8();
        }
        else if (value == 0b10100000 &&                   // A0
                 _read_uint8_relative(1) == 0b00000101 && // 05
                 _read_uint8_relative(2) == 0b01111100    // 7C
        )
        {
            uint8_t pattern0 = _read_uint8_relative(1);
            buf_rseek(reader, 2);

            // A0 05 를 그냥 출력

            draw_1byte(&row, &col, 0xa0);
            draw_1byte(&row, &col, 0x05);
        }
        else if (value == 0b10100001 &&                   // A1
                 _read_uint8_relative(2) == 0b10100000 && // A0
                 _read_uint8_relative(4) == 0b01110000    // 70
        )
        {
            uint8_t pattern0 = _read_uint8_relative(1);
            uint8_t pattern1 = _read_uint8_relative(3);
            buf_rseek(reader, 4);

            for (int i = 0; i < 3; i++)
            {
                draw_1byte(&row, &col, pattern0);
            }
            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern1);
            }
        }
        else if (value == 0b10100101 && _read_uint8_relative(2) == 0b01111100) // A5 7C
        {
            uint8_t pattern0 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 7; i++)
            {
                draw_1byte(&row, &col, pattern0);
            }
        }
        else if (value == 0b10100000 && _read_uint8_relative(2) == 0b01110000) // A0 70
        {
            uint8_t pattern0 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern0);
            }
        }
        // 패턴 1바이트 반복
        else if (value == 0b10100100 && _read_uint8_relative(2) == 0b01110110) // A4 76
        {
            uint8_t pattern0 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 6; i++)
            {
                draw_1byte(&row, &col, pattern0);
            }
        }
        // 패턴 1번 반복
        else if (value == 0b01100101 && _read_uint8_relative(5) == 0b01110001) // 65 71
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();
            uint8_t pattern2 = _read_uint8();
            uint8_t pattern3 = _read_uint8();

            uint8_t end = _read_uint8();

            draw_1byte(&row, &col, 0x65);
            draw_1byte(&row, &col, pattern0);
            draw_1byte(&row, &col, pattern1);
            draw_1byte(&row, &col, pattern2);
            draw_1byte(&row, &col, pattern3);
        }
        // 패턴 2바이트 3번 반복
        else if (value == 0b01100001 && _read_uint8_relative(3) == 0b01111111) // 61 7F
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }
        }
        // 패턴 2바이트 2번 반복
        else if (value == 0b01100001 && _read_uint8_relative(3) == 0b01110110) // 61 76
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }
        }
        // 패턴 2바이트 2번 반복
        else if (value == 0b01100001 && _read_uint8_relative(3) == 0b01110111) // 61 77
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }
        }
        // 패턴 2바이트 2번 반복
        else if (value == 0b01100001 && _read_uint8_relative(3) == 0b01111110) // 61 7E
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }
        }
        // 패턴 2바이트 2번 반복
        else if (value == 0b01100001 && _read_uint8_relative(3) == 0b01110100) // 61 74
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 2; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }
        }
        // 패턴 2바이트 3번 반복
        else if (value == 0b01100010 && _read_uint8_relative(3) == 0b00101001) // 62 29
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 3; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }
        }
        // 패턴 2바이트 3번 반복
        else if (value == 0b01100010 && _read_uint8_relative(3) == 0b01111100) // 62 7C
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 3; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }
        }
        // 패턴 2바이트 3번 반복
        else if (value == 0b01100010 && _read_uint8_relative(3) == 0b01111111) // 62 7F
        {
            uint8_t pattern0 = _read_uint8();
            uint8_t pattern1 = _read_uint8();

            uint8_t end = _read_uint8();

            for (int i = 0; i < 3; i++)
            {
                draw_1byte(&row, &col, pattern0);
                draw_1byte(&row, &col, pattern1);
            }
        }
        // 위에 12바이트를 그대로 복사
        else if (value == 0b00001100 && _read_uint8_relative(1) == 0b01110011) // 0C 73
        {
            // 일단 위에 4바이트를 읽어서 그대로 복사
            uint8_t pattern[12] = {
                build[build_pos - 12],
                build[build_pos - 11],
                build[build_pos - 10],
                build[build_pos - 9],
                build[build_pos - 8],
                build[build_pos - 7],
                build[build_pos - 6],
                build[build_pos - 5],
                build[build_pos - 4],
                build[build_pos - 3],
                build[build_pos - 2],
                build[build_pos - 1]};

            // Read dummy
            _read_uint8();

            for (int i = 0; i < 12; i++)
            {
                draw_1byte(&row, &col, pattern[i]);
            }
        }
        else if (value == 0b00100001 && _read_uint8_relative(1) == 0b00001101) // 21 0D
        {
            _read_uint8();
            draw_1byte(&row, &col, 0b00101010);
            draw_1byte(&row, &col, 0b01010101);
            draw_1byte(&row, &col, 0b00101010);
            draw_1byte(&row, &col, 0b01010101);
        }
        else if (value == 0b00100001 && _read_uint8_relative(3) == 0b10100101) // 21 A5
        {
            // 일단 위에 4바이트를 읽어서 그대로 복사
            uint8_t pattern[4] = {
                build[build_pos - 4],
                build[build_pos - 3],
                build[build_pos - 2],
                build[build_pos - 1]};

            // Read dummy
            _read_uint8();
            _read_uint8();
            _read_uint8();

            draw_1byte(&row, &col, pattern[0]);
            draw_1byte(&row, &col, pattern[1]);
            draw_1byte(&row, &col, pattern[2]);
            draw_1byte(&row, &col, pattern[3]);

            uint8_t pattern2[2] = {
                build[build_pos - 2],
                build[build_pos - 1]};

            // 마지막 2바이트를 5회 복사
            for (int i = 0; i < 5; i++)
            {
                draw_1byte(&row, &col, pattern2[0]);
                draw_1byte(&row, &col, pattern2[1]);
            }
        }
        else if (value == 0b10011111 && _read_uint8_relative(1) == 0b01111010) // 9F 7A
        {
            // 9F 7A는 그냥 9F
            draw_1byte(&row, &col, 0x9F);
            _read_uint8();
        }
        else if (value == 0b00000101 && _read_uint8_relative(1) == 0b01110111) // 05 77
        {
            _read_uint8();
            draw_1byte(&row, &col, 0x01);
            draw_1byte(&row, &col, 0x01);
        }
        else if (value == 0b10000001 && _read_uint8_relative(1) == 0b01011010) // 81 5A
        {
            _read_uint8();
            draw_1byte(&row, &col, 0xaa);
            draw_1byte(&row, &col, 0x55);
            draw_1byte(&row, &col, 0xaa);
            draw_1byte(&row, &col, 0x55);
        }
        else if (value == 0b10101010 && _read_uint8_relative(1) == 0b01110110) // AA 76
        {
            _read_uint8();
            draw_1byte(&row, &col, 0xaa);
        }
        else if (value == 0b00000110 && _read_uint8_relative(1) == 0b01110110) // 06 76
        {
            _read_uint8();
            draw_1byte(&row, &col, 0xff);
            draw_1byte(&row, &col, 0xff);
        }
        else if (value == 0b00100000 &&                   // 20
                 _read_uint8_relative(1) == 0b10001101 && // 8D
                 _read_uint8_relative(2) == 0b01111111)   // 7F
        {
            buf_rseek(reader, 2);

            draw_1byte(&row, &col, 0x01);
            draw_1byte(&row, &col, 0x02);
            draw_1byte(&row, &col, 0x01);
        }
        else if (value == 0b00100000 &&                   // 20
                 _read_uint8_relative(1) == 0b00100001 && // 21
                 _read_uint8_relative(2) == 0b01110010)   // 72
        {
            buf_rseek(reader, 3);
            draw_1byte(&row, &col, 0x54);
            draw_1byte(&row, &col, 0xaa);
            draw_1byte(&row, &col, 0x54);
            draw_1byte(&row, &col, 0xea);
        }
        else if (value == 0b00000110 && _read_uint8_relative(1) == 0b01110101) // 06 75
        {
            buf_rseek(reader, 1);
            draw_1byte(&row, &col, 0xff);
            draw_1byte(&row, &col, 0xff);
        }
        else if (value == 0b00000110) // 06
        {
            inc_inst(value);
            printf("Inst Count: %d\n", inst_count(value));

            if (inst_count(value) == 2 || inst_count(value) == 3 || inst_count(value) == 4)
            {
                // 두 번째 06 은 흰 줄 2개
                draw_1byte(&row, &col, 0xff);
                draw_1byte(&row, &col, 0xff);
            }
            else
            {
                draw_1byte(&row, &col, value);
            }
        }
        else if (value == 0b01111010) // 7A
        {
            inc_inst(value);
            printf("Inst Count: %d\n", inst_count(value));

            // Draw Count
            if (inst_count(value) == 1 || inst_count(value) == 2 || inst_count(value) == 3 || inst_count(value) == 4)
            {
                // draw_1byte(&row, &col, value);
            }
        }
        else if (value == 0b01111101) // 7D
        {
            inc_inst(value);
            printf("Inst Count: %d\n", inst_count(value));

            // Draw Count
            if (inst_count(value) == 1 || inst_count(value) == 2 || inst_count(value) == 3 || inst_count(value) == 4)
            {
                draw_1byte(&row, &col, value);
            }
        }
        else if (value == 0b01111111) // 7F
        {
            inc_inst(value);
            printf("Inst Count: %d\n", inst_count(value));

            // 4번째는 그림
            if (inst_count(value) == 4 || inst_count(value) == 5 ||
                inst_count(value) == 8 || inst_count(value) == 9 || inst_count(value) == 10 || inst_count(value) == 11 || inst_count(value) == 16)
            {
                draw_1byte(&row, &col, value);
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
