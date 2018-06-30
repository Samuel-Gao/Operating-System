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

/* Takes two argument, first argument is virtual disk image,
 * second and third arguments are absolute path. Second argument is 
 * source file and third argument is targeted path.
 * This command creates a hard link for the source file at the targeted
 * link. 
 * If source file does not exst, return ENOENT.
 * If source already exist at targeted path, return EEXIST. 
 * If source file is directory, return EISDIR.
 * If flag -s given, creates symbolic link instead.  
 */

int main(int argc, char *argv[]){
	if (argc < 4){
		perror("Error: Expected argument ext2_ls <disk>[-s]<source file path><absolute path>");
		exit(1);
	}

	char *vir_disk = argv[1];
	int flag;
	char *src_path;
	char *target_path;

	//check if user flag -s
	if (strcmp(argv[2], "-s") == 0){
		flag = 1;
		src_path = argv[3];
		target_path = argv[4];
	}else{
		flag = 0;
		src_path = argv[2];
		target_path = argv[3];
	}
	

	//check absolute path is provided
	if ((strncmp(src_path, "/", 1) != 0) || (strncmp(target_path, "/", 1) != 0)){
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

    struct ext2_inode *src_file_inode = find_inode_by_dir(src_path);
    struct ext2_inode *target_path_inode = find_inode_by_dir(target_path);

    
    //check source file exist
    if (src_file_inode == NULL){
    	printf("Error: No such file or directory \n");
    	return ENOENT;
    
    }else if (target_path_inode != NULL){
    	printf("Error: File already exist.\n");
    	return EEXIST;
  
    //check if source file is directory
    }else if (src_file_inode->i_mode & EXT2_S_IFDIR){
    	printf("Error: Source file is directory.\n");
    	return EISDIR;
    
    //check source file does not exist at targeted path
    }

    char *actualpath = malloc(sizeof(char));
    strncpy(actualpath, target_path, strlen(target_path) - strlen(get_last_dir(target_path)));
    struct ext2_inode *target_path_file_inode = find_inode_by_dir(actualpath);

    if (target_path_file_inode == NULL){
    	printf("Error: Directory to destination does not exist.\n");
    	return ENOENT;
    }

    if (!flag){
    	create_hard_link(src_file_inode, target_path_file_inode, get_last_dir(target_path));
    }else{
    	create_symbolic_link(src_path, target_path_file_inode, get_last_dir(target_path));
    }

	return 0;
}