// Used for reading in the tracefile as in sim.c
#define MAXLINE 256
#define SIMPAGESIZE 16  /* Simulated physical memory page frame size */

extern char *tracefile;

extern char *physmem;

extern int memsize;

extern int debug;

extern struct frame *coremap;

// Linked list struct for reference timestamps
typedef struct {
	unsigned int timeStamp; // Timestamp relative to start of trace
	timeStamp *next; // Next time this address is referenced
} timeNode;

// Linked list struct for unique references
typedef struct {
	unsigned int vaddr; // The unique vaddr
	timeNode *timeStamp; // The unique refrence node this holds
	refNode *next; // Next unique reference
} refNode

// Top of our linked list structure for unique references
refNode *topRef;

// The current time of our trace
unsigned int currTime;

// Various functions needed to keep track of traces
refNode *findRef(unsigned int vaddr);
int nextUse(refNode *ref);
refNode *initRefNode();
timeNode *initTimeNode(refNode *ref, unsigned int timeVal);
