#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int *clock_list; //array of reference bit
int clock_index; //index of the oldest frame


/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {

	int temp = clock_index;
	while (1){

		int clock_value = clock_list[clock_index];
		if (clock_value == 0){
			int frame_to_evic = clock_index;
			clock_index = (clock_index + 1) % memsize;
			return frame_to_evic;
		
		}else if (clock_value >= 1){
			clock_list[clock_index] = 0;
			clock_index = (clock_index + 1) % memsize;
		}
	}
	return temp;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	
	//for every reference, update the reference bit in clock list.
	int frame = p->frame >> PAGE_SHIFT;
	clock_list[frame] ++;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	clock_list = malloc(sizeof(int) * memsize);
	int c;
	for (c=0; c<memsize; c++){
		clock_list[c] = -1;
	}
	clock_index = 0;
}
