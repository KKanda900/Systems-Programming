typedef struct MetaData
{
    size_t size;
    int free;
    struct MetaData * next;
} MyBlock;

#define MEMORY_SIZE 4096
#define BLOCK_SIZE sizeof(MyBlock)

#define malloc(size) mymalloc(size, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__) 