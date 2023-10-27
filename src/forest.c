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


void forest_init(forest_node **root) {
    *root = NULL;
}

void forest_dispose(forest_node **root) {

    if (*root != NULL) {
        while (*root->children != NULL) {
            symtable_dispose(*root->symtable);
            forest_dispose(*root->children);
        }
        free(root);
    }

}

void forrest_insert_first(forest_node **root) {

    if (root == NULL) {
        *root->parent = (forest_node*)allocate_memory(sizeof(forest_node), "forest node", FOREST);
    }

}
