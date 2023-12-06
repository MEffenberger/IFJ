/**
 * @file codegen.c
 *
 * IFJ23 compiler
 *
 * @brief Generator of IFJcode23 with doubly linked list of instructions
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
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
#include <string.h>

int add_arg_cnt = 0; // for codegen_add_arg for unique naming of arguments
int write_renamer = 0; // for codegen_write for unique naming of arguments
extern FILE *file;


void inst_list_init(instruction_list *list) {
    instruction *inst = inst_init(MAIN, 'G', NULL, 0, 0, 0.0, NULL);
    list->active = list->first = list->last = inst;
}


instruction* inst_init(inst_type type,
                        char frame,
                        char *name,
                        int cnt,
                        int int_value,
                        double float_value,
                        char *string_value
) {
	instruction* new_inst = (instruction*) allocate_memory(sizeof(instruction));

    new_inst->relevant_node = active;
    new_inst->inst_type = type;
    new_inst->frame = frame;
    if (name == NULL) {
        new_inst->name = (char*) allocate_memory(sizeof(char) * 6);
        strcpy(new_inst->name, "null");
    }
    else {
        new_inst->name = name;
    }
    new_inst->cnt = cnt;
    new_inst->int_value = int_value;
    new_inst->float_value = float_value;
    new_inst->string_value = string_value;
    return new_inst;
 }


void inst_list_insert_last(instruction_list *list, instruction *new_inst) {
    new_inst->next = NULL;
    new_inst->prev = list->last;

    // re-setting already existing pointers
    list->last->next = new_inst;
    list->last = new_inst;

    // active is now the new instruction
    list->active = new_inst;
}

void inst_list_insert_before(instruction_list *list, instruction *new_inst) {
    new_inst->next = list->active;
    new_inst->prev = list->active->prev;

    // re-setting already existing pointers
    list->active->prev->next = new_inst;
    list->active->prev = new_inst;

    // active is now the new instruction
    list->active = new_inst;
}


void inst_list_delete_after(instruction_list *list) {
    instruction *inst = list->active->next;
    // re-pointing the pointers across the deleted node
    if (inst == list->last) {
        list->last = list->active;
    }
    else {
        inst->next->prev = list->active;
    }
    list->active->next = inst->next;
    free(inst);
}

void inst_list_dispose(instruction_list *list) {
	instruction* inst = list->first;
	while (list->first != NULL) {
		list->first = list->first->next;
		free(inst);
		inst = list->first;
	}

	list->active = NULL;
	list->first = NULL;
	list->last = NULL;
}

// search outermost while loop
void inst_list_search_while(instruction_list *list, char *while_name) {
    // move list->active based on the name of the node
    while (strcmp(list->active->name, while_name) != 0 || list->active->inst_type != WHILE_START) {
        list->active = list->active->prev;
    }
    // list->active is now outermost while loop, you can insert before it
}

// search call of void function
void inst_list_search_void_func_call(instruction_list *list, char *func_name) {
    // move list->active based on the name of the node found 
    list->active = list->last;
    while (strcmp(list->active->name, func_name) != 0 || list->active->inst_type != FUNC_CALL || list->active->next == NULL || list->active->next->inst_type != FUNC_CALL_RETVAL) {
        list->active = list->active->prev;
    }
    // list->active is now void_function call, you can delete the instruction after it
}

// goes through the whole list and prints the instructions based on their type
void codegen_generate_code_please(instruction_list *list) {
    instruction *inst = list->first;
    while (inst != NULL) {
        switch (inst->inst_type) {
            case MAIN:
                codegen_main(inst);
                break;
            case VAR_DEF:
                codegen_var_def(inst);
                break;
            case VAR_ASSIGN:
                codegen_var_assign(inst);
                break;
            case VAR_ASSIGN_NIL:
                codegen_var_assign_nil(inst);
                break;
            case IMPLICIT_NIL: // implicit nil assignment
                fprintf(file, "MOVE %cF@%s nil@nil\n", inst->frame, inst->name);
                break;
            case FUNC_DEF:
                codegen_func_def(inst);
                break;
            case FUNC_DEF_RETURN:
                codegen_func_def_return(inst);
                break;
            case FUNC_DEF_RETURN_VOID:
                codegen_func_def_return_void(inst);
                break;
            case FUNC_DEF_END:
                codegen_func_def_end(inst);
                break;
            case FUNC_CALL_START:
                codegen_func_call_start(inst);
                break;
            case ADD_ARG:
                codegen_add_arg(inst);
                break;
            case FUNC_CALL:
                fprintf(file, "CALL %s\n", inst->name);
                break;
            case FUNC_CALL_RETVAL:
                fprintf(file, "PUSHS TF@$retval$\n");
                break;
            case IF_LABEL:
                fprintf(file, "LABEL if_%d\n", inst->cnt);
                break;
            case IF_DEFVAR:
                fprintf(file, "DEFVAR %cF@$cond_%d$\n", inst->frame, inst->cnt);
                break;
            case IF_LET:
                codegen_if_let(inst);
                break;
            case IF:
                codegen_if(inst);
                break;
            case ELSE:
                codegen_else(inst);
                break;
            case IFELSE_END:
                codegen_ifelse_end(inst);
                break;
            case WHILE_COND_DEF:
                fprintf(file, "DEFVAR %cF@$cond_%s$\n", inst->frame, inst->name);
                break;
            case WHILE_START:
                fprintf(file, "LABEL %s\n", inst->name);
                break;
            case WHILE_DO:
                codegen_while_do(inst);
                break;
            case WHILE_END:
                codegen_while_end(inst);
                break;
            case READ_STRING:
                codegen_readString(inst);
                break;
            case READ_INT:
                codegen_readInt(inst);
                break;
            case READ_DOUBLE:
                codegen_readDouble(inst);
                break;
            case WRITE:
                codegen_write(inst);
                break;
            case INT2DOUBLE:
                codegen_Int2Double(inst);
                break;
            case DOUBLE2INT:
                codegen_Double2Int(inst);
                break;
            case LENGTH:
                codegen_length(inst);
                break;
            case SUBSTRING:
                codegen_substring(inst);
                break;
            case ORD:
                codegen_ord(inst);
                break;
            case CHR:
                codegen_chr(inst);
                break;
            case CONCAT:
                codegen_concat(inst);
                break;
            case CONCAT_DEFVAR:
                fprintf(file, "DEFVAR %cF@$$s%d$$\n", inst->frame, inst->cnt);
                fprintf(file, "DEFVAR %cF@$$s%d$$\n", inst->frame, inst->cnt + 1);
                break;
            case INT2FLOATS:
                fprintf(file, "INT2FLOATS\n");
                break;
            case INT2FLOATS_2:
                codegen_int2floats(inst);
                break;
            case INT2FLOATS_2_DEFVAR:
                fprintf(file, "DEFVAR %cF@$$tmp%d$$\n", inst->frame, inst->cnt);
                break;
            case DIV_ZERO_DEFVAR:
                fprintf(file, "DEFVAR %cF@div_zero_%d\n", inst->frame, inst->cnt);
                break;
            case DIV_BY_ZERO:
                codegen_div_zero(inst);
                break;
            case DIVS:
                fprintf(file, "DIVS\n");
                break;
            case IDIV_ZERO_DEFVAR:
                fprintf(file, "DEFVAR %cF@idiv_zero_%d\n", inst->frame, inst->cnt);
                break;
            case IDIV_BY_ZERO:
                codegen_idiv_zero(inst);
                break;
            case IDIVS:
                fprintf(file, "IDIVS\n");
                break;
            case PUSHS_INT_CONST:
                fprintf(file, "PUSHS int@%d\n", inst->int_value);
                break;
            case PUSHS_FLOAT_CONST:
                fprintf(file, "PUSHS float@%a\n", inst->float_value);
                break;
            case PUSHS_STRING_CONST:
                fprintf(file, "PUSHS string@%s\n", inst->string_value);
                break;
            case PUSHS_NIL:
                fprintf(file, "PUSHS nil@nil\n");
                break;
            case PUSHS:
                fprintf(file, "PUSHS %cF@%s\n", inst->frame, inst->name);
                break;
            case EXCLAMATION_RULE:
                codegen_exclamation_rule(inst);
                break;
            case EXCLAMATION_RULE_DEFVAR:
                fprintf(file, "DEFVAR %cF@$$excl%d\n", inst->frame, inst->cnt);
                break;
            case ADDS:
                fprintf(file, "ADDS\n");
                break;
            case MULS:
                fprintf(file, "MULS\n");
                break;
            case SUBS:
                fprintf(file, "SUBS\n");
                break;
            case LTS:
                fprintf(file, "LTS\n");
                break;
            case EQS:
                fprintf(file, "EQS\n");
                break;
            case ORS:
                fprintf(file, "ORS\n");
                break;
            case GTS:
                fprintf(file, "GTS\n");
                break;
            case NOTS:
                fprintf(file, "NOTS\n");
                break;
            case LEQ_RULE:
                codegen_leq_rule(inst);
                break;
            case LEQ_RULE_DEFVAR:
                fprintf(file, "DEFVAR %cF@$$leq%d$$\n", inst->frame, inst->cnt);
                fprintf(file, "DEFVAR %cF@$$leq%d$$\n", inst->frame, inst->cnt + 1);
                break;
            case GEQ_RULE:
                codegen_geq_rule(inst);
                break;
            case GEQ_RULE_DEFVAR:
                fprintf(file, "DEFVAR %cF@$$geq%d$$\n", inst->frame, inst->cnt);
                fprintf(file, "DEFVAR %cF@$$geq%d$$\n", inst->frame, inst->cnt + 1);
                break;
            case QMS_RULE:
                codegen_qms_rule(inst);
                break;
            case QMS_RULE_DEFVAR:
                fprintf(file, "DEFVAR %cF@$$rule_qms%d\n", inst->frame, inst->cnt);
                fprintf(file, "DEFVAR %cF@$$rule_qms%d\n", inst->frame, inst->cnt + 1);
                break;
            default:
                break;
        }
        inst = inst->next;
    }
}


void codegen_var_def(instruction *inst) {
    fprintf(file, "DEFVAR %cF@%s\n", inst->frame, inst->name);
}

// assign value from the top of the stack to the variable
void codegen_var_assign(instruction *inst) {
    fprintf(file, "POPS %cF@%s\n", inst->frame, inst->name);
}

// nil assignment
void codegen_var_assign_nil(instruction *inst) {
    fprintf(file, "MOVE %cF@%s nil@nil\n", inst->frame, inst->name);
}

// function definition
void codegen_func_def(instruction *inst) {
    fprintf(file, "JUMP !!skip_%s\n", inst->name);
    fprintf(file, "LABEL %s\n", inst->name);
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    for (int i = 1; i <= inst->cnt; i++) {
        // find the parameter in the symtable based on its param_order
        AVL_tree* param = symtable_find_param(inst->relevant_node->symtable, i);
        fprintf(file, "DEFVAR LF@%s\n", param->key);
        fprintf(file, "MOVE LF@%s LF@$%d\n", param->key, i);
    } 
}

// return value is on the top of the stack
void codegen_func_def_return(instruction *inst) {
    fprintf(file, "POPS LF@$retval$\n");
    fprintf(file, "JUMP end_%s\n", inst->name);
}

// void function without return value
void codegen_func_def_return_void(instruction *inst) {
    fprintf(file, "JUMP end_%s\n", inst->name);
}

// end of function definition
void codegen_func_def_end(instruction *inst) {
    fprintf(file, "LABEL end_%s\n", inst->name);
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_%s\n", inst->name);
}


void codegen_func_call_start(instruction *inst) {
    add_arg_cnt = 0;
    fprintf(file, "CREATEFRAME\n");
}

void codegen_add_arg(instruction *inst) {
    fprintf(file, "DEFVAR TF@$%d\n", ++add_arg_cnt);
    fprintf(file, "POPS TF@$%d\n", add_arg_cnt);
}


// if let - do the else statement if the variable is nil
void codegen_if_let(instruction *inst) {
    fprintf(file, "TYPE %cF@$cond_%d$ %cF@%s\n", inst->frame, inst->cnt, inst->frame, inst->name);
    fprintf(file, "JUMPIFEQ else_%d %cF@$cond_%d$ string@nil\n", inst->cnt, inst->frame, inst->cnt); 
}

// if - do the else statement if the condition is false
void codegen_if(instruction *inst) {
    fprintf(file, "POPS %cF@$cond_%d$\n", inst->frame, inst->cnt);
    fprintf(file, "JUMPIFEQ else_%d %cF@$cond_%d$ bool@false\n", inst->cnt, inst->frame, inst->cnt);
}

// else statement
void codegen_else(instruction *inst) {
    fprintf(file, "JUMP end_if_%d\n", inst->cnt);
    fprintf(file, "LABEL else_%d\n", inst->cnt);

}

void codegen_ifelse_end(instruction *inst) {
    fprintf(file, "LABEL end_if_%d\n", inst->cnt);
}   

// while - jump to the end of the while loop if the condition is false
void codegen_while_do(instruction *inst) {
    fprintf(file, "POPS %cF@$cond_%s$\n", inst->frame, inst->name);
    fprintf(file, "JUMPIFEQ end_%s %cF@$cond_%s$ bool@false\n", inst->name, inst->frame, inst->name);
}

void codegen_while_end(instruction *inst) {
    fprintf(file, "JUMP %s\n", inst->name);
    fprintf(file, "LABEL end_%s\n", inst->name);
}

// built-in functions
void codegen_readString(instruction *inst) {
    fprintf(file, "JUMP !!skip_readString\n");
    fprintf(file, "LABEL readString\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "READ LF@$retval$ string\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_readString\n");
}

void codegen_readInt(instruction *inst) {
    fprintf(file, "JUMP !!skip_readInt\n");
    fprintf(file, "LABEL readInt\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "READ LF@$retval$ int\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_readInt\n");
}

void codegen_readDouble(instruction *inst) {
    fprintf(file, "JUMP !!skip_readDouble\n");
    fprintf(file, "LABEL readDouble\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "READ LF@$retval$ float\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_readDouble\n");
}

void codegen_write(instruction *inst) {
    fprintf(file, "CREATEFRAME\n");
    fprintf(file, "PUSHFRAME\n");
    // get the arguments from the stack and print them in the correct order
    for (int i = 1; i <= inst->cnt; i++) {
        fprintf(file, "DEFVAR LF@$%d\n", write_renamer + i);
        fprintf(file, "POPS LF@$%d\n", write_renamer + i);
    }
    for (int i = inst->cnt; i >= 1; i--) {
        fprintf(file, "WRITE LF@$%d\n", write_renamer + i);
    }
    write_renamer += inst->cnt;
    fprintf(file, "POPFRAME\n");
}

void codegen_Int2Double(instruction *inst) {
    fprintf(file, "JUMP !!skip_Int2Double\n");  
    fprintf(file, "LABEL Int2Double\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "INT2FLOAT LF@$retval$ LF@$1\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_Int2Double\n");
}

void codegen_Double2Int(instruction *inst) {  
    fprintf(file, "JUMP !!skip_Double2Int\n");
    fprintf(file, "LABEL Double2Int\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "FLOAT2INT LF@$retval$ LF@$1\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_Double2Int\n");
}

void codegen_length(instruction *inst) {
    fprintf(file, "JUMP !!skip_length\n");
    fprintf(file, "LABEL length\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "STRLEN LF@$retval$ LF@$1\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_length\n");
}

void codegen_substring(instruction *inst) {
    fprintf(file, "JUMP !!skip_substring\n");
    fprintf(file, "LABEL substring\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "MOVE LF@$retval$ string@\n");
    fprintf(file, "DEFVAR LF@$tmp1\n");
    fprintf(file, "DEFVAR LF@$check\n");
    fprintf(file, "MOVE LF@$check bool@false\n"); // retval is nil if:
    fprintf(file, "LT LF@$check LF@$2 int@0\n"); // startingAt < 0
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "LT LF@$check LF@$3 int@0\n"); // endingBefore < 0
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "GT LF@$check LF@$2 LF@$3\n"); // startingAt > endingBefore
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "EQ LF@$check LF@$2 LF@$3\n"); 
    fprintf(file, "JUMPIFEQ !!load_result LF@$check bool@true\n");
    fprintf(file, "DEFVAR LF@$tmp_strlen\n"); 
    fprintf(file, "STRLEN LF@$tmp_strlen LF@$1\n");
    fprintf(file, "GT LF@$check LF@$2 LF@$tmp_strlen\n"); // startingAt > strlen
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "EQ LF@$check LF@$2 LF@$tmp_strlen\n"); // startingAt == strlen 
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "GT LF@$check LF@$3 LF@$tmp_strlen\n"); // endingBefore > strlen
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "LABEL !!substring_loop\n");
    fprintf(file, "GETCHAR LF@$tmp1 LF@$1 LF@$2\n");
    fprintf(file, "CONCAT LF@$retval$ LF@$retval$ LF@$tmp1\n");
    fprintf(file, "ADD LF@$2 LF@$2 int@1\n");
    fprintf(file, "JUMPIFNEQ !!substring_loop LF@$2 LF@$3\n");
    fprintf(file, "JUMP !!load_result\n");
    fprintf(file, "LABEL !!load_nil\n"); // load nil if any of the conditions above is true
    fprintf(file, "MOVE LF@$retval$ nil@nil\n");
    fprintf(file, "LABEL !!load_result\n"); // load result, if startingAt == endingBefore, it's empty string
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_substring\n");
}

void codegen_ord(instruction *inst) {
    fprintf(file, "JUMP !!skip_ord\n");
    fprintf(file, "LABEL ord\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "JUMPIFNEQ !!valid_param LF@$1 string@\n");
    fprintf(file, "MOVE LF@$retval$ int@0\n");
    fprintf(file, "JUMP !!end_ord\n");
    fprintf(file, "LABEL !!valid_param\n");
    fprintf(file, "STRI2INT LF@$retval$ LF@$1 int@0\n");
    fprintf(file, "LABEL !!end_ord\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_ord\n");  
}

void codegen_chr(instruction *inst) {
    fprintf(file, "JUMP !!skip_chr\n");
    fprintf(file, "LABEL chr\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "INT2CHAR LF@$retval$ LF@$1\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_chr\n");
}

// main function - start of the program
void codegen_main(instruction *inst) {
    fprintf(file, ".IFJcode23\n");
    fprintf(file, "CREATEFRAME\n");
    fprintf(file, "PUSHFRAME\n");
}

// vardefs in the following functions are separated in case of while loop:
// concatenation of two strings,
void codegen_concat(instruction *inst) {
    fprintf(file, "POPS %cF@$$s%d$$\n", inst->frame, inst->cnt + 1);
    fprintf(file, "POPS %cF@$$s%d$$\n", inst->frame, inst->cnt);
    fprintf(file, "CONCAT %cF@$$s%d$$ %cF@$$s%d$$ %cF@$$s%d$$\n", inst->frame, inst->cnt, inst->frame, inst->cnt, inst->frame, inst->cnt + 1);
    fprintf(file, "PUSHS %cF@$$s%d$$\n", inst->frame, inst->cnt);
}

// int to float conversion
void codegen_int2floats(instruction *inst) {
    fprintf(file, "POPS %cF@$$tmp%d$$\n", inst->frame, inst->cnt);
    fprintf(file, "INT2FLOATS\n");
    fprintf(file, "PUSHS %cF@$$tmp%d$$\n", inst->frame, inst->cnt);
}

// exclamation rule
void codegen_exclamation_rule(instruction *inst) {
    fprintf(file, "POPS %cF@$$excl%d\n", inst->frame, inst->cnt);
    fprintf(file, "PUSHS %cF@$$excl%d\n", inst->frame, inst->cnt);
    fprintf(file, "PUSHS nil@nil\n");
    fprintf(file, "JUMPIFNEQS $RULE_EXCL_CORRECT%d$\n", inst->cnt);
    fprintf(file, "LABEL $RULE_EXCL_ERROR%d$\n", inst->cnt);
    fprintf(file, "WRITE string@Variable\\032is\\032NULL\n");
    fprintf(file, "EXIT int@7\n");
    fprintf(file, "LABEL $RULE_EXCL_CORRECT%d$\n", inst->cnt);
    fprintf(file, "PUSHS %cF@$$excl%d\n", inst->frame, inst->cnt);
}

// less than equal rule
void codegen_leq_rule(instruction *inst) {
    fprintf(file, "POPS %cF@$$leq%d$$\n", inst->frame, inst->cnt);
    fprintf(file, "EQS\n");
    fprintf(file, "POPS %cF@$$leq%d$$\n", inst->frame, inst->cnt + 1);
    fprintf(file, "PUSHS %cF@$$leq%d$$\n", inst->frame, inst->cnt);
    fprintf(file, "PUSHS %cF@$$leq%d$$\n", inst->frame, inst->cnt + 1);
    fprintf(file, "ORS\n");
}

// greater than equal rule
void codegen_geq_rule(instruction *inst) {
    fprintf(file, "POPS %cF@$$geq%d$$\n", inst->frame, inst->cnt);
    fprintf(file, "EQS\n");
    fprintf(file, "POPS %cF@$$geq%d$$\n", inst->frame, inst->cnt + 1);
    fprintf(file, "PUSHS %cF@$$geq%d$$\n", inst->frame, inst->cnt);
    fprintf(file, "PUSHS %cF@$$geq%d$$\n", inst->frame, inst->cnt + 1);
    fprintf(file, "ORS\n");
}

// question mark rule
void codegen_qms_rule(instruction *inst) {
    fprintf(file, "POPS %cF@$$rule_qms%d\n", inst->frame, inst->cnt);
    fprintf(file, "POPS %cF@$$rule_qms%d\n", inst->frame, inst->cnt + 1);
    fprintf(file, "PUSHS %cF@$$rule_qms%d\n", inst->frame, inst->cnt + 1);
    fprintf(file, "PUSHS nil@nil\n");
    fprintf(file, "JUMPIFNEQS $RULE_QMS_NOT_NILL%d$\n", inst->cnt);

    fprintf(file, "LABEL $RULE_QMS_NILL%d$\n", inst->cnt);
    fprintf(file, "PUSHS %cF@$$rule_qms%d\n", inst->frame, inst->cnt);
    fprintf(file, "JUMP $END_RULE_QMS%d$\n",  inst->cnt);

    fprintf(file, "LABEL $RULE_QMS_NOT_NILL%d$\n", inst->cnt);
    fprintf(file, "PUSHS %cF@$$rule_qms%d\n", inst->frame, inst->cnt + 1);

    fprintf(file, "LABEL $END_RULE_QMS%d$\n",  inst->cnt);
}

// checking division by zero for integers
void codegen_idiv_zero(instruction *inst) {
    fprintf(file, "POPS %cF@idiv_zero_%d\n", inst->frame, inst->cnt);
    fprintf(file, "JUMPIFNEQ !!SKIP_IDIV_BY_ZERO%d %cF@%s%d int@0\n", inst->cnt, inst->frame, inst->name, inst->cnt);
    fprintf(file, "EXIT int@7\n");
    fprintf(file, "LABEL !!SKIP_IDIV_BY_ZERO%d\n", inst->cnt);
    fprintf(file, "PUSHS %cF@idiv_zero_%d\n", inst->frame, inst->cnt);
}

// checking division by zero for floats
void codegen_div_zero(instruction *inst) {
    fprintf(file, "POPS %cF@div_zero_%d\n", inst->frame, inst->cnt);
    fprintf(file, "JUMPIFNEQ !!SKIP_DIV_BY_ZERO%d %cF@%s%d float@0x0p+0\n", inst->cnt, inst->frame, inst->name, inst->cnt);
    fprintf(file, "EXIT int@7\n");
    fprintf(file, "LABEL !!SKIP_DIV_BY_ZERO%d\n", inst->cnt);
    fprintf(file, "PUSHS %cF@div_zero_%d\n", inst->frame, inst->cnt);
}

