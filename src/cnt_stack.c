/**
 * @file cnt_stack.c
 * 
 * IFJ23 compiler
 * 
 * @brief cnt stack implementation
 * 
 * @author Adam Valik <xvalik05>
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


void cnt_init(cnt_stack_t* cnt_stack) {
    cnt_stack->cnt_array = (int*)allocate_memory(STACK_SIZE * sizeof(int));
    cnt_stack->size = 0;
    cnt_stack->capacity = STACK_SIZE;
}

void cnt_push(cnt_stack_t* cnt_stack) {
    if (cnt_stack->size == cnt_stack->capacity-1) {
        cnt_stack->capacity *= 2;
        cnt_stack->cnt_array = reallocate_memory(cnt_stack->cnt_array, cnt_stack->capacity * sizeof(int*));
    }
    cnt_stack->cnt_array[cnt_stack->size++] = ifelse_cnt;
}

void cnt_pop(cnt_stack_t* cnt_stack) {
    cnt_stack->size--;
}

int cnt_top(cnt_stack_t* cnt_stack) {
    return cnt_stack->cnt_array[cnt_stack->size-1];
}

void cnt_dispose_stack(cnt_stack_t* cnt_stack) {
    free(cnt_stack->cnt_array);
    cnt_stack->cnt_array = NULL;
    free(cnt_stack);
    cnt_stack = NULL;
}

