#include "fstree.h"
#include "gc.h"

/* Iterate through the fstree and mark every item that is being used 
 * for the garbage collector.
 */
void mark_fstree(void *obj) {
    Fstree *ptr = obj;
    if (mark_one(ptr) != 1) {
        mark_one(ptr->name);
        Link *linkptr = ptr->links;
        while (linkptr) {
            mark_one(linkptr);
            mark_fstree(linkptr->fptr);
            linkptr = linkptr->next;
        }
    }
}