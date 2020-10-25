typedef struct MetaData
{
    size_t size;
    unsigned short free;
    struct MetaData * next;
} MyBlock;

void *mymalloc(size_t size, const char *file, int line);
void myfree(void *p, const char *file, int line);
void clean();

#define DEBUG 0

#define malloc(size) mymalloc(size, __FILE__, __LINE__)
#define free(p) myfree(p, __FILE__, __LINE__) 

