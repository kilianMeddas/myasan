#include <stdlib.h>

int main(void)
{
    char *ptr = malloc(16);

    if (!ptr)
        return 10;

    char *alias = ptr;

    free(ptr);
    free(alias);

    return 0;
}
