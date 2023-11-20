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
    printf("hovnicko\n");
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
    callee->arg_count = 0;
    callee->args_names = NULL; // Initialize args_names to NULL

    return callee;
}

void insert_callee_into_list(callee_list_t* list, const char* name, data_type return_type) {
    // Initialize a new callee
    printf("hovno\n");
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

void insert_into_callee(callee_t* callee, char* name) {
    // Assuming that callee->args_names is initially NULL or allocated dynamically
    // You need to reallocate the array to accommodate the new name

    callee->args_names = realloc(callee->args_names, (callee->arg_count + 1) * sizeof(char*));
    if (callee->args_names == NULL) {
        // Handle memory reallocation failure
        return;
    }

    // Allocate memory for the new name
    callee->args_names[callee->arg_count] = strdup(name); 
    if (callee->args_names[callee->arg_count] == NULL) {
        // Handle memory allocation failure
        return;
    }

    // Increment the arg_count
    callee->arg_count++;
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

    // Free the callee
    free(callee);
}
