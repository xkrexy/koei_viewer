#include <string.h>
#include <sys/stat.h>
#include <vector>

#include "buf_reader.h"

struct ENTITY
{
    uint32_t addr;
    uint16_t size;
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <packed filename> <output dir>\n", argv[0]);
        return 0;
    }

    mkdir(argv[2], 0777);

    FILE *fp = fopen(argv[1], "r");
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *buf = new uint8_t[filesize];
    size_t ret = fread(buf, 1, filesize, fp);
    fclose(fp);

    buf_reader_t *reader = create_buffer_reader(buf, filesize, false);

    std::vector<ENTITY> entities;

    int index = 0;
    int data_sum = 0;
    while (!buf_end(reader))
    {
        uint32_t address = read_uint32(reader);
        uint16_t datasize = read_uint16(reader);
        int32_t curpos = buf_get_seek_pos(reader);

        ENTITY entity;
        entity.addr = address;
        entity.size = datasize;
        entities.push_back(entity);

        data_sum += datasize;
        if (curpos + data_sum >= filesize)
        {
            int data_start = buf_get_seek_pos(reader) - 6;
            printf("\nAll unpacked!\n");
            printf("Count: %d\n", index + 1);
            break;
        }

        index++;
    }

    size_t header_size = entities.size() * (sizeof(uint32_t) + sizeof(uint16_t));
    printf("Count: %lu\n", entities.size());
    printf("Header size: %lu\n", header_size);

    for (int i = 0; i < (int)entities.size(); i++)
    {
        // Save compreseed
        char path[32];
        sprintf(path, "%s/%03d.UNPACKED", argv[2], i);
        FILE *fp = fopen(path, "w");

        uint8_t *comp = new uint8_t[entities[i].size];
        memset(comp, 0, entities[i].size);

        buf_seek(reader, entities[i].addr + header_size);
        read_bytes(reader, comp, entities[i].size);
        fwrite(comp, 1, entities[i].size, fp);
        fclose(fp);

        delete[] comp;
    }

    delete[] buf;
    destroy_buffer_reader(reader);

    return 0;
}
