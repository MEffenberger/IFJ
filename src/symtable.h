/**
 * @file symtable.h
 *
 * IFJ23 compiler
 *
 * @brief Symbol table using AVL tree: header file
 *
 * @author Adam Valík <xvalik05>
 * @author Marek Effenberger <xeffen00>
 */

#ifndef IFJ_SYMTABLE_H
#define IFJ_SYMTABLE_H

#include <stdbool.h>     
#include "error.h"   

#define MAX(a, b) ((a) > (b) ? (a) : (b))





typedef enum e_data_type {
    INT,
    DOUBLE,
    STRING,
    NIL,
    BOOL,
    INT_QM,
    DOUBLE_QM,
    STRING_QM,
    VOID,
    UNKNOWN
} data_type;

typedef enum e_var_type {
    VAR,
    LET
} var_type;

// Struct for the data of the symbol
typedef struct symbol_data {
    bool defined; // initialized

    // Variable
    bool is_var;
    var_type var_type;
    data_type data_type;

    // Function
    bool is_func;
    data_type return_type;

    // Parameter
    bool is_param;
    const char *param_name;
    data_type param_type;
    int param_order;

} sym_data;

// Struct for the AVL tree
typedef struct avl_tree {
    char *key;          // name of the symbol (identifier)
    sym_data data;
    struct avl_tree *left;
    struct avl_tree *right;
    int height; 
    int nickname; // for renaming purposes for codegen, number of special characters in the name     
} AVL_tree;


/**
 * @brief Symbol table initialization
 * 
 * @param tree Pointer to the root of the tree
 */
void symtable_init(AVL_tree **tree);


sym_data data_init();
 

/**
 * @brief Symbol insertion
 * 
 * @param tree Pointer to the root of the tree
 * @param key Key of the node
 */
void symtable_insert(AVL_tree **tree, char *key);

/**
 * @brief Symbol search in the symbol table
 * 
 * @param tree Pointer to the root of the tree
 * @param key Key of the node
 * @return AVL_tree* Pointer to the node (symbol)
 */
AVL_tree *symtable_search(AVL_tree *tree, char *key);


/**
 * @brief Symbol lookup
 * 
 * @param tree Pointer to the root of the tree
 * @param key Key of the node
 * @return sym_data* Pointer to the data of the node
 */
sym_data *symtable_lookup(AVL_tree *tree, char *key);


/**
 * @brief Get the geight of the tree
 * 
 * @param tree Pointer to the root of the tree
 * @return int Height of the tree, 0 if failed
 */
int height(AVL_tree *tree);


/**
 * @brief Get the balance of the tree
 * 
 * @param tree Pointer to the root of the tree
 * @return int Balance of the tree, 0 if failed
 */
int get_balance(AVL_tree *tree);


/**
 * @brief Right rotation
 * 
 * @param tree Pointer to the root of the tree
 */
void right_rotate(AVL_tree **tree);


/**
 * @brief Left rotation
 * 
 * @param tree Pointer to the root of the tree
 */
void left_rotate(AVL_tree **tree);


/**
 * @brief Symbol deletion
 * 
 * @param tree Pointer to the root of the tree
 * @param key Key of the node
 */
void symtable_delete(AVL_tree **tree, char *key);


/**
 * @brief Symbol table dispose
 * 
 * @param tree Pointer to the root of the tree
 */
void symtable_dispose(AVL_tree **tree);


/**
 * @brief Prints the keys in the symbol table in order
 * 
 * @param tree Pointer to the root of the tree
 */
void inorder(AVL_tree **tree);

AVL_tree *symtable_find_param(AVL_tree *tree, int order_arg);
bool validation_of_id(AVL_tree *tree, char *key, int order_arg);

#endif //IFJ_SYMTABLE_H
