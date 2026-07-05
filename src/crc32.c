#include "crc32.h"

#define CRC32_POLY 0xEDB88320u

uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t length)
{
    size_t index;

    if (data == NULL) {
        return crc;
    }

    for (index = 0; index < length; index++) {
        uint32_t bit;
        uint8_t byte = data[index];

        crc ^= (uint32_t) byte;
        for (bit = 0; bit < 8u; bit++) {
            if ((crc & 1u) != 0u) {
                crc = (crc >> 1) ^ CRC32_POLY;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

uint32_t crc32_compute(const uint8_t *data, size_t length)
{
    return crc32_update(CRC32_INIT, data, length) ^ CRC32_INIT;
}