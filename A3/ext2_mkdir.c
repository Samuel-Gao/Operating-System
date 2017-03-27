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

		//create directory
		}else{
			int alloc_node = alloc_inode();
    		printf("node alloc:%i\n", alloc_node);
    		// printf("block alloc is:%i\n", alloc_inode->i_block[0]);
	        
	        if (alloc_node == -1){
	          printf("Insufficient number of inodes\n");
	          exit(1);
	        }

	        struct ext2_inode *inode = find_inode(alloc_node);
	        // inode->i_block[0] = allocate_block();
	        inode->i_links_count = 1;
	        inode->i_mode |= EXT2_S_IFDIR;
       		
			struct ext2_dir_entry_2 *entry = create_new_entry(alloc_node, get_last_dir(path), EXT2_FT_DIR);
			add_entry(path_inode, entry, get_last_dir(path));
		}

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

		int alloc_node = alloc_inode();
		struct ext2_inode *inode = find_inode(alloc_node);
		// inode->i_block[0] = allocate_block();
        inode->i_links_count = 1;
        inode->i_mode |= EXT2_S_IFDIR;

        if (alloc_node == -1){
          printf("Insufficient number of inodes\n");
          exit(1);
        }
   
		struct ext2_dir_entry_2 *entry = create_new_entry(alloc_node, get_last_dir(path), EXT2_FT_DIR);
		add_entry(path_inode, entry, get_last_dir(path));
	}
	
	

	return 0;

}