#ifndef MALLOC_H
#define MALLOC_H

#include "myasan.h"

extern struct asan my_asan;

void *malloc(size_t size);

#endif // MALLOC_H
