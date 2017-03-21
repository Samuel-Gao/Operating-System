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
unsigned int glx = 10;

/* Helper function to check whether given directory exist of dis.
 * Return 1 if exist, else 0.
*/
int check_dir_valid(char *dir){
  
  int is_root = strcmp(dir, "/") == 0;
  int is_lostnfound = strcmp(dir, "/lost+found") == 0;

  if (is_root || is_lostnfound){
    return 1;
  }

  //split into token
  struct ext2_inode* root = find_inode(2);
  struct ext2_inode* cur_inode = root;
  struct ext2_dir_entry_2* cur_dir = (struct ext2_dir_entry_2*)(disk + cur_inode->i_block[0] * EXT2_BLOCK_SIZE);

  // int counter=0;
  // char *file_name = cur_dir->name;

  // printf("inode size is %i\n", cur_dir->rec_len);
  // while (counter < cur_inode->i_size){
  //   printf("file name is %s\n",file_name);
  //   file_name += cur_dir->rec_len;
  //   counter += cur_dir->rec_len;;
  // }

  return 1;
}

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

    // check_dir_valid(dir);
    //struct ext2_inode *root_node = find_inode(2);
    //print_inode_dir(root_node);
    struct ext2_inode *inode = find_inode_by_dir(dir);
    if (inode == NULL){
    	printf("No such file or directory\n");
    }else{
    	print_inode_dir(inode, flag_a);
    	return ENOENT; 
    }

   
    return 0;

}
