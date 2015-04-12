#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

/* our makefile also includes a custom test suite to determine validity
 */

void setUp(int argc, char **argv, int req_amt)
{
    // Code from exercises
    if(argc != req_amt) {
        fprintf(stderr, "Usage: readimg <image file name>\n");
        exit(1);
    }
    fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * EXT2_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    superblock = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

    // Get the block holding the inode table from the group descriptor and store in inode_table
    group_descr = malloc(sizeof(struct ext2_group_desc));
    lseek(fd, EXT2_BLOCK_SIZE * 2, SEEK_SET);
    read(fd, group_descr, sizeof(struct ext2_group_desc));  

    // Get the block bitmap to alter in the program
    block_bitmap = malloc(EXT2_BLOCK_SIZE);
    lseek(fd, EXT2_BLOCK_SIZE *group_descr->bg_block_bitmap, SEEK_SET);
    read(fd, block_bitmap, EXT2_BLOCK_SIZE);

    // Get the inode bitmap to alter in the program
    node_bitmap = malloc(EXT2_BLOCK_SIZE);
    //Go 4 times since 8*4 = 32, which is the number of inodes we have!
    lseek(fd, EXT2_BLOCK_SIZE * group_descr->bg_inode_bitmap, SEEK_SET);
    read(fd, node_bitmap, EXT2_BLOCK_SIZE);
    
    inode_table = group_descr->bg_inode_table;
}

// Write the altered bitmaps back to the img file before returning
void cleanUp() {
    lseek(fd, EXT2_BLOCK_SIZE, SEEK_SET);
    write(fd, superblock, EXT2_BLOCK_SIZE);
    lseek(fd, EXT2_BLOCK_SIZE * 2, SEEK_SET);
    write(fd, group_descr, sizeof(struct ext2_group_desc));
    lseek(fd, EXT2_BLOCK_SIZE *group_descr->bg_block_bitmap, SEEK_SET);
    write(fd, block_bitmap, EXT2_BLOCK_SIZE);
    lseek(fd, EXT2_BLOCK_SIZE *group_descr->bg_inode_bitmap, SEEK_SET);
    write(fd, node_bitmap, EXT2_BLOCK_SIZE);
}

/* Initialize an inode with initial properties
 * Return block number it was written to
 */
int init_inode(struct ext2_inode *inode) {
    // Malloc an inode to work with that will be written back to disk
    unsigned int blockNum = reserve('i');
    lseek(fd, EXT2_BLOCK_SIZE * inode_table + 
        (blockNum - 1) * sizeof(struct ext2_inode), SEEK_SET);
    write(fd, inode, sizeof(struct ext2_inode));
    return blockNum;
}

/* Allocate block to given indirection block. 
 * Create the indirection block if it doesn't exist
 * Return the indirection block
 */
int allocate_to_ind_block(int indBlock, int blockNum) {
    // If it doesn't exist, reserve a spot for it now and let's use it
    if (!indBlock)
        indBlock = reserve('b');
    lseek(fd, EXT2_BLOCK_SIZE * indBlock, SEEK_SET);
    int i = 0;
    unsigned int currBlock;
    // Iterate to find an empty spot on the block to place our block number
    while (read(fd, &currBlock, sizeof(unsigned int)) > 0)
        if (!currBlock)
            break;
        else
            i++;
    // We passed over the spot with the read, iterate back and write to it.
    lseek(fd, ((EXT2_BLOCK_SIZE * indBlock) + (i * sizeof(unsigned int))), SEEK_SET);
    write(fd, &blockNum, sizeof(unsigned int));
    return indBlock;
}

/* Write file data to an empty block (not provided)
 * Return block number written to
 */
int write_file_block(struct ext2_inode *inode, char buf[EXT2_BLOCK_SIZE], int amt_read) {
    // Reserve a block to write to
    unsigned int blockNum = reserve('b');
    int freeBlock;
    for(freeBlock = 0; inode->i_block[freeBlock] != 0; freeBlock++);
        // Check if we're dealing with an indirect block; allocate block 
        // number there if need to
        if (freeBlock > 11)
            inode->i_block[freeBlock] = allocate_to_ind_block(
                inode->i_block[12], blockNum);
        // Otherwise alter the inode itself
        else
            inode->i_block[freeBlock] = blockNum;
    // Write to the inode and record its changes
    lseek(fd, EXT2_BLOCK_SIZE * blockNum, SEEK_SET);
    write(fd, buf, amt_read);
    inode->i_size += amt_read;
    inode->i_blocks++;
    return blockNum;
}

