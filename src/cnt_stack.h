/**
 * @file cnt_stack.h
 * 
 * IFJ compiler 2023
 * 
 * @brief Counter stack header file
 * @author Dominik Horut <xhorut01>
 */

#ifndef CNT_STACK_H
#define CNT_STACK_H

#include <stdlib.h>
#include <stdbool.h>
#include "codegen.h"

/**
 * @brief A stack data structure for cnts.
 * 
 * This struct represents a stack data structure for cnts. It contains a dynamically allocated array of cnts,
 * along with the current size and capacity of the stack.
 */
typedef struct cntstack {
    int* cnt_array;
    int size;
    int capacity;
} cnt_stack_t;

extern cnt_stack_t *cnt_stack;

/**
 * @brief Initializes a cnt stack with a given capacity.
 * 
 * This function initializes a cnt stack with a given capacity. It allocates memory for the cnt array and sets
 * the size and capacity of the stack to 0 and the given capacity, respectively.
 * 
 * @param cnt_stack A pointer to the cnt stack to be initialized.
 */
void cnt_init(cnt_stack_t* cnt_stack);

/**
 * @brief Pushes a cnt onto the top of the stack.
 * 
 * This function pushes a cnt onto the top of the stack. If the stack is full, it doubles the capacity of the stack
 * before pushing the cnt.
 * 
 * @param cnt_stack A pointer to the cnt stack.
 * @param cnt The cnt to be pushed onto the stack.
 */
void cnt_push(cnt_stack_t* cnt_stack);

/**
 * @brief Pops a cnt from the top of the stack.
 * 
 * This function pops a cnt from the top of the stack and returns it. If the stack is empty, it returns a cnt with
 * type cnt_EOF.
 * 
 * @param cnt_stack A pointer to the cnt stack.
 * @return The cnt popped from the top of the stack.
 */
void cnt_pop(cnt_stack_t* cnt_stack);

/**
 * @brief Returns the cnt at the top of the stack.
 * 
 * This function returns the cnt at the top of the stack without removing it from the stack. If the stack is empty,
 * it returns a cnt with type cnt_EOF.
 * 
 * @param cnt_stack A pointer to the cnt stack.
 * @return The cnt at the top of the stack.
 */
int cnt_top(cnt_stack_t* cnt_stack);

/**
 * @brief Checks if the stack is empty.
 * 
 * This function checks if the stack is empty and returns true if it is, false otherwise.
 * 
 * @param cnt_stack A pointer to the cnt stack.
 * @return True if the stack is empty, false otherwise.
 */
bool cnt_is_empty(cnt_stack_t* cnt_stack);

/**
 * @brief Disposes of a cnt stack.
 * 
 * This function disposes of a cnt stack by freeing the memory allocated for the cnt array and setting the size
 * and capacity of the stack to 0.
 * 
 * @param cnt_stack A pointer to the cnt stack to be disposed of.
 */
void cnt_dispose_stack(cnt_stack_t* cnt_stack);

void cnt_print_stack(cnt_stack_t* cnt_stack);

#endif /* cnt_STACK_H */