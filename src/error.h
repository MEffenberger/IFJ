/**
 * @file error.h
 *
 * IFJ23 compiler
 *
 * @brief Error handling header file
 *
 * @author Marek Effenberger <xeffen00>
 */

#ifndef IFJ_ERROR_H
#define IFJ_ERROR_H

// Error related to lexical analysis, bad structure of the lexeme
#define ERROR_LEX 1

// Error related to syntax analysis, wrong syntax of the source code, missing header, etc.
#define ERROR_SYN 2

// Error related to semantic analysis, undefined function, redefinition of a variable, etc.
#define ERROR_SEM_UNDEF_FUN 3

// Error related to semantic analysis, wrong type of a variable, wrong number of parameters, wrong return value etc.
#define ERROR_SEM_TYPE 4

// Error related to semantic analysis, use of undefined/ unicnitialized variable, etc.
#define ERROR_SEM_UNDEF_VAR 5

// Error related to semantic analysis, missing/ redundant return value, etc.
#define ERROR_SEM_EXPR_RET 6

// Error related to semantic analysis, type compatibility in expressions, etc.
#define ERROR_SEM_EXPR_TYPE 7

// Error related to semantic analysis, wrong parameter type where there's no possibility to derive the type, etc.
#define ERROR_SEM_DERIV 8

// Error related to semantic analysis, other semantic errors not covered by the previous error codes.
#define ERROR_SEM_OTHER 9

// Error unrelated to input files, memory allocation, etc.
#define ERROR_INTERNAL 99

//magical function for error handling with memory deallocation
void error_exit(int error_code);


#endif //IFJ_ERROR_H
