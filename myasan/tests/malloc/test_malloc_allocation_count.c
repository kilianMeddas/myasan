
#include <stdlib.h>

#include "../../include/myasan.h"
#include "../test_suite.h"

int main(void)
{
    void *ptr1;
    void *ptr2;
    void *ptr3;

    if (my_asan.allocation_count != 0)
        return 1;

    ptr1 = malloc(10);
    if (ptr1 == NULL || my_asan.allocation_count != 1)
        return 2;

    ptr2 = malloc(20);
    if (ptr2 == NULL || my_asan.allocation_count != 2)
    {
        free(ptr1);
        if (ptr2 != NULL)
            free(ptr2);
        return 3;
    }

    ptr3 = malloc(30);
    if (ptr3 == NULL || my_asan.allocation_count != 3)
    {
        free(ptr1);
        free(ptr2);
        if (ptr3 != NULL)
            free(ptr3);
        return 4;
    }

    free(ptr1);
    free(ptr2);
    free(ptr3);

    if (my_asan.allocation_count != 3)
        return 5;

    if (my_asan.free_count != 3)
        return 6;

    return 0;
}
