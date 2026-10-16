#include <stdint.h>

#include "../../include/poison.h"

int main(void)
{
    void *raw = my_asan.real_malloc(1024);

    if (!raw)
        return 1;

    void *ptr = (uint8_t *)raw + 512;
    uint8_t *shadow = (uint8_t *)(SHADOW_OFFSET + ((uintptr_t)ptr >> 3));
    uint8_t *bad = NULL;

    shadow[0] = 0xFA;

    enum memory_error error = poison_is_poisoned(ptr, 1, &bad);

    my_asan.real_free(raw);

    if (error != MEMORY_ERROR_BUFFER_OVERFLOW)
        return 2;

    if (bad != &shadow[0])
        return 3;

    return 0;
}
