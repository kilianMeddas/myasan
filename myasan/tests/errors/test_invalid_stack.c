#include <stdlib.h>

int main(void)
{
    int value = 42;
    void (*my_free)(void *) = free;

    my_free(&value);

    return 0;
}
