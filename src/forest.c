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


#include "callee.h"
#include "cnt_stack.h"
#include "codegen.h"
#include "error.h"
#include "expression_parser.h"
#include "forest.h"
#include "parser.h"
#include "queue.h"
#include "scanner.h"
#include "string_vector.h"
#include "symtable.h"
#include "token_stack.h"
#include <string.h>

int forest_node_cnt = -10; // skip built-in functions


forest_node *forest_insert_global() {
    forest_node *root = (forest_node*)allocate_memory(sizeof(forest_node), "global forest node", FOREST);
    root->name = "global";
    root->keyword = W_GLOBAL;
    root->parent = NULL;
    root->children = NULL;
    root->children_count = 0;
    root->symtable = NULL;
    root->cond_cnt = 0;
    root->param_cnt = 0;
    root->node_cnt = 0;
    root->has_return = false;
    return root; 
}

void forest_insert(forest_node *parent, f_keyword_t keyword, char *name, forest_node **active) {
    if (parent != NULL) {
        forest_node *child = (forest_node*)allocate_memory(sizeof(forest_node), "forest node", FOREST);
        child->name = name;
        child->keyword = keyword;
        child->parent = parent;
        child->children = NULL;
        child->cond_cnt = 0;
        child->param_cnt = 0;
        child->node_cnt = ++forest_node_cnt;
        child->has_return = false;
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

forest_node* forest_search_function(forest_node *global, char *key) {
    if (global->children != NULL) {
        for (int i = 0; i < global->children_count; i++) {
            if (global->children[i]->keyword == W_FUNCTION && strcmp(global->children[i]->name, key) == 0) {
                return global->children[i];
            }
        }
    }
    return NULL;
}



// search for a symbol in a symtable, if not found, search in the parent's symtable
AVL_tree *forest_search_symbol(forest_node *node, char *key) {
    if (node != NULL) {
        if (node->symtable != NULL) {
            AVL_tree *found = symtable_search(node->symtable, key);
            if (found != NULL) {
                return found;
            }
        }
        return forest_search_symbol(node->parent, key);
    }
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

void traverse_forest(forest_node *node) {
    if (node == NULL) {
        return;
    }

    // Process the current node
    printf("Node Name: %s %p\n", node->name, node);

    // Recursively traverse the children
    for (int i = 0; i < node->children_count; i++) {
        traverse_forest(node->children[i]);
    }
}
