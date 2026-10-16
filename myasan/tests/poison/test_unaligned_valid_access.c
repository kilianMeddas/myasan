#include <stdlib.h>

#include "../../include/poison.h"

int main(void)
{
    char *ptr = malloc(9);

    if (ptr == NULL)
        return 1;

    check_ptr_validity(ptr + 7, 2);

    free(ptr);
    return 0;
}
