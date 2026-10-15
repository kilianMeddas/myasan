
#include "../include/poison.h"

#include <stdint.h>

#include "../include/myasan.h"

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
    uintptr_t ptr_addr;
    uintptr_t shadow_addr;
    uint8_t *shadow_ptr;
    size_t offset;
    size_t shadow_size;

    ptr_addr = (uintptr_t)ptr;
    shadow_addr = SHADOW_OFFSET + (ptr_addr >> 3);
    shadow_ptr = (uint8_t *)shadow_addr;

    offset = ptr_addr & 7;
    shadow_size = (offset + access_size) / 8;
    if ((offset + access_size) % 8 != 0)
        shadow_size++;

    for (size_t i = 0; i < shadow_size; i++)
    {
        uint8_t value = shadow_ptr[i];

        if (value == 0x00)
            continue;

        *bad_ptr = &shadow_ptr[i];

        if (value == 0xFA)
            return MEMORY_ERROR_BUFFER_OVERFLOW;

        if (value == 0xFD)
            return MEMORY_ERROR_USE_AFTER_FREE;

        if (value >= 0x01 && value <= 0x07)
        {
            size_t access_end = offset + access_size;
            size_t granule_end = i * 8 + value;

            if (access_end > granule_end)
                return MEMORY_ERROR_BUFFER_OVERFLOW;

            continue;
        }

        return MEMORY_ERROR_INVALID_PTR;
    }

    return MEMORY_ERROR_NONE;
}
