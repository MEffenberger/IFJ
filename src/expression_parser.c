/**
 * @file expression_parser.c
 * 
 * IFJ23 compiler
 * 
 * @brief Expression parser implemetation
 *
 * @author Dominik Horut <xhorut01>
 * @author Samuel Hejnicek <xhejni00>
 */
#include "expression_parser.h"
#include "token_stack.h"


#define TABLE_SIZE 16

static char precedence_table[TABLE_SIZE][TABLE_SIZE] = {

/*    *//* +   -   *   /   <   >   <=  >=  !=  ==  !   ??  (   )   i   $   */
/* +  */ {'>','>','<','<','>','>','>','>','>','>','<','>','<','>','<','>'},
/* -  */ {'>','>','<','<','>','>','>','>','>','>','<','>','<','>','<','>'},
/* *  */ {'>','>','>','>','>','>','>','>','>','>','<','>','<','>','<','>'},
/* /  */ {'>','>','>','>','>','>','>','>','>','>','<','>','<','>','<','>'},
/* <  */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* >  */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* <= */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* >= */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* != */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* == */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* !  */ {'>','>','>','>','>','>','>','>','>','>',' ','>',' ','>','>','>'},
/* ?? */ {'<','<','<','<','<','<','<','<','<','?','<','<','<','>','<','>'},
/* (  */ {'<','<','<','<','<','<','<','<','<','<',' ','<','<','=','<',' '},
/* )  */ {'>','>','>','>','>','>','>','>','>','>','>','>',' ','>',' ','>'},
/* i  */ {'>','>','>','>','>','>','>','>','>','>','>','>',' ','>',' ','>'},
/* $  */ {'<','<','<','<','<','<','<','<','<','<','<','<','<',' ','<',' '},

};


int get_index(token_type_t token){

    switch(token){

        case TOKEN_PLUS:
        return 0;

        case TOKEN_MINUS:
        return 1;

        case TOKEN_MULTIPLY:
        return 2;

        case TOKEN_DIVIDE:  
        return 3;

        case TOKEN_LESS:
        return 4;

        case TOKEN_GREAT:
        return 5;

        case TOKEN_LESS_EQ:
        return 6;

        case TOKEN_GREAT_EQ:
        return 7;

        case TOKEN_EXCLAMEQ:
        return 8;

        case TOKEN_EQEQ:
        return 9;

        case TOKEN_EXCLAM:
        return 10;

        case TOKEN_DOUBLE_QM:
        return 11;

        case TOKEN_LPAR:
        return 12;

        case TOKEN_RPAR:
        return 13;

        case TOKEN_ID:
        return 14;

        case TOKEN_DOLLAR:
        return 15;

        // token ktery neni podporovan expresion parserem 
        default:
        return -1;

    }

}


void eval_expression(){

    

    
}