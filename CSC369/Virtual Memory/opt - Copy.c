#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"
#include "opt.h"

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	// To evict a frame in OPT, we have to go across all frames currently in
	// the coremap and see which of those has the longest time to be called next.
	// In this implementation, the timeNode with the longest time from now that 
	// belongs to a frame in the coremap.
	int i;
	int nextUse;
	char *mem_ptr;
	unsigned int evictFrame = 0;
	unsigned int evictTime = currTime;
	for (i = 0; i < memsize; i++) {
		mem_ptr = &physmem[i * SIMPAGESIZE];
		// Touch ever coremap entry and find its next use
		nextUse = nextUse(findRef(vaddr));
		// If the frame is never used again, evict it immediately
		if (nextUse == -1)
			return frame;
		// If it's used after the one closest to 'never' found so far,
		// replace it with the reference we just found
		if (nextUse > evictTime) {
			evictTime = nextUse;
			evictFrame = frame;
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
	// Used to create new reference nodes
	currTime = 0;
	topRef = NULL;
	refNode *refNode;
	timeNode *timeNode;
	// Used to read in trace file
	char buf[MAXLINE];
	addr_t vaddr = 0;
	char junk;
	FILE *tfp;

	// We need to read into the trace and file and record all the traces
	// that will happen. We use the implemented structs to do this.
	if((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
	}
	while(fgets(buf, MAXLINE, tfp) != NULL) {
		if (buf[0] != '=') {
			sscanf(buf, "%c lx", &junk, &vaddr);
		}
		// If we don't have a top of our linked list, make one
		if (topRef == NULL) {
			topRef = initRefNode(<<FRAMENUM>>);
			topRef->timeStamp = initTimeNode();
		} else {
			// Check if the frame already exists or needs to be added
			if ((refNode = findRef(<<FRAMENUM>>)) == NULL) {
				initTimeNode(refNode, currTime++);
			} else {
				refNode = initRefNode(<<FRAMENUM>>);
			}
		}
	}
	// Reset time for the real trace to begin
	currTime = 0;
}

/* Given a frame number vaddr, find the corresponding refNode
 * in our struct. We are guaranteed to find it because we recorded
 * everything that would have been called in the trace.
 *
 * For when we first trace through the file, return NULL if not found.
 */
refNode *findRef(unsigned int vaddr) {
	refNode *iter = topRef;
	while (iter) {
		if (iter->frame == vaddr)
			return iter;
		iter = iter->next;
	}
	return NULL;
}


/* Given some reference frame f, find its next occurrence in the trace
 * (first occurrence after our current time in the trace)
 * We can assume there is at least one timeNode because it has a refNode
 */
int nextUse(refNode *ref) {
	timeNode *iter = ref->timeNode;
	// Go through events until we get to the one closest to the curret one
	while (iter && (iter->timeStamp =< currTime))
		iter = iter->next;
	// The next one will be the next time the event occurs, if at all
	iter = iter->next;
	// If it is never used again, have a special case of return -1
	if (!iter)
		return -1;
	return iter->timeStamp;
}

/* Initializer for our refNode
 * We handle the case of first ever initialization here; this means
 * creating a new time event as well.
 */
refNode *initRefNode(unsigned int vaddr) {
	refNode *node;
	if ((node = malloc(sizeof(refNode))) == NULL)
		exit(1);
	node->vaddr = vaddr;
	node->timeStamp = initTimeNode(node, currTime);
	// Move this node to the front of our linked list structure
	node->next = topRef;
	topRef = node;
	return node;
}

/* Initializer for our timeNode which puts it to the end of the
 * linked list of timeNodes for a given refNode.
 */
timeNode *initTimeNode(refNode *ref, unsigned int timeVal) {
	timeNode *node;
	if ((node = malloc(sizeof(timeNode))) == NULL)
		exit(1);
	// Initialize its time value
	node->timeStampe = timeVal;
	// Move this node to the back of our linked list structure
	timeNode *iter = ref->timeStamp;
	// If the list doesn't exist, start it
	if (!iter) {
		ref->timeStamp = node;
		node->next = NULL;
	} else {
		// A new event always needs to go to the end
		while (iter)
			iter = iter->next;
		iter->next = node;
		node->next = NULL;
	}
	return node;
}