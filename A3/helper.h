#include "ext2.h"
#include <errno.h>

struct ext2_inode *find_inode(int index);
void print_inode_dir(struct ext2_inode *inode, int flag);
int find_inode_by_dir(char *dir, int flag_a);
void print_inode_file(char *file, int flag_a);
int path_exist(char *dir);