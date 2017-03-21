#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "helper.h"

unsigned char *disk;

/* Helper function to check argument validity.
* return -1 if error exist, else return 1 
*/
int check_error(int argc, char *argv[]){
	if (argc < 3){
		perror("Error: Expected argument ext2_ls <disk>[-a]<absolute path>");
		return -1;
	}

	return 1;
}

/* Perform ls -1
*
* Takes a ext2 virtual disk and absolute path as 
* arguments. Print each dir on seperate line. If
* flag -a given, print absolute path of each entry. 
* If path not exist, print "No such file or directory" 
* and return ENOENT. If path end in "/", print content
* of last directory. If path is file/link, only print 
* file name
*/
int main(int argc, char *argv[]) {
	
	char *vir_disk=argv[1];
	char *dir = NULL;
	int flag_a;

	if (check_error(argc, argv) == -1){
		exit(1);
	}

	//check if user flag -a
	if (strcmp(argv[2], "-a") == 0){
		flag_a = 1;
		dir = argv[3];
	}else{
		flag_a = 0;
		dir = argv[2];
	}

	if (strncmp(dir, "/", 1) != 0){
		perror("Error: Invalid path. Please use absolute path.");
		return -1;
	} 

	//Read virtual disk
	int fd = open(vir_disk, O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

    if (find_inode_by_dir(dir, flag_a) == -1){
      printf("No such file or directory\n");
      return ENOENT;
    }
   
    return 0;

}
