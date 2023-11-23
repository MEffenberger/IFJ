/**
 * @file cnt_stack.c
 * 
 * IFJ23 compiler
 * 
 * @brief cnt stack implementation
 * 
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

#define STACK_SIZE 4

void cnt_init(cnt_stack_t* cnt_stack) {
    
    cnt_stack->cnt_array = (int*) malloc(STACK_SIZE * sizeof(int));
    cnt_stack->size = 0;
    cnt_stack->capacity = STACK_SIZE;
    
}

void cnt_push(cnt_stack_t* cnt_stack) {

    if (cnt_stack->size == cnt_stack->capacity-1) {

        cnt_stack->capacity *= 2;
        cnt_stack->cnt_array = realloc(cnt_stack->cnt_array, cnt_stack->capacity * sizeof(int*));
    }


    cnt_stack->cnt_array[cnt_stack->size++] = ifelse_cnt;
}

void cnt_pop(cnt_stack_t* cnt_stack) {
    if (cnt_stack->size == 0) {
     // or some other error value
    return;
    }
    cnt_stack->size--;
}

int cnt_top(cnt_stack_t* cnt_stack) {
    if (cnt_stack->size == 0) {
        // or some other error value
        return -1;
    }
    return cnt_stack->cnt_array[cnt_stack->size-1];
}

bool cnt_is_empty(cnt_stack_t* cnt_stack) {
    return cnt_stack->size == 0;
}

void cnt_dispose_stack(cnt_stack_t* cnt_stack) {

    free(cnt_stack->cnt_array);
    cnt_stack->cnt_array = NULL;
}

void cnt_print_stack(cnt_stack_t* cnt_stack) {
    printf("cnt_stack:\n");
    for (int i = 0; i < cnt_stack->size; i++) {
        printf("%d\n", cnt_stack->cnt_array[i]);
    }
}
