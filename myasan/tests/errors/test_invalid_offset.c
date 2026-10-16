#include <stdlib.h>

int main(void)
{
    char *ptr = malloc(32);

    if (!ptr)
        return 10;

    free(ptr + 1);

    return 0;
}
