#include <stdlib.h>

#include "../../include/poison.h"
#include "../test_suite.h"

int main(void)
{
    char *ptr = malloc(16);

    if (!ptr)
        return 1;

    check_ptr_validity(ptr, 16);

    free(ptr);

    return 0;
}
