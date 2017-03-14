#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int *stack;
int stack_count; //0 is bottom of stack

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	return stack[0];
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {

	int frame = p->frame >> PAGE_SHIFT;
	
	
	//If stack is not full, add to top of stack
	if (stack_count < memsize){
		stack[stack_count] = frame;
		stack_count++;

	}else if (stack_count >= memsize){

		//if stack full and page already exist on stack, move to top
		if (stack[stack_count] == frame){
			return; //already at top of the stack. Do nothing
		}else{
			
			//page exist somewhere on stack
			//move page to top of stack
			int c;
			int x;
			
			for (c=0; c < stack_count; c++){
				if (stack[c] == frame){
			
					int target = stack[c];
					for (x = c; x < stack_count; x++){
						stack[x] = stack[x + 1];

					}

					stack[stack_count] = target;
					
					return;
				}
			}

			//page does not exist on stack, replace bottom page
			for (x = 0; x < stack_count; x++){
				stack[x] = stack[x + 1];
			}	
			stack[stack_count] = frame;

		}
	}

	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	stack = malloc(memsize * sizeof(int));

}
