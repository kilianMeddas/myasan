#ifndef POISON_H
#define POISON_H

#include <stddef.h>

#include "hash_map.h"
#include "myasan.h"

void poison_shadow_malloc(void *ptr, size_t size);
void poison_shadow_free(void *ptr, size_t size);

#endif // POISON_H
