#include "buf_reader.h"
#include <string.h>
#include <sys/stat.h>

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

    int index = 0;
    int data_sum = 0;
    while (!buf_end(reader))
    {
        int address = read_uint32(reader);
        int datasize = read_uint16(reader);
        int curpos = buf_get_seek_pos(reader);

        // Save compreseed
        char path[32];
        sprintf(path, "%s/%s.%03d", argv[2], argv[2], index);
        FILE *fp = fopen(path, "w");

        uint8_t *comp = new uint8_t[datasize];
        memset(comp, 0, datasize);

        buf_seek(reader, address + 2040);
        read_bytes(reader, comp, datasize);
        fwrite(comp, 1, datasize, fp);
        fclose(fp);

        delete[] comp;

        buf_seek(reader, curpos);

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

    delete[] buf;
    destroy_buffer_reader(reader);

    return 0;
}
