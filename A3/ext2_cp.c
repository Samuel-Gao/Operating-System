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
		return ENOENT;
	}	

	char* vir_disk = argv[1];
	char *source = argv[2];
	char *dest = argv[3];
  char *dest_n_src = malloc(sizeof(dest) + sizeof(source));
  strcpy(dest_n_src, dest);

  //create /path/file, use for checking if file already exist
  if (strcmp(&dest[strlen(dest) - 1], "/") == 0){
    strcat(dest_n_src, get_last_dir(source));
  }else{
    strcat(dest_n_src, "/");
    strcat(dest_n_src, get_last_dir(source));
  }

  //If paths are not absolute, return error
	if (strncmp(dest, "/", 1) != 0 || strncmp(source, "/", 1) != 0){
		printf("Error: Invalid path. Please use absolute path.\n");
		return ENOENT;
	} 

  //read virtual disk
	int fd = open(vir_disk, O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(disk == MAP_FAILED) {
  	perror("mmap");
  	exit(1);
  }

 	struct ext2_inode *src_file_inode = find_inode_by_dir(source);
 	struct ext2_inode *dest_dir_inode = find_inode_by_dir(dest);
  
  //if source or destination path is invalid, return error
  if ( src_file_inode == NULL || dest_dir_inode == NULL){
    printf("No such file or directory\n");
    return ENOENT;
  }

  //if destination not a dir, return error
  if (!(dest_dir_inode->i_mode & EXT2_S_IFDIR)){
      printf("Destination is not a directory.\n");
      return ENOENT;
  }
  
  //if source file already exist in dest, return error
  struct ext2_inode *dest_n_src_inode = find_inode_by_dir(dest_n_src);
  if (dest_n_src_inode != NULL){
    printf("File or directory already exist\n");
    return ENOENT;
  }
  
  //copy file or dir to dest
  if (src_file_inode->i_mode & EXT2_S_IFREG){
      
        int alloc_node = alloc_inode();
    
        if (alloc_node == -1){
          printf("Insufficient number of inodes\n");
          exit(1);
        }

        copy_inode(find_inode(alloc_node), src_file_inode);
        struct ext2_dir_entry_2 *entry = create_new_entry(alloc_node, get_last_dir(source), EXT2_FT_REG_FILE);
        add_entry(dest_dir_inode, entry, get_last_dir(source));
       
    
    //if source is not a file
  }else{
    int alloc_node = alloc_inode();
    
    if (alloc_node == -1){
      printf("Insufficient number of inodes\n");
      exit(1);
    }

    copy_inode(find_inode(alloc_node), src_file_inode);
    struct ext2_dir_entry_2 *entry = create_new_entry(alloc_node, get_last_dir(source), EXT2_FT_DIR );
    add_entry(dest_dir_inode, entry, get_last_dir(source));
  }
  

	return 0;
}