// Move through a given path, checking it for validity
struct ext2_dir_entry_2* full_walk(char *full_path)
{
    //file_to_find stores current element we're looking for in the path
    char* file_to_find = strtok(full_path, "/");
    //This means that they entered the root as their dir in the path, we need to
    //give it something
    if (!file_to_find)
    {
        file_to_find = ".";
    }
    struct ext2_dir_entry_2* next_dir = walk_one(EXT2_ROOT_INO, file_to_find);

    file_to_find = strtok(NULL, "/");
    // Check across all files/directories within a given path to see if they're valid
    while (file_to_find)
    {
        // Check a single file/directory for validity and move into it
        next_dir = walk_one(next_dir->inode, file_to_find);
        if (!next_dir)
        {
            return NULL;
        }
        file_to_find = strtok(NULL, "/");
    }

    return next_dir;
}

// Move one step further through a path, checking if the file/dir exists
struct ext2_dir_entry_2* walk_one(int inode_num, char *file_to_find)
{
    //Now read in the root node, store into inode
    struct ext2_inode *inode;
    inode = malloc(sizeof(struct ext2_inode));
    lseek(fd, EXT2_BLOCK_SIZE* inode_table + 
        ((inode_num-1) * sizeof(struct ext2_inode)), SEEK_SET);
    if (read(fd, inode, sizeof(struct ext2_inode)) < 1)
    {
        perror("read");
        exit(1);
    }

    //Now we're going to search the root inode for the next file or directory in the path
    struct ext2_dir_entry_2* entry;
    //entry = malloc(sizeof(struct ext2_dir_entry_2));
    //block is the buffer
    struct ext2_dir_entry_2* block = malloc(EXT2_BLOCK_SIZE);
    unsigned int size;

    //Go through all the root's iblocks and check what's in them: if we find what we're looking
    //for, we're good 
    int j;
    int found =0;
    for(j=0; (j < 15 && (!found)); j++)
    {
        // Read in data from disk
        lseek(fd, EXT2_BLOCK_SIZE*(inode->i_block[j]), SEEK_SET);
        read(fd, block, EXT2_BLOCK_SIZE);

        size = 0;
        entry = (struct ext2_dir_entry_2*) block;
        // Iterate over the directory entries in the block until 
        // we reach the end or find the proper thing
        while((size < inode->i_size) && entry->rec_len > 0) 
        {
            char file_name[entry->name_len+1];
            memcpy(file_name, entry->name, entry->name_len);
            file_name[entry->name_len] = '\0';

            //If our filenames match, we found what we were looking for! On to the next
            if(strncmp(file_name, file_to_find, strlen(file_to_find)) == 0)
            {
                found =1;
                break;
            }
            // Move forward in the block to check another entry
            entry = (void*) entry + entry->rec_len;
            size += entry->rec_len;
        }
    }
    if (found)
    {
        return entry;
    }
    return NULL;

}


//Looks through the bistring corresponding to mode and returns the first empty
//block or inode (depend on mode) if it exists. If not, returns null
unsigned int reserve(char mode)
{
    unsigned empty =8;
    unsigned char *bitmap;
    unsigned short *count;
    unsigned int *sbcount;
    int numblocks;
    // For each mode, set whether to change inode or block count
    if (mode == 'b')
    {
        bitmap = block_bitmap;
        count = &(group_descr->bg_free_blocks_count);
        sbcount = &(superblock->s_free_blocks_count);
        numblocks = 16;
    }
    if (mode == 'i') 
    {
        bitmap = node_bitmap;
        count = &(group_descr->bg_free_inodes_count);
        sbcount = &(superblock->s_free_inodes_count);
        numblocks = 4;
    }
    int i;
    // We need to check every chunk of the bitmap for the appropriate bit to return
    for (i = 0; i < numblocks; i++) 
    {
        empty = mostSignificantBit(bitmap[i]);
        if(empty != 8) 
        {
            break;
        }
    }
    if (empty != 8)
    {
        // Set the bit as used so no one else can take it; reduce counts appropriately
        setBit (empty + (i * 8) + 1, mode);
        (*sbcount)--;
        (*count)--;
        return (empty + (i * 8) + 1);
    }
    else
    {
        printf("No remaining space to allocate files on the image\n");
        exit(ENOSPC);
    }
}

