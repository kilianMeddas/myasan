#ifndef MYASAN_H
#define MYASAN_H

#define SHADOW_OFFSET 0x100000000000UL
#define SHADOW_SIZE (1UL << 44)
#define REDZONE_SIZE 512

#include <stdbool.h>
#include <stddef.h>

__attribute__((constructor)) void on_start(void);
__attribute__((destructor)) void on_quit(void);

enum memory_error
{
    MEMORY_ERROR_NONE = 0,
    MEMORY_ERROR_INVALID_PTR,
    MEMORY_ERROR_USE_AFTER_FREE,
    MEMORY_ERROR_BUFFER_OVERFLOW,
    MEMORY_ERROR_DOUBLE_FREE,
    MEMORY_ERROR_MEMCPY_PARAM_OVERLAP
};

struct asan
{
    bool initialized;
    enum memory_error error;
    struct hash_map *hm;
    size_t allocation_count;
    size_t free_count;

    void (*real_free)(void *);
    void *(*real_malloc)(size_t);
    void *(*real_calloc)(size_t, size_t);
    void *(*real_memcpy)(void *, const void *, size_t);

    void *shadow_memory;
};

#endif // MYASAN_H
