#include "unity.h"

#include "image_integrity.h"

#include <string.h>

void test_image_integrity_region_skips_crc_slot(void)
{
    uint8_t image[64];
    uint8_t *start = image;
    uint8_t *slot = image + 32;
    uint8_t *end = image + sizeof(image);
    uint32_t digest;

    memset(image, 0x5Au, sizeof(image));
    digest = image_integrity_crc_region_ex(start, slot, end);
    ((uint32_t *) slot)[0] = digest;

    TEST_ASSERT_EQUAL_UINT32(digest, image_integrity_crc_region_ex(start, slot, end));
}

void test_image_integrity_region_detects_corruption(void)
{
    uint8_t image[64];
    uint8_t *start = image;
    uint8_t *slot = image + 32;
    uint8_t *end = image + sizeof(image);
    uint32_t digest;

    memset(image, 0xAAu, sizeof(image));
    digest = image_integrity_crc_region_ex(start, slot, end);
    ((uint32_t *) slot)[0] = digest;
    image[0] ^= 0x01u;

    TEST_ASSERT_NOT_EQUAL(digest, image_integrity_crc_region_ex(start, slot, end));
}