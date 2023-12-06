/**
 * @file codegen.h
 *
 * IFJ23 compiler
 *
 * @brief Generator of IFJcode23 with doubly linked list of instructions
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */

#ifndef IFJ_CODEGEN_H
#define IFJ_CODEGEN_H

#include "forest.h"

// Enumeration of instruction types
typedef enum instruction_type {
    MAIN,
    VAR_DEF,
    VAR_ASSIGN,
    VAR_ASSIGN_NIL,
    IMPLICIT_NIL,
    FUNC_DEF,
    FUNC_DEF_RETURN,
    FUNC_DEF_RETURN_VOID,
    FUNC_DEF_END,
    FUNC_CALL_START,
    ADD_ARG,
    FUNC_CALL,
    FUNC_CALL_RETVAL,
    IF_LABEL,
    IF_DEFVAR,
    IF_LET,
    IF,
    ELSE,
    IFELSE_END,
    WHILE_COND_DEF,
    WHILE_START,
    WHILE_DO,
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
    CONCAT_DEFVAR,
    INT2FLOATS,
    INT2FLOATS_2,
    INT2FLOATS_2_DEFVAR,
    IDIVS,
    DIVS,
    DIV_BY_ZERO,
    IDIV_BY_ZERO,
    DIV_ZERO_DEFVAR,
    IDIV_ZERO_DEFVAR,
    PUSHS_INT_CONST,
    PUSHS_FLOAT_CONST,
    PUSHS_STRING_CONST,
    PUSHS_NIL,
    PUSHS,
    EXCLAMATION_RULE,
    EXCLAMATION_RULE_DEFVAR,
    ADDS,
    MULS,
    SUBS,
    LTS,
    EQS,
    ORS,
    GTS,
    NOTS,
    LEQ_RULE,
    LEQ_RULE_DEFVAR,
    GEQ_RULE,
    GEQ_RULE_DEFVAR,
    QMS_RULE,
    QMS_RULE_DEFVAR,
} inst_type;


// Structure of instruction holding its informations
typedef struct s_instruction {
	struct s_instruction *prev; // previous instruction
	struct s_instruction *next; // next instruction

    forest_node *relevant_node; // relevant node from forest

    inst_type inst_type; // type of instruction

    char frame; // G(F), L(F)
    
    char *name; // name of variable or function
    int cnt; // counter for relevant naming

    int int_value;
    double float_value;
    char *string_value;
} instruction;


// Structure of instruction list element
typedef struct {
	instruction *first;
	instruction *active;
	instruction *last;
} instruction_list;


/**
 * @brief Instruction list initialization
 * 
 * @param list Instruction list to be initialized
 */
void inst_list_init(instruction_list *list);

/**
 * @brief Instruction initialization - sets the relevant informations and the rest to default
 * 
 * @param type Type of instruction
 * @param frame G(F), L(F)
 * @param name Name of variable or function
 * @param cnt Counter for relevant naming
 * @param int_value Integer value
 * @param float_value Float value
 * @param string_value String value
 * @return instruction* 
 */
instruction* inst_init(inst_type type,
                        char frame,
                        char *name,
                        int cnt,
                        int int_value,
                        double float_value,
                        char *string_value);


/**
 * @brief Insert instruction at the end of the list
 * 
 * @param list Instruction list
 * @param new_inst Instruction to be inserted
 */
void inst_list_insert_last(instruction_list *list, instruction *new_inst);

/**
 * @brief Insert instruction before the active instruction
 * 
 * @param list Instruction list
 * @param new_inst Instruction to be inserted
 */
void inst_list_insert_before(instruction_list *list, instruction *new_inst);

/**
 * @brief Delete instruction after the active instruction
 * 
 * @param list Instruction list
 */
void inst_list_delete_after(instruction_list *list);

/**
 * @brief Dispose of the whole instruction list
 * 
 * @param list Instruction list
 */
void inst_list_dispose(instruction_list *list);

/**
 * @brief Search for the outermost while instruction
 * 
 * @param list Instruction list
 * @param while_name Name of the while instruction
 */
void inst_list_search_while(instruction_list *list, char *while_name);

/**
 * @brief Search for the void function calls to remove the return value
 * 
 * @param list Instruction list
 * @param func_name Name of the function
 */
void inst_list_search_void_func_call(instruction_list *list, char *func_name);

/**
 * @brief Goes through the instruction list and generates the IFJcode23 for each instruction
 * 
 * @param list Instruction list
 */
void codegen_generate_code_please(instruction_list *list);

/**
 * @brief Helping functions for generating the IFJcode23 to separate larger blocks of printing
 * 
 * @param inst Instruction to be generated
 */
void codegen_var_def(instruction *inst);
void codegen_var_assign(instruction *inst);
void codegen_var_assign_nil(instruction *inst);
void codegen_func_def(instruction *inst);
void codegen_func_def_return_void(instruction *inst);
void codegen_func_def_return(instruction *inst);
void codegen_func_def_end(instruction *inst);
void codegen_func_call_start(instruction *inst);
void codegen_add_arg(instruction *inst);
void codegen_if_let(instruction *inst);
void codegen_if(instruction *inst);
void codegen_else(instruction *inst);
void codegen_ifelse_end(instruction *inst);
void codegen_while_do(instruction *inst);
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
void codegen_div_zero(instruction *inst);
void codegen_idiv_zero(instruction *inst);

#endif //IFJ_CODEGEN_H
