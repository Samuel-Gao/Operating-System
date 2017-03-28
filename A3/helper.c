
#include "helper.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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
			
			if (strcmp(fname, file_name) == 0 && dir->file_type){
				return dir->inode;
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

	char *token;
	char *last_token;
	
	while ((token = strsep(&file, "/"))) {
		last_token = token;
	}

	if (flag_a){
		printf(".\n");
		printf("..\n");
		printf("%s\n", last_token);
	}else {
		printf("%s\n", last_token);
	}
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
	// char *last_token;
	struct ext2_inode *cur_inode;

	//look for inode for the dir
	int inode_num = 0;
	char *dir_cpy = malloc(sizeof(dir));
	strcpy(dir_cpy, dir);

	while ((token = strsep(&dir_cpy, "/"))) {
		
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
				break;
			}
			
		}
	}
	return cur_inode;
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

/* Helper function to find an empty node and assign
*/
int alloc_inode(){

	// struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	struct ext2_super_block *sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	struct ext2_group_desc *gd;
	int c;

	//loop through bitmap to find an empty inode

	for (c=2; c< sb->s_blocks_count; c += sb->s_blocks_per_group){
		gd = (struct ext2_group_desc*)(disk + c * EXT2_BLOCK_SIZE);
		if (gd->bg_free_inodes_count >= 1){
			char *bitmap = (char *)disk + (gd->bg_inode_bitmap * EXT2_BLOCK_SIZE);

			int x,y;
			int bit_pos = -1;
			for (x=0; x < 4; x++){
				for (y=0; y<8; y++){
					if (((bitmap[x] >> y) & 1) == 0){
						bit_pos = 8 * x + y;
						break;
					}
				}
			}
			
			int inode_idx =  (c * sb->s_inodes_per_group) + bit_pos + 1;
			char *byte = bitmap + bit_pos / 8;
			*byte |= 1 << (bit_pos % 8);

			gd->bg_free_inodes_count--;
			return inode_idx;
		}
	}

	return -1;
}

/* Helper function to copy inode from src to dest. Does not handle
 * doubbly or tripply indirect pointer
 */

void copy_inode(struct ext2_inode *dest, struct ext2_inode *src){

	memcpy(dest, src, EXT2_BLOCK_SIZE);
	dest->i_links_count = 1;

	int i;
	//copy direct block 
	for(i=0; i < 12; i++){
		if (src->i_block[i] != 0){
			dest->i_block[i] = allocate_block();
			char *dest_block = (char *)(disk + dest->i_block[i] * EXT2_BLOCK_SIZE);
			 char *src_block = (char *)(disk + src->i_block[i] * EXT2_BLOCK_SIZE);
			memcpy(dest_block, src_block, EXT2_BLOCK_SIZE);

		}else{
			dest->i_block[i] = 0;
		}
	}

	//copy indirect block
	if (src->i_block[12] != 0){
		dest->i_block[12] = allocate_block();

		int i;

		unsigned int *src_block_pt = (unsigned int *)(disk + src->i_block[12] * EXT2_BLOCK_SIZE);
		unsigned int *dest_block_pt = (unsigned int *)(disk + dest->i_block[12] * EXT2_BLOCK_SIZE);

		int len = (EXT2_BLOCK_SIZE / sizeof (unsigned int));

		for(i = 0; i < len; i++) {
			if (src_block_pt[i] != 0) {
				dest_block_pt[i] = allocate_block();

				struct ext2_inode *src_inode = (struct ext2_inode *)disk + src_block_pt[i] * EXT2_BLOCK_SIZE;
				struct ext2_inode *dest_inode = (struct ext2_inode *)disk + dest_block_pt[i] * EXT2_BLOCK_SIZE;
				memcpy(dest_inode, src_inode, EXT2_BLOCK_SIZE);
			} else {
				dest_block_pt[i] = 0;
			}
		}
	}

}

/*Helper function to find an unused block */

int allocate_block() {
	//iterate through all block groups
	struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	int num_block = super_block->s_blocks_count;
	int cur_block;

	for (cur_block=0; cur_block < num_block; cur_block += super_block->s_blocks_per_group){
		struct ext2_group_desc * gd = (struct ext2_group_desc *)(disk + (cur_block + 2) * EXT2_BLOCK_SIZE);
		
		//alloct a block if there exist one free block
		if (gd->bg_free_blocks_count >= 1){
			char * bitmap = (char *)disk + (gd->bg_block_bitmap) * EXT2_BLOCK_SIZE;
			int x,y;
			
			//loop through bitmap to find an unused block
			int bit_pos = -1;
			for (x=0; x < 16; x++){
				for(y=0; y< 8; y++){
					if (((bitmap[x] >> y) & 1) == 0){
						bit_pos = 8 * x + y;
						break;
					}
				}
			}

			int block_idx = bit_pos + cur_block + super_block->s_first_data_block;

			char * byte = bitmap + bit_pos / 8;
			*byte |= 1 << (bit_pos % 8);
			
			gd->bg_free_blocks_count--;
			return block_idx;
		}
	}

	return 0;
}

