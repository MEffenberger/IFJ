/**
 * @file symtable.c
 *
 * IFJ23 compiler
 *
 * @brief Symbol table using AVL tree: implementation
 *
 * @author Adam Valík <xvalik05>
 * @author Marek Effenberger <xeffen00>
 */

#include "symtable.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// void symtable_init(AVL_tree **tree) {
//     *tree = NULL;
// }

void data_init(sym_data *data){
    data->declared = true;
    data->defined = false; // initialized

    data->is_var = false;
    data->var_type = VAR;
    data->data_type = T_NIL;
    data->int_value = 0;
    data->double_value = 0.0;
    data->string_value = NULL;

    data->is_func = false;
    data->return_type = T_NIL;

    data->is_param = false;
    data->param_name = NULL;
    data->param_type = T_NIL;
}

sym_data set_data_var(sym_data data, bool initialized, data_type data_type, var_type var_type, int int_value, double double_value, char *string_value) {
    data_init(&data);
    data.defined = initialized;
    data.data_type = data_type;
    data.var_type = var_type;
    data.int_value = int_value;
    data.double_value = double_value;
    data.string_value = string_value;
    return data;
}

sym_data set_data_func(sym_data *data, bool defined, data_type return_type) {
    data_init(data);
    data->defined = defined;
    data->return_type = return_type;
    return *data;
}

sym_data set_data_param(sym_data *data, data_type param_type, char *param_name) {
    data_init(data);
    data->param_type = param_type;
    data->param_name = param_name;
    return *data;
}


AVL_tree *symtable_search(AVL_tree *tree, char *key) {
    if (tree == NULL || key == NULL) {
        return NULL;
    }
    else if (strcmp(tree->key, key) == 0) {
        printf("found %s\n", key);
        return tree;
    } 
    else if (strcmp(tree->key, key) > 0) {
        return symtable_search(tree->left, key);
    }
    else { // strcmp(tree->key, key) < 0
        return symtable_search(tree->right, key);
    }
}

sym_data *symtable_lookup(AVL_tree *tree, char *key) {
    if (tree == NULL || key == NULL) {
        return NULL;
    }
    else if (strcmp(tree->key, key) == 0) {
        return &(tree->data);
    } 
    else if (strcmp(tree->key, key) > 0) {
        return symtable_lookup(tree->left, key);
    }
    else { // strcmp(tree->key, key) < 0
        return symtable_lookup(tree->right, key);
    }
}


int height(AVL_tree *tree) {
    if (tree == NULL) {
        return 0;
    }
    else {
        return tree->height;
    }
}


int get_balance(AVL_tree *tree) {
    if (tree == NULL) {
        return 0;
    }
    else {
        return (height(tree->left) - height(tree->right));
    }
}


void right_rotate(AVL_tree **tree) {
    AVL_tree *x = *tree;
    AVL_tree *y = x->left;
    AVL_tree *T2 = y->right;

    y->right = x;
    x->left = T2;

    x->height = MAX(height(x->left), height(x->right)) + 1;
    y->height = MAX(height(y->left), height(y->right)) + 1;

    *tree = y;
}


void left_rotate(AVL_tree **tree) {
    AVL_tree *x = *tree;
    AVL_tree *y = x->right;
    AVL_tree *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = MAX(height(x->left), height(x->right)) + 1;
    y->height = MAX(height(y->left), height(y->right)) + 1;

    *tree = y;
}


