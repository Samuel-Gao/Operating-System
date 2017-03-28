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
	char *dir = argv[2];
	// int flag;

	if (check_error(argc, argv) == -1){
		exit(1);
	}

	//check if user flag -a
	// if (strcmp(argv[2], "-r") == 0){
	// 	flag = 1;
	// 	dir = argv[3];
	// }else{
	// 	flag = 0;
	// 	dir = argv[2];
	// }

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


    //Error Check
    //If the file does not exist return ENOENT
    struct ext2_inode *inode = find_inode_by_dir(dir);
    if (inode == NULL){
    	printf("Error: No such file or directory.\n");
    	return ENOENT;
    
    //If it is a directory, return EISDIR
    }else if ((inode != NULL) & (inode->i_mode & EXT2_S_IFDIR)){
    	printf("Error: File is a directory.\n");
    	return EISDIR;
    
    }else {
    	remove_file(dir);
    	return 0;
    }

    exit(1);

  	if (inode != NULL){
  		struct ext2_inode *root = find_inode(2);
  		int block = root->i_block[0];
  		struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + block * 1024);
  		
  		char *len = (char *)(disk + block * 1024);
  		char *total = len + 1024;

  		struct ext2_inode *f_inode = find_inode(12);
  		f_inode->i_size = 0;
  		f_inode->i_block[0] = 0;
  		printf("i_size %i\n",f_inode->i_size );

  		while (len < total){
  			printf("inode %i\n",dir->inode );
  			printf("rec_len %i\n",dir->rec_len );
  			printf("name_len %i\n",dir->name_len );
  			printf("name %s\n",dir->name);
  			printf("================\n");
  			
  			len += dir->rec_len;
  			dir = (struct ext2_dir_entry_2 *)len;
  		}
  		// printf("%lu\n", (unsigned long)time(NULL)); 
  	}


}