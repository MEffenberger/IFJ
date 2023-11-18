/**
 * @file error.h
 *
 * IFJ23 compiler
 *
 * @brief Error handling header file
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
#include "queue.h"


// Instruction struct
typedef struct Instruction {
    char opcode; // Instruction opcode
    void* operands[3]; // Operands (addresses, registers, or immediate values)
    int numOperands; // Number of operands
} Instruction;

// Code block struct
typedef struct CodeBlock {
    struct CodeBlock* parent; // Pointer to parent code block
    SymbolTable* symbolTable; // Symbol table for local variables
    Instruction* instructions; // Vector or list of instructions
    int numInstructions; // Number of instructions
} CodeBlock;


#endif //IFJ_CODEGEN_H
