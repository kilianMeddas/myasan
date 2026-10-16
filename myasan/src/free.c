#include "../include/free.h"

#include "../include/hash_map.h"
#include "../include/poison.h"
#include "../include/print.h"

void free(void *ptr)
{
    if (!ptr)
        return;

    struct bucket *bucket = hash_map_get(my_asan.hm, ptr);

    if (!bucket)
        report_memory_error(MEMORY_ERROR_INVALID_PTR, ptr);

    if (bucket->allocation.freed)
        report_memory_error(MEMORY_ERROR_DOUBLE_FREE, ptr);

    size_t size = bucket->allocation.size;

    poison_shadow_free(ptr, size);
    hash_map_mark_freed(my_asan.hm, ptr);

    my_asan.free_count++;

    void *base = (char *)ptr - REDZONE_SIZE;
    my_asan.real_free(base);
}
