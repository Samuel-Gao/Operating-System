#include "ext2.h"
#include <errno.h>

struct ext2_inode *find_inode(int index);
void print_inode_dir(struct ext2_inode *inode, int flag);
struct ext2_inode *find_inode_by_dir(char *dir);