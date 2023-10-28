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


#include "forest.h"
#include "symtable.h"
#include "error.h"


forest_node *forest_insert_global() {
    forest_node *root = (forest_node*)allocate_memory(sizeof(forest_node), "global forest node", FOREST);
    root->name = "global";
    root->keyword = KW_GLOBAL;
    root->parent = NULL;
    root->children = NULL;
    root->children_count = 0;
    root->symtable = NULL;
    return root; 
}

void forest_insert(forest_node *parent, keyword_t keyword, char *name, forest_node **active) {
    if (parent != NULL) {
        forest_node *child = (forest_node*)allocate_memory(sizeof(forest_node), "forest node", FOREST);
        child->name = name;
        child->keyword = keyword;
        child->parent = parent;
        child->children = NULL;
        if (parent->children == NULL) {
            parent->children = (forest_node**)allocate_memory((sizeof(forest_node*)) , "forest node children", BASIC);
        } else {
            // Resize the existing array to accommodate the new child
            parent->children = (forest_node**)realloc(parent->children, (parent->children_count + 1) * sizeof(forest_node*));
            if (parent->children == NULL) {
                error_exit(ERROR_INTERNAL, "REALLOC", "Memory reallocation for forest node children failed");
            }
        }
        parent->children[parent->children_count] = child;
        parent->children_count++;
        child->symtable = NULL;

        *active = child;
    }
}

// search for a symbol in a symtable, if not found, search in the parent's symtable
AVL_tree *forest_search_symbol(forest_node *node, char *key) {
    printf("start\n");
    if (node != NULL) {
        printf("searching in %s\n", node->name);
        if (node->symtable != NULL) {
            printf("tryind to find %s\n", key);
            AVL_tree *found = symtable_search(node->symtable, key);
            printf("found: %s\n", found->key);
            if (found != NULL) {
                printf("returning\n");
                return found;
            }
        }
        return forest_search_symbol(node->parent, key);
    }
    printf("returning null\n");
    return NULL;
}

void forest_dispose(forest_node *global) {
    if (global != NULL) {
        if (global->children != NULL) {
            for (int i = 0; i < global->children_count; i++) {
                forest_dispose(global->children[i]);
            }
            free(global->children);
        }
        if (global->symtable != NULL) {
            symtable_dispose(&(global->symtable));
        }
        free(global);
        global = NULL;
    }
}