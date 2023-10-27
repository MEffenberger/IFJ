/**
 * @file forest.c
 *
 * IFJ23 compiler
 *
 * @brief A structure to better represent the hierarchy between the nodes of symtable and ast
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */

#ifndef IFJ_FOREST_H
#define IFJ_FOREST_H

#include "symtable.h"
#include "error.h"

typedef struct s_forest_node {
    struct s_forest_node *parent;
    struct s_forest_node **children;
    int children_count;
    AVL_tree *symtable;
    // add key to represent the node's type
} forest_node;



void forest_init(forest_node **root);

forest_node *forest_insert_global();







void forest_dispose(forest_node **root);

void forest_insert_first(forest_node **forest);

forest_node *forest_get_child(forest_node *parent, int index);

forest_node *forest_get_parent(forest_node *node);

AVL_tree *forest_get_symtable(forest_node *node);




#endif //IFJ_FOREST_H
