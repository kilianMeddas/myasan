#include <stdlib.h>

#include "../test_suite.h"

int main(void)
{
    void *ptr1;
    void *ptr2;
    void *ptr3;

    if (my_asan.free_count != 0)
        return 1;

    ptr1 = malloc(10);
    ptr2 = malloc(20);
    ptr3 = malloc(30);

    if (ptr1 == NULL || ptr2 == NULL || ptr3 == NULL)
    {
        free(ptr1);
        free(ptr2);
        free(ptr3);
        return 2;
    }

    if (my_asan.free_count != 0)
    {
        free(ptr1);
        free(ptr2);
        free(ptr3);
        return 3;
    }

    free(ptr1);
    if (my_asan.free_count != 1)
    {
        free(ptr2);
        free(ptr3);
        return 4;
    }

    free(ptr2);
    if (my_asan.free_count != 2)
    {
        free(ptr3);
        return 5;
    }

    free(ptr3);
    if (my_asan.free_count != 3)
        return 6;

    return 0;
}
