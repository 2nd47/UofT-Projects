/*
 * Generate a random series of add, delete, and print operations on a 
 * linked list.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "list.h"
#include "gc.h"

#define MAX_VAL 10000

/* The probability of generating print and add operations
 * The probability of generating a delete operation is 
 * 1 - (PROB_PRINT + PROB_ADD)
 */
#define PROB_PRINT 0.05
#define PROB_ADD 0.55



int main(int argc, char **argv) {
    FILE *stream;
    long type;
    long value;
    int length = 0;
    List *ll;

    if (argc == 2) {
        if (!(stream = fopen(argv[1], "w"))) {
            perror("fopen");
            exit(1);
        }
    } else {
        stream = stdout;
    }

    int i;
    //srandom(time(NULL));
    for(i = 0; i < MAX_VAL; i++) {
        double prob = (double)random() / RAND_MAX;
        if(prob < PROB_PRINT) {
            type = 3;
            fprintf(stream, "%ld\n", type);
        }
        if(prob < PROB_PRINT + PROB_ADD) {
            type = 1;
            value = random() % MAX_VAL;
            ll = add_node(ll, value);
            length++;
            fprintf(stream, "%ld %ld\n", type, value);
        } else {
            type = 2;
            if(length > 0) {
                // choose a node that is in the list to delete
                int index = random() % length;
                value = find_nth(ll, index);
                ll = remove_node(ll, value);
                length--;
                fprintf(stream, "%ld %ld\n", type, value);
            } 
        } 
    }
    if (argc == 2) {
        fclose(stream);
    }
    return 0;
}
