#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

static char MemoryBlock[MEMORY_SIZE]; //Memory Size is 4096

MyBlock *front = (MyBlock *)MemoryBlock; // Returns the start of the Block

/* void initialize(){
    front->size = 0;
    front->free = 0;
    front->next = NULL;
} */

// find the next available block
MyBlock *next(MyBlock *block)
{
    MyBlock *ptr = block;

    while (ptr != NULL)
    {

        if (ptr->free == 0)
        {
            break;
        }

        ptr = ptr->next;
    }

    return ptr;
}

void fitNextBlock(MyBlock *ptr, size_t size)
{
    MyBlock *newNode = (MyBlock *)(MemoryBlock + size + BLOCK_SIZE);
    newNode->size = (ptr->size) - size - BLOCK_SIZE;
    newNode->free = 1;
    newNode->next = NULL;
    ptr->size = size;
    ptr->free = 0;
    ptr->next = newNode;
}

void *mymalloc(size_t size, const char *file, int line)
{

    if(size == 0) 
    {
        fprintf(stderr, "cannot call malloc() without a request size %s, %d\n", file, line);
    }

    if(size > MEMORY_SIZE - BLOCK_SIZE) 
    {
        fprintf(stderr, "requested size for allocation is too large %s, %d\n", file, line);
    }

    void *returnPTR;

    // front is null so we are going to set the metadata chunk
    if (front == NULL)
    {
        front->size = MEMORY_SIZE - BLOCK_SIZE;
        front->free = 1;
        front->next = NULL;
    }

    // Let's find the next node to fill
    MyBlock *ptr = front;
    ptr = next(ptr);

    if (size >= MEMORY_SIZE + BLOCK_SIZE)
    {
        printf("Cannot allocate this memory size");
    }
    else if (size < MEMORY_SIZE + BLOCK_SIZE)
    {
        fitNextBlock(ptr, size);
        returnPTR = (void *)ptr++;
        return returnPTR;
    }
    else
    {
        printf("Not enough memory!\n");
        returnPTR = NULL;
        return returnPTR;
    }

    return ptr;
}
