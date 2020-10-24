#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main(int argc, char **argv)
{
    char *ptr = (char *)malloc(4);
    printf("%p\n", ptr);
    free(ptr);
    return 0;
}