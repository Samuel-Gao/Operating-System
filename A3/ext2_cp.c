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
  

  // char *dest_n_src = malloc(sizeof(dest) + sizeof(source));
  // strcpy(dest_n_src, dest);

  //create /path/file, use for checking if file already exist
  // if (strcmp(&dest[strlen(dest) - 1], "/") == 0){
  //   strcat(dest_n_src, get_last_dir(source));
  // }else{
  //   strcat(dest_n_src, "/");
  //   strcat(dest_n_src, get_last_dir(source));
  // }

  //If paths are not absolute, return error
	if (strncmp(dest, "/", 1) != 0 || strncmp(source, "/", 1) != 0){
		printf("Error: Invalid path. Please use absolute path.\n");
		return ENOENT;
	} 

  //Open file on native OS
  FILE *file;
  file = fopen(source,"r"); 
  if( file == NULL ) {
      printf("No such file or directory.\n");
      return ENOENT;
  }

  //check if source file is directory
  struct stat statbuf;
  stat(source, &statbuf);
  if(S_ISDIR(statbuf.st_mode)){
    printf("Error:Cannot copy directory. Please choose a file.\n");
    return ENOENT;
  }
  

  fseek(file, 0, SEEK_END); 
  int file_size = ftell(file); 
  fseek(file, 0, SEEK_SET); 

  //read virtual disk
	int fd = open(vir_disk, O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(disk == MAP_FAILED) {
  	perror("mmap");
  	exit(1);
  }

  char actualpath [strlen(dest)+1];
  realpath(dest, actualpath);
  
  if (strcmp(actualpath, dest) == 0){
    strcpy(actualpath, "/");
  }

  struct ext2_inode *dest_dir_inode = find_inode_by_dir(dest);
  
  //if source or destination path is invalid, return error
  if (dest_dir_inode != NULL){
    printf("Error:File or directory already exist.\n");
    return EEXIST;
  }

  struct ext2_inode *dest_path_inode = find_inode_by_dir(actualpath);

  //if path to destination does not exist
  if (dest_path_inode == NULL){
    printf("Error:Path to destination does not exist.\n");
    return ENOENT;
  }


  // if (!(dest_path_inode->i_mode & EXT2_S_IFDIR)){
  //     printf("File or directory already exist.\n");
  //     return EEXIST;
  // }

 	
  
  //if source file already exist in dest, return error
  // struct ext2_inode *dest_n_src_inode = find_inode_by_dir(dest_n_src);
  // if (dest_n_src_inode != NULL){
  //   printf("File or directory already exist\n");
  //   return EEXIST;
  // }
  
  int alloc_node = alloc_inode();
    
  if (alloc_node == -1){
    printf("Error:Insufficient number of inodes\n");
    exit(1);
  }

  char *ch = malloc(EXT2_BLOCK_SIZE);
  int block_count = 0;
  struct ext2_inode *inode = find_inode(alloc_node);
  inode->i_mode = EXT2_S_IFREG;
  inode->i_size = file_size;

  int alloc_block = 0;
  int indir_alloc_block;
  int indirect_block_count = 0;
  unsigned int *indirect_block;

  while( !feof(file)){
       // ch = fgetc(file);
       fgets(ch, EXT2_BLOCK_SIZE, file);
        
        if (block_count < 12) { // direct block
            alloc_block = allocate_block();

            if  (alloc_block == -1){
                inode->i_block[block_count] = 0;
                printf("Error:Insufficient number of blocks.\n");
                exit(1);
            }

            inode->i_block[block_count] = alloc_block;
            
            unsigned char *block = (unsigned char *) (disk + EXT2_BLOCK_SIZE *alloc_block);
            memcpy(block, ch, EXT2_BLOCK_SIZE);

            inode->i_blocks+=2;
            block_count++;

        }

        else if (block_count == 12) { 

          if (indirect_block_count == 0){

            alloc_block = allocate_block();

            if  (alloc_block == -1){
                inode->i_block[block_count] = 0;
                printf("Error:Insufficient number of blocks.\n");
                exit(1);
            }
            inode->i_blocks+=2;
            indirect_block = (unsigned int *)(disk + EXT2_BLOCK_SIZE * alloc_block);
            inode->i_block[block_count] = alloc_block;

          }
        
            //Block size is 1024, each pointer is 4byte, we have total of 256 available blocks
            indir_alloc_block = allocate_block();
            if  (indir_alloc_block == -1){
                inode->i_block[block_count] = 0;
                int x;
                for (x=0; x <128; x++){
                  printf("block at %i is %i\n",x, indirect_block[x]);
                }
                printf("Error:Insufficient number of blocks.\n");
                exit(1);
            }

            indirect_block[indirect_block_count] = indir_alloc_block;
            unsigned char *block = (unsigned char *) (disk + EXT2_BLOCK_SIZE *indir_alloc_block);
            memcpy(block, ch, EXT2_BLOCK_SIZE);

            inode->i_blocks+=2;
            indirect_block_count++;
        }
        else{
          continue; 
        }
    }

  add_entry(dest_path_inode,alloc_node, get_last_dir(dest),EXT2_FT_REG_FILE);

	return 0;
}

