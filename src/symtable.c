/**
 * @file symtable.c
 *
 * IFJ23 compiler
 *
 * @brief Symbol table using self-balancing binary search tree (AVL tree)
 *
 * @author Adam Valík <xvalik05>
 * @author Marek Effenberger <xeffen00>
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



sym_data *data_init(sym_data *data){
    data = (sym_data *)allocate_memory(sizeof(sym_data));

    data->defined = false; // initialized

    data->is_var = false;
    data->var_type = VAR;
    data->data_type = NIL;

    data->is_func = false;
    data->return_type = NIL;

    data->is_param = false;
    data->param_name = NULL;
    data->param_type = NIL;
    data->param_order = 0;
    return data;
}

sym_data *set_data_var(bool initialized, data_type data_type, var_type var_type) { 
    sym_data *data = data_init(data);
    data->is_var = true;
    data->defined = initialized;
    data->data_type = data_type;
    data->var_type = var_type;
    return data;
}

sym_data *set_data_func(data_type return_type) {
    sym_data *data = data_init(data);
    data->is_func = true;
    data->defined = true;
    data->return_type = return_type;
    return data;
}

sym_data *set_data_param(data_type param_type, char *param_name, int param_order) {
    sym_data *data = data_init(data);
    data->defined = true;
    data->is_param = true;
    data->param_type = param_type;
    data->param_name = (char *)allocate_memory(sizeof(char) * (strlen(param_name) + 2));
    strcpy(data->param_name, param_name);
    data->param_order = param_order;
    return data;
}


AVL_tree *symtable_search(AVL_tree *tree, char *key) {
    if (tree == NULL || key == NULL) {
        return NULL;
    }
    else if (strcmp(tree->key, key) == 0) {
        return tree;
    } 
    else if (strcmp(tree->key, key) > 0) {
        return symtable_search(tree->left, key);
    }
    else { // strcmp(tree->key, key) < 0
        return symtable_search(tree->right, key);
    }
}


AVL_tree *symtable_find_param(AVL_tree *tree, int order_arg) {
    if (tree == NULL) {
        return NULL;
    }
    else if (tree->data->is_param && tree->data->param_order == order_arg) {
        return tree;
    }
    else {
        AVL_tree *left = symtable_find_param(tree->left, order_arg);
        AVL_tree *right = symtable_find_param(tree->right, order_arg);
        if (left != NULL) {
            return left;
        }
        else if (right != NULL) {
            return right;
        }
        else {
            return NULL;
        }
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


void symtable_insert(AVL_tree **tree, char *key, sym_data *data) {
    if (*tree == NULL) { // insert first to an empty tree
        *tree = (AVL_tree *)allocate_memory(sizeof(AVL_tree));
        (*tree)->key = key;
        (*tree)->data = data;
        (*tree)->left = NULL;
        (*tree)->right = NULL;
        (*tree)->height = 0;
        (*tree)->nickname = 0;
    }
    else if (strcmp((*tree)->key, key) > 0) {
        symtable_insert(&((*tree)->left), key, data);
    }
    else if (strcmp((*tree)->key, key) < 0) {
        symtable_insert(&((*tree)->right), key, data);
    }
    else { // strcmp((*tree)->key, key) == 0
        error_exit(ERROR_SEM_UNDEF_FUN, "SYMTABLE", "Key already exists in the symbol table.");
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
            if ((*tree)->data->param_name) {
                free((*tree)->data->param_name);
                (*tree)->data->param_name = NULL;
            }
            if ((*tree)->data) {
                free((*tree)->data);
                (*tree)->data = NULL;
            }
            free(*tree);
            *tree = NULL;
        }
    }
}
