#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

/* This program takes three command line arguments. The first is 
 * the name of an ext2 formatted virtual disk. The second is the 
 * path to a file on your native operating system, and the third 
 * is an absolute path on your ext2 formatted disk. The program 
 * should work like cp, copying the file on your native file 
 * system onto the specified location on the disk. If the 
 * specified file or target location does not exist, then your 
 * program should return the appropriate error (ENOENT). 
 */

int main(int argc, char **argv) {

	setUp(argc, argv, 4);

    //Do a walk to make sure that the specified directory exists, if not,
    //return an error
    struct ext2_dir_entry_2* dir_to_store = full_walk(argv[3]);

    if (!dir_to_store)
    {
    	exit(ENOENT);
    }

	//fd native will be the fd of the file on our native OS
	int fd_native;
    if ((fd_native = open(argv[2], O_RDONLY)) == -1)
    {
    	exit(ENOENT);
    }

    char *native_name;
    char *namechunk = strtok(argv[2], "/");
    while(namechunk)
    {
        native_name = namechunk;
        namechunk = strtok(NULL, "/");
    }

    int native_dirsize = sizeof(struct ext2_dir_entry_2) + strlen(native_name);
    //rounding size to a multiple of 4
    native_dirsize += (4-(native_dirsize % 4));

    // We already check if there's more to be read in the read() call
	// int fsize = lseek(fd_native, 0, SEEK_END);
	lseek(fd_native, 0, SEEK_SET);

 	//New File's iNode
	struct ext2_inode *new_inode;
 	new_inode = malloc(sizeof(struct ext2_inode));
 	new_inode->i_links_count = 1;		
 	new_inode->i_size = 0;
    new_inode->i_blocks = 0;
    new_inode->i_mode = EXT2_S_IFREG;
 	
 	//The amount we read in from the native file
 	int amt_read;
 	char buf[EXT2_BLOCK_SIZE];

    // Allocate blocks for file data as it comes in block-sized chunks
    while ((amt_read = read(fd_native, buf, EXT2_BLOCK_SIZE)) > 0)
        write_file_block(new_inode, buf, amt_read);

    // Save the inode struct we've been working with onto the image
    int inode_num = init_inode(new_inode);

    //Now we have read in our new file into its inode and blocks, we need to
    //put the file info into the chosen directory
	
	//get the inode of the dir
  	struct ext2_inode *dir_to_store_inode = to_inode(dir_to_store->inode);

    // Find a spot for the inode in a block within the directory
	int off = empty_dir_spot(dir_to_store_inode, native_dirsize);
	new_dir_entry(off, native_dirsize, native_name, EXT2_FT_REG_FILE, inode_num);

    // Allocate the inode onto disk to represent any potential changes to the directory
    lseek(fd, EXT2_BLOCK_SIZE * inode_table + (((dir_to_store->inode)-1) 
        * sizeof(struct ext2_inode)), SEEK_SET);
    write(fd, dir_to_store_inode, sizeof(struct ext2_inode));

    cleanUp();

    free(dir_to_store_inode);

	return 0;
}
