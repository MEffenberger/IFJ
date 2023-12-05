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
    callee_list_t* list = (callee_list_t*)allocate_memory(sizeof(callee_list_t));

    list->callee = NULL;
    list->next = NULL;

    return list;
}

callee_t* init_callee(const char* name) {
    callee_t* callee = (callee_t*)allocate_memory(sizeof(callee_t));

    callee->name = (char*)allocate_memory(strlen(name) + 1);
    callee->name = strcpy(callee->name, name);

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
    
    // Find the last node in the list
    callee_list_t* current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->callee = new_callee;

    // Create a new node for the next callee (if any)
    current->next = init_callee_list();
}

void insert_name_into_callee(callee_t* callee, char* name) {

    callee->args_names = reallocate_memory(callee->args_names, (callee->arg_count + 2) * sizeof(char*));
 
    // Increment the arg_count
    arg_counter++;
    callee->arg_count = arg_counter;

    callee->args_names[callee->arg_count] = (char*)allocate_memory(strlen(name) + 1);
    callee->args_names[callee->arg_count] = name;
}


void insert_type_into_callee(callee_t* callee, data_type type) {
    callee->args_types = reallocate_memory(callee->args_types, (callee->arg_count + 1) * sizeof(data_type));
    callee->args_types[callee->arg_count] = type; 
}


void insert_bool_into_callee(callee_t* callee, bool is_initialized) {
    callee->args_initialized = reallocate_memory(callee->args_initialized, (callee->arg_count + 1) * sizeof(bool));
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

