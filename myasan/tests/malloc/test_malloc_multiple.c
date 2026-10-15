#include <stdlib.h>

int main(void)
{
    int *ptr = malloc(sizeof(int));

    if (ptr == NULL)
        return 1;

    char *ptr2 = malloc(sizeof(char));

    if (!ptr2)
    {
        free(ptr);
        return 1;
    }

    free(ptr);
    free(ptr2);

    return 0;
}
