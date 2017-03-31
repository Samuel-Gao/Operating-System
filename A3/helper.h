#include "ext2.h"
#include <errno.h>


struct ext2_inode *find_inode(int index);
void print_inode_dir(struct ext2_inode *inode, int flag);
struct ext2_inode *find_inode_by_dir(char *dir);
void print_inode_file(char *file, int flag_a);
int path_exist(char *dir);

int alloc_inode();
int allocate_block();
char *get_last_dir(char *dir);
struct ext2_dir_entry_2 * create_new_entry(int src_inode, char *file_name, unsigned char file_type);
void add_entry(struct ext2_inode * inode, int inode_num, char * entry_name, unsigned char file_type);

void remove_file(char *dir);
void remove_dir(char* dir);
void create_hard_link(struct ext2_inode *src, struct ext2_inode *target, char *file_name);
void create_symbolic_link(struct ext2_inode *src, struct ext2_inode *dest_path,char *old_fname, char *file_name);
int find_inode_idx(struct ext2_inode *inode);
