#pragma once

#include <stdio.h>
#include <stdint.h>

typedef struct
{
    uint8_t *buf;
    size_t size;
    int32_t seek_pos;
    bool big_endian;
} buf_reader_t;

buf_reader_t *create_buffer_reader(uint8_t *buf, size_t size, bool big_endian);
void destroy_buffer_reader(buf_reader_t *buf_reader);

void buf_seek(buf_reader_t *buf_reader, int32_t seek_pos);
void buf_rseek(buf_reader_t *buf_reader, int32_t seek_amount);
int32_t buf_get_seek_pos(buf_reader_t *buf_reader);
bool buf_end(buf_reader_t *buf_reader);

void read_bytes(buf_reader_t *buf_reader, uint8_t *buf, size_t size);
int8_t read_int8(buf_reader_t *buf_reader);
int16_t read_int16(buf_reader_t *buf_reader);
int32_t read_int32(buf_reader_t *buf_reader);
uint8_t read_uint8(buf_reader_t *buf_reader);
uint16_t read_uint16(buf_reader_t *buf_reader);
uint32_t read_uint32(buf_reader_t *buf_reader);
