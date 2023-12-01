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
#include "error.h"
#include "expression_parser.h"
#include "forest.h"
#include "parser.h"
#include "queue.h"
#include "scanner.h"
#include "string_vector.h"
#include "symtable.h"
#include "token_stack.h"

int add_arg_cnt = 0;
int write_renamer = 0;
extern FILE *file;




void codegen_var_def(char *name) {
    if (active->parent == NULL) {
        fprintf(file, "DEFVAR GF@%s\n", name);
    } else {
        fprintf(file, "DEFVAR LF@%s\n", name);
    }
}

void codegen_var_assign(char *name) {
    if (active->parent == NULL) {
        fprintf(file, "POPS GF@%s\n", name);
    } else {
        fprintf(file, "POPS LF@%s\n", name);
    }
}




void codegen_func_def() {
    fprintf(file, "JUMP !!skip_%s\n", active->name);
    fprintf(file, "LABEL %s\n", active->name);
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    for (int i = 1; i <= active->param_cnt; i++) {
        AVL_tree* param = symtable_find_param(active->symtable, i);
        fprintf(file, "DEFVAR LF@%s\n", param->data.param_name);
        fprintf(file, "MOVE LF@%s LF@$%d\n", param->data.param_name, i);
    } 
}

void codegen_func_def_return(char *func_name) {
    fprintf(file, "POPS LF@$retval$\n");
    fprintf(file, "JUMP end_%s\n", func_name);
}

void codegen_func_def_return_void(char *func_name) {
    fprintf(file, "JUMP end_%s\n", func_name);
}

void codegen_func_def_end() {
    fprintf(file, "LABEL end_%s\n", active->name);
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_%s\n", active->name);
}




void codegen_func_call_start() {
    add_arg_cnt = 0;
    fprintf(file, "CREATEFRAME\n");
}

void codegen_add_arg() {
    fprintf(file, "DEFVAR TF@$%d\n", ++add_arg_cnt);
    fprintf(file, "POPS TF@$%d\n", add_arg_cnt);
}

void codegen_func_call_end(char *label) {
    fprintf(file, "CALL %s\n", label);
    fprintf(file, "PUSHS TF@$retval$\n");
}




void codegen_if_let(char *name) {
    fprintf(file, "LABEL if_%d\n", active->cond_cnt);
    fprintf(file, "DEFVAR LF@$cond_%d$\n", active->cond_cnt);
    fprintf(file, "TYPE LF@$cond_%d$ LF@%s\n", active->cond_cnt, name);
    fprintf(file, "JUMPIFEQ else_%d LF@$cond_%d$ string@nil\n", active->cond_cnt, active->cond_cnt); 
}

void codegen_if() {
    fprintf(file, "LABEL if_%d\n", active->cond_cnt);
    fprintf(file, "DEFVAR LF@$cond_%d$\n", active->cond_cnt);
    fprintf(file, "POPS LF@$cond_%d$\n", active->cond_cnt);
    fprintf(file, "JUMPIFEQ else_%d LF@$cond_%d$ bool@false\n", active->cond_cnt, active->cond_cnt);
}

void codegen_else() {
    fprintf(file, "JUMP end_if_%d\n", active->cond_cnt);
    fprintf(file, "LABEL else_%d\n", active->cond_cnt);

}

void codegen_ifelse_end() {
    fprintf(file, "LABEL end_if_%d\n", active->cond_cnt);
}   




void codegen_while_start() {
    fprintf(file, "LABEL while_%d\n", while_cnt);
    fprintf(file, "DEFVAR LF@$wcond_%d$\n", while_cnt);
    fprintf(file, "POPS LF@$wcond_%d$\n", while_cnt);
    fprintf(file, "JUMPIFEQ end_while_%d LF@$wcond_%d$ bool@false\n", while_cnt, while_cnt);
}

void codegen_while_end() {
    fprintf(file, "JUMP %s\n", active->name);
    fprintf(file, "LABEL end_%s\n", active->name);
}




