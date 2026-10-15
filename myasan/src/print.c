#include "../include/print.h"

#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/hash_map.h"

static const char *get_memory_error_name(enum memory_error error)
{
    switch (error)
    {
    case MEMORY_ERROR_INVALID_PTR:
        return "invalid-ptr";
    case MEMORY_ERROR_USE_AFTER_FREE:
        return "heap-use-after-free";
    case MEMORY_ERROR_BUFFER_OVERFLOW:
        return "heap-buffer-overflow";
    case MEMORY_ERROR_DOUBLE_FREE:
        return "double-free";
    case MEMORY_ERROR_MEMCPY_PARAM_OVERLAP:
        return "memcpy-param-overlap";
    default:
        return "unknown";
    }
}

static int write_stderr(const char *buffer, size_t len)
{
    size_t written = 0;

    while (written < len)
    {
        ssize_t ret = write(STDERR_FILENO, buffer + written, len - written);
        if (ret <= 0)
            return -1;
        written += (size_t)ret;
    }
    return 0;
}

void print_memory_error(enum memory_error error, void *ptr)
{
    char buffer[256];
    int len;

    len = snprintf(
        buffer, sizeof(buffer),
        "=================================================================\n"
        "==%04u== ERROR: AddressSanitizer: %s on address %p\n",
        error, get_memory_error_name(error), ptr);
    if (len < 0 || (size_t)len >= sizeof(buffer))
        return;

    write_stderr(buffer, (size_t)len);
}

void print_backtrace(void)
{
    void *frames[64];
    int count = backtrace(frames, 64);

    if (count <= 2)
        return;

    char **symbols = backtrace_symbols(frames, count);
    if (!symbols)
        return;

    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer), "\t#0 %s\n", symbols[2]);

    if (len > 0 && (size_t)len < sizeof(buffer))
        write_stderr(buffer, (size_t)len);

    free(symbols);
}

void report_memory_error(enum memory_error error, void *ptr)
{
    print_memory_error(error, ptr);
    print_backtrace();
    exit(error);
}

void print_leak(void)
{
    char buffer[1024];
    int len;
    size_t total_size = 0;
    size_t total_allocations = 0;

    if (my_asan.hm == NULL)
        return;

    len = snprintf(
        buffer, sizeof(buffer),
        "=================================================================\n"
        "LeakSanitizer: Detected memory leaks\n");

    if (len < 0 || (size_t)len >= sizeof(buffer))
        return;

    write_stderr(buffer, (size_t)len);

    for (size_t i = 0; i < my_asan.hm->size; i++)
    {
        struct bucket *current = my_asan.hm->data[i];

        while (current != NULL)
        {
            if (!current->allocation.freed)
            {
                len = snprintf(buffer, sizeof(buffer),
                               "\t-> Leak at %p (size %zu bytes)\n",
                               current->ptr, current->allocation.size);

                if (len >= 0 && (size_t)len < sizeof(buffer))
                    write_stderr(buffer, (size_t)len);

                total_size += current->allocation.size;
                total_allocations++;
            }
            current = current->next;
        }
    }

    len = snprintf(
        buffer, sizeof(buffer),
        "SUMMARY: LeakSanitizer: %zu byte(s) leaked in %zu allocation(s).\n",
        total_size, total_allocations);

    if (len >= 0 && (size_t)len < sizeof(buffer))
        write_stderr(buffer, (size_t)len);
}
