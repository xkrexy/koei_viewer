#include <stdio.h>
#include <stdint.h>
#include <vector>

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

void ls11_decode(const char *filename, void (*on_decode)(uint8_t *data, uint32_t size))
{
    FILE *fp = fopen(filename, "r");
    if (fp)
    {
        fread(header, 1, 16, fp);
        fread(dict, 1, 256, fp);

        // Decode Fat
        while (!feof(fp))
        {
            FAT_ENTRY fat_entry;
            memset(&fat_entry, 0, sizeof(FAT_ENTRY));
            fread(&fat_entry, sizeof(FAT_ENTRY), 1, fp);

            // Little Endian to Big Endian
            fat_entry.comp_size = htonl(fat_entry.comp_size);
            fat_entry.uncomp_size = htonl(fat_entry.uncomp_size);
            fat_entry.offset = htonl(fat_entry.offset);

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

            fseek(fp, fat_entries[i].offset, SEEK_SET);
            fread(comp_data, 1, fat_entries[i].comp_size, fp);

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

        fclose(fp);
    }
}
