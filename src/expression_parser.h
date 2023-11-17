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
#include "error.h"

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

#endif