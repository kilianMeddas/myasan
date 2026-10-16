#include <stdlib.h>

#include "../../include/poison.h"

int main(void)
{
    char *ptr = malloc(8);

    if (!ptr)
        return 10;

    check_ptr_validity(ptr, 16);

    free(ptr);
    return 0;
}
