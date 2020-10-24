#include <stdio.h>
#include "mymalloc.h"

static char MemoryBlock[MEMORY_SIZE]; //Memory Size is 4096

MyBlock *front = (MyBlock *)MemoryBlock; // Returns the start of the Block

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

    if (size == 0)
    {
        fprintf(stderr, "cannot call malloc() without a request size %s, %d\n", file, line);
        return NULL;
    }

    if (size > MEMORY_SIZE - BLOCK_SIZE)
    {
        fprintf(stderr, "requested size for allocation is too large %s, %d\n", file, line);
        return NULL;
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

    if (size > MEMORY_SIZE + BLOCK_SIZE)
    {
        fprintf(stderr, "Memory size too large %s %d\n", file, line);
        return NULL;
    }
    else if (size <= MEMORY_SIZE + BLOCK_SIZE)
    {
        fitNextBlock(ptr, size);
        returnPTR = (void *)ptr++;
        return returnPTR;
    }
    else
    {
        fprintf(stderr, "No More Space %s %d\n", file, line);
        returnPTR = NULL;
        return returnPTR;
    }

    fprintf(stderr, "Allocated %s %d\n", file, line);
    return returnPTR;
}

void deleteBlock(MyBlock *currBlock)
{
    MyBlock *ptr = (MyBlock *)MemoryBlock;
    MyBlock *prev;
    while (ptr != NULL)
    {
        if (ptr->free == 1 && ptr->next->free == 1)
        {
            ptr->size += (ptr->next->size + BLOCK_SIZE);
            ptr->next = ptr->next->next;
        }
        prev = ptr;
        ptr = ptr->next;
    }
}

void myfree(void *p, const char *file, int line)
{

    MyBlock*mem = (MyBlock*)p;

    if(mem->free == 0){
        fprintf(stderr, "This memory is already freed %s %d\n", file, line);
    }

    void *start = (void *)MemoryBlock;
    void *end = (void *)(MemoryBlock + MEMORY_SIZE);

    // Checks if p is within range of the MemoryBlock
    if (p > start || p <= end)
    {
        mem->free = 0;
        mem--;
        deleteBlock(mem);
        printf("freed\n");
    }
    else if(p < start || p > end) 
    {
        fprintf(stderr, "This pointer does not exist in memory %s %d\n", file, line);
    }
}
