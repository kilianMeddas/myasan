
#include "../include/hash_map.h"

#include <stdint.h>

#include "../include/myasan.h"

static size_t hash_pointer(const void *ptr, size_t size)
{
    uint64_t key = (uint64_t)(uintptr_t)ptr;

    key = (~key) + (key << 21);
    key ^= key >> 24;
    key = (key + (key << 3)) + (key << 8);
    key ^= key >> 14;
    key = (key + (key << 2)) + (key << 4);
    key ^= key >> 28;
    key += key << 31;

    return (size_t)(key % size);
}

struct hash_map *hash_map_init(size_t size)
{
    if (size == 0 || !my_asan.real_malloc || !my_asan.real_free)
        return NULL;

    struct hash_map *hm = my_asan.real_malloc(sizeof(*hm));
    if (!hm)
        return NULL;

    hm->data = my_asan.real_malloc(size * sizeof(*hm->data));
    if (!hm->data)
    {
        my_asan.real_free(hm);
        return NULL;
    }

    hm->size = size;
    for (size_t i = 0; i < size; i++)
        hm->data[i] = NULL;

    return hm;
}

struct bucket *hash_map_get(const struct hash_map *hm, const void *ptr)
{
    if (!hm || !ptr || hm->size == 0)
        return NULL;

    size_t index = hash_pointer(ptr, hm->size);
    struct bucket *current = hm->data[index];

    while (current)
    {
        if (current->ptr == ptr)
            return current;

        current = current->next;
    }

    return NULL;
}

bool hash_map_insert(struct hash_map *hm, void *ptr, size_t size)
{
    if (!hm || !ptr || hm->size == 0 || !my_asan.real_malloc)
        return false;

    struct bucket *existing = hash_map_get(hm, ptr);
    if (existing)
    {
        existing->allocation.size = size;
        existing->allocation.freed = false;
        return true;
    }

    struct bucket *new = my_asan.real_malloc(sizeof(*new));
    if (!new)
        return false;

    size_t index = hash_pointer(ptr, hm->size);

    new->ptr = ptr;
    new->allocation.size = size;
    new->allocation.freed = false;
    new->next = hm->data[index];
    hm->data[index] = new;

    return true;
}

bool hash_map_mark_freed(struct hash_map *hm, const void *ptr)
{
    struct bucket *bucket = hash_map_get(hm, ptr);

    if (!bucket || bucket->allocation.freed)
        return false;

    bucket->allocation.freed = true;
    return true;
}

void hash_map_free(struct hash_map *hm)
{
    if (!hm || !my_asan.real_free)
        return;

    for (size_t i = 0; i < hm->size; i++)
    {
        struct bucket *current = hm->data[i];

        while (current)
        {
            struct bucket *next = current->next;
            my_asan.real_free(current);
            current = next;
        }
    }

    my_asan.real_free(hm->data);
    my_asan.real_free(hm);
}
