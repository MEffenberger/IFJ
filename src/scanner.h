/**
 * @file scanner.h
 * 
 * IFJ23 compiler
 * 
 * @brief Lexical analysis for IFJ23 compiler
 *
 * @author Dominik Horut <xhorut01>
 */

#include "error.h"


typedef enum types {
    S_START,
    S_MINUS,
    S_PLUS,
    S_MULTIPLY,

    S_LESS,
    S_LESS_EQ,
    S_GREAT,
    S_GREAT_EQ,
    S_LPAR,
    S_RPAR,
    S_EQ,
    S_EQEQ,
    S_EXCLAM,
    S_EXCLAMEQ,
    S_COMMA,
    S_COLON,
    S_DOUBLE_QM,
    S_RET_TYPE, //->
    S_ID,
    S_ID_QM, //ID?
    S_NUM,
    S_EXP,
    S_DEC
} type_t;

typedef enum states {

} state_t;


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

typedef enum state{

    // Not finite states

    S_SINGLE_QM,    // first question mark in ?? operator        
    S_UNDERSCORE,   // _ underscore
    S_DOT,
    S_E,            // exponent e or E
    S_E_SIGN,       // e+ or e- in exponent
    
    // not sure abt
    /*************/

    S_STR_START,     // first quotation mark "
    S_STR_SIMPLE,    // just two quotation marks
    S_

    
}state_t;