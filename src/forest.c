/**
 * @file forrest.c
 *
 * IFJ23 compiler
 *
 * @brief A structure to better represent the hierarchy between the nodes of symtable and ast
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */


#include "forest.h"
#include "symtable.h"


void forest_init(forest_node **root) {
    *root = NULL;
}

forest_node *forest_insert_global() {
    forest_node *global = NULL;
    global = (forest_node*)allocate_memory(sizeof(forest_node), "forest node", FOREST);
    global->parent = NULL;
    global->children_count = 0;
    global->children = NULL;
    AVL_tree *symtable;
    symtable_init(&symtable);
    global->symtable = symtable;
    return global;
}

void forest_insert_local(forest_node **parent) {
    forest_node *local;
    forest_init(&local);
    local = (forest_node*)allocate_memory(sizeof(forest_node), "forest node", FOREST);
    local->parent = parent;
    local->parent->children[local->parent->children_count] = local;
    local->parent->children_count++;
    local->children_count = 0;
    local->children = NULL;
    AVL_tree *symtable;
    symtable_init(&symtable);
    local->symtable = symtable;
}













void forest_dispose(forest_node **root) {
    if ((*root) != NULL) {
        while ((*root)->children != NULL) {
            symtable_dispose((*root)->symtable);
            forest_dispose((*root)->children);
        }
        free(root);
        *root = NULL;
    }
}