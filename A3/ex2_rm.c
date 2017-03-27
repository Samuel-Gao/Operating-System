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
Once again, please read the specifications of ext2 carefully, 
to figure out what needs to actually happen when a file or 
link is removed (e.g., no need to zero out data blocks, 
must set i_dtime in the inode, removing a directory 
entry need not shift the directory entries after 
the one being deleted, etc.). 
Bonus(5% extra): Implement an additional "-r" 
flag (after the disk image argument), which allows 
removing directories as well. In this case, you will 
have to recursively remove all the contents of the 
directory specified in the last argument. If "-r" is 
used with a regular file or link, then it should 
be ignored (the ext2_rm operation should be carried out 
as if the flag had not been entered). If you decide to do 
the bonus, make sure first that your ext2_rm works, 
then create a new copy of it and rename it to ext2_rm_bonus.c, 
and implement the additional functionality in this separate source file.
*/
int main(int argc, char *argv[]) {

	char *vir_disk=argv[1];
	char *dir = NULL;
	int flag_a;

	if (check_error(argc, argv) == -1){
		exit(1);
	}

	//check if user flag -a
	if (strcmp(argv[2], "-r") == 0){
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

    //If the file does not exist return ENOENT
    //If it is a directory, return EISDIR


}