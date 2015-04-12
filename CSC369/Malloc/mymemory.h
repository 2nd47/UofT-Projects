#define HEADER_SIZE sizeof(struct allocnode)

#define MAGIC_NUMBER 12345

#define PAGE 4096
#define ALIGNMENT 8
#define ALIGN8(size) (((size) + (ALIGNMENT - 1)) & (~(ALIGNMENT - 1)))
#define ALIGNPAGE(size) (((size) + (PAGE - 1)) & (~(PAGE - 1)))

struct freenode{
    int size;
    struct freenode *prev;
    struct freenode *next;
}__attribute__((packed, aligned(8)));

struct allocnode {
    int size;
    int magic;
}__attribute__((packed, aligned(8)));

typedef struct freenode FreeHeader;
typedef struct allocnode AllocHeader;

FreeHeader *moveFreeHeader(FreeHeader *node, int movement);
FreeHeader *addBlock(AllocHeader *ptr, FreeHeader *prev);
void coalesceBlocks(FreeHeader *free);
FreeHeader *extendHeap(FreeHeader *endHeap, unsigned int size);
AllocHeader *createHeader(AllocHeader *ptr, unsigned int size);

FreeHeader *freep;
pthread_mutex_t *lock;