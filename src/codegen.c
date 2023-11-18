/**
 * @file codegen.c
 *
 * IFJ23 compiler
 *
 * @brief 
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 * @author Samuel Hejnicek <xhejni00>
 * @author Dominik Horut <xhorut01>
 */

#include "codegen.h"

token_stack_t *token_stack = NULL;

// void codegen_write(instruction *inst) {
//     for (int i = 0; i < inst->num_operands; i++) {
//         printf("WRITE LF@%s\n", inst->operands[i]);
//     }
// }









// function for generating code for function definition
void codegen_funcdef() {
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("LABEL %s\n", active->name);
    for (int i = 0; i < token_stack->size; i++) {
        if (token_stack->token_array[i]->type == TOKEN_ID) {
            printf("DEFVAR LF@%s\n", token_stack->token_array[i]->value.vector->array);
        }
    }
}


void codegen_funcdef_end() {

}


void codegen_if() {
    printf("LABEL if_%d\n\n\n\n", active->cond_cnt);
    printf("DEFVAR LF@%%cond\n\n\n\n");
    printf("POPS LF@%%cond\n\n\n\n");
    printf("JUMPIFEQ else_%d LF@%%cond bool@false\n\n\n\n", active->cond_cnt);
}

void codegen_else() {
    printf("JUMP end_if_%d\n\n\n\n", active->cond_cnt);
    printf("LABEL else_%d\n\n\n\n", active->cond_cnt);

}

void codegen_ifelse_end() {
    printf("LABEL end_if_%d\n\n\n\n", active->cond_cnt);
}   


void codegen_while_start() {
    printf("LABEL while_%d\n\n\n\n", while_cnt);
    printf("DEFVAR LF@%%cond\n\n\n\n");
    printf("POPS LF@%%cond\n\n\n\n");
    printf("JUMPIFEQ end_while_%d LF@%%cond bool@false\n\n\n\n", while_cnt);
}

void codegen_while_end() {
    printf("JUMP %s\n\n\n\n", active->name);
    printf("LABEL end_%s\n\n\n\n", active->name);
}


void codegen_generate_code_please() {
    init(token_stack);
    printf(".IFJcode23\n");
}

