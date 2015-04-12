#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

int main(int argc, char **argv) {

	setUp(argc, argv, 3);

    char *temp_path = malloc(strlen(argv[2]));
    strncpy(temp_path, argv[2], strlen(argv[2]));
    temp_path[strlen(argv[2])] = '\0';

    //Case where they enter the root as the path: it exists already, quit!
    if (!(strncmp(temp_path, "/", strlen(temp_path))))
    {
        exit(EEXIST);
    }

    //Get the name of the dir to be allocated (at the very end of the path)
    char *newdir_name;
    char *namechunk = strtok(argv[2], "/");

    while(namechunk)
    {
        newdir_name = namechunk;
        namechunk = strtok(NULL, "/");
    }

    //Get the rest of the path and walk through it
    char *file_path = malloc(strlen(temp_path) - strlen(newdir_name) + 1);
    strncpy(file_path, temp_path, (strlen(temp_path) - strlen(newdir_name)));

    struct ext2_dir_entry_2 *parent_dir = malloc(sizeof(struct ext2_dir_entry_2));

    //If we couldn't find the parent directory right before the 
    // dir we're supposed to allocate, error!
    parent_dir = full_walk(file_path);
    if (!parent_dir) {
        exit(ENOENT);
    }

    //Check to see if the current dir already exists: if it does, throw an error
    struct ext2_dir_entry_2 *check_dupl = malloc(sizeof(struct ext2_dir_entry_2));
    check_dupl = walk_one(parent_dir->inode, newdir_name);

    if(check_dupl)
    {
        exit(EEXIST);
    }

    //Now we know there's no more errors to throw: proceed!

    int pdir_inode = parent_dir->inode;

    struct ext2_inode *parent_dir_inode = to_inode(pdir_inode);

    struct ext2_inode *new_inode = malloc(sizeof(struct ext2_inode));
    // Set link count to 2 for . and ..
    new_inode->i_links_count = 2;       
    new_inode->i_size = 0;
    new_inode->i_blocks = 0;
    new_inode->i_mode = EXT2_S_IFDIR;

    int newdir_inode = reserve('i');

    int off = empty_dir_spot(new_inode, EXT2_DOT_DIR_SIZE);
    new_dir_entry(off, EXT2_DOT_DIR_SIZE, ".", EXT2_FT_DIR, newdir_inode);
    off = empty_dir_spot(new_inode, EXT2_PAR_DIR_SIZE);
    new_dir_entry(off, EXT2_PAR_DIR_SIZE, "..", EXT2_FT_DIR, pdir_inode);
    int new_dirsize = sizeof(struct ext2_dir_entry_2) + strlen(newdir_name);
    
    //rounding size to a multiple of 4
    new_dirsize += (4-(new_dirsize % 4));

    off = empty_dir_spot(parent_dir_inode, new_dirsize);
    new_dir_entry(off, new_dirsize, newdir_name, EXT2_FT_DIR, newdir_inode);

    parent_dir_inode->i_links_count++;
    lseek(fd, EXT2_BLOCK_SIZE * inode_table + ((pdir_inode - 1) *
        sizeof(struct ext2_inode)), SEEK_SET);
    write(fd, parent_dir_inode, sizeof(struct ext2_inode));

    lseek(fd, EXT2_BLOCK_SIZE * inode_table + 
        (newdir_inode - 1) * sizeof(struct ext2_inode), SEEK_SET);
    write(fd, new_inode, sizeof(struct ext2_inode));

    free(new_inode);
    //free(parent_dir);
    free(parent_dir_inode);
    free(temp_path);
    free(file_path);

    cleanUp();

	return 0;
}