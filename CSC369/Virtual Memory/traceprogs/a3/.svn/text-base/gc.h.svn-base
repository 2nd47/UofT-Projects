/* External functions that are the interface to the garbage collector */
#ifndef LOGFILE
#define LOGFILE "gc.log"
#endif

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

struct mem_chunk *MEMORY_LIST_PTR;

struct mem_chunk {
    int in_use;
    void * address;
    struct mem_chunk *next;
};

typedef struct mem_chunk Mem_chunk;

//Functions associated with garbage collection in both fstrees and lists
void mark_list(void *obj);
void mark_fstree(void *obj);

//Functions associated with garbage collection data structure manipulation
void reset_chunks(Mem_chunk *chunk);
void rm_chunks(Mem_chunk *chunk);

//Functions associated with debugging and log generation
void report_changes(Mem_chunk *chunk, int mode);
int calculate_chunks(FILE *stream, Mem_chunk *chunk, int mode);

//Functions associated with signal handling
void gcSigHandlerList(int signum);
void gcSigHandlerFstree(int signum);

/* A wrapper around malloc to keep track of dynamically allocated memory
 */
void *gc_malloc(int nbytes);

/* Executes the garbage collector.
 * mark_obj will traverse the data structure rooted at obj, calling
 * mark_one for each dynamically allocated chunk of memory to mark
 * that it is still in use.
 */
void mark_and_sweep(void *obj, void (*mark_obj)(void *));

// how do function pointers work?
// obj points to root of data structure?
// mark obj is the appropriate mark function


/* Mark vptr as still in use
 * Return code:
 *   0 on success
 *   1 if memory chunk pointed to by vptr was already marked as in use
 */
int mark_one(void *vptr);