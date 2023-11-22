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

#ifndef IFJ_ERROR_H
#define IFJ_ERROR_H

#include <stddef.h>

// Error codes
typedef enum e_error_code {
    // Error related to lexical analysis, bad structure of the lexeme
    ERROR_LEX = 1,
    // Error related to syntax analysis, wrong syntax of the source code, missing header, etc.
    ERROR_SYN = 2,
    // Error related to semantic analysis, undefined function, redefinition of a variable, etc.
    ERROR_SEM_UNDEF_FUN = 3,
    // Error related to semantic analysis, wrong type of a variable, wrong number of parameters, wrong return value etc.
    ERROR_SEM_TYPE = 4,
    // Error related to semantic analysis, use of undefined/uninitialized variable, etc.
    ERROR_SEM_UNDEF_VAR = 5,
    // Error related to semantic analysis, missing/redundant return value, etc.
    ERROR_SEM_EXPR_RET = 6,
    // Error related to semantic analysis, type compatibility in expressions, etc.
    ERROR_SEM_EXPR_TYPE = 7,
    // Error related to semantic analysis, wrong parameter type where there's no possibility to derive the type, etc.
    ERROR_SEM_DERIV = 8,
    // Error related to semantic analysis, other semantic errors not covered by the previous error codes.
    ERROR_SEM_OTHER = 9,
    // Error unrelated to input files, memory allocation, etc.
    ERROR_INTERNAL = 99
} error_code_t;

// Data types for allocated pointers
typedef enum e_alloc_type {
    BASIC,
    VECTOR,
    SYMTABLE,
    FOREST
} alloc_type;

// Structure to represent a node in the stack of allocated pointers
typedef struct allocated_pointer {
    alloc_type type;
    void *ptr;
    struct allocated_pointer *next;
} alloc_ptr;

// Global top of the stack for allocated pointers
extern alloc_ptr *allocated_pointers;

/**
 * @brief Function to push an allocated pointer onto the stack
 * 
 * @param ptr to newly allocated memory
 * @param type type of the allocated memory (BASIC, VECTOR, TOKEN, ...)
 */
void push_alloc_ptr(void *ptr, alloc_type type);


/**
 * @brief Function free allocated pointers from the stack
 * 
 */
void free_alloc_memory();

/**
 * @brief Function to allocate memory, check for malloc errors and push it onto the stack
 * 
 * @note example: AVL_tree *tree = (AVL_tree *) allocate_memory(sizeof(AVL_tree), "tree node", SYMTABLE);
 * @param size Size of the memory to be allocated
 * @param whats_allocated String describing what is being allocated
 * @param type Type of the allocated memory (BASIC, VECTOR, TOKEN, ...)
 * @return void* Pointer to the allocated memory
 */
void *allocate_memory(size_t size, const char* whats_allocated, alloc_type type);


/**
 * @brief Error handler for IFJ23 compiler
 * 
 * @param error_code Error code
 * @param module Module where the error occured
 * @param message Custom formatted error message
 */
void error_exit(error_code_t error_code, const char* module, const char* message);


#endif //IFJ_ERROR_H
