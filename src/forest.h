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
#include "scanner.h"
//#include "parser.h"

typedef enum f_keyword {
    W_GLOBAL,
    W_FUNCTION,
    W_IF,
    W_ELSE,
    W_WHILE,
    W_NONE
} f_keyword_t;


// A node in the forest
typedef struct s_forest_node {
    f_keyword_t keyword;
    char *name;
    struct s_forest_node *parent;
    struct s_forest_node **children; // array of pointers to children
    int children_count; // number of children (last index + 1)
    AVL_tree *symtable; // pointer to the scope's symbol table
    int cond_cnt; // counter for if/else/while
} forest_node;

// Global pointer to the active node in forest
extern forest_node *active;

/**
 * @brief Inserts a global node into the forest
 * 
 * @return forest_node* pointer to the global node
 */
forest_node *forest_insert_global();

/**
 * @brief Inserts a local node into the forest
 * 
 * @param parent Pointer to the parent node of the new node
 * @param keyword 
 * @param name 
 * @param active Pointer to the active node in forest for correction
 */
void forest_insert(forest_node *parent, f_keyword_t keyword, char *name, forest_node **active);


/**
 * @brief Search if the function is already defined in the global scope
 * 
 * @param global Pointer to the global node (root of the forest)
 * @param key Key of the function to search for
 * @return forest_node* Pointer to the function if found, NULL otherwise
 */
forest_node* forest_search_function(forest_node *global, char *key);

/**
 * @brief Searches for a symbol in the symbol table of the node (and its parents if not found)
 * 
 * @param node Pointer to the node where to start searching
 * @param key Key of the symbol to search for
 * @return AVL_tree* Pointer to the symbol if found, NULL otherwise
 */
AVL_tree *forest_search_symbol(forest_node *node, char *key);

/**
 * @brief Forest dispose
 * 
 * @param global Pointer to the global node (root of the forest)
 */
void forest_dispose(forest_node *global);

void traverse_forest(forest_node *node);

//void rename_keep_exit();


///**
// * @brief Converts the data type of a symbol from T_Q_STRING to T_STRING, T_Q_INT to T_INT, T_Q_FLOAT to T_FLOAT, used for if let redeclaration
// *
// * @param key Key of the symbol to convert
// */
//void forest_convert_to_nonq_data_type(char *key){};


#endif //IFJ_FOREST_H
