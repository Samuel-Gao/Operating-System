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

int find_inode_number(unsigned char *disk, char *path){
	int result = -1;
	if (strlen(path) == 1 && path[0] == '/')
		return 2;

	struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + 2048);
	unsigned int inode_start_id = group_desc->bg_inode_table;
	unsigned char *inode_start_addr = disk + (EXT2_BLOCK_SIZE * inode_start_id);
	struct ext2_inode *cur_inode = (struct ext2_inode *)(inode_start_addr + (128 * 1));
	char *cur_folder; // folder name to look for
	cur_folder = malloc(255);
	int cursor = 1;
	if (path[0] != '/') return -1;
	while (cursor < strlen(path)){ // in each iteration cur_inode will go 1 step deeper.

		// update *cur_folder
		int k = 0;
		while (path[cursor] != '/' && cursor < strlen(path))
			cur_folder[k++] = path[cursor++];
		cur_folder[k] = 0;
		cursor++;

		int found = 0;
		for (int i=0;i<12 && cur_inode->i_block[i] && !found;i++){ // look at all blocks cur_inode has

			int block_no = cur_inode->i_block[i];
			struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)( disk + (EXT2_BLOCK_SIZE * block_no) );

            int sum = 0;
		    while (sum < EXT2_BLOCK_SIZE) { // go through "dir_entry"s in this block
		        sum += dir->rec_len;

		        char *copy = malloc(255); // copy of directory name
		        strncpy(copy, dir->name, dir->name_len);
		        copy[dir->name_len] = 0;

		        if (strlen(copy) == strlen(cur_folder))
			        if (strncmp(copy, cur_folder, strlen(copy)) == 0){
			        	if (dir->file_type == 1){
			             if (cursor >= strlen(path)){
                                return dir->inode;
                            }
                            else {
                                return -1;
                            }
			             }
			             else if (dir->file_type == 2){
			            	// update cur_folder = next in *path  <-- break for this
			            	// update cur_inode = dir->inode
							cur_inode =(struct ext2_inode *)(inode_start_addr + (128 * (dir->inode -1)));
							result = dir->inode;
							found = 1;
							break;
						}
					}

		        // iterate to the next directory
		        char *ptr = (char *)dir;
		        ptr += dir->rec_len;
		        dir = (struct ext2_dir_entry_2 *) ptr;
		    }

		}
		if (!found)
			return -1;
	}
	return result;
}

int find_free_block(unsigned int *block_bitmap){
    unsigned f = 1;
    int block_index = 0;
    for (int jj=0; jj < 4; jj++){ // 4 block bitmaps
        for (int j = 0; j < 32; j++) { // 32 bits in each block bitmap
            block_index++;
            if ((*block_bitmap & f) == 0){ // block number - "block_index" is free
                *block_bitmap |= f;
                return block_index;
            }
            f = f << 1;
        }
        f = 1;
        block_bitmap++;
    }
    return -1;
}

int find_free_inode(unsigned int *inode_bitmap){
    int f = 1;
    for (int i=0;i<32;i++){
        if ((*inode_bitmap & f) == 0){
            *inode_bitmap |= f;
            return i+1;
        }
        f = f << 1;
    }
    return -1;
}

//  /ext2_mkdir  ///////////////////////////////////////

void set_bitmap_zero(unsigned int * bitmap, int index) {
  bitmap += (index / 32);
  unsigned int f = ~(1 << (index%32));
  *bitmap &= f;
}

