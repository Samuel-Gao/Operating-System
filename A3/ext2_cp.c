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
* Copt file from source to destination 
* Return ENOENT if path does not exist
*/
int main(int argc, char *argv[]) {
	
	if (argc < 4){
		perror("Error: Expected argument ext2_cp <disk> <path to a file> <absolute path>");
		return -1;
	}	

	char* vir_disk = argv[1];
	char *source = argv[2];
	char *dest = argv[3];

	if (strncmp(dest, "/", 1) != 0 || strncmp(source, "/", 1) != 0){
		perror("Error: Invalid path. Please use absolute path.");
		return -1;
	} 

	int fd = open(vir_disk, O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

   	struct ext2_inode *src_file_inode = find_inode_by_dir(source);
   	struct ext2_inode *dest_dir_inode = find_inode_by_dir(dest);

   	//check if path are both valid
   	if ( src_file_inode == NULL|| dest_dir_inode == NULL){
   		printf("No such file or directory\n");
   		return ENOENT;
   	
   	//if source is not a file
   	}else if (!(src_file_inode->i_mode & EXT2_S_IFREG)){
   		printf("Source is not a file\n");
   		return ENOENT;

   	//if destination is not a dir
   	}else if (!(dest_file_inode->i_mode & EXT2_S_IFDIR)){
   		printf("Destination is not a directory\n");
   		return ENOENT;
   	}

   	
    //remove inode for source, updates data
    //create new inode for destination, updates data
	return 0;
}
