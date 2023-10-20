/**
 * @file error.h
 *
 * IFJ23 compiler
 *
 * @brief Error handling header file
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */

#include "error.h"

void push_alloc_ptr(void *ptr) {
    alloc_ptr_t *new = (alloc_ptr_t *)malloc(sizeof(alloc_ptr_t));
    if (new == NULL) {
        error_exit(ERROR_INTERNAL, "MALLOC", "Memory allocation for stack node failed.");
    }
    new->ptr = ptr;
    new->next = allocated_pointers;
    allocated_pointers = new;
}

void free_alloc_memory() {
    while (allocated_pointers != NULL) {
        alloc_ptr_t *tmp = allocated_pointers;
            // TODO: if allocated_pointers is token_t, needs special free of its contents ?
        allocated_pointers = allocated_pointers->next;
        free(tmp->ptr);
        free(tmp);
    }
}

void error_exit(error_code_t error_code, const char* module, const char* message) { 
    fprintf(stderr, "%s: %s\n", module, message);
    free_alloc_memory(); // Free all allocated memory
    exit(error_code);
}
