/**
 * @file scanner.h
 * 
 * IFJ23 compiler
 * 
 * @brief Lexical analysis for IFJ23 compiler
 *
 * @author Dominik Horut <xhorut01>
 * @author Samuel Hejnicek <xhejni00>
 */
#ifndef IFJ_SCANNER_H
#define IFJ_SCANNER_H

#include <stdbool.h>
#include <stdio.h>
#include "string_vector.h"
#include "symtable.h"

/// Enumeration of all types of tokens
typedef enum token_types {

    TOKEN_WHITESPACE,
    TOKEN_TAB,
    TOKEN_EOL,
    TOKEN_EOF,
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
    TOKEN_RET_TYPE,
    TOKEN_ID,
    TOKEN_KEYWORD_QM,
    TOKEN_NUM,
    TOKEN_EXP,
    TOKEN_DEC,
    TOKEN_STRING,
    TOKEN_ML_STRING,
    TOKEN_KEYWORD,
    TOKEN_UNDERSCORE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_DIVIDE,

    TOKEN_DOLLAR,
    TOKEN_SHIFT,  
    TOKEN_EXPRESSION, 
    TOKEN_LITERAL_INT,
    TOKEN_LITERAL_DOUBLE,
    TOKEN_LITERAL_STRING,
    TOKEN_BOOL

} token_type_t;

/// Enumeration of all states of automata
typedef enum state{

    S_START,
    S_COLON,   
    S_COMMA,
    S_QM,
    S_DOUBLE_QM,  
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
    S_RET_TYPE,    
    S_LPAR,
    S_RPAR,
    S_UNDERSCORE,
    S_ID, 
    S_ID_QM,      
    S_NUM,
    S_NUM_DOT,
    S_NUM_E,
    S_NUM_E_SIGN,
    S_DEC, 
    S_EXP,

    S_START_QUOTES, 
    S_END_QUOTES,    
    S_START_ESC_SENTENCE, 
    S_START_HEX, 
    S_LEFT_BRACKET,
    S_FIRST_HEX, 

    S_THREE_QUOTES,
    S_START_MULTILINE, 

    S_SL_COM,
    S_NESTED_COM,
    S_NESTED_END,

    S_STR_EMPTY,
    S_IS_MULTILINE,
    S_END_MULTILINE,
    S_FAKE_END_MULTILINE

} automat_state_t;


/// Enumeration of all keywords
typedef enum keyword{

    KW_DOUBLE,  // Double
    KW_INT,     // Int
    KW_STRING,  // String
    KW_ELSE,    // else 
    KW_FUNC,    // func
    KW_IF,      // if
    KW_LET,     // let
    KW_NIL,     // nil
    KW_RETURN,  // return
    KW_VAR,     // var
    KW_WHILE,   // while

    // In built functions

    KW_RD_STR,   // readString()
    KW_RD_INT,   // readInt()
    KW_RD_DBL,   // readDouble()
    KW_WRT,      // write()
    KW_INT_2_DBL,// Int2Double()
    KW_DBL_2_INT,// Double2Int()
    KW_LENGHT,   // lenght()
    KW_SUBSTR,   // substring()
    KW_ORD,      // ord()
    KW_CHR,      // chr()

    KW_GLOBAL,   // global (for global forest node)

}keyword_t;

/// @brief Enumeration of types esential for semantic analysis
typedef enum expression_type{
    ID,
    CONST,
} expression_type_t;

/// @brief Struct of data types 
typedef struct token_value {
    int integer;
    double type_double;
    keyword_t keyword;
    vector* vector;
} value_type_t;

/// @brief Struct of token
typedef struct token {
    value_type_t value;
    token_type_t type;
    bool prev_was_eol;
    expression_type_t exp_type;
    data_type exp_value;
    bool was_id;
} token_t;

/**
 * @brief Function checks if the token is whether a keyword or an identifier
 * 
 * @param v string value of token 
 * @return keyword_t type of token 
*/
keyword_t compare_keyword(vector* v);

/**
 * @brief Function checks if there is correct indent in multiline string
 * 
 * @param cnt_array array of every line indent in multiline string 
 * @param size indent of multiline end 
 * 
 * @return bool true if indent is correct, false otherwise
*/
bool check_indent(int* cnt_array, int size);

/**
 * @brief Function to delete token and free all the memory it had allocated
 * 
 * @param token token to be deleted
*/
void destroy_token(token_t* token);

/**
 * @brief Function to get token from source code
 * 
 * @return token_t* pointer to newly allocated token
*/
token_t* get_me_token();

#endif
