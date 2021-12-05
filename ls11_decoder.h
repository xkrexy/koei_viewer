#include <stdint.h>

void ls11_decode(const char *filename, void (*on_decode)(uint8_t *buf, uint32_t size));
