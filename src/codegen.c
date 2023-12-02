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
void inst_list_search_while(instruction_list *list) {

    // move list->active based on the name of the node found of the outermost while loop found by forest_search_while (while_X)
    while (strcmp(list->active->name, forest_search_while(active)->name) != 0) {
        list->active = list->active->prev;
    }
    // list->active is now outermost while loop, you can insert before it
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
            case FUNC_CALL_END:
                codegen_func_call_end(inst);
                break;
            case FUNC_CALL_END_VOID:
                codegen_func_call_end_void(inst);
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
            case WHILE_START:
                codegen_while_start(inst);
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
                printf("INT2FLOATS\n");
                break;
            case INT2FLOATS_2:
                codegen_int2floats(inst);
            case DIVS:
                printf("DIVS\n");
                break;
            case IDIVS:
                printf("IDIVS\n");
                break;
            case PUSHS_INT_CONST:
                printf("PUSHS int@%d\n", inst->int_value);
                break;
            case PUSHS_FLOAT_CONST:
                printf("PUSHS float@%a\n", inst->float_value);
                break;
            case PUSHS_STRING_CONST:
                printf("PUSHS string@%s\n", inst->string_value);
                break;
            case PUSHS_NIL:
                printf("PUSHS nil@nil\n");
                break;
            case PUSHS_INT:
                printf("PUSHS int@%s\n", inst->name);
                break;
            case PUSHS_FLOAT:
                printf("PUSHS float@%s\n", inst->name);
                break;
            case PUSHS_STRING:
                printf("PUSHS string@%s\n", inst->name);
                break;
            case PUSHS:
                printf("PUSHS %cF@%s\n", inst->frame, inst->name);
                break;
            case EXCLAMATION_RULE:
                codegen_exclamation_rule(inst);
                break;
            case ADDS:
                printf("ADDS\n");
                break;
            case MULS:
                printf("MULS\n");
                break;
            case SUBS:
                printf("SUBS\n");
                break;
            case LTS:
                printf("LTS\n");
                break;
            case EQS:
                printf("EQS\n");
                break;
            case ORS:
                printf("ORS\n");
                break;
            case GTS:
                printf("GTS\n");
                break;
            case NOTS:
                printf("NOTS\n");
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








// // V případě chybějícího inicializačního výrazu se u proměnných typu zahrnujícího nil provádí implicitní inicializace hodnotou nil. Proměnné ostatních typů nejsou bez inicializačního výrazu inicializovány.
void codegen_var_def(instruction *inst) {
    printf("DEFVAR %cF@%s\n", inst->frame, inst->name);
}

void codegen_var_assign(instruction *inst) {
    printf("POPS %cF@%s\n", inst->frame, inst->name);
}




void codegen_func_def(instruction *inst) {
    printf("JUMP !!skip_%s\n", inst->name);
    printf("LABEL %s\n", inst->name);
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    for (int i = 1; i <= inst->cnt; i++) {
        AVL_tree* param = symtable_find_param(inst->relevant_node->symtable, i);
        printf("DEFVAR LF@%s\n", param->key);
        printf("MOVE LF@%s LF@$%d\n", param->key, i);
    } 
}

void codegen_func_def_return(instruction *inst) {
    printf("POPS LF@$retval$\n");
    printf("JUMP end_%s\n", inst->name);
}

void codegen_func_def_return_void(instruction *inst) {
    printf("JUMP end_%s\n", inst->name);
}

void codegen_func_def_end(instruction *inst) {
    printf("LABEL end_%s\n", inst->name);
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_%s\n", inst->name);
}




void codegen_func_call_start(instruction *inst) {
    add_arg_cnt = 0;
    printf("CREATEFRAME\n");
}

void codegen_add_arg(instruction *inst) {
    printf("DEFVAR TF@$%d\n", ++add_arg_cnt);
    printf("POPS TF@$%d\n", add_arg_cnt);
}

void codegen_func_call_end(instruction *inst) {
    printf("CALL %s\n", inst->name);
    printf("PUSHS TF@$retval$\n");
}

void codegen_func_call_end_void(instruction *inst) {
    printf("CALL %s\n", inst->name);
}




void codegen_if_let(instruction *inst) {
    printf("LABEL if_%d\n", inst->cnt);
    printf("DEFVAR %cF@$cond_%d$\n", inst->frame, inst->cnt);
    printf("TYPE %cF@$cond_%d$ %cF@%s\n", inst->frame, inst->cnt, inst->frame, inst->name);
    printf("JUMPIFEQ else_%d %cF@$cond_%d$ string@nil\n", inst->cnt, inst->frame, inst->cnt); 
}

void codegen_if(instruction *inst) {
    printf("LABEL if_%d\n", inst->cnt);
    printf("DEFVAR %cF@$cond_%d$\n", inst->frame, inst->cnt);
    printf("POPS %cF@$cond_%d$\n", inst->frame, inst->cnt);
    printf("JUMPIFEQ else_%d %cF@$cond_%d$ bool@false\n", inst->cnt, inst->frame, inst->cnt);
}

void codegen_else(instruction *inst) {
    printf("JUMP end_if_%d\n", inst->cnt);
    printf("LABEL else_%d\n", inst->cnt);

}

void codegen_ifelse_end(instruction *inst) {
    printf("LABEL end_if_%d\n", inst->cnt);
}   




void codegen_while_start(instruction *inst) {
    printf("LABEL %s\n", inst->name);
    printf("DEFVAR %cF@$cond_%s$\n", inst->frame, inst->name);
    printf("POPS %cF@$cond_%s$\n", inst->frame, inst->name);
    printf("JUMPIFEQ end_%s %cF@$cond_%s$ bool@false\n", inst->name, inst->frame, inst->name);
}

void codegen_while_end(instruction *inst) {
    printf("JUMP %s\n", inst->name);
    printf("LABEL end_%s\n", inst->name);
}




void codegen_readString(instruction *inst) {
    printf("JUMP !!skip_readString\n");
    printf("LABEL readString\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("READ LF@$retval$ string\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_readString\n");
}

void codegen_readInt(instruction *inst) {
    printf("JUMP !!skip_readInt\n");
    printf("LABEL readInt\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("READ LF@$retval$ int\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_readInt\n");
}

void codegen_readDouble(instruction *inst) {
    printf("JUMP !!skip_readDouble\n");
    printf("LABEL readDouble\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("READ LF@$retval$ double\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_readDouble\n");
}

void codegen_write(instruction *inst) {
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    for (int i = 1; i <= inst->cnt; i++) {
        printf("DEFVAR LF@$%d\n", write_renamer + i);
        printf("POPS LF@$%d\n", write_renamer + i);
    }
    for (int i = inst->cnt; i >= 1; i--) {
        printf("WRITE LF@$%d\n", write_renamer + i);
    }
    write_renamer += inst->cnt;
    printf("POPFRAME\n");
}

void codegen_Int2Double(instruction *inst) {
    printf("JUMP !!skip_Int2Double\n");  
    printf("LABEL Int2Double\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("INT2FLOAT LF@$retval$ LF@$1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_Int2Double\n");
}

void codegen_Double2Int(instruction *inst) {  
    printf("JUMP !!skip_Double2Int\n");
    printf("LABEL Double2Int\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("FLOAT2INT LF@$retval$ LF@$1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_Double2Int\n");
}

void codegen_length(instruction *inst) {
    printf("JUMP !!skip_length\n");
    printf("LABEL length\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("STRLEN LF@$retval$ LF@$1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_length\n");
}

void codegen_substring(instruction *inst) {
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

void codegen_ord(instruction *inst) {
    printf("JUMP !!skip_ord\n");
    printf("LABEL ord\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("STR2INT LF@$retval$ LF@$1 int@0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_ord\n");  
}

void codegen_chr(instruction *inst) {
    printf("JUMP !!skip_chr\n");
    printf("LABEL chr\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@$retval$\n");
    printf("INT2CHAR LF@$retval$ LF@$1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL !!skip_chr\n");
}


void codegen_main(instruction *inst) {
    printf(".IFJcode23\n");
    //printf("DEFVAR GF$check1\n"); pomocne promenne
    //printf("DEFVAR GF$tmp2\n"); dle potreby
    printf("JUMP $$main\n");

    // space for sth before main
    // exit labels for error handling

    printf("LABEL $$main\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
}

void codegen_concat(instruction *inst) {
    printf("DEFVAR GF@$$s%d$$\n", inst->cnt);
    printf("DEFVAR GF@$$s%d$$\n", inst->cnt + 1);
    printf("POPS GF@$$s%d$$\n", inst->cnt + 1);
    printf("POPS GF@$$s%d$$\n", inst->cnt);
    printf("CONCAT GF@$$s%d$$ GF@$$s%d$$ GF@$$s%d$$\n", inst->cnt, inst->cnt, inst->cnt + 1);
    printf("PUSHS GF@$$s%d$$\n", inst->cnt);
}


void codegen_int2floats(instruction *inst) {
    printf("DEFVAR GF@$$tmp%d$$\n", inst->cnt);
    printf("POPS GF@$$tmp%d$$\n", inst->cnt);
    printf("INT2FLOATS\n");
    printf("PUSHS GF@$$tmp%d$$\n", inst->cnt);
}


void codegen_exclamation_rule(instruction *inst) {
    printf("DEFVAR GF@$$excl%d\n", inst->cnt);
    printf("POPS GF@$$excl%d\n", inst->cnt);
    printf("PUSHS GF@$$excl%d\n", inst->cnt);
    printf("PUSHS nil@nil\n");
    printf("JUMPIFNEQS $RULE_EXCL_CORRECT$\n");
    printf("LABEL $RULE_EXCL_ERROR$\n");
    printf("WRITE string@Variable\\032is\\032NULL\n");
    printf("EXIT int@7\n"); //doresit cislo chyby
    printf("LABEL $RULE_EXCL_CORRECT$\n");
    printf("PUSHS GF@$$excl%d\n", inst->cnt);
}

void codegen_leq_rule(instruction *inst) {
    printf("LTS\n");
    printf("DEFVAR GF@$$leq%d$$\n", inst->cnt);
    printf("DEFVAR GF@$$leq%d$$\n", inst->cnt + 1);
    printf("POPS GF@$$leq%d$$\n", inst->cnt);
    printf("EQS\n");
    printf("POPS GF@$$leq%d$$\n", inst->cnt + 1);
    printf("PUSHS GF@$$leq%d$$\n", inst->cnt);
    printf("PUSHS GF@$$leq%d$$\n", inst->cnt + 1);
    printf("ORS\n");
}

void codegen_geq_rule(instruction *inst) {
    printf("GTS\n");
    printf("DEFVAR GF@$$geq%d$$\n", inst->cnt);
    printf("DEFVAR GF@$$geq%d$$\n", inst->cnt + 1);
    printf("POPS GF@$$geq%d$$\n", inst->cnt);
    printf("EQS\n");
    printf("POPS GF@$$geq%d$$\n", inst->cnt + 1);
    printf("PUSHS GF@$$geq%d$$\n", inst->cnt);
    printf("PUSHS GF@$$geq%d$$\n", inst->cnt + 1);
    printf("ORS\n");
}

void codegen_qms_rule(instruction *inst) {
    printf("DEFVAR GF@$$rule_qms%d\n", inst->cnt);
    printf("DEFVAR GF@$$rule_qms%d\n", inst->cnt + 1);
    printf("POPS GF@$$rule_qms%d\n", inst->cnt);
    printf("POPS GF@$$rule_qms%d\n", inst->cnt + 1);
    printf("PUSHS GF@$$rule_qms%d\n", inst->cnt + 1);
    printf("PUSHS nil@nil\n");
    printf("JUMPIFNEQS $RULE_QMS_NOT_NILL$\n");

    printf("LABEL $RULE_QMS_NILL$\n");
    printf("PUSHS GF@$$rule_qms%d\n", inst->cnt);
    printf("JUMP $END_RULE_QMS\n");

    printf("LABEL $RULE_QMS_NOT_NILL$\n");
    printf("PUSHS GF@$$rule_qms%d\n", inst->cnt + 1);

    printf("LABEL $END_RULE_QMS\n$");
}


