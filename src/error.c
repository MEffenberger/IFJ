/**
 * @file error.c
 *
 * IFJ23 compiler
 *
 * @brief Error handling header file
 *s
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


void *allocate_memory(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        error_exit(ERROR_INTERNAL, "MALLOC", "Memory allocation failed.");
    }
    return ptr;
}

void *reallocate_memory(void *to_be_reallocated, size_t size) {
    void* ptr = realloc(to_be_reallocated, size);
    if (ptr == NULL) {
        error_exit(ERROR_INTERNAL, "REALLOC", "Memory reallocation failed.");
    }
    return ptr;
}


void error_exit(error_code_t error_code, const char* module, const char* message) { 
    fprintf(stderr, "%s: %s\n", module, message);
    exit(error_code);
}
