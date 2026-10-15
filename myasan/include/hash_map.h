#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>
#include <stddef.h>

#define HASHMAP_ARRAY_SIZE 250
extern struct asan my_asan;

struct allocation
{
    size_t size;
    bool freed;
};

struct bucket
{
    void *ptr;
    struct allocation allocation;
    struct bucket *next;
};

struct hash_map
{
    struct bucket **data;
    size_t size;
};

struct hash_map *hash_map_init(size_t size);
bool hash_map_insert(struct hash_map *hm, void *ptr, size_t size);
struct bucket *hash_map_get(const struct hash_map *hm, const void *ptr);
bool hash_map_mark_freed(struct hash_map *hm, const void *ptr);
void hash_map_free(struct hash_map *hm);

#endif /* ! HASH_MAP_H */
