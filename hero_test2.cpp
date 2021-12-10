#include <stdio.h>
#include <stdint.h>
#include "buf_reader.h"

uint8_t read_next(buf_reader_t *reader, int32_t pos)
{
    uint8_t value = reader->buf[reader->seek_pos + pos - 1];
    return value;
}

int main(int argc, char *argv[])
{
    FILE *fp = fopen("FACEDAT/003.UNPACKED", "r");

    if (fp)
    {
        uint8_t buf[1592];
        fread(buf, 1, 1592, fp);
        buf_reader_t *reader = create_buffer_reader(buf, 1592, false);

        buf_seek(reader, 27);

        bool inst_mode = false;
        int params = 0;

        while (!buf_end(reader))
        {
            uint8_t value = read_uint8(reader);
            uint8_t next = read_next(reader, 2);
            bool next_is_inst = ((next & 0xF0) == 0xA0);
            next_is_inst |= ((next & 0xF0) == 0x70);
            next_is_inst |= ((next == 0x61) || (next == 0x62));

            if (value == 0x61 || value == 0x62)
            {
                if (!inst_mode)
                {
                    printf("[%04X] [", buf_get_seek_pos(reader) - 1);
                }
                inst_mode = true;

                params = 2 + 1;
            }
            if ((value >= 0xA0 && value <= 0xA5) && next_is_inst)
            {
                if (!inst_mode)
                {
                    printf("[%04X] [", buf_get_seek_pos(reader) - 1);
                }
                inst_mode = true;

                params = 1 + 1;
            }

            if (inst_mode)
            {
                printf("%02X ", value);

                if (params > 0)
                    params--;
                if (params == 0 && ((value & 0xF0) == 0x70))
                {
                    printf("]\n");
                    inst_mode = false;
                }
            }
            else if (!inst_mode)
            {
                printf("Normal Pixel: %02X\n", value);
            }
        }

        destroy_buffer_reader(reader);
        fclose(fp);
    }

    return 0;
}
