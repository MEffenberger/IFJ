/**
 * @file codegen.c
 *
 * IFJ23 compiler
 *
 * @brief Generator of IFJcode23 with DLL of instructions
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
#include <string.h>

int add_arg_cnt = 0;
int write_renamer = 0;
extern FILE *file;

// allocate mem for the list
// inst_list_init(list);
// set the first&active instruction manually
// when adding new instruction, use:
    // instruction *new_inst = inst_init(type, frame, name, cnt, int_value, float_value, string_value);
    // inst_list_insert_before/after(list, new_inst);


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
	instruction* new_inst = (instruction*) allocate_memory(sizeof(instruction), "instruction in DLL", BASIC);

    new_inst->relevant_node = active;
    new_inst->inst_type = type;
    new_inst->frame = frame;
    if (name == NULL) {
        new_inst->name = (char*) allocate_memory(sizeof(char) * 6, "instruction name", BASIC);
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

    list->last->next = new_inst;
    list->last = new_inst;

    list->active = new_inst;
}




// void inst_list_insert_after(instruction_list *list, instruction *new_inst) {
//     new_inst->next = list->active->next;
//     new_inst->prev = list->active;

//     if (list->last == list->active) {
//         list->last = new_inst;
//     }
//     else {
//         list->active->next->prev = new_inst;
//     }
//     list->active->next = new_inst;
// }


void inst_list_insert_before(instruction_list *list, instruction *new_inst) {
    new_inst->next = list->active;
    new_inst->prev = list->active->prev;

    list->active->prev->next = new_inst;
    list->active->prev = new_inst;

    list->active = new_inst;
}


void inst_list_delete_after(instruction_list *list) {
    instruction *inst = list->active->next;
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
    while (strcmp(list->active->name, func_name) != 0 || list->active->inst_type != FUNC_CALL) {
        list->active = list->active->prev;
    }
    // list->active is now void_function call, you can delete the instruction after it
}



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
            case IMPLICIT_NIL:
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
            case INT2FLOATS:
                fprintf(file, "INT2FLOATS\n");
                break;
            case INT2FLOATS_2:
                codegen_int2floats(inst);
                break;
            case DIVS:
                fprintf(file, "DIVS\n");
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
            case PUSHS_INT:
                fprintf(file, "PUSHS int@%s\n", inst->name);
                break;
            case PUSHS_FLOAT:
                fprintf(file, "PUSHS float@%s\n", inst->name);
                break;
            case PUSHS_STRING:
                fprintf(file, "PUSHS string@%s\n", inst->name);
                break;
            case PUSHS:
                fprintf(file, "PUSHS %cF@%s\n", inst->frame, inst->name);
                break;
            case EXCLAMATION_RULE:
                codegen_exclamation_rule(inst);
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
            case GEQ_RULE:
                codegen_geq_rule(inst);
                break;
            case QMS_RULE:
                codegen_qms_rule(inst);
                break;

            default:
                break;
        }
        inst = inst->next;
    }
}








//V případě chybějícího inicializačního výrazu se u proměnných typu zahrnujícího nil provádí implicitní inicializace hodnotou nil. 
//Proměnné ostatních typů nejsou bez inicializačního výrazu inicializovány.
void codegen_var_def(instruction *inst) {
    fprintf(file, "DEFVAR %cF@%s\n", inst->frame, inst->name);
}

void codegen_var_assign(instruction *inst) {
    fprintf(file, "POPS %cF@%s\n", inst->frame, inst->name);
}

void codegen_var_assign_nil(instruction *inst) {
    fprintf(file, "MOVE %cF@%s nil@nil\n", inst->frame, inst->name);
}




void codegen_func_def(instruction *inst) {
    fprintf(file, "JUMP !!skip_%s\n", inst->name);
    fprintf(file, "LABEL %s\n", inst->name);
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    for (int i = 1; i <= inst->cnt; i++) {
        AVL_tree* param = symtable_find_param(inst->relevant_node->symtable, i);
        fprintf(file, "DEFVAR LF@%s\n", param->key);
        fprintf(file, "MOVE LF@%s LF@$%d\n", param->key, i);
    } 
}

void codegen_func_def_return(instruction *inst) {
    fprintf(file, "POPS LF@$retval$\n");
    fprintf(file, "JUMP end_%s\n", inst->name);
}

void codegen_func_def_return_void(instruction *inst) {
    fprintf(file, "JUMP end_%s\n", inst->name);
}

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



void codegen_if_let(instruction *inst) {
    fprintf(file, "LABEL if_%d\n", inst->cnt);
    fprintf(file, "DEFVAR %cF@$cond_%d$\n", inst->frame, inst->cnt);
    fprintf(file, "TYPE %cF@$cond_%d$ %cF@%s\n", inst->frame, inst->cnt, inst->frame, inst->name);
    fprintf(file, "JUMPIFEQ else_%d %cF@$cond_%d$ string@nil\n", inst->cnt, inst->frame, inst->cnt); 
}

void codegen_if(instruction *inst) {
    fprintf(file, "LABEL if_%d\n", inst->cnt);
    fprintf(file, "DEFVAR %cF@$cond_%d$\n", inst->frame, inst->cnt);
    fprintf(file, "POPS %cF@$cond_%d$\n", inst->frame, inst->cnt);
    fprintf(file, "JUMPIFEQ else_%d %cF@$cond_%d$ bool@false\n", inst->cnt, inst->frame, inst->cnt);
}

void codegen_else(instruction *inst) {
    fprintf(file, "JUMP end_if_%d\n", inst->cnt);
    fprintf(file, "LABEL else_%d\n", inst->cnt);

}

void codegen_ifelse_end(instruction *inst) {
    fprintf(file, "LABEL end_if_%d\n", inst->cnt);
}   

void codegen_while_do(instruction *inst) {
    fprintf(file, "POPS %cF@$cond_%s$\n", inst->frame, inst->name);
    fprintf(file, "JUMPIFEQ end_%s %cF@$cond_%s$ bool@false\n", inst->name, inst->frame, inst->name);
}

void codegen_while_end(instruction *inst) {
    fprintf(file, "JUMP %s\n", inst->name);
    fprintf(file, "LABEL end_%s\n", inst->name);
}


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
    fprintf(file, "READ LF@$retval$ double\n");
    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
    fprintf(file, "LABEL !!skip_readDouble\n");
}

void codegen_write(instruction *inst) {
    fprintf(file, "CREATEFRAME\n");
    fprintf(file, "PUSHFRAME\n");
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
    fprintf(file, "MOVE LF@$check bool@false\n");
    fprintf(file, "LT LF@$check LF@$2 int@0\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "LT LF@$check LF@$3 int@0\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "GT LF@$check LF@$2 LF@$3\n");
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n");
    fprintf(file, "EQ LF@$check LF@$2 LF@$3\n"); //
    fprintf(file, "JUMPIFEQ !!load_nil LF@$check bool@true\n"); //
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
    //fprintf(file, "JUMPIFEQ !!load_result LF@$2 LF@$3\n");
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

void codegen_ord(instruction *inst) {
    fprintf(file, "JUMP !!skip_ord\n");
    fprintf(file, "LABEL ord\n");
    fprintf(file, "PUSHFRAME\n");
    fprintf(file, "DEFVAR LF@$retval$\n");
    fprintf(file, "STR2INT LF@$retval$ LF@$1 int@0\n");
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


void codegen_main(instruction *inst) {
    fprintf(file, ".IFJcode23\n");
    //fprintf(file, "DEFVAR GF$check1\n"); pomocne promenne
    //fprintf(file, "DEFVAR GF$tmp2\n"); dle potreby
    fprintf(file, "JUMP $$main\n");

    // space for sth before main
    // exit labels for error handling

    fprintf(file, "LABEL $$main\n");
    fprintf(file, "CREATEFRAME\n");
    fprintf(file, "PUSHFRAME\n");
}

void codegen_concat(instruction *inst) {
    fprintf(file, "DEFVAR GF@$$s%d$$\n", inst->cnt);
    fprintf(file, "DEFVAR GF@$$s%d$$\n", inst->cnt + 1);
    fprintf(file, "POPS GF@$$s%d$$\n", inst->cnt + 1);
    fprintf(file, "POPS GF@$$s%d$$\n", inst->cnt);
    fprintf(file, "CONCAT GF@$$s%d$$ GF@$$s%d$$ GF@$$s%d$$\n", inst->cnt, inst->cnt, inst->cnt + 1);
    fprintf(file, "PUSHS GF@$$s%d$$\n", inst->cnt);
}


void codegen_int2floats(instruction *inst) {
    fprintf(file, "DEFVAR GF@$$tmp%d$$\n", inst->cnt);
    fprintf(file, "POPS GF@$$tmp%d$$\n", inst->cnt);
    fprintf(file, "INT2FLOATS\n");
    fprintf(file, "PUSHS GF@$$tmp%d$$\n", inst->cnt);
}


void codegen_exclamation_rule(instruction *inst) {
    fprintf(file, "DEFVAR LF@$$excl%d\n", inst->cnt);
    fprintf(file, "POPS LF@$$excl%d\n", inst->cnt);
    fprintf(file, "PUSHS LF@$$excl%d\n", inst->cnt);
    fprintf(file, "PUSHS nil@nil\n");
    fprintf(file, "JUMPIFNEQS $RULE_EXCL_CORRECT%d$\n", inst->cnt);
    fprintf(file, "LABEL $RULE_EXCL_ERROR%d$\n", inst->cnt);
    fprintf(file, "WRITE string@Variable\\032is\\032NULL\n");
    fprintf(file, "EXIT int@7\n"); //doresit cislo chyby
    fprintf(file, "LABEL $RULE_EXCL_CORRECT%d$\n", inst->cnt);
    fprintf(file, "PUSHS LF@$$excl%d\n", inst->cnt);
}

void codegen_leq_rule(instruction *inst) {
    fprintf(file, "LTS\n");
    fprintf(file, "DEFVAR GF@$$leq%d$$\n", inst->cnt);
    fprintf(file, "DEFVAR GF@$$leq%d$$\n", inst->cnt + 1);
    fprintf(file, "POPS GF@$$leq%d$$\n", inst->cnt);
    fprintf(file, "EQS\n");
    fprintf(file, "POPS GF@$$leq%d$$\n", inst->cnt + 1);
    fprintf(file, "PUSHS GF@$$leq%d$$\n", inst->cnt);
    fprintf(file, "PUSHS GF@$$leq%d$$\n", inst->cnt + 1);
    fprintf(file, "ORS\n");
}

void codegen_geq_rule(instruction *inst) {
    fprintf(file, "GTS\n");
    fprintf(file, "DEFVAR GF@$$geq%d$$\n", inst->cnt);
    fprintf(file, "DEFVAR GF@$$geq%d$$\n", inst->cnt + 1);
    fprintf(file, "POPS GF@$$geq%d$$\n", inst->cnt);
    fprintf(file, "EQS\n");
    fprintf(file, "POPS GF@$$geq%d$$\n", inst->cnt + 1);
    fprintf(file, "PUSHS GF@$$geq%d$$\n", inst->cnt);
    fprintf(file, "PUSHS GF@$$geq%d$$\n", inst->cnt + 1);
    fprintf(file, "ORS\n");
}

void codegen_qms_rule(instruction *inst) {
    fprintf(file, "DEFVAR GF@$$rule_qms%d\n", inst->cnt);
    fprintf(file, "DEFVAR GF@$$rule_qms%d\n", inst->cnt + 1);
    fprintf(file, "POPS GF@$$rule_qms%d\n", inst->cnt);
    fprintf(file, "POPS GF@$$rule_qms%d\n", inst->cnt + 1);
    fprintf(file, "PUSHS GF@$$rule_qms%d\n", inst->cnt + 1);
    fprintf(file, "PUSHS nil@nil\n");
    fprintf(file, "JUMPIFNEQS $RULE_QMS_NOT_NILL%d$\n", inst->cnt);

    fprintf(file, "LABEL $RULE_QMS_NILL%d$\n", inst->cnt);
    fprintf(file, "PUSHS GF@$$rule_qms%d\n", inst->cnt);
    fprintf(file, "JUMP $END_RULE_QMS%d$\n",  inst->cnt);

    fprintf(file, "LABEL $RULE_QMS_NOT_NILL%d$\n", inst->cnt);
    fprintf(file, "PUSHS GF@$$rule_qms%d\n", inst->cnt + 1);

    fprintf(file, "LABEL $END_RULE_QMS%d$\n",  inst->cnt);
}


