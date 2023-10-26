/**
 * @file symtable.h
 *
 * IFJ23 compiler
 *
 * @brief Symbol table using AVL tree: header file
 *
 * @author Adam Valík <xvalik05>
 */

#ifndef IFJ_SYMTABLE_H
#define IFJ_SYMTABLE_H

#define MAX(a, b) ((a) > (b) ? (a) : (b))


typedef struct symbol_data {
    //TODO
} sym_data;


typedef struct avl_tree {
    char *key;          // name of the variable
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
 * @brief Symbol insertion
 * 
 * @param tree Pointer to the root of the tree
 * @param key Key of the node
 * @param value Value of the node
 */
void symtable_insert(AVL_tree **tree, char *key, sym_data data);


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
 * @param tree 
 */
void symtable_dispose(AVL_tree **tree);



// Compiler symbol tables often have multiple scopes (e.g., global scope, function scopes, nested scopes).
// A symbol table can be organized as a hierarchy of BSTs. Each scope corresponds to a separate tree, and parent scopes are stored higher in the hierarchy.
// When looking up a symbol, the compiler starts from the current scope and, if the symbol is not found, searches in the parent scopes.


#endif //IFJ_SYMTABLE_H