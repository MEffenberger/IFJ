/**
 * @file symtable.h
 *
 * IFJ23 compiler
 *
 * @brief Symbol table using self-balancing binary search tree (AVL tree)
 *
 * @author Adam Valík <xvalik05>
 * @author Marek Effenberger <xeffen00>
 */

#ifndef IFJ_SYMTABLE_H
#define IFJ_SYMTABLE_H

#include <stdbool.h>     
#include "error.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Data types
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

// Variable type (modifiable or unmodifiable)
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
    char *param_name;
    data_type param_type;
    int param_order;
} sym_data;

// Struct for the AVL tree
typedef struct avl_tree {
    char *key;  // name of the symbol (identifier)
    sym_data *data;
    struct avl_tree *left;
    struct avl_tree *right;
    int height; 
    int nickname; // for renaming purposes (codegen)
} AVL_tree;


/**
 * @brief Data initialization
 * 
 * @param data Pointer to the data
 */
sym_data *data_init(sym_data *data);


/**
 * @brief Set the variable's data
 * 
 * @param initialized If the variable is initialized
 * @param data_type Data type of the variable 
 * @param var_type modifiable or unmodifiable (VAR/LET)
 */
sym_data *set_data_var(bool initialized, data_type data_type, var_type var_type);


/**
 * @brief Set the function's data
 * 
 * @param return_type Return type of the function
 */
sym_data *set_data_func(data_type return_type);


/**
 * @brief Set the parameter's data
 * 
 * @param param_type Data type of the parameter
 * @param param_name Name of the parameter
 * @param param_order Order of the parameter
 */
sym_data *set_data_param(data_type param_type, char *param_name, int param_order);


/**
 * @brief Symbol search in the symbol table
 * 
 * @param tree Pointer to the root of the tree
 * @param key Key of the node
 * @return AVL_tree* Pointer to the node (symbol)
 */
AVL_tree *symtable_search(AVL_tree *tree, char *key);


/**
 * @brief Traverse the tree and find the parameter with the given order
 *
 * @param tree Pointer to the root of the tree
 * @param order_arg Order of the parameter
 * @return AVL_tree* Pointer to the node (symbol)
 */
AVL_tree *symtable_find_param(AVL_tree *tree, int order_arg);


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
 * @brief Insertion of the symbol into the symbol table
 * 
 * @param tree Pointer to the root of the tree
 * @param key Key of the node
 * @param data Data of the node
 */
void symtable_insert(AVL_tree **tree, char *key, sym_data *data);


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


#endif //IFJ_SYMTABLE_H
