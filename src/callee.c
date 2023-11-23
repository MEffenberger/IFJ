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

callee_t* init_callee(const char* name, data_type return_type) {
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

    callee->return_type = return_type;
    callee->arg_count = 0;
    callee->args_names = NULL; // Initialize args_names to NULL
    callee->args_types = NULL; // Initialize args_types to NULL

    return callee;
}

void insert_callee_into_list(callee_list_t* list, const char* name, data_type return_type) {
    // Initialize a new callee
    callee_t* new_callee = init_callee(name, return_type);
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

    callee->args_names = realloc(callee->args_names, (callee->arg_count + 1) * sizeof(char*));
    if (callee->args_names == NULL) {
        // Handle memory reallocation failure
        return;
    }

    // Increment the arg_count
    callee->arg_count++;

    // Allocate memory for the new name
    callee->args_names[callee->arg_count] = (char*)malloc(strlen(name) + 1);
    callee->args_names[callee->arg_count] = strcpy(callee->args_names[callee->arg_count], name);

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


void callee_list_dispose(callee_list_t* list) {
    // Iterate over the list and free all callees
    callee_list_t* current = list;
    while (current != NULL) {
        // Free the callee
        callee_dispose(current->callee);

        // Move to the next node
        callee_list_t* next = current->next;
        free(current);
        current = next;
    }
}

void callee_dispose(callee_t* callee) {
    // Free the name
    free(callee->name);

    // Free the args_names
    for (int i = 0; i < callee->arg_count; i++) {
        free(callee->args_names[i]);
    }
    free(callee->args_names);
    free(callee->args_types);

    // Free the callee
    free(callee);
}
