/* Read and execute a list of operations on a linked list.
 * Periodically call the garbage collector.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "gc.h"
#include "gc_list.h"

#define MAX_LINE 128
#define ADD_NODE 1
#define DEL_NODE 2
#define PRINT_LIST 3

List *ll = NULL;

void gcSigHandlerList(int signum) {
    if (signum == SIGUSR1) {
        mark_and_sweep(ll, mark_list);
    }
}

int main(int argc, char **argv) {
    //Implement the signal handler
    signal(SIGUSR1, gcSigHandlerList);

    char line[MAX_LINE];
    char *str;

    if(argc != 2) {
        fprintf(stderr, "Usage: do_trans filename\n");
        exit(1);
    }

    FILE *fp;
    if((fp = fopen(argv[1], "r")) == NULL) {
        perror("fopen");
        exit(1);
    }

    int count = 1;

    while(fgets(line, MAX_LINE, fp) != NULL) {

        char *next;
        int value;
        int type = strtol(line, &next, 0);

        switch(type) {
            case ADD_NODE :
                ; // Included to remain compilable
                sigset_t sigs;
                sigemptyset(&sigs);
                sigset_t old_sigs;
                sigaddset(&sigs, SIGUSR1);
                //Block signals
                if(sigprocmask(SIG_BLOCK, &sigs, &old_sigs) != 0) {
                    exit(1);
                }
                value = strtol(next, NULL, 0);
                ll = add_node(ll, value);
                //Unblock signals
                if(sigprocmask(SIG_SETMASK, &old_sigs, NULL) != 0) {
                    exit(1);
                }
                break;
            case DEL_NODE :
                value = strtol(next, NULL, 0);
                ll = remove_node(ll, value);
                break;
            case PRINT_LIST :
                str = tostring(ll);
                printf("List is %s\n", str);
                break;
            default :
                fprintf(stderr, "Error: bad transaction type\n");

        }
    
        
        if(count % 10 == 0) {
            mark_and_sweep(ll, mark_list);
        }       
        count++;

    }
    return 0;
}
