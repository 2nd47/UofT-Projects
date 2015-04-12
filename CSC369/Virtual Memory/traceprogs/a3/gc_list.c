#include "list.h"
#include "gc.h"

/* Iterate through each item of the linked list and 
 * make every used item in the garbage collector
 */
void mark_list(void *obj) {
    List *ptr = obj;
    if(ptr) {
        mark_one(ptr);
        if(ptr->next) {
            ptr = ptr->next;
            mark_list(ptr);
        }
    }
}