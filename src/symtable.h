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

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#include <stdbool.h>


typedef enum e_data_type {
    T_INT,
    T_DOUBLE,
    T_STRING,
    T_INT_Q,
    T_DOUBLE_Q,
    T_STRING_Q,
    T_NIL, // void function
    T_VOID
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
    // ukládat hodnoty proměnných do TS?
    // int int_value;
    // double double_value;
    // char *string_value;

    // Function
    bool is_func;
    data_type return_type;

    // Parameter
    bool is_param;
    const char *param_name;
    data_type param_type;

} sym_data;

// Struct for the AVL tree
typedef struct avl_tree {
    char *key;          // name of the symbol (identifier)
    sym_data data;
    struct avl_tree *left;
    struct avl_tree *right;
    int height;      
} AVL_tree;


/**
 * @brief Symbol table initialization
 * 
 * @param tree Pointer to the root of the tree
 */
void symtable_init(AVL_tree **tree);

/**
 * @brief Data initialization
 * 
 * @param data Pointer to the data
 */
void data_init(sym_data *data);

/**
 * @brief Set the variable's data
 * 
 * @param data Data to be set
 * @param initialized If the variable is initialized
 * @param data_type Data type of the variable (T_INT/DOUBLE/...)
 * @param var_type LET/VAR
 * @param int_value Integer value when T_INT(_Q)
 * @param double_value Double value when T_DOUBLE(_Q)
 * @param string_value String value when T_STRING(_Q)
 */
sym_data set_data_var(sym_data data, bool initialized, data_type data_type, var_type var_type); //int int_value, double double_value, char *string_value);

/**
 * @brief Set the function's data
 * 
 * @param data Data to be set
 * @param return_type Return type of the function
 */
sym_data set_data_func(sym_data *data, data_type return_type);

/**
 * @brief Set the parameter's data
 * 
 * @param data Data to be set
 * @param param_type Data type of the parameter
 * @param param_name Name of the parameter
 */
sym_data set_data_param(sym_data *data, data_type param_type, char *param_name);

/**
 * @brief Symbol insertion
 * 
 * @param tree Pointer to the root of the tree
 * @param key Key of the node
 * @param value Value of the node
 */
void symtable_insert(AVL_tree **tree, char *key, sym_data data);

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


#endif //IFJ_SYMTABLE_H
