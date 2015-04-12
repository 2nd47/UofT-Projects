#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

/*
 * This program takes two command line arguments. The first is the name of an ext2 
 * formatted virtual disk, and the second is an absolute path to a file or link 
 * (not a directory) on that disk. The program should work like rm, removing the 
 * specified file from the disk. If the file does not exist or if it is a 
 * directory, then your program should return the appropriate error.
*/

int main(int argc, char **argv) {

    //Set up the structs by reading in necessary items from the img
	setUp(argc, argv, 3);

    //Do some path parsing here to get the name we're looking for to remove,
    //and the parent directory entry item
    int argvsize = strlen(argv[2]);
    char *temp_file_path = malloc(strlen(argv[2]));
    strncpy(temp_file_path, argv[2], strlen(argv[2]));
    temp_file_path[strlen(argv[2])] = '\0';

    //Loop through to get the very last item in the path: this is the name
    //of the file we'll remove
    char *file_to_rm_name;
    char *namechunk = strtok(argv[2], "/");

    while (namechunk)
    {
        file_to_rm_name = namechunk;
        namechunk = strtok(NULL, "/");
    }

    //parent_path will contain the path of everything up until the file we
    //want to remove
    char *parent_path = malloc(argvsize - strlen(file_to_rm_name) + 1);
    strncpy(parent_path, temp_file_path, (argvsize - strlen(file_to_rm_name)));

    //get the directory entry of the parent directory
    struct ext2_dir_entry_2 *parent_dir = malloc(sizeof(struct ext2_dir_entry_2));
    parent_dir = full_walk(parent_path);

    //path walk and return an error if the file doesn't exist
    struct ext2_dir_entry_2* file_to_rm = walk_one(parent_dir->inode,
        file_to_rm_name);
    if (!file_to_rm)
    {
        exit(ENOENT);
    }

    //check if type is directory: error if so
    if(file_to_rm->file_type == EXT2_FT_DIR)
    {
        exit(EISDIR);
    }

    //get the parent dir & inode of the file we want to delete so we can modify 
    //its entries
    int parent_inode_num = parent_dir->inode;
    struct ext2_inode *parent_dir_inode = to_inode(parent_inode_num);

    //delete the file from the parent dir block's entries
    dir_rm_spot(parent_dir_inode, file_to_rm_name, file_to_rm->rec_len);

    //decrease the file we want to delete's node links count
        struct ext2_inode *rm_file_inode = to_inode(file_to_rm->inode);
        rm_file_inode->i_links_count--;

        //if node links count < 1, delete the file's inode & blocks too, since
        //it's not being linked to by anything anymore
        if (rm_file_inode->i_links_count < 1)
        {
            //clear_all will clear all blocks allocated for this file
            clear_all(rm_file_inode);

            //clear_inode will clear the inode for the file
            clear_inode(file_to_rm->inode);
        }
        //Otherwise just write the inode struct with the updated links count
        //back into the img
        else
        {
            lseek(fd, EXT2_BLOCK_SIZE* inode_table + ((file_to_rm->inode-1) 
                * sizeof(struct ext2_inode)), SEEK_SET);    
            write(fd, rm_file_inode, sizeof(struct ext2_inode));            
        }

    cleanUp();

	return 0;
}