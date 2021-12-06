#include "buf_reader.h"
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *fp = fopen("facedat/facedat.000", "r");
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *buf = new uint8_t[filesize];
    size_t ret = fread(buf, 1, filesize, fp);
    fclose(fp);

    buf_reader_t *reader = create_buffer_reader(buf, filesize, false);
    destroy_buffer_reader(reader);

    delete[] buf;

    return 0;
}
