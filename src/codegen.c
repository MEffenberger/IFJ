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


// void codegen_write(instruction *inst) {
//     for (int i = 0; i < inst->num_operands; i++) {
//         printf("WRITE LF@%s\n", inst->operands[i]);
//     }
// }









// function for generating code for function definition
void codegen_func_def() {
    // printf("\t\t\t\t\t\t\t\t\t\t  CREATEFRAME\n");
    // printf("\t\t\t\t\t\t\t\t\t\t  PUSHFRAME\n");
    printf("\t\t\t\t\t\t\t\t\t\t  LABEL %s\n", active->name);
    while (!(token_is_empty(token_stack))) {
        printf("\t\t\t\t\t\t\t\t\t\t  DEFVAR LF@%s\n", token_top(token_stack)->value.vector->array);
        printf("\t\t\t\t\t\t\t\t\t\t  POPS LF@%s\n", token_top(token_stack)->value.vector->array);
        token_pop(token_stack);
    }
}


void codegen_func_def_end() {
    // printf("\t\t\t\t\t\t\t\t\t\t  POPFRAME\n");
    printf("\t\t\t\t\t\t\t\t\t\t  RETURN\n");
}

void codegen_func_call(char *label) {
    printf("\t\t\t\t\t\t\t\t\t\t  CALL %s\n", label);
}


void codegen_if() {
    printf("\t\t\t\t\t\t\t\t\t\t  LABEL if_%d\n", active->cond_cnt);
    printf("\t\t\t\t\t\t\t\t\t\t  DEFVAR LF@%%cond\n");
    printf("\t\t\t\t\t\t\t\t\t\t  POPS LF@%%cond\n");
    printf("\t\t\t\t\t\t\t\t\t\t  JUMPIFEQ else_%d LF@%%cond bool@false\n", active->cond_cnt);
}

void codegen_else() {
    printf("\t\t\t\t\t\t\t\t\t\t  JUMP end_if_%d\n", active->cond_cnt);
    printf("\t\t\t\t\t\t\t\t\t\t  LABEL else_%d\n", active->cond_cnt);

}

void codegen_ifelse_end() {
    printf("\t\t\t\t\t\t\t\t\t\t  LABEL end_if_%d\n", active->cond_cnt);
}   


void codegen_while_start() {
    printf("\t\t\t\t\t\t\t\t\t\t  LABEL while_%d\n", while_cnt);
    printf("\t\t\t\t\t\t\t\t\t\t  DEFVAR LF@%%cond\n");
    printf("\t\t\t\t\t\t\t\t\t\t  POPS LF@%%cond\n");
    printf("\t\t\t\t\t\t\t\t\t\t  JUMPIFEQ end_while_%d LF@%%cond bool@false\n", while_cnt);
}

void codegen_while_end() {
    printf("\t\t\t\t\t\t\t\t\t\t  JUMP %s\n", active->name);
    printf("\t\t\t\t\t\t\t\t\t\t  LABEL end_%s\n", active->name);
}


void codegen_generate_code_please() {
    printf(".IFJcode23\n");
}

