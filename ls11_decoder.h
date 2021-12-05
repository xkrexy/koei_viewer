#include <stdint.h>
#include <functional>
#include "buf_reader.h"

void ls11_decode(uint8_t *buf, size_t size, std::function<void(uint8_t *buf, uint32_t size)> on_decode);
