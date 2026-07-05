#include "image_integrity.h"
#include "crc32.h"

#ifndef HOST_TEST
#include "log.h"
#include "uart.h"

extern const uint8_t __image_integrity_start[];
extern const uint8_t __image_crc_slot[];
extern const uint8_t __image_integrity_end[];

extern const uint32_t g_image_crc_expected;

static bool image_integrity_ok = true;
static uint32_t image_integrity_computed;
static uint32_t image_integrity_expected;
#endif

uint32_t image_integrity_crc_region_ex(const uint8_t *start,
                                       const uint8_t *slot,
                                       const uint8_t *end)
{
    uint32_t crc = CRC32_INIT;

    if (start == NULL || slot == NULL || end == NULL || slot < start ||
        end < slot || (slot + sizeof(uint32_t)) > end) {
        return 0u;
    }

    crc = crc32_update(crc, start, (size_t) (slot - start));
    crc = crc32_update(crc, slot + sizeof(uint32_t),
                        (size_t) (end - (slot + sizeof(uint32_t))));

    return crc ^ CRC32_INIT;
}

#ifndef HOST_TEST
static uint32_t image_integrity_crc_region(void)
{
    return image_integrity_crc_region_ex(__image_integrity_start,
                                         __image_crc_slot,
                                         __image_integrity_end);
}

bool image_integrity_verify(void)
{
    image_integrity_expected = g_image_crc_expected;
    image_integrity_computed = image_integrity_crc_region();

    if (image_integrity_expected == 0u) {
        image_integrity_ok = false;
        uart_puts(LOG_PREFIX_ERROR "image CRC slot not patched (expected=0)\r\n");
        return false;
    }

    image_integrity_ok = (image_integrity_computed == image_integrity_expected);
    if (image_integrity_ok) {
        uart_puts(LOG_PREFIX_BOOT "image CRC OK (0x");
        uart_put_hex(image_integrity_computed);
        uart_puts(")\r\n");
    } else {
        uart_puts(LOG_PREFIX_ERROR "image CRC mismatch expected=0x");
        uart_put_hex(image_integrity_expected);
        uart_puts(" computed=0x");
        uart_put_hex(image_integrity_computed);
        uart_puts("\r\n");
    }

    return image_integrity_ok;
}

bool image_integrity_boot_ok(void)
{
    return image_integrity_ok;
}

uint32_t image_integrity_computed_crc(void)
{
    return image_integrity_computed;
}

uint32_t image_integrity_expected_crc(void)
{
    return image_integrity_expected;
}
#endif /* HOST_TEST */