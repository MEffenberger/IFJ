/**
 * @file forest.h
 *
 * IFJ23 compiler
 *
 * @brief Hierarchical tree structure to represent the scopes of the program
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */

#ifndef IFJ_FOREST_H
#define IFJ_FOREST_H

#include "symtable.h"


// Keyword of the node
typedef enum f_keyword {
    W_GLOBAL,
    W_FUNCTION,
    W_FUNCTION_BODY,
    W_IF,
    W_ELSE,
    W_WHILE,
    W_NONE
} f_keyword_t;

// Structure of the forest node
typedef struct s_forest_node {
    f_keyword_t keyword; // type of the node
    char *name;
    struct s_forest_node *parent;
    struct s_forest_node **children; // array of pointers to children nodes
    int children_count; // number of children (last index + 1)
    AVL_tree *symtable; // pointer to the scope's symbol table
    int cond_cnt; // counter for if/else
    int param_cnt; // counter for parameters
    int node_cnt; // counter for nodes, used for renaming
    bool has_return; // true if the scope has return statement in it
    char frame; // frame of the scope (G/L)
} forest_node;

extern forest_node *active; // Global pointer to the active node in forest

/**
 * @brief Inserts a global node into the forest
 * 
 * @return forest_node* pointer to the global node
 */
forest_node *forest_insert_global();


/**
 * @brief Inserts a new node into the forest
 * 
 * @param parent Pointer to the parent node
 * @param keyword Keyword of the node
 * @param name Name of the node
 * @param active Pointer to the active node in forest
 */
void forest_insert(forest_node *parent, f_keyword_t keyword, char *name, forest_node **active);


/**
 * @brief Search the function in the global scope
 * 
 * @param global Pointer to the global node (root of the forest)
 * @param key Key of the function to search for
 * @return forest_node* Pointer to the function if found, NULL otherwise
 */
forest_node* forest_search_function(forest_node *global, char *key);


/**
 * @brief Checks if the node is inside a function
 * 
 * @param node Pointer to the node to be checked
 * @return true Node is inside a function
 * @return false Node is not inside a function
 */
bool forest_check_inside_func(forest_node *node);


/**
 * @brief Search for the outermost while loop (codegen problem)
 * 
 * @param node Pointer to the node where to start searching
 * @return forest_node* Pointer to the outermost while loop
 */
forest_node* forest_search_while(forest_node *node);


/**
 * @brief Searches for a symbol in the symbol table of the node (and its parents if not found)
 * 
 * @param node Pointer to the node where to start searching
 * @param key Key of the symbol to search for
 * @return AVL_tree* Pointer to the symbol if found, NULL otherwise
 */
AVL_tree *forest_search_symbol(forest_node *node, char *key);


/**
 * @brief Searches for a scope where the symbol is (and its parents if not found)
 * 
 * @param node Pointer to the node where to start searching
 * @param key Key of the symbol to search for
 * @return forest_node* Pointer to the scope if found, NULL otherwise
 */
forest_node *forest_search_scope(forest_node *node, char *key);


/**
 * @brief Forest dispose
 * 
 * @param global Pointer to the global node (root of the forest)
 */
void forest_dispose(forest_node *global);


#endif //IFJ_FOREST_H
