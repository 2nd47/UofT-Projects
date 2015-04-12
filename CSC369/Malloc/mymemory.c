#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "mymemory.h"

/* mymalloc_init: initialize any data structures that your malloc needs in
                  order to keep track of allocated and free blocks of 
                  memory.  Get an initial chunk of memory for the heap from
                  the OS using sbrk() and mark it as free so that it can  be 
                  used in future calls to mymalloc()
*/

int mymalloc_init() {
  if ((freep = sbrk(PAGE)) < 0) {
    return 1;
  }
  lock = (pthread_mutex_t *) freep;
  freep = (FreeHeader *) (char *) freep + sizeof(*lock);
  freep->size = PAGE - sizeof(lock);
  freep->prev = freep;
  freep->next = freep;
  if (pthread_mutex_init(lock, NULL) < 0) {
    return 1;
  }
	return 0; // non-zero return value indicates an error
}


/* mymalloc: allocates memory on the heap of the requested size. The block
             of memory returned should always be padded so that it begins
             and ends on a word boundary.
     unsigned int size: the number of bytes to allocate.
     retval: a pointer to the block of memory allocated or NULL if the 
             memory could not be allocated. 
             (NOTE: the system also sets errno, but we are not the system, 
                    so you are not required to do so.)
*/
void *mymalloc(unsigned int size) {
  AllocHeader *newAlloc;
  FreeHeader *freeHeader = freep;
  unsigned int totalSize = ALIGN8(size + HEADER_SIZE);
  unsigned int totalPages = ALIGNPAGE(totalSize);
  
  while (freeHeader->size < totalSize) {
    if (freeHeader->next != freep) {
      freeHeader = freeHeader->next;
    }
  }
  if (pthread_mutex_lock(lock) < 0) {
    return NULL;
  }
  if (freeHeader->next == freep && freeHeader->size < totalSize) {
    freeHeader = extendHeap(freeHeader, totalPages);
    if (freeHeader == NULL) {
        return NULL;
    }
  }
  newAlloc = (AllocHeader *) freeHeader;
  freeHeader = moveFreeHeader(freeHeader, totalSize);
  newAlloc = createHeader(newAlloc, size);
  if (pthread_mutex_unlock(lock) < 0) {
    return NULL;
  }
  return (void *) ((char *) newAlloc + HEADER_SIZE);
}


/* myfree: unallocates memory that has been allocated with mymalloc.
     void *ptr: pointer to the first byte of a block of memory allocated by 
                mymalloc.
     retval: 0 if the memory was successfully freed and 1 otherwise.
             (NOTE: the system version of free returns no error.)
*/
unsigned int myfree(void *ptr) {
	FreeHeader *freeHeader, *newFree;
  freeHeader = freep;
  AllocHeader *allocHeader = (AllocHeader *) ((char *) ptr - HEADER_SIZE);

  if (allocHeader->magic != MAGIC_NUMBER) {
    return 1;
  }
  while (((FreeHeader *)ptr < freeHeader) && (freeHeader->next != freep)) {
    freeHeader = freeHeader->next;
  }
  if (pthread_mutex_lock(lock) < 0) {
    return 1;
  }
  newFree = addBlock(allocHeader, freeHeader);
  coalesceBlocks(newFree);
  if (pthread_mutex_unlock(lock) < 0) {
    return 1;
  }
	return 0;
}

FreeHeader *moveFreeHeader(FreeHeader *node, int movement) {
  FreeHeader *temp = node;

  node = (FreeHeader *) ((char *) node + movement);
  node->size = temp->size;
  node->prev = temp->prev;
  node->next = temp->next;
  node->size = temp->size;
  if ((char *) node->prev == (char *) temp && (char *) node->next == (char *) temp) {
    node->next = node;
    node->prev = node;
  }
  else {
    node->next->prev = node;
    node->prev->next = node;
  }
  if (temp == freep) {
    freep = node;
  }
  node->size -= movement;
  return node;
}

FreeHeader *addBlock(AllocHeader *ptr, FreeHeader *prev) {
  FreeHeader *newFree = (FreeHeader *) ptr;

  newFree->size = ptr->size;
  newFree->prev = prev;
  newFree->next = prev->next;
  prev->next->prev = newFree;
  prev->next = newFree;
  return newFree;
}

void coalesceBlocks(FreeHeader *free) {
  if ((char *) free->next == (char *) free + free->size) {
    free->size += free->next->size;
    free->next = free->next->next;
    free->next->prev = free;
  }
  if ((char *) free == (char *) free->prev + free->prev->size) {
    free->prev->size += free->size;
    free->next->prev = free->prev;
    free->prev->next = free->next;
  }
}

FreeHeader *extendHeap(FreeHeader *endHeap, unsigned int size) {
  if ((endHeap->next = sbrk(size)) < 0) {
    return NULL;
  }
  endHeap->next->prev = endHeap;
  endHeap = endHeap->next;
  endHeap->size = size;
  endHeap->next = freep;
  return endHeap;
}

AllocHeader *createHeader(AllocHeader *ptr, unsigned int size) {
  ptr->size = size;
  ptr->magic = MAGIC_NUMBER;
  return ptr;
}
  