// Find most significant 0 bit of a byte
unsigned char mostSignificantBit(unsigned myInt){
    unsigned i;
    for (i = 0; i < 8; i++) 
    {
        if (!(myInt & (1 << i)))
        {
            return i;
        }
    }
    return 8;
}

//Set a bit_toSet bit of the bitstring of type mode to 1
void setBit(int bit_toSet, char mode) {
    
    int offset = (bit_toSet % 8);
    int section = (bit_toSet / 8);
    if (offset == 0) 
    {
        offset = 7;
        section -= 1;
    } 
    else 
    {
        offset -= 1;
    }
    // Set the bits of the appropriate bitmap
    if (mode == 'b')
    {
        block_bitmap[section] |= 1 << offset;
    }
    if (mode == 'i') 
    {
        node_bitmap[section] |= 1 << offset;
    }    
}

/* Given an inode number, return the inode object
 * Requires to be freed later in program execution
 */
struct ext2_inode *to_inode(int inode_num)
{
    // Returns a mallocd inode to play with and save later on
    struct ext2_inode *inode;
    inode = malloc(sizeof(struct ext2_inode));
    lseek(fd, EXT2_BLOCK_SIZE* inode_table + ((inode_num-1) 
        * sizeof(struct ext2_inode)), SEEK_SET);
    if (read(fd, inode, sizeof(struct ext2_inode)) < 1)
    {
        perror("read");
        exit(1);
    }    
    return inode;
}

/* Returns a spot on a block which has enough space to
 * write to
 */
int empty_dir_spot(struct ext2_inode *dirnode, int size)
{
    //Assuming size has already been rounded to a multiple of 4
    int off;
    int j;
    for(j=0; j<12; j++)
    {
        if(dirnode->i_block[j]) {
            // Check for a spot to put the entry in a block
            if(!((off = dir_block_offset(dirnode->i_block[j],size)) == -1)) {
                return EXT2_BLOCK_SIZE * (dirnode->i_block[j]) + off;
            }
        // If there isn't enough space then we need to allocate a new block
        } else {
            int empty_block = reserve('b');
            dirnode->i_block[j] = empty_block;
            dirnode->i_size += EXT2_BLOCK_SIZE;
            dirnode->i_blocks++;
            return EXT2_BLOCK_SIZE*(empty_block);
        }       
    }
    // Should never happen in this assignment
    exit(ENOSPC);
    return -1;
}

/* Returns the offset at which the directory entry
 * with enough space in front of it to hold our new directory
 * entry is.
 */
int dir_block_offset(int block_num, int size)
{
    struct ext2_dir_entry_2* entry;
    struct ext2_dir_entry_2* block = malloc(EXT2_BLOCK_SIZE);
    entry = (struct ext2_dir_entry_2*) block;

    lseek(fd, EXT2_BLOCK_SIZE* (block_num), SEEK_SET);    
    read(fd, block, EXT2_BLOCK_SIZE);

    unsigned int cur_size = 0;
    int entry_size;
    // Iterate over the block as long as there is more space for entries
    while((cur_size < EXT2_BLOCK_SIZE) && (entry->rec_len > 0)) 
    {
        // Check how big the current entry is and, if there is enough space, 
        // we know we can fit our entry
        entry_size = sizeof(struct ext2_dir_entry_2) + 
            entry->name_len + (4-(entry->name_len % 4));        
        if((entry->rec_len - entry_size) >= size)
        {   
            return cur_size;
        }
        cur_size += entry->rec_len;
        entry = (void*) entry + entry->rec_len;
    }
    return -1;
}

/* Creates a directory entry within a given block following
 * the provided parameters
 */
