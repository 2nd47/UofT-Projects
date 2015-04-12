#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

void stackShift(int idx);
void stackPush(int e);

// Declare our stack and its top entry
int *stack;
int stackSize;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	int retFrame = stack[0];
	stackShift(0);
	return retFrame;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	int frameNum = p->frame >> PAGE_SHIFT;
	stackPush(frameNum);
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	// Initially 0 elements in our stack
	stackSize = 0;
	// Malloc our stack, exit on error
	if ((stack = (int *) malloc(memsize * sizeof(int))) == NULL) {
		exit(1);
	}
}

/* Push an item onto the stack and shift the stack over
 */
void stackPush(int e) {
	int i;
	// Remove element from stack if it exists in it
	for (i = 0; i < stackSize; i++)
		if (stack[i] == e) 
			stackShift(i);
	// Make room in the stack if it is full
	if (stackSize == memsize) 
		stackShift(0);
	stack[stackSize++] = e;
}

/* Shifts elements in the stack back to idx to close any holes
 */
void stackShift(int idx) {
	int i;
	for (i = idx; i < stackSize; i++) {
		stack[i] = stack[i + 1];
	}
	stackSize--;
}