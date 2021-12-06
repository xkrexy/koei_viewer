#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <arpa/inet.h>
#endif

#include "buf_reader.h"

buf_reader_t *create_buffer_reader(uint8_t *buf, size_t size, bool big_endian)
{
    buf_reader_t *buf_reader = (buf_reader_t *)malloc(sizeof(buf_reader_t));
    memset(buf_reader, 0, sizeof(buf_reader_t));

    buf_reader->buf = buf;
    buf_reader->size = size;
    buf_reader->seek_pos = 0;
    buf_reader->big_endian = big_endian;

    return buf_reader;
}

void destroy_buffer_reader(buf_reader_t *buf_reader)
{
    if (buf_reader)
    {
        free(buf_reader);
    }
}

void buf_seek(buf_reader_t *buf_reader, int32_t seek_pos)
{
    if (buf_reader)
    {
        buf_reader->seek_pos = seek_pos;
    }
}

bool buf_end(buf_reader_t *buf_reader)
{
    if (buf_reader)
    {
        return buf_reader->seek_pos >= buf_reader->size;
    }

    return true;
}

int32_t buf_get_seek_pos(buf_reader_t *buf_reader)
{
    if (buf_reader)
    {
        return buf_reader->seek_pos;
    }
    return 0;
}

void read_bytes(buf_reader_t *buf_reader, uint8_t *buf, size_t size)
{
    if (buf_reader)
    {
        memcpy(buf, buf_reader->buf + buf_reader->seek_pos, size);
        buf_reader->seek_pos += size;
    }
}

int8_t read_int8(buf_reader_t *buf_reader)
{
    if (buf_reader)
    {
        int8_t value = 0;
        value = *(int8_t *)(buf_reader->buf + buf_reader->seek_pos);
        buf_reader->seek_pos += sizeof(int8_t);

        return value;
    }

    return 0;
}

int16_t read_int16(buf_reader_t *buf_reader)
{
    if (buf_reader)
    {
        int16_t value = 0;
        value = *(int16_t *)(buf_reader->buf + buf_reader->seek_pos);
        buf_reader->seek_pos += sizeof(int16_t);

        if (buf_reader->big_endian)
        {
            value = htons(value);
        }

        return value;
    }

    return 0;
}

int32_t read_int32(buf_reader_t *buf_reader)
{
    if (buf_reader)
    {
        int32_t value = 0;
        value = *(int32_t *)(buf_reader->buf + buf_reader->seek_pos);
        buf_reader->seek_pos += sizeof(int32_t);

        if (buf_reader->big_endian)
        {
            value = htonl(value);
        }

        return value;
    }

    return 0;
}

uint8_t read_uint8(buf_reader_t *buf_reader)
{
    return (uint8_t)read_int8(buf_reader);
}

uint16_t read_uint16(buf_reader_t *buf_reader)
{
    return (uint16_t)read_int16(buf_reader);
}

uint32_t read_uint32(buf_reader_t *buf_reader)
{
    return (uint32_t)read_int32(buf_reader);
}
