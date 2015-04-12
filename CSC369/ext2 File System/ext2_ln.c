#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

/* This program takes three command line arguments. The first is the name of an 
 * ext2 formatted virtual disk. The other two are absolute paths on your ext2 
 * formatted disk. The program should work like ln, creating a link from the 
 * first specified file to the second specified path. If the first location does 
 * not exist (ENOENT), if the second location already exists (EEXIST), or if 
 * either location refers to a directory (EISDIR), then your program should 
 * return the appropriate error. Note that this version of ln only works with 
 * files.
 */
int main(int argc, char **argv) {

    //Set up the structs by reading in necessary items from the img
	setUp(argc, argv, 4);

    //Do some path parsing here to get the name we're looking for to remove,
    //and the parent directory entry item
    int argvsize = strlen(argv[3]);

    lseek(fd, EXT2_BLOCK_SIZE * inode_table + 
        (EXT2_ROOT_INO - 1) * sizeof(struct ext2_inode), SEEK_SET);
    struct ext2_inode *test = malloc(sizeof(struct ext2_inode));
    read(fd, test, sizeof(struct ext2_inode));

    //Get the directory entry of the file we're going to link to
    struct ext2_dir_entry_2 *file_to_link = malloc(sizeof(struct ext2_dir_entry_2));
    file_to_link = full_walk(argv[2]);


    //If the file we want to link to doesn't exist, throw an error
    if (!file_to_link)
    {
        exit(ENOENT);
    }

    char *temp_file_path = malloc(strlen(argv[3]));
    strncpy(temp_file_path, argv[3], strlen(argv[3]));
    temp_file_path[strlen(argv[3])] = '\0';

    //Loop through to get the very last item in the path: this is the name
    //of our link
    char *linkingfile_name;
    char *namechunk = strtok(argv[3], "/");

    while (namechunk)
    {
        linkingfile_name = namechunk;
        namechunk = strtok(NULL, "/");
    }

    //Get the path to where we want to store our new link
    char *file_path = malloc(argvsize- strlen(linkingfile_name) + 1);
    strncpy(file_path, temp_file_path, argvsize - strlen(linkingfile_name));

    //Get the directory entry of the path we want to store our link in
    struct ext2_dir_entry_2 *parent_dir = malloc(sizeof(struct ext2_dir_entry_2));
    parent_dir = full_walk(file_path);

    //Get the inode of the parent directory that we're going to store our link in
    int parent_inode_num = parent_dir->inode;
    struct ext2_inode *parent_dir_inode = to_inode(parent_inode_num);

    //Check if the link name already exists in the dir we're supposed to put it
    //in: error if so (and if it's a directory)
    struct ext2_dir_entry_2 *linkingfile = malloc(sizeof(struct ext2_dir_entry_2));
    linkingfile = walk_one(parent_inode_num, linkingfile_name);

    if (linkingfile) {
        if (linkingfile->file_type == EXT2_FT_DIR)
            exit(EISDIR);
        else
            exit(EEXIST);
    }
    if (file_to_link->file_type == EXT2_FT_DIR)
        exit(EISDIR);

    //Now we know our new link name is valid, and the file we want to link to
    //exists! Set the dir entry linkingfile to have all of our link's 
    //necessary info
    linkingfile = malloc(sizeof(struct ext2_dir_entry_2));
    linkingfile->inode = file_to_link->inode;
    int linkingfile_size = sizeof(struct ext2_dir_entry_2) 
        + strlen(linkingfile_name);
    linkingfile_size += (4-(linkingfile_size % 4));
    linkingfile->name_len = strlen(linkingfile_name);
    linkingfile->file_type = EXT2_FT_REG_FILE;

    //Get the offset in the parent directory for where we should store our 
    //new link
    int off = empty_dir_spot(parent_dir_inode, linkingfile_size);

    //Store our new link in the parent directory
    new_dir_entry(off, linkingfile_size, linkingfile_name, linkingfile->file_type, 
        linkingfile->inode);

    //Get the inode table of the file we're linking to
    lseek(fd, EXT2_BLOCK_SIZE * inode_table + 
        ((parent_inode_num - 1) * sizeof(struct ext2_inode)), SEEK_SET);
    write(fd, parent_dir_inode,sizeof(struct ext2_inode));

    //Increase the links count of the inode of the file we're linking to, and
    //write the inode back to the img
    struct ext2_inode *filetolink_inode = to_inode(file_to_link->inode);
    filetolink_inode->i_links_count++;
    lseek(fd, EXT2_BLOCK_SIZE * inode_table + 
        ((file_to_link->inode - 1) * sizeof(struct ext2_inode)), SEEK_SET);
    write(fd, filetolink_inode,sizeof(struct ext2_inode));

    free(linkingfile);
    free(temp_file_path);
    free(parent_dir_inode);
    cleanUp();

	return 0;
}
