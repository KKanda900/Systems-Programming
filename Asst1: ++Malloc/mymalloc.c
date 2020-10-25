#include <stdio.h>
#include "mymalloc.h"

#define MEMORY_SIZE 4096
#define BLOCK_SIZE sizeof(MyBlock)

static char MemoryBlock[MEMORY_SIZE]; //Memory Size is 4096

void clean()
{
    for (int i = 0; i < MEMORY_SIZE; ++i) MemoryBlock[i] = 0;
}

int ptrIsInMem(void* ptr)
{
    return ptr >= (void*)MemoryBlock && ptr <= (void*)MemoryBlock + MEMORY_SIZE - BLOCK_SIZE;
}

// Returns a free block with enough size, or the last block
MyBlock *next(MyBlock *front, size_t size)
{
    MyBlock* ptr = front;

    while (ptrIsInMem(ptr) && ptr->next != NULL)
    {
        if (ptr->free == 1 && ptr->size >= size) break;
        ptr = ptr->next;
    }
    return ptr;
}

// Writes size info into ptr, and prepare the free block after if possible
void fitNextBlock(MyBlock *ptr, size_t size)
{
    ptr->size = size;
    ptr->free = 0;
    if (ptr->next == NULL && ptrIsInMem((void*)ptr + size + BLOCK_SIZE))
    { // If this is the last free block
        MyBlock *newNode = (void*)ptr + size + BLOCK_SIZE;
        newNode->size = (size_t)((void*)MemoryBlock + MEMORY_SIZE - (void*)newNode - BLOCK_SIZE);
        newNode->next = NULL;
        newNode->free = 1;
        ptr->next = newNode;
    }
    else if (ptr->next != NULL && (void*)ptr + size + BLOCK_SIZE*2 <= (void*)(ptr->next))
    { // If there's at least space for metadata in between
        MyBlock *newNode = (void*)ptr + size + BLOCK_SIZE;
        newNode->size = (void*)(ptr->next) - (void*)ptr - size - BLOCK_SIZE*2;
        newNode->free = 1;
        newNode->next = ptr->next;
        ptr->next = newNode;
    }
}

void *mymalloc(size_t size, const char *file, int line)
{
    if (DEBUG) printf("\tmalloc(%d)\n", (int)size);
    MyBlock *front = (MyBlock *)MemoryBlock;

    if (size > MEMORY_SIZE - BLOCK_SIZE)
    {
        fprintf(stderr, "requested size for allocation is too large %s, %d\n", file, line);
        return NULL;
    }

    // front won't be null so check if the size of the front is allocated or not 

    if (front->size == 0 && front->free == 0)
    {
        front->size = MEMORY_SIZE - BLOCK_SIZE;
        front->free = 1;
        front->next = NULL;
    }

    // Let's find the next node to fill
    MyBlock *ptr = front;
    ptr = next(ptr, size);
    
    if (!ptrIsInMem(ptr) || ptr->size < size || ptr->free != 1)
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
    currBlock->free = 1;
    if (currBlock->next != NULL)
    {
        currBlock->size = (size_t)((void*)(currBlock->next) - (void*)currBlock - BLOCK_SIZE);
    }
    else
    {
        currBlock->size = (size_t)((void*)MemoryBlock + MEMORY_SIZE - (void*)currBlock - BLOCK_SIZE);
    }
}

void cleanFragments()
{
    MyBlock* ptr = (MyBlock*)MemoryBlock;
    MyBlock* prev = NULL;
    MyBlock* start = NULL;
    MyBlock* end = NULL;
    while (ptrIsInMem(ptr))
    {
        if (ptr->free)
        {
            if (start == NULL) start = ptr;
            else
            {
                end = ptr;
                prev->next = NULL;
            }
        }
        else // Not free starting from here
        {
            if (start != NULL && end != NULL) // end is NULL means start is an isolated free block, no action needed
            {
                start->size = (size_t)((void*)ptr - (void*)start - BLOCK_SIZE);
                start->next = ptr;
            }
            start = NULL;
            end = NULL;
        }
        prev = ptr;
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
    if (DEBUG) printf("\tfree(%p)\n", p);

    MyBlock *mem = (MyBlock *)p;

    // Checks if p is within range of the MemoryBlock
    if (!ptrIsInMem(mem))
    {
        fprintf(stderr, "This pointer does not exist in memory %s %d\n", file, line);
        return;
    }

    if (mem->free != 0)
    {
        fprintf(stderr, "Memory is already freed %s %d\n", file, line);
        return;
    }

    deleteBlock(mem);
    cleanFragments();
}
