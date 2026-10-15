#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>

#include "myasan.h"

extern struct asan my_asan;

void print_memory_error(enum memory_error error, void *ptr);
void print_backtrace(void);
void report_memory_error(enum memory_error error, void *ptr);

void print_leak(void);

#endif // PRINT_H
