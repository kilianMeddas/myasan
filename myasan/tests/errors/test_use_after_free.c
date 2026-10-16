#include <stdlib.h>

#include "../../include/poison.h"

int main(void)
{
    char *ptr = malloc(16);

    if (!ptr)
        return 10;

    free(ptr);

    check_ptr_validity(ptr, 8);

    return 0;
}
