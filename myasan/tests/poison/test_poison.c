
#include <criterion/criterion.h>
#include <stdint.h>

#include "../../include/poison.h"
#include "../test_suite.h"

static uint8_t *get_shadow(void *ptr)
{
    uintptr_t address = (uintptr_t)ptr;

    return (uint8_t *)(SHADOW_OFFSET + (address >> 3));
}

static void *create_test_ptr(void)
{
    return my_asan.real_malloc(1024);
}

Test(poison, valid_access)
{
    void *raw = create_test_ptr();
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    uint8_t *shadow = get_shadow(ptr);
    uint8_t *bad = NULL;

    shadow[0] = 0x00;

    enum memory_error error = poison_is_poisoned(ptr, 8, &bad);

    cr_assert_eq(error, MEMORY_ERROR_NONE);
    cr_assert_null(bad);

    my_asan.real_free(raw);
}

Test(poison, buffer_overflow)
{
    void *raw = create_test_ptr();
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    uint8_t *shadow = get_shadow(ptr);
    uint8_t *bad = NULL;

    shadow[0] = 0xFA;

    enum memory_error error = poison_is_poisoned(ptr, 1, &bad);

    cr_assert_eq(error, MEMORY_ERROR_BUFFER_OVERFLOW);
    cr_assert_eq(bad, &shadow[0]);

    my_asan.real_free(raw);
}

Test(poison, use_after_free)
{
    void *raw = create_test_ptr();
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    uint8_t *shadow = get_shadow(ptr);
    uint8_t *bad = NULL;

    shadow[0] = 0xFD;

    enum memory_error error = poison_is_poisoned(ptr, 1, &bad);

    cr_assert_eq(error, MEMORY_ERROR_USE_AFTER_FREE);
    cr_assert_eq(bad, &shadow[0]);

    my_asan.real_free(raw);
}

Test(poison, invalid_shadow_value)
{
    void *raw = create_test_ptr();
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    uint8_t *shadow = get_shadow(ptr);
    uint8_t *bad = NULL;

    shadow[0] = 0x80;

    enum memory_error error = poison_is_poisoned(ptr, 1, &bad);

    cr_assert_eq(error, MEMORY_ERROR_INVALID_PTR);
    cr_assert_eq(bad, &shadow[0]);

    my_asan.real_free(raw);
}

Test(poison, partial_granule_valid_access)
{
    void *raw = create_test_ptr();
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    uint8_t *shadow = get_shadow(ptr);
    uint8_t *bad = NULL;

    shadow[0] = 0x04;

    enum memory_error error = poison_is_poisoned(ptr, 4, &bad);

    cr_assert_eq(error, MEMORY_ERROR_NONE);
    cr_assert_null(bad);

    my_asan.real_free(raw);
}

Test(poison, partial_granule_overflow)
{
    void *raw = create_test_ptr();
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    uint8_t *shadow = get_shadow(ptr);
    uint8_t *bad = NULL;

    shadow[0] = 0x04;

    enum memory_error error = poison_is_poisoned(ptr, 5, &bad);

    cr_assert_eq(error, MEMORY_ERROR_BUFFER_OVERFLOW);
    cr_assert_eq(bad, &shadow[0]);

    my_asan.real_free(raw);
}

Test(poison, overflow_in_second_granule)
{
    void *raw = create_test_ptr();
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    uint8_t *shadow = get_shadow(ptr);
    uint8_t *bad = NULL;

    shadow[0] = 0x00;
    shadow[1] = 0xFA;

    enum memory_error error = poison_is_poisoned(ptr, 9, &bad);

    cr_assert_eq(error, MEMORY_ERROR_BUFFER_OVERFLOW);
    cr_assert_eq(bad, &shadow[1]);

    my_asan.real_free(raw);
}

Test(poison, partial_granule_with_offset)
{
    void *raw = create_test_ptr();
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512 + 2;
    uint8_t *shadow = get_shadow(ptr);
    uint8_t *bad = NULL;

    shadow[0] = 0x04;

    enum memory_error valid_error = poison_is_poisoned(ptr, 2, &bad);

    cr_assert_eq(valid_error, MEMORY_ERROR_NONE);
    cr_assert_null(bad);

    enum memory_error invalid_error = poison_is_poisoned(ptr, 3, &bad);

    cr_assert_eq(invalid_error, MEMORY_ERROR_BUFFER_OVERFLOW);
    cr_assert_eq(bad, &shadow[0]);

    my_asan.real_free(raw);
}