void new_dir_entry(int entry_ptr, int size, char *name, char file_type, int inode_num)
{
    //Assuming size has already been rounded to a multiple of 4
    int prev_rec_len = 0;
    int free_space = EXT2_BLOCK_SIZE;

    // Disregard the state of the block if it is the first entry
    if (strncmp(name, ".", strlen(name))) {
        // Keep the previous entry on hand
        struct ext2_dir_entry_2* prev_entry;
        prev_entry = malloc(sizeof(struct ext2_dir_entry_2));
        lseek(fd, entry_ptr, SEEK_SET);    
        read(fd, prev_entry, sizeof(struct ext2_dir_entry_2));
        if (!(prev_entry->rec_len == 0)) {
            // Change properties of the previous entry
            prev_rec_len = prev_entry->name_len + 
                sizeof(struct ext2_dir_entry_2) + (4-(prev_entry->name_len % 4));
            free_space = prev_entry->rec_len - prev_rec_len;
            prev_entry->rec_len = prev_rec_len;
            // Write it to disk
            lseek(fd, entry_ptr, SEEK_SET);    
            write(fd, prev_entry, sizeof(struct ext2_dir_entry_2));
        }
        free(prev_entry);
    }
    // Keep the new entry mallocd for now
    struct ext2_dir_entry_2 *new_entry = malloc(size);
    // Allocate information onto the new entry
    new_entry->inode = inode_num;
    new_entry->name_len = strlen(name);
    strncpy(new_entry->name, name, new_entry->name_len);
    new_entry->file_type = file_type;
    new_entry->rec_len = free_space;
    // Write it to disk
    lseek(fd, entry_ptr+prev_rec_len, SEEK_SET);
    write(fd, new_entry, size); 
    free(new_entry);
    // Make sure to increment how many directories we've used
    group_descr->bg_used_dirs_count++;
}

// Converts binary to hex, useful for testing and printing
void hex_to_binary(unsigned hex)
{
    unsigned i;
    for (i = 0; i < 8; i++) 
    {
        if (hex & (1 << i))
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
        if ((i+1)%8 == 0)
        {
            printf(" ");
        }
    }
}

/* Loops through the directory's inode and calls rm_block_offset until it finds
 * the item we're going to remove: removes it, and returns the offset of the 
 * item right before it to us
 */
int dir_rm_spot(struct ext2_inode *dirnode, char *name_to_rm, int rm_rec_len)
{
    int off;
    int j;
    for(j=0; j<12; j++)
    {
        if(dirnode->i_block[j]) 
        {
            off = rm_block_offset(dirnode->i_block[j],name_to_rm,rm_rec_len);

            //If off isn't -1, it means we found it and deleted it already:
            //just return!
            if (off >= 0)
            {
                return EXT2_BLOCK_SIZE * (dirnode->i_block[j]) + off;
            }
        }
        //This should never happen, the file should always be there
        else 
        {
            perror("something went wrong");
            exit(1);
        }       
    }
    // Should never happen in this assignment
    exit(ENOSPC);
    return -1;
}

/* Returns the offset of the entry in front of the directory entry we need to
 * remove, if it's there, and calls rm_dir_entry to remove it. 
 * Return -1 if not in this block
 */
int rm_block_offset(int block_num, char *name_to_rm, int rm_rec_len)
{
    struct ext2_dir_entry_2* entry;
    struct ext2_dir_entry_2* block = malloc(EXT2_BLOCK_SIZE);

    entry = (struct ext2_dir_entry_2*) block;
    int prev_size = -1;

    //Read the block at block_num into buffer block
    lseek(fd, EXT2_BLOCK_SIZE* (block_num), SEEK_SET);    
    read(fd, block, EXT2_BLOCK_SIZE);

    //Loop while there's still dir entries left, and we haven't gone over the
    //block size
    unsigned int cur_size = 0;
    while((cur_size < EXT2_BLOCK_SIZE) && (entry->rec_len > 0)) 
    {
        //Get the file name of the current dir entry
        char file_name[entry->name_len+1];
        memcpy(file_name, entry->name, entry->name_len);
        file_name[entry->name_len] = '\0';

        //If our filenames match, we found what we were looking for! 
        //prev will have the thing before it
        if(strncmp(file_name, name_to_rm, strlen(name_to_rm)) == 0)
        {
            //The entry to remove is at the beginning of a block
            if (prev_size == -1)
            {
                rm_dir_entry(EXT2_BLOCK_SIZE* (block_num), rm_rec_len, 
                    name_to_rm, 1);
                return EXT2_BLOCK_SIZE* (block_num);
            }

            //The entry to remove is not at the beginning of a block
            else
            {
                rm_dir_entry(EXT2_BLOCK_SIZE* (block_num) + prev_size, 
                    rm_rec_len, name_to_rm, 0);
                return EXT2_BLOCK_SIZE* (block_num) + prev_size;
            }
        }

        //Increment all the size counters & go to the next dir entry
        prev_size = cur_size;
        cur_size += entry->rec_len;
        entry = (void*) entry + entry->rec_len;
    }
    return -1;
}   

/* Removes the directory entry following the entry at offset prev_ptr, if 
 * isfirst is high. If not, removes the directory entry at prev_ptr, by
 * changing it to a blank entry, as it is the first entry in the block
 */
