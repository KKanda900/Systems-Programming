typedef struct MetaData
{
    size_t size;
    unsigned short free;
    struct MetaData * next;
} MyBlock;

void *mymalloc(size_t size, const char *file, int line);
void myfree(void *p, const char *file, int line);

#define DEBUG 1

#define MEMORY_SIZE 4096
#define BLOCK_SIZE sizeof(MyBlock)

#define malloc(size) mymalloc(size, __FILE__, __LINE__)
#define free(p) myfree(p, __FILE__, __LINE__) 
