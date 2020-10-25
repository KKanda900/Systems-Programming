/*
 * This struct type is used to hold metadata in the beginning of a malloc()ed memory block
 *
 * It contains three fields: size_t to record the size of the malloc()ed block after metadata,
 * unsigned short to indicate if this block is free for a new malloc() block,
 * and pointer to another struct Metadata.
 */

typedef struct MetaData
{
    size_t size;
    unsigned short free;
    struct MetaData * next;
} MyBlock;

/*
 * These two are the function prototypes of our customized malloc and free
 */

void *mymalloc(size_t size, const char *file, int line);
void myfree(void *p, const char *file, int line);

/*
 * The clean function for use in memgrind.c
 */

void clean();

// DEBUG printout switch
#define DEBUG 0

// These two marco replace malloc and free with our customized malloc and free
#define malloc(size) mymalloc(size, __FILE__, __LINE__)
#define free(p) myfree(p, __FILE__, __LINE__) 
