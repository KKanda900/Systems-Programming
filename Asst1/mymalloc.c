#include <stdio.h>
#include "mymalloc.h"

// Here are some macros used to access the total amount of memory in MemoryBlock and the block size of the metadata
#define MEMORY_SIZE 4096
#define BLOCK_SIZE sizeof(MyBlock)

// MemoryBlock is a global variable that in this case will simulate the heap
static char MemoryBlock[MEMORY_SIZE]; //Memory Size is 4096

/*
 * The clean function is used in memgrind to reset MemoryBlock between each test case. 
 */

void clean()
{
    for (int i = 0; i < MEMORY_SIZE; ++i)
        MemoryBlock[i] = 0;
}

/*
 * The ptrIsInMem function checks if that pointer provided is within the MemoryBlock
 */

int ptrIsInMem(void *ptr)
{
    return ptr >= (void *)MemoryBlock && ptr <= (void *)MemoryBlock + MEMORY_SIZE - BLOCK_SIZE;
}

/*
 * The next function is used in the mymalloc function that is used to find the next block
 * fill after the front of the MemoryBlock has been initialized. 
 * 
 * The next function takes in two arguments (MyBlock* front and size_t size) and returns a free block with 
 * enough size on the condition: 
 *              ptr->free == 1 and ptr->size >= size
 * and returns the last block if that condition is not satisified and it continues to loop thru the
 * front of the MemoryBlock.
 */

MyBlock *next(MyBlock *front, size_t size)
{
    MyBlock *ptr = front;

    while (ptrIsInMem(ptr) && ptr->next != NULL)
    {
        if (ptr->free == 1 && ptr->size >= size)
            break;
        ptr = ptr->next;
    }
    return ptr;
}

/*
 * The fitNextBlock function is used in mymalloc function that is used to fit a block if the next block to allocate
 * is within the MemoryBlock, the next block is >= size that the user is trying to allocate and if the next block is
 * free.
 * 
 * fitNextBlock takes MyBlock*ptr (metadata) and the requested size for allocation and writes the sizes info into the 
 * pointer and prepares the free block after if possible. 
 */

void fitNextBlock(MyBlock *ptr, size_t size)
{
    ptr->size = size;
    ptr->free = 0;
    if (ptr->next == NULL && ptrIsInMem((void *)ptr + size + BLOCK_SIZE))
    { // If this is the last free block
        MyBlock *newNode = (void *)ptr + size + BLOCK_SIZE;
        newNode->size = (size_t)((void *)MemoryBlock + MEMORY_SIZE - (void *)newNode - BLOCK_SIZE);
        newNode->next = NULL;
        newNode->free = 1;
        ptr->next = newNode;
    }
    else if (ptr->next != NULL && (void *)ptr + size + BLOCK_SIZE * 2 <= (void *)(ptr->next))
    { // If there's at least space for metadata in between
        MyBlock *newNode = (void *)ptr + size + BLOCK_SIZE;
        newNode->size = (void *)(ptr->next) - (void *)ptr - size - BLOCK_SIZE * 2;
        newNode->free = 1;
        newNode->next = ptr->next;
        ptr->next = newNode;
    }
}

/*
 * The mymalloc function takes the requested size for allocation, the file the user is working in and the line number 
 * of the requested allocation and returns a void pointer based on the requested size of allocation. 
 * 
 * Unlike the regular malloc function, mymalloc accounts for more error checking of the requested bytes the user requests like: 
 * 1. if the size is larger than the total bytes in the MemoryBlock - the block size of the metadata
 * 2. Checks if the block that is being allocated is within the MemoryBlock
 * 3. The block that is being allocated is < requested size of allocation
 */

void *mymalloc(size_t size, const char *file, int line)
{
    if (DEBUG)
        printf("\tmalloc(%d)\n", (int)size);

    MyBlock *front = (MyBlock *)MemoryBlock;

    if (size < 0)
    {
        fprintf(stderr, "%s:%d: Error: Size cannot be a negative value\n", file, line);
    }

    if (size > MEMORY_SIZE - BLOCK_SIZE)
    {
        fprintf(stderr, "%s:%d: Error: Requested size for allocation is too large\n", file, line);
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
        fprintf(stderr, "%s:%d: Error: Not enough space to allocate this pointer\n", file, line);
        return NULL;
    }
    else
    {
        fitNextBlock(ptr, size);
        return ptr;
    }
}

/*
 * The findInMem function checks if the pointer provided is allocated by malloc()
 * */

int findInMem(void* p)
{
    MyBlock* ptr = (MyBlock*)MemoryBlock;
    while (ptrIsInMem(ptr))
    {
        if (ptr == p) return 1;
        ptr = ptr->next;
    }
    return 0;
}

/*
 * The deleteBlock function is used in myfree function to delete a block from the MemoryBlock. 
 * 
 * The deleteBlock function takes the current block and first makes it free and checks the condition if the next metadata
 * data isn't null which it would make the current block's size the size of the next one - the current one. Otherwise
 * it would set the current block's size to the MEMORY_SIZE (4096) - the size of the current block - BLOCK_SIZE (sizeof(MyBlock))
 */

void deleteBlock(MyBlock *currBlock)
{
    currBlock->free = 1;
    if (currBlock->next != NULL)
    {
        currBlock->size = (size_t)((void *)(currBlock->next) - (void *)currBlock - BLOCK_SIZE);
    }
    else
    {
        currBlock->size = (size_t)((void *)MemoryBlock + MEMORY_SIZE - (void *)currBlock - BLOCK_SIZE);
    }
}

/*
 * The cleanFragments function is used in myfree anytime after a valid pointer is freed. It will coalesce
 * adjacent free blocks and find the correct size for the coalesced free block.
 */

void cleanFragments()
{
    MyBlock *ptr = (MyBlock *)MemoryBlock;
    MyBlock *prev = NULL;
    MyBlock *start = NULL;
    MyBlock *end = NULL;
    while (ptrIsInMem(ptr))
    {
        if (ptr->free)
        {
            if (start == NULL)
                start = ptr;
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
                start->size = (size_t)((void *)ptr - (void *)start - BLOCK_SIZE);
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
        start->size = (size_t)((void *)MemoryBlock + MEMORY_SIZE - BLOCK_SIZE);
    }
}

/*
 * The myfree function takes the pointer that was passed into it and frees it from the MemoryBlock using deleteBlock function
 * and cleanFragments function. 
 * 
 * It error checks for if the user provides a pointer that isn't in memory and if the pointer requested is already freed. 
 * Providing the file name and line number of the error.  
 */

void myfree(void *p, const char *file, int line)
{
    if (DEBUG)
        printf("\tfree(%p)\n", p);

    MyBlock *mem = (MyBlock *)p;

    // Checks if p is within range of the MemoryBlock
    if (!findInMem(mem))
    {
        fprintf(stderr, "%s:%d: Error: This pointer does not exist in memory\n", file, line);
        return;
    }

    if (mem->free != 0)
    {
        fprintf(stderr, "%s:%d: Error: This pointer is already freed from memory\n", file, line);
        return;
    }

    deleteBlock(mem);
    cleanFragments(); 
}