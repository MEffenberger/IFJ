/**
 * @file callee.c
 *
 * IFJ23 compiler
 *
 * @brief A structure to represent indivnameual function calls and their parameters
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
#include <stdbool.h>

int arg_counter = 0;

callee_list_t* init_callee_list() {
    callee_list_t* list = (callee_list_t*)malloc(sizeof(callee_list_t));
    if (list == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    list->callee = NULL;
    list->next = NULL;

    return list;
}

callee_t* init_callee(const char* name) {
    callee_t* callee = (callee_t*)malloc(sizeof(callee_t));
    if (callee == NULL) {
        // Handle memory allocation failure
        return NULL;
    }


    // Allocate memory for the name

    callee->name = (char*)malloc(strlen(name) + 1);
    callee->name = strcpy(callee->name, name);

    if (callee->name == NULL) {
        // Handle memory allocation failure
        free(callee);
        return NULL;
    }

    callee->return_type = UNKNOWN;
    arg_counter = 0;
    callee->arg_count = arg_counter;
    callee->args_names = NULL; // Initialize args_names to NULL
    callee->args_types = NULL; // Initialize args_types to NULL
    callee->args_initialized = NULL; // Initialize args_initialized to NULL

    return callee;
}

void insert_callee_into_list(callee_list_t* list, const char* name) {
    // Initialize a new callee
    callee_t* new_callee = init_callee(name);
    if (new_callee == NULL) {
        // Handle initialization failure
        return;
    }
    // Find the last node in the list
    callee_list_t* current = list;
    while (current->next != NULL) {
        current = current->next;
    }

    // Set the new callee for the last list node
    current->callee = new_callee;

    // Create a new node for the next callee (if any)
    current->next = init_callee_list();
    if (current->next == NULL) {
        // Handle node creation failure
        free(new_callee->name);
        free(new_callee);
        return;
    }
}

void insert_name_into_callee(callee_t* callee, char* name) {
    // Assuming that callee->args_names is initially NULL or allocated dynamically
    // You need to reallocate the array to accommodate the new name

    callee->args_names = realloc(callee->args_names, (callee->arg_count + 2) * sizeof(char*));
    if (callee->args_names == NULL) {
        // Handle memory reallocation failure
        return;
    }

    // Increment the arg_count
    arg_counter++;
    callee->arg_count = arg_counter;


    // Allocate memory for the new name
    callee->args_names[callee->arg_count] = (char*)malloc(strlen(name) + 1);
    callee->args_names[callee->arg_count] = name;

    if (callee->args_names[callee->arg_count] == NULL) {
        // Handle memory allocation failure
        return;
    }
}


void insert_type_into_callee(callee_t* callee, data_type type) {
    // Assuming that callee->arg_types is initially NULL or allocated dynamically
    // You need to reallocate the array to accommodate the new name

    callee->args_types = realloc(callee->args_types, (callee->arg_count + 1) * sizeof(data_type));
    if (callee->args_types == NULL) {
        // Handle memory reallocation failure
        return;
    }

    // Allocate memory for the new name
    callee->args_types[callee->arg_count] = type; 
}


void insert_bool_into_callee(callee_t* callee, bool is_initialized) {
    // Assuming that callee->arg_types is initially NULL or allocated dynamically
    // You need to reallocate the array to accommodate the new name

    callee->args_initialized = realloc(callee->args_initialized, (callee->arg_count + 1) * sizeof(bool));
    if (callee->args_initialized == NULL) {
        // Handle memory reallocation failure
        return;
    }

    callee->args_initialized[callee->arg_count] = is_initialized; 
}



void callee_list_dispose(callee_list_t *first) {
    callee_list_t *current = first;
    callee_list_t *next;

    while (current != NULL) {
        if (current->callee) {
            callee_t *callee = current->callee;

            // Free callee structure members
            if (callee->name) {
                free(callee->name);
                callee->name = NULL;
            }

            if (callee->args_names) {
                for (int i = 0; i < callee->arg_count; ++i) {
                    if (callee->args_names[i]) {
                        free(callee->args_names[i]);
                        callee->args_names[i] = NULL;
                    }
                }
                free(callee->args_names);
                callee->args_names = NULL;
            }

            if (callee->args_initialized) {
                free(callee->args_initialized);
                callee->args_initialized = NULL;
            }

            if (callee->args_types) {
                free(callee->args_types);
                callee->args_types = NULL;
            }

            // Free callee structure itself
            free(callee);
            current->callee = NULL;
        }

        next = current->next;
        free(current);
        current = next;
    }
}

