
#include <criterion/criterion.h>
#include <stdint.h>
#include <stdlib.h>

#include "../../include/poison.h"
#include "../test_suite.h"

static uint8_t *get_shadow(void *ptr)
{
    uintptr_t address = (uintptr_t)ptr;
    uintptr_t shadow_address = SHADOW_OFFSET + (address >> 3);

    return (uint8_t *)shadow_address;
}

Test(poison, malloc_full_granules)
{
    size_t size = 16;
    size_t total_size = REDZONE_SIZE + size + REDZONE_SIZE;

    unsigned char *raw = my_asan.real_malloc(total_size);
    cr_assert_not_null(raw);

    unsigned char *ptr = raw + REDZONE_SIZE;

    poison_shadow_malloc(ptr, size);

    uint8_t *shadow = get_shadow(ptr);
    size_t redzone_size = REDZONE_SIZE / 8;

    cr_assert_eq(shadow[0], 0x00);
    cr_assert_eq(shadow[1], 0x00);
    cr_assert_eq(shadow[2], 0xFA);
    cr_assert_eq(shadow[-(ptrdiff_t)redzone_size], 0xFA);

    my_asan.real_free(raw);
}

Test(poison, malloc_partial_granule)
{
    size_t size = 20;
    size_t total_size = REDZONE_SIZE + size + REDZONE_SIZE;

    unsigned char *raw = my_asan.real_malloc(total_size);
    cr_assert_not_null(raw);

    unsigned char *ptr = raw + REDZONE_SIZE;

    poison_shadow_malloc(ptr, size);

    uint8_t *shadow = get_shadow(ptr);

    cr_assert_eq(shadow[0], 0x00);
    cr_assert_eq(shadow[1], 0x00);
    cr_assert_eq(shadow[2], 0x04);
    cr_assert_eq(shadow[3], 0xFA);

    my_asan.real_free(raw);
}

Test(poison, free_marks_allocation)
{
    size_t size = 16;
    size_t total_size = REDZONE_SIZE + size + REDZONE_SIZE;

    unsigned char *raw = my_asan.real_malloc(total_size);
    cr_assert_not_null(raw);

    unsigned char *ptr = raw + REDZONE_SIZE;

    poison_shadow_malloc(ptr, size);
    poison_shadow_free(ptr, size);

    uint8_t *shadow = get_shadow(ptr);

    cr_assert_eq(shadow[0], 0xFD);
    cr_assert_eq(shadow[1], 0xFD);

    my_asan.real_free(raw);
}
