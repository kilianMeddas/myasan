#include <stdint.h>

#include "../../include/poison.h"
#include "../test_suite.h"

int main(void)
{
    void *raw = my_asan.real_malloc(1024);

    if (!raw)
        return 1;

    void *ptr = (uint8_t *)raw + 512 + 2;
    uint8_t *shadow = (uint8_t *)(SHADOW_OFFSET + ((uintptr_t)ptr >> 3));
    uint8_t *bad = NULL;

    shadow[0] = 0x04;

    enum memory_error valid_error = poison_is_poisoned(ptr, 2, &bad);

    if (valid_error != MEMORY_ERROR_NONE || bad != NULL)
    {
        my_asan.real_free(raw);
        return 2;
    }

    enum memory_error invalid_error = poison_is_poisoned(ptr, 3, &bad);

    my_asan.real_free(raw);

    if (invalid_error != MEMORY_ERROR_BUFFER_OVERFLOW)
        return 3;

    if (bad != &shadow[0])
        return 4;

    return 0;
}
