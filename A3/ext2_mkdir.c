#include "helper.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

unsigned char *disk;

int ext2_mkdir(char *path, struct ext2_inode *path_inode){
	
	int alloc_node = alloc_inode();

    if (alloc_node == -1){
      printf("Insufficient number of inodes\n");
      return -1;
    }

	struct ext2_inode *inode = find_inode(alloc_node);
    for (int i=0;i<15;i++)
		inode->i_block[i] = 0;

    //set metatdata
    inode->i_mode = EXT2_S_IFDIR;
    
	char *fname = get_last_dir(path);
	
	add_entry(path_inode, alloc_node, fname, EXT2_FT_DIR);
	return 0;

}

/* Take 2 arguments. First one is virtual disk, second
 * is absolute path in which the dir is to be created.
 * If path does not exist, dir already exist, return
 * ENOENT or EEXIST
 */

int main(int argc, char *argv[]) {

	if (argc != 3){
		perror("Error: Expected argument ext2_mkdir <disk> <absolute path>");
		exit(1);
	}	

	char *vir_disk = argv[1];
	char *path = argv[2];
	
	//Error checking
	if (strncmp(path, "/", 1) != 0){
		printf("Error: Invalid path. Please use absolute path.\n");
		return ENOENT;
	} 
	//Read virtual disk
	int fd = open(vir_disk, O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

	char actualpath [strlen(path)+1];

	realpath(path, actualpath);
	struct ext2_inode *dir_inode = find_inode_by_dir(path);

	//if mkdir at root level
	if (strcmp(actualpath, path) == 0){
		struct ext2_inode *path_inode = find_inode(2);

		//check if dir already exist
		if (dir_inode != NULL){
			printf("Error: Directory already exist.\n");
			return EEXIST;
		}

		return ext2_mkdir(path, path_inode);

	//create dir not at root level
	}else{
		char *parent_path = malloc(sizeof(char *));
		strncpy(parent_path, path, strlen(path) - strlen(get_last_dir(path)));
		struct ext2_inode *path_inode = find_inode_by_dir(parent_path);
	
		if (path_inode == NULL || (path_inode->i_mode & EXT2_S_IFREG)){
			printf("Error: No such file or directory.\n");
			return ENOENT;
		}

		// struct ext2_inode *dir_inode = find_inode_by_dir(path);
		if (dir_inode != NULL) {
			printf("Error: Directory already exist.\n");
			return EEXIST;
		}

		return ext2_mkdir(path, path_inode);

	}

	return 0;

}