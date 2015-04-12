/* Test three cases of lost memory in this list:
 *   - Node removed from top of list
 *   - Node removed from midle of list
 *   - Node removed from bottom of list
 *   - All nodes removed from list
 *
 * If all of these three cases work then all combinations of
 * cases must also work.
 *
 * Consult the LOGFILE generate by gc.c for explicit details regarding
 * garbage collection in the linked list structure.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "gc.h"

int main(int argc, char **argv) {
	List *ll = NULL;

	//Setup the initial linked list structure
	//In LOGFILE, this is represented as five chunks of memory
	ll = add_node(ll, 1);
	ll = add_node(ll, 2);
	ll = add_node(ll, 3);
	ll = add_node(ll, 4);
	ll = add_node(ll, 5);

	//Remove node from top of list
	//In LOGFILE, this is represented as five chunks of memory:
	//	4 in-use
	//	1 not-in-use
	//The not-in-use chunk is removed from memory
	ll = remove_node(ll, 1);
	mark_and_sweep(ll, mark_list);

	//Remove node from middle of list
	//In LOGFILE, this is represented as five chunks of memory:
	//	3 in-use
	//	1 not-in-use
	//The not-in-use chunk is removed from memory
	ll = remove_node(ll, 4);
	mark_and_sweep(ll, mark_list);

	//Remove node from botom of list
	//In LOGFILE, this is represented as five chunks of memory:
	//	2 in-use
	//	1 not-in-use
	//The not-in-use chunk is removed from memory
	ll = remove_node(ll, 5);
	mark_and_sweep(ll, mark_list);

	//Remove all remaining nodes
	//In LOGFILE, this is represented as five chunks of memory:
	//	0 in-use
	//	2 not-in-use
	//The not-in-use chunks are removed from memory
	ll = remove_node(ll, 2);
	ll = remove_node(ll, 3);
	mark_and_sweep(ll, mark_list);

    return 0;
}