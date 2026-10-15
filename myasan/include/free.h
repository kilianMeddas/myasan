#ifndef FREE_H
#define FREE_H

#include "myasan.h"

extern struct asan my_asan;

void free(void *ptr);

#endif // FREE_H
