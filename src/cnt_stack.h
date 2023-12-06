/**
 * @file cnt_stack.h
 * 
 * IFJ23 compiler
 * 
 * @brief Stack of integers for nested if&else node naming 
 * 
 * @author Adam Valik <xvalik05>
 * @author Marek Effenberger <xeffen00>
 */

#ifndef CNT_STACK_H
#define CNT_STACK_H

#include <stdbool.h>

//default stack size
#define STACK_SIZE 4

// Structure representing a stack holding integers
typedef struct cntstack {
    int* cnt_array;
    int size;
    int capacity;
} cnt_stack_t;

extern cnt_stack_t *cnt_stack; // for parser

/**
 * @brief Initializes a stack with a given capacity
 * 
 * @param cnt_stack A pointer to the cnt stack to be initialized
 */
void cnt_init(cnt_stack_t* cnt_stack);

/**
 * @brief Pushes a integer onto the top of the stack
 * 
 * @param cnt_stack A pointer to the cnt stack
 * @param cnt The cnt to be pushed onto the stack
 */
void cnt_push(cnt_stack_t* cnt_stack);

/**
 * @brief Pops a integer from the top of the stack
 * 
 * @param cnt_stack A pointer to the cnt stack
 * @return The cnt popped from the top of the stack
 */
void cnt_pop(cnt_stack_t* cnt_stack);

/**
 * @brief Returns the integer at the top of the stack
 * 
 * @param cnt_stack A pointer to the cnt stack
 * @return The cnt at the top of the stack
 */
int cnt_top(cnt_stack_t* cnt_stack);

/**
 * @brief Dispose of the stack
 * 
 * @param cnt_stack A pointer to the cnt stack to be disposed of
 */
void cnt_dispose_stack(cnt_stack_t* cnt_stack);

#endif /* cnt_STACK_H */