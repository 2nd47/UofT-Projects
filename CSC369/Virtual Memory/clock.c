#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

// Keep track of which frame our clock hand is pointing at
int clockHand;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	if (clockHand == memsize - 1)
			clockHand = 0;
	// Iterate over core map, setting frame PG_REF off until we find one
	// that is on; evict that frame
	while (coremap[clockHand].pte->frame & PG_REF) {
		// Mark PG_REF as off when we iterate over a frame
		coremap[clockHand].pte->frame &= ~(PG_REF);
		// If our clock has reached the end of our possible memory space,
		// restart from the beginning of the list again
		if (clockHand == memsize - 1)
			clockHand = -1;
		// Move one step forward regardless
		clockHand++;
	}
	return clockHand;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// Set the reference bit on
	p->frame |= PG_REF;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	// Set our clock hand to start
	clockHand = 0;
}
