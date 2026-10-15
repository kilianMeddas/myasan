#include <stdlib.h>

int main(void)
{
    int *ptr = malloc(sizeof(int));

    if (ptr == NULL)
        return 1;

    *ptr = 42;
    free(ptr);

    return 0;
}
