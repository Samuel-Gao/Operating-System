
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
  inode = (struct ext2_inode*)(inode_tbl + ((index-1) * sizeof(struct ext2_inode)));

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

		unsigned char *cur_size;
		unsigned char *total_size = (disk + (dir_block * EXT2_BLOCK_SIZE));
		cur_size = total_size;
		
		//loop through entries for each inode
		while (cur_size < total_size + EXT2_BLOCK_SIZE){

			char *fname = malloc(sizeof(dir->name_len));
			strncpy(fname,dir->name, dir->name_len);
			
			if (!flag && (strcmp(fname, ".") == 0 || strcmp(fname, "..")==0)){
				//don't print 
			}else{
				printf("%s\n", fname);
				// printf("and inode for this dir is %i at blcok %i\n", dir->inode, dir_block);
			}
			
			cur_size += dir->rec_len;
			dir = (struct ext2_dir_entry_2 *)cur_size;
		}
		block++;
	}
}

/* Helper function to find the inode number of a file in the given inode.
* If it exist and is dir, return inode number.
* If it exist and is regular file, return 0.
* Else return -1
*/
int find_file_inode(struct ext2_inode *inode, char *file_name){

	unsigned int dir_block;

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
			
			if (strcmp(fname, file_name) == 0 && dir->file_type !=  EXT2_FT_REG_FILE){
				return dir->inode;
			}else if (strcmp(fname, file_name) == 0 && dir->file_type == EXT2_FT_REG_FILE){
				return 0;
			}
			
			cur_size += dir->rec_len;
			dir = (struct ext2_dir_entry_2 *)cur_size;
		}
		block++;
	}
	
	return -1;
}
/* Helper function to print the file */
void print_inode_file(char *file, int flag_a){
	if (flag_a){
		printf(".\n");
		printf("..\n");
		printf("%s\n", file);
	}else {
		printf("%s\n", file);
	}
}
/* Helper function to find the inode given the directory
*/
int find_inode_by_dir(char *dir, int flag_a){

	//return root node if dir is /
	if (strlen(dir) == 1){
		print_inode_dir(find_inode(2), flag_a);
		return 1;
	//trim if there is / in the path
	}else if (strlen(dir) > 1 && strcmp(&dir[strlen(dir) - 1], "/") == 0){
		dir[strlen(dir) - 1] = '\0';
	}

	char *token;
	char *last_token;
	struct ext2_inode *cur_inode;

	//look for inode for the dir
	int inode_num = 0;
	while ((token = strsep(&dir, "/"))) {
		last_token = token;
		
		if (strcmp(token, "") == 0){
			cur_inode = find_inode(2);
			find_file_inode(cur_inode, token);
		}else{
			inode_num = find_file_inode(cur_inode, token);
			
			//check path exist, else return null
			if (inode_num >0){
				cur_inode = find_inode(inode_num);
			}else if (inode_num == -1){
				cur_inode = NULL;
			}
			
		}
	}

	if (inode_num == -1){
      	return -1; 

    }else if (inode_num > 0){
    	print_inode_dir(cur_inode, flag_a);
    	return 1;

    }else if (inode_num == 0){
    	print_inode_file(last_token, flag_a);
    	return 1;
    }

	return 1;
} 

int path_exist(char *dir){

	//return root node if dir is /
	if (strlen(dir) == 1){
		return 1;

	//trim if there is / in the path
	}else if (strlen(dir) > 1 && strcmp(&dir[strlen(dir) - 1], "/") == 0){
		dir[strlen(dir) - 1] = '\0';
	}

	char *token;
	struct ext2_inode *cur_inode;

	//look for inode for the dir
	int inode_num = 0;
	while ((token = strsep(&dir, "/"))) {
		
		if (strcmp(token, "") == 0){
			cur_inode = find_inode(2);
			find_file_inode(cur_inode, token);
		}else{
			inode_num = find_file_inode(cur_inode, token);
			
			//check path exist, else return null
			if (inode_num >0){
				cur_inode = find_inode(inode_num);
			}else if (inode_num == -1){
				return -1;
			}
			
		}
	}

	return 1;
}