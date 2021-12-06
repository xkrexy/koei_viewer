#include "buf_reader.h"
#include "ls11_decoder.h"
#include <string.h>

int main(int argc, char *argv[]) {
    FILE *fp = fopen("hero/FACEDAT.R3", "r");
    //FILE *fp = fopen("sam4/KAODATA.S4", "r");
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *buf = new uint8_t[filesize];
    size_t ret = fread(buf, 1, filesize, fp);
    printf("FileSize: %ld / Read: %ld\n", filesize, ret);
    fclose(fp);

    buf_reader_t *reader = create_buffer_reader(buf, filesize, false);

    int index = 0;
    while (!buf_end(reader)) {
        int address = read_uint32(reader);
        int datasize = read_uint16(reader);
        int curpos = buf_get_seek_pos(reader);
        printf("Value[%3d]: Seek(%5d) %8d %5d\n", index, curpos - 6, address, datasize);

        // Save uncompressed file (testing)
        // char path[32];
        // sprintf(path, "temp/%03d.dat", index);
        // FILE *fp = fopen(path, "w");

        // uint8_t *comp = new uint8_t[datasize];
        // uint8_t *uncomp = new uint8_t[1920];

        // memset(comp, 0, datasize);
        // memset(uncomp, 0, 1920);

        // buf_seek(reader, address);
        // read_bytes(reader, comp, datasize);

        // fat_decode(comp, datasize, uncomp, 1920);
        // fwrite(uncomp, 1, 1920, fp);

        // fclose(fp);

        // delete[] comp;
        // delete[] uncomp;

        buf_seek(reader, curpos);

        index++;
    }

    printf("Last buffer seek pos: %d(0x%08x)\n", buf_get_seek_pos(reader), buf_get_seek_pos(reader));

    delete[] buf;

    return 0;
}
