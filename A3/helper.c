
#include "helper.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

extern unsigned char *disk;

/* Helper function to find the inode at the given index
 * from the inode table. Our index starts at 1. Root inode is at 2
*/
struct ext2_inode *find_inode(int index){
  
  struct ext2_group_desc* group_desc;
  void* inode_tbl;
  struct ext2_inode *inode;

  group_desc = (struct ext2_group_desc*)(disk + 2 * EXT2_BLOCK_SIZE);
  inode_tbl = disk + group_desc->bg_inode_table * EXT2_BLOCK_SIZE;
  inode = (struct ext2_inode*)(inode_tbl + (index-1) * sizeof(struct ext2_inode));

  return inode;
}

/* Helper function to print directories given the inode
*  If flag is 1, print . and .. as well. 
*/
void print_inode_dir(struct ext2_inode *inode, int flag){

	unsigned int dir_block;
	
	//loop through for each block
	int block = 0;
	while (inode->i_block[block] != 0){
		dir_block = inode->i_block[block];
		struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + (dir_block * EXT2_BLOCK_SIZE));
	
		// if (strcmp(dir->file_type, EXT2_FT_REG_FILE) == 0){
		// 	printf("inode is %s\n",dir->name);
		// 	return;
		// }

		// printf("inode is %i\n",dir->name_len);
		unsigned char *cur_size;
		unsigned char *total_size = (disk + (dir_block * EXT2_BLOCK_SIZE));
		cur_size = total_size;
		
		//loop through entries for each inode
		while (cur_size < total_size + EXT2_BLOCK_SIZE){

			char *fname = malloc(sizeof(dir->name_len));
			strncpy(fname,dir->name, dir->name_len);
			
			if (flag && (strcmp(fname, ".") == 0 || strcmp(fname, "..")==0)){
				//don't print 
			}else{
				printf("%s\n", fname);
			}
			
			cur_size += dir->rec_len;
			dir = (struct ext2_dir_entry_2 *)cur_size;
		}
		block++;
	}
}

/* Helper function to find the inode number of a file in the given inode
* if it exist, else return -1
*/
int find_file_inode(struct ext2_inode *inode, char *file_name){

	unsigned int dir_block;
	// unsigned int inode_size;
	// unsigned int cur_size;
	



	// dir_block = inode->i_block[block];

	// struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + (dir_block * EXT2_BLOCK_SIZE));
	// inode_size = inode->i_size;
	// cur_size = 0;
	
	
	// while (cur_size < inode_size){

	// 	char *fname = malloc(sizeof(dir->name_len));
	// 	strncpy(fname,dir->name, dir->name_len);
		
	// 	if (strcmp(fname, file_name) == 0){
	// 		return dir->inode;

	// 	}
	// 	dir = (void *)dir + dir->rec_len;
	// 	cur_size += dir->rec_len;


	// 	if (cur_size % EXT2_BLOCK_SIZE == 0) { 
	//       block++;
	//       dir_block = inode->i_block[block];
	//       dir = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * dir_block);
	//     }
	// }

	//loop through for each block
	int block = 0;
	while (inode->i_block[block] != 0){
		dir_block = inode->i_block[block];
		struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + (dir_block * EXT2_BLOCK_SIZE));
		
		unsigned char *cur_size;
		unsigned char *total_size = (disk + (dir_block * EXT2_BLOCK_SIZE));
		cur_size = total_size;
		
		//loop through entries for each inode
		while (cur_size < total_size + EXT2_BLOCK_SIZE){

			char *fname = malloc(sizeof(dir->name_len));
			strncpy(fname,dir->name, dir->name_len);
			
			if (strcmp(fname, file_name) == 0){
				printf("size of bfiel is %i\n", dir->name_len);
				printf("inode is %i\n", dir->inode);
				return dir->inode;
			}
			
			cur_size += dir->rec_len;
			dir = (struct ext2_dir_entry_2 *)cur_size;
		}
		block++;
	}
	
	return -1;
}

/* Helper function to find the inode given the directory
*/
struct ext2_inode *find_inode_by_dir(char *dir){

	//return root node if dir is /
	if (strlen(dir) == 1){
		return find_inode(2);
	
	//trim if there is / in the path
	}else if (strlen(dir) > 1 && strcmp(&dir[strlen(dir) - 1], "/") == 0){
		dir[strlen(dir) - 1] = '\0';
	}

	char *token;
	struct ext2_inode *cur_inode;

	//look for inode for the dir
	while ((token = strsep(&dir, "/"))) {

		if (strcmp(token, "") == 0){
			cur_inode = find_inode(2);
			find_file_inode(cur_inode, token);
		}else{
			int inode_num = find_file_inode(cur_inode, token);
			
			//check path exist, else return null
			if (inode_num != -1){
				cur_inode = find_inode(inode_num);
			}else{
				return NULL;
			}
			
		}
	}

	return cur_inode;
} 