void codegen_readString() {
    fprintf(file, "JUMP !!skip_readString\n");
    fprintf(file, "LABEL readString\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "READ LF@$retval$ string\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_readString\n");
}

void codegen_readInt() {
    fprintf(file, "JUMP !!skip_readInt\n");
    fprintf(file, "LABEL readInt\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "READ LF@$retval$ int\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_readInt\n");
}

void codegen_readDouble() {
    fprintf(file, "JUMP !!skip_readDouble\n");
    fprintf(file, "LABEL readDouble\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "READ LF@$retval$ double\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_readDouble\n");
}

void codegen_write(int count) {
    fprintf(file, "CREATEFRAME\n");
    fprintf(file, "PUSHFRAME\n");
    for (int i = 1; i <= count; i++) {
        fprintf(file, "DEFVAR LF@$%d\n", write_renamer + i);
        fprintf(file, "POPS LF@$%d\n", write_renamer + i);
    }
    for (int i = count; i >= 1; i--) {
        fprintf(file, "WRITE LF@$%d\n", write_renamer + i);
    }
    write_renamer += count;
    fprintf(file, "POPFRAME\n");
}

void codegen_Int2Double() {
    fprintf(file, "JUMP !!skip_Int2Double\n");  
    fprintf(file, "LABEL Int2Double\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "INT2FLOAT LF@$retval$ LF@$1\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_Int2Double\n");
}

void codegen_Double2Int() {  
    fprintf(file, "JUMP !!skip_Double2Int\n");
    fprintf(file, "LABEL Double2Int\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "FLOAT2INT LF@$retval$ LF@$1\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_Double2Int\n");
}

void codegen_length() {
    fprintf(file, "JUMP !!skip_length\n");
    fprintf(file, "LABEL length\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "STRLEN LF@$retval$ LF@$1\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_length\n");
}

void codegen_substring() {
    fprintf(file, "JUMP !!skip_substring\n");
    fprintf(file, "LABEL substring\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "DEFVAR LF@$tmp1\n");
    fprintf(file, "DEFVAR LF@$check\n");
    fprintf(file, "MOVE LF@$check bool@false\n");
    fprintf(file, "LT LF@$check LF@$2 int@0\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "LT LF@$check LF@$3 int@0\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "GT LF@$check LF@$2 LF@$3\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "DEFVAR LF@$tmp_strlen\n");
    fprintf(file, "STRLEN LF@$tmp_strlen LF@$1\n");
    fprintf(file, "GT LF@$check LF@$2 LF@$tmp_strlen\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "EQ LF@$check LF@$2 LF@$tmp_strlen\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "GT LF@$check LF@$3 LF@$tmp_strlen\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "LABEL !!substring_loop\n");
    fprintf(file, "GETCHAR LF@$tmp1 LF@$1 LF@$2\n");
    fprintf(file, "CONCAT LF@$retval$ LF@$retval$ LF@$tmp1\n");
    fprintf(file, "ADD LF@$2 LF@$2 int@1\n");
    fprintf(file, "JUMPIFNEQ !!substring_loop LF@$2 LF@$3\n");
    fprintf(file, "JUMP !!load_result\n");
    fprintf(file, "LABEL !!load_nil\n");
    fprintf(file, "MOVE LF@$retval$ nil@nil\n");
    fprintf(file, "LABEL !!load_result\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_substring\n");
}

void codegen_ord() {
    fprintf(file, "JUMP !!skip_ord\n");
    fprintf(file, "LABEL ord\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "STR2INT LF@$retval$ LF@$1 int@0\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_ord\n");  
}

void codegen_chr() {
    fprintf(file, "JUMP !!skip_chr\n");
    fprintf(file, "LABEL chr\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "INT2CHAR LF@$retval$ LF@$1\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_chr\n");
}




void codegen_generate_code_please() {
    fprintf(file, ".IFJcode23\n");
    // fprintf(file, "DEFVAR GF$check1\n"); dle potreby
    // fprintf(file, "DEFVAR GF$tmp2\n");
    // fprintf(file, "JUMP $$main\n");

    // //space for sth before main

    // fprintf(file, "LABEL $$main\n");
}

