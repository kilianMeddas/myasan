#ifndef POISON_H
#define POISON_H

#include <stddef.h>
#include <stdint.h>

#include "hash_map.h"
#include "myasan.h"

enum memory_error poison_is_poisoned(void *ptr, size_t access_size,
                                     uint8_t **bad_ptr);

void check_ptr_validity(void *ptr, size_t access_size);
void poison_shadow_malloc(void *ptr, size_t size);
void poison_shadow_free(void *ptr, size_t size);

#endif // POISON_H
