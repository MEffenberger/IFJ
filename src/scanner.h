/**
 * @file scanner.h
 * 
 * IFJ23 compiler
 * 
 * @brief Lexical analysis for IFJ23 compiler
 *
 * @author Dominik Horut <xhorut01>
 */

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

    // Finite states

    S_START,
    S_TYPE_DECL,    // id type declaration
    S_COMMA,
    S_DOUBLE_QM,    // ?? operator
    S_GTR,
    S_GTR_EQ,
    S_LESS,
    S_LESS_EQ,
    S_ASSIGN,
    S_EQ,
    S_EXCL_MARK,
    S_NOT_EQ,
    S_ASTERISK,
    S_PLUS,
    S_MINUS,
    S_RET_TYPE,     // -> returned type
    S_LPAR,
    S_RPAR,
    S_ID, 
    S_T_RECOG,      // String? type recognition
    S_NUM,
    S_EXP,
    
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