/**
 * @file callee.c
 *
 * IFJ23 compiler
 *
 * @brief A structure to represent individual function calls and their parameters
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */

#include "callee.h"

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

callee_t* init_callee(const char* name, data_type return_type, int param_count) {
    callee_t* callee = (callee_t*)malloc(sizeof(callee_t));
    if (callee == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Allocate memory for the name
    callee->name = strdup(name); // Don't forget to include <string.h>
    if (callee->name == NULL) {
        // Handle memory allocation failure
        free(callee);
        return NULL;
    }

    callee->return_type = return_type;
    callee->param_count = param_count;
    callee->ids = NULL; // Initialize ids to NULL

    return callee;
}

void insert_callee_into_list(callee_list_t* list, const char* name, data_type return_type, int param_count) {
    // Initialize a new callee
    callee_t* new_callee = init_callee(name, return_type, param_count);
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

void insert_into_callee(callee_t* callee, char* id) {
    // Assuming that callee->ids is initially NULL or allocated dynamically
    // You need to reallocate the array to accommodate the new id

    callee->ids = realloc(callee->ids, (callee->param_count + 1) * sizeof(char*));
    if (callee->ids == NULL) {
        // Handle memory reallocation failure
        return;
    }

    // Allocate memory for the new id
    callee->ids[callee->param_count] = strdup(id); // Don't forget to include <string.h>
    if (callee->ids[callee->param_count] == NULL) {
        // Handle memory allocation failure
        return;
    }

    // Increment the param_count
    callee->param_count++;
}




