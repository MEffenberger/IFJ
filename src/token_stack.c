/**
 * @file token_stack.c
 * 
 * IFJ23 compiler
 * 
 * @brief Token stack implementation
 *
 * @author Dominik Horut <xhorut01>
 * 
 */

#include "token_stack.h"

void init(token_stack* token_stack, int capacity) {
    token_stack->token_array = malloc(capacity * sizeof(token_t));
    token_stack->size = 0;
    token_stack->capacity = capacity;
}

void push(token_stack* token_stack, token_t token) {

    if (token_stack->size == token_stack->capacity-1) {

        token_stack->capacity *= 2;
        token_stack->token_array = realloc(token_stack->token_array, token_stack->capacity * sizeof(token_t));
    }

    token_stack->token_array[token_stack->size++] = token;
}

token_t pop(token_stack* token_stack) {
    if (token_stack->size == 0) {
     // or some other error value
    }
    return token_stack->token_array[--token_stack->size];
}

token_t top(token_stack* token_stack) {
    if (token_stack->size == 0) {
        // or some other error value
    }
    return token_stack->token_array[token_stack->size - 1];
}

bool is_empty(token_stack* token_stack) {
    return token_stack->size == 0;
}

void dispose_stack(token_stack* token_stack) {
    free(token_stack->token_array);
}
