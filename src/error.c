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

void push_alloc_ptr(void *ptr, alloc_type type) {
    alloc_ptr *new = malloc(sizeof(alloc_ptr));
    if (new == NULL) {
        error_exit(ERROR_INTERNAL, "MALLOC", "Memory allocation for stack node failed.");
    }
    new->type = type;
    new->ptr = ptr;
    new->next = allocated_pointers;
    allocated_pointers = new;
}

/*void free_alloc_memory() {
    while (allocated_pointers != NULL) {
        alloc_ptr *tmp = allocated_pointers;
        switch (tmp->type) {
            case NONE:
                break;
            case BASIC:
                free(tmp->ptr);
                tmp->ptr = NULL;
                break;
            case VECTOR:
                vector_dispose(tmp->ptr);
                break;
            case SYMTABLE:
                symtable_dispose(tmp->ptr);
                break;
            case FOREST:
                forest_dispose(tmp->ptr);
                break;
            //case: // tbd other types
            default:
                break;
        }
        allocated_pointers = allocated_pointers->next;
        free(tmp);
    }
}*/


void *allocate_memory(size_t size, const char* whats_allocated, alloc_type type) {
    void* ptr = malloc(size);
    push_alloc_ptr(ptr, type);
    if (ptr == NULL) {
        char error_message[100];
        sprintf(error_message, "Memory allocation for %s failed.", whats_allocated);
        error_exit(ERROR_INTERNAL, "MALLOC", error_message);
    }
    return ptr;
}


void error_exit(error_code_t error_code, const char* module, const char* message) { 
    fprintf(stderr, "%s: %s\n", module, message);
    //free_alloc_memory(); // free all allocated memory
    exit(error_code);
}
