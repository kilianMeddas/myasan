
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
        shadow_ptr[i] = 0x00;

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

void poison_shadow_free(void *ptr, size_t size);
