#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

/* A wrapper around malloc to keep track of dynamically allocated memory
 */
void *gc_malloc(int nbytes) {
	Mem_chunk *newChunk;
	if (!(newChunk = malloc(sizeof(Mem_chunk)))) {
		perror("malloc");
		exit(1);
	}
	if (!(newChunk->address = malloc(nbytes))) {
		perror("malloc");
		exit(1);
	}
	// Add to beginning of the new garbage collector list
	if (!MEMORY_LIST_PTR) {
		newChunk->next = NULL;
	} else {
		newChunk->next = MEMORY_LIST_PTR;
	}
	MEMORY_LIST_PTR = newChunk;
	newChunk->in_use = 1;
	return newChunk->address;
}

//Reset all memory chunks to not-in-use linked to by the chunk
void reset_chunks(Mem_chunk *chunk) {
	if(chunk) {
		chunk->in_use = 0;
		if(chunk->next) {
			reset_chunks(chunk->next);
		}
	}
}

//Removes all not-in-use memory chunks from memory
void rm_chunks(Mem_chunk *chunk) {
	Mem_chunk *prev = chunk;
	while (chunk) {
		//For each chunk removal, keep MEMORY_LIST_PTR accurate
		if (chunk->in_use == 0) {
			//We are at the top, simply free and move the list up one.
			if (chunk == MEMORY_LIST_PTR) {
				MEMORY_LIST_PTR = chunk->next;
				chunk = chunk->next;
				if(prev->address) {
					free(prev->address);
				}
				free(prev);
			} else {
				//Skip over the middle element and then free it.
				if (prev == MEMORY_LIST_PTR) {
					MEMORY_LIST_PTR->next = MEMORY_LIST_PTR->next->next;
				}
				prev->next = chunk->next;
				if(chunk->address) {
					free(chunk->address);
				}
				free(chunk);
				chunk = prev;
			}
		}
		prev = chunk;
		chunk = chunk->next;
	}
}

/* Executes the garbage collector.
 * mark_obj will traverse the data structure rooted at obj, calling
 * mark_one for each dynamically allocated chunk of memory to mark
 * that it is still in use.
 */
void mark_and_sweep(void *obj, void (*mark_obj)(void *)) {
	//Report the state of the gc data structure during the mark and sweep
	//Reset
	reset_chunks(MEMORY_LIST_PTR);
	report_changes(MEMORY_LIST_PTR, 0);
	//Mark
	mark_obj(obj);
	report_changes(MEMORY_LIST_PTR, 1);
	//Sweep
	rm_chunks(MEMORY_LIST_PTR);
	report_changes(MEMORY_LIST_PTR, 2);
}

/* Mark vptr as still in use
 * Return code:
 *   0 on success
 *   1 if memory chunk pointed to by vptr was already marked as in use
 */
int mark_one(void *vptr) {
	Mem_chunk *chunk = MEMORY_LIST_PTR;
	while (chunk) {
		if (chunk->address == vptr) {
			if (chunk->in_use == 1) {
				return 1;
			} else {
				chunk->in_use = 1;
			}
		}
		if (!chunk->next) {
			return 0;
		}
		chunk = chunk->next;
	}
	return 0;
}

/* Report all changes conducted by this mark and sweep iteration.
 * Includes modes:
 *   - 0, report how many total chunks are being used
 *   - 1, report how many total chunks were marked
 */
void report_changes(Mem_chunk *chunk, int mode) {
 	FILE *openedFile;
 	int chunkCount;
 	if (!(openedFile = fopen(LOGFILE, "a"))) {
 		perror("fopen");
 		exit(1);
 	}
 	switch(mode) {
 		case 0:
 			chunkCount = calculate_chunks(openedFile, MEMORY_LIST_PTR, 0);
 			fprintf(openedFile, 
 				"RESET : Total chunks: %d\n\n", chunkCount);
 			break;

 		case 1:
 			chunkCount = calculate_chunks(openedFile, MEMORY_LIST_PTR, 1);
 			fprintf(openedFile, 
 				"MARK : Total chunks marked: %d\n\n", chunkCount);
 			break;

 		case 2:
 			chunkCount = calculate_chunks(openedFile, MEMORY_LIST_PTR, 0);
 			fprintf(openedFile, 
 				"SWEEP : Total chunks now remaining: %d\n\n", chunkCount);
 			break;

 	}
 	fclose(openedFile);
 }

/* Calculate all the chunks within the gc data structure
 * With mode n returns:
 *   0, number of total allocated chunks
 *   1, number of total marked chunks
 */
int calculate_chunks(FILE *stream, Mem_chunk *chunk, int mode) {
	Mem_chunk *ptr = chunk;
	int totalChunks = 0;
	while (ptr) {
		if (mode && ptr->in_use) {
			totalChunks++;
		} else if (!mode) {
			totalChunks++;
		}
		fprintf(stream, "Chunk with in_use %d pointing to %p \n", 
			ptr->in_use, ptr->address);
		ptr = ptr->next;
	}
	return totalChunks;
}
