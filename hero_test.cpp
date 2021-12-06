#include "buf_reader.h"
#include <string.h>

int main(int argc, char *argv[]) {
    //FILE *fp = fopen("hero/FACEDAT.R3", "r");
    FILE *fp = fopen("sam4/KAODATA.S4", "r");
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *buf = new uint8_t[filesize];
    size_t ret = fread(buf, 1, filesize, fp);
    fclose(fp);

    buf_reader_t *reader = create_buffer_reader(buf, filesize, false);

    // File Size: 341808
    // File Size - 1440 = 340368
    // Size 총합: 340368
    // 0번 FAT Size: 1302

    // 1440 까지 Header고, 1440부터 끝까지는 전부 데이터
    // 즉 Dict가 다른 파일에 있을 가능성이 높음

    int index = 0;
    int data_sum = 0;
    while (!buf_end(reader)) {
        int address = read_uint32(reader);
        int datasize = read_uint16(reader);
        int curpos = buf_get_seek_pos(reader);
        printf("Value[%3d]: Seek(%5d) %8d %5d\n", index, curpos - 6, address, datasize);

        if (address >= filesize) {
            int data_start = buf_get_seek_pos(reader) - 6;
            printf("\n>> Result\n");
            printf("FileSize: %ld / Read: %ld\n", filesize, ret);
            printf("Data starts at %d\n", data_start);
            printf("Data Size Sum: %d\n", data_sum);
            printf("%ld - %d = %ld\n", filesize, data_start, filesize - data_start);
            break;
        }

        data_sum += datasize;

#if 1
        // Save compreseed
        char path[32];
        sprintf(path, "temp/%03d.DAT", index);
        FILE *fp = fopen(path, "w");

        uint8_t *comp = new uint8_t[datasize];
        memset(comp, 0, datasize);

        buf_seek(reader, address + 2040);
        read_bytes(reader, comp, datasize);

        fwrite(comp, 1, datasize, fp);

        fclose(fp);

        delete[] comp;
#endif

#if 0
        // Save uncompressed file (testing)
        char path[32];
        sprintf(path, "temp/%03d.DAT", index);
        FILE *fp = fopen(path, "w");

        uint8_t *comp = new uint8_t[datasize];
        uint8_t *uncomp = new uint8_t[1920];

        memset(comp, 0, datasize);
        memset(uncomp, 0, 1920);

        buf_seek(reader, address + 1440 + 1302);
        read_bytes(reader, comp, datasize);

        fat_decode(comp, datasize, uncomp, 1920);
        fwrite(uncomp, 1, 1920, fp);

        fclose(fp);

        delete[] comp;
        delete[] uncomp;
#endif

        buf_seek(reader, curpos);

        index++;
    }

    delete[] buf;

    return 0;
}
