// Used for reading in the tracefile as in sim.c
#define MAXLINE 256
#define SIMPAGESIZE 16  /* Simulated physical memory page frame size */

#define PG_INDEX(x) ((PGDIR_INDEX(x) * PTRS_PER_PGDIR) + PGTBL_INDEX(x))

extern char *tracefile;

extern char *physmem;

extern int memsize;

extern int debug;

extern struct frame *coremap;

// Linked list struct for reference timestamps
typedef struct timeNode_ {
	unsigned int timeStamp; // Timestamp relative to start of trace
	struct timeNode_ *next; // Next time this address is referenced
} timeNode;

timeNode *pageArray;

// The current time of our trace
unsigned int currTime;

// Various functions needed to keep track of traces
timeNode *indexArray(addr_t vaddr);
int nextUse(addr_t vaddr);
void initTimeNode(addr_t vaddr, unsigned int timeVal);
