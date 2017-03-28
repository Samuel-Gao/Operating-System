#include "ext2.h"
#include <errno.h>


struct ext2_inode *find_inode(int index);
void print_inode_dir(struct ext2_inode *inode, int flag);
struct ext2_inode *find_inode_by_dir(char *dir);
void print_inode_file(char *file, int flag_a);
int path_exist(char *dir);

int alloc_inode();
void copy_inode(struct ext2_inode *, struct ext2_inode*);
int allocate_block();
char *get_last_dir(char *dir);
struct ext2_dir_entry_2 * create_new_entry(int src_inode, char *file_name, unsigned char file_type);
void add_entry(struct ext2_inode * inode, struct ext2_dir_entry_2 *dir, char * entry_name);

void remove_file(char *dir);