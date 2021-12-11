#include <iostream>
#include <map>
#include "buf_reader.h"

struct INST_INFO
{
    int param_count;
    void (*handler)(INST_INFO *info);
};

std::map<int, INST_INFO> inst_map;

void buildUpInst();
void draw_1byte(uint8_t byte);
bool is_inst(uint8_t value);

void handler_61(INST_INFO *info);
void handler_62(INST_INFO *info);
size_t getFileSize(FILE *fp);
char *bitString(uint8_t byte);

uint8_t *buf;
buf_reader_t *r;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <unpacked file>" << std::endl;
        return 0;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp)
    {
        std::cout << "File not found: " << argv[1] << std::endl;
        return 0;
    }

    size_t fileSize = getFileSize(fp);
    buf = new uint8_t[fileSize];
    fread(buf, 1, fileSize, fp);
    fclose(fp);

    r = create_buffer_reader(buf, fileSize, true);

    while (!buf_end(r))
    {
    }

    destroy_buffer_reader(r);
    delete[] buf;
    return 0;
}

void handler_61(INST_INFO *info)
{
    uint8_t patterns[2];
    read_bytes(r, patterns, 2);

    for (int i = 0; i < 2; i++)
    {
        draw_1byte(patterns[0]);
        draw_1byte(patterns[1]);
    }
}

void handler_62(INST_INFO *info)
{
    uint8_t patterns[2];
    read_bytes(r, patterns, 2);

    for (int i = 0; i < 3; i++)
    {
        draw_1byte(patterns[0]);
        draw_1byte(patterns[1]);
    }
}

void buildUpInst()
{
    INST_INFO i61 = {2, handler_61};
    INST_INFO i62 = {2, handler_62};
}

void draw_1byte(uint8_t byte)
{
    printf("%s\n", bitString(byte));
}

bool is_inst(uint8_t byte)
{
}

char *bitString(uint8_t byte)
{
    static char _str[9];
    for (int i = 0; i < 8; i++)
    {
        _str[7 - i] = '0' + ((byte >> i) & 0x01);
    }

    _str[8] = 0;
    return _str;
}

size_t getFileSize(FILE *fp)
{
    long current = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size_t size = (size_t)ftell(fp);
    fseek(fp, current, SEEK_SET);
    return size;
}