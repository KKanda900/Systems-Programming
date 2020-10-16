#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

static char MemoryBlock[MEMORY_SIZE]; //Memory Size is 4096

MyBlock *front = (MyBlock*) MemoryBlock; // Returns the start of the Block

// find the next available block
MyBlock *next(MyBlock *front)
{
    MyBlock *ptr = front;
    MyBlock *prev = NULL;

    while (ptr != null)
    {
        if((ptr->next->free == 0) {
            break;
        }
        prev = ptr;
        ptr = ptr->next;
    }

    return ptr;
}

int main(int argc, char **argv)
{
    printf("SomethingElse\n");
    return 0;
}