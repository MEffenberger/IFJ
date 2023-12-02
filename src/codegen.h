/**
 * @file codegen.h
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

#ifndef IFJ_CODEGEN_H
#define IFJ_CODEGEN_H

#include "forest.h"

typedef enum instruction_type {
    MAIN,
    VAR_DEF,
    VAR_ASSIGN,
    FUNC_DEF,
    FUNC_DEF_RETURN,
    FUNC_DEF_RETURN_VOID,
    FUNC_DEF_END,
    FUNC_CALL_START,
    ADD_ARG,
    FUNC_CALL_END,
    IF_LET,
    IF,
    ELSE,
    IFELSE_END,
    WHILE_START,
    WHILE_END,
    READ_STRING,
    READ_INT,
    READ_DOUBLE,
    WRITE,
    INT2DOUBLE,
    DOUBLE2INT,
    LENGTH,
    SUBSTRING,
    ORD,
    CHR,
    CONCAT,
    INT2FLOATS,
    INT2FLOATS_2,
    IDIVS,
    DIVS,
    PUSHS_INT_CONST,
    PUSHS_FLOAT_CONST,
    PUSHS_STRING_CONST,
    PUSHS_NIL,
    PUSHS_INT,
    PUSHS_FLOAT,
    PUSHS_STRING,
    PUSHS,
    EXCLAMATION_RULE,
    ADDS,
    MULS,
    SUBS,
    LTS,
    EQS,
    ORS,
    GTS,
    NOTS,
    LEQ_RULE,
    GEQ_RULE,
    QMS_RULE




} inst_type;


typedef struct s_instruction {
	struct s_instruction *prev;
	struct s_instruction *next;

    forest_node *relevant_node;

    inst_type inst_type;

    char frame; // G(F), L(F), T(F)
    
    char *name;
    int cnt;

    int int_value;
    double float_value;
    char *string_value;

} instruction;


typedef struct {
	instruction *first;
	instruction *active;
	instruction *last;
} instruction_list;


void inst_list_init(instruction_list *list);

/**
 * @brief Instruction initialization
 * 
 * @param type Type of the instruction
 * @param frame G(F), L(F), T(F)
 * @param name 
 * @param cnt 
 * @param int_value 
 * @param float_value 
 * @param string_value 
 * @return instruction* Initialized instruction
 */
instruction* inst_init(inst_type type,
                        char frame,
                        char *name,
                        int cnt,
                        int int_value,
                        double float_value,
                        char *string_value);

void inst_list_insert_last(instruction_list *list, instruction *new_inst);
void inst_list_insert_before(instruction_list *list, instruction *new_inst);
void inst_list_dispose(instruction_list *list);



void codegen_var_def(instruction *inst);
void codegen_var_assign(instruction *inst);

void codegen_func_def(instruction *inst);
void codegen_func_def_return_void(instruction *inst);
void codegen_func_def_return(instruction *inst);
void codegen_func_def_end(instruction *inst);

void codegen_func_call_start(instruction *inst);
void codegen_add_arg(instruction *inst);
void codegen_func_call_end(instruction *inst);

void codegen_if_let(instruction *inst);
void codegen_if(instruction *inst);
void codegen_else(instruction *inst);
void codegen_ifelse_end(instruction *inst);

void codegen_while_start(instruction *inst);
void codegen_while_end(instruction *inst);

void codegen_readString(instruction *inst);
void codegen_readInt(instruction *inst);
void codegen_readDouble(instruction *inst);
void codegen_write(instruction *inst);
void codegen_Int2Double(instruction *inst);
void codegen_Double2Int(instruction *inst);
void codegen_length(instruction *inst);
void codegen_substring(instruction *inst);
void codegen_ord(instruction *inst);
void codegen_chr(instruction *inst);
void codegen_main(instruction *inst);

void codegen_concat(instruction *inst);
void codegen_int2floats(instruction *inst);
void codegen_exclamation_rule(instruction *inst);
void codegen_leq_rule(instruction *inst);
void codegen_geq_rule(instruction *inst);
void codegen_qms_rule(instruction *inst);



void codegen_generate_code_please(instruction_list *list);

#endif //IFJ_CODEGEN_H
