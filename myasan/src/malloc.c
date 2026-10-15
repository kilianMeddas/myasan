#include "../include/malloc.h"

#include "../include/hash_map.h"

void *malloc(size_t size)
{
    void *ptr = my_asan.real_malloc(size);
    if (!ptr)
        return NULL;

    bool succeed = hash_map_insert(my_asan.hm, ptr, size);

    if (!succeed)
    {
        my_asan.real_free(ptr);
        return NULL;
    }
    my_asan.allocation_count += 1;

    return ptr;
}
