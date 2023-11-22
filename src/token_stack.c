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
#define STACK_SIZE 4

void stack_init(token_stack* token_stack) {
    
    token_stack->token_array = (token_t**) malloc(STACK_SIZE * sizeof(token_t*));
    token_stack->size = 0;
    token_stack->capacity = STACK_SIZE;
    
}

void stack_push(token_stack* token_stack, token_t* token) {

    if (token_stack->size == token_stack->capacity-1) {

        token_stack->capacity *= 2;
        token_stack->token_array = realloc(token_stack->token_array, token_stack->capacity * sizeof(token_t*));
    }

    token_stack->token_array[token_stack->size++] = token;
}

void stack_pop(token_stack* token_stack) {
    if (token_stack->size == 0) {
     // or some other error value
    return;
    }
    token_stack->size--;
}

token_t* stack_top(token_stack* token_stack) {
    if (token_stack->size == 0) {
        // or some other error value
        return NULL;
    }
    return token_stack->token_array[token_stack->size-1];
}

bool stack_is_empty(token_stack* token_stack) {
    return token_stack->size == 0;
}


token_t* stack_top_terminal(token_stack* token_stack){
    int counter = 0;
    token_t* tmp = token_stack->token_array[token_stack->size - 1 + counter];
    while(tmp->type == TOKEN_EXPRESSION){
        counter--;
        tmp = token_stack->token_array[token_stack->size -1 + counter];
    }
    return tmp;
}

void dispose_stack(token_stack* token_stack) {
    for(int i = 0; i < token_stack->size; i++){
        destroy_token(token_stack->token_array[i]);
    }

    free(token_stack->token_array);
    token_stack->token_array = NULL;
    //free(token_stack);
    //token_stack = NULL;
}
