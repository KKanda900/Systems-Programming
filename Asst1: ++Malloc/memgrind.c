#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

int main(int argc, char**argv) {
    char * ptr = (char*)malloc(sizeof(100));
    printf("%p\n", ptr);
    return 0;
}