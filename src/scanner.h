/**
 * @file scanner.h
 * 
 * IFJ23 compiler
 * 
 * @brief Lexical analysis for IFJ23 compiler
 *
 * @author Dominik Horut <xhorut01>
 * @author Samuel Hejnicek
 */
#ifndef IFJ_SCANNER_H
#define IFJ_SCANNER_H
#include "error.h"


typedef enum token_types {
    TOKEN_WHITESPACE,
    TOKEN_TAB,
    TOKEN_EOL,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_MULTIPLY,
    TOKEN_LESS,
    TOKEN_LESS_EQ,
    TOKEN_GREAT,
    TOKEN_GREAT_EQ,
    TOKEN_LPAR,
    TOKEN_RPAR,
    TOKEN_EQ,
    TOKEN_EQEQ,
    TOKEN_EXCLAM,
    TOKEN_EXCLAMEQ,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_DOUBLE_QM,
    TOKEN_RET_TYPE, //->
    TOKEN_ID,
    TOKEN_ID_QM, //ID?
    TOKEN_NUM,
    TOKEN_EXP,
    TOKEN_DEC,
    TOKEN_STRING,
    TOKEN_ML_STRING
} token_type_t;

typedef enum state{

    S_START,
    S_COLON,    // id type declaration
    S_COMMA,
    S_QM,
    S_DOUBLE_QM,    // ?? operator
    S_GTR,
    S_GTR_EQ,
    S_LESS,
    S_LESS_EQ,
    S_EQ,
    S_EQEQ,        
    S_EXCLAM,
    S_EXCLAMEQ,
    S_MULTIPLY,
    S_PLUS,
    S_MINUS,
    S_RET_TYPE,     // -> returned type
    S_LPAR,
    S_RPAR,
    S_UNDERSCORE,
    S_ID, 
    S_ID_QM,      // String?
    S_NUM,
    S_NUM_DOT,
    S_NUM_E,
    S_NUM_E_SIGN,
    S_DEC, 
    S_EXP,

    
}state_t;

typedef enum keyword{

    KW_DOUBLE,  // Double
    KW_ELSE,    // else 
    KW_FUNC,    // func
    KW_IF,      // if
    KW_INT,     // Int
    KW_LET,     // let
    KW_NIL,     // nil
    KW_RETURN,  // return
    KW_STRING,  // String
    KW_VAR,     // var
    KW_WHILE,   // while

}keyword_t;

#endif