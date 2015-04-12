#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

/* This program takes two command line arguments. The first is the name of an 
 * ext2 formatted virtual disk. The second is an absolute path on the ext2 
 * formatted disk. The program should work like ls -1, printing each 
 * directory entry on a separate line. Unlike ls -1, it should also print 
 * . and ... In other words, it will print one line for every directory entry 
 * in the directory specified by the absolute path. If the directory does not 
 * exist print "No such file or diretory".
 */
int main(int argc, char **argv) {

    setUp(argc, argv, 3);
    
    // Walk to the directory to get information from it
    struct ext2_dir_entry_2* my_dir = full_walk(argv[2]);

    if (!my_dir)
    {
        printf("No such file or diretory\n");
        return 0;
    }

    int my_dir_inode = my_dir->inode;

    // Get the inode the directory we need to write out to STDIN
    struct ext2_inode *inode = to_inode(my_dir_inode);

    struct ext2_dir_entry_2* entry;
    unsigned char block[EXT2_BLOCK_SIZE];
    unsigned int size;

    //Go through all the root's iblocks and check what's in them: 
    // if we find what we're looking
    //for, we're good 
    int j;
    for(j=0; j < 15; j++)
    {
        // As we iterate, read block by block
        lseek(fd, EXT2_BLOCK_SIZE*(inode->i_block[j]), SEEK_SET);
        read(fd, block, EXT2_BLOCK_SIZE);

        size = 0;
        entry = (struct ext2_dir_entry_2*) block;
        while((size < inode->i_size) && (entry->rec_len > 0)) 
        {
            // For each block, iterate across every directory entry and 
            // retrieve its name
            char file_name[entry->name_len+1];
            memcpy(file_name, entry->name, entry->name_len);
            // Append null character to print on screen
            file_name[entry->name_len] = '\0';

            printf("%s\n", file_name);

            // Move to next entry
            entry = (void*) entry + entry->rec_len;
            size += entry->rec_len;
        }
    }
    return 0;
}
