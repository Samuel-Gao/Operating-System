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

/* Helper function to check argument validity.
* return -1 if error exist, else return 1 
*/
int check_error(int argc, char *argv[]){
	if (argc < 3){
		perror("Error: Expected argument ext2_rm <disk>[-r]<absolute path>");
		return -1;
	}

	return 1;
}

/*
Takes two command line arguments. 
The first is the name of an ext2 formatted virtual disk, 
and the second is an absolute path to a file or link 
(not a directory) on that disk. Remove the specified file from the disk. 
If the file does not exist or if it is a directory, 
then your program should return the appropriate error. 
*/
int main(int argc, char *argv[]) {

	char *vir_disk=argv[1];
	char *dir = argv[2];

	if (check_error(argc, argv) == -1){
		exit(1);
	}

	if (strncmp(dir, "/", 1) != 0){
		perror("Error: Invalid path. Please use absolute path.");
		exit(1);
	} 

	//Read virtual disk
	int fd = open(vir_disk, O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }


    //Error Check
    //If the file does not exist return ENOENT
    struct ext2_inode *inode = find_inode_by_dir(dir);
    if (inode == NULL){
    	printf("Error: No such file or directory.\n");
    	return ENOENT;
    
    //If it is a directory, return EISDIR
    }else if ((inode != NULL) && (inode->i_mode & EXT2_S_IFDIR)){
    	printf("Error: Cannot remove a directory.\n");
    	return EISDIR;
    
    }else {
    	remove_file(dir);
    	return 0;
    }

    exit(1);
}