#include "../../include/hash_map.h"

static int test_init(void)
{
    struct hash_map *hm = hash_map_init(10);

    if (!hm)
        return 1;

    if (hm->size != 10)
    {
        hash_map_free(hm);
        return 1;
    }

    hash_map_free(hm);
    return 0;
}

static int test_init_zero(void)
{
    if (hash_map_init(0) != NULL)
        return 1;

    return 0;
}

static int test_get_invalid(void)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);

    if (!hm)
        return 1;

    int failed = hash_map_get(NULL, &value) != NULL
        || hash_map_get(hm, NULL) != NULL || hash_map_get(hm, &value) != NULL;

    hash_map_free(hm);
    return failed;
}

static int test_insert_invalid(void)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);

    if (!hm)
        return 1;

    int failed = hash_map_insert(NULL, &value, sizeof(value))
        || hash_map_insert(hm, NULL, sizeof(value));

    hash_map_free(hm);
    return failed;
}

static int test_insert_and_get(void)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);

    if (!hm)
        return 1;

    if (!hash_map_insert(hm, &value, sizeof(value)))
    {
        hash_map_free(hm);
        return 1;
    }

    struct bucket *bucket = hash_map_get(hm, &value);

    int failed = !bucket || bucket->allocation.size != sizeof(value)
        || bucket->allocation.freed;

    hash_map_free(hm);
    return failed;
}

static int test_insert_same_pointer(void)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);

    if (!hm)
        return 1;

    if (!hash_map_insert(hm, &value, sizeof(value))
        || !hash_map_insert(hm, &value, 100))
    {
        hash_map_free(hm);
        return 1;
    }

    struct bucket *bucket = hash_map_get(hm, &value);

    int failed = !bucket || bucket->allocation.size != 100;

    hash_map_free(hm);
    return failed;
}

static int test_mark_freed(void)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);

    if (!hm)
        return 1;

    if (hash_map_mark_freed(NULL, &value) || hash_map_mark_freed(hm, NULL)
        || hash_map_mark_freed(hm, &value))
    {
        hash_map_free(hm);
        return 1;
    }

    if (!hash_map_insert(hm, &value, sizeof(value)))
    {
        hash_map_free(hm);
        return 1;
    }

    int failed =
        !hash_map_mark_freed(hm, &value) || hash_map_mark_freed(hm, &value);

    hash_map_free(hm);
    return failed;
}

static int test_collision(void)
{
    int values[100];
    struct hash_map *hm = hash_map_init(7);

    if (!hm)
        return 1;

    for (size_t i = 0; i < 100; i++)
    {
        values[i] = (int)i;

        if (!hash_map_insert(hm, &values[i], sizeof(values[i])))
        {
            hash_map_free(hm);
            return 1;
        }
    }

    for (size_t i = 0; i < 100; i++)
    {
        if (!hash_map_get(hm, &values[i]))
        {
            hash_map_free(hm);
            return 1;
        }
    }

    hash_map_free(hm);
    return 0;
}

static int test_free_map(void)
{
    int value = 42;
    struct hash_map *hm = hash_map_init(10);

    if (!hm)
        return 1;

    if (!hash_map_insert(hm, &value, sizeof(value)))
    {
        hash_map_free(hm);
        return 1;
    }

    hash_map_free(hm);
    hash_map_free(NULL);

    return 0;
}

int main(void)
{
    if (test_init())
        return 1;

    if (test_init_zero())
        return 2;

    if (test_get_invalid())
        return 3;

    if (test_insert_invalid())
        return 4;

    if (test_insert_and_get())
        return 5;

    if (test_insert_same_pointer())
        return 6;

    if (test_mark_freed())
        return 7;

    if (test_collision())
        return 8;

    if (test_free_map())
        return 9;

    return 0;
}
