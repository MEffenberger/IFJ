/**
 * @file token_stack.h
 * 
 * IFJ compiler 2023
 * 
 * @brief Token stack header file
 * @author Dominik Horut <xhorut01>
 */

#ifndef TOKEN_STACK_H
#define TOKEN_STACK_H

#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"

/**
 * @brief A stack data structure for tokens.
 * 
 * This struct represents a stack data structure for tokens. It contains a dynamically allocated array of tokens,
 * along with the current size and capacity of the stack.
 */
typedef struct {
    token_t** token_array;
    int size;
    int capacity;
} token_stack;



/**
 * @brief Initializes a token stack with a given capacity.
 * 
 * This function initializes a token stack with a given capacity. It allocates memory for the token array and sets
 * the size and capacity of the stack to 0 and the given capacity, respectively.
 * 
 * @param token_stack A pointer to the token stack to be initialized.
 */
void init(token_stack* token_stack);

/**
 * @brief Pushes a token onto the top of the stack.
 * 
 * This function pushes a token onto the top of the stack. If the stack is full, it doubles the capacity of the stack
 * before pushing the token.
 * 
 * @param token_stack A pointer to the token stack.
 * @param token The token to be pushed onto the stack.
 */
void push(token_stack* token_stack, token_t* token);

/**
 * @brief Pops a token from the top of the stack.
 * 
 * This function pops a token from the top of the stack and returns it. If the stack is empty, it returns a token with
 * type TOKEN_EOF.
 * 
 * @param token_stack A pointer to the token stack.
 * @return The token popped from the top of the stack.
 */
void pop(token_stack* token_stack);

/**
 * @brief Returns the token at the top of the stack.
 * 
 * This function returns the token at the top of the stack without removing it from the stack. If the stack is empty,
 * it returns a token with type TOKEN_EOF.
 * 
 * @param token_stack A pointer to the token stack.
 * @return The token at the top of the stack.
 */
token_t* top(token_stack* token_stack);

/**
 * @brief Checks if the stack is empty.
 * 
 * This function checks if the stack is empty and returns true if it is, false otherwise.
 * 
 * @param token_stack A pointer to the token stack.
 * @return True if the stack is empty, false otherwise.
 */
bool is_empty(token_stack* token_stack);

/**
 * @brief Disposes of a token stack.
 * 
 * This function disposes of a token stack by freeing the memory allocated for the token array and setting the size
 * and capacity of the stack to 0.
 * 
 * @param token_stack A pointer to the token stack to be disposed of.
 */
void dispose_stack(token_stack* token_stack);

#endif /* TOKEN_STACK_H */