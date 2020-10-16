typedef struct Block
{
    char metadata;
    int free : 0; //returns 1 if its not free and 0 if its free
    short byteSize : 12;
    struct Block * next : NULL;
} MyBlock;

#define MEMORY_SIZE 4096

#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)