void symtable_insert(AVL_tree **tree, char *key, sym_data data) {
    if (*tree == NULL) { // insert first to an empty tree
        *tree = (AVL_tree *)allocate_memory(sizeof(AVL_tree), "tree node", SYMTABLE);
        (*tree)->key = key;
        (*tree)->data = data;
        (*tree)->left = NULL;
        (*tree)->right = NULL;
        (*tree)->height = 0;
    }
    else if (strcmp((*tree)->key, key) > 0) {
        symtable_insert(&((*tree)->left), key, data);
    }
    else if (strcmp((*tree)->key, key) < 0) {
        symtable_insert(&((*tree)->right), key, data);
    }
    else { // strcmp((*tree)->key, key) == 0
        error_exit(ERROR_SEM_OTHER, "SYMTABLE", "Key already exists in the symbol table.");
    }

    // correct the height and continue with balancing
    (*tree)->height = 1 + MAX(height((*tree)->left), height((*tree)->right));

    // check if the tree is balanced: 0 balanced, -1 left higher, 1 right higher
    int balance = get_balance(*tree);
    if (balance != 0) { // if the tree is not balanced

        // LL case
        if (balance > 1 && strcmp((*tree)->key, key) > 0) {
            right_rotate(tree);
        }
        // RR case
        else if (balance < -1 && strcmp((*tree)->key, key) < 0) {
            left_rotate(tree);
        }
        // LR case
        else if (balance > 1 && strcmp((*tree)->key, key) < 0) {
            left_rotate(&((*tree)->left));
            right_rotate(tree);
        }
        // RL case
        else if (balance < -1 && strcmp((*tree)->key, key) > 0) {
            right_rotate(&((*tree)->right));
            left_rotate(tree);
        }
    }
}


void symtable_delete(AVL_tree **tree, char *key) {
    if ((*tree) != NULL) {
        if (strcmp((*tree)->key, key) > 0) {
            symtable_delete(&((*tree)->left), key);
        }
        else if (strcmp((*tree)->key, key) < 0) {
            symtable_delete(&((*tree)->right), key);
        }
        else {  // strcmp((*tree)->key, key) == 0 -> found the node to delete
            if ((*tree)->left == NULL && (*tree)->right == NULL) { // no children
                free(*tree);
                *tree = NULL;
            }
            else if ((*tree)->left != NULL && (*tree)->right != NULL) { // both children
                AVL_tree *tmp = (*tree)->right;
                while (tmp->left != NULL) {
                    tmp = tmp->left;
                }
                (*tree)->key = tmp->key;
                (*tree)->data = tmp->data;
                symtable_delete(&((*tree)->right), tmp->key);
            }
            else { // only one child
                AVL_tree *tmp = *tree;
                if ((*tree)->left == NULL) { // only right child
                    *tree = (*tree)->right;
                }
                else if ((*tree)->right == NULL) { // only left child
                    *tree = (*tree)->left;
                }
                free(tmp);
            }
        }

        // if the tree is not empty, correct the height and continue with balancing
        if (*tree != NULL) { 
            (*tree)->height = 1 + MAX(height((*tree)->left), height((*tree)->right));
                    
            // check if the tree is balanced: 0 balanced, -1 left higher, 1 right higher
            int balance = get_balance(*tree);
            if (balance != 0) { // if the tree is not balanced

                // LL case
                if (balance > 1 && get_balance((*tree)->left) >= 0) {
                    right_rotate(tree);
                }
                // RR case
                else if (balance < -1 && get_balance((*tree)->right) <= 0) {
                    left_rotate(tree);
                }
                // LR case
                else if (balance > 1 && get_balance((*tree)->left) < 0) {
                    left_rotate(&((*tree)->left));
                    right_rotate(tree);
                }
                // RL case
                else if (balance < -1 && get_balance((*tree)->right) > 0) {
                    right_rotate(&((*tree)->right));
                    left_rotate(tree);
                }
            }
        }
    }
}


void symtable_dispose(AVL_tree **tree) {
    if ((*tree) != NULL) {
        if ((*tree)->left != NULL && (*tree)->right != NULL) {
            symtable_dispose(&((*tree)->left));
            symtable_dispose(&((*tree)->right));
        }
        else if ((*tree)->left != NULL) {
            symtable_dispose(&((*tree)->left));
        }
        else if ((*tree)->right != NULL) {
            symtable_dispose(&((*tree)->right));
        }
        else { // (*tree)->left == NULL && (*tree)->right == NULL
            free(*tree);
            *tree = NULL;
        }
    }
}

void inorder(AVL_tree **tree) { 
    if (*tree != NULL) {
        inorder(&((*tree)->left));
        printf("key: %s\n", (*tree)->key);
        inorder(&((*tree)->right));
    }
}
