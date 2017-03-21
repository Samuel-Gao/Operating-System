#include "helper.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

unsigned char *disk;
/* Takes 3 arguments, disk, native path to a file, absolute path
* Copt file on native path to absolute path. 
* Return ENOENT if path does not exist
*/
int main(int argc, char *argv[]) {
	
	if (argc < 4){
		perror("Error: Expected argument ext2_cp <disk> <path to a file> <absolute path>");
		return -1;
	}	

	char* vir_disk = argv[1];
	char *nat_path = argv[2];
	char *ab_path = argv[3];

	if (strncmp(ab_path, "/", 1) != 0 || strncmp(nat_path, "/", 1) != 0){
		perror("Error: Invalid path. Please use absolute path.");
		return -1;
	} 

	int fd = open(vir_disk, O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }


   	//check if path are both valid
   	if (path_exist(nat_path) == -1 || path_exist(ab_path) == -1){
   		printf("No such file or directory\n");
   		return ENOENT;
   	}

   	
   	
    //remove inode for source, updates data
    //create new inode for destination, updates data
	return 0;
}