void rm_dir_entry(int prev_ptr, int rm_rec_len, char *name, int isfirst)
{
    //Seek and read in the dir entry at prev_ptr
    struct ext2_dir_entry_2* prev_entry;
    prev_entry = malloc(sizeof(struct ext2_dir_entry_2));
    lseek(fd, prev_ptr, SEEK_SET);    
    read(fd, prev_entry, sizeof(struct ext2_dir_entry_2));

    //Where the entry to remove isn't the first entry
    if(!isfirst) {
        //Just set the previous entry's rec_len to be its rec_len plus
        //the entry to remove's rec_len
        prev_entry->rec_len += rm_rec_len;
    }

    //Otherwise it's the first entry, prev_entry actually refers to the entry
    //we want to delete: make it empty
    else 
    {
        memset(prev_entry->name, 0, strlen(prev_entry->name));
        prev_entry->name_len = 0;
        prev_entry->inode = 0;
        prev_entry->file_type = 0;
    }
    
    //write the new directory entry back into the image
    lseek(fd, prev_ptr, SEEK_SET);    
    write(fd, prev_entry, sizeof(struct ext2_dir_entry_2));
    free(prev_entry);
}

/* 
 * Loops through all the blocks that an inode is pointing to and calls clear_block
 * on them to clear them out
 */
void clear_all(struct ext2_inode *inode_to_rm)
{
    int j;
    for(j=0; j<12; j++)
    {
        if(inode_to_rm->i_block[j]) 
        {
           clear_block(inode_to_rm->i_block[j]);
        }

        //We're done clearing blocks if one in between doesn't exist
        else 
        {
           break;
        }       
    }

    //If there's single-indirection, we must go to that block and
    //clear all the blocks it's pointing to
    if (inode_to_rm->i_block[12])
    {
        lseek(fd, EXT2_BLOCK_SIZE * inode_to_rm->i_block[12], SEEK_SET);    
        unsigned int currBlock;
        int amt_read = 0;      

        //Read in the next block entry in the indirection block: As long as it
        //exists, go to it and clear it
        while (((amt_read += read(fd, &currBlock, sizeof(unsigned int))) > 0) 
            && (amt_read < EXT2_BLOCK_SIZE))
        {
            //If there's no more blocks read in, we're done
            if(!currBlock)
            {
                break;
            }
            else
            {
               clear_block(currBlock);
            }
        }

        //Now we're done clearing the indirect blocks, clear the block holding 
        //them!
        clear_block(inode_to_rm->i_block[12]);
    }
}

/*
 * Takes a block number and clears that block by setting everything in the image
 * at that block to 0.
 * Clears the bitstring for that block too so we can use it again.
*/
void clear_block(int blocknum)
{
    char buf [EXT2_BLOCK_SIZE];
    memset(buf, 0, EXT2_BLOCK_SIZE);
    lseek(fd, EXT2_BLOCK_SIZE * blocknum, SEEK_SET);    
    write(fd, buf, EXT2_BLOCK_SIZE);    
    clear_bit(blocknum, 'b');
}

/*
 * Takes an inode number and clears that inode table entry by setting everything 
 * in the image at that inode to 0.
 * Clears the bitstring for that inode too.
*/
void clear_inode(int inode_num)
{
    // Set up a temporary buffer
    char buf [sizeof(struct ext2_inode)];
    memset(buf, 0, sizeof(struct ext2_inode));
    // Seek to the disk
    lseek(fd, EXT2_BLOCK_SIZE* inode_table + ((inode_num-1) 
        * sizeof(struct ext2_inode)), SEEK_SET);    
    write(fd, buf, sizeof(struct ext2_inode));
    // Clear the bit on the inode_bitmap
    clear_bit(inode_num, 'i');
}

/* 
 * Clears a bit in a specified bitmap on disk by setting it back to 0
 */
void clear_bit(int bit, char mode) {
    unsigned char *bitmap;
    unsigned short *count;
    unsigned int *sbcount;
    // Set the appropriate bitmaps to change
    if (mode == 'b'){
        bitmap = block_bitmap;
        count = &(group_descr->bg_free_blocks_count);
        sbcount = &(superblock->s_free_blocks_count);
    }
    if (mode == 'i') {
        bitmap = node_bitmap;
        count = &(group_descr->bg_free_inodes_count);
        sbcount = &(superblock->s_free_inodes_count);
    }
    // Set the appropriate counts for these bitmaps and unset the appropriate bit
    (*count)++;
    (*sbcount)++;
    bitmap[bit + 1] = 0;
}
