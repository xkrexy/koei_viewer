#include <stdio.h>
#include <stdint.h>
#include <vector>

#include "ls11_decoder.h"

uint8_t header[16];
uint8_t dict[256];

struct FAT_ENTRY
{
    uint32_t comp_size;
    uint32_t uncomp_size;
    uint32_t offset;
};

std::vector<FAT_ENTRY> fat_entries;

uint32_t comp_pos = 0;
uint32_t uncomp_pos = 0;
int32_t bit_pos = 7;

static int get_code(uint8_t *comp, uint8_t *uncomp)
{
    int code1 = 0;
    int code2 = 0;
    int count = 0;
    int bit = 0;

    do
    {
        bit = (comp[comp_pos] >> bit_pos) & 0x01;
        code1 = (code1 << 1) | bit;
        count++;
        bit_pos--;
        if (bit_pos < 0)
        {
            bit_pos = 7;
            comp_pos++;
        }
    } while (bit == 1);

    for (int i = 0; i < count; i++)
    {
        bit = (comp[comp_pos] >> bit_pos) & 0x01;
        code2 = (code2 << 1) | bit;
        bit_pos--;
        if (bit_pos < 0)
        {
            bit_pos = 7;
            comp_pos++;
        }
    }

    return (code1 + code2);
}

static void fat_decode(uint8_t *comp, uint32_t comp_size, uint8_t *uncomp, uint32_t uncomp_size)
{
    static const int MR = 3;
    comp_pos = 0;
    uncomp_pos = 0;
    bit_pos = 7;

    while (comp_pos < comp_size && uncomp_pos < uncomp_size)
    {
        int code = get_code(comp, uncomp);
        if (code < 256)
        {
            uncomp[uncomp_pos] = dict[code];
            uncomp_pos++;
        }
        else
        {
            int offset = code - 256;
            int len = get_code(comp, uncomp) + MR;
            for (int i = 0; i < len; i++)
            {
                uncomp[uncomp_pos] = uncomp[uncomp_pos - offset];
                uncomp_pos++;
            }
        }
    }
}

void ls11_decode(uint8_t *buf, size_t size, std::function<void(uint8_t *buf, uint32_t size)> on_decode)
{
    if (!buf)
    {
        return;
    }

    buf_reader_t *reader = create_buffer_reader(buf, size, true);
    fat_entries.clear();

    read_bytes(reader, header, 16);
    read_bytes(reader, dict, 256);

    // Decode Fat
    while (!buf_end(reader))
    {
        FAT_ENTRY fat_entry;
        memset(&fat_entry, 0, sizeof(FAT_ENTRY));

        // Default endian of reader is big endian.
        // Values of fat_entry are also big endian.
        fat_entry.comp_size = read_uint32(reader);
        fat_entry.uncomp_size = read_uint32(reader);
        fat_entry.offset = read_uint32(reader);

        if (fat_entry.comp_size == 0)
        {
            break;
        }

        fat_entries.push_back(fat_entry);
    }

    // Decode Dat
    for (int i = 0; i < fat_entries.size(); i++)
    {
        uint8_t *comp_data = new uint8_t[fat_entries[i].comp_size];
        uint8_t *uncomp_data = new uint8_t[fat_entries[i].uncomp_size];

        memset(comp_data, 0, fat_entries[i].comp_size);
        memset(uncomp_data, 0, fat_entries[i].uncomp_size);

        buf_seek(reader, fat_entries[i].offset);
        read_bytes(reader, comp_data, fat_entries[i].comp_size);

        if (fat_entries[i].comp_size == fat_entries[i].uncomp_size)
        {
            memcpy(uncomp_data, comp_data, fat_entries[i].comp_size);
        }
        else
        {
            fat_decode(comp_data, fat_entries[i].comp_size, uncomp_data, fat_entries[i].uncomp_size);
        }

        if (on_decode)
        {
            on_decode(uncomp_data, fat_entries[i].uncomp_size);
        }

        delete[] comp_data;
        delete[] uncomp_data;
    }

    destroy_buffer_reader(reader);
}
