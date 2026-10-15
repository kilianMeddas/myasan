#include "../include/free.h"

#include "../include/hash_map.h"

void free(void *ptr)
{
    if (!ptr)
        return;
    struct bucket *bucket = hash_map_get(my_asan.hm, ptr);

    if (!bucket)
    {
        my_asan.error = MEMORY_ERROR_INVALID_PTR;
        return;
    }

    if (bucket->allocation.freed)
    {
        my_asan.error = MEMORY_ERROR_DOUBLE_FREE;
        return;
    }

    hash_map_mark_freed(my_asan.hm, ptr);
    my_asan.free_count++;
    my_asan.real_free(ptr);
}
