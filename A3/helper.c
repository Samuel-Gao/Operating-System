
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

/* Helper function to find the inode index given 
 * the inode structure.
 */

int find_inode_idx(struct ext2_inode *inode){
	if (inode != NULL){
		int i = 2;
		while(1){
			if (find_inode(i) == inode){
				return i;
			} 
			i++;
		}
	}

	return -1;
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

	struct ext2_super_block *sb = (struct ext2_super_block *) (disk + 1024);
	struct ext2_group_desc * gd = (struct ext2_group_desc *)(disk  + 2 * EXT2_BLOCK_SIZE);
	unsigned int *inode_bitmap = (unsigned int *)(disk + (1024*gd->bg_block_bitmap));
	
	int f = 1;
    for (int i=0;i<32;i++){
        if ((*inode_bitmap & f) == 0){
            *inode_bitmap |= f;

            //update inode metatdat
            struct ext2_inode *inode = find_inode(i+1);
            inode->i_links_count = 1;
		    inode->i_ctime = (unsigned int)time(NULL);
		    inode->i_blocks = 2;
		    inode->i_dtime = 0;
		    
		    //update super block
		    sb->s_free_inodes_count--;
            return i+1;
        }
        f = f << 1;
    }
    return -1;
}

/*Helper function to find an unused block */

int allocate_block() {
	
	struct ext2_super_block *sb = (struct ext2_super_block *) (disk + 1024);
	struct ext2_group_desc * gd = (struct ext2_group_desc *)(disk  + 2 * EXT2_BLOCK_SIZE);
	unsigned int *block_bitmap = (unsigned int *)(disk + (1024*gd->bg_block_bitmap));
	
	unsigned f = 1;
    int block_index = 0;
    for (int jj=0; jj < 4; jj++){ // 4 block bitmaps
        for (int j = 0; j < 32; j++) { // 32 bits in each block bitmap
            block_index++;
            if ((*block_bitmap & f) == 0){ // block number - "block_index" is free
                *block_bitmap |= f;
                
                 sb->s_free_blocks_count--;
                return block_index;
            }
            f = f << 1;
        }
        f = 1;
        block_bitmap++;
    }
    return -1;
}

/* Helper function to create a new dir entry */
struct ext2_dir_entry_2 * create_new_entry(int src_inode, char *file_name, unsigned char file_type){
	
	struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + 2 * EXT2_BLOCK_SIZE);
	struct ext2_dir_entry_2 *new_entry = malloc(sizeof(struct ext2_dir_entry_2));
	
	new_entry->inode = src_inode;
	new_entry->name_len = strlen(file_name);
	new_entry->file_type = file_type;
	strncpy(new_entry->name, file_name, strlen(file_name));

	
	gd->bg_used_dirs_count++;
	return new_entry;
}


void add_entry(struct ext2_inode * inode, int inode_num, char * entry_name, unsigned char file_type) {

	int i=0;
	//loop through blocks 
	for(; i < 12; i++){
		int block = inode->i_block[i];
		//if block is not in use, alloc an block and add entry there.
		if (block == 0){
			inode->i_block[i] = allocate_block();
			inode->i_blocks += 2;
			
			struct ext2_dir_entry_2 * new_entry = (struct ext2_dir_entry_2 *) disk + block * EXT2_BLOCK_SIZE;
			
			new_entry->rec_len = EXT2_BLOCK_SIZE;
			strncpy(new_entry->name, entry_name, strlen(entry_name));
            new_entry->name[strlen(entry_name)] = 0;               //                     not sure
            new_entry->name_len = strlen(entry_name);
            new_entry->file_type = file_type;
            new_entry->inode = inode_num;
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
					
					int actual_size = 8 + strlen(cur_entry->name);
					actual_size = (actual_size/4 + (actual_size%4!=0)) *4;
					int remain = cur_entry->rec_len - actual_size;
					
					int sum = 8 + strlen(entry_name);
    				int len_needed = ( (sum/4) +(sum%4!=0) ) *4;
					
					if (remain > len_needed){
						cur_entry->rec_len = actual_size;
						cur_dir += cur_entry->rec_len;
						struct ext2_dir_entry_2 * new_entry = (struct ext2_dir_entry_2 *) cur_dir;

						new_entry->rec_len = remain;
	                    //new_entry->name = malloc(strlen(dir_name));                              // necessary?
	                    strncpy(new_entry->name, entry_name, strlen(entry_name));
	                    new_entry->name[strlen(entry_name)] = 0;               //                     not sure
	                    new_entry->name_len = strlen(entry_name);
	                    new_entry->file_type = file_type;
	                    new_entry->inode = inode_num;
						return;
					}

				}
		
				cur_dir += cur_entry->rec_len;
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

/* Helper function to set given bitmap at index to zero
*/

void set_bitmap_zero(unsigned int * bitmap, int index) {
  bitmap += (index / 32);
  unsigned int f = ~(1 << (index%32));
  *bitmap &= f;
}

/* Helper function to set inode & block bitmap to zero for given inode
*/

void set_bitmap_zero_for_this_inode(int inode_num){
	struct ext2_group_desc * gd = (struct ext2_group_desc *)(disk + 2048);
	struct ext2_inode *inode = find_inode(inode_num);

	unsigned int *inode_bitmap = (unsigned int *)(disk + (1024 * gd->bg_inode_bitmap));
	unsigned int *block_bitmap = (unsigned int *)(disk + (1024 * gd->bg_block_bitmap));

	set_bitmap_zero(inode_bitmap, inode_num);
	
	int i;
	for (i=0; i<15; i++){
		if (inode->i_block[i] == 0){
			return;
		}else if (inode->i_block[i] != 0){
			set_bitmap_zero(block_bitmap, inode->i_block[i]);
		}
	}

}

void remove_file(char *dir){

	//set i_dttime
	struct ext2_inode *inode_to_remove = find_inode_by_dir(dir);
	inode_to_remove->i_dtime = (unsigned int)time(NULL);
	inode_to_remove->i_links_count--;

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

				char *fname = malloc(sizeof(cur_entry->name_len));
				strncpy(fname,cur_entry->name, cur_entry->name_len);
				
				if (strcmp(fname, entry_to_remove) == 0){
					// Set bitmap of block and inode to zero
					set_bitmap_zero_for_this_inode(cur_entry->inode);
					pre_entry->rec_len += cur_entry->rec_len;
					return;
					
				}
				pre_entry = cur_entry;
				cur_dir += cur_entry->rec_len;
			}

		}else{
			break;
		}
	}
}

/* Helper function to create a hard link of src at target. 
 */
void create_hard_link(struct ext2_inode *src, struct ext2_inode *target, char *file_name){
	int inode_idx = find_inode_idx(src);
	add_entry(target, inode_idx, file_name, EXT2_FT_REG_FILE);
}
//note to myslef:
//1) Remember to update inode metatdata, link_count, i_block, 