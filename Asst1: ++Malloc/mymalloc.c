#include <stdio.h>
#include "mymalloc.h"

static char MemoryBlock[MEMORY_SIZE]; //Memory Size is 4096

// Returns a free block with enough size, or the last block
MyBlock *next(MyBlock *block, size_t size)
{
    MyBlock *ptr = block;

    while (ptr->next != NULL)
    {
        if (ptr->free != 0 && ptr->size >= size) break;
        ptr = ptr->next;
    }

    return ptr;
}

// Writes size info into ptr, and prepare the free block after if possible
void fitNextBlock(MyBlock *ptr, size_t size)
{
    ptr->size = size;
    ptr->free = 0;
    if (ptr->next == NULL && (void*)ptr + size + BLOCK_SIZE <= (void*)MemoryBlock + MEMORY_SIZE)
    { // If this is the last free block
        MyBlock *newNode = (void*)((void*)ptr + size + BLOCK_SIZE);
        newNode->size = (size_t)((void*)MemoryBlock + MEMORY_SIZE - (void*)newNode - BLOCK_SIZE);
        newNode->free = 1;
        ptr->next = newNode;
    }
    else if (ptr->next != NULL && (void*)ptr + size + BLOCK_SIZE*2 <= (void*)ptr->next)
    { // If there's at least space for metadata in between
        MyBlock *newNode = (void*)((void*)ptr + size + BLOCK_SIZE);
        newNode->size = (void*)ptr->next - (void*)ptr - size - BLOCK_SIZE*2;
        newNode->free = 1;
        newNode->next = ptr->next;
        ptr->next = newNode;
    }
}

void *mymalloc(size_t size, const char *file, int line)
{

    MyBlock *front = (MyBlock *)MemoryBlock;

    if (size > MEMORY_SIZE - BLOCK_SIZE)
    {
        DEBUG;
        fprintf(stderr, "requested size for allocation is too large %s, %d\n", file, line);
        return NULL;
    }

    // front won't be null so check if the size of the front is allocated or not 

    if (front->size == 0 && front->free == 0)
    {
        DEBUG;
        front->size = MEMORY_SIZE - BLOCK_SIZE;
        front->free = 1;
        front->next = NULL;
    }

    // Let's find the next node to fill
    MyBlock *ptr = front;
    ptr = next(ptr, size);
    
    if (ptr->size < size || ptr->free != 1)
    { // This is the last block, but still cannot allocate
        fprintf(stderr, "No More Space %s %d\n", file, line);
        return NULL;
    }
    else
    {
        fitNextBlock(ptr, size);
        return ptr;
    }
}

void deleteBlock(MyBlock *currBlock)
{
    MyBlock *ptr = (MyBlock *)MemoryBlock;
    MyBlock *prev = NULL;
    while (ptr->next != NULL)
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

void cleanFragments()
{
    MyBlock* ptr = (MyBlock*)MemoryBlock;
    MyBlock* start = NULL;
    MyBlock* end = NULL;
    while (ptr != NULL)
    {
        if (ptr->free)
        {
            if (start == NULL) start = ptr;
            else end = ptr;
        }
        else
        {
            if (start != NULL && end != NULL)
            {
                start->size = (size_t)((void*)ptr - (void*)start - BLOCK_SIZE);
                start->next = ptr;
            }
            start = NULL;
            end = NULL;
        }
        ptr = ptr->next;
    }
    if (start != NULL && end != NULL)
    { // It's free all the way till the end
        start->next = NULL;
        start->size = (size_t)((void*)MemoryBlock + MEMORY_SIZE - BLOCK_SIZE);
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

    if (mem->free == 1)
    {
        fprintf(stderr, "Memory is already freed %s %d\n", file, line);
    }

    void *start = (void *)MemoryBlock;
    void *end = (void *)(MemoryBlock + MEMORY_SIZE);

    // Checks if p is within range of the MemoryBlock
    if (p > start || p <= end)
    {
        mem->free = 0; 
        --mem;
        deleteBlock(mem);
        /* printf("freed\n"); */
        cleanFragments();
    }
    else if (p < start || p > end)
    {
        fprintf(stderr, "This pointer does not exist in memory %s %d\n", file, line);
    }
}
