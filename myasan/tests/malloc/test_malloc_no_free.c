#include <stdlib.h>

int main(void)
{
    int *ptr = malloc(sizeof(int));

    if (!ptr)
        return 1;

    return 0;
}
