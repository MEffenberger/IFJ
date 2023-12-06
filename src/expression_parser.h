/**
 * @file expression_parser.h
 * 
 * IFJ23 compiler
 * 
 * @brief Expression parser for IFJ23 compiler
 *
 * @author Dominik Horut <xhorut01>
 * @author Samuel Hejnicek <xhejni00>
 */

#ifndef IFJ_EXPRESSION_PARSER_H
#define IFJ_EXPRESSION_PARSER_H

#include "token_stack.h"

typedef enum expression_rules{

    RULE_PAR,       // E -> (E)
    RULE_OPERAND,   // E -> i
    RULE_ADD,       // E -> E+E
    RULE_SUB,       // E -> E-E
    RULE_MUL,       // E -> E*E
    RULE_DIV,       // E -> E/E
    RULE_LESS,      // E -> E<E
    RULE_GTR,       // E -> E>E
    RULE_LEQ,       // E -> E<=E
    RULE_GEQ,       // E -> E>=E
    RULE_EQ,        // E -> E==E
    RULE_NEQ,       // E -> E!=E
    RULE_EXCL,      // E -> E!
    RULE_QMS,       // E -> E??E
    NOT_RULE        // NOT A RULE

} expression_rules_t;




/**
 * @brief Get the index of token from precedence table
 * 
 * @param token Type of token 
 * @return int index of token in precedence table
 */
int get_index(token_type_t token);

/**
 * @brief Creates new token 
 * 
 * @param token_type specifies type of token to be created
 * @return token_t* pointer to newly allocated token
 */
token_t* token_create(token_type_t token_type);


/**
 * @brief Puts a shift symbol on stack
 * 
 * @param stack Pointer to the stack of tokens
 */
void token_shift(token_stack* stack);

/**
 * @brief Function to find appropriate rule for reducing expressions on stack
 * 
 * @param token1 Pointer to the token on top of the stack
 * @param token2 Pointer to the 2nd token
 * @param token3 Pointer to the 3rd token
 * @param number_of_tokens Number of tokens for which the rule should be found
 * @return expression_rules_t Type of rule to be applied
 */
expression_rules_t find_reduce_rule(token_t* token1, token_t* token2, token_t* token3, int number_of_tokens);



/**
 * @brief Converts QM token types to non qm token types
 * 
 * @param tmp1 Pointer to the token to be converted
 */
void convert_qm(token_t* token1);


/**
 * @brief Checks the tokens for sematic analysis and if possible converts them
 * 
 * @param token1 Pointer to the first token to be checked
 * @param token2 Pointer to the second token to be checked
 * @param token3 Pointer to the third token to be checked
 */
void check_types(token_t* token1, token_t* token2, token_t* token3);

/**
 * @brief Prints specific tokens for operators Lequal and Gequal to stdout in IFJcode 
 * 
 * @param token1 Pointer to first token to be printed
 * @param token2 Pointer to second token to be printed
 */
void push_for_leq_geq(token_t* token1, token_t* token2);

/**
 * @brief Function core parser uses to process expressions
 * 
 * @param return_type Return type of of the expression parser expects
 */
void call_expr_parser(data_type return_type);

#endif //IFJ_EXPRESSION_PARSER_H
