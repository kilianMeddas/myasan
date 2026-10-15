
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdint.h>
#include <stdlib.h>

#include "../../include/poison.h"
#include "../test_suite.h"

static uint8_t *get_shadow(void *ptr)
{
    uintptr_t address = (uintptr_t)ptr;
    return (uint8_t *)(SHADOW_OFFSET + (address >> 3));
}

Test(check_ptr_validity, valid_access)
{
    void *raw = my_asan.real_malloc(1024);
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    get_shadow(ptr)[0] = 0x00;

    check_ptr_validity(ptr, 4);

    my_asan.real_free(raw);
}

Test(check_ptr_validity, buffer_overflow, .exit_code = 3)
{
    void *raw = my_asan.real_malloc(1024);
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    get_shadow(ptr)[0] = 0xFA;

    check_ptr_validity(ptr, 1);
}

Test(check_ptr_validity, use_after_free, .exit_code = 3)
{
    void *raw = my_asan.real_malloc(1024);
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    get_shadow(ptr)[0] = 0xFD;

    check_ptr_validity(ptr, 1);
}

Test(check_ptr_validity, partial_granule_overflow, .exit_code = 3)
{
    void *raw = my_asan.real_malloc(1024);
    cr_assert_not_null(raw);

    void *ptr = (uint8_t *)raw + 512;
    get_shadow(ptr)[0] = 0x04;

    check_ptr_validity(ptr, 5);
}