int ext2_mkdir(char *path){

    // some pointers
    struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + 2048);
    unsigned int inode_start_id = group_desc->bg_inode_table;
    unsigned char *inode_start_addr = disk + (EXT2_BLOCK_SIZE * inode_start_id);

    //printf("=============================> %s  inodenumber= %d\n", path, find_inode_number(disk, path));
    if (find_inode_number(disk, path) != -1){
        printf("EEXIST\n");
        return 17;
    }

    char *location = malloc(255); //     <=== 255 chars for pathname?
    char *dir_name = malloc(255);
    int cur = strlen(path)-1;
    if (path[cur] == '/') cur--;
    while (path[cur] != '/')
        cur--;

    int N=0;
    for (int i=cur+1;i<strlen(path) && path[i] != '/';i++)
        dir_name[N++] = path[i];
    dir_name[N] = 0;

    strncpy(location, path, cur);
    location[cur] = 0;

    //printf("=============================> %s  inodenumber= %d\n", location, find_inode_number(disk, location));
    if (location[0] == 0)
        location[0] = '/',
        location[1] = 0;
    if (find_inode_number(disk, location) == -1){
        printf("ENOENT\n");
        return 2;
    }

    int location_inode_num = find_inode_number(disk, location);

    int sum = 8 + strlen(dir_name);
    unsigned short  len_needed = ( (sum/4) +(sum%4!=0) ) *4;

    // get a new inode here
    unsigned int *inode_bitmap = (unsigned int *)(disk + (1024*(group_desc->bg_inode_bitmap)));
    unsigned int inode_index = find_free_inode(inode_bitmap); // index of the created folder inode
    if (inode_index == -1) {printf("Can't find a free inode\n"); return -1;}
    struct ext2_inode *new_inode = (struct ext2_inode *) (inode_start_addr + (128 * (inode_index-1)));
    new_inode->i_mode = EXT2_S_IFDIR;
    //new_inode->i_size = 1024;
    new_inode->i_links_count = 1;
    int inode_block = find_free_block((unsigned int *)(disk + (1024*group_desc->bg_block_bitmap)));
    if (inode_block == -1) {printf("Can't find a free block\n"); return -1; }
    new_inode->i_block[0] = inode_block;
    new_inode->i_blocks = 2;
    group_desc->bg_free_blocks_count -= 2;
    group_desc->bg_free_inodes_count -= 1;
    group_desc->bg_used_dirs_count += 1;
    for (int i=1;i<15;i++)
        new_inode->i_block[i] = 0;
    struct ext2_dir_entry_2 *inode_dirs = (struct ext2_dir_entry_2 *) (disk + (EXT2_BLOCK_SIZE * inode_block));
    // add folder => .
    inode_dirs->inode = inode_index;
    inode_dirs->rec_len = 12;
    inode_dirs->name_len = 1;
    inode_dirs->file_type = 2;
    inode_dirs->name[0] = '.';
    inode_dirs->name[1] = 0;
    //next entry;
    char *ptr = (char *)inode_dirs;
    ptr += inode_dirs->rec_len;
    inode_dirs = (struct ext2_dir_entry_2 *) ptr;
    //add folder => ..
    inode_dirs->inode = location_inode_num;
    inode_dirs->rec_len = 1012; // 1024-12
    inode_dirs->name_len = 2;
    inode_dirs->file_type = 2;
    inode_dirs->name[0] = '.';
    inode_dirs->name[1] = '.';
    inode_dirs->name[2] = 0;

    // go to location_inode
    struct ext2_inode *location_inode = (struct ext2_inode *)(inode_start_addr + (128 * (location_inode_num-1)));
    int found = 0;
    for (int i=0;i<12 && !found;i++){ // go through blocks of location_inode to find a space for ext2_dir_entry_2
        if (location_inode->i_block[i] == 0){ // an uninitialized block   <---  need a new block
            location_inode->i_blocks++;
            unsigned int *block_bitmap = (unsigned int *)(disk + (1024*group_desc->bg_block_bitmap));
            int block_index = find_free_block(block_bitmap);
            if (block_index == -1) return -1;
            struct ext2_dir_entry_2 *new_entry;
            new_entry = (struct ext2_dir_entry_2 *) (disk + (EXT2_BLOCK_SIZE * block_index));
            new_entry->inode = inode_index; // = ?????
            new_entry->rec_len = 1024;
            new_entry->name_len = strlen(dir_name);
            new_entry->file_type = 2;

            //new_entry->name = malloc(strlen(dir_name));                              // necessary?
            strncpy(new_entry->name, dir_name, strlen(dir_name));
            new_entry->name[strlen(dir_name)] = 0;  //                                  not sure
            found = 1;
        }
        else{ // a block with entries
            struct ext2_dir_entry_2 *cur_entry = (struct ext2_dir_entry_2 *)(disk+(location_inode->i_block[i]*EXT2_BLOCK_SIZE));
            sum = 0;
            while (sum < EXT2_BLOCK_SIZE){
                sum += cur_entry->rec_len;
                unsigned short actual_len = 8 + strlen(cur_entry->name);
                actual_len = (actual_len/4 + (actual_len%4!=0)) *4;
                unsigned short remaining = (cur_entry->rec_len) - actual_len; // unused space in this ext2_dir_entry_2
                if (remaining >= len_needed) { // it is not a valid ext2_dir_entry_2
                    found = 1;
                    cur_entry->rec_len = actual_len;
                    struct ext2_dir_entry_2 *new_entry = cur_entry + cur_entry->rec_len;
                    // go to next
                    char *ptr = (char *)cur_entry;
                    ptr += cur_entry->rec_len;
                    new_entry = (struct ext2_dir_entry_2 *) ptr;

                    new_entry->rec_len = remaining;
                    //new_entry->name = malloc(strlen(dir_name));                              // necessary?
                    strncpy(new_entry->name, dir_name, strlen(dir_name));
                    new_entry->name[strlen(dir_name)] = 0;               //                     not sure
                    new_entry->name_len = strlen(dir_name);
                    new_entry->file_type = 2;
                    new_entry->inode = inode_index; //  = ??????????
                    break;
                }

                // iterate to the next directory entry
                char *ptr = (char *)cur_entry;
                ptr += cur_entry->rec_len;
                cur_entry = (struct ext2_dir_entry_2 *) ptr;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
	char *vir_disk = argv[1];
	char *path = argv[2];

	int fd = open(vir_disk, O_RDWR);
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
    }

    return ext2_mkdir(path);
}