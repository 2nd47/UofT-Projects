#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "pagetable.h"
#include "opt.h"

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	// To evict a frame in OPT, we have to go across all frames currently in
	// the physmem and see which of those has the longest time to be called next.
	// In this implementation, the timeNode with the longest time from now that 
	// belongs to a frame in the physmem.
	int i;
	int nextTime;
	addr_t *vaddr;
	char *memptr;
	unsigned int evictFrame = 0;
	unsigned int evictTime = currTime;
	for (i = 0; i < memsize; i++) {
		memptr = &physmem[i * SIMPAGESIZE];
		vaddr = (addr_t *)(memptr + sizeof(int));
		// Touch every physmem entry and find its next use
		// If the frame is never used again, evict it immediately
		if ((nextTime = nextUse(*vaddr)) == -1)
			return i;
		// If it's used after the one closest to 'never' found so far,
		// replace it with the reference we just found
		if (nextTime > evictTime) {
			evictTime = nextTime;
			evictFrame = i;
		}
	}
	return evictFrame;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	// Make sure we're moving ahead in time whenever a reference occurs
	currTime++;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	// Used to create new time stamp nodes
	currTime = 0;
	// Used to read in trace file
	FILE *tfp;
	char buf[MAXLINE];
	addr_t vaddr = 0;
	char junk;

	// Initiliaze our page table entry array to hold timeNode entries
	if ((pageArray = (malloc(
		PTRS_PER_PGDIR * PTRS_PER_PGTBL * sizeof(timeNode)))) == NULL) 
		exit(1);
	memset(pageArray, 0, PTRS_PER_PGDIR * PTRS_PER_PGTBL * sizeof(timeNode));
	// We need to read into the trace and file and record all the traces
	// that will happen. We use the implemented structs to do this.
	if ((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
	}
	while(fgets(buf, MAXLINE, tfp) != NULL) {
		if (buf[0] != '=') {
			sscanf(buf, "%c %lx", &junk, &vaddr);
		}
		initTimeNode(vaddr, currTime++);
	}
	// Reset time for the real trace to begin
	currTime = 0;
}

/* Index into our pageArray
 * -- I know this could be done by a macro but then I still need to
 * index pageArray. This is cleaner in my opinion.
 */
timeNode *indexArray(addr_t vaddr) {
	return &pageArray[PG_INDEX(vaddr)];
}

/* Given some reference frame f, find its next occurrence in the trace
 * (first occurrence after our current time in the trace)
 * We can assume there is at least one timeNode because it has a refNode
 */
int nextUse(addr_t vaddr) {
	timeNode *iter = indexArray(vaddr);
	// Go through events until we get to the one closest to the curret one
	while (iter->next && (iter->timeStamp <= currTime))
		iter = iter->next;
	// The next one will be the next time the event occurs, if at all
	iter = iter->next;
	// If it is never used again, have a special case of return -1
	if (!iter)
		return -1;
	return iter->timeStamp;
}

/* Initializer for our timeNode which puts it to the end of the
 * linked list of timeNodes for a given refNode.
 */
void initTimeNode(addr_t vaddr, unsigned int timeVal) {
	timeNode *node;
	// Allocate room for the timeNode if it doesn't exist in the array yet
	if ((node = malloc(sizeof(timeNode))) == NULL)
		exit(1);
	// Move this node to the back of our linked list structure
	timeNode *iter = indexArray(vaddr);
	// If the list doesn't exist, start it
	if (!iter) {
		node = &pageArray[PG_INDEX(vaddr)];
		node->timeStamp = timeVal;
		node->next = NULL;
	} else {
		// A new event always needs to go to the end
		while (iter->next)
			iter = iter->next;
		iter->next = node;
	}
	// Initialize its time value
	node->timeStamp = timeVal;
	node->next = NULL;
}