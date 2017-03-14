#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

unsigned int head;

/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int fifo_evict() {
	
	//Return the head which is the oldest frame then update new head to second oldest
	unsigned int cur_head = head;
	head = (head + 1) % memsize;

	return cur_head;
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {
	return; 
	//nothing to do here since we don't need to keep a reference for fifo
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	head = 0;
}
