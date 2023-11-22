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


int add_arg_cnt = 0;


void codegen_var_def(char *name) {
    if (active->parent == NULL) {
        printf("DEFVAR GF@%s\n", name);

    } else {
        printf("DEFVAR LF@%s\n", name);
    }
}

void codegen_var_assign(char *name) {
    if (active->parent == NULL) {
        printf("POPS GF@%s\n", name);
    } else {
        printf("POPS LF@%s\n", name);
    }
}




// function for generating code for function definition
void codegen_func_def() {
    printf("JUMP !!skip_%s\n", active->name);
    printf("LABEL %s\n", active->name);
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    for (int i = 1; i <= active->param_cnt; i++) {
        AVL_tree* param = symtable_find_param(active->symtable, i);
        printf("DEFVAR LF@%s\n", param->data.param_name);
        printf("MOVE LF@%s LF@$%d\n", param->data.param_name, i);
    } 
}

void codegen_func_def_return() {
    printf("POPS LF@$retval$\n");
}

void codegen_func_def_end() {
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_%s\n", active->name);
}




void codegen_func_call_start() {
    printf("CREATEFRAME\n");
}

void codegen_add_arg() {
    printf("DEFVAR TF@$%d\n", ++add_arg_cnt);
    printf("POPS TF@$%d\n", add_arg_cnt);
}

void codegen_func_call_end(char *label) {
    add_arg_cnt = 0;
    printf("CALL %s\n", label);
    printf("PUSHS TF@$retval$\n");
}



void codegen_if() {
    printf("LABEL if_%d\n", active->cond_cnt);
    printf("DEFVAR LF@$cond$\n");
    printf("POPS LF@$cond$\n");
    printf("JUMPIFEQ else_%d LF@$cond$ bool@false\n", active->cond_cnt);
}

void codegen_else() {
    printf("JUMP end_if_%d\n", active->cond_cnt);
    printf("LABEL else_%d\n", active->cond_cnt);

}

void codegen_ifelse_end() {
    printf("LABEL end_if_%d\n", active->cond_cnt);
}   




void codegen_while_start() {
    printf("LABEL while_%d\n", while_cnt);
    printf("DEFVAR LF@$cond$\n");
    printf("POPS LF@$cond$\n");
    printf("JUMPIFEQ end_while_%d LF@$cond$ bool@false\n", while_cnt);
}

void codegen_while_end() {
    printf("JUMP %s\n", active->name);
    printf("LABEL end_%s\n", active->name);
}




void codegen_readString() {
    printf("JUMP !!skip_readString\n");
    printf("LABEL readString\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("READ LF@$retval$ string\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_readString\n");
}

void codegen_readInt() {
    printf("JUMP !!skip_readInt\n");
    printf("LABEL readInt\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("READ LF@$retval$ int\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_readInt\n");
}

void codegen_readDouble() {
    printf("JUMP !!skip_readDouble\n");
    printf("LABEL readDouble\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("READ LF@$retval$ double\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_readDouble\n");
}

void codegen_write() {
    printf("JUMP !!skip_write\n");
    printf("LABEL write\n");
    printf("PUSHFRAME\n");
    // for (int i = 1; i <= XXXXXXXXXXXX; i++) {
    //     printf("WRITE LF@$%d\n", i);
    // }
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_write\n");
}

void codegen_Int2Double() {
    printf("JUMP !!skip_Int2Double\n");  
    printf("LABEL Int2Double\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("INT2FLOAT LF@$retval$ LF@$1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_Int2Double\n");
}

void codegen_Double2Int() {  
    printf("JUMP !!skip_Double2Int\n");
    printf("LABEL Double2Int\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("FLOAT2INT LF@$retval$ LF@$1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_Double2Int\n");
}

void codegen_length() {
    printf("JUMP !!skip_length\n");
    printf("LABEL length\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("STRLEN LF@$retval$ LF@$1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_length\n");
}

void codegen_substring() {
    printf("JUMP !!skip_substring\n");
    printf("LABEL substring\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("DEFVAR LF@$tmp1\n");
    printf("DEFVAR LF@$check\n");
    printf("MOVE LF@$check bool@false\n");
    printf("LT LF@$check LF@$2 int@0\n");
    printf("JUMPIFEQ !!load_nil LF@$check bool@true\n");
    printf("LT LF@$check LF@$3 int@0\n");
    printf("JUMPIFEQ !!load_nil LF@$check bool@true\n");
    printf("GT LF@$check LF@$2 LF@$3\n");
    printf("JUMPIFEQ !!load_nil LF@$check bool@true\n");
    printf("DEFVAR LF@$tmp_strlen\n");
    printf("STRLEN LF@$tmp_strlen LF@$1\n");
    printf("GT LF@$check LF@$2 LF@$tmp_strlen\n");
    printf("JUMPIFEQ !!load_nil LF@$check bool@true\n");
    printf("EQ LF@$check LF@$2 LF@$tmp_strlen\n");
    printf("JUMPIFEQ !!load_nil LF@$check bool@true\n");
    printf("GT LF@$check LF@$3 LF@$tmp_strlen\n");
    printf("JUMPIFEQ !!load_nil LF@$check bool@true\n");
    printf("LABEL !!substring_loop\n");
    printf("GETCHAR LF@$tmp1 LF@$1 LF@$2\n");
    printf("CONCAT LF@$retval$ LF@$retval$ LF@$tmp1\n");
    printf("ADD LF@$2 LF@$2 int@1\n");
    printf("JUMPIFNEQ !!substring_loop LF@$2 LF@$3\n");
    printf("JUMP !!load_result\n");
    printf("LABEL !!load_nil\n");
    printf("MOVE LF@$retval$ nil@nil\n");
    printf("LABEL !!load_result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_substring\n");
}

void codegen_ord() {
    printf("JUMP !!skip_ord\n");
    printf("LABEL ord\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("STR2INT LF@$retval$ LF@$1 int@0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_ord\n");  
}

void codegen_chr() {
    printf("JUMP !!skip_chr\n");
    printf("LABEL chr\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("INT2CHAR LF@$retval$ LF@$1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_chr\n");
}




void codegen_generate_code_please() {
    printf(".IFJcode23\n");
    // printf("DEFVAR GF$tmp1\n"); dle potreby
    // printf("DEFVAR GF$tmp2\n");
    printf("JUMP $$main\n");

    //space for sth before main

    printf("LABEL $$main\n");
}