/* Helper function to create a new dir entry */
struct ext2_dir_entry_2 * create_new_entry(int src_inode, char *file_name, unsigned char file_type){
	
	struct ext2_dir_entry_2 *new_entry = malloc(sizeof(struct ext2_dir_entry_2));
	new_entry->inode = src_inode;
	new_entry->name_len = strlen(file_name);
	new_entry->file_type = file_type;
	
	return new_entry;
}


void add_entry(struct ext2_inode * inode, struct ext2_dir_entry_2 *dir_entry, char * entry_name) {

	int dir_entry_size = sizeof(struct ext2_dir_entry_2);

	int i=0;
	//loop through blocks 
	for(; i < 12; i++){
		int block = inode->i_block[i];
		
		//if block is not in use, alloc an block and add entry there.
		if (block == 0){
			
			inode->i_block[i] = allocate_block();
			dir_entry->rec_len = EXT2_BLOCK_SIZE;

			memcpy(disk + inode->i_block[i] * EXT2_BLOCK_SIZE + dir_entry_size, entry_name, dir_entry->name_len);
			(*(struct ext2_dir_entry_2 *)(disk + inode->i_block[i] * EXT2_BLOCK_SIZE)) = *dir_entry;
			return;
		
		//if block is in use, check if there is space for new entry
		//if there is, re-calculate dir len and add entry 
		//if not, then continue to next block. 
		}else if (block > 0){
			struct ext2_dir_entry_2 *cur_entry = (struct ext2_dir_entry_2 *)disk + block * EXT2_BLOCK_SIZE;
			char *cur_dir = (char *)disk + block * EXT2_BLOCK_SIZE;
			char *end_dir = (char *)cur_dir + EXT2_BLOCK_SIZE;

			while (cur_dir < end_dir){

				//recalculate dir_length
				cur_entry = (struct ext2_dir_entry_2*)cur_dir;
				if (cur_dir + cur_entry->rec_len >= end_dir){
					
					int actual_size = dir_entry_size + cur_entry->name_len;
					char *size_after_adding_new_dir = (char *) cur_dir + actual_size + dir_entry->name_len + dir_entry_size;
					
					if (size_after_adding_new_dir < end_dir) {
						cur_entry->rec_len = actual_size;
						cur_dir += cur_entry->rec_len;
						break;
					}

				}
		
				cur_dir += cur_entry->rec_len;
			}

			//add entry if there is enough space
			if (cur_dir != end_dir) {
				dir_entry->rec_len = end_dir - cur_dir;
				(*(struct ext2_dir_entry_2 *)cur_dir) = *dir_entry;
				memcpy(cur_dir + dir_entry_size, entry_name, dir_entry->name_len);
				return;
			}
		}
	}
}

/* Helper function to get the last file name of the 
 * given dir
 */
char *get_last_dir(char *dir){
	char *token;
	char *last_token;
	
	char *dir_cpy = malloc(sizeof(dir));
	strcpy(dir_cpy, dir);

	while ((token = strsep(&dir_cpy, "/"))) {
		last_token = token;
	}
	return last_token;
}

void remove_file(char *dir){

	//set i_dttime
	struct ext2_inode *inode_to_remove = find_inode_by_dir(dir);
	inode_to_remove->i_dtime = (unsigned int)time(NULL);

	char *entry_to_remove = get_last_dir(dir);
	char parent [strlen(dir)+1];
	realpath(dir, parent);

	struct ext2_inode *parent_inode;
	
	//found the parent inode
	if (strcmp(parent, dir) == 0){
		parent_inode = find_inode(2);
	}else{
		parent_inode = find_inode_by_dir(parent);
	}

	//remove entry from directory;
	//first search on directed link
	int i;
	int block;
	for (i=0; i < 12; i++){
		block = parent_inode->i_block[i];
		if (block != 0){
			
			char *cur_dir = (char *)disk + block * EXT2_BLOCK_SIZE;
			char *end_dir = (char *)cur_dir + EXT2_BLOCK_SIZE;

			struct ext2_dir_entry_2 *cur_entry;
			struct ext2_dir_entry_2 *pre_entry = (struct ext2_dir_entry_2*)cur_dir;

			while (cur_dir < end_dir){
				//recalculate dir_length
				cur_entry = (struct ext2_dir_entry_2*)cur_dir;
				printf("file name %s\n", cur_entry->name);
				printf("trying to delete %s\n", entry_to_remove);

				char *fname = malloc(sizeof(cur_entry->name_len));
				strncpy(fname,cur_entry->name, cur_entry->name_len);
				
				if (strcmp(fname, entry_to_remove) == 0){
					pre_entry->rec_len += cur_entry->rec_len;
					printf("removed\n");
					return;
					
				}
				pre_entry = cur_entry;
				cur_dir += cur_entry->rec_len;
			}

		}else{
			break;
		}
	}

	//if not in direct link, search indirect link
}



//note to myslef:
//1) Remember to update inode metatdata, link_count, i_block, 