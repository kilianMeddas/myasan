#include "../include/malloc.h"

#include <stdint.h>

#include "../include/hash_map.h"
#include "../include/poison.h"

void *malloc(size_t size)
{
    if (size > SIZE_MAX - 2 * REDZONE_SIZE)
        return NULL;

    size_t total_size = size + 2 * REDZONE_SIZE;
    void *base = my_asan.real_malloc(total_size);

    if (!base)
        return NULL;

    void *ptr = (char *)base + REDZONE_SIZE;

    bool succeed = hash_map_insert(my_asan.hm, ptr, size);
    if (!succeed)
    {
        my_asan.real_free(base);
        return NULL;
    }

    poison_shadow_malloc(ptr, size);
    my_asan.allocation_count++;

    return ptr;
}
