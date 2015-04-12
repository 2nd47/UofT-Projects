#include <stdio.h>
#include "fstree.h"
#include "gc.h"
#include "gc_fstree.h"

extern Fstree *root;

void gcSigHandlerFstree(int signum) {
    if (signum == SIGUSR1) {
        mark_and_sweep(root, mark_fstree);
    }
}

int main() {
    //Implement the signal handler
    signal(SIGUSR1, gcSigHandlerFstree);

    //Code has been modified such that garbage collection occurs within
    //every do_transactions call. See fstree.c:243 for details 
    root = create_node("root");

    do_transactions("fs_trans1");
    print_tree(root, 0);

    do_transactions("fs_trans2");
    print_tree(root, 0);

    do_transactions("fs_trans_loop");

    //Or you can put the calls directly in here but be careful, you
    //can't use string literals for the paths or you will get Segmentation errors.
    /*
    add_node("","one");  // add_node (path, name)
    print_tree(root, 0);
    add_node("one","oneone");  // add_node (path, name)
    print_tree(root, 0);
    printf("************\n");

    char dest_path[20] = "one/oneone";
    char src_path[20] = "";
    char name[10] = "one";

    // this makes a link as a child of  one/oneone back to /one
    add_hard_link(dest_path, src_path, name);
    //DON"T call print_tree now since it doesn't handle loops and 
    //will print infinitely
    //print_tree(root, 0);
    */


    // once you've implemented your garbage collector on fstree, you can
    // call it from here at various points to clean up the garbage
    mark_and_sweep(root, mark_fstree);

    return 0;
}
