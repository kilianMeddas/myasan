#include "../include/myasan.h"

#include <assert.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/mman.h>

#include "../include/hash_map.h"
#include "../include/print.h"

struct asan my_asan;

void on_start(void)
{
    *(void **)(&my_asan.real_malloc) = dlsym(RTLD_NEXT, "malloc");
    *(void **)(&my_asan.real_calloc) = dlsym(RTLD_NEXT, "calloc");
    *(void **)(&my_asan.real_memcpy) = dlsym(RTLD_NEXT, "memcpy");
    *(void **)(&my_asan.real_free) = dlsym(RTLD_NEXT, "free");
    my_asan.initialized = false;
    my_asan.error = MEMORY_ERROR_NONE;
    my_asan.allocation_count = 0;
    my_asan.free_count = 0;

    my_asan.hm = hash_map_init(100);
    if (my_asan.hm == NULL)
        return;

    my_asan.initialized = true;

    my_asan.shadow_memory =
        mmap((void *)SHADOW_OFFSET, SHADOW_SIZE, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE | MAP_NORESERVE,
             -1, 0);

    assert(my_asan.shadow_memory != MAP_FAILED
           && "shadow memory 16TB allocation failed");
}

void on_quit(void)
{
    if (my_asan.error != MEMORY_ERROR_NONE)
        return;

    if (my_asan.allocation_count != my_asan.free_count)
        print_leak();

    hash_map_free(my_asan.hm);
    my_asan.hm = NULL;
}
