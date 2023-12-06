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


///@brief Structure representing stack of tokens
typedef struct {
    token_t** token_array;
    int size;
    int capacity;
} token_stack;


/**
 * @brief Initializes a token stack with a given capacity
 * 
 * @param token_stack A pointer to the token stack to be initialized
 */
void stack_init(token_stack* token_stack);

/**
 * @brief Pushes a token onto the top of the stack
 * 
 * @param token_stack A pointer to the token stack
 * 
 * @param token The token to be pushed onto the stack
 */
void stack_push(token_stack* token_stack, token_t* token);

/**
 * @brief Pops a token from the top of the stack
 * 
 * @param token_stack A pointer to the token stack
 */
void stack_pop(token_stack* token_stack);

/**
 * @brief Returns the token at the top of the stack
 * 
 * @param token_stack A pointer to the token stack
 * 
 * @return The token at the top of the stack
 */
token_t* stack_top(token_stack* token_stack);


/**
 * @brief Returns top terminal token from stack
 * 
 * @param token_stack A pointer to the token stack
 * 
 * @return top terminal token on stack
*/
token_t* stack_top_terminal(token_stack* token_stack);


/**
 * @brief Disposes of a token stack
 * 
 * @param token_stack A pointer to the token stack to be disposed of
 */
void dispose_stack(token_stack* token_stack);

#endif /* TOKEN_STACK_H */
