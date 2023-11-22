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

#include "error.h"
#include "scanner.h"
#include "symtable.h"
#include "forest.h"
#include "token_stack.h"
#include "parser.h"
#include "cnt_stack.h"


// typedef enum instruction_name {
//     while_start,
//     while_end,
//     write
// } inst_name;


// // Instruction struct
// typedef struct instruction {
//     inst_name instruction_name; // Instruction opcode
//     void* operands; // Operands (addresses, registers, or immediate values)
//     int num_operands; // Number of operands
// } instruction;

// // Code block struct
// typedef struct codeblock {
//     instruction** instructions; // Vector or list of instructions
//     int num_instructions; // Number of instructions
// } code_block;






void codegen_var_def(char *name);
void codegen_var_assign(char *name);

void codegen_func_def();
void codegen_func_def_return();
void codegen_func_def_end();

void codegen_func_call_start();
void codegen_add_arg();
void codegen_func_call_end(char *label);

void codegen_if();
void codegen_else();
void codegen_ifelse_end();

void codegen_while_start();
void codegen_while_end();

void codegen_readString();
void codegen_readInt();
void codegen_readDouble();
void codegen_write();
void codegen_Int2Double();
void codegen_Double2Int();
void codegen_length();
void codegen_substring();
void codegen_ord();
void codegen_chr();


void codegen_generate_code_please();

#endif //IFJ_CODEGEN_H
