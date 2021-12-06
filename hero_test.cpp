#include "buf_reader.h"
#include "ls11_decoder.h"
#include <string.h>

int main(int argc, char *argv[]) {
    FILE *fp = fopen("hero/FACEDAT.R3", "r");
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *buf = new uint8_t[filesize];
    size_t ret = fread(buf, 1, filesize, fp);
    printf("FileSize: %d / Read: %d\n", filesize, ret);
    fclose(fp);

    buf_reader_t *reader = create_buffer_reader(buf, filesize, false);
    int header = read_int32(reader);
    // header must be 0x00000000

    int index = 0;
    while (index < 240) {
        int curpos = buf_get_seek_pos(reader);
        int datasize = read_uint16(reader);
        int address = read_uint32(reader);
        printf("Value[%d]: %5d %8d\n", index, datasize, address);

        char path[32];
        sprintf(path, "temp/%03d.dat", index);
        FILE *fp = fopen(path, "w");

        uint8_t *comp = new uint8_t[datasize];
        uint8_t *uncomp = new uint8_t[1920];

        memset(comp, 0, datasize);
        memset(uncomp, 0, 1920);

        buf_seek(reader, address);
        read_bytes(reader, comp, datasize);

        fat_decode(comp, datasize, uncomp, 1920);
        fwrite(uncomp, 1, 1920, fp);

        fclose(fp);

        delete[] comp;
        delete[] uncomp;

        buf_seek(reader, curpos);

        index++;
    }

    delete[] buf;

    return 0;
}
