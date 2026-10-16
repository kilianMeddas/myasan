#include <stdlib.h>

int main(void)
{
    char *ptr = malloc(16);

    if (!ptr)
        return 10;

    free(ptr);
    free(ptr);

    return 0;
}
