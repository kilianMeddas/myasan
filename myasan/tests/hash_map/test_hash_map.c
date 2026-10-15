#include <criterion/criterion.h>

#include "../../include/hash_map.h"

Test(hash_map, init)
{
    struct hash_map *hm = hash_map_init(10);
    cr_assert_not_null(hm);
    cr_assert_eq(hm->size, 10);
    hash_map_free(hm);
}

Test(hash_map, init_zero)
{
    cr_assert_null(hash_map_init(0));
}

Test(hash_map, get_invalid)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);
    cr_assert_null(hash_map_get(NULL, &value));
    cr_assert_null(hash_map_get(hm, NULL));
    cr_assert_null(hash_map_get(hm, &value));
    hash_map_free(hm);
}

Test(hash_map, insert_invalid)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);
    cr_assert_not(hash_map_insert(NULL, &value, sizeof(value)));
    cr_assert_not(hash_map_insert(hm, NULL, sizeof(value)));
    hash_map_free(hm);
}

Test(hash_map, insert_and_get)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);
    cr_assert(hash_map_insert(hm, &value, sizeof(value)));
    struct bucket *bucket = hash_map_get(hm, &value);
    cr_assert_not_null(bucket);
    cr_assert_eq(bucket->allocation.size, sizeof(value));
    cr_assert_not(bucket->allocation.freed);
    hash_map_free(hm);
}

Test(hash_map, insert_same_pointer)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);
    cr_assert(hash_map_insert(hm, &value, sizeof(value)));
    cr_assert(hash_map_insert(hm, &value, 100));
    cr_assert_eq(hash_map_get(hm, &value)->allocation.size, 100);
    hash_map_free(hm);
}

Test(hash_map, mark_freed)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);
    cr_assert_not(hash_map_mark_freed(NULL, &value));
    cr_assert_not(hash_map_mark_freed(hm, NULL));
    cr_assert_not(hash_map_mark_freed(hm, &value));
    cr_assert(hash_map_insert(hm, &value, sizeof(value)));
    cr_assert(hash_map_mark_freed(hm, &value));
    cr_assert_not(hash_map_mark_freed(hm, &value));
    hash_map_free(hm);
}

Test(hash_map, collision)
{
    int values[100];
    struct hash_map *hm = hash_map_init(7);
    cr_assert_not_null(hm);
    for (size_t i = 0; i < 100; i++)
    {
        values[i] = (int)i;
        cr_assert(hash_map_insert(hm, &values[i], sizeof(values[i])));
    }

    for (size_t i = 0; i < 100; i++)
        cr_assert_not_null(hash_map_get(hm, &values[i]));
    hash_map_free(hm);
}

Test(hash_map, free_map)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);
    cr_assert(hash_map_insert(hm, &value, sizeof(value)));
    hash_map_free(hm);
    hash_map_free(NULL);
}
