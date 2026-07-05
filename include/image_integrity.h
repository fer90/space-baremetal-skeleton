#ifndef IMAGE_INTEGRITY_H
#define IMAGE_INTEGRITY_H

#include <stdbool.h>
#include <stdint.h>

#define IMAGE_INTEGRITY_REASON_CODE 0x05u

bool image_integrity_verify(void);
bool image_integrity_boot_ok(void);
uint32_t image_integrity_computed_crc(void);
uint32_t image_integrity_expected_crc(void);

uint32_t image_integrity_crc_region_ex(const uint8_t *start,
                                       const uint8_t *slot,
                                       const uint8_t *end);

#endif /* IMAGE_INTEGRITY_H */