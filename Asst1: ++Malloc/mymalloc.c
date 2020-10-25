#include <stdio.h>
#include "mymalloc.h"

static char MemoryBlock[MEMORY_SIZE]; //Memory Size is 4096

// find the next available block
MyBlock *next(MyBlock *block)
{
    MyBlock *ptr = block;

    while (ptr->next != NULL)
    {
        if (ptr->free != 0)
        {
            break;
        }
        ptr = ptr->next;
    }

    return ptr;
}

void fitNextBlock(MyBlock *ptr, unsigned short size)
{
    MyBlock *newNode = (void *)((void *)ptr + size + BLOCK_SIZE);
    newNode->size = ptr->size - size - BLOCK_SIZE;
    newNode->free = 1;
    newNode->next = ptr->next;
    ptr->size = size;
    ptr->free = 0;
    ptr->next = newNode;
}

void *mymalloc(size_t size, const char *file, int line)
{

    MyBlock *front = (MyBlock *)MemoryBlock;

    if ((unsigned short) size > MEMORY_SIZE - BLOCK_SIZE)
    {
        fprintf(stderr, "requested size for allocation is too large %s, %d\n", file, line);
        return NULL;
    }

    // front won't be null so check if the size of the front is allocated or not

    if (front->size == 0)
    {
        front->size = MEMORY_SIZE - BLOCK_SIZE;
        front->free = 1;
        front->next = NULL;
    }

    // Let's find the next node to fill
    MyBlock *ptr = front;
    ptr = next(ptr);

    void *returnPTR;

    if (ptr->size > TOTAL_SIZE)
    {
        fitNextBlock(ptr, (unsigned short) size);
        returnPTR = ptr++;
        return (void*)returnPTR;
    }
    else if (ptr->size == TOTAL_SIZE)
    {
        ptr->free = 1;
        returnPTR = ptr++;
        return (void*)returnPTR;
    }
    else
    {

        fprintf(stderr, "No More Space %s %d\n", file, line);
        return NULL;
    }
}

void deleteBlock(MyBlock *currBlock)
{
    MyBlock *ptr = currBlock;
    MyBlock *prev = NULL;
    while (ptr != NULL)
    {
        if (ptr->free != 0 && ptr->next->free != 0)
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

    MyBlock *mem = (MyBlock *)p;

    /*  printf("%d\n", mem->free); */

    if (MemoryBlock == NULL)
    {
        fprintf(stderr, "Nothing in the main memory %s %d\n", file, line);
    }

    if (mem->free == 0)
    {
        fprintf(stderr, "Memory is already freed %s %d\n", file, line);
    }

    void *start = (void *)MemoryBlock;
    void *end = (void *)(MemoryBlock + MEMORY_SIZE);

    // Checks if p is within range of the MemoryBlock
    if (p > start || p <= end)
    {
        mem->free = 0;
        mem--;
        deleteBlock(mem);
        /* printf("freed\n"); */
    }
    else if (p < start || p > end)
    {
        fprintf(stderr, "This pointer does not exist in memory %s %d\n", file, line);
    }
}
