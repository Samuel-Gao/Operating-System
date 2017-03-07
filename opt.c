#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

#define MAXLINE 256

extern int memsize;

extern int debug;

extern struct frame *coremap;
extern char *tracefile; //used for getting future reference

int file_size; //num of address in trace file
int *vaddr_list; //array of virtual address. Used for knowing future ref.
int vaddr_counter;
int *ref_list;


/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	
	int longest_not_used = 0;

	//loop through our ref_list and check if any vaddr
	//is no longer used
	int ref_c;
	for (ref_c=0; ref_c < memsize; ref_c++){
		int vaddr_c;
		int used_for_future = 0;
		for (vaddr_c = vaddr_counter; vaddr_c < file_size; vaddr_c++){
			
			//if we find vaddr is used for future, save the position and break
			if (ref_list[ref_c] == vaddr_list[vaddr_c]){
				if(vaddr_c >= longest_not_used){
					longest_not_used = vaddr_c;
				}
				used_for_future = 1;
				break;
			}
		}
		
		//If vaddr not used anymore for the future, evic this frame
		if (used_for_future == 0){
			return ref_c;
		}
	}
 	
 	//if all vaddr will be used for future, evic page not used for longest time
 	for (ref_c=0; ref_c < memsize; ref_c++){
 		if (ref_list[ref_c] == vaddr_list[longest_not_used]){
 			return ref_c;
 		}
 	}

 	return -1; //should not get to this point unless bug occur
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	
	//for each
	int frame = p->frame >> PAGE_SHIFT;

	ref_list[frame] = vaddr_list[vaddr_counter];
	vaddr_counter++;
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	
	//Find the number of vaddr in trace file 
	//and then assign that much of memeory to vaddr_list
	FILE *tracef_size =  fopen(tracefile, "r"); //for counting
	FILE *tracef_content =  fopen(tracefile, "r"); //for populating

	file_size = 0;
	vaddr_counter = 0;
	
	char buf[MAXLINE];
	addr_t vaddr = 0;
	char type;

	//code taken from sim.c 
	while(fgets(buf, MAXLINE, tracef_size) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			file_size++;
		} 

	}
	vaddr_list = malloc(file_size * sizeof(int));	

	//populate vaddr_list with all the virtual address in tracefile
	int i = 0;
	while(fgets(buf, MAXLINE, tracef_content) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			vaddr_list[i] = vaddr;
			i ++; 
		} 
	}

	ref_list = malloc(sizeof(int) * memsize);

}

