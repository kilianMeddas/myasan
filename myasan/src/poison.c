
#include "../include/poison.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/myasan.h"
#include "../include/print.h"

void poison_shadow_malloc(void *ptr, size_t size)
{
    size_t full_granules = size / 8;
    size_t remainder = size % 8;
    size_t shadow_size = full_granules + (remainder != 0);
    size_t redzone_size = REDZONE_SIZE / 8;

    uintptr_t ptr_addr;
    uintptr_t shadow_addr;
    unsigned char *shadow_ptr;
    unsigned char *left_redzone;
    unsigned char *right_redzone;

    ptr_addr = (uintptr_t)ptr;
    shadow_addr = SHADOW_OFFSET + (ptr_addr >> 3);
    shadow_ptr = (unsigned char *)shadow_addr;

    left_redzone = shadow_ptr - redzone_size;
    right_redzone = shadow_ptr + shadow_size;

    for (size_t i = 0; i < full_granules; i++)
    {
        shadow_ptr[i] = 0x00;
    }

    for (size_t i = 0; i < redzone_size; i++)
    {
        left_redzone[i] = 0xFA;
        right_redzone[i] = 0xFA;
    }

    if (remainder != 0)
    {
        shadow_ptr[full_granules] = (unsigned char)remainder;
    }
}

void poison_shadow_free(void *ptr, size_t size)
{
    size_t full_granules = size / 8;
    size_t remainder = size % 8;
    size_t shadow_size = full_granules + (remainder != 0);

    uintptr_t ptr_addr;
    uintptr_t shadow_addr;
    unsigned char *shadow_ptr;

    ptr_addr = (uintptr_t)ptr;
    shadow_addr = SHADOW_OFFSET + (ptr_addr >> 3);
    shadow_ptr = (unsigned char *)shadow_addr;

    for (size_t i = 0; i < shadow_size; i++)
    {
        shadow_ptr[i] = 0xFD;
    }
}

enum memory_error poison_is_poisoned(void *ptr, size_t access_size,
                                     uint8_t **bad_ptr)
{
    if (access_size == 0)
        return MEMORY_ERROR_NONE;
    uintptr_t ptr_addr;
    uintptr_t shadow_addr;
    uint8_t *shadow_ptr;
    size_t offset;
    size_t shadow_size;
    size_t remaining;
    size_t current_offset;

    assert(ptr != NULL);

    ptr_addr = (uintptr_t)ptr;
    shadow_addr = SHADOW_OFFSET + (ptr_addr >> 3);
    shadow_ptr = (uint8_t *)shadow_addr;

    offset = ptr_addr & 7;
    shadow_size = (offset + access_size) / 8;
    if ((offset + access_size) % 8 != 0)
        shadow_size++;

    remaining = access_size;
    current_offset = offset;

    for (size_t i = 0; i < shadow_size; i++)
    {
        uint8_t value = shadow_ptr[i];
        size_t bytes = 8 - current_offset;

        if (bytes > remaining)
            bytes = remaining;

        if (value == 0x00)
        {
            /* Granule entirely accessible. */
        }
        else if (value == 0xFA)
        {
            *bad_ptr = &shadow_ptr[i];
            return MEMORY_ERROR_BUFFER_OVERFLOW;
        }
        else if (value == 0xFD)
        {
            *bad_ptr = &shadow_ptr[i];
            return MEMORY_ERROR_USE_AFTER_FREE;
        }
        else if (value >= 0x01 && value <= 0x07)
        {
            if (current_offset + bytes > value)
            {
                *bad_ptr = &shadow_ptr[i];
                return MEMORY_ERROR_BUFFER_OVERFLOW;
            }
        }
        else
        {
            *bad_ptr = &shadow_ptr[i];
            return MEMORY_ERROR_INVALID_PTR;
        }

        remaining -= bytes;
        current_offset = 0;
    }

    return MEMORY_ERROR_NONE;
}

void poison_print_shadow(void *addr, void *shadow_addr)
{
    uintptr_t bad_addr = (uintptr_t)shadow_addr;
    uintptr_t shadow_start;
    uintptr_t line_start;
    uint8_t *shadow;
    int bad_line;

    shadow_start = bad_addr & ~(uintptr_t)15;
    line_start = shadow_start - 4 * 16;
    shadow = (uint8_t *)line_start;
    bad_line = (int)((bad_addr - line_start) / 16);

    for (int i = 0; i < 9; i++)
    {
        uintptr_t current = line_start + (uintptr_t)i * 16;

        if (i == bad_line)
            fprintf(stderr, "=>%p:\t", (void *)current);
        else
            fprintf(stderr, " %p:\t", (void *)current);

        for (int j = 0; j < 16; j++)
        {
            uintptr_t current_byte = current + (uintptr_t)j;

            if (current_byte == bad_addr)
                fprintf(stderr, "[%02x]", shadow[i * 16 + j]);
            else
                fprintf(stderr, " %02x ", shadow[i * 16 + j]);

            if (j != 15)
                fprintf(stderr, " ");
        }

        fprintf(stderr, "\n");
    }

    (void)addr;
}

void check_ptr_validity(void *ptr, size_t access_size)
{
    uint8_t *bad = NULL;
    enum memory_error error;

    assert(ptr != NULL);

    error = poison_is_poisoned(ptr, access_size, &bad);

    if (error == MEMORY_ERROR_NONE)
        return;

    my_asan.error = error;
    print_memory_error(error, ptr);
    poison_print_shadow(ptr, bad);
    print_backtrace();
    exit(error);
}
