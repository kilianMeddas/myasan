#include <stdint.h>
#include <stdlib.h>

#include "../../include/myasan.h"

int main(void)
{
    char *ptr = malloc(16);

    if (!ptr)
        return 1;

    uintptr_t address = (uintptr_t)ptr;
    uint8_t *shadow = (uint8_t *)(SHADOW_OFFSET + (address >> 3));

    if (shadow[0] != 0x00 || shadow[1] != 0x00)
    {
        free(ptr);
        return 2;
    }

    if (shadow[-1] != 0xFA || shadow[2] != 0xFA)
    {
        free(ptr);
        return 3;
    }

    free(ptr);

    return 0;
